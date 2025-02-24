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
#include "AlgoConstants.h"

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

    // Select cuts: values set in init()
    // outer vector: NumSelect entries. Inner vector: N_eta entries
    std::vector<int> m_EtMin;
    std::vector<int> m_REtaMin;
    std::vector<int> m_RHadMin;
    std::vector<int> m_WsTotMin;

    // Count cuts. Each count has three eta regions
    // The outer vector is of length 3: one entry per eta region
    std::vector<std::vector<unsigned int>> m_count_EtMin;
    std::vector<std::vector<int>> m_count_EtaMin;
    std::vector<std::vector<int>> m_count_EtaMax;

    StatusCode
    make_selectedTobs(const std::vector<eEmTobPtr>&,
		      std::vector<std::vector<eEmTobPtr>>&) const;

    std::vector<std::size_t>
    count_tobs(const std::vector<std::vector<GenTobPtr>>&) const;

    constexpr static std::size_t s_NumCnt{24};
    constexpr static std::size_t s_NumEtaRanges{3};


    // Each count category uses one of many selected GenTob vevtors.
    // s_CntSelN chooses which of these to use.
    constexpr static std::array<std::size_t, s_NumCnt> s_CntSelN{
      0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  };
}
    
#endif
