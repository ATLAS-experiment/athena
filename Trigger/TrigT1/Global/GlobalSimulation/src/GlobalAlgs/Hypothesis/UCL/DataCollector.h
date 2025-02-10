/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_DATACOLLECTOR_H
#define GLOBALSIM_DATACOLLECTOR_H

#include "AlgoDataTypes.h"
#include "eEmSortSelectCountContainerPortsOut.h"
#include <ostream>
#include <map>
#include <vector>

namespace GlobalSim {
  class DataCollector;
}

std::ostream& operator << (std::ostream& os,
			   const GlobalSim::DataCollector& col);
namespace GlobalSim {
  class DataCollector{
  public:
    using GenTobPtr = typename eEmSortSelectCountContainerPortsOut::GenTobPtr;


    friend std::ostream& ::operator << (std::ostream& os,
					const GlobalSim::DataCollector& col);
    
    void collect(const std::string& label, const std::vector<int>& val) {
      m_ints[label] = val;
    }

    void collect(const std::string& label,
		 const std::vector<std::size_t>& val) {
      m_sz_ts[label] = val;
    }
 
    void collect(const std::string& label,  const std::vector<eEmTobPtr>& val) {
      m_eEmTobContainers[label] = val;
    }

  
     void collect(const std::string& label,
		  const std::vector<std::vector<eEmTobPtr>>& val) {
      m_vec_eEmTobContainers[label] = val;
    }

    void collect(const std::string& label,
		 const std::vector<std::vector<GenTobPtr>>& val) {
      m_vec_GenericTobContainers[label] = val;
    }
     
  private:
    std::map<std::string, std::vector<int>> m_ints;
    std::map<std::string, std::vector<std::size_t>> m_sz_ts;
    std::map<std::string, std::vector<eEmTobPtr>> m_eEmTobContainers;
    std::map<std::string,
	     std::vector<std::vector<eEmTobPtr>>> m_vec_eEmTobContainers;


    std::map<std::string, std::vector<std::vector<GenTobPtr>>>
    m_vec_GenericTobContainers;


  };
}

std::ostream& operator << (std::ostream& os,
			   const GlobalSim::DataCollector& col);

#endif
