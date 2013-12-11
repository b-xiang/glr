/*
 * TextureManager.h
 *
 *  Created on: 2011-05-08
 *      Author: jarrett
 */

#ifndef TEXTUREMANAGER_H_
#define TEXTUREMANAGER_H_

#include <memory>
#include <map>
#include <string>

#include "IOpenGlDevice.h"

#include "ITextureManager.h"

namespace glr {
namespace glw {

class TextureManager : public ITextureManager {
public:
	TextureManager(IOpenGlDevice* openGlDevice);
	virtual ~TextureManager();

	virtual Texture2D* getTexture2D(const std::string& name);
	virtual Texture2DArray* getTexture2DArray(const std::string& name);
	
	virtual Texture2D* addTexture2D(const std::string& name, const std::string& filename);
	virtual Texture2D* addTexture2D(const std::string& name, utilities::Image* image);
	virtual Texture2DArray* addTexture2DArray(const std::string& name, const std::vector<std::string> filenames);
	virtual Texture2DArray* addTexture2DArray(const std::string& name, const std::vector<utilities::Image*> images);
	
private:
	IOpenGlDevice* openGlDevice_;

	std::map< std::string, std::unique_ptr<Texture2D> > textures_;
};

}
}
#endif /* TEXTUREMANAGER_H_ */
