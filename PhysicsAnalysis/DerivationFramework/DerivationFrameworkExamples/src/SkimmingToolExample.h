/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SkimmingToolExample.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_SKIMMINGTOOLEXAMPLE_H
#define DERIVATIONFRAMEWORK_SKIMMINGTOOLEXAMPLE_H 1

#include<string>

// Gaudi & Athena basics
#include "AthenaBaseComps/AthAlgTool.h"

// DerivationFramework includes
#include "DerivationFrameworkInterfaces/ISkimmingTool.h"
#include "xAODMuon/MuonContainer.h"

namespace DerivationFramework {

  /** @class SkimmingToolExample
  
      the code used in this implementation is kindly stolen from:
      atlasoff:: ISF/ISF_Core/ISF_Tools

      @author James Catmore -at- cern.ch
     */
  class SkimmingToolExample : public extends<AthAlgTool, ISkimmingTool> {
    
  public: 
    /** Use constructor from base class */
    using base_class::base_class;

    /** Athena algtool's Hooks */
    virtual StatusCode finalize() override;
    
    /** Check that the current event passes this filter */
    virtual bool eventPassesFilter() const override;
    
  private:
    Gaudi::Property<std::string> m_muonSGKey
      {this, "MuonContainerKey", "Muons", "Key for muon container"};

    Gaudi::Property<unsigned int> m_nMuons
      {this, "NumberOfMuons", 2, "Minimum number of muons"};

    Gaudi::Property<double> m_muonPtCut
      {this, "MuonPtCut", 10000.0, "p_T cut on muon in MeV"};

    mutable std::atomic<unsigned int> m_ntot{0};
    mutable std::atomic<unsigned int> m_npass{0};
  }; 
  
}


#endif
