/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1CALOMONITORING_GFEXMONITORALGORITHM_H
#define TRIGT1CALOMONITORING_GFEXMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODTrigger/gFexJetRoI.h"
#include "xAODTrigger/gFexJetRoIContainer.h"
#include "xAODTrigger/gFexGlobalRoI.h"
#include "xAODTrigger/gFexGlobalRoIContainer.h"

class GfexMonitorAlgorithm : public AthMonitorAlgorithm {
public:GfexMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~GfexMonitorAlgorithm()=default;
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

private:
  // Private members
  enum class FPGAType {FPGAa, FPGAb, FPGAc, None};
  std::map<std::string, std::pair<std::string, std::string>> m_globTobVarMap;
  StringProperty m_packageName{this,"PackageName","gfexMonitor","group name for histograming"};
  FloatArrayProperty m_ptCutValuesgLJ{this, "ptCutValuesgLJ", { 0., 30., 400.}, "List of lower pt cut values, -1. for not cut applied."};
  FloatArrayProperty m_ptCutValuesgJ{this, "ptCutValuesgJ", { 0., 100.}, "List of lower pt cut values, -1. for not cut applied."};
  // Define read handles
  SG::ReadHandleKeyArray<xAOD::gFexJetRoIContainer> m_gFexJetTobKeyList{this,"gFexJetTobKeyList",{"L1_gFexLRJetRoI", "L1_gFexSRJetRoI"},"Array of gFEX jet ReadHandleKeys to fill histograms for"};
  SG::ReadHandleKeyArray<xAOD::gFexJetRoIContainer> m_gFexRhoTobKeyList{this,"gFexRhoTobKeyList",{"L1_gFexRhoRoI"},"Array of gFEX rho ReadHandleKeys to fill histograms for"};
  SG::ReadHandleKeyArray<xAOD::gFexGlobalRoIContainer> m_gFexGlobalTobKeyList{this,"gFexGlobalTobKeyList", {"L1_gScalarEJwoj","L1_gMETComponentsJwoj","L1_gMHTComponentsJwoj","L1_gMSTComponentsJwoj"},"Array of gFEX global TOBs ReadHandleKeys to fill histograms for"};
  // Define private methods
  StatusCode fillJetHistograms(const std::string& handleKey, const xAOD::gFexJetRoIContainer* container, const float& ptCutValue, const auto& lbn) const;
  StatusCode fillRhoHistograms(const std::string& handleKey, const xAOD::gFexJetRoIContainer* container) const;
  StatusCode fillGlobalTobHistograms(const std::string& handleKey, const xAOD::gFexGlobalRoIContainer* container) const;
  FPGAType getFPGAType(const float& eta) const;
};
#endif
