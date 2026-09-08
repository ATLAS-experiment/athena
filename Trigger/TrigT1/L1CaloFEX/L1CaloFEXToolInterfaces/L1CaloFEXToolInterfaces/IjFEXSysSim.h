/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           jFEXSysSim.h  -
//                              -------------------
//     begin                : 23 03 2019
//     email                :  jacob.julian.kempster@cern.ch
//  ***************************************************************************/


#ifndef IjFEXSysSim_H
#define IjFEXSysSim_H

#include "GaudiKernel/IAlgTool.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloEvent/CaloCellContainer.h"
#include "xAODTrigger/jFexSRJetRoIContainer.h"
#include "xAODTrigger/jFexSRJetRoIAuxContainer.h"
#include "xAODTrigger/jFexLRJetRoIContainer.h"
#include "xAODTrigger/jFexLRJetRoIAuxContainer.h"
#include "xAODTrigger/jFexTauRoIContainer.h"
#include "xAODTrigger/jFexTauRoIAuxContainer.h"
#include "xAODTrigger/jFexFwdElRoIContainer.h"
#include "xAODTrigger/jFexFwdElRoIAuxContainer.h"
#include "xAODTrigger/jFexMETRoIContainer.h"
#include "xAODTrigger/jFexMETRoIAuxContainer.h"
#include "xAODTrigger/jFexSumETRoIContainer.h"
#include "xAODTrigger/jFexSumETRoIAuxContainer.h"
#include "L1CaloFEXSim/jFEXOutputCollection.h"
namespace LVL1 {
  
/*
Interface definition for jFEXSysSim
*/

  class IjFEXSysSim : virtual public IAlgTool {
  public:
    DeclareInterfaceID(IjFEXSysSim, 1, 0);

    virtual StatusCode execute(const EventContext& ctx, jFEXOutputCollection* inputOutputCollection) = 0;

    virtual void init() const = 0;

    virtual void cleanup() = 0;

    virtual int calcTowerID(int eta, int phi, int mod) const = 0;
    
  private:


  };

} // end of namespace

#endif
