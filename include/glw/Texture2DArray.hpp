#ifndef TEXTURE2DARRAY_H_
#define TEXTURE2DARRAY_H_

#include <atomic>

#include <GL/glew.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "shaders/IShaderProgram.hpp"
#include "IOpenGlDevice.hpp"
#include "ITexture.hpp"
#include "ITextureBindListener.hpp"

#include "common/utilities/ImageLoader.hpp"

namespace glr
{
namespace glw
{

namespace glmd = glm::detail;

class IOpenGlDevice;

class Texture2DArray : public ITexture
{
public:
	Texture2DArray(IOpenGlDevice* openGlDevice, std::string name, TextureSettings settings = TextureSettings());
	Texture2DArray(const std::vector<utilities::Image*>& images, IOpenGlDevice* openGlDevice, std::string name, TextureSettings settings = TextureSettings(), bool initialize = true);
	virtual ~Texture2DArray();

	virtual void bind(GLuint texturePosition = 0);
	
	GLuint getBufferId() const;
	GLuint getBindPoint() const;
	
	void setData(const std::vector<utilities::Image*>& images);
	std::vector<utilities::Image>& getData();
	
	virtual void allocateVideoMemory();
	virtual void pushToVideoMemory();
	virtual void pullFromVideoMemory();
	virtual void freeVideoMemory();
	virtual bool isVideoMemoryAllocated() const;
	virtual void loadLocalData();
	virtual void freeLocalData();
	virtual bool isLocalDataLoaded() const;
	virtual bool isDirty() const;

	virtual void serialize(const std::string& filename);
	virtual void serialize(serialize::TextOutArchive& outArchive);

	virtual void deserialize(const std::string& filename);
	virtual void deserialize(serialize::TextInArchive& inArchive);

	virtual const std::string& getName() const;
	void setName(std::string name);
	
	virtual void addBindListener(ITextureBindListener* bindListener);
	virtual void removeBindListener(ITextureBindListener* bindListener);
	void removeAllBindListeners();

private:
	/**
	 * Required by serialization.
	 */
	Texture2DArray();
	
	IOpenGlDevice* openGlDevice_;
	std::string name_;
	TextureSettings settings_;
	
	GLuint bufferId_;
	GLuint bindPoint_;
	
	std::vector<utilities::Image> images_;
	
	std::atomic<bool> isLocalDataLoaded_;
	std::atomic<bool> isVideoMemoryAllocated_;
	std::atomic<bool> isDirty_;
	
	std::vector<ITextureBindListener*> bindListeners_;
	
	/**
	 * Helper method - returns true if all of the images in the vector are the same internal format, and false otherwise.
	 */
	bool areImagesSameFormat();
	
	friend class boost::serialization::access;
	
	template<class Archive> inline void serialize(Archive& ar, const unsigned int version);
};

}
}

#include "Texture2DArray.inl"

#endif /* TEXTURE2DARRAY_H_ */
