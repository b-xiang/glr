#include <utility>

#define GLM_FORCE_RADIANS
#include <glm/gtx/string_cast.hpp>

#include "terrain/TerrainManager.hpp"
#include "terrain/Constants.hpp"

#include "models/Model.hpp"

#include "exceptions/Exception.hpp"

#include "common/logger/Logger.hpp"

namespace glr
{
namespace terrain
{

TerrainManager::TerrainManager(glw::IOpenGlDevice* openGlDevice, IFieldFunction* fieldFunction) : openGlDevice_(openGlDevice), fieldFunction_(fieldFunction)
{
	initialize();
}

TerrainManager::~TerrainManager()
{
}

void TerrainManager::initialize()
{
	followTarget_ = nullptr;
	voxelChunkMeshGenerator_ = VoxelChunkMeshGenerator(fieldFunction_);
	terrainChunks_ = std::vector< std::unique_ptr<TerrainSceneNode> >();
	
	idManager_ = IdManager();

	LOG_DEBUG( "Terrain Manager initialized." );
}

ITerrain* TerrainManager::getTerrain(glm::detail::int32 x, glm::detail::int32 y, glm::detail::int32 z) const
{
}

glm::ivec3 TerrainManager::getTargetGridLocation()
{
	glm::ivec3 retVal = glm::ivec3(0, 0, 0);

	if (followTarget_ != nullptr)
	{
		// TODO: make size not hard coded
		const glm::vec3 pos = followTarget_->getPosition() / 4.0f; // - 0.5f * totalWorldGridSize  (?)
		retVal = glm::ivec3((int)pos.x, (int)pos.y, (int)pos.z);
	}

	return retVal;
}

void TerrainManager::tick()
{
	auto followTargetGridLocation = getTargetGridLocation();
	//std::cout << glm::to_string(followTargetGridLocation) << "    " << glm::to_string(currentGridLocation_) << std::endl;
	if (followTargetGridLocation != currentGridLocation_)
	{
		previousGridLocation_ = currentGridLocation_;
		currentGridLocation_ = followTargetGridLocation;
		std::cout << "updateChunks START " << std::endl;
		this->updateChunks();
		std::cout << "updateChunks END " << std::endl;
	}
}

const int MAX_QUADRANT_DISTANCE = 2;
void TerrainManager::updateChunks()
{
	// update current location
	addChunk( currentGridLocation_ );
	// update cube around location (distance = 1)
	/*
	for (int i=-1; i <= 1; i++)
	{
		for (int j=-1; j <= 1; j++)
		{
			for (int k=-1; k <= 1; k++)
			{
				addChunk( currentGridLocation_.x+i, currentGridLocation_.y+j, currentGridLocation_.z+k );
			}
		}
	}
	*/
	
	// update cubes around location ( 1 < distance <= MAX_QUADRANT_DISTANCE)
	// TODO: level of detail
	std::cout << "currentGridLocation_: " << glm::to_string(currentGridLocation_) << std::endl;
	std::cout << "previousGridLocation_: " << glm::to_string(previousGridLocation_) << std::endl;
	for (int i=-MAX_QUADRANT_DISTANCE; i <= MAX_QUADRANT_DISTANCE; i++)
	{
		for (int j=-MAX_QUADRANT_DISTANCE; j <= MAX_QUADRANT_DISTANCE; j++)
		{
			for (int k=-MAX_QUADRANT_DISTANCE; k <= MAX_QUADRANT_DISTANCE; k++)
			{
				//std::cout << "add: " << currentGridLocation_.x+i << ", " << currentGridLocation_.y+j << ", " << currentGridLocation_.z+k << std::endl;
				addChunk( currentGridLocation_.x+i, currentGridLocation_.y+j, currentGridLocation_.z+k );
			}
		}
	}
	
	// update (i.e. hide) cubes too far from new location ( distance > MAX_QUADRANT_DISTANCE)
	auto diff = currentGridLocation_ - previousGridLocation_;
	int diffX = diff.x;
	int diffY = diff.y;
	int diffZ = diff.z;
	
	std::cout << "diff: " << glm::to_string(diff) << std::endl;
	
	// along x plane
	for (int i=0; i < std::abs(diff.x); i++)
	{
		int plane = 0;
		if (diffX >= 0)
			plane = -(MAX_QUADRANT_DISTANCE + diff.x);
		else
			plane = MAX_QUADRANT_DISTANCE + std::abs(diff.x);
		
		for (int j=-MAX_QUADRANT_DISTANCE; j <= MAX_QUADRANT_DISTANCE; j++)
		{
			for (int k=-MAX_QUADRANT_DISTANCE; k <= MAX_QUADRANT_DISTANCE; k++)
			{
				//std::cout << "remove X: " << plane << ", " << j << ", " << k << std::endl;
				removeChunk( currentGridLocation_.x+plane, currentGridLocation_.y+j, currentGridLocation_.z+k );
			}
		}
	}
	
	// along y plane
	for (int i=0; i < std::abs(diff.y); i++)
	{
		int plane = 0;
		if (diffY >= 0)
			plane = -(MAX_QUADRANT_DISTANCE + diff.y);
		else
			plane = MAX_QUADRANT_DISTANCE + std::abs(diff.y);
		
		for (int j=-MAX_QUADRANT_DISTANCE; j <= MAX_QUADRANT_DISTANCE; j++)
		{
			for (int k=-MAX_QUADRANT_DISTANCE; k <= MAX_QUADRANT_DISTANCE; k++)
			{
				//std::cout << "remove Y: " << j << ", " << plane<< ", " << k << std::endl;
				removeChunk( currentGridLocation_.x+j, currentGridLocation_.y+plane, currentGridLocation_.z+k );
			}
		}
	}
	
	// along z plane
	for (int i=0; i < std::abs(diff.z); i++)
	{
		int plane = 0;
		if (diffZ >= 0)
			plane = -(MAX_QUADRANT_DISTANCE + diff.z);
		else
			plane = MAX_QUADRANT_DISTANCE + std::abs(diff.z);
		
		for (int j=-MAX_QUADRANT_DISTANCE; j <= MAX_QUADRANT_DISTANCE; j++)
		{
			for (int k=-MAX_QUADRANT_DISTANCE; k <= MAX_QUADRANT_DISTANCE; k++)
			{
				//std::cout << "remove Z: " << j << ", " << k << ", " << plane << std::endl;
				removeChunk( currentGridLocation_.x+j, currentGridLocation_.y+k, currentGridLocation_.z+plane );
			}
		}
	}
}

void TerrainManager::postOpenGlWork(std::function<void()> work)
{
	openGlWorkMutex_.lock();
	openGlWork_.emplace_back(std::move(work));
	openGlWorkMutex_.unlock();
}

void TerrainManager::addChunk(glmd::float32 x, glmd::float32 y, glmd::float32 z)
{
	glmd::int32 i = (glmd::int32)(x / (glmd::float32)constants::CHUNK_SIZE);
	glmd::int32 j = (glmd::int32)(y / (glmd::float32)constants::CHUNK_SIZE);
	glmd::int32 k = (glmd::int32)(z / (glmd::float32)constants::CHUNK_SIZE);
	
	addChunk(i, j, k);
}

void TerrainManager::addChunk(const glm::ivec3& coordinates)
{
	addChunk(coordinates.x, coordinates.y, coordinates.z);
}

void TerrainManager::addChunk(glmd::int32 x, glmd::int32 y, glmd::int32 z)
{
	auto chunk = getChunk(x, y, z);
	
	if (chunk == nullptr)
	{
		//std::cout << "add: " << x << ", " << y << ", " << z << std::endl;
		chunk = new TerrainSceneNode(idManager_.createId(), openGlDevice_, x, y, z);
		//terrainChunks_.push_back( std::unique_ptr<TerrainSceneNode>(  ) );
		//chunk = terrainChunks_.back().get();
	}
	else
	{
		terrainChunksMutex_.lock();
		if (!chunk->isActive())
			chunk->setIsActive( true );
		terrainChunksMutex_.unlock();
		return;
	}

	// MARK: Previous lambda function
	{
		//std::cout << "addChunk START 1 " << std::this_thread::get_id() << std::endl;
		
		terrain::VoxelChunk voxelChunk = terrain::VoxelChunk(chunk->getGridX(), chunk->getGridY(), chunk->getGridZ());
	
		generateNoise(voxelChunk, *fieldFunction_);
		
		bool isEmptyOrSolid = determineIfEmptyOrSolid(voxelChunk, *fieldFunction_);

		//std::cout << "addChunk START 2 " << std::this_thread::get_id() << std::endl;
		if (!isEmptyOrSolid)
		{
			auto vertices = std::vector< glm::vec3 >();
			auto normals = std::vector< glm::vec3 >();
			auto textureBlendingValues = std::vector< glm::vec4 >();
			
			voxelChunkMeshGenerator_.generateMesh(voxelChunk, vertices, normals, textureBlendingValues);

			auto shader = openGlDevice_->getShaderProgramManager()->getShaderProgram( std::string("voxel") );
			assert( shader != nullptr );
			
			std::stringstream ss;
			ss << "terrain_" << chunk->getGridX() << "_" << chunk->getGridY() << "_" << chunk->getGridZ();
			ss << "_model";
			
			auto mesh = std::unique_ptr<TerrainMesh>( new TerrainMesh(openGlDevice_, ss.str()) );
			mesh->setVertices( vertices );
			mesh->setNormals( normals );
			mesh->setTextureBlendingData( textureBlendingValues );
			
			const std::string materialName = std::string("terrain_material_1");
			auto material = openGlDevice_->getMaterialManager()->getMaterial(materialName);
			if (material == nullptr)
			{
				material = openGlDevice_->getMaterialManager()->addMaterial(materialName);
				assert(material != nullptr);
				
				auto materialData = glr::models::MaterialData();
				materialData.name = materialName;
				materialData.ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
				materialData.diffuse = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
				// None of these are actually used (yet)
				materialData.specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				materialData.emission = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				materialData.shininess = 1.0f;
				materialData.strength = 1.0f;
				
				material->setAmbient(materialData.ambient);
				material->setDiffuse(materialData.diffuse);
				material->setSpecular(materialData.specular);
				material->setEmission(materialData.emission);
				material->setShininess(materialData.shininess);
				material->setStrength(materialData.strength);
			}
			
			chunk->attach(shader);
			
			auto meshPtr = mesh.get();
			chunk->setMesh( std::move(mesh) );
			
			//glmd::float32 posX = (glmd::float32)(chunk->getGridX()*constants::CHUNK_SIZE) * constants::RESOLUTION;
			//glmd::float32 posY = (glmd::float32)(chunk->getGridY()*constants::CHUNK_SIZE) * constants::RESOLUTION;
			//glmd::float32 posZ = (glmd::float32)(chunk->getGridZ()*constants::CHUNK_SIZE) * constants::RESOLUTION;
			
			// Note: We don't need to set position, as the model vertices already have the correct positions
			// TODO: Do we want to remove the positions from the vertices, and instead store the 'relative origin' position in the scene node (i.e. this TerrainSceneNode object)?
			//std::cout << posX << ", " << posY << ", " << posZ << std::endl;
			//this->setPosition(posX, posY, posZ);
			
			// Center the node
			chunk->translate( glm::vec3(-(glmd::float32)(constants::CHUNK_SIZE/2), 0.0f, -(glmd::float32)(constants::CHUNK_SIZE/2)) );
			
			chunk->setIsActive( true );
			auto function = [=]() {
				LOG_DEBUG( "Pushing terrain to graphics card." );

				auto texture = openGlDevice_->getTextureManager()->getTexture2DArray(std::string("terrain_textures_2d_array"));
				if (texture == nullptr)
				{
					auto textureFilenames = std::vector<std::string>();
					textureFilenames.push_back( std::string("terrain/cgrass2.jpg") );
					textureFilenames.push_back( std::string("terrain/co_stone.jpg") );
					
					auto textureSettings = glr::glw::TextureSettings();
					textureSettings.textureWrapS = GL_REPEAT;
					textureSettings.textureWrapT = GL_REPEAT;
					
					texture = openGlDevice_->getTextureManager()->addTexture2DArray(std::string("terrain_textures_2d_array"), textureSettings);
					
					assert(texture != nullptr);
					
					const std::string& basepath = openGlDevice_->getOpenGlDeviceSettings().defaultTextureDir;
				
					utilities::ImageLoader il = utilities::ImageLoader();
					
					auto images = std::vector< std::unique_ptr<utilities::Image> >();
					for (auto& s : textureFilenames)
					{
						auto image = il.loadImageData(basepath + s);
						if (image.get() == nullptr)
						{
							// Cleanup
							images.clear();
							std::string msg = std::string( "Unable to load texture for texture 2d array: " + s );
							LOG_ERROR( msg );
							throw exception::Exception( msg );
						}
						
						images.push_back( std::move(image) );
					}
					
					auto imagesAsPointers = std::vector<utilities::Image*>();
					for (auto& image : images)
					{
						imagesAsPointers.push_back( image.get() );
					}
					
					texture->setData( imagesAsPointers );
					
					texture->allocateVideoMemory();
					texture->pushToVideoMemory();
				}
				
				// We wrap this pointer in a unique ptr eventually
				auto model = new models::Model(glr::Id(), std::string(""), meshPtr, texture, material, std::vector<glw::IAnimation*>(), glw::BoneNode(), glm::mat4(), openGlDevice_);
				
				// Create the model
				auto modelPtr = std::unique_ptr<models::IModel>( model );
				chunk->attach( modelPtr.get() );
				chunk->setModel( std::move(modelPtr) );
				
				// TODO: Load material only once
				if (material->getBufferId() == 0)
				{
					material->allocateVideoMemory();
					material->pushToVideoMemory();
				}
				
				GLint loc = shader->getVertexAttributeLocationByName( std::string("in_texBlend") );
				assert(loc >= 0);
				
				meshPtr->setShaderVariableLocation( loc );
				
				meshPtr->allocateVideoMemory();
				meshPtr->pushToVideoMemory();
				
				std::stringstream ss;
				ss << "terrain_" << chunk->getGridX() << "_" << chunk->getGridY() << "_" << chunk->getGridZ();
				chunk->setName( ss.str() );
				
				this->addChunk( chunk );
			};

			postOpenGlWork( function );
		}
		else
		{
			delete chunk;
		}
		//std::cout << "addChunk END " << std::this_thread::get_id() << std::endl;
	}
}

void TerrainManager::addChunk(TerrainSceneNode* chunk)
{
	terrainChunksMutex_.lock();
	auto up = std::unique_ptr<TerrainSceneNode>( chunk );
	terrainChunks_.push_back( std::move(up) );
	terrainChunksMutex_.unlock();
}

void TerrainManager::removeChunk(glmd::float32 x, glmd::float32 y, glmd::float32 z)
{
	glmd::int32 i = (glmd::int32)(x / (glmd::float32)constants::CHUNK_SIZE);
	glmd::int32 j = (glmd::int32)(y / (glmd::float32)constants::CHUNK_SIZE);
	glmd::int32 k = (glmd::int32)(z / (glmd::float32)constants::CHUNK_SIZE);
	
	removeChunk(i, j, k);
}

void TerrainManager::removeChunk(const glm::ivec3& coordinates)
{
	removeChunk(coordinates.x, coordinates.y, coordinates.z);
}

void TerrainManager::removeChunk(glmd::int32 x, glmd::int32 y, glmd::int32 z)
{
	auto chunk = getChunk(x, y, z);
	
	if (chunk != nullptr)
	{
		terrainChunksMutex_.lock();
		
		if (chunk->isActive())
		{			
			chunk->setIsActive( false );
			
			// MARK: Previous lambda function
			{
				//std::cout << "remove: " << x << ", " << y << ", " << z << std::endl;
				//std::cout << "removeChunk START " << std::this_thread::get_id() << std::endl;
				
				auto f = [=]() {
					chunk->freeVideoMemory();
					this->removeChunk( chunk );
				};
				
				
				postOpenGlWork( f );
				//std::cout << "removeChunk END " << std::this_thread::get_id() << std::endl;
				//this->openGlLoader_->postWork( std::bind(&TerrainSceneNode::freeVideoMemory, c) );
			}
		}
		
		terrainChunksMutex_.unlock();
	}
}

void TerrainManager::removeChunk(TerrainSceneNode* chunk)
{
	glmd::int32 index = 0;
	bool found = false;
	
	terrainChunksMutex_.lock();
	for ( auto& c : terrainChunks_ )
	{
		if (c.get() == chunk)
		{
			found = true;
			break;
		}
		index++;
	}
	if (found)
		terrainChunks_.erase(terrainChunks_.begin() + index);
	terrainChunksMutex_.unlock();
}

void TerrainManager::removeAllChunks()
{
	// TODO: Implement
	assert(0);
	idManager_ = IdManager();
}

TerrainSceneNode* TerrainManager::getChunk(glmd::float32 x, glmd::float32 y, glmd::float32 z)
{
	glmd::int32 i = (glmd::int32)(x / (glmd::float32)constants::CHUNK_SIZE);
	glmd::int32 j = (glmd::int32)(y / (glmd::float32)constants::CHUNK_SIZE);
	glmd::int32 k = (glmd::int32)(z / (glmd::float32)constants::CHUNK_SIZE);
	
	return getChunk(i, j, k);
}

TerrainSceneNode* TerrainManager::getChunk(const glm::ivec3& coordinates)
{	
	return getChunk(coordinates.x, coordinates.y, coordinates.z);
}

TerrainSceneNode* TerrainManager::getChunk(glmd::int32 x, glmd::int32 y, glmd::int32 z)
{
	//std::cout << "getChunk START " << std::this_thread::get_id() << std::endl;
	TerrainSceneNode* retVal = nullptr;
	
	terrainChunksMutex_.lock();
	for ( auto& chunk : terrainChunks_ )
	{
		if (chunk->isActive() && chunk->getGridX() == x && chunk->getGridY() == y && chunk->getGridZ() == z)
		{
			retVal = chunk.get();
			break;
		}
	}
	terrainChunksMutex_.unlock();
	//std::cout << "getChunk END " << std::this_thread::get_id() << std::endl;
	return retVal;
}

void TerrainManager::update(glmd::uint32 maxUpdates)
{
	{
		std::lock_guard<std::mutex> lock(openGlWorkMutex_);
		for (glmd::uint32 i=0; i < maxUpdates && i < openGlWork_.size(); i++)
		{
			auto work = std::move(openGlWork_.front());
			openGlWork_.pop_front();
			work();
		}
	}
}

void TerrainManager::render()
{
	terrainChunksMutex_.lock();
	for ( auto& terrain : terrainChunks_ )
	{
		if (terrain.get() != nullptr && terrain->isActive())
		{
			terrain->render();
		}
	}
	terrainChunksMutex_.unlock();
}

void TerrainManager::setFollowTarget(ISceneNode* target)
{
}

ISceneNode* TerrainManager::getFollowTarget() const
{
}

void TerrainManager::generate()
{
}

void TerrainManager::generate(glm::detail::int32 x, glm::detail::int32 y, glm::detail::int32 z)
{
}

void TerrainManager::generate(ITerrain* terrain)
{
}

void TerrainManager::addTerrainManagerEventListener(ITerrainManagerEventListener* listener)
{
}

void TerrainManager::removeTerrainManagerEventListener(ITerrainManagerEventListener* listener)
{
}

void TerrainManager::serialize(const std::string& filename)
{
}

void TerrainManager::deserialize(const std::string& filename)
{
}

glm::detail::float32 TerrainManager::getWidth() const
{
}

glm::detail::float32 TerrainManager::getHeight() const
{
}

glm::detail::float32 TerrainManager::getLength() const
{
}

glm::detail::int32 TerrainManager::getBlockSize() const
{
}

}
}
