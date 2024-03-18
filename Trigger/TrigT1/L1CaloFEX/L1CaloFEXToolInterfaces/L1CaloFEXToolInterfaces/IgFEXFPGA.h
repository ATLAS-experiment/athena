/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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

    virtual StatusCode init(int id) = 0;

    virtual void reset() = 0;

    virtual int getID() const = 0;


    // virtual void SetTowersAndCells_SG(gTowersCentral) = 0;
    // virtual void SetTowersAndCells_SG(gTowersForward) = 0;

    // virtual void GetEnergyMatrix(gTowersCentral &) const = 0;
    // virtual void GetEnergyMatrix(gTowersForward &) const = 0;

    virtual void FillgTowerEDMCentral(SG::WriteHandle<xAOD::gFexTowerContainer> &, gTowersCentral &, gTowersType &, gTowersType &, gTowersType &) = 0;
    virtual void FillgTowerEDMForward(SG::WriteHandle<xAOD::gFexTowerContainer> &, gTowersForward &, gTowersForward &, gTowersType &, gTowersType &, gTowersType &) = 0;


  private:

  };

  inline const InterfaceID& LVL1::IgFEXFPGA::interfaceID()
  {
    return IID_IgFEXFPGA;
  }

} // end of namespace

#endif
