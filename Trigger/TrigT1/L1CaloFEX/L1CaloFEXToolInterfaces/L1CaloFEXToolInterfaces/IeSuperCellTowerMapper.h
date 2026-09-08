/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           IeSuperCellTowerMapper.h  -
//                              -------------------
//     begin                : 23 03 2019
//     email                :  jacob.julian.kempster@cern.ch
//  ***************************************************************************/

#ifndef IeSuperCellTowerMapper_H
#define IeSuperCellTowerMapper_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXSim/eTower.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include "L1CaloFEXSim/eTowerContainer.h"

namespace LVL1 {
  
/*
Interface definition for eSuperCellTowerMapper
*/

  static const InterfaceID IID_IeSuperCellTowerMapper("LVL1::IeSuperCellTowerMapper", 1, 0);

  class IeSuperCellTowerMapper : virtual public IAlgTool {
  public:
    static const InterfaceID& interfaceID( ) ;

    virtual StatusCode AssignSuperCellsToTowers(std::unique_ptr<eTowerContainer> & my_eTowerContainerRaw) const = 0;
    virtual StatusCode AssignTriggerTowerMapper(std::unique_ptr<eTowerContainer> & my_eTowerContainerRaw) const = 0;
    
    virtual int FindAndConnectTower(std::unique_ptr<eTowerContainer> & my_eTowerContainerRaw,CaloSampling::CaloSample sample,const int region, int layer, const int pos_neg, const int eta_index, const int phi_index, Identifier ID, float et, int prov, bool doPrint) const = 0;
    virtual void ConnectSuperCellToTower(std::unique_ptr<eTowerContainer> & my_eTowerContainerRaw, int iETower, Identifier ID, int iCell, float et, int layer, bool doenergysplit) const = 0;
    virtual int FindTowerIDForSuperCell(int towereta, int towerphi) const = 0;
    virtual void PrintCellSpec(const CaloSampling::CaloSample sample, int layer, const int region, const int eta_index, const int phi_index, const int pos_neg, int iETower, int iCell, int prov, Identifier ID, bool doenergysplit) const = 0;

  private:

  };

  inline const InterfaceID& LVL1::IeSuperCellTowerMapper::interfaceID()
  {
    return IID_IeSuperCellTowerMapper;
  }

} // end of namespace

#endif
