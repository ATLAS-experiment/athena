///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// TauJetCnvAlg.h 
// Header file for class TauJetCnvAlg
// Author: Michel Janus , janus@cern.ch
/////////////////////////////////////////////////////////////////// 
#ifndef XAODTAUCNV_TAUJETCNVALG_H
#define XAODTAUCNV_TAUJETCNVALG_H 

// STL includes
#include <string>

// FrameWork includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "AsgTools/PropertyWrapper.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTauCnv/ITauJetCnvTool.h"


namespace xAODMaker {
  class TauJetCnvAlg
    : public ::AthAlgorithm
  { 
    
    /////////////////////////////////////////////////////////////////// 
    // Public methods: 
    /////////////////////////////////////////////////////////////////// 
  public: 

    // Copy constructor: 

    /// Constructor with parameters: 
    TauJetCnvAlg( const std::string& name, ISvcLocator* pSvcLocator );

    /// Destructor: 
    virtual ~TauJetCnvAlg(); 

    // Assignment operator: 
    //TauJetCnvAlg &operator=(const TauJetCnvAlg &alg); 

    // Athena algorithm's Hooks
    virtual StatusCode  initialize();
    virtual StatusCode  execute();
    virtual StatusCode  finalize();

    /////////////////////////////////////////////////////////////////// 
    // Private data: 
    /////////////////////////////////////////////////////////////////// 
  private: 

    /// Default constructor: 
    TauJetCnvAlg();

    /// Containers
    Gaudi::Property<std::string> m_inputTauJetContainerName{this, "InputTauJetContainer", "TauRecContainer"};
    Gaudi::Property<std::string> m_xaodTauJetContainerName{this, "xAODTauJetContainer", "TauRecContainer"};

    /** @brief Tool to perform taujet container conversion*/
    ToolHandle<ITauJetCnvTool> m_cnvTool{this, "CnvTool", "", "The converter tool for TauJets"};
    
  }; 

}
#endif //> !XAODTAUCNV_TAUJETCNVALG_H
