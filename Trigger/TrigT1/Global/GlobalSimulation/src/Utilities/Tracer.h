//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBSIM_TRACER_H
#define GLOBSIM_TRACER_H

#include <string>

/*
 * Debug tool - issues messages at points of construction and destruction
 */

namespace GlobalSim {
  struct Tracer{
    Tracer(const std::string& msg);
    ~Tracer();
    std::string m_msg;
  };
}

#endif
