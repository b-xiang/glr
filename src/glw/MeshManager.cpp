#include "Configure.hpp"

#ifdef OS_WINDOWS
#include <windows.h>
#endif

#include "glw/MeshManager.hpp"


namespace glr
{
namespace glw
{
	
MeshManager::MeshManager(IOpenGlDevice* openGlDevice) : openGlDevice_(openGlDevice)
{
}

MeshManager::~MeshManager()
{
}

Mesh* MeshManager::getMesh(const std::string& name) const
{
	auto it = meshes_.find(name);
	if ( it != meshes_.end() )
	{
		LOG_DEBUG( "Mesh '" + name + "' found." );
		return it->second.get();
	}

	LOG_DEBUG( "Mesh '" + name + "' not found." );
	
	return nullptr;
}

Mesh* MeshManager::addMesh(const std::string& name)
{
	LOG_DEBUG( "Loading mesh..." );

	auto it = meshes_.find(name);
	if ( it != meshes_.end() && it->second.get() != nullptr )
	{
		LOG_DEBUG( "Mesh already exists." );
		return it->second.get();
	}

	LOG_DEBUG( "Creating Mesh." );
	meshes_[name] = std::unique_ptr<Mesh>(new Mesh(openGlDevice_, name));

	return meshes_[name].get();
}

Mesh* MeshManager::addMesh(
		const std::string& name, 
		std::vector< glm::vec3 > vertices, 
		std::vector< glm::vec3 > normals,
		std::vector< glm::vec2 > textureCoordinates,
		std::vector< glm::vec4 > colors,
		std::vector< VertexBoneData > bones,
		BoneData boneData
	)
{
	LOG_DEBUG( "Loading mesh..." );

	auto it = meshes_.find(name);
	if ( it != meshes_.end() && it->second.get() != nullptr )
	{
		LOG_DEBUG( "Mesh already exists." );
		return it->second.get();
	}

	LOG_DEBUG( "Creating Mesh." );
	meshes_[name] = std::unique_ptr<Mesh>(new Mesh(openGlDevice_, name, vertices, normals, textureCoordinates, colors, bones, boneData));

	return meshes_[name].get();
}

Mesh* MeshManager::addMesh(
		const std::string& name, 
		std::vector< glm::vec3 > vertices, 
		std::vector< glm::vec3 > normals,
		std::vector< glm::vec2 > textureCoordinates,
		std::vector< glm::vec4 > colors
	)
{
	LOG_DEBUG( "Loading mesh..." );

	auto it = meshes_.find(name);
	if ( it != meshes_.end() && it->second.get() != nullptr )
	{
		LOG_DEBUG( "Mesh already exists." );
		return it->second.get();
	}

	LOG_DEBUG( "Creating Mesh." );
	meshes_[name] = std::unique_ptr<Mesh>(new Mesh(openGlDevice_, name, vertices, normals, textureCoordinates, colors));

	return meshes_[name].get();
}

}
}
