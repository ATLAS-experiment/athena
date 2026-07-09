/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef IgFEXSysSim_H
#define IgFEXSysSim_H

#include "GaudiKernel/IAlgTool.h"
#include "L1CaloFEXSim/gTowerContainer.h"
#include "L1CaloFEXSim/gFEXOutputCollection.h"
#include "xAODTrigger/gFexJetRoIContainer.h"
#include "xAODTrigger/gFexGlobalRoIContainer.h"


namespace LVL1 {

/*
Interface definition for gFEXSysSim
*/

  class IgFEXSysSim : virtual public IAlgTool {
  public:
    DeclareInterfaceID(IgFEXSysSim, 1, 0);

    virtual StatusCode execute(const EventContext& ctx, gFEXOutputCollection* gFEXOutputs) = 0;

    virtual void cleanup() = 0;

    virtual int calcTowerID(int eta, int phi, int nphi, int mod) const = 0 ;

    virtual StatusCode fillgRhoEDM(xAOD::gFexJetRoIContainer* gRhoContainer, uint32_t tobWord, int scale) const = 0;

    virtual StatusCode fillgBlockEDM(xAOD::gFexJetRoIContainer* gBlockContainer, uint32_t tobWord, int scale) const = 0;

    virtual StatusCode fillgJetEDM(xAOD::gFexJetRoIContainer* gJetContainer, uint32_t tobWord, int scale) const = 0;

    virtual StatusCode fillgScalarEJwojEDM(xAOD::gFexGlobalRoIContainer* gScalarEJwojContainer, uint32_t tobWord, int scale1, int scale2) const = 0;

    virtual StatusCode fillgMETComponentsJwojEDM(xAOD::gFexGlobalRoIContainer* gMETComponentsJwojContainer, uint32_t tobWord, int scale1, int scale2) const = 0;

    virtual StatusCode fillgMHTComponentsJwojEDM(xAOD::gFexGlobalRoIContainer* gMHTComponentsJwojContainer, uint32_t tobWord, int scale1, int scale2) const = 0;

    virtual StatusCode fillgMSTComponentsJwojEDM(xAOD::gFexGlobalRoIContainer* gMSTComponentsJwojContainer, uint32_t tobWord, int scale1, int scale2) const = 0;

    virtual StatusCode fillgMETComponentsNoiseCutEDM(xAOD::gFexGlobalRoIContainer* gMETComponentsNoiseCutContainer, uint32_t tobWord, int scale1, int scale2) const = 0;

    virtual StatusCode fillgMETComponentsRmsEDM(xAOD::gFexGlobalRoIContainer* gMETComponentsRmsContainer, uint32_t tobWord, int scale1, int scale2) const = 0;

    virtual StatusCode fillgScalarENoiseCutEDM(xAOD::gFexGlobalRoIContainer* gScalarENoiseCutContainer, uint32_t tobWord, int scale1, int scale2) const = 0;

    virtual StatusCode fillgScalarERmsEDM(xAOD::gFexGlobalRoIContainer* gScalarERmsContainer, uint32_t tobWord, int scale1, int scale2) const = 0;


  private:

  };

} // end of namespace

#endif
