/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Zhengcheng Tao


#ifndef ASG_IOSTATSALG_H
#define ASG_IOSTATSALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>

namespace CP {

  class IOStatsAlg final : public EL::AnaAlgorithm {

  public:
    using EL::AnaAlgorithm::AnaAlgorithm;

    // finalize function: run once at the end of a job after event loop
    virtual StatusCode finalize() override;

  private:
    Gaudi::Property<std::string> m_printOption {this, "printOption", "Summary", "option to pass the standard ROOT printing function. Can be 'Summary', 'ByEntries' or 'ByBytes'."};

  };

} // namespace

#endif
