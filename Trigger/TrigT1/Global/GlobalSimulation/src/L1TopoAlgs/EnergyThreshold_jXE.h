/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GLOBALSIM_ENERGYTHRESHOLD_JXE_H
#define GLOBALSIM_ENERGYTHRESHOLD_JXE_H

#include "GaudiKernel/StatusCode.h"

#include <vector>
#include <string>
#include <map>

namespace GlobalSim {

  class Count;

  class jXETOBArray;
  
  class EnergyThreshold_jXE {
    
  public:

    EnergyThreshold_jXE(const std::string& name,
			unsigned int nbits,
			unsigned int threshold);
    
    virtual ~EnergyThreshold_jXE() = default;
  
    // not  const due to variables to be monitored
    StatusCode run(const jXETOBArray& input, 
		   Count& count);

    // expose data to be monitored
    const std::vector<double>& input_ET() const;
    const std::vector<double>& passing_ET() const;
    const std::vector<double>& counts() const;

    std::string toString() const;
  private:
    std::string m_name{};

    unsigned int  m_nbits{0u};
    unsigned int m_100MeVthreshold{0u};

    // data to monitor
    std::vector<double> m_inputET{};
    std::vector<double> m_passingET{};
    std::vector<double> m_counts{};
  };
}

#endif
