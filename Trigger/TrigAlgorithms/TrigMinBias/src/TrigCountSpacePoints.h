/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMINBIAS_TRIGCOUNTSPACEPOINTS_H
#define TRIGMINBIAS_TRIGCOUNTSPACEPOINTS_H

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "TrkSpacePoint/SpacePointContainer.h"
#include "xAODTrigger/TrigCompositeAuxContainer.h"
#include "xAODTrigger/TrigCompositeContainer.h"

class TrigCountSpacePoints : public AthReentrantAlgorithm {
 public:
  TrigCountSpacePoints(const std::string &name, ISvcLocator *pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext &context) const override;

 private:
  Gaudi::Property<bool> m_doOnlyBLayer{this, "doOnlyBLayer", false, " "};
  Gaudi::Property<int> m_pixModuleThreshold{this, "pixModuleThreshold", 100, "Dont take modules with SP Count higher than this threshold in Pixel Det."};
  Gaudi::Property<int> m_sctModuleHigherThreshold{this, "sctModuleHigherThreshold", 2000000, "Dont take modules with SP Count higher than this threshold in SCT"};
  Gaudi::Property<int> m_sctModuleLowerThreshold{this, "sctModuleLowerThreshold", 0, "Dont take modules with SP Count lowerer than this threshold in SCT"};
  Gaudi::Property<int> m_pixelClusToTCut{this, "pixelClusToTCut", 20, "ToT Cut for Pixel Clusters"};

  SG::ReadHandleKey<SpacePointContainer> m_pixelSpKey{this, "PixelSpKey", "PixelTrigSpacePoints", " "};
  SG::ReadHandleKey<SpacePointContainer> m_sctSpKey{this, "SCTSpKey", "SCT_TrigSpacePoints", " "};
  SG::ReadHandleKey<PixelID> m_pixelHelperKey{this, "pixelHelperKey", "DetectorStore+PixelID", " "};
  SG::ReadHandleKey<SCT_ID> m_sctHelperKey{this, "sctHelperKey", "DetectorStore+SCT_ID", " "};
  SG::WriteHandleKey<xAOD::TrigCompositeContainer> m_spacePointsKey{this, "SpacePointsKey", "Undefined", ""};
  SG::WriteHandleKey<xAOD::TrigCompositeAuxContainer> m_spacePointsAuxKey{this, "SpacePointsAuxKey", "Undefined", ""};

  ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring tool"};
};

#endif  // TRIGMINBIAS_TRIGCOUNTSPACEPOINTS_H
