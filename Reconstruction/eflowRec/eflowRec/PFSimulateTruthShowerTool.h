/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFSIMULATETRUTHSHOWERTOOL_H
#define PFSIMULATETRUTHSHOWERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

static const InterfaceID IID_PFSimulateTruthShowerTool("PFSimulateTruthShowerTool", 1, 0);

#include "eflowRec/eflowCaloObject.h"

#include "CaloSimEvent/CaloCalibrationHitContainer.h"

class PFSimulateTruthShowerTool : public AthAlgTool {

public:

    PFSimulateTruthShowerTool(const std::string& type, const std::string& name, const IInterface* parent);

    static const InterfaceID& interfaceID();

    StatusCode initialize();
    StatusCode finalize();

    void simulateShower(eflowCaloObject& thisEFlowCaloObject) const;

private:

    /** ReadHandleKey for Active Tile Calibration Hits */
    SG::ReadHandleKey<CaloCalibrationHitContainer> m_tileActiveCaloCalibrationHitReadHandleKey{this,"tileActiveCaloCalibrationHitsName","TileCalibHitActiveCell","ReadHandleKey for Active Tile Calibration Hits"};

    /** ReadHandleKey for Active LAr Calibration Hits */
    SG::ReadHandleKey<CaloCalibrationHitContainer> m_lArActiveCaloCalibrationHitReadHandleKey{this,"lArActiveCaloCalibrationHitsName","LArCalibrationHitActive","ReadHandleKey for Active LAr Calibration Hits"};

  void fillMap(std::map<Identifier,double>& identifierToTruthEnergyMap, int uniqueID, const CaloCalibrationHit& thisCalibHit) const;

};

inline const InterfaceID& PFSimulateTruthShowerTool::interfaceID() { return IID_PFSimulateTruthShowerTool; }


#endif
