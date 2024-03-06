/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

*/

#ifndef TrigHLTJetHypo_TrigJetCRVARHypoAlg_H
#define TrigHLTJetHypo_TrigJetCRVARHypoAlg_H

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODJet/JetContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "CaloEvent/CaloConstCellContainer.h"

#include "DecisionHandling/HypoBase.h"

#include "TrigJetCRVARHypoTool.h"

/**
 * @class TrigJetCRVARHypoAlg
 * @brief HypoAlg for low-EMF trackless jets algorithm
 * @details HypoAlg needed to associate trigger navigation to jets selected for writing out by exotic jets algorithms
 * @author
 **/

class TrigJetCRVARHypoAlg : public ::HypoBase {
 public: 

  TrigJetCRVARHypoAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual StatusCode  initialize() override;
  virtual StatusCode  execute( const EventContext& context ) const override;

 private:
  ToolHandleArray< TrigJetCRVARHypoTool > m_hypoTools {this, 
      "HypoTools", 
	{}, 
      "Tools to perfrom selection"};

  SG::ReadHandleKey<CaloConstCellContainer> m_cellKey {this,
      "Cells","Key for input CaloCellContainer"};

}; 

#endif
