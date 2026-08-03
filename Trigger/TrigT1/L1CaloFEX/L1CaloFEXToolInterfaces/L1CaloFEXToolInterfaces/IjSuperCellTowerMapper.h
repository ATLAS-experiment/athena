/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           IjSuperCellTowerMapper.h  -
//                              -------------------
//     begin                : 23 03 2019
//     email                :  jacob.julian.kempster@cern.ch
//  ***************************************************************************/

#ifndef IjSuperCellTowerMapper_H
#define IjSuperCellTowerMapper_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXSim/jTower.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include "L1CaloFEXSim/jTowerContainer.h"

namespace LVL1 {
  
/*
Interface definition for jSuperCellTowerMapper
*/

  static const InterfaceID IID_IjSuperCellTowerMapper("LVL1::IjSuperCellTowerMapper", 1, 0);

  class IjSuperCellTowerMapper : virtual public IAlgTool {
  public:
    static const InterfaceID& interfaceID( ) ;

    virtual StatusCode AssignSuperCellsToTowers(std::unique_ptr<jTowerContainer> & my_jTowerContainerRaw) const = 0;
    virtual StatusCode AssignTriggerTowerMapper(std::unique_ptr<jTowerContainer> & my_jTowerContainerRaw) const = 0;
    
    virtual int FindAndConnectTower(std::unique_ptr<jTowerContainer> & my_jTowerContainerRaw,CaloSampling::CaloSample sample,const int region, int layer, const int pos_neg, const int eta_index, const int phi_index, Identifier ID, float et, int prov, bool doPrint,float eta_min, float eta_max, float eta0, float phi_min, float phi_max, float phi0) const = 0;
    virtual void ConnectSuperCellToTower(std::unique_ptr<jTowerContainer> & my_jTowerContainerRaw, int iETower, Identifier ID, int iCell, float et, int layer) const = 0;
    virtual int FindTowerIDForSuperCell(int towereta, int towerphi) const = 0;
    virtual void PrintCellSpec(const CaloSampling::CaloSample sample, int layer, const int region, const int eta_index, const int phi_index, const int pos_neg, int iETower, int iCell, int prov, Identifier ID, bool doenergysplit,float eta_min, float eta_max, float eta0, float phi_min, float phi_max, float phi0, bool cellValid = false) const = 0;

  private:

  };

  inline const InterfaceID& LVL1::IjSuperCellTowerMapper::interfaceID()
  {
    return IID_IjSuperCellTowerMapper;
  }

} // end of namespace

#endif
