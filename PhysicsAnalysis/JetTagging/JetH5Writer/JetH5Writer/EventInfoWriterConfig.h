#ifndef EVENT_INFO_WRITER_CONFIG
#define EVENT_INFO_WRITER_CONFIG

#include "Primitive.h"

#include <string>
#include <vector>

struct EventInfoWriterConfig
{
  std::string name;
  std::vector<Primitive> inputs;
};

#endif
