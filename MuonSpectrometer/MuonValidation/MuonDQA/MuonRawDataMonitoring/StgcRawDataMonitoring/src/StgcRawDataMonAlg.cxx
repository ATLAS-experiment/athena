/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Package : sTgcRawDataMonAlg
// Author: Sebastian Fuenzalida Garrido
// Local supervisor: Edson Carquin Lopez
// Technical supervisor: Gerardo Vasquez
//
// DESCRIPTION:
// Subject: sTgc --> sTgc raw data monitoring
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include "StgcRawDataMonitoring/StgcRawDataMonAlg.h"

/////////////////////////////////////////////////////////////////////////////
// *********************************************************************
// Public Methods
// *********************************************************************
/////////////////////////////////////////////////////////////////////////////

sTgcRawDataMonAlg::sTgcRawDataMonAlg(const std::string& name, ISvcLocator* pSvcLocator) : AthMonitorAlgorithm(name, pSvcLocator) {
  //Declare the property
}

StatusCode sTgcRawDataMonAlg::initialize() {
  ATH_CHECK(AthMonitorAlgorithm::initialize());
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_sTgcContainerKey.initialize());
  ATH_CHECK(m_detectorManagerKey.initialize());
  ATH_CHECK(m_meTrkKey.initialize());
  ATH_CHECK(m_residualPullCalculator.retrieve());
  ATH_CHECK(m_muonKey.initialize());
  ATH_CHECK(m_rdoKey.initialize(SG::AllowEmpty));

  return StatusCode::SUCCESS;
}

StatusCode sTgcRawDataMonAlg::fillHistograms(const EventContext& ctx) const {
  const int lumiblock = GetEventInfo(ctx) -> lumiBlock();

  SG::ReadHandle<Muon::sTgcPrepDataContainer> sTgcContainer(m_sTgcContainerKey, ctx);
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> detectorManagerKey(m_detectorManagerKey, ctx);
  SG::ReadHandle<xAOD::TrackParticleContainer> meTPContainer(m_meTrkKey, ctx);
  SG::ReadHandle<xAOD::MuonContainer> muonContainer(m_muonKey, ctx);

  if (!meTPContainer.isValid()) {
    ATH_MSG_FATAL("Could not get track particle container: " << m_meTrkKey.fullKey());
    return StatusCode::FAILURE;
  }

  if (!muonContainer.isValid()) {
    ATH_MSG_FATAL("Could not get muon container: " << m_muonKey.fullKey());
    return StatusCode::FAILURE;
  }

  if(!m_rdoKey.key().empty()){
    SG::ReadHandle<Muon::NSW_PadTriggerDataContainer> NSWpadTriggerContainer(m_rdoKey, ctx);
    if (!NSWpadTriggerContainer.isValid()) {
      ATH_MSG_FATAL("Could not get pad trigger data container: " << m_rdoKey.fullKey());
      return StatusCode::FAILURE;
    }
    fillsTgcPadTriggerDataHistograms(muonContainer.cptr(), NSWpadTriggerContainer.cptr(), lumiblock);
    fillsTgcPadTriggerEfficiencyHistograms(muonContainer.cptr(), NSWpadTriggerContainer.cptr(), detectorManagerKey.cptr());
  }

  fillsTgcClusterFromTrackHistograms(meTPContainer.cptr());
  fillsTgcEfficiencyHistograms(muonContainer.cptr(), detectorManagerKey.cptr());
  fillsTgcOccupancyHistograms(sTgcContainer.cptr(), detectorManagerKey.cptr());
  fillsTgcLumiblockHistograms(sTgcContainer.cptr(), lumiblock);

  return StatusCode::SUCCESS;
}

void sTgcRawDataMonAlg::fillsTgcOccupancyHistograms(const Muon::sTgcPrepDataContainer* sTgcContainer, const MuonGM::MuonDetectorManager* muonDetectorManagerObject) const {
  for(const Muon::sTgcPrepDataCollection* coll : *sTgcContainer) {
    for (const Muon::sTgcPrepData* prd : *coll) {
      Identifier id = prd -> identify();

      if(!id.is_valid()) {
	ATH_MSG_WARNING("Invalid identifier found in Muon::sTgcPrepData");
	return;
      }

      std::string stationName = m_idHelperSvc -> stgcIdHelper().stationNameString(m_idHelperSvc -> stgcIdHelper().stationName(id));
      int stationEta          = m_idHelperSvc -> stgcIdHelper().stationEta(id);
      int stationPhi          = m_idHelperSvc -> stgcIdHelper().stationPhi(id);
      int multiplet           = m_idHelperSvc -> stgcIdHelper().multilayer(id);
      int gasGap              = m_idHelperSvc -> stgcIdHelper().gasGap(id);
      int channelType         = m_idHelperSvc -> stgcIdHelper().channelType(id);
      int sector              = m_idHelperSvc -> sector(id);
      int sectorsTotal        = getSectors(id);
      int layer               = getLayer(multiplet, gasGap);

      if (channelType == sTgcIdHelper::sTgcChannelTypes::Pad) {
	int padNumber = m_idHelperSvc -> stgcIdHelper().channel(id);
	Identifier idPadQ1 = m_idHelperSvc -> stgcIdHelper().channelID(stationName, 1, stationPhi, multiplet, gasGap, channelType, 1);
	Identifier idPadQ2 = m_idHelperSvc -> stgcIdHelper().channelID(stationName, 2, stationPhi, multiplet, gasGap, channelType, 1);
	const MuonGM::sTgcReadoutElement* sTgcReadoutObjectPadQ1 = muonDetectorManagerObject -> getsTgcReadoutElement(idPadQ1);
	const MuonGM::sTgcReadoutElement* sTgcReadoutObjectPadQ2 = muonDetectorManagerObject -> getsTgcReadoutElement(idPadQ2);
	int maxPadNumberQ1 = sTgcReadoutObjectPadQ1 -> maxPadNumber(idPadQ1);
	int maxPadNumberQ2 = sTgcReadoutObjectPadQ2 -> maxPadNumber(idPadQ2);

	if (std::abs(stationEta) == 1) {
	  auto sectorMon    = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sectorsTotal);
	  auto padNumberMon = Monitored::Scalar<int>("padNumber_layer_" + std::to_string(layer), padNumber);
	  fill("sTgcOccupancy", sectorMon, padNumberMon);
	}

	else if (std::abs(stationEta) == 2) {
	  auto sectorMon    = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sectorsTotal);
	  auto padNumberMon = Monitored::Scalar<int>("padNumber_layer_" + std::to_string(layer), padNumber + maxPadNumberQ1);
	  fill("sTgcOccupancy", sectorMon, padNumberMon);
	}

	else {
	  auto sectorMon    = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sectorsTotal);
	  auto padNumberMon = Monitored::Scalar<int>("padNumber_layer_" + std::to_string(layer), padNumber + maxPadNumberQ1 + maxPadNumberQ2);
	  fill("sTgcOccupancy", sectorMon, padNumberMon);
	}

	auto sectorSidedMon       = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sector);
	auto stationEtaSidedMon   = Monitored::Scalar<int>("stationEta_layer_" + std::to_string(layer), stationEta);
	fill("sTgcQuadOccupancyPad", sectorSidedMon, stationEtaSidedMon);
      }

      else if (channelType == sTgcIdHelper::sTgcChannelTypes::Strip) {
	int stripNumber      = m_idHelperSvc -> stgcIdHelper().channel(id);
	Identifier idStripQ1 = m_idHelperSvc -> stgcIdHelper().channelID(stationName, 1, stationPhi, multiplet, gasGap, channelType, 1);
	Identifier idStripQ2 = m_idHelperSvc -> stgcIdHelper().channelID(stationName, 2, stationPhi, multiplet, gasGap, channelType, 1);
	const MuonGM::sTgcReadoutElement* sTgcReadoutObjectStripQ1 = muonDetectorManagerObject -> getsTgcReadoutElement(idStripQ1);
	const MuonGM::sTgcReadoutElement* sTgcReadoutObjectStripQ2 = muonDetectorManagerObject -> getsTgcReadoutElement(idStripQ2);
	int maxStripNumberQ1 = sTgcReadoutObjectStripQ1 -> numberOfStrips(idStripQ1);
	int maxStripNumberQ2 = sTgcReadoutObjectStripQ2 -> numberOfStrips(idStripQ2);

	if (std::abs(stationEta) == 1) {
	  auto sectorMon      = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sectorsTotal);
	  auto stripNumberMon = Monitored::Scalar<int>("stripNumber_layer_" + std::to_string(layer), stripNumber);
	  fill("sTgcOccupancy", sectorMon, stripNumberMon);
	}

	else if (std::abs(stationEta) == 2) {
	  auto sectorMon      = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sectorsTotal);
	  auto stripNumberMon = Monitored::Scalar<int>("stripNumber_layer_" + std::to_string(layer), stripNumber + maxStripNumberQ1 + 1);
	  fill("sTgcOccupancy", sectorMon, stripNumberMon);
	}

	else {
	  auto sectorMon      = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sectorsTotal);
	  auto stripNumberMon = Monitored::Scalar<int>("stripNumber_layer_" + std::to_string(layer), stripNumber + maxStripNumberQ1 + maxStripNumberQ2 + 1);
	  fill("sTgcOccupancy", sectorMon, stripNumberMon);
	}

	auto sectorSidedMon         = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sector);
	auto stationEtaSidedMon     = Monitored::Scalar<int>("stationEta_layer_" + std::to_string(layer), stationEta);
	fill("sTgcQuadOccupancyStrip", sectorSidedMon, stationEtaSidedMon);
      }

      else if (channelType == sTgcIdHelper::sTgcChannelTypes::Wire) {
	int wireGroupNumber      = m_idHelperSvc -> stgcIdHelper().channel(id);
	Identifier idWireGroupQ3 = m_idHelperSvc -> stgcIdHelper().channelID("STL", 3, stationPhi, 1, 3, channelType, 1);
	const MuonGM::sTgcReadoutElement* sTgcReadoutObjectWireGroupQ3 = muonDetectorManagerObject -> getsTgcReadoutElement(idWireGroupQ3);
	int maxWireGroupNumberQ3 = sTgcReadoutObjectWireGroupQ3 -> numberOfStrips(idWireGroupQ3);

	auto stationEtaMon      = Monitored::Scalar<int>("stationEta_layer_" + std::to_string(layer), stationEta);
	auto wireGroupNumberMon = Monitored::Scalar<int>("wireGroupNumber_layer_" + std::to_string(layer), wireGroupNumber + (sector - 1)*maxWireGroupNumberQ3);
	fill("sTgcOccupancy", stationEtaMon, wireGroupNumberMon);

	auto sectorSidedMon             = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sector);
	auto stationEtaSidedMon         = Monitored::Scalar<int>("stationEta_layer_" + std::to_string(layer), stationEta);
	fill("sTgcQuadOccupancyWire", sectorSidedMon, stationEtaSidedMon);
      }
    }
  }
}

void sTgcRawDataMonAlg::fillsTgcLumiblockHistograms(const Muon::sTgcPrepDataContainer* sTgcContainer, int lb) const {
  for(const Muon::sTgcPrepDataCollection* coll : *sTgcContainer) {
    for (const Muon::sTgcPrepData* prd : *coll) {
      Identifier id = prd -> identify();

      if(!id.is_valid()) {
	ATH_MSG_WARNING("Invalid identifier found in Muon::sTgcPrepData");
	return;
      }

      std::string stationName = m_idHelperSvc -> stgcIdHelper().stationNameString(m_idHelperSvc -> stgcIdHelper().stationName(id));
      int stationEta          = m_idHelperSvc -> stgcIdHelper().stationEta(id);
      int multiplet           = m_idHelperSvc -> stgcIdHelper().multilayer(id);
      int gasGap              = m_idHelperSvc -> stgcIdHelper().gasGap(id);
      int channelType         = m_idHelperSvc -> stgcIdHelper().channelType(id);
      int layer               = getLayer(multiplet, gasGap);
      int sectorsTotal        = getSectors(id);

      if (channelType == sTgcIdHelper::sTgcChannelTypes::Pad) {
	auto padSectorMon = Monitored::Scalar<int>("padSector_quad_" + std::to_string(std::abs(stationEta)) + "_layer_" + std::to_string(layer), sectorsTotal);
	auto padLumiblockMon  = Monitored::Scalar<int>("padLumiblock_quad_" + std::to_string(std::abs(stationEta)) + "_layer_" + std::to_string(layer), lb);
	fill("sTgcLumiblockPad_quad_" + std::to_string(std::abs(stationEta)), padSectorMon, padLumiblockMon);

	auto padSectorGlobalMon = Monitored::Scalar<int>("padSector", sectorsTotal);
	auto padLumiblockGlobalMon  = Monitored::Scalar<int>("padLumiblock", lb);
	fill("sTgcLumiblock", padSectorGlobalMon, padLumiblockGlobalMon);
      }

      else if (channelType == sTgcIdHelper::sTgcChannelTypes::Strip) {
	auto stripSectorMon = Monitored::Scalar<int>("stripSector_quad_" + std::to_string(std::abs(stationEta)) + "_layer_" + std::to_string(layer), sectorsTotal);
	auto stripLumiblockMon  = Monitored::Scalar<int>("stripLumiblock_quad_" + std::to_string(std::abs(stationEta)) + "_layer_" + std::to_string(layer), lb);
	fill("sTgcLumiblockStrip_quad_" + std::to_string(std::abs(stationEta)), stripSectorMon, stripLumiblockMon);

	auto stripSectorGlobalMon = Monitored::Scalar<int>("stripSector", sectorsTotal);
	auto stripLumiblockGlobalMon  = Monitored::Scalar<int>("stripLumiblock", lb);
	fill("sTgcLumiblock", stripSectorGlobalMon, stripLumiblockGlobalMon);
      }

      else if (channelType == sTgcIdHelper::sTgcChannelTypes::Wire) {
	auto wireSectorMon = Monitored::Scalar<int>("wireSector_quad_" + std::to_string(std::abs(stationEta)) + "_layer_" + std::to_string(layer), sectorsTotal);
	auto wireLumiblockMon  = Monitored::Scalar<int>("wireLumiblock_quad_" + std::to_string(std::abs(stationEta)) + "_layer_" + std::to_string(layer), lb);
	fill("sTgcLumiblockWire_quad_" + std::to_string(std::abs(stationEta)), wireSectorMon, wireLumiblockMon);

	auto wireSectorGlobalMon = Monitored::Scalar<int>("wireSector", sectorsTotal);
	auto wireLumiblockGlobalMon  = Monitored::Scalar<int>("wireLumiblock", lb);
	fill("sTgcLumiblock", wireSectorGlobalMon, wireLumiblockGlobalMon);
      }
    }
  }
}

void sTgcRawDataMonAlg::fillsTgcClusterFromTrackHistograms(const xAOD::TrackParticleContainer*  trkPartCont) const {
  for (const xAOD::TrackParticle* meTP : *trkPartCont) {
    const Trk::Track* meTrack = meTP -> track();
    if(!meTrack) continue;
    for (const Trk::TrackStateOnSurface* trkState : *meTrack -> trackStateOnSurfaces()) {
      std::optional<std::tuple<Identifier, const Trk::RIO_OnTrack*>> status = getRotIdAndRotObject(trkState);
      if (!status.has_value()) continue;
      std::tuple<Identifier, const Trk::RIO_OnTrack*> rotIDtuple = status.value();

      Identifier rot_id = std::get<Identifier>(rotIDtuple);

      const Muon::sTgcClusterOnTrack* cluster = dynamic_cast<const Muon::sTgcClusterOnTrack*>(std::get<const Trk::RIO_OnTrack*>(rotIDtuple));
      if(!cluster) continue;

      const Muon::sTgcPrepData* prd = cluster -> prepRawData();
      if (!prd) continue;

      int channelType = m_idHelperSvc  -> stgcIdHelper().channelType(rot_id);
      int stEta       =  m_idHelperSvc -> stgcIdHelper().stationEta(rot_id);
      int multi       = m_idHelperSvc  -> stgcIdHelper().multilayer(rot_id);
      int gap         = m_idHelperSvc  -> stgcIdHelper().gasGap(rot_id);
      int sector      = m_idHelperSvc -> sector(rot_id);
      int sectorsTotal        = getSectors(rot_id);
      int layer               = getLayer(multi, gap);
      int iside               = (stEta > 0) ? 1 : 0;
      std::string side        = GeometricSectors::sTgcSide[iside];

      if (channelType == sTgcIdHelper::sTgcChannelTypes::Pad) {
	float padCharge     = prd -> charge();
	auto padChargeMon = Monitored::Scalar<float>("padTrackCharge_" + side + "_quad_" + std::to_string(std::abs(stEta)) + "_sector_" + std::to_string(sector)  + "_layer_" + std::to_string(layer), padCharge);
	fill("padCharge_" + side + std::to_string(sector) + "_quad_" + std::to_string(std::abs(stEta)), padChargeMon);

	short int padTiming = prd -> time();
	auto padSectorSidedMon = Monitored::Scalar<int>("padTrackSectorSided_layer_" + std::to_string(layer), sectorsTotal);
	auto padTimingMon      = Monitored::Scalar<float>("padTrackTiming_layer_" + std::to_string(layer), padTiming);
	fill("sTgcTiming", padSectorSidedMon, padTimingMon);

	auto padSectorSidedExpertMon = Monitored::Scalar<int>("padTrackSectorSided_quad_" + std::to_string(std::abs(stEta)) + "_layer_" + std::to_string(layer), sectorsTotal);
	auto padTimingExpertMon      = Monitored::Scalar<float>("padTrackTiming_quad_" + std::to_string(std::abs(stEta)) + "_layer_" + std::to_string(layer), padTiming);
	fill("padTiming_quad_" + std::to_string(std::abs(stEta)), padSectorSidedExpertMon, padTimingExpertMon);
      }

      else if (channelType == sTgcIdHelper::sTgcChannelTypes::Strip) {
	const std::vector<Identifier>& stripIds = prd->rdoList();
	unsigned int csize = stripIds.size();

	std::vector<short int> stripTimesVec = prd -> stripTimes();
	std::vector<int> stripChargesVec = prd -> stripCharges();

	float stripClusterTimes = 0;
	float stripClusterCharges = 0;

	for (unsigned int sIdx = 0; sIdx < csize; ++sIdx) {
	  stripClusterTimes += stripTimesVec.at(sIdx);
	  stripClusterCharges += stripChargesVec.at(sIdx);
	}

	stripClusterTimes /= stripTimesVec.size();

	auto stripClusterChargesPerSideQuadMon = Monitored::Scalar<float>("stripTrackCharge_" + side  + "_quad_" + std::to_string(std::abs(stEta)) + "_sector_" + std::to_string(sector)  +  "_layer_" + std::to_string(layer), stripClusterCharges);
	fill("stripCharge_" + side + std::to_string(sector) + "_quad_" + std::to_string(std::abs(stEta)), stripClusterChargesPerSideQuadMon);

	auto stripClusterSectorSidedMon = Monitored::Scalar<int>("stripTrackSectorSided_layer_" + std::to_string(layer), sectorsTotal);
	auto stripClusterTimesMon       = Monitored::Scalar<float>("stripTrackTiming_layer_" + std::to_string(layer), stripClusterTimes);
	auto stripClusterSizeMon        = Monitored::Scalar<unsigned int>("stripTrackClusterSize_layer_" + std::to_string(layer), csize);
	fill("sTgcTiming", stripClusterSectorSidedMon, stripClusterTimesMon);
	fill("sTgcOverview", stripClusterSectorSidedMon, stripClusterTimesMon, stripClusterSizeMon);

	auto stripSectorSidedExpertMon = Monitored::Scalar<int>("stripTrackSectorSided_quad_" + std::to_string(std::abs(stEta)) + "_layer_" + std::to_string(layer), sectorsTotal);
	auto stripTimingExpertMon      = Monitored::Scalar<float>("stripTrackTiming_quad_" + std::to_string(std::abs(stEta)) + "_layer_" + std::to_string(layer), stripClusterTimes);
	fill("stripTiming_quad_" + std::to_string(std::abs(stEta)), stripSectorSidedExpertMon, stripTimingExpertMon);


	std::optional<Trk::ResidualPull> resPull(m_residualPullCalculator -> residualPull(trkState -> measurementOnTrack(), trkState -> trackParameters(), Trk::ResidualPull::ResidualType::Biased));

	if (resPull) {
	  float residual = resPull -> residual()[Trk::locX];
	  auto residualMon  = Monitored::Scalar<float>("residual_" + side + "_quad_" + std::to_string(std::abs(stEta)) + "_sector_" + std::to_string(sector) + "_layer_" + std::to_string(layer), residual);
	  fill("sTgcResiduals_" + side + std::to_string(sector) + "_quad_" + std::to_string(std::abs(stEta)), residualMon);
	}
      }

      else if (channelType == sTgcIdHelper::sTgcChannelTypes::Wire) {
	float wireGroupCharge     = prd -> charge();
	auto wireGroupChargeMon = Monitored::Scalar<float>("wireGroupTrackCharge_" + side + "_quad_" + std::to_string(std::abs(stEta)) + "_sector_" + std::to_string(sector)  + "_layer_" + std::to_string(layer), wireGroupCharge);
	fill("wireGroupCharge_" + side + std::to_string(sector) + "_quad_" + std::to_string(std::abs(stEta)), wireGroupChargeMon);

	short int wireGroupTiming = prd -> time();
	auto wireGroupSectorSidedMon = Monitored::Scalar<int>("wireGroupTrackSectorSided_layer_" + std::to_string(layer), sectorsTotal);
	auto wireGroupTimingMon      = Monitored::Scalar<float>("wireGroupTrackTiming_layer_" + std::to_string(layer), wireGroupTiming);
	fill("sTgcTiming", wireGroupSectorSidedMon, wireGroupTimingMon);

	auto wireSectorSidedExpertMon = Monitored::Scalar<int>("wireTrackSectorSided_quad_" + std::to_string(std::abs(stEta)) + "_layer_" + std::to_string(layer), sectorsTotal);
	auto wireTimingExpertMon      = Monitored::Scalar<float>("wireTrackTiming_quad_" + std::to_string(std::abs(stEta)) + "_layer_" + std::to_string(layer), wireGroupTiming);
	fill("wireTiming_quad_" + std::to_string(std::abs(stEta)), wireSectorSidedExpertMon, wireTimingExpertMon);
      }
    }
  }
}

void sTgcRawDataMonAlg::fillsTgcPadTriggerDataHistograms(const xAOD::MuonContainer*  muonContainer, const Muon::NSW_PadTriggerDataContainer* NSWpadTriggerObject, int lb) const {
  for (const xAOD::Muon* mu : *muonContainer) {
    if(mu -> pt() < m_cutPt) continue;
    if(!(mu -> author() == xAOD::Muon::Author::MuidCo || mu -> author() == xAOD::Muon::Author::MuidSA)) continue;
        
    const xAOD::TrackParticle* meTP = mu -> trackParticle(xAOD::Muon::TrackParticleType::ExtrapolatedMuonSpectrometerTrackParticle);
    if(meTP == nullptr) continue;

    const Trk::Track* meTrack = meTP -> track();
    if(!meTrack) continue;

    for(const Trk::TrackStateOnSurface* trkState : *meTrack->trackStateOnSurfaces()) {
      std::optional<Identifier> status = getRotId(trkState);
      if (!status.has_value()) continue;
      Identifier rot_id = status.value();

      int channelType = m_idHelperSvc -> stgcIdHelper().channelType(rot_id);
      if (channelType != sTgcIdHelper::sTgcChannelTypes::Pad) continue;

      for (const Muon::NSW_PadTriggerData* rdo : *NSWpadTriggerObject) {
	bool sideA = rdo -> sideA();
	bool largeSector = rdo -> largeSector();

	int iside = (sideA) ? 1 : 0;
	int isize = (largeSector) ? 1 : 0;

	std::string side = GeometricSectors::sTgcSide[iside];
	std::string size = GeometricSectors::sTgcSize[isize];

	size_t numberOfTriggers = rdo -> getNumberOfTriggers();
	size_t numberOfHits     = rdo -> getNumberOfHits();

	for (size_t trigger = 0; trigger < numberOfTriggers; ++trigger) {
	  int triggerPhiIdsUnsigned = rdo -> getTriggerPhiIds().at(trigger);
	  int triggerBandIds        = rdo -> getTriggerBandIds().at(trigger);
	  int triggerRelBCID        = rdo -> getTriggerRelBcids().at(trigger);
	  int sourceId     = rdo -> getSourceid();
	  int sectorNumber = sourceidToSector(sourceId, sideA);

	  if (triggerPhiIdsUnsigned == m_cutTriggerPhiId || triggerBandIds == m_cutTriggerBandId) continue;

	  int triggerPhiIds = getSignedPhiId(triggerPhiIdsUnsigned);

	  auto phiIdsPerSideSizeMon  = Monitored::Scalar<int>("phiIds_"  + side + "_" + size, triggerPhiIds);
	  auto bandIdsPerSideSizeMon = Monitored::Scalar<int>("bandIds_" + side + "_" + size, triggerBandIds);
	  fill("padTriggerShifter", phiIdsPerSideSizeMon, bandIdsPerSideSizeMon);

	  auto lbMon = Monitored::Scalar<int>("lb", lb);
	  auto relBCIDMon = Monitored::Scalar<int>("relBCID", triggerRelBCID);
	  auto sectorMon = Monitored::Scalar<int>("sector", sectorNumber);
	  auto numberOfTriggersMon = Monitored::Scalar<int>("numberOfTriggers", numberOfTriggers);
	  fill("padTriggerShifter", lbMon, relBCIDMon, sectorMon, numberOfTriggersMon);

	  auto numberOfTriggersPerSectorMon = Monitored::Scalar<int>("numberOfTriggers_" + side + "_sector_" + std::to_string(std::abs(sectorNumber)), numberOfTriggers);
	  auto phiIdsSidedSizedPerSectorMon  = Monitored::Scalar<int>("phiIds_"  + side + "_sector_" + std::to_string(std::abs(sectorNumber)), triggerPhiIds);
	  auto bandIdsSidedSizedPerSectorMon = Monitored::Scalar<int>("bandIds_" + side + "_sector_" + std::to_string(std::abs(sectorNumber)), triggerBandIds);
	  auto lbPerSectorMon = Monitored::Scalar<int>("lb_" + side + "_sector_" + std::to_string(std::abs(sectorNumber)), lb);
	  auto relBCIDperSectorMon = Monitored::Scalar<int>("relBCID_"  + side + "_sector_" + std::to_string(std::abs(sectorNumber)), triggerRelBCID);
	  fill("padTriggerExpert", numberOfTriggersPerSectorMon, phiIdsSidedSizedPerSectorMon, bandIdsSidedSizedPerSectorMon, lbPerSectorMon, relBCIDperSectorMon);

	  auto RelBCIDPerSectorMon = Monitored::Scalar<int>("relBCID_"+ side + "_sector_" + std::to_string(std::abs(sectorNumber)), triggerRelBCID);
	  auto PhiIDPerSectorMon   = Monitored::Scalar<int>("phiIds_"  + side + "_sector_" + std::to_string(std::abs(sectorNumber)), triggerPhiIds);
	  auto BandIDPerSectorMon  = Monitored::Scalar<int>("bandID_" + side + "_sector_" + std::to_string(std::abs(sectorNumber)), triggerBandIds);
	  fill("padTriggerShifter", RelBCIDPerSectorMon, PhiIDPerSectorMon, BandIDPerSectorMon);
	} // end Number of triggers loop

	for (size_t hits = 0; hits < numberOfHits; ++hits){
	  std::optional<Identifier> status = getPadId(rdo->getSourceid(), rdo->getHitPfebs().at(hits), rdo->getHitTdsChannels().at(hits));
	  if (!status.has_value()) continue;
	  Identifier pad_id = status.value();

	  if (rot_id != pad_id) continue;

	  int sourceIds = rdo -> getSourceid();
	  int sectorNumbers = sourceidToSector(sourceIds, sideA);
	  int hitRelBCID = rdo -> getHitRelBcids().at(hits);
	  int hitpfebs = rdo -> getHitPfebs().at(hits);
	  int hitTdsChannels = rdo->getHitTdsChannels().at(hits);

	  std::optional<std::tuple<int, int, std::string, std::string, int>> statusPadEtaPhi = getPadEtaPhiTuple(sourceIds, hitpfebs, hitTdsChannels);
	  if (!statusPadEtaPhi.has_value()) continue;
	  std::tuple<int, int, std::string, std::string, int> padEtaPhiTuple = statusPadEtaPhi.value();

	  int padPhi = std::get<0>(padEtaPhiTuple);
	  int padEta = std::get<1>(padEtaPhiTuple);
	  std::string sideName = std::get<2>(padEtaPhiTuple);
	  std::string sizeName = std::get<3>(padEtaPhiTuple);
	  int layer = std::get<4>(padEtaPhiTuple);
	    	    
	  auto padPhiMon = Monitored::Scalar<int>("padPhi_" + sideName + "_" + sizeName + "_layer_" + std::to_string(layer), padPhi);
	  auto padEtaMon = Monitored::Scalar<int>("padEta_" + sideName + "_" + sizeName + "_layer_" + std::to_string(layer), padEta);
	  fill("padTriggerOccupancy", padPhiMon, padEtaMon);

	  auto hitRelBCIDmon = Monitored::Scalar<int>("hitRelBCID", hitRelBCID);
	  auto hitPfebsMon   = Monitored::Scalar<int>("hitPfebs", hitpfebs);
	  auto sectorMon     = Monitored::Scalar<int>("sector", sectorNumbers);
	  fill("padTriggerShifter", hitRelBCIDmon, hitPfebsMon, sectorMon);

	  auto hitRelBCIDPerSectorMon = Monitored::Scalar<int>("hitRelBCID_"+side+"_sector_"+std::to_string(std::abs(sectorNumbers)), hitRelBCID);
	  auto hitpfebsPerSectorMon = Monitored::Scalar<int>("hitPfebs_"+side+"_sector_"+std::to_string(std::abs(sectorNumbers)), hitpfebs);
	  fill("padTriggerShifter", hitRelBCIDPerSectorMon, hitpfebsPerSectorMon);
	} // end number of hits loop
      } // end NSW_PadTriggerData loop
    } // end TrackStateOnSurface loop
  } // end Muon loop
} // end fillsTgcPadTriggerDataHistograms function

void sTgcRawDataMonAlg::fillsTgcEfficiencyHistograms(const xAOD::MuonContainer*  muonContainer, const MuonGM::MuonDetectorManager* muonDetectorManagerObject) const {
  for (const xAOD::Muon* mu : *muonContainer) {
    if(mu -> pt() < m_cutPt) continue;
    if(!(mu -> author() == xAOD::Muon::Author::MuidCo || mu -> author() == xAOD::Muon::Author::MuidSA)) continue;

    const xAOD::TrackParticle* meTP = mu -> trackParticle(xAOD::Muon::TrackParticleType::ExtrapolatedMuonSpectrometerTrackParticle);
    if(meTP == nullptr) continue;

    const Trk::Track* meTrack = meTP -> track();
    if(!meTrack) continue;

    sTGCeff effPlots[2][3][16][2];

    for(const Trk::TrackStateOnSurface* trkState : *meTrack->trackStateOnSurfaces()) {
      std::optional<Identifier> status = getRotId(trkState);
      if (!status.has_value()) continue;

      Identifier rot_id = status.value();

      int channelType = m_idHelperSvc -> stgcIdHelper().channelType(rot_id);
      if (channelType != sTgcIdHelper::sTgcChannelTypes::Strip) continue;

      int stEta  = m_idHelperSvc -> stgcIdHelper().stationEta(rot_id);
      int iside  = (stEta > 0) ? 1 : 0;
      int sector = m_idHelperSvc -> sector(rot_id);
      int multi  = m_idHelperSvc -> stgcIdHelper().multilayer(rot_id);
      int gap    = m_idHelperSvc -> stgcIdHelper().gasGap(rot_id);

      const Amg::Vector2D& positionsMultiplet = (trkState) -> trackParameters() -> localPosition();
      float xPosStripInMultiplet = positionsMultiplet.x();
      float yPosStripInMultiplet = positionsMultiplet.y();

      Amg::Vector3D posStripGlobal{Amg::Vector3D::Zero()};
      (muonDetectorManagerObject -> getsTgcReadoutElement(rot_id)) -> stripGlobalPosition(rot_id, posStripGlobal);
      float zPosStripInMultiplet = posStripGlobal.z();

      if( ! (std::find(effPlots[iside][std::abs(stEta) - 1][sector - 1][multi - 1].layerMultiplet.begin(), effPlots[iside][std::abs(stEta) - 1][sector - 1][multi - 1].layerMultiplet.end(), gap) != effPlots[iside][std::abs(stEta) - 1][sector - 1][multi - 1].layerMultiplet.end()) ) {
	effPlots[iside][std::abs(stEta) - 1][sector - 1][multi - 1].layerMultiplet.push_back(gap);
	effPlots[iside][std::abs(stEta) - 1][sector - 1][multi - 1].xPosMultiplet.push_back(xPosStripInMultiplet);
	effPlots[iside][std::abs(stEta) - 1][sector - 1][multi - 1].yPosMultiplet.push_back(yPosStripInMultiplet);
	effPlots[iside][std::abs(stEta) - 1][sector - 1][multi - 1].zPosMultiplet.push_back(zPosStripInMultiplet);
      }
    } // end track state loop

    for (unsigned int isideIndex = 0; isideIndex <= 1; ++isideIndex) {
      for (unsigned int stEtaIndex = 1; stEtaIndex <= 3; ++stEtaIndex) {
	for (unsigned int sectorIndex = 1; sectorIndex <= 16; ++sectorIndex) {
	  for (unsigned int multiIndex = 1; multiIndex <= 2; ++multiIndex) {
	    if (effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.size() == 4) {
	      for (unsigned int gapIndex = 1; gapIndex <= effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.size(); ++gapIndex) {
		float xPos = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].xPosMultiplet.at(gapIndex - 1);
		float yPos = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].yPosMultiplet.at(gapIndex - 1);

		std::string side = GeometricSectors::sTgcSide[isideIndex];
		int quad = (side == "A") ? stEtaIndex : -stEtaIndex;

		bool isValid = false;
		const Identifier IDeffLayerStrip = m_idHelperSvc -> stgcIdHelper().channelID((sectorIndex % 2 == 0) ? "STS" : "STL", quad, (sectorIndex % 2 == 0) ? sectorIndex/2 : (sectorIndex + 1)/2,  multiIndex, gapIndex, sTgcIdHelper::sTgcChannelTypes::Strip, 1, isValid);

		if (!isValid) {
		  ATH_MSG_WARNING("Identifier of eff layer isn't valid");
		  continue;
		}

		int gasGapEff = m_idHelperSvc -> stgcIdHelper().gasGap(IDeffLayerStrip);
		int layerEff  = getLayer(multiIndex, gasGapEff);

		Amg::Vector2D localPos(xPos, yPos);
		Amg::Vector3D globalPos(0, 0, 0);
		const MuonGM::sTgcReadoutElement* sTgcReadoutObjectStrip = muonDetectorManagerObject -> getsTgcReadoutElement(IDeffLayerStrip);
		sTgcReadoutObjectStrip -> surface(IDeffLayerStrip).localToGlobal(localPos, Amg::Vector3D::Zero(), globalPos);

		float xPosGlobal = globalPos.x();
		float yPosGlobal = globalPos.y();
		float rPosGlobal = std::hypot(xPosGlobal, yPosGlobal);

		auto effQuestion = true;
		auto effQuestionMon = Monitored::Scalar<bool>("hitLayer", effQuestion);

		auto rPosStripMon = Monitored::Scalar<float>("rPosStrip_" + side + "_sector_" + std::to_string(sectorIndex)  + "_layer_" + std::to_string(layerEff), rPosGlobal);
		fill("rPosStrip_" + side + std::to_string(sectorIndex), rPosStripMon, effQuestionMon);

		auto xPosStripmon = Monitored::Scalar<float>("xPosStrip_" + side + "_layer_" + std::to_string(layerEff), xPosGlobal);
		auto yPosStripmon = Monitored::Scalar<float>("yPosStrip_" + side + "_layer_" + std::to_string(layerEff), yPosGlobal);
		fill("sTgcOverview", xPosStripmon, yPosStripmon, effQuestionMon);
	      } // close gap loop
	    } // close 4 out 4 if case

	    else if (effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.size() == 3) {
	      std::vector<int> refLayers(4, 0);

	      refLayers.at(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(0) - 1) = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(0);
	      refLayers.at(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(1) - 1) = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(1);
	      refLayers.at(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(2) - 1) = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(2);

	      int probeL1 = -999;

	      for (long unsigned int layerIt = 0; layerIt < refLayers.size(); ++layerIt) {
		if (refLayers.at(layerIt) == 0) {probeL1 = layerIt + 1;}
	      }

	      std::string side = GeometricSectors::sTgcSide[isideIndex];
	      int quad = (side == "A") ? stEtaIndex : -stEtaIndex;

	      bool isValid = false;
	      const Identifier idProbeL1 = m_idHelperSvc -> stgcIdHelper().channelID((sectorIndex % 2 == 0) ? "STS" : "STL", quad, (sectorIndex % 2 == 0) ? sectorIndex/2 : (sectorIndex + 1)/2, multiIndex, probeL1, sTgcIdHelper::sTgcChannelTypes::Strip, 1, isValid);

	      if (!isValid) {
		ATH_MSG_WARNING("Identifier of probe L1 is invalid");
		continue;
	      }

	      Amg::Vector3D posProbeL1{Amg::Vector3D::Zero()};
	      (muonDetectorManagerObject -> getsTgcReadoutElement(idProbeL1)) -> stripGlobalPosition(idProbeL1, posProbeL1);
	      float posZprobeL1 = posProbeL1.z();

	      float xSlope = (effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].xPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].xPosMultiplet.at(1))/(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(1));
	      float ySlope = (effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].yPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].yPosMultiplet.at(1))/(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(1));

	      float xPosProbeLayer1 = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].xPosMultiplet.at(0) + xSlope*(posZprobeL1 - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0));
	      float yPosProbeLayer1 = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].yPosMultiplet.at(0) + ySlope*(posZprobeL1 - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0));

	      int layerProbeL1 = getLayer(multiIndex, probeL1);

	      Amg::Vector2D localPosProbeLayer1(xPosProbeLayer1, yPosProbeLayer1);
	      Amg::Vector3D globalPosProbeLayer1(0, 0, 0);
	      const MuonGM::sTgcReadoutElement* sTgcReadoutObjectStrip = muonDetectorManagerObject -> getsTgcReadoutElement(idProbeL1);
	      sTgcReadoutObjectStrip -> surface(idProbeL1).localToGlobal(localPosProbeLayer1, Amg::Vector3D::Zero(), globalPosProbeLayer1);

	      float xPosGlobalProbeLayer1 = globalPosProbeLayer1.x();
	      float yPosGlobalProbeLayer1 = globalPosProbeLayer1.y();
	      float rPosGlobalProbeLayer1 = std::hypot(xPosGlobalProbeLayer1, yPosGlobalProbeLayer1);

	      auto effQuestion = false;
	      auto effQuestionMon = Monitored::Scalar<bool>("hitLayer", effQuestion);

	      auto rPosStripProbeL1mon = Monitored::Scalar<float>("rPosStrip_" + side + "_sector_" + std::to_string(sectorIndex)  + "_layer_" + std::to_string(layerProbeL1), rPosGlobalProbeLayer1);
	      fill("rPosStrip_" + side + std::to_string(sectorIndex), rPosStripProbeL1mon, effQuestionMon);

	      auto xPosStripProbeL1mon = Monitored::Scalar<float>("xPosStrip_" + side + "_layer_" + std::to_string(layerProbeL1), xPosGlobalProbeLayer1);
	      auto yPosStripProbeL1mon = Monitored::Scalar<float>("yPosStrip_" + side + "_layer_" + std::to_string(layerProbeL1), yPosGlobalProbeLayer1);
	      fill("sTgcOverview", xPosStripProbeL1mon, yPosStripProbeL1mon, effQuestionMon);
	    } // close 3 out 4 if case


	    else if (effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.size() == 2) {
	      std::vector<int> refLayers(4, 0);

	      refLayers.at(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(0) - 1) = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(0);
	      refLayers.at(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(1) - 1) = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(1);

	      std::vector<int> probeLayers;

	      for (long unsigned int layerIt = 0; layerIt < refLayers.size(); ++layerIt) {
		if (refLayers.at(layerIt) == 0) {probeLayers.push_back(layerIt + 1);}
	      }

	      int probeL1 = probeLayers.at(0);
	      int probeL2 = probeLayers.at(1);

	      std::string side = GeometricSectors::sTgcSide[isideIndex];
	      int quad = (side == "A") ? stEtaIndex : -stEtaIndex;

	      bool isValid = false;

	      const Identifier idProbeL1 = m_idHelperSvc -> stgcIdHelper().channelID((sectorIndex % 2 == 0) ? "STS" : "STL", quad, (sectorIndex % 2 == 0) ? sectorIndex/2 : (sectorIndex + 1)/2, multiIndex, probeL1, sTgcIdHelper::sTgcChannelTypes::Strip, 1, isValid);

	      if (!isValid) {
		ATH_MSG_WARNING("Identifier of probe L1 is invalid");
		continue;
	      }

	      const Identifier idProbeL2 = m_idHelperSvc -> stgcIdHelper().channelID((sectorIndex % 2 == 0) ? "STS" : "STL", quad, (sectorIndex % 2 == 0) ? sectorIndex/2 : (sectorIndex + 1)/2, multiIndex, probeL2, sTgcIdHelper::sTgcChannelTypes::Strip, 1, isValid);

	      if (!isValid) {
		ATH_MSG_WARNING("Identifier of probe L2 is invalid");
		continue;
	      }

	      Amg::Vector3D posProbeL1{Amg::Vector3D::Zero()};
	      (muonDetectorManagerObject -> getsTgcReadoutElement(idProbeL1)) -> stripGlobalPosition(idProbeL1, posProbeL1);
	      float posZprobeL1 = posProbeL1.z();

	      Amg::Vector3D posProbeL2{Amg::Vector3D::Zero()};
	      (muonDetectorManagerObject -> getsTgcReadoutElement(idProbeL2)) -> stripGlobalPosition(idProbeL2, posProbeL2);
	      float posZprobeL2 = posProbeL2.z();

	      float xSlope = (effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].xPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].xPosMultiplet.at(1))/(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(1));
	      float ySlope = (effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].yPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].yPosMultiplet.at(1))/(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(1));

	      float xPosProbeLayer1 = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].xPosMultiplet.at(0) + xSlope*(posZprobeL1 - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0));
	      float yPosProbeLayer1 = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].yPosMultiplet.at(0) + ySlope*(posZprobeL1 - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0));

	      float xPosProbeLayer2 = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].xPosMultiplet.at(0) + xSlope*(posZprobeL2 - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0));
	      float yPosProbeLayer2 = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].yPosMultiplet.at(0) + ySlope*(posZprobeL2 - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0));

	      int layerProbeL1   = getLayer(multiIndex, probeL1);
	      int layerProbeL2   = getLayer(multiIndex, probeL2);

	      Amg::Vector2D localPosProbeLayer1(xPosProbeLayer1, yPosProbeLayer1);
	      Amg::Vector3D globalPosProbeLayer1(0, 0, 0);
	      const MuonGM::sTgcReadoutElement* sTgcReadoutObjectStripProbeLayer1 = muonDetectorManagerObject -> getsTgcReadoutElement(idProbeL1);
	      sTgcReadoutObjectStripProbeLayer1 -> surface(idProbeL1).localToGlobal(localPosProbeLayer1, Amg::Vector3D::Zero(), globalPosProbeLayer1);

	      float xPosGlobalProbeLayer1 = globalPosProbeLayer1.x();
	      float yPosGlobalProbeLayer1 = globalPosProbeLayer1.y();
	      float rPosGlobalProbeLayer1 = std::hypot(xPosGlobalProbeLayer1, yPosGlobalProbeLayer1);

	      Amg::Vector2D localPosProbeLayer2(xPosProbeLayer2, yPosProbeLayer2);
	      Amg::Vector3D globalPosProbeLayer2(0, 0, 0);
	      const MuonGM::sTgcReadoutElement* sTgcReadoutObjectStripProbeLayer2 = muonDetectorManagerObject -> getsTgcReadoutElement(idProbeL2);
	      sTgcReadoutObjectStripProbeLayer2 -> surface(idProbeL2).localToGlobal(localPosProbeLayer2, Amg::Vector3D::Zero(), globalPosProbeLayer2);

	      float xPosGlobalProbeLayer2 = globalPosProbeLayer2.x();
	      float yPosGlobalProbeLayer2 = globalPosProbeLayer2.y();
	      float rPosGlobalProbeLayer2 = std::hypot(xPosGlobalProbeLayer2, yPosGlobalProbeLayer2);

	      auto effQuestion = false;
	      auto effQuestionMon = Monitored::Scalar<bool>("hitLayer", effQuestion);

	      auto rPosStripProbeL1mon = Monitored::Scalar<float>("rPosStrip_" + side + "_sector_" + std::to_string(sectorIndex)  + "_layer_" + std::to_string(layerProbeL1), rPosGlobalProbeLayer1);
	      fill("rPosStrip_" + side + std::to_string(sectorIndex), rPosStripProbeL1mon, effQuestionMon);

	      auto rPosStripProbeL2mon = Monitored::Scalar<float>("rPosStrip_" + side + "_sector_" + std::to_string(sectorIndex)  + "_layer_" + std::to_string(layerProbeL2), rPosGlobalProbeLayer2);
	      fill("rPosStrip_" + side + std::to_string(sectorIndex), rPosStripProbeL2mon, effQuestionMon);

	      auto xPosStripProbeL1mon = Monitored::Scalar<float>("xPosStrip_" + side + "_layer_" + std::to_string(layerProbeL1), xPosGlobalProbeLayer1);
	      auto yPosStripProbeL1mon = Monitored::Scalar<float>("yPosStrip_" + side + "_layer_" + std::to_string(layerProbeL1), yPosGlobalProbeLayer1);
	      fill("sTgcOverview", xPosStripProbeL1mon, yPosStripProbeL1mon, effQuestionMon);

	      auto xPosStripProbeL2mon = Monitored::Scalar<float>("xPosStrip_" + side + "_layer_" + std::to_string(layerProbeL2), xPosGlobalProbeLayer2);
	      auto yPosStripProbeL2mon = Monitored::Scalar<float>("yPosStrip_" + side + "_layer_" + std::to_string(layerProbeL2), yPosGlobalProbeLayer2);
	      fill("sTgcOverview", xPosStripProbeL2mon, yPosStripProbeL2mon, effQuestionMon);
	    } // close 2 out 4 if case
	  } // multiIndex loop end
	} // sectorIndex loop end
      } // stEtaIndex loop end
    } // isideIndex loop end
  } // End muon container loop
} // end stgc strip function

void sTgcRawDataMonAlg::fillsTgcPadTriggerEfficiencyHistograms(const xAOD::MuonContainer*  muonContainer, const Muon::NSW_PadTriggerDataContainer* NSWpadTriggerObject, const MuonGM::MuonDetectorManager* muonDetectorManagerObject) const {
  for (const xAOD::Muon* mu : *muonContainer) {
    if (!(mu -> author() == xAOD::Muon::Author::MuidCo || mu -> author() == xAOD::Muon::Author::MuidSA)) continue;
    if (mu -> pt() < m_cutPt) continue;
    if (std::abs(mu -> eta()) < m_cutEtaDown || std::abs(mu -> eta()) > m_cutEtaUp) continue; 

    double recoMuonEta = mu -> eta();
    double recoMuonPhi = mu -> phi();

    std::string sideRecoMuon = GeometricSectors::sTgcSide[recoMuonEta > 0];

    std::string minSideRecoMuon = "", minSideTrigger = "";
    
    double minTriggerEta = 999., minTriggerPhi = 999.,
      minRecoEta = 999., minRecoPhi = 999., minDeltaR = 999.;
    
    auto minDeltaRtrigIt = -1;
  
    for (const Muon::NSW_PadTriggerData* rdo : *NSWpadTriggerObject) {
      bool sideA = rdo -> sideA();
      bool largeSector = rdo -> largeSector();     
      std::string sideTrigger = GeometricSectors::sTgcSide[sideA];
      size_t numberOfTriggers = rdo -> getNumberOfTriggers();
      
      for (size_t trigger = 0; trigger < numberOfTriggers; ++trigger) {
        int triggerPhiIdsUnsigned = rdo -> getTriggerPhiIds().at(trigger);	  
        int triggerBandIds = rdo -> getTriggerBandIds().at(trigger);
        int sourceId = rdo -> getSourceid();
        
        if (triggerPhiIdsUnsigned == m_cutTriggerPhiId || triggerBandIds == m_cutTriggerBandId) continue;
        
        int triggerPhiIds = getSignedPhiId(triggerPhiIdsUnsigned);
        std::optional<double> status = bandId2eta(triggerBandIds, largeSector, sideA, muonDetectorManagerObject);
        if (!status.has_value()) continue;
        double triggerBandIdToEta = status.value();
        double triggerPhiIDtoPhi = triggersectorphiid2phi(sourceId, triggerPhiIds);
        double deltaR =  xAOD::P4Helpers::deltaR(recoMuonEta, recoMuonPhi, triggerBandIdToEta, triggerPhiIDtoPhi); 

        if (std::abs(triggerBandIdToEta) < m_cutEtaDown || std::abs(triggerBandIdToEta) > m_cutEtaUp) continue;
	
        if (sideRecoMuon == sideTrigger) { 
          if (deltaR < minDeltaR) {
            minSideRecoMuon = sideRecoMuon;
            minSideTrigger = sideTrigger;
            minTriggerEta = triggerBandIdToEta;
            minTriggerPhi = triggerPhiIDtoPhi;
            minRecoEta = recoMuonEta;
            minRecoPhi = recoMuonPhi;
            minDeltaR = deltaR;
            minDeltaRtrigIt = trigger;
          }
        }
      } // end number of triggers loop
    } // end pad trigger data container loop
    
    bool muonRecoTriggerMatch = false;
    
    if (minDeltaRtrigIt != -1) {
      if (minDeltaR < 0.1) {
        muonRecoTriggerMatch = true;
      }
    }
        
    auto deltaRmon = Monitored::Scalar<double>("deltaR", minDeltaR);
    fill("sTgcOverview", deltaRmon);
    
    auto etaRecoMuonMon = Monitored::Scalar<double>("etaRecoMuon", minRecoEta);
    auto phiRecoMuonMon = Monitored::Scalar<double>("phiRecoMuon", minRecoPhi);
    fill("sTgcOverview", etaRecoMuonMon, phiRecoMuonMon);
    
    auto etaPadTriggerMon = Monitored::Scalar<double>("etaPadTrigger", minTriggerEta);
    auto phiPadTriggerMon = Monitored::Scalar<double>("phiPadTrigger", minTriggerPhi);
    fill("sTgcOverview", etaPadTriggerMon, phiPadTriggerMon);

    auto phiRecoMuonSidedMon = Monitored::Scalar<double>("phiRecoMuon_" + minSideRecoMuon, minRecoPhi);
    auto phiPadTriggerSidedMon = Monitored::Scalar<double>("phiPadTrigger_" + minSideTrigger, minTriggerPhi);
    fill("sTgcOverview", phiRecoMuonSidedMon, phiPadTriggerSidedMon);
    
    auto muonRecoTriggerMatchMon = Monitored::Scalar<bool>("muonRecoTriggerMatch", muonRecoTriggerMatch);
    auto etaRecoMuonEffMon = Monitored::Scalar<double>("etaRecoMuonEff", minRecoEta);
    auto phiRecoMuonEffMon = Monitored::Scalar<double>("phiRecoMuonEff", minRecoPhi);
    fill("sTgcOverview", muonRecoTriggerMatchMon, etaRecoMuonEffMon, phiRecoMuonEffMon);

    auto muonRecoTriggerMatchSidedMon = Monitored::Scalar<bool>("muonRecoTriggerMatch", muonRecoTriggerMatch);
    auto phiRecoMuonEffSidedMon = Monitored::Scalar<double>("phiRecoMuonEff_" + minSideRecoMuon, minRecoPhi);
    fill("sTgcOverview", muonRecoTriggerMatchSidedMon, phiRecoMuonEffSidedMon);
  } // end muon container loop
}
