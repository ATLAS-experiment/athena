/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EEMSORTSELECTCOUNTCONTAINERALGTOOL_H
#define GLOBALSIM_EEMSORTSELECTCOUNTCONTAINERALGTOOL_H

/**
 * AlgTool that reads in all eEM TOBS for an event, and
 * passes them collectively to the eEmSortSelectCount Algorithm
 */

#include "GepAlgoHypothesisPortsIn.h"
#include "eEmSortSelectCountContainerPortsOut.h"
#include "AlgoDataTypes.h"

#include "../../../IGlobalSimAlgTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

namespace GlobalSim {
  class eEmSortSelectCountContainerAlgTool: public extends<AthAlgTool,
							   IGlobalSimAlgTool> {
    
  public:
    using GenTobPtr = typename eEmSortSelectCountContainerPortsOut::GenTobPtr;

    
    eEmSortSelectCountContainerAlgTool(const std::string& type,
				       const std::string& name,
				       const IInterface* parent);
    
    virtual ~eEmSortSelectCountContainerAlgTool() = default;
    
    StatusCode initialize() override;

    virtual StatusCode run(const EventContext& ctx) const override;
    
    virtual std::string toString() const override;
    
  private:
 
    Gaudi::Property<bool>
    m_enableDump{this,
	"enableDump",
	  {false},
	"flag to enable dumps"};


    SG::ReadHandleKey<GlobalSim::GepAlgoHypothesisFIFO>
    m_HypoFIFOReadKey {
      this,
      "HypoFIFOReadKey",
      "hypoFIFO",
      "key to read input port data for the hypo block"};

    

    SG::WriteHandleKey<GlobalSim::eEmSortSelectCountContainerPortsOut>
    m_portsOutWriteKey {
      this,
      "PortsOutKey",
      "eEmSortSelectCount",
      "key to write output ports data"};


    // FIXME the following should be properties

    // Cut values for Select part of Algorithm
    std::size_t m_nSelect{3};
    std::vector<int> m_EtMin{0, 1, 2};
    std::vector<int> m_REtaMin{0, 0, 0};
    std::vector<int> m_RHadMin{0, 0, 0};
    std::vector<int> m_WsTotMin{0, 0, 0};

    // Cut values for Count part of Algorithm
    std::size_t m_nEtaRegions{3};
    std::vector<int> m_etaReg_EtaMin{0, 10, 20};
    std::vector<int> m_etaReg_EtaMax{100, 100, 100};
    std::vector<unsigned int> m_etaReg_EtMin{0, 1, 2};

    StatusCode
    make_selectedTobs(const std::vector<eEmTobPtr>&,
		      std::vector<std::vector<eEmTobPtr>>&) const;

    std::vector<std::size_t>
    count_tobs(const std::vector<std::vector<GenTobPtr>>&) const;
	  

  };
}
    
#endif
