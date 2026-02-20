//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#include "Tracer.h"
#include <iostream>


namespace GlobalSim {
  Tracer::Tracer(const std::string& msg): m_msg{msg}{
    std::cerr << "Tracer start " << m_msg << '\n';
  }

  Tracer::~Tracer(){
    std::cerr << "Tracer end " << m_msg << '\n';
  }
}
  
  
