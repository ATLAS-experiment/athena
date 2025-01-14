/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
    
#include "MuonCalibSegmentCreator/MuonSegmentReader.h"
#include "GaudiKernel/ITHistSvc.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "MuonRIO_OnTrack/RpcClusterOnTrack.h"
#include "MuonRIO_OnTrack/TgcClusterOnTrack.h"
#include "TrkTrackSummary/MuonTrackSummary.h"
#include "TrkTrackSummary/TrackSummary.h"
#include "MuonCompetingRIOsOnTrack/CompetingMuonClustersOnTrack.h"


namespace MuonCalib {

StatusCode MuonSegmentReader::initialize()
{

  ATH_CHECK(m_evtKey.initialize() );
  //ATH_CHECK(m_TrkSegKey.initialize());
  ATH_CHECK(m_TrkKey.initialize());
  //ATH_CHECK(m_CbTrkKey.initialize());
  //ATH_CHECK(m_EMEO_TrkKey.initialize());  // run3 only EM EO MS tracks
  ATH_CHECK(m_MdtPrepDataKey.initialize());
  ATH_CHECK(m_RpcPrepDataKey.initialize());
  ATH_CHECK(m_TgcPrepDataKey.initialize());
  ATH_CHECK(m_calibrationTool.retrieve());
  ATH_MSG_VERBOSE("MdtCalibrationTool retrieved with pointer = " << m_calibrationTool);

  ATH_CHECK(m_DetectorManagerKey.initialize());
  ATH_CHECK(m_idToFixedIdTool.retrieve());
  ATH_CHECK(m_pullCalculator.retrieve());
  // ATH_CHECK(m_assocTool.retrieve());
  // ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_MuonIdHelper.retrieve());
  ATH_CHECK(m_printer.retrieve());
  //ATH_CHECK(histSvc.retrieve() );  
  ATH_CHECK(m_tree.init(this));


  
  return StatusCode::SUCCESS;
}

StatusCode MuonSegmentReader::execute()
{

  // fill eventInfo branches
  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_evtKey,ctx);
  if (!eventInfo.isValid()) {
    ATH_MSG_ERROR("Did not find xAOD::EventInfo");
    return StatusCode::FAILURE;
  }

  m_runNumber   = eventInfo->runNumber();
  m_eventNumber = eventInfo->eventNumber();
  m_lumiBlock    = eventInfo->lumiBlock();
  m_bcId        = eventInfo->bcid();
  m_timeStamp = eventInfo->timeStamp();
  m_pt = eventInfo->timeStampNSOffset(); // special treatment in eventInfo for muon calibration stream data
  
  // load Geo
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> DetectorManagerHandle{m_DetectorManagerKey, ctx};
  const MuonGM::MuonDetectorManager* MuonDetMgr = DetectorManagerHandle.cptr();
  if (!MuonDetMgr) {
      ATH_MSG_ERROR("Null pointer to the read MuonDetectorManager conditions object");
      return StatusCode::FAILURE;
  }

  // fill the rawMdt Hit branches
  SG::ReadHandle<Muon::MdtPrepDataContainer> mdtPrepRawData(m_MdtPrepDataKey, ctx);
  ATH_MSG_DEBUG("Size of MdtPrepDataContainer : "<<mdtPrepRawData->size());
  // Loop MdtPrepDataContainer
  for( auto mdtColl : *mdtPrepRawData){
    for (const Muon::MdtPrepData* prd : *mdtColl) {
      MuonFixedId fixid = m_idToFixedIdTool->idToFixedId(prd->identify());
      m_rawMdt_id.push_back(fixid.getIdInt()) ;
      m_rawMdt_tdc.push_back(prd->tdc());
      m_rawMdt_adc.push_back(prd->adc());
      m_rawMdt_gPos.push_back(prd->globalPosition());
      ATH_MSG_DEBUG("Mdt FixedId "<<fixid.getIdInt()<<" prd phi "<<prd->globalPosition().phi()<<" prd theta "<<prd->globalPosition().theta());
    }  // end of MdtPrepDataCollection                             
  } // end of MdtPrepDataContainer
  m_rawMdt_nRMdt = m_rawMdt_id.size() ;
  ATH_MSG_DEBUG("Number of MDT raw Hits : "<<m_rawMdt_nRMdt);

  // fill the rawRpc Hit branches
  SG::ReadHandle<Muon::RpcPrepDataContainer> rpcPrepRawData(m_RpcPrepDataKey, ctx);
  ATH_MSG_DEBUG("Size of RpcPrepDataContainer : "<<rpcPrepRawData->size());
  // Loop RpcPrepDataContainer
  for( auto rpcColl : *rpcPrepRawData) {
    for (const Muon::RpcPrepData* prd : *rpcColl) {
      MuonFixedId fixid = m_idToFixedIdTool->idToFixedId(prd->identify());
      m_rawRpc_id.push_back(fixid.getIdInt()) ;
      m_rawRpc_time.push_back(prd->time());
      m_rawRpc_gPos.push_back(prd->globalPosition());
      ATH_MSG_DEBUG("Rpc FixedId "<<fixid.getIdInt()<<" prd phi "<<prd->globalPosition().phi()<<" prd theta "<<prd->globalPosition().theta());
    }  // end of RpcPrepDataCollection                             
  } // end of RpcPrepDataContainer
  m_rawRpc_nRRpc = m_rawRpc_id.size() ;
  ATH_MSG_DEBUG("Number of Rpc raw Hits : "<<m_rawRpc_nRRpc);

  // fill the rawTgc Hit branches
  SG::ReadHandle<Muon::TgcPrepDataContainer> tgcPrepRawData(m_TgcPrepDataKey, ctx);
  ATH_MSG_DEBUG("Size of TgcPrepDataContainer : "<<tgcPrepRawData->size());
  // Loop TgcPrepDataContainer
  for (auto tgcColl : *tgcPrepRawData) {
    for (const Muon::TgcPrepData* prd : *tgcColl) {
      MuonFixedId fixid = m_idToFixedIdTool->idToFixedId(prd->identify());
      m_rawTgc_id.push_back(fixid.getIdInt()) ;
      m_rawTgc_bcBitMap.push_back(prd->getBcBitMap());
      m_rawTgc_gPos.push_back(prd->globalPosition());
      ATH_MSG_DEBUG("Tgc FixedId "<<fixid.getIdInt()<<" prd phi "<<prd->globalPosition().phi()<<" prd theta "<<prd->globalPosition().theta());
    }  // end of TgcPrepDataCollection                             
  } // end of TgcPrepDataContainer
  m_rawTgc_nRTgc = m_rawTgc_id.size() ;
  ATH_MSG_DEBUG("Number of Tgc raw Hits : "<<m_rawTgc_nRTgc);

  // fill the muon standalone tracks branches
  SG::ReadHandle<TrackCollection> muTrks(m_TrkKey, ctx);
  ATH_MSG_INFO("Number of Muon StandAlone Tracks : "<<muTrks->size());
  m_trk_nTracks = muTrks->size() ;
  // if tracks were found, print MDT track hits
  for (unsigned int itrk = 0; itrk < muTrks->size(); ++itrk) {
    const Trk::Track* trkSA = muTrks->at(itrk) ;
    ATH_MSG_DEBUG(m_printer->print(*trkSA));
    ATH_MSG_DEBUG("Track author : "<<m_printer->printPatRec(*trkSA));
    ATH_MSG_DEBUG("TrackHit measurements : "<<m_printer->printMeasurements(*trkSA));
    m_trk_author.push_back(208); // Hardcode 208 as Moore for MuonStandaloneTrack
    // get trackSummary
    const Trk::MuonTrackSummary* summary = nullptr;
    // check if the track already has a MuonTrackSummary, if not calculate it using the helper
    const Trk::TrackSummary* trkSummary = trkSA->trackSummary();
    if (not trkSummary) continue;
    m_trk_nMdtHits.push_back(trkSummary->get(Trk::numberOfMdtHits));
    //!< number of measurements flaged as outliers in TSOS
    m_trk_nOutliersHits.push_back(trkSummary->get(Trk::numberOfOutliersOnTrack));
    m_trk_nRpcPhiHits.push_back(trkSummary->get(Trk::numberOfRpcPhiHits));
    m_trk_nTgcPhiHits.push_back(trkSummary->get(Trk::numberOfTgcPhiHits));
    m_trk_nRpcEtaHits.push_back(trkSummary->get(Trk::numberOfRpcEtaHits));
    m_trk_nTgcEtaHits.push_back(trkSummary->get(Trk::numberOfTgcEtaHits));
    m_trk_nMdtHoles.push_back(trkSummary->get(Trk::numberOfMdtHoles));

    ATH_MSG_DEBUG("Mdt Hits " << trkSummary->get(Trk::numberOfMdtHits)
                  <<" Mdt Holes "<< trkSummary->get(Trk::numberOfMdtHoles) 
                  << "Mdt Outliers " << trkSummary->get(Trk::numberOfOutliersOnTrack) 
                  << " TGC Phi Eta Hits "<<trkSummary->get(Trk::numberOfTgcPhiHits)<<" "<<trkSummary->get(Trk::numberOfTgcEtaHits)
                  << " RPC Phi Eta Hits "<<trkSummary->get(Trk::numberOfRpcPhiHits)<<" "<<trkSummary->get(Trk::numberOfRpcEtaHits));

    summary = trkSummary->muonTrackSummary();
    if (!summary) {
	    ATH_MSG_WARNING("No muon summary is present");
    } else  {
      ATH_MSG_DEBUG("print MuonSummary : "<<m_printer->print(*summary));
    }

    // Trk perigee 
    const Trk::Perigee* perigee = trkSA->perigeeParameters();
    if (!perigee) {
      ATH_MSG_WARNING(" was expecting a perigee here... ");
      return StatusCode::FAILURE;
    }
    // track direction vector
    const Amg::Vector3D dir = perigee->momentum().unit();
    m_trk_perigee.push_back(dir);
    m_trk_d0.push_back(perigee->parameters()[Trk::d0]);
    m_trk_z0.push_back(perigee->parameters()[Trk::z0]);
    m_trk_phi.push_back(perigee->parameters()[Trk::phi0]);
    m_trk_theta.push_back(perigee->parameters()[Trk::theta]);
    m_trk_qOverP.push_back(perigee->parameters()[Trk::qOverP]);
    m_trk_eta.push_back(perigee->eta());
    m_trk_pt.push_back(perigee->pT());

    // Trk fit quality
    const Trk::FitQuality* fq = trkSA->fitQuality();        
    if (!fq)  {            
      ATH_MSG_WARNING(" was expecting a FitQuality here... ");            
      return StatusCode::FAILURE;        
    }
    m_trk_chi2.push_back(fq->chiSquared()) ;
    m_trk_ndof.push_back(fq->numberDoF());
    
    // Loop track hits by trk_states and store the measure 
    const DataVector<const Trk::TrackStateOnSurface>* trk_states = trkSA->trackStateOnSurfaces();
    if (!trk_states) {
      ATH_MSG_WARNING(" track without states, discarding track ");
      return StatusCode::FAILURE;
    }
    
    unsigned int nMeasurement = 0, nHole = 0;
    for (const Trk::TrackStateOnSurface* trk_state : *trk_states) {
     // First handle the case of mdtHole
      if (trk_state->type(Trk::TrackStateOnSurface::Hole)) {
          storeHole(trk_state, itrk);
          nHole ++;  
          continue;
      }

    const Trk::MeasurementBase* measurement = trk_state->measurementOnTrack();
    if (!measurement) continue;
    storeMeasurement(ctx, MuonDetMgr, itrk, trk_state, measurement);
    nMeasurement ++;
    } // end of track hit loop
    ATH_MSG_DEBUG("Total recorded hits in the track "<<itrk<<" #Mesurement "<<nMeasurement<<" #Hole "<<nHole) ; 
  } // end of track loop
  m_trkHit_nMdtHits=m_trkHit_type.size();
  m_trkTriggerHit_nHits=m_trkTriggerHit_type.size();
  m_trkHole_nHoles=m_trkHole_type.size();
  ATH_MSG_DEBUG("Total recorded "<<m_trkHit_type.size()<<" MDT hits, "<<m_trkTriggerHit_type.size()<<" Trigger hits and "<<m_trkHole_type.size()<<" hole hits in all tracks!") ; 


  if (!m_tree.fill(ctx)) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}

void MuonSegmentReader::storeHole(const Trk::TrackStateOnSurface * trk_state, unsigned int itrk) {
  ATH_MSG_DEBUG(" MDT Hole on track ");
  const Trk::TrackParameters *trackPars = trk_state->trackParameters();
  if (!trackPars) return;
  Amg::Vector3D pos = trackPars->position();
  m_trkHole_gPos.push_back(pos);
  
  const Identifier idHole = trk_state->trackParameters()->associatedSurface().associatedDetectorElementIdentifier();
  MuonFixedId fixid = m_idToFixedIdTool->idToFixedId(idHole);
  m_trkHole_id.push_back(fixid.getIdInt()) ;
  ATH_MSG_DEBUG(" Hole stored at position x " << pos.x() << " y " << pos.y() << " z " << pos.z());
  // check the hit type (1 MDT /2 RPC/3 TGC)
  if (m_MuonIdHelper->isMdt(idHole)) m_trkHole_type.push_back(1);
  else if (m_MuonIdHelper->isRpc(idHole)) m_trkHole_type.push_back(2);
  else if (m_MuonIdHelper->isTgc(idHole)) m_trkHole_type.push_back(3);
  else {
    m_trkHole_type.push_back(0);
    ATH_MSG_DEBUG("None of MDT/RPC/TGC Identifier match, something wrong!");
  }

  m_trkHole_driftRadius.push_back(std::abs(trackPars->parameters()[Trk::locR]));
  m_trkHole_trackIndex.push_back(itrk);

}  // end storeHole


void MuonSegmentReader::storeMeasurement(const EventContext& ctx, const MuonGM::MuonDetectorManager* MuonDetMgr, unsigned int itrk, const Trk::TrackStateOnSurface* trk_state, const Trk::MeasurementBase* measurement) {

  const Trk::TrackParameters *trackPars = trk_state->trackParameters();
  const Trk::RIO_OnTrack* rot = dynamic_cast<const Trk::RIO_OnTrack*>(measurement);

  if (rot) { // case of measurement exist
    Identifier rot_id = rot->identify();
    if (m_MuonIdHelper->isMdt(rot_id))  {  
      const Muon::MdtDriftCircleOnTrack* mrot = dynamic_cast<const Muon::MdtDriftCircleOnTrack*>(rot);
      if (!mrot) {
        ATH_MSG_DEBUG("This is not a  MdtDriftCircleOnTrack!!! ");
        return;
      }
      int hitType = 0;
      if (trk_state->type(Trk::TrackStateOnSurface::Measurement)) {
        hitType = 1;
      }
      else if (trk_state->type(Trk::TrackStateOnSurface::Outlier)) {
        hitType = 4;
      }
      else ATH_MSG_WARNING("This hit is not measurement o outlier, hit type wrong!");
      m_trkHit_type.push_back(hitType); 

      // get PRD  
      const Muon::MdtPrepData* prd = mrot->prepRawData();
      Identifier id = prd->identify();
      int adc = prd->adc();
      int tdc = prd->tdc();
      Amg::Vector3D prd_pos = prd->globalPosition();
      ATH_MSG_DEBUG("PRD ADC "<<adc<<" PRD TDC "<<tdc<<" PRD gPos "<<prd_pos);
      // add the implement of calibrationTool, initialize global position by mrot 
      MdtCalibInput calibIn{*prd};
      calibIn.setClosestApproach(mrot->globalPosition());
      calibIn.setTrackDirection(trackPars->momentum().unit());
      const MdtCalibOutput calibResult{m_calibrationTool->calibrate(ctx, calibIn, false)};
      ATH_MSG_DEBUG("print "<<calibIn  << " calibResult : "<<calibResult);
      m_trkHit_tubeT0.push_back(calibResult.tubeT0());
      m_trkHit_tubeMeanAdc.push_back(calibResult.meanAdc());
      m_trkHit_lorTime.push_back(calibResult.lorentzTime());
      m_trkHit_slewTime.push_back(calibResult.slewingTime());
      m_trkHit_propTime.push_back(calibResult.signalPropagationTime());
      m_trkHit_tempTime.push_back(calibResult.temperatureTime());
      m_trkHit_bkgTime.push_back(calibResult.backgroundTime());
      m_trkHit_tof.push_back(calibIn.timeOfFlight());
      m_trkHit_triggerTime.push_back(calibIn.triggerTime());
      m_trkHit_calibStatus.push_back(calibResult.status());

      // fill the trkHit branches
      m_trkHit_adc.push_back(adc) ;
      m_trkHit_tdc.push_back(tdc) ;
      m_trkHit_trackIndex.push_back(itrk) ;
      m_trkHit_driftTime.push_back(mrot->driftTime());
        
      float localAngle = mrot->localAngle() ;
      float driftRadius = mrot->driftRadius() ;
      float error = std::sqrt(mrot->localCovariance()(0, 0));
      m_trkHit_error.push_back(error) ;
      m_trkHit_localAngle.push_back(localAngle);
      m_trkHit_driftRadius.push_back(driftRadius) ;

      const MuonGM::MdtReadoutElement* detEl = MuonDetMgr->getMdtReadoutElement(id);
      if( !detEl ) {
        ATH_MSG_WARNING( "getGlobalToStation failed to retrieve detEL byebye"  );
        return;
      }

      // get the 2nd coordinator from the track hit measurement
      Amg::Vector3D trkHitPos = trackPars->position();
      // global to local transformation for chamber coordination
      Amg::Transform3D gToStation = detEl->GlobalToAmdbLRSTransform();

      Amg::Vector3D trkHitPosLoc = gToStation* trkHitPos;
      m_trkHit_closestApproach.push_back(trkHitPosLoc);
      m_trkHit_gClosestApproach.push_back(trkHitPos);

      // get trkHit local position from measurement 
      Amg::Vector3D mrot_gPos = mrot->globalPosition();   // equal to Amg::Vector3D mb_pos = it->globalPosition();
      Amg::Vector3D mrot_pos = gToStation* mrot_gPos;

      //distion to readout 2nd coordinators 
      float distRo_det = detEl->distanceFromRO(trkHitPos, id);
      m_trkHit_distRO.push_back(distRo_det) ;

      // save the local postion of hit center for local refitting, set x = 0, y = coordinator along the Ntube, z = coordinator along the Nlayer
      Amg::Vector3D wireCenter = gToStation*detEl->tubePos(id);
      wireCenter.x() = 0;
      ATH_MSG_DEBUG(m_MuonIdHelper->mdtIdHelper().stationNameString(int(m_MuonIdHelper->mdtIdHelper().stationName(id)))<<" "<<m_MuonIdHelper->stationEta(id)<<" "<<m_MuonIdHelper->stationPhi(id)
            <<" "<<"ml: "<<m_MuonIdHelper->mdtIdHelper().multilayer(id)<<" lb: "<<m_MuonIdHelper->mdtIdHelper().tubeLayer(id)<<" tube: "<<m_MuonIdHelper->mdtIdHelper().tube(id)
            << " detEl wireLocalCenter " << " x : "<<wireCenter.x()<< " y : " << wireCenter.y()<< " z : "<<wireCenter.z());

      m_trkHit_center.push_back(wireCenter);      
      m_trkHit_gPos.push_back(mrot_gPos);
      m_trkHit_pos.push_back(mrot_pos);

      // residual calculator  
      float residualBiased = -999.;
      float pullBiased = -999.;
      {
        std::optional<Trk::ResidualPull> resPullBiased = m_pullCalculator->residualPull(measurement, trackPars, Trk::ResidualPull::Biased );
        if(resPullBiased.has_value()){
          residualBiased = resPullBiased.value().residual().front();
          pullBiased = resPullBiased.value().pull().front();
        }
      }
      // residual definition double residual = measurement->localParameters()[Trk::loc1] - trkPar->parameters()[Trk::loc1];
      m_trkHit_resi.push_back(residualBiased) ;
      m_trkHit_pull.push_back(pullBiased) ;
      m_trkHit_rTrk.push_back(trackPars->parameters()[Trk::loc1]) ;

      MuonFixedId fixid = m_idToFixedIdTool->idToFixedId(id);
      m_trkHit_id.push_back(fixid.getIdInt()) ;

      // debugging printout
      ATH_MSG_DEBUG("trackHit distRO " << distRo_det<<" positionAlongWire "<<mrot->positionAlongWire()<<" tubeLength "<<detEl->tubeLength(id));
      ATH_MSG_DEBUG("prd hit global position measurement " <<" x : "<< prd_pos.x()<<" y : "<< prd_pos.y()<<" z : "<<prd_pos.z());
      ATH_MSG_DEBUG("MdtDriftCircleOnTrack hit global position measurement " << " x : " <<mrot_gPos.x()<<" y : "<< mrot_gPos.y() <<" z : "<<mrot_gPos.z());
      ATH_MSG_DEBUG("trackHitPos from trackPars " << " x : "<<trkHitPos.x() <<" y : "<<trkHitPos.y() <<" z : "<< trkHitPos.z() );
      ATH_MSG_DEBUG("trackHit Local Pos from globaltolocalCoords " <<" x : "<< trkHitPosLoc.x() <<" y : "<< trkHitPosLoc.y() <<" z : "<< trkHitPosLoc.z());
      ATH_MSG_DEBUG("mrod Local Pos from globaltolocalCoords " <<" x : "<< mrot_pos.x() <<" y : "<< mrot_pos.y() <<" z : "<< mrot_pos.z());
      ATH_MSG_DEBUG("TrackIndex "<<itrk<<" station " << fixid.stationNumberToFixedStationString(fixid.stationName()) << " eta " << fixid.eta() << " phi " << fixid.phi() << " ML " << fixid.mdtMultilayer() << " Layer " << fixid.mdtTubeLayer()
                                  <<" Tube "<<fixid.mdtTube()<< " MROT drift R " << mrot->driftRadius() << " errDriftRadius " << error<< " drift Time "
                                  << mrot->driftTime() <<" ADC "<<adc<<" TDC "<<tdc);
      ATH_MSG_DEBUG(" driftRadius from driftRadius func "<<driftRadius<<" rTrk track fit to wire "<<trackPars->parameters()[Trk::loc1]<<" residualBiased " << residualBiased << " pullBiased " << pullBiased);

    } // finish mdt hit loop

  else if (m_MuonIdHelper->isRpc(rot_id)) {

    ATH_MSG_DEBUG("Found the RPC ROT "<<rot_id<<" !");
    MuonFixedId fixid = m_idToFixedIdTool->idToFixedId(rot_id);
    m_trkTriggerHit_id.push_back(fixid.getIdInt()) ;
    m_trkTriggerHit_trackIndex.push_back(itrk);
    m_trkTriggerHit_type.push_back(1);  // RPC ROT
    const Muon::RpcClusterOnTrack* rpc = dynamic_cast<const Muon::RpcClusterOnTrack*>(rot);
    const Muon::RpcPrepData* rpcPRD = rpc->prepRawData();
    m_trkTriggerHit_time.push_back(rpcPRD->time());
    m_trkTriggerHit_gPos.push_back(rpcPRD->globalPosition());
      
  }   // end of RPC rot
  else if (m_MuonIdHelper->isTgc(rot_id)) {

    ATH_MSG_DEBUG("Found the TGC ROT "<<rot_id<<" !");
    MuonFixedId fixid = m_idToFixedIdTool->idToFixedId(rot_id);
    m_trkTriggerHit_id.push_back(fixid.getIdInt()) ;
    m_trkTriggerHit_trackIndex.push_back(itrk);
    m_trkTriggerHit_type.push_back(3); // tgc ROT
    const Muon::TgcClusterOnTrack* tgc  = dynamic_cast<const Muon::TgcClusterOnTrack*>(rot);
    const Muon::TgcPrepData*       tgcPRD = tgc->prepRawData() ;
    m_trkTriggerHit_time.push_back(tgcPRD->getBcBitMap());
    m_trkTriggerHit_gPos.push_back(tgcPRD->globalPosition());

  } //end of TGC rot
  else {
  ATH_MSG_DEBUG("Couldn't find track ROT ID "<<rot_id);
  }

  }   // end of rot case
  else {
    ATH_MSG_DEBUG("No ROT! Check the competingROT");
    // use competingROT from RPC and TGC
    const Muon::CompetingMuonClustersOnTrack* crot = dynamic_cast<const Muon::CompetingMuonClustersOnTrack*>(measurement);
    if (crot) {
      const std::vector<const Muon::MuonClusterOnTrack*>& rots = crot->containedROTs();
      if (rots.size() > 0) {
        std::vector<const Muon::MuonClusterOnTrack*>::const_iterator itR = rots.begin(), itR_end = rots.end();
        for (; itR != itR_end; ++itR) {
          Identifier crot_id = (*itR)->identify() ;
          MuonFixedId fixid = m_idToFixedIdTool->idToFixedId(crot_id);
          m_trkTriggerHit_id.push_back(fixid.getIdInt());
          m_trkTriggerHit_trackIndex.push_back(itrk);
          if (m_MuonIdHelper->isTgc(crot_id))  {
            ATH_MSG_DEBUG("This is TGC competing hits on track!") ;                  
            if (m_MuonIdHelper->tgcIdHelper().isStrip(crot_id)) m_trkTriggerHit_type.push_back(5);  // tgc strip hit, measuresPhi
            else m_trkTriggerHit_type.push_back(4); // tgc wire hit
            const Muon::TgcClusterOnTrack* tgc    = dynamic_cast<const Muon::TgcClusterOnTrack*>(*itR);
            const Muon::TgcPrepData*       tgcPRD = tgc ? tgc->prepRawData() : nullptr;
            if (tgcPRD) {
              m_trkTriggerHit_time.push_back(tgcPRD->getBcBitMap());
              m_trkTriggerHit_gPos.push_back(tgcPRD->globalPosition());
            }
            else {
              m_trkTriggerHit_time.push_back(0);
              m_trkTriggerHit_gPos.push_back(Amg::Vector3D(0,0,0));
            }
          } // tgc case
          else if (m_MuonIdHelper->isRpc(crot_id))  {
            ATH_MSG_DEBUG("This is RPC competing hits on track!") ;
            m_trkTriggerHit_type.push_back(2);
            const Muon::RpcClusterOnTrack* rpc    = dynamic_cast<const Muon::RpcClusterOnTrack*>(*itR);
            const Muon::RpcPrepData*       rpcPRD = rpc ? rpc->prepRawData() : nullptr;
            if (rpcPRD) {
              m_trkTriggerHit_time.push_back(rpcPRD->time());
              m_trkTriggerHit_gPos.push_back(rpcPRD->globalPosition());
            }
            else    {
            m_trkTriggerHit_time.push_back(0);
            m_trkTriggerHit_gPos.push_back(Amg::Vector3D(0,0,0));
            } // rpc case
          } // rpc case
          else {
            m_trkTriggerHit_type.push_back(0);
            m_trkTriggerHit_time.push_back(0);
            m_trkTriggerHit_gPos.push_back(Amg::Vector3D(0,0,0));
          }
        } // end of crots loop
      }  // end of crots size check
    }  // end of crot check
  }  // end of CompetingROT
}  // end of store measurement 

StatusCode MuonSegmentReader::finalize()
{
  ATH_MSG_INFO("MuonSegmentReader :: Finalize + Matching");
  ATH_CHECK(m_tree.write());
  return StatusCode::SUCCESS;
}

} // namespace MuonCalib
