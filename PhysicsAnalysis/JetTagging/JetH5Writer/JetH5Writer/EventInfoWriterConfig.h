/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
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
