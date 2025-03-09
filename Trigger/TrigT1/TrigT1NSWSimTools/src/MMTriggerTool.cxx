/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1NSWSimTools/MMTriggerTool.h"

namespace NSWL1 {

  MMTriggerTool::MMTriggerTool( const std::string& type, const std::string& name, const IInterface* parent) :
    base_class(type,name,parent),
    m_MmIdHelper(nullptr) {}

  StatusCode MMTriggerTool::initialize() {

    ATH_MSG_DEBUG( "initializing -- " << name() );

    ATH_MSG_DEBUG( name() << " configuration:");
    ATH_MSG_DEBUG(" " << std::setw(32) << std::setfill('.') << std::setiosflags(std::ios::left) << m_mmDigitContainer.name() << m_mmDigitContainer.value());
    ATH_MSG_DEBUG(" " << std::setw(32) << std::setfill('.') << std::setiosflags(std::ios::left) << m_doNtuple.name() << ((m_doNtuple)? "[True]":"[False]")
                      << std::setfill(' ') << std::setiosflags(std::ios::right) );

    ATH_CHECK(m_keyMcEventCollection.initialize(m_isMC));
    ATH_CHECK(m_keyMuonEntryLayer.initialize(m_isMC));
    ATH_CHECK(m_keyMmDigitContainer.initialize());

    if(m_doNtuple and Gaudi::Concurrency::ConcurrencyFlags::numConcurrentEvents() > 1) {
      ATH_MSG_ERROR("DoNtuple is not possible in multi-threaded mode");
      return StatusCode::FAILURE;
    }

    //  retrieve the MuonDetectormanager
    ATH_CHECK(m_detManagerKey.initialize());

    //  retrieve the Mm offline Id helper
    ATH_CHECK( detStore()->retrieve( m_MmIdHelper ) );

    return StatusCode::SUCCESS;
  }

  void MMTriggerTool::fillPointers(const MuonGM::MuonDetectorManager* detManager) const{

    std::lock_guard guard{m_mutex};
    if (m_isInitialized) {
        return;
    } 

    m_par_large = std::make_shared<MMT_Parameters>("xxuvuvxx",'L', detManager);
    m_par_small = std::make_shared<MMT_Parameters>("xxuvuvxx",'S', detManager);

    m_isInitialized=true;
  }

  StatusCode MMTriggerTool::attachBranches(MuonVal::MuonTesterTree &tree) {
    m_trigger_diamond_ntrig = std::make_shared<MuonVal::VectorBranch<unsigned int> >(tree, "MM_diamond_ntrig");
    m_trigger_diamond_bc = std::make_shared<MuonVal::VectorBranch<int> >(tree, "MM_diamond_bc");
    m_trigger_diamond_sector = std::make_shared<MuonVal::VectorBranch<char> >(tree, "MM_diamond_sector");
    m_trigger_diamond_stationPhi = std::make_shared<MuonVal::VectorBranch<int> >(tree, "MM_diamond_stationPhi");
    m_trigger_diamond_totalCount = std::make_shared<MuonVal::VectorBranch<unsigned int> >(tree, "MM_diamond_totalCount");
    m_trigger_diamond_realCount = std::make_shared<MuonVal::VectorBranch<unsigned int> >(tree, "MM_diamond_realCount");
    m_trigger_diamond_iX = std::make_shared<MuonVal::VectorBranch<int> >(tree, "MM_diamond_iX");
    m_trigger_diamond_iU = std::make_shared<MuonVal::VectorBranch<int> >(tree, "MM_diamond_iU");
    m_trigger_diamond_iV = std::make_shared<MuonVal::VectorBranch<int> >(tree, "MM_diamond_iV");
    m_trigger_diamond_XbkgCount = std::make_shared<MuonVal::VectorBranch<unsigned int> >(tree, "MM_diamond_XbkgCount");
    m_trigger_diamond_UVbkgCount = std::make_shared<MuonVal::VectorBranch<unsigned int> >(tree, "MM_diamond_UVbkgCount");
    m_trigger_diamond_XmuonCount = std::make_shared<MuonVal::VectorBranch<unsigned int> >(tree, "MM_diamond_XmuonCount");
    m_trigger_diamond_UVmuonCount = std::make_shared<MuonVal::VectorBranch<unsigned int> >(tree, "MM_diamond_UVmuonCount");
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
    tree.addBranch(m_trigger_diamond_stationPhi);
    tree.addBranch(m_trigger_diamond_totalCount);
    tree.addBranch(m_trigger_diamond_realCount);
    tree.addBranch(m_trigger_diamond_iX);
    tree.addBranch(m_trigger_diamond_iU);
    tree.addBranch(m_trigger_diamond_iV);
    tree.addBranch(m_trigger_diamond_XbkgCount);
    tree.addBranch(m_trigger_diamond_UVbkgCount);
    tree.addBranch(m_trigger_diamond_XmuonCount);
    tree.addBranch(m_trigger_diamond_UVmuonCount);
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

    int event = ctx.eventID().event_number();
    ATH_MSG_DEBUG("********************************************************* EVENT NUMBER = " << event);

    //////////////////////////////////////////////////////////////
    //                                                          //
    // Load Variables From Containers into our Data Structures  //
    //                                                          //
    //////////////////////////////////////////////////////////////
    SG::ReadCondHandle<MuonGM::MuonDetectorManager> detManagerHandle{m_detManagerKey, ctx};
    const MuonGM::MuonDetectorManager* detManager = detManagerHandle.cptr();

    if(!m_isInitialized) {fillPointers(detManager);}
    std::map<std::string, std::shared_ptr<MMT_Parameters> > pars;
    pars["MML"] = m_par_large;
    pars["MMS"] = m_par_small;
    MMLoadVariables load = MMLoadVariables(detManager, m_MmIdHelper);

    std::map<std::pair<int, unsigned int>,std::vector<digitWrapper> > entries;
    std::map<std::pair<int, unsigned int>,std::vector<hitData_entry> > Hits_Data_Set_Time;
    std::map<std::pair<int, unsigned int>,evInf_entry> Event_Info;

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
      if(m_doTruth) ptrMuonEntryLayer = readMuonEntryLayer.cptr();
    }

    SG::ReadHandle<MmDigitContainer> readMmDigitContainer( m_keyMmDigitContainer, ctx );
    if( !readMmDigitContainer.isValid() ){
      ATH_MSG_ERROR("Cannot retrieve MmDigitContainer");
      return StatusCode::FAILURE;
    }

    ATH_CHECK( load.getMMDigitsInfo(ctx, ptrMcEventCollection, ptrMuonEntryLayer, readMmDigitContainer.cptr(), entries, Hits_Data_Set_Time, Event_Info) );

    if (entries.empty()) {
      ATH_MSG_WARNING("No digits available for processing, exiting");
      Hits_Data_Set_Time.clear();
      Event_Info.clear();
      return StatusCode::SUCCESS;
    }

    std::unique_ptr<MMT_Diamond> diamond = std::make_unique<MMT_Diamond>(detManager);
    if (do_MMDiamonds) {
      diamond->setTrapezoidalShape(m_trapShape);
      diamond->setXthreshold(m_diamXthreshold);
      diamond->setUV(m_uv);
      diamond->setUVthreshold(m_diamUVthreshold);
      diamond->setRoadSize(m_diamRoadSize);
      diamond->setRoadSizeUpX(m_diamOverlapEtaUp);
      diamond->setRoadSizeDownX(m_diamOverlapEtaDown);
      diamond->setRoadSizeUpUV(m_diamOverlapStereoUp);
      diamond->setRoadSizeDownUV(m_diamOverlapStereoDown);
    }

    // We need to extract truth info, if available
    for (const auto &it : Event_Info) {
      double trueta = -999., truphi = -999., trutheta = -999., trupt = -999., dt = -999., tpos = -999., ppos = -999., epos = -999., tent = -999., pent = -999., eent = -999.;
      trutheta = it.second.theta_ip; // truth muon at the IP
      truphi = it.second.phi_ip;
      trueta = it.second.eta_ip;
      trupt = it.second.pt;
      tpos = it.second.theta_pos; // muEntry position
      ppos = it.second.phi_pos;
      epos = it.second.eta_pos;
      tent = it.second.theta_ent; // muEntry momentum
      pent = it.second.phi_ent;
      eent = it.second.eta_ent;
      dt = it.second.dtheta;
      if (m_doNtuple) {
        m_trigger_trueEtaRange->push_back(trueta);
        m_trigger_truePtRange->push_back(trupt);
        m_trigger_trueThe->push_back(trutheta);
        m_trigger_truePhi->push_back(truphi);
        m_trigger_trueDth->push_back(dt); // theta_pos-theta_ent
        m_trigger_trueEtaPos->push_back(epos);
        m_trigger_trueThePos->push_back(tpos);
        m_trigger_truePhiPos->push_back(ppos);
        m_trigger_trueEtaEnt->push_back(eent);
        m_trigger_trueTheEnt->push_back(tent);
        m_trigger_truePhiEnt->push_back(pent);
      }
    }

    unsigned int particles = entries.rbegin()->first.second +1,  nskip=0;
    for (unsigned int i=0; i<particles; i++) {
      std::pair<int, unsigned int> pair_event (event,i);

      // Now let's switch to reco hits: firstly, extracting the station name we're working on...
      std::string station = "-";
      auto event_it = entries.find(pair_event);
      station = event_it->second[0].stName; // Station name is taken from the first digit! In MMLoadVariables there's a check to ensure all digits belong to the same station

      // Secondly, extracting the Phi of the station we're working on...
      int stationPhi = -999;
      digitWrapper dW = event_it->second[0];
      Identifier tmpID = dW.id();
      stationPhi = m_MmIdHelper->stationPhi(tmpID);

      // Finally, let's start with hits
      auto reco_it = Hits_Data_Set_Time.find(pair_event);
      if (reco_it != Hits_Data_Set_Time.end()) {
        if (reco_it->second.size() >= (diamond->getXthreshold()+diamond->getUVthreshold())) {
          if (do_MMDiamonds) {
            /*
             * Filling hits for each event: a new class, MMT_Hit, is called in
             * order to use both algorithms witghout interferences
             */
            diamond->createRoads_fillHits(i-nskip, reco_it->second, detManager, pars[station], stationPhi);
            if (m_doNtuple) {
              for(const auto &hit : reco_it->second) {
                m_trigger_VMM->push_back(hit.VMM_chip);
                m_trigger_plane->push_back(hit.plane);
                m_trigger_station->push_back(hit.station_eta);
                m_trigger_strip->push_back(hit.strip);
              }
              std::vector<double> slopes = diamond->getHitSlopes();
              for (const auto &s : slopes) m_trigger_RZslopes->push_back(s);
              slopes.clear();
            }
            diamond->resetSlopes();
            /*
             * Here we create roads with all MMT_Hit collected before (if any), then we save the results
             */
            diamond->findDiamonds(i-nskip, event);

            if (!diamond->getSlopeVector(i-nskip).empty()) {
              if (m_doNtuple) {
                m_trigger_diamond_ntrig->push_back(diamond->getSlopeVector(i-nskip).size());
                for (const auto &slope : diamond->getSlopeVector(i-nskip)) {
                  m_trigger_diamond_sector->push_back(diamond->getDiamond(i-nskip).sector);
                  m_trigger_diamond_stationPhi->push_back(diamond->getDiamond(i-nskip).stationPhi);
                  m_trigger_diamond_bc->push_back(slope.BC);
                  m_trigger_diamond_totalCount->push_back(slope.totalCount);
                  m_trigger_diamond_realCount->push_back(slope.realCount);
                  m_trigger_diamond_XbkgCount->push_back(slope.xbkg);
                  m_trigger_diamond_UVbkgCount->push_back(slope.uvbkg);
                  m_trigger_diamond_XmuonCount->push_back(slope.xmuon);
                  m_trigger_diamond_UVmuonCount->push_back(slope.uvmuon);
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
              for (const auto &slope : diamond->getSlopeVector(i-nskip)) slopeBC.push_back(slope.BC);
              std::sort(slopeBC.begin(), slopeBC.end());
              slopeBC.erase( std::unique(slopeBC.begin(), slopeBC.end()), slopeBC.end() );
              for (const auto &bc : slopeBC) {
                Muon::NSW_TrigRawData* trigRawData = new Muon::NSW_TrigRawData(diamond->getDiamond(i-nskip).stationPhi, diamond->getDiamond(i-nskip).side, bc);

                for (const auto &slope : diamond->getSlopeVector(i-nskip)) {
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
              ATH_MSG_DEBUG("Filled MM RDO container now having size: " << rdo->size() << ". Clearing event information!");
            } else ATH_MSG_DEBUG("No output slopes to store");
          } else ATH_MSG_WARNING("No algorithm defined, exiting gracefully");
        } else {
          ATH_MSG_DEBUG( "Available hits are " << reco_it->second.size() << ", less than X+UV threshold, skipping" );
          nskip++;
        }
      } else {
          ATH_MSG_WARNING( "Empty hit map, skipping" );
          nskip++;
      }
    } // Main particle loop
    entries.clear();
    Hits_Data_Set_Time.clear();
    Event_Info.clear();
    if (do_MMDiamonds) diamond->clearEvent();

    return StatusCode::SUCCESS;
  }
}//end namespace
