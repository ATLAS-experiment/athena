/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MMTriggerTool.h"

namespace NSWL1 {

  MMTriggerTool::MMTriggerTool( const std::string& type, const std::string& name, const IInterface* parent) :
    base_class(type,name,parent) {}

  StatusCode MMTriggerTool::initialize() {

    ATH_MSG_DEBUG( name() << " configuration:");
    ATH_MSG_DEBUG(" " << std::setw(32) << std::setfill('.') << std::setiosflags(std::ios::left) << m_mmDigitContainer.name() << m_mmDigitContainer.value());
    ATH_MSG_DEBUG(" " << std::setw(32) << std::setfill('.') << std::setiosflags(std::ios::left) << m_doNtuple.name() << ((m_doNtuple)? "[True]":"[False]")
                      << std::setfill(' ') << std::setiosflags(std::ios::right) );

    ATH_CHECK(m_keyMcEventCollection.initialize(m_isMC));
    ATH_CHECK(m_keyMuonEntryLayer.initialize(m_isMC));
    ATH_CHECK(m_keyMmDigitContainer.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_detectorManagerKey.initialize());

    if(m_doNtuple and Gaudi::Concurrency::ConcurrencyFlags::numConcurrentEvents() > 1) {
      ATH_MSG_ERROR("DoNtuple is not possible in multi-threaded mode");
      return StatusCode::FAILURE;
    }

    m_diamond = std::make_unique<MMT_Diamond>(m_diamXthreshold, m_uv, m_diamUVthreshold, m_diamRoadSize, m_diamOverlapEtaUp, m_diamOverlapEtaDown, m_diamOverlapStereoUp, m_diamOverlapStereoDown);

    return StatusCode::SUCCESS;
  }

  StatusCode MMTriggerTool::attachBranches(MuonVal::MuonTesterTree &tree) {
    m_trigger_diamond_ntrig = std::make_shared<MuonVal::VectorBranch<unsigned int> >(tree, "MM_diamond_ntrig");
    m_trigger_diamond_bc = std::make_shared<MuonVal::VectorBranch<int> >(tree, "MM_diamond_bc");
    m_trigger_diamond_sector = std::make_shared<MuonVal::VectorBranch<char> >(tree, "MM_diamond_sector");
    m_trigger_diamond_sectorPhi = std::make_shared<MuonVal::VectorBranch<int> >(tree, "MM_diamond_sectorPhi");
    m_trigger_diamond_totalCount = std::make_shared<MuonVal::VectorBranch<unsigned int> >(tree, "MM_diamond_totalCount");
    m_trigger_diamond_iX = std::make_shared<MuonVal::VectorBranch<int> >(tree, "MM_diamond_iX");
    m_trigger_diamond_iU = std::make_shared<MuonVal::VectorBranch<int> >(tree, "MM_diamond_iU");
    m_trigger_diamond_iV = std::make_shared<MuonVal::VectorBranch<int> >(tree, "MM_diamond_iV");
    m_trigger_diamond_xCount = std::make_shared<MuonVal::VectorBranch<unsigned int> >(tree, "MM_diamond_xCount");
    m_trigger_diamond_uCount = std::make_shared<MuonVal::VectorBranch<unsigned int> >(tree, "MM_diamond_uCount");
    m_trigger_diamond_age = std::make_shared<MuonVal::VectorBranch<int> >(tree, "MM_diamond_age");
    m_trigger_diamond_mx = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_diamond_mx");
    m_trigger_diamond_my = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_diamond_my");
    m_trigger_diamond_Uavg = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_diamond_Uavg");
    m_trigger_diamond_Vavg = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_diamond_Vavg");
    m_trigger_diamond_mxl = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_diamond_mxl");
    m_trigger_diamond_theta = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_diamond_theta");
    m_trigger_diamond_eta = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_diamond_eta");
    m_trigger_diamond_dtheta = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_diamond_dtheta");
    m_trigger_diamond_phi = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_diamond_phi");
    m_trigger_diamond_phiShf = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_diamond_phiShf");
    m_trigger_diamond_TP_phi_id = std::make_shared<MuonVal::VectorBranch<uint8_t> >(tree, "MM_diamond_TP_phi_id");
    m_trigger_diamond_TP_R_id = std::make_shared<MuonVal::VectorBranch<uint8_t> >(tree, "MM_diamond_TP_R_id");
    m_trigger_diamond_TP_dTheta_id = std::make_shared<MuonVal::VectorBranch<uint8_t> >(tree, "MM_diamond_TP_dTheta_id");
    m_trigger_RZslopes = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_RZslopes");
    m_trigger_trueEtaRange = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_trueEtaRange");
    m_trigger_truePtRange = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_truePtRange");
    m_trigger_VMM = std::make_shared<MuonVal::VectorBranch<int> >(tree, "MM_VMM");
    m_trigger_plane = std::make_shared<MuonVal::VectorBranch<int> >(tree, "MM_plane");
    m_trigger_station = std::make_shared<MuonVal::VectorBranch<int> >(tree, "MM_station");
    m_trigger_strip = std::make_shared<MuonVal::VectorBranch<int> >(tree, "MM_strip");
    m_trigger_slope = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_slope");
    m_trigger_trueThe = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_trueThe");
    m_trigger_truePhi = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_truePhi");
    m_trigger_trueDth = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_trueDth");
    m_trigger_trueEtaEnt = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_trueEtaEnt");
    m_trigger_trueTheEnt = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_trueTheEnt");
    m_trigger_truePhiEnt = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_truePhiEnt");
    m_trigger_trueEtaPos = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_trueEtaPos");
    m_trigger_trueThePos = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_trueThePos");
    m_trigger_truePhiPos = std::make_shared<MuonVal::VectorBranch<double> >(tree, "MM_truePhiPos");

    tree.addBranch(m_trigger_diamond_ntrig);
    tree.addBranch(m_trigger_diamond_bc);
    tree.addBranch(m_trigger_diamond_sector);
    tree.addBranch(m_trigger_diamond_sectorPhi);
    tree.addBranch(m_trigger_diamond_totalCount);
    tree.addBranch(m_trigger_diamond_iX);
    tree.addBranch(m_trigger_diamond_iU);
    tree.addBranch(m_trigger_diamond_iV);
    tree.addBranch(m_trigger_diamond_xCount);
    tree.addBranch(m_trigger_diamond_uCount);
    tree.addBranch(m_trigger_diamond_age);
    tree.addBranch(m_trigger_diamond_mx);
    tree.addBranch(m_trigger_diamond_my);
    tree.addBranch(m_trigger_diamond_Uavg);
    tree.addBranch(m_trigger_diamond_Vavg);
    tree.addBranch(m_trigger_diamond_mxl);
    tree.addBranch(m_trigger_diamond_theta);
    tree.addBranch(m_trigger_diamond_eta);
    tree.addBranch(m_trigger_diamond_dtheta);
    tree.addBranch(m_trigger_diamond_phi);
    tree.addBranch(m_trigger_diamond_phiShf);
    tree.addBranch(m_trigger_diamond_TP_phi_id);
    tree.addBranch(m_trigger_diamond_TP_R_id);
    tree.addBranch(m_trigger_diamond_TP_dTheta_id);
    tree.addBranch(m_trigger_RZslopes);
    tree.addBranch(m_trigger_trueEtaRange);
    tree.addBranch(m_trigger_truePtRange);
    tree.addBranch(m_trigger_VMM);
    tree.addBranch(m_trigger_plane);
    tree.addBranch(m_trigger_station);
    tree.addBranch(m_trigger_strip);
    tree.addBranch(m_trigger_slope);
    tree.addBranch(m_trigger_trueThe);
    tree.addBranch(m_trigger_truePhi);
    tree.addBranch(m_trigger_trueDth);
    tree.addBranch(m_trigger_trueEtaEnt);
    tree.addBranch(m_trigger_trueTheEnt);
    tree.addBranch(m_trigger_truePhiEnt);
    tree.addBranch(m_trigger_trueEtaPos);
    tree.addBranch(m_trigger_trueThePos);
    tree.addBranch(m_trigger_truePhiPos);
    return StatusCode::SUCCESS;
  }

  StatusCode MMTriggerTool::runTrigger(const EventContext& ctx, Muon::NSW_TrigRawDataContainer* rdo, const bool do_MMDiamonds) const {

    uint64_t event = ctx.eventID().event_number();
    ATH_MSG_DEBUG("********************************************************* EVENT NUMBER = " << event);

    SG::ReadCondHandle<MuonGM::MuonDetectorManager> detManager{m_detectorManagerKey, ctx};
    if(!detManager.isValid()){
      ATH_MSG_ERROR("Failed to retrieve the MuonDetectorManager conditions object");
      return StatusCode::FAILURE;
    }

    const McEventCollection* ptrMcEventCollection = nullptr;
    const TrackRecordCollection* ptrMuonEntryLayer = nullptr;
    if(m_isMC){
      SG::ReadHandle<McEventCollection> readMcEventCollection( m_keyMcEventCollection, ctx );
      if( !readMcEventCollection.isValid() ){
        ATH_MSG_ERROR("Cannot retrieve McEventCollection");
        return StatusCode::FAILURE;
      }
      if(m_doTruth) ptrMcEventCollection = readMcEventCollection.cptr();
      SG::ReadHandle<TrackRecordCollection> readMuonEntryLayer( m_keyMuonEntryLayer, ctx );
      if( !readMuonEntryLayer.isValid() ){
        ATH_MSG_ERROR("Cannot retrieve MuonEntryLayer");
        return StatusCode::FAILURE;
      }
      if(m_doTruth and m_doNtuple) {
        ptrMuonEntryLayer = readMuonEntryLayer.cptr();
        MMLoadVariables load = MMLoadVariables();
        std::map<std::pair<uint64_t, unsigned int>,evInf_entry> Event_Info;
        ATH_CHECK(load.getTruthInfo(ctx, ptrMcEventCollection, ptrMuonEntryLayer, Event_Info));

        // Extract truth info, if available
        for (const auto &it : Event_Info) {
          m_trigger_trueEtaRange->push_back(it.second.eta_ip);
          m_trigger_truePtRange->push_back(it.second.pt);
          m_trigger_trueThe->push_back(it.second.theta_ip);
          m_trigger_truePhi->push_back(it.second.phi_ip);
          m_trigger_trueDth->push_back(it.second.dtheta); // theta_pos-theta_ent
          m_trigger_trueEtaPos->push_back(it.second.eta_pos);
          m_trigger_trueThePos->push_back(it.second.theta_pos);
          m_trigger_truePhiPos->push_back(it.second.phi_pos);
          m_trigger_trueEtaEnt->push_back(it.second.eta_ent);
          m_trigger_trueTheEnt->push_back(it.second.theta_ent);
          m_trigger_truePhiEnt->push_back(it.second.phi_ent);
        }
      }
    }

    SG::ReadHandle<MmDigitContainer> readMmDigitContainer( m_keyMmDigitContainer, ctx );
    if( !readMmDigitContainer.isValid() ){
      ATH_MSG_ERROR("Cannot retrieve MmDigitContainer");
      return StatusCode::FAILURE;
    }

    for (const MmDigitCollection* digitCollection : *readMmDigitContainer) {

      std::vector<std::shared_ptr<MMT_Hit> > ev_hits;
      for (const MmDigit* digit : *digitCollection) {
        const Identifier id = digit->identify();
        if (not m_idHelperSvc->isMM(id)) continue;

        const std::string stationName = m_idHelperSvc->chamberNameString(id);
        const int stationEta = m_idHelperSvc->stationEta(id);
        const int stationPhi = m_idHelperSvc->stationPhi(id);
        const int sector = m_idHelperSvc->sector(id);
        const int multiplet = m_idHelperSvc->mmIdHelper().multilayer(id);
        const int gasGap = m_idHelperSvc->mmIdHelper().gasGap(id);

        const int channel = m_idHelperSvc->mmIdHelper().channel(id);
        // Checking whether strip exceeds allowed ranges
        const MuonGM::MMReadoutElement* readout = detManager->getMMReadoutElement(id);
        if (channel < 1 or channel > (readout->getDesign(id))->totalStrips) continue;

        const float stripTime = digit->stripResponseTime();
        // Checking positive digitization time
        if (stripTime < 0.) continue;
        const int BC = std::ceil(stripTime/25.);
        ev_hits.emplace_back(std::make_shared<MMT_Hit>(id, stationName, stationEta, stationPhi, sector, multiplet, gasGap, channel, stripTime, BC, detManager.cptr()));

        if (ev_hits.back()->infSlope()) {
          ATH_MSG_WARNING("Infinite slope, removing hit");
          ev_hits.pop_back();
          continue;
        }
        if (m_doNtuple) {
          m_trigger_VMM->push_back(ev_hits.back()->getVMM());
          m_trigger_plane->push_back(ev_hits.back()->getPlane());
          m_trigger_station->push_back(ev_hits.back()->getStationEta());
          m_trigger_strip->push_back(ev_hits.back()->getChannel());
          m_trigger_RZslopes->push_back(ev_hits.back()->getRZSlope());
        }
      }

      // Go ahead when hits are more than X+UV thresholds
      if (do_MMDiamonds and ev_hits.size() >= (m_diamond->getXthreshold()+m_diamond->getUVthreshold())) {
        const bool isLarge = (std::ranges::all_of(ev_hits, [](const auto &hit) { return hit->getSector() == 'L'; }));
        const char sector = (isLarge) ? 'L' : 'S';
        const char side = (std::ranges::all_of(ev_hits, [](const auto &hit) { return hit->getStationEta() < 0; })) ? 'C' : 'A';
        const int sectorPhi = ev_hits[0]->getSectorPhi();
        const bool allSectorPhi = (std::ranges::all_of(ev_hits, [&](const auto &hit) { return hit->getSectorPhi() == sectorPhi; }));
        if (not allSectorPhi) {
          ATH_MSG_ERROR("Available digits belongs to different sectors IDs, unable to assign an unique ID in output RDO");
          return StatusCode::FAILURE;
        }
        const bool isEta1 = (std::ranges::all_of(ev_hits, [](const auto &hit) { return std::abs(hit->getStationEta()) == 1; }));

        // Setup roads
        std::vector<MMT_Road> ev_roads;
        m_diamond->createRoads(ev_roads, isLarge, isEta1);

        // Evaluate coincidences
        std::vector<slope_t> diamondSlopes;
        m_diamond->findDiamonds(ev_hits, ev_roads, diamondSlopes, sectorPhi);

        // Store output, if any, in debug ntuple (if enabled) and in trigger RDO
        if (not diamondSlopes.empty()) {
          if (m_doNtuple) {
            m_trigger_diamond_ntrig->push_back(diamondSlopes.size());
            for (const auto &slope : diamondSlopes) {
              m_trigger_diamond_sector->push_back(sector);
              m_trigger_diamond_sectorPhi->push_back(sectorPhi);
              m_trigger_diamond_bc->push_back(slope.BC);
              m_trigger_diamond_totalCount->push_back(slope.totalCount);
              m_trigger_diamond_xCount->push_back(slope.xCount);
              m_trigger_diamond_uCount->push_back(slope.uCount);
              m_trigger_diamond_iX->push_back(slope.iRoad);
              m_trigger_diamond_iU->push_back(slope.iRoadu);
              m_trigger_diamond_iV->push_back(slope.iRoadv);
              m_trigger_diamond_age->push_back(slope.age);
              m_trigger_diamond_mx->push_back(slope.mx);
              m_trigger_diamond_my->push_back(slope.my);
              m_trigger_diamond_Uavg->push_back(slope.uavg);
              m_trigger_diamond_Vavg->push_back(slope.vavg);
              m_trigger_diamond_mxl->push_back(slope.mxl);
              m_trigger_diamond_theta->push_back(slope.theta);
              m_trigger_diamond_eta->push_back(slope.eta);
              m_trigger_diamond_dtheta->push_back(slope.dtheta);
              m_trigger_diamond_phi->push_back(slope.phi);
              m_trigger_diamond_phiShf->push_back(slope.phiShf);
            }
          }

          // MM RDO filling below
          std::vector<int> slopeBC;
          for (const auto &slope : diamondSlopes) slopeBC.push_back(slope.BC);
          std::ranges::sort(slopeBC);
          slopeBC.erase( std::unique(slopeBC.begin(), slopeBC.end()), slopeBC.end() );
          for (const auto bc : slopeBC) {
            Muon::NSW_TrigRawData* trigRawData = new Muon::NSW_TrigRawData(sectorPhi-1, side, bc);

            for (const auto &slope : diamondSlopes) {
              if (bc == slope.BC) {
                Muon::NSW_TrigRawDataSegment* trigRawDataSegment = new Muon::NSW_TrigRawDataSegment();

                // Phi-id - here use local phi (not phiShf)
                uint8_t phi_id = 0;
                if (slope.phi > m_phiMax || slope.phi < m_phiMin) trigRawDataSegment->setPhiIndex(phi_id);
                else {
                  uint8_t nPhi = (1<<m_phiBits) -2; // To accomodate the new phi-id encoding prescription around 0
                  float phiSteps = (m_phiMax - m_phiMin)/nPhi;
                  for (uint8_t i=0; i<nPhi; i++) {
                    if ((slope.phi) < (m_phiMin+i*phiSteps)) {
                      phi_id = i;
                      break;
                    }
                  }
                  trigRawDataSegment->setPhiIndex(phi_id);
                }
                if (m_doNtuple) m_trigger_diamond_TP_phi_id->push_back(phi_id);

                // R-id
                double extrapolatedR = 7824.46*std::abs(std::tan(slope.theta)); // The Z plane is a fixed value, taken from SL-TP documentation
                uint8_t R_id = 0;
                if (extrapolatedR > m_rMax || extrapolatedR < m_rMin) trigRawDataSegment->setRIndex(R_id);
                else {
                  uint8_t nR = (1<<m_rBits) -1;
                  float Rsteps = (m_rMax - m_rMin)/nR;
                  for (uint8_t j=0; j<nR; j++) {
                    if (extrapolatedR < (m_rMin+j*Rsteps)) {
                      R_id = j;
                      break;
                    }
                  }
                  trigRawDataSegment->setRIndex(R_id);
                }
                if (m_doNtuple) m_trigger_diamond_TP_R_id->push_back(R_id);

                // DeltaTheta-id
                uint8_t dTheta_id = 0;
                if (slope.dtheta > m_dThetaMax || slope.dtheta < m_dThetaMin) trigRawDataSegment->setDeltaTheta(dTheta_id);
                else {
                  uint8_t ndTheta = (1<<m_dThetaBits) -1;
                  float dThetaSteps = (m_dThetaMax - m_dThetaMin)/ndTheta;
                  for (uint8_t k=0; k<ndTheta; k++) {
                    if ((slope.dtheta) < (m_dThetaMin+k*dThetaSteps)) {
                      dTheta_id = k;
                      break;
                    }
                  }
                  trigRawDataSegment->setDeltaTheta(dTheta_id);
                }
                if (m_doNtuple) m_trigger_diamond_TP_dTheta_id->push_back(dTheta_id);

                // Low R-resolution bit
                trigRawDataSegment->setLowRes(slope.lowRes);

                trigRawData->push_back(trigRawDataSegment);
              }
            }
            rdo->push_back(trigRawData);
          }
        }
      }
      else {
        ATH_MSG_DEBUG("Available hits are " << ev_hits.size() << ", less than X+UV threshold, skipping digit collection");
      }
    }

    return StatusCode::SUCCESS;
  }
}//end namespace
