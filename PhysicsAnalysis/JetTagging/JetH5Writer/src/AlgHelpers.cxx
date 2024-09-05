#include "AlgHelpers.h"

#include <stdexcept>

#define CHECK_TYPE(string, target)                      \
  if (string == #target) return Primitive::Type::target
Primitive::Type getPrimitiveType(const std::string& name) {
  CHECK_TYPE(name, PRECISION_CUSTOM);
  CHECK_TYPE(name, CUSTOM);
  CHECK_TYPE(name, UCHAR);
  CHECK_TYPE(name, CHAR);
  CHECK_TYPE(name, USHORT);
  CHECK_TYPE(name, SHORT);
  CHECK_TYPE(name, UINT);
  CHECK_TYPE(name, INT);
  CHECK_TYPE(name, ULONG);
  CHECK_TYPE(name, LONG);
  CHECK_TYPE(name, ULL);
  CHECK_TYPE(name, LL);
  CHECK_TYPE(name, HALF);
  CHECK_TYPE(name, FLOAT);
  CHECK_TYPE(name, DOUBLE);
  CHECK_TYPE(name, UINT2UCHAR);
  CHECK_TYPE(name, INT2CHAR);
  CHECK_TYPE(name, UINT2USHORT);
  CHECK_TYPE(name, INT2SHORT);
  CHECK_TYPE(name, UL2ULL);
  throw std::domain_error("unknown type " + name);
}
#undef CHECK_TYPE

#define CHECK_FORMAT(string, target)                                    \
if (string == #target) return IParticleWriterConfig::ArrayFormat::target
IParticleWriterConfig::ArrayFormat getArrayFormat(const std::string& name) {
  CHECK_FORMAT(name, AWKWARD);
  CHECK_FORMAT(name, FLAT);
  CHECK_FORMAT(name, PADDED);
  throw std::domain_error("unknown format " + name);
}


