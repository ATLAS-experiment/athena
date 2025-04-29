/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EVENT_LOOP_ALGS_PRINT_TOOL_CONFIG_H
#define EVENT_LOOP_ALGS_PRINT_TOOL_CONFIG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>

namespace CP
{
  class PrintToolConfigAlg : public EL::AnaAlgorithm
  {

    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode finalize () override;

  private:
    Gaudi::Property<std::string> m_outputFileName {this, "OutputFile", "tool_config.txt", "name of the file to save the tool configuration to"};

  };
}

#endif
