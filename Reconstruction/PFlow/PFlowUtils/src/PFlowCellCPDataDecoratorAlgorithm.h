/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFLOWCELLCPDATADECORATORALGORITHM_H
#define PFLOWCELLCPDATADECORATORALGORITHM_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

//EDM Container Classes
#include "xAODPFlow/FlowElementContainer.h"

//Core classes for some private function classdefs
#include "StoreGate/WriteDecorHandle.h"

/** This algorithm decorates xAOD::FlowElement with lists of calorimeter cells that were removed from the clusters
 *  during the particle flow reconstruction.
**/

class PFlowCellCPDataDecoratorAlgorithm : public AthReentrantAlgorithm {

public:
  /** Constructor from base class */
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  /** Destructor */
  virtual ~PFlowCellCPDataDecoratorAlgorithm() {};
  
  /* Gaudi algorithm hooks */
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;

private:
    
    /** Write handle key to decorate PFO with threeN leading truth particle barcode and energy */
    SG::WriteDecorHandleKey<xAOD::FlowElementContainer> m_cellListWriteDecorHandleKey{this,"PFOWriteDecorHandleKey_CellCPData","JetETMissChargedParticleFlowObjects.cellCPData",
    "Decorate PFO with list of cells removed from the cluster during PFlow reconstruction"};

    /** Read handle key to read in the neutral particle flow objects */
    SG::ReadHandleKey<xAOD::FlowElementContainer> m_neutralPFOReadHandleKey{this,"NeutralPFOReadHandleKey","JetETMissNeutralParticleFlowObjects","Read handle key for neutral particle flow objects"};
    
};

#endif


