/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_INVARIANTMASSDELTAPHIINCLUSIVE2_H
#define GLOBALSIM_INVARIANTMASSDELTAPHIINCLUSIVE2_H

/**
 * Algtool to run the Global InvariantMassDeltaPhiInclusive2
 */

#include "GepAlgoHypothesisPortsIn.h"
#include "InvariantMassDeltaPhiInclusive2ContainerPortsOut.h"

#include "../../../IGlobalSimAlgTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

namespace GlobalSim {
  class InvariantMassDeltaPhiInclusive2AlgTool: public extends<AthAlgTool,
							   IGlobalSimAlgTool> {
    
  public:
    using PortsOut = InvariantMassDeltaPhiInclusive2ContainerPortsOut;
    using GenTobPtr = PortsOut::GenTobPtr;
    
    InvariantMassDeltaPhiInclusive2AlgTool(const std::string& type,
					   const std::string& name,
					   const IInterface* parent);
    
    virtual ~InvariantMassDeltaPhiInclusive2AlgTool() = default;
    
    virtual StatusCode initialize() override;

    virtual StatusCode run(const EventContext& ctx) const override;
    
    virtual std::string toString() const override;
    
  private:
 
    Gaudi::Property<bool>
    m_enableDump{this,
	"enableDump",
	  {false},
	"flag to enable dumps"};


    SG::ReadHandleKey<GepAlgoHypothesisFIFO>
    m_HypoFIFOReadKey {
      this,
      "HypoFIFOReadKey",
      "hypoFIFO",
      "key to read input port data for the hypo block"};

    

    SG::WriteHandleKey<InvariantMassDeltaPhiInclusive2ContainerPortsOut>
    m_portsOutWriteKey {
      this,
      "PortsOutKey",
      "eEmSortSelectCount",
      "key to write output ports data"};


  };
}
    
#endif
