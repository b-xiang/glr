#include <sstream>
#include <utility>

#include "glw/Texture2D.hpp"

#include "common/logger/Logger.hpp"
#include "common/utilities/Macros.hpp"

#include "exceptions/GlException.hpp"
#include "exceptions/FormatException.hpp"
#include "exceptions/InvalidArgumentException.hpp"

namespace glr
{
namespace glw
{

Texture2D::Texture2D() : internalFormat_(utilities::Format::FORMAT_UNKNOWN), bufferId_(0)
{
	openGlDevice_ = nullptr;
	name_ = std::string();
	settings_ = TextureSettings();
	
	isLocalDataLoaded_ = false;
	isVideoMemoryAllocated_ = false;
	isDirty_ = false;
	
	this->addBindListener(openGlDevice_);
}

Texture2D::Texture2D(IOpenGlDevice* openGlDevice, std::string name, TextureSettings settings) : openGlDevice_(openGlDevice), name_(std::move(name)), settings_(std::move(settings)), internalFormat_(utilities::Format::FORMAT_UNKNOWN), bufferId_(0)
{
	isLocalDataLoaded_ = false;
	isDirty_ = false;
	
	this->addBindListener(openGlDevice_);
}

Texture2D::Texture2D(utilities::Image* image, IOpenGlDevice* openGlDevice, std::string name, TextureSettings settings, bool initialize) : openGlDevice_(openGlDevice), name_(std::move(name)), settings_(std::move(settings)), internalFormat_(utilities::Format::FORMAT_UNKNOWN), bufferId_(0)
{
	isLocalDataLoaded_ = false;
	isVideoMemoryAllocated_ = false;
	isDirty_ = false;
	
	if ( image == nullptr )
	{
		std::string message = std::string("No image data sent into Texture2D constructor - use different constructor if no image data is available.");
		LOG_ERROR(message);
		throw exception::InvalidArgumentException(message);
	}
	
	// Copy image data
	setData( image );
	
	if (initialize)
	{
		loadLocalData();
		allocateVideoMemory();
		pushToVideoMemory();
	}
	
	this->addBindListener(openGlDevice_);
}

Texture2D::~Texture2D()
{
	freeVideoMemory();
	
	if (openGlDevice_->getCurrentlyBoundTexture() == this)
	{
		for ( auto bindListener : bindListeners_ )
		{
			bindListener->textureBindCallback( nullptr );
		}
	}
}

void Texture2D::bind(GLuint texturePosition)
{
	assert(bufferId_ >= 0);
	
	// Don't bind if we are already bound
	if (openGlDevice_->getCurrentlyBoundTexture() == this)
	{
		return;
	}
	
	glActiveTexture(GL_TEXTURE0 + texturePosition);
	glBindTexture(GL_TEXTURE_2D, bufferId_);
	
	// Notify all listeners that we have bound this texture
	for ( auto bindListener : bindListeners_ )
	{
		bindListener->textureBindCallback( static_cast<ITexture*>( this ) );
	}
	
	//bindPoint_ = openGlDevice_->bindBuffer( bufferId_ );
	//std::cout << "texture: " << name_ << " | " << bufferId_ << " | " << bindPoint_ << std::endl;
	// to unbind, we use the following
	//glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint Texture2D::getBufferId() const
{
	return bufferId_;
}

GLuint Texture2D::getBindPoint() const
{
	return bindPoint_;
}

void Texture2D::setData(utilities::Image* image)
{
	image_ = *image;
	internalFormat_ = utilities::getOpenGlImageFormat(image_.format);
	
	isDirty_ = true;
}

utilities::Image* Texture2D::getData()
{
	return &image_;
}

void Texture2D::pushToVideoMemory()
{
	bind();
	
	if (internalFormat_ == utilities::Format::FORMAT_UNKNOWN)
	{
		std::string msg = std::string( "Texture2D::pushToVideoMemory: Unknown image format." );
		LOG_ERROR( msg );
		throw exception::FormatException( msg );
	}
	
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,  image_.width, image_.height, internalFormat_, GL_UNSIGNED_BYTE, &(image_.data[0]));

	// error check
	GlError err = openGlDevice_->getGlError();
	if (err.type != GL_NONE)
	{
		// Cleanup
		freeVideoMemory();
		
		std::string msg = std::string( "Error while loading texture '" + name_ + "' in OpenGL: " + err.name);
		LOG_ERROR( msg );
		throw exception::GlException( msg );
	}
	else
	{
		LOG_DEBUG( "Successfully loaded texture." );
	}
	
	isDirty_ = false;
}

void Texture2D::pullFromVideoMemory()
{
	
	// TODO: implement
	/*
	//bind();
	GLfloat* pixels = new GLfloat[size * size];
	glGetTexImage( GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, pixels );
	
	pixelDataVector.resize( size * size );
	for ( int i = 0; i < size * size; i++ )
	{
	  pixelDataVector[i] =  (float) pixels[i];
	}
	*/
}

void Texture2D::loadLocalData()
{
	isLocalDataLoaded_ = true;
}

void Texture2D::freeLocalData()
{
	// Clear up the data, but leave the width, height, and format unchanged
	image_.data = std::vector<char>();
	
	isLocalDataLoaded_ = false;
}

void Texture2D::freeVideoMemory()
{
	if (bufferId_ == 0)
	{
		LOG_WARN( "Cannot free video memory - buffer does not exist for texture." );
		return;
	}
	
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &bufferId_);
	
	OPENGL_CHECK_ERRORS(openGlDevice_)
	
	bufferId_ = 0;
}

void Texture2D::allocateVideoMemory()
{
	if (bufferId_ != 0)
	{
		std::string msg = std::string( "Cannot allocate video memory - buffer already exists for texture.");
		LOG_ERROR( msg );
		throw exception::GlException( msg );
	}
	
	glGenTextures(1, &bufferId_);

	glBindTexture(GL_TEXTURE_2D, bufferId_);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, settings_.textureWrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, settings_.textureWrapT);
	
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat_,  image_.width, image_.height, 0, internalFormat_, GL_UNSIGNED_BYTE, nullptr);
	
	GlError err = openGlDevice_->getGlError();
	if (err.type != GL_NONE)
	{
		// Cleanup
		freeVideoMemory();
		
		std::string msg = std::string( "Error while allocating memory for texture '" + name_ + "' in OpenGL: " + err.name);
		LOG_ERROR( msg );
		throw exception::GlException( msg );
	}
	else
	{
		LOG_DEBUG( "Successfully allocated memory for texture." );
	}
	
	isVideoMemoryAllocated_ = true;
}

bool Texture2D::isVideoMemoryAllocated() const
{
	return isVideoMemoryAllocated_;
}

bool Texture2D::isLocalDataLoaded() const
{
	return isLocalDataLoaded_;
}

bool Texture2D::isDirty() const
{
	return isDirty_;
}

const std::string& Texture2D::getName() const
{
	return name_;
}

void Texture2D::addBindListener(ITextureBindListener* bindListener)
{
	if (bindListener == nullptr)
	{
		std::string msg = std::string( "Bind listener must not be null." );
		LOG_ERROR( msg );
		throw exception::InvalidArgumentException(msg);
	}
	
	bindListeners_.push_back(bindListener);
}

void Texture2D::removeBindListener(ITextureBindListener* bindListener)
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

void Texture2D::removeAllBindListeners()
{
	bindListeners_.clear();
}

void Texture2D::serialize(const std::string& filename)
{
	std::ofstream ofs(filename.c_str());
	serialize::TextOutArchive textOutArchive(ofs);
	serialize(textOutArchive);
}

void Texture2D::serialize(serialize::TextOutArchive& outArchive)
{
	outArchive << *this;
}

void Texture2D::deserialize(const std::string& filename)
{
	std::ifstream ifs(filename.c_str());
	serialize::TextInArchive textInArchive(ifs);
	deserialize(textInArchive);
}

void Texture2D::deserialize(serialize::TextInArchive& inArchive)
{
	inArchive >> *this;
	loadLocalData();
}


/*
template<class Archive> void Texture2D::serialize(Archive& ar, const unsigned int version)
{
	boost::serialization::void_cast_register<Texture2D, ITexture>(
		static_cast<Texture2D*>(nullptr),
		static_cast<ITexture*>(nullptr)
	);

	ar & name_;
	ar & image_;
	ar & internalFormat_;
}
*/

}
}

BOOST_CLASS_EXPORT(glr::glw::ITexture)
BOOST_CLASS_EXPORT_GUID(glr::glw::Texture2D, "glr::glw::Texture2D")
