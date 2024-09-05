#ifndef ALG_HELPERS_H
#define ALG_HELPERS_H

#include "JetH5Writer/Primitive.h"
#include "JetH5Writer/IParticleWriterConfig.h"

#include <string>

Primitive::Type getPrimitiveType(const std::string& name);
IParticleWriterConfig::ArrayFormat getArrayFormat(const std::string& name);

#endif
