
/**
 * This file is automatically generated.  Changes made to this file will
 * not be reflected after compiling.
 *
 * If you wish to make changes to shader information for Glr, edit the shaders
 * and shader programs in the 'data/shaders/' directory.
 *
 */
 
#include <map> 
 
namespace glr {

namespace shaders {

static std::map<std::string, std::string> SHADER_DATA = {

{"glr_basic.program", std::string(
	R"<STRING>(
#name glr_basic
#type program

#include "shader.vert"
#include "shader.frag"

)<STRING>"
)}	
, 
{"shader.vert", std::string(
	R"<STRING>(
#version 150 core

#ifndef NUM_LIGHTS
#define NUM_LIGHTS 1
#endif

#type vertex

#include <glr>
#include <light>
#include <material>

in vec3 in_Position;
in vec2 in_Texture;
in vec3 in_Normal;

out vec2 textureCoord;
out vec3 normalDirection;
out vec3 lightDirection;

//@bind texture0
//uniform sampler2DArray texture;

@bind Light
layout(std140) uniform LightSources 
{
	LightSource lightSources[ NUM_LIGHTS ];
};

//@bind Material
Material material = Material(
	vec4(1.0, 0.8, 0.8, 1.0),
	vec4(1.0, 0.8, 0.8, 1.0),
	vec4(1.0, 0.8, 0.8, 1.0),
	0.995
);


void main() {
	gl_Position = pvmMatrix * vec4(in_Position, 1.0);
	
	textureCoord = in_Texture;
	
	normalDirection = normalize(normalMatrix * in_Normal);
	lightDirection = normalize(vec3(lightSources[0].direction));
	
	vec3 diffuseReflection = vec3(lightSources[0].diffuse) * vec3(material.diffuse) * max(0.0, dot(normalDirection, lightDirection));
	
	/*
	float bug = 0.0;	
	bvec3 result = equal( diffuseReflection, vec3(0.0, 0.0, 0.0) );
	if(result[0] && result[1] && result[2]) bug = 1.0;
	diffuseReflection.x += bug;
	*/
	
	/*
	float bug2 = 0.0;
	bool result2 = dot(normalDirection, lightDirection) > 0.0;
	if(result2) bug2 = 1.0;
	diffuseReflection.x += bug2;
	*/
}

)<STRING>"
)}	
, 
{"material", std::string(
	R"<STRING>(
#type na
#name material

struct Material {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

)<STRING>"
)}	
, 
{"light", std::string(
	R"<STRING>(
#type na
#name light

struct LightSource {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 position;
	vec4 direction;
};

)<STRING>"
)}	
, 
{"shader.frag", std::string(
	R"<STRING>(
#version 150 core

#extension GL_EXT_texture_array : enable

#type fragment

#include <material>

@bind texture0
uniform sampler2DArray tex;

in vec2 textureCoord;
in vec3 normalDirection;
in vec3 lightDirection;

Material material = Material(
	vec4(0.4, 0.4, 0.4, 1.0),
	vec4(0.6, 0.6, 0.6, 1.0),
	vec4(1.0, 0.8, 0.8, 1.0),
	0.995
);

void main() {
	vec3 ct, cf;
	vec4 texel;
	float intensity, at, af;
	intensity = max( dot(lightDirection, normalize(normalDirection)), 0.0 );
 
	cf = intensity * (material.diffuse).rgb + material.ambient.rgb;
	af = material.diffuse.a;
	texel = texture2DArray(tex, vec3(textureCoord, 1));
 
	ct = texel.rgb;
	at = texel.a;
	
	gl_FragColor = vec4(ct * cf, at * af);
}

)<STRING>"
)}	
, 
{"glr", std::string(
	R"<STRING>(
#type na
#name glr

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat4 pvmMatrix;
uniform mat3 normalMatrix;

)<STRING>"
)}	

};

}

}
