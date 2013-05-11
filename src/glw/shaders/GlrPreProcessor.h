/*
 * GlrPreProcessor.h
 *
 * Class that will extend the functionality of the CPreProcessor class for anything specific to processing
 * 'glr' shader source data.
 * 
 * Author: Jarrett Chisholm <j.chisholm@chisholmsoft.com>
 * Date: 2013
 *
 *
 */


#ifndef GLRPREPROCESSOR_H
#define GLRPREPROCESSOR_H

#include "CPreProcessor.h"

namespace glr {
namespace shaders {
class GlrPreProcessor : public CPreProcessor {
public:
	GlrPreProcessor(std::string source);

private:
};
}
}
#endif /* GLRPREPROCESSOR_H */