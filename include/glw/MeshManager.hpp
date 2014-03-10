#ifndef MESHMANAGER_H_
#define MESHMANAGER_H_

#include <memory>
#include <string>
#include <map>

#include "IMeshManager.hpp"

#include "IOpenGlDevice.hpp"

#include "Mesh.hpp"

namespace glr
{
namespace glw
{
	
class MeshManager : public IMeshManager
{
public:
	MeshManager(IOpenGlDevice* openGlDevice);
	virtual ~MeshManager();

	virtual Mesh* getMesh(const std::string& name) const;
	virtual Mesh* addMesh(const std::string& name);
	virtual Mesh* addMesh(
		const std::string& name, 
		std::vector< glm::vec3 > vertices, 
		std::vector< glm::vec3 > normals,
		std::vector< glm::vec2 > textureCoordinates,
		std::vector< glm::vec4 > colors,
		std::vector< VertexBoneData > bones,
		BoneData boneData
	);
	virtual Mesh* addMesh(
		const std::string& name, 
		std::vector< glm::vec3 > vertices, 
		std::vector< glm::vec3 > normals,
		std::vector< glm::vec2 > textureCoordinates,
		std::vector< glm::vec4 > colors
	);
	
private:	
	IOpenGlDevice* openGlDevice_;

	std::map< std::string, std::unique_ptr<Mesh> > meshes_;
};

}
}

#endif /* MESHMANAGER_H_ */
