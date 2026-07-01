/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef IgFEXSysSim_H
#define IgFEXSysSim_H

#include "GaudiKernel/IAlgTool.h"
#include "L1CaloFEXSim/gTowerContainer.h"
#include "L1CaloFEXSim/gFEXOutputCollection.h"

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

    virtual StatusCode fillgRhoEDM(uint32_t tobWord, int scale) = 0;

    virtual StatusCode fillgBlockEDM(uint32_t tobWord, int scale) = 0;

    virtual StatusCode fillgJetEDM(uint32_t tobWord, int scale) = 0;

    virtual StatusCode fillgScalarEJwojEDM(uint32_t tobWord, int scale1, int scale2) = 0;

    virtual StatusCode fillgMETComponentsJwojEDM(uint32_t tobWord, int scale1, int scale2) = 0;

    virtual StatusCode fillgMHTComponentsJwojEDM(uint32_t tobWord, int scale1, int scale2) = 0;

    virtual StatusCode fillgMSTComponentsJwojEDM(uint32_t tobWord, int scale1, int scale2) = 0;

    virtual StatusCode fillgMETComponentsNoiseCutEDM(uint32_t tobWord, int scale1, int scale2) = 0;

    virtual StatusCode fillgMETComponentsRmsEDM(uint32_t tobWord, int scale1, int scale2) = 0;

    virtual StatusCode fillgScalarENoiseCutEDM(uint32_t tobWord, int scale1, int scale2) = 0;

    virtual StatusCode fillgScalarERmsEDM(uint32_t tobWord, int scale1, int scale2) = 0;


  private:

  };

} // end of namespace

#endif
