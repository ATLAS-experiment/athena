/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef IgFEXFPGA_H
#define IgFEXFPGA_H


#include "GaudiKernel/IAlgTool.h"
#include "xAODTrigger/gFexJetRoI.h"

#include "xAODTrigL1Calo/gFexTowerContainer.h"
#include "xAODTrigL1Calo/gFexTowerAuxContainer.h"


namespace LVL1 {

/*
Interface definition for gFEXFPGA
*/

  static const InterfaceID IID_IgFEXFPGA("LVL1::IgFEXFPGA", 1, 0);

  typedef  std::array<std::array<int, 12>, 32> gTowersType;
  typedef  std::array<std::array<int, 12>, 32> gTowersCentral;
  typedef  std::array<std::array<int, 8>, 32> gTowersForward;

  class IgFEXFPGA : virtual public IAlgTool {
  public:
    static const InterfaceID& interfaceID( ) ;

    // virtual void SetTowersAndCells_SG(gTowersCentral) = 0;
    // virtual void SetTowersAndCells_SG(gTowersForward) = 0;

    // virtual void GetEnergyMatrix(gTowersCentral &) const = 0;
    // virtual void GetEnergyMatrix(gTowersForward &) const = 0;

    virtual void FillgTowerEDMCentral(const EventContext& , SG::WriteHandle<xAOD::gFexTowerContainer> &, int, const gTowersCentral &, gTowersType &, gTowersType &, gTowersType &) const = 0;
    virtual void FillgTowerEDMForward(const EventContext& , SG::WriteHandle<xAOD::gFexTowerContainer> &, int, const gTowersForward &, const gTowersForward &, gTowersType &, gTowersType &, gTowersType &) const = 0;


  private:

  };

  inline const InterfaceID& LVL1::IgFEXFPGA::interfaceID()
  {
    return IID_IgFEXFPGA;
  }

} // end of namespace

#endif
