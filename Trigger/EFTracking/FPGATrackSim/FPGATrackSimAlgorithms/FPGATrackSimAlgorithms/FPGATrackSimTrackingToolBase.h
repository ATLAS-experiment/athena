// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATRACKSIMTRACKINGTOOLBASE_H
#define FPGATRACKSIMTRACKINGTOOLBASE_H

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "FPGATrackSimAlgorithms/IFPGATrackSimTrackingTool.h"

#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimBanks/FPGATrackSimSectorBank.h"
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"

#include "FPGATrackSimHough/IFPGATrackSimRoadFilterTool.h"


class FPGATrackSimTrackingToolBase : public extends<AthAlgTool, IFPGATrackSimTrackingTool> {
public:
  FPGATrackSimTrackingToolBase(const std::string& type, const std::string& name, const IInterface* parent);

  StatusCode setRoadSectors(std::vector<FPGATrackSimRoad> & roads);
  void matchIdealGeoSector(FPGATrackSimRoad& r);

protected:
  ServiceHandle<IFPGATrackSimMappingSvc>   m_FPGATrackSimMapping{ this,"FPGATrackSimMappingSvc","FPGATrackSimMappingSvc" };
  ServiceHandle<IFPGATrackSimBankSvc>   m_FPGATrackSimBank{ this,"FPGATrackSimBankSvc","FPGATrackSimBankSvc" };

  ToolHandle<IFPGATrackSimRoadFilterTool>  m_spRoadFilterTool {this, "SPRoadFilterTool", "FPGATrackSimSpacepointRoadFilterTool", "Spacepoint Road Filter Tool"};

  Gaudi::Property<bool> m_doRegionalMapping{ this, "RegionalMapping", false, "Use the sub-region maps to define the sector" };
  Gaudi::Property<bool> m_doEtaPatternConsts{ this, "doEtaPatternConsts", false, "Whether to use the eta pattern tool for constant generation" };
  Gaudi::Property<bool> m_useSpacePoints{ this, "useSpacePoints", false, "Whether we are using spacepoints." };
  Gaudi::Property<bool> m_useSectors { this, "useSectors", false, "Will reverse calculate the sector for track-fitting purposes" };
  Gaudi::Property<bool> m_idealGeoRoads { this, "IdealGeoRoads", true, "Set sectors to use ideal geometry fit constants" };
  Gaudi::Property<bool> m_isSecondStage { this, "isSecondStage", true, "Is this the second stage?" };
  Gaudi::Property <bool> m_do2ndStage {this, "Do2ndStageTrackFit", false, "Do 2nd stage track fit"};
};

#endif // FPGATRACKSIMTRACKINGTOOLBASE_H
