#include <algorithm>

#include <GL/glew.h>

#include "glw/shaders/ShaderProgramManager.hpp"
#include "glw/shaders/IShader.hpp"

#include "glw/OpenGlDevice.hpp"

#include "common/logger/Logger.hpp"

#include "exceptions/GlException.hpp"
#include "exceptions/InvalidArgumentException.hpp"

#include "glw/MaterialManager.hpp"
#include "glw/TextureManager.hpp"
#include "glw/MeshManager.hpp"
#include "glw/AnimationManager.hpp"

namespace glr
{
namespace glw
{
	
OpenGlDevice::OpenGlDevice(const OpenGlDeviceSettings& settings)
{
	initialize( settings );
}

OpenGlDevice::~OpenGlDevice()
{
}

/**
 * Will setup our OpenGlDevice.
 * 
 * @param properties Used to initialize the OpenGlDevice settings
 */
void OpenGlDevice::initialize(const OpenGlDeviceSettings& settings)
{	
	initializeSettings( settings );
	bufferIds_ = std::vector<GLuint>();
	bindPoints_ = std::vector<GLuint>();
	boundBuffers_ = std::unordered_map<GLuint, GLuint>();
	
	maxNumBindPoints_ = 0;
	currentBindPoint_ = 0;
	
	modelMatrix_ = glm::mat4();
	viewMatrix_ = glm::mat4();
	projectionMatrix_ = glm::mat4();
	
	// Find and set the number of bind points available
	GLint maxNumBindPoints = 0;
	glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxNumBindPoints);
	
	if (maxNumBindPoints > 0)
	{
		maxNumBindPoints_ = (GLuint) maxNumBindPoints;
	}
	else
	{
		std::stringstream ss;
		ss << std::string("No bind points available in this OpenGL implementation.");
		LOG_ERROR( ss.str() );
		throw exception::GlException( ss.str() );
	}
	
	for (GLuint i=0; i < maxNumBindPoints_; i++)
	{
		bindPoints_.push_back(i);
	}
	
	//bindings_ = std::vector< glmd::int32 >( 1000, -1 );
	
	shaderProgramManager_ = std::unique_ptr< shaders::ShaderProgramManager >(new shaders::ShaderProgramManager(this, true));
	
	materialManager_ = std::unique_ptr<IMaterialManager>( new MaterialManager(this) );
	textureManager_ = std::unique_ptr<ITextureManager>( new TextureManager(this) );
	meshManager_ = std::unique_ptr<IMeshManager>( new MeshManager(this) );
	animationManager_ = std::unique_ptr<IAnimationManager>( new AnimationManager(this) );
}

/**
 * Will override any properties in the OpenGlDevice settings with values set in the parameter settings.
 * 
 * @param settings The settings used to override the OpenGlDevice settings
 */
void OpenGlDevice::initializeSettings(const OpenGlDeviceSettings& settings)
{
	if ( !settings.defaultTextureDir.empty() )
	{
		settings_.defaultTextureDir = settings.defaultTextureDir;
	}
}

void OpenGlDevice::destroy()
{
}

const glm::mat4& OpenGlDevice::getViewMatrix()
{	
	return viewMatrix_;
}

const glm::mat4& OpenGlDevice::getProjectionMatrix()
{
	return projectionMatrix_;
}

const glm::mat4& OpenGlDevice::getModelMatrix()
{
	return modelMatrix_;
}

void OpenGlDevice::setModelMatrix(const glm::mat4& modelMatrix)
{
	modelMatrix_ = modelMatrix;
}

void OpenGlDevice::setViewMatrix(const glm::mat4& viewMatrix)
{
	viewMatrix_ = viewMatrix;
}

void OpenGlDevice::setProjectionMatrix(const glm::mat4& projectionMatrix)
{
	projectionMatrix_ = projectionMatrix;
}

GLuint OpenGlDevice::createBufferObject(GLenum target, glmd::uint32 totalSize, const void* dataPointer, GLenum usage)
{
	GLuint bufferId = 0;
	glGenBuffers(1, &bufferId);
	glBindBuffer(target, bufferId);

	glBufferData(target, totalSize, dataPointer, usage);
	glBindBuffer(target, 0);
	
	GlError err = getGlError();
	if (err.type != GL_NONE)
	{
		// Cleanup
		releaseBufferObject( bufferId );
		
		std::string msg = std::string("Error while creating buffer object in OpenGl: ") + err.name;
		LOG_ERROR( msg );
		throw exception::GlException( msg );
	}
	
	bufferIds_.push_back(bufferId);
	
	return bufferId;
}

void OpenGlDevice::releaseBufferObject(GLuint bufferId)
{
	auto it = std::find(bufferIds_.begin(), bufferIds_.end(), bufferId);
	
	if (it == bufferIds_.end())
	{
		// warning - buffer object not present
		LOG_WARN("Buffer object with id " << bufferId << " not present - cannot release it.");
		return;
	}	
	
	unbindBuffer( bufferId );
	
	bufferIds_.erase(it);
	glDeleteBuffers(1, &bufferId);
	
	GlError err = getGlError();
	if (err.type != GL_NONE)
	{
		std::string msg = std::string("Error while releasing buffer object in OpenGl: ") + err.name;
		LOG_ERROR( msg );
		throw exception::GlException( msg );
	}
}

GLuint OpenGlDevice::createFrameBufferObject(GLenum target, glmd::uint32 totalSize, const void* dataPointer)
{	
	GLuint bufferId = 0;
	glGenFramebuffers(1, &bufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, bufferId);

	//glBufferData(target, totalSize, dataPointer, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_FRAMEBUFFER, 0);
	
	GlError err = getGlError();
	if (err.type != GL_NONE)
	{
		// Cleanup
		releaseBufferObject( bufferId );
		
		std::string msg = std::string("Error while creating frame buffer object in OpenGl: ") + err.name;
		LOG_ERROR( msg );
		throw exception::GlException( msg );
	}
	
	bufferIds_.push_back(bufferId);
	
	return bufferId;
}

void OpenGlDevice::releaseFrameBufferObject(GLuint bufferId)
{
	/*
	auto it = std::find(bufferIds_.begin(), bufferIds_.end(), bufferId);
	
	if (it == bufferIds_.end())
	{
		// warning - buffer object not present
		return;
	}	
	
	unbindBuffer( bufferId );
	
	bufferIds_.erase(it);
	glDeleteBuffers(1, &bufferId);
	*/
	
	GlError err = getGlError();
	if (err.type != GL_NONE)
	{
		std::string msg = std::string("Error while releasing frame buffer object in OpenGl: ") + err.name;
		LOG_ERROR( msg );
		throw exception::GlException( msg );
	}
}

/**
 * 
 */
void OpenGlDevice::bindBuffer(GLuint bufferId, GLuint bindPoint)
{
	//std::cout << bufferId << " | " << bindPoint << " / " << maxNumBindPoints_ << std::endl;
	assert(bindPoint >= 0);
	assert(bindPoint < maxNumBindPoints_);

	glBindBufferBase(GL_UNIFORM_BUFFER, bindPoint, bufferId);
	
	/*
	if (bindings_[bufferId] < 0)
	{
		GLuint bindPoint = bindPoints_[currentBindPoint_];
		currentBindPoint_++;
		
		if ( currentBindPoint_ >= bindPoints_.size() )
			currentBindPoint_ = 0;
			
		bindings_[bufferId] = bindPoint;
		std::cout << bufferId << ": " << bindings_[bufferId] << std::endl;
	}
	
	assert(bindings_[bufferId] >= 0);
	assert(bindings_[bufferId] < maxNumBindPoints_);
	
	glBindBufferBase(GL_UNIFORM_BUFFER, bindings_[bufferId], bufferId);
		
	return bindings_[bufferId];
	*/
	

	// This algorithm was my first attempt at making it more 'efficient' by keeping a cache, and by moving
	// recently used bind points to the bottom of a 'queue' (so they wouldn't be chosen again for a longer
	// period of time).  However, it didn't work :S
	/*
	GLuint bindPoint = 0; 
	 
	// Check if we have bound this buffer already
	auto boundBufferIter = boundBuffers_.find( bufferId );

	if (boundBufferIter != boundBuffers_.end())
	{
		// Pull bind point out of the list and push it on the back
		auto it = std::find( bindPoints_.begin(), bindPoints_.end(), boundBufferIter->second );
		bindPoint = *it;
		bindPoints_.erase( it );
		bindPoints_.push_back( bindPoint );
	} else
	{
		// If we haven't bound it already, use the first available bind point
		bindPoint = bindPoints_[0];

		// Remove any buffers from the bound buffers list that were bound to bindPoint (as it is now used by a different buffer)
		auto it = std::find( bindPoints_.begin(), bindPoints_.end(), bindPoint);
		if ( it != bindPoints_.end() ) 
		{
		    bindPoints_.erase( it );
		}

		// Pop bind point off the top of the list and push it on the back
		bindPoints_.erase( bindPoints_.begin() );
		bindPoints_.push_back( bindPoint );

		// Bind the buffer
		boundBuffers_[bufferId] = bindPoint;
		
	}

	//glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBindBufferBase(GL_UNIFORM_BUFFER, bindPoint, bufferId);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	return bindPoint;
	*/
}

// Do I need this function any more?
void OpenGlDevice::unbindBuffer(GLuint bufferId)
{
	auto it = boundBuffers_.find( bufferId );
	if (it != boundBuffers_.end())
	{
		boundBuffers_.erase( it );
	}
}

GLuint OpenGlDevice::getBindPoint()
{
	// TODO: Do I need a better algorithm here?
	GLuint bindPoint = bindPoints_[currentBindPoint_];
	currentBindPoint_++;

	if ( currentBindPoint_ >= bindPoints_.size() )
	{
		//currentBindPoint_ = 0;
		// We have run out of bind points
		std::stringstream ss;
		ss << std::string("Unable to get bind point - maximum number of bind points (") << currentBindPoint_ << std::string(")reached.");
		LOG_ERROR( ss.str() );
		throw exception::GlException( ss.str() );
	}

	assert(bindPoint >= 0);
	assert(bindPoint < maxNumBindPoints_);

	return bindPoint;
}

void OpenGlDevice::invalidateBindPoints()
{
	currentBindPoint_ = 0;
}

glm::detail::uint32 OpenGlDevice::getMaximumNumberOfBindPoints()
{
	return maxNumBindPoints_;
}

GlError OpenGlDevice::getGlError()
{
	GlError glErrorObj = GlError();
	
	GLenum glError = glGetError();
	
	if ( glError )
	{
		switch ( glError )
		{
		case GL_INVALID_ENUM:
			glErrorObj.type = glError;
			glErrorObj.name = "GL_INVALID_ENUM";
			break;

		case GL_INVALID_VALUE:
			glErrorObj.type = glError;
			glErrorObj.name = "GL_INVALID_VALUE";
			break;

		case GL_INVALID_OPERATION:
			glErrorObj.type = glError;
			glErrorObj.name = "GL_INVALID_OPERATION";
			break;

		case GL_STACK_OVERFLOW:
			glErrorObj.type = glError;
			glErrorObj.name = "GL_STACK_OVERFLOW";
			break;

		case GL_STACK_UNDERFLOW:
			glErrorObj.type = glError;
			glErrorObj.name = "GL_STACK_UNDERFLOW";
			break;

		case GL_OUT_OF_MEMORY:
			glErrorObj.type = glError;
			glErrorObj.name = "GL_OUT_OF_MEMORY";
			break;

		case GL_INVALID_FRAMEBUFFER_OPERATION:
			glErrorObj.type = glError;
			glErrorObj.name = "GL_INVALID_FRAMEBUFFER_OPERATION​";
			break;
		}
	}
	
	return glErrorObj;
}

shaders::IShaderProgramManager* OpenGlDevice::getShaderProgramManager()
{
	return shaderProgramManager_.get();
}

IMaterialManager* OpenGlDevice::getMaterialManager()
{
	return materialManager_.get();
}

ITextureManager* OpenGlDevice::getTextureManager()
{
	return textureManager_.get();
}

IMeshManager* OpenGlDevice::getMeshManager()
{
	return meshManager_.get();
}

IAnimationManager* OpenGlDevice::getAnimationManager()
{
	return animationManager_.get();
}

const OpenGlDeviceSettings& OpenGlDevice::getOpenGlDeviceSettings()
{
	return settings_;
}

void OpenGlDevice::shaderBindCallback(shaders::IShaderProgram* shader)
{
	currentlyBoundShaderProgram_ = shader;
	
	// Notify all listeners that we have bound this shader program
	for ( auto bindListener : bindListeners_ )
	{
		bindListener->shaderBindCallback( currentlyBoundShaderProgram_ );
	}
}

void OpenGlDevice::textureBindCallback(ITexture* texture)
{
	currentlyBoundTexture_ = texture;
	
	// Notify all listeners that we have bound this texture program
	for ( auto bindListener : textureBindListeners_ )
	{
		bindListener->textureBindCallback( currentlyBoundTexture_ );
	}
}

void OpenGlDevice::materialBindCallback(IMaterial* material)
{
	currentlyBoundMaterial_ = material;
	
	// Notify all listeners that we have bound this material program
	for ( auto bindListener : materialBindListeners_ )
	{
		bindListener->materialBindCallback( currentlyBoundMaterial_ );
	}
}

void OpenGlDevice::addBindListener(shaders::IShaderProgramBindListener* bindListener)
{
	if (bindListener == nullptr)
	{
		std::string msg = std::string( "Bind listener must not be null." );
		LOG_ERROR( msg );
		throw exception::InvalidArgumentException(msg);
	}
	
	bindListeners_.push_back(bindListener);
}

void OpenGlDevice::removeBindListener(shaders::IShaderProgramBindListener* bindListener)
{
	if (bindListener == nullptr)
	{
		std::string msg = std::string( "Bind listener must not be null." );
		LOG_ERROR( msg );
		throw exception::InvalidArgumentException(msg);
	}
	
	auto it = std::find(bindListeners_.begin(), bindListeners_.end(), bindListener);

	if ( it != bindListeners_.end())
	{
		bindListeners_.erase(it);
	}
}

void OpenGlDevice::removeAllBindListeners()
{
	bindListeners_.clear();
}

void OpenGlDevice::addBindListener(ITextureBindListener* bindListener)
{
	if (bindListener == nullptr)
	{
		std::string msg = std::string( "Bind listener must not be null." );
		LOG_ERROR( msg );
		throw exception::InvalidArgumentException(msg);
	}
	
	textureBindListeners_.push_back(bindListener);
}

void OpenGlDevice::removeBindListener(ITextureBindListener* bindListener)
{
	if (bindListener == nullptr)
	{
		std::string msg = std::string( "Bind listener must not be null." );
		LOG_ERROR( msg );
		throw exception::InvalidArgumentException(msg);
	}
	
	auto it = std::find(textureBindListeners_.begin(), textureBindListeners_.end(), bindListener);

	if ( it != textureBindListeners_.end())
	{
		textureBindListeners_.erase(it);
	}
}

void OpenGlDevice::removeAllTextureBindListeners()
{
	textureBindListeners_.clear();
}

void OpenGlDevice::addBindListener(IMaterialBindListener* bindListener)
{
	if (bindListener == nullptr)
	{
		std::string msg = std::string( "Bind listener must not be null." );
		LOG_ERROR( msg );
		throw exception::InvalidArgumentException(msg);
	}
	
	materialBindListeners_.push_back(bindListener);
}

void OpenGlDevice::removeBindListener(IMaterialBindListener* bindListener)
{
	if (bindListener == nullptr)
	{
		std::string msg = std::string( "Bind listener must not be null." );
		LOG_ERROR( msg );
		throw exception::InvalidArgumentException(msg);
	}
	
	auto it = std::find(materialBindListeners_.begin(), materialBindListeners_.end(), bindListener);

	if ( it != materialBindListeners_.end())
	{
		materialBindListeners_.erase(it);
	}
}

void OpenGlDevice::removeAllMaterialBindListeners()
{
	materialBindListeners_.clear();
}

void OpenGlDevice::unbindAllShaderPrograms()
{
	glUseProgram(0);
	
	shaderBindCallback( nullptr );
}

void OpenGlDevice::unbindAllTextures()
{
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	
	textureBindCallback( nullptr );
}

void OpenGlDevice::unbindAllMaterials()
{
	// TODO: Implement?
	
	materialBindCallback( nullptr );
}

shaders::IShaderProgram* OpenGlDevice::getCurrentlyBoundShaderProgram() const
{
	return currentlyBoundShaderProgram_;
}

IMaterial* OpenGlDevice::getCurrentlyBoundMaterial() const
{
	return currentlyBoundMaterial_;
}

ITexture* OpenGlDevice::getCurrentlyBoundTexture() const
{
	return currentlyBoundTexture_;
}

}
}
