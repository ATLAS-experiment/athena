/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_HYPOTESTBENCHALG_H
#define GLOBALSIM_HYPOTESTBENCHALG_H

/*
 * Create and write out a FIFO (vector) of eEMTObs to the event store/
 * This simulates the action of the APP FIFOs, which feed TOBs
 *
 */
 
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "GepAlgoHypothesisPortsIn.h"

namespace GlobalSim {
  
  class HypoTestBenchAlg : public AthReentrantAlgorithm {
  public:
     
    HypoTestBenchAlg(const std::string& name, ISvcLocator *pSvcLocator);
    
    virtual StatusCode initialize () override;
    virtual StatusCode execute (const EventContext& ctx) const override;

  private:

    SG::WriteHandleKey<GepAlgoHypothesisFIFO>
    m_hypothesisFIFO_WriteKey {
      this,
	"hypothesisFIFOWriteKey",
	"hypoFIFO",
	"key to write out Fifo containing ports data"};

  };
}
#endif
