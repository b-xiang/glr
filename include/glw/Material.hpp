#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <GL/glew.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "IOpenGlDevice.hpp"
#include "shaders/IShaderProgram.hpp"

#include "IMaterial.hpp"

#include "common/utilities/Macros.hpp"

namespace glr
{
namespace glw
{
	
class IOpenGlDevice;
	
struct MaterialData
{
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;	
	glm::vec4 emission;
	//glm::detail::float32 shininess;
	//glm::detail::float32 strength;
};
	
class Material : public IMaterial
{
public:
	Material(IOpenGlDevice* openGlDevice, std::string name);
	Material(
		IOpenGlDevice* openGlDevice,
		std::string name,
		glm::vec4 ambient,
		glm::vec4 diffuse,
		glm::vec4 specular,
		glm::vec4 emission,
		glm::detail::float32 shininess,
		glm::detail::float32 strength
	);
	virtual ~Material();

	virtual void bind();
	virtual GLuint getBufferId() const;
	GLuint getBindPoint() const;

	virtual void pushToVideoMemory();
	virtual void pullFromVideoMemory();
	virtual void freeLocalData();
	virtual void freeVideoMemory();
	virtual void allocateVideoMemory();

	void setAmbient(const glm::vec4& ambient);
	void setDiffuse(const glm::vec4& diffuse);
	void setSpecular(const glm::vec4& specular);
	void setEmission(const glm::vec4& emission);
	void setShininess(glm::detail::float32 shininess);
	void setStrength(glm::detail::float32 strength);

	GETSET(std::string, name_, Name)
private:
	std::string name_;
	
	IOpenGlDevice* openGlDevice_;
	GLuint bufferId_;
	GLuint bindPoint_;

	GLenum fill_mode_;
	glm::detail::int32 ret1_, ret2_;
	glm::vec4 ambient_;
	glm::vec4 diffuse_;
	glm::vec4 specular_;
	glm::vec4 emission_;
	glm::detail::float32 shininess_, strength_;
	
	MaterialData materialData_;
	
	glm::detail::int32 two_sided_;
	glm::detail::int32 two_sided_true_;
	glm::detail::int32 wireframe_;
	glm::detail::uint32 max_;
};

}
}

#endif /* MATERIAL_H_ */
