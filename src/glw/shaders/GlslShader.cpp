#include <sstream>
#include <utility>

#include <GL/glew.h>

#include "glw/shaders/GlslShader.hpp"

#include "common/logger/Logger.hpp"

#include "exceptions/GlException.hpp"

namespace glr
{
namespace shaders
{

GlslShader::GlslShader(std::string source, Type type) : source_(std::move(source)), type_(type)
{
	shaderId_ = -1;

	//initialize();
}

GlslShader::GlslShader(std::string name, std::string source, Type type, std::vector< std::pair<std::string, std::string> > bindings, std::vector< std::pair<glmd::int32, std::string> > locationBindings)
	: name_(std::move(name)), source_(std::move(source)), type_(type)
{
	shaderId_ = -1;

	// Convert bindings
	for ( auto& it : bindings )
	{
		IShader::Binding binding = IShader::Binding();
		binding.type = IShader::parseBindType(it.first);
		binding.variableName = it.second;
		binding.bindPoint = -1;
		
		bindings_.push_back(binding);
	}
	
	GLint maxNumLocations = 0;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxNumLocations);
	
	for ( auto& it : locationBindings )
	{
		// Validate the int32
		// TODO: Do I need to check and make sure the value is greater than the reserved vertex attribute locations?  Or is it
		// a good idea to let the user override these reserved locations...?
		if (it.first < 0 || it.first >= maxNumLocations)
		{
			std::stringstream ss;
			ss << "Location defined by '@location' annotation is outside of the available range of locations.";
			ss << "\n";
			ss << "Value defined: " << it.first;
			ss << "\n";
			ss << "Acceptable range: 0 to " << maxNumLocations;
			LOG_ERROR( ss.str() );
			throw exception::GlException( ss.str() );
		}
		
		IShader::Binding binding = IShader::Binding();
		binding.type = IShader::BindType::BIND_TYPE_LOCATION;
		binding.variableName = it.second;
		binding.bindPoint = it.first;
		
		bindings_.push_back(binding);
	}
}

GlslShader::~GlslShader()
{
	glDeleteShader(shaderId_);
}

void GlslShader::compile()
{
	LOG_DEBUG( "Compiling shader '" + name_ + "'." );

	if ( shaderId_ < 0 )
	{
		std::string msg("Could not load shader '" + name_ + "' - shader already has an OpenGL id assigned to it.");
		LOG_ERROR( msg );
		throw exception::GlException(msg);
	}


	GLenum shaderType;

	switch ( type_ )
	{
		case TYPE_NONE:
		{
			std::string msg("Could not load shader '" + name_ + "' - shader type is not known.");
			LOG_ERROR( msg );
			throw exception::GlException(msg);
		}
	
		case TYPE_VERTEX:
			shaderType = GL_VERTEX_SHADER;
			break;
	
		case TYPE_FRAGMENT:
			shaderType = GL_FRAGMENT_SHADER;
			break;
	
		case TYPE_GEOMETRY:
			shaderType = GL_GEOMETRY_SHADER;
			break;
	
		case TYPE_TESSELLATION:
		{
			std::string msg("Could not load shader '" + name_ + "' - tessellation shaders are not yet implemented.");
			LOG_ERROR( msg );
			throw exception::GlException(msg);
		}
		
		default:
		{
			std::string msg("Could not load shader '" + name_ + "' - shader type is an invalid value.");
			LOG_ERROR( msg );
			throw exception::GlException(msg);
		}
	}

	std::stringstream msg;
	msg << "Shader is of type '" << type_ << "'.";
	LOG_DEBUG( msg.str() );

	shaderId_ = glCreateShader(shaderType);

	const char* source = source_.c_str();
	glShaderSourceARB(shaderId_, 1, &source, nullptr);

	glCompileShader(shaderId_);

	GLint compiled = GL_FALSE;
	glGetShaderiv(shaderId_, GL_COMPILE_STATUS, &compiled);

	// Handle any errors
	if ( compiled == GL_FALSE )
	{
		std::stringstream errorMsg;
		errorMsg << "Could not initialize shader '" << name_ << "'.";

		GLint infoLogLength;
		glGetShaderiv(shaderId_, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shaderId_, infoLogLength, nullptr, strInfoLog);

		errorMsg << "\ncompiler_log: " << strInfoLog;

		delete[] strInfoLog;

		// Cleanup
		glDeleteShader(shaderId_);

		LOG_ERROR( errorMsg.str() );
		throw exception::GlException(errorMsg.str());
	}


	LOG_DEBUG( "Done initializing shader '" + name_ + "'." );
}

IShader::Type GlslShader::getType() const
{
	return type_;
}

const std::string& GlslShader::getName() const
{
	return name_;
}

IShader::BindingsMap GlslShader::getBindings()
{
	return bindings_;
}

GLuint GlslShader::getGlShaderId() const
{
	return shaderId_;
}

void GlslShader::bind()
{
}

}
}
