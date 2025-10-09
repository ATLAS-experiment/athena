/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EEMSORTSELECTCOUNTCONTAINERCOMPARATOR_H
#define GLOBALSIM_EEMSORTSELECTCOUNTCONTAINERCOMPARATOR_H

/**
 * AlgTool that reads in the potout ports for
 * eEmSortSelectCountContainerAlgTool,  a test FIFO used as its
 * input and (5/3/25 not yet availble ...) a set of expected values for 
 * the output port. The output port valeus and the expected values
 * are compared.
 */

#include "GepAlgoHypothesisPortsIn.h"
#include "eEmSortSelectCountContainerPortsOut.h"
#include "eEmSortSelectCountExpectations.h"

#include "../../../IGlobalSimAlgTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

namespace GlobalSim {
  class eEmSortSelectCountContainerComparator: public extends<AthAlgTool,
							   IGlobalSimAlgTool> {
    
  public:
    using GenTobPtr = typename eEmSortSelectCountContainerPortsOut::GenTobPtr;

    
    eEmSortSelectCountContainerComparator(const std::string& type,
				       const std::string& name,
				       const IInterface* parent);
    
    virtual ~eEmSortSelectCountContainerComparator() = default;
    
    virtual StatusCode initialize() override;

    virtual StatusCode run(const EventContext& ctx) const override;
    
    virtual std::string toString() const override;

    virtual StatusCode updateTIP(std::bitset<s_nbits_TIP>&,
				 const EventContext&) const override;
    
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

    

    SG::ReadHandleKey<GlobalSim::eEmSortSelectCountContainerPortsOut>
    m_portsOutReadKey {
      this,
      "PortsOutKey",
      "eEmSortSelectCount",
      "key to read in  output ports data"};

     

    SG::ReadHandleKey<GlobalSim::eEmSortSelectCountExpectations>
    m_eEmSortSelectCountExpectationsReadKey {
      this,
      "eEmSortSelectCountExpectationsReadKey",
      "eEmSortSelectCountExpectations",
      "key to read in eEmSortSelectCount regression test expectations"};

    constexpr static auto& s_SortOutWidth =
      eEmSortSelectCountContainerPortsOut::SortOutWidth;
  };
}
    
#endif
