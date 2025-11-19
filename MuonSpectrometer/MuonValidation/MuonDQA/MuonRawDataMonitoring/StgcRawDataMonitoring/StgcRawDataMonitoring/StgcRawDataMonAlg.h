/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Package : sTgcRawDataMonAlg
// Author: Sebastian Fuenzalida Garrido
// Local supervisor: Edson Carquin Lopez
// Technical supervisor: Gerardo Vasquez
//
// DESCRIPTION:
// Subject: sTgc --> sTgc raw data monitoring
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef sTgcRawDataMonAlg_H
#define sTgcRawDataMonAlg_H

// Core Include
#include "AthenaMonitoring/AthMonitorAlgorithm.h"

// Helper Includes
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/sTgcPrepDataContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "MuonPrepRawData/sTgcPrepData.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonIdHelpers/sTgcIdHelper.h"
#include "MuonRIO_OnTrack/sTgcClusterOnTrack.h"

#include "TrkToolInterfaces/IResidualPullCalculator.h"
#include "TrkParameters/TrackParameters.h"

#include "MuonRDO/NSW_PadTriggerDataContainer.h"

#include "MuonNSWCommonDecode/NSWPadTriggerL1a.h"
#include "MuonNSWCommonDecode/MapperSTG.h"

#include "FourMomUtils/xAODP4Helpers.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"

// stl includes                                                                                 
#include <string>

namespace Muon {
  class sTgcPrepData;
}

namespace GeometricSectors {
  static const std::array<std::string, 2> sTgcSide = {"C", "A"};
  static const std::array<std::string, 2> sTgcSize = {"S", "L"};
}

class sTgcRawDataMonAlg: public AthMonitorAlgorithm {
  using decoder = Muon::nsw::NSWPadTriggerL1a;
  using mapper  = Muon::nsw::MapperSTG;
  using MuonSectorMapping = Muon::MuonSectorMapping;
  static constexpr uint32_t NVMMCHAN = Muon::nsw::Constants::N_CHAN_PER_VMM;
  static constexpr uint32_t FIRSTPFEBVMM = 1;
 public:
  sTgcRawDataMonAlg(const std::string& name, ISvcLocator* pSvcLocator);
  
  virtual ~sTgcRawDataMonAlg()=default;
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms(const EventContext& ctx) const override;
 private:  
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
  ToolHandle<Trk::IResidualPullCalculator> m_residualPullCalculator {this, "ResPullCalc", "Trk::ResidualPullCalculator/ResidualPullCalculator"};
  
  void fillsTgcOccupancyHistograms(const Muon::sTgcPrepDataContainer*, const MuonGM::MuonDetectorManager*, const int lb) const;
  void fillsTgcLumiblockHistograms(const Muon::sTgcPrepDataContainer*, const int lb) const;
  void fillsTgcClusterFromTrackHistograms(const xAOD::TrackParticleContainer*) const;  
  void fillsTgcPadTriggerDataHistograms(const xAOD::MuonContainer*, const Muon::NSW_PadTriggerDataContainer*, const int lb) const;
  void fillsTgcEfficiencyHistograms(const xAOD::MuonContainer*, const MuonGM::MuonDetectorManager*) const;
  void fillsTgcPadTriggerEfficiencyHistograms(const xAOD::MuonContainer*, const Muon::NSW_PadTriggerDataContainer*, const MuonGM::MuonDetectorManager* muonDetectorManagerObject) const;
  
  int getFEBs(int eta,int layers) const;
  int getSectors(const Identifier& id) const;
  int getLayer(const int multiplet, const int gasGap) const;
  int32_t sourceidToSector(uint32_t sourceid, bool isSideA) const;
  int getSignedPhiId(const uint32_t phiid) const;
  std::optional<Identifier> getPadId(uint32_t sourceid, uint32_t pfeb, uint32_t tdschan) const;
  std::optional<std::tuple<Identifier, const Trk::RIO_OnTrack*>> getRotIdAndRotObject(const Trk::TrackStateOnSurface* trkState) const;
  std::optional<Identifier> getRotId(const Trk::TrackStateOnSurface* trkState) const;
  std::optional<std::tuple<int, int, std::string, std::string, int>> getPadEtaPhiTuple(uint32_t sourceid, uint32_t pfeb, uint32_t tdschan) const;
  std::optional<double> band2theta(double rPosAtNSW, const MuonGM::MuonDetectorManager* muonDetectorManagerObject) const;
  std::optional<double> band2eta(double rPosAtNSW, const MuonGM::MuonDetectorManager* muonDetectorManagerObject) const;
  std::optional<double> rPosAtNsw2eta(double rPosAtNSW, bool isA, const MuonGM::MuonDetectorManager* muonDetectorManagerObject) const;
  std::optional<double> bandId2eta(int bandid, bool isLarge, bool isA, const MuonGM::MuonDetectorManager* muonDetectorManagerObject) const;
  double triggersectorphiid2phi(uint32_t sourceid, int trigger_phiid) const;
  
  SG::ReadHandleKey<Muon::sTgcPrepDataContainer> m_sTgcContainerKey{this,"sTgcPrepDataContainerName", "STGC_Measurements"};
  SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detectorManagerKey{this, "DetectorManagerKey", "MuonDetectorManager","Key of input MuonDetectorManager condition data"}; 
  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_meTrkKey{this, "METrkContainer", "ExtrapolatedMuonTrackParticles"};
  SG::ReadHandleKey<Muon::NSW_PadTriggerDataContainer> m_rdoKey{this, "NSW_PadTriggerDataKey", ""};  
  SG::ReadHandleKey<xAOD::MuonContainer> m_muonKey{this, "MuonsKey", "Muons"};

  Gaudi::Property<double> m_cutPt{this, "cutPt", 15000.};
  Gaudi::Property<double> m_cutEtaDown{this, "cutEtaDown", 1.3};
  Gaudi::Property<double> m_cutEtaUp{this, "cutEtaUp", 2.4};
  Gaudi::Property<double> m_minDeltaR{this, "minDeltaR", 0.1};
  Gaudi::Property<int> m_cutTriggerPhiId{this, "cutTriggerPhiId", 63};
  Gaudi::Property<int> m_cutTriggerBandId{this, "cutTriggerBandId", 255};
};    
#endif
