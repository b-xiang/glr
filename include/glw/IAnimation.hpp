#ifndef IANIMATION_H_
#define IANIMATION_H_

#include <string>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "IAnimation.hpp"
#include "IMesh.hpp"

#include "AnimatedBoneNode.hpp"

#include "common/utilities/Macros.hpp"

namespace glr
{
namespace glw
{

struct BoneNode
{
	std::string name;
	glm::mat4 transformation;
	std::vector< BoneNode > children;
};

class IAnimation
{
public:
	virtual ~IAnimation()
	{
	}
	;

	virtual const std::string& getName() const = 0;

	virtual void bind() = 0;

	// TODO: Should we have this in the interface?
	virtual GLuint getBufferId() const = 0;

	// TODO: make rootBoneNode const?
	// TODO: Should we really be sending in BoneData&?  Shouldn't we send IMesh* or something?
	// TODO: Should we have this in the interface?
	/**
	 * Will generate the transformation matrices to be used to animate the model with the given bone data.
	 * 
	 * @param globalInverseTransformation - Not sure what this is...
	 * @param rootBoneNode - Root node of the skeleton used for the current animation.
	 * @param boneData - Data about the bones we are generating the animation on.  The method will use some of the boneData variables as a cache.
	 * (TODO: Should I be using this as a cache?  We currently use finalTransformation...)
	 * @param indexCache - If passed in, represents a cache of previous rotation, position, and scaling node search indices.  The algorithm to find
	 * the correct indices for the animation will use the indexCache index values to determine where to begin the search for the next
	 * rotation, position, and scaling index.
	 */
	virtual void generateBoneTransforms(glm::mat4& globalInverseTransformation, BoneNode& rootBoneNode, BoneData& boneData) = 0;
	virtual void generateBoneTransforms(glm::mat4& globalInverseTransformation, BoneNode& rootBoneNode, BoneData& boneData, std::vector<glm::detail::uint32>& indexCache) = 0;

	/**
	 * Will set the animation time to runningTime.
	 */
	virtual void setAnimationTime(glm::detail::float32 runningTime) = 0;

	/**
	 * Set the animation to run only between the given start and end frame.
	 * 
	 * Note: A value of 0 for the start and end frame will run through all frames of the animation.
	 * 
	 * endFrame must be larger than or equal to startFrame.  
	 * 
	 * A value below zero for the start or end frame is invalid.
	 */
	virtual void setFrameClampping(glm::detail::uint32 startFrame, glm::detail::uint32 endFrame) = 0;
};

}
}

#endif /* IANIMATION_H_ */
