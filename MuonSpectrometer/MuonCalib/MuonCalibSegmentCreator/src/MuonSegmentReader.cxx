/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
    
#include "MuonCalibSegmentCreator/MuonSegmentReader.h"
#include "GaudiKernel/ITHistSvc.h"
//#include "MuonRIO_OnTrack/CscClusterOnTrack.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "MuonRIO_OnTrack/RpcClusterOnTrack.h"
#include "MuonRIO_OnTrack/TgcClusterOnTrack.h"
#include "TrkTrackSummary/MuonTrackSummary.h"
#include "TrkTrackSummary/TrackSummary.h"




namespace MuonCalib {

StatusCode MuonSegmentReader::initialize()
{

  ATH_CHECK(m_evtKey.initialize() );
  //ATH_CHECK(m_TrkSegKey.initialize());
  ATH_CHECK(m_TrkKey.initialize());
  //ATH_CHECK(m_CbTrkKey.initialize());
  //ATH_CHECK(m_EMEO_TrkKey.initialize());  // run3 only EM EO MS tracks
  ATH_CHECK(m_MdtPrepData.initialize());
  ATH_CHECK(m_calibrationTool.retrieve());
  ATH_MSG_VERBOSE("MdtCalibrationTool retrieved with pointer = " << m_calibrationTool);

  ATH_CHECK(m_DetectorManagerKey.initialize());
  ATH_CHECK(m_idToFixedIdTool.retrieve());
  ATH_CHECK(m_pullCalculator.retrieve());
  // ATH_CHECK(m_assocTool.retrieve());
  // ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_MuonIdHelper.retrieve());
  //ATH_CHECK(histSvc.retrieve() );
  
  ATH_CHECK(m_tree.init(this));
  
  return StatusCode::SUCCESS;
}

StatusCode MuonSegmentReader::execute()
{

  // eventInfo
  //const xAOD::EventInfo* eventInfo = 0;
  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_evtKey,ctx);
  if (!eventInfo.isValid()) {
    ATH_MSG_ERROR("Did not find xAOD::EventInfo");
    return StatusCode::FAILURE;
  }
  
  // load Geo
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> DetectorManagerHandle{m_DetectorManagerKey, ctx};
  const MuonGM::MuonDetectorManager* MuonDetMgr = DetectorManagerHandle.cptr();
  if (!MuonDetMgr) {
      ATH_MSG_ERROR("Null pointer to the read MuonDetectorManager conditions object");
      return StatusCode::FAILURE;
  }

  // fill the rawMdt Hit branches
  SG::ReadHandle<Muon::MdtPrepDataContainer> mdtPrepRawData(m_MdtPrepData, ctx);
  ATH_MSG_INFO("Number of MDT raw Hits : "<<mdtPrepRawData->size());

  Muon::MdtPrepDataContainer::const_iterator mdtColl = mdtPrepRawData->begin();
  Muon::MdtPrepDataContainer::const_iterator last_mdtColl = mdtPrepRawData->end();
  unsigned int i_coll(0);
   // if raw MDT hits were found, print MDT track hits
  for( ; mdtColl!=last_mdtColl ; ++mdtColl, ++i_coll ){
    if (mdtColl->size() > 0) {
      for (Muon::MdtPrepDataCollection::const_iterator mdtPrd = mdtColl->begin(); mdtPrd != mdtColl->end(); ++mdtPrd) {
          const Muon::MdtPrepData* prd = (*mdtPrd);
          MuonFixedId fixid = m_idToFixedIdTool->idToFixedId(prd->identify());
          m_rawMdt_id.push_back(fixid.getIdInt()) ;
          //m_rawMdt_id = (*mdtPrd)->identify();
          m_rawMdt_tdc.push_back(prd->tdc());
          m_rawMdt_adc.push_back(prd->adc());
          m_rawMdt_gPos.push_back(prd->globalPosition());
                            }
  }  // end of MdtPrepDataCollection                             
  } // end of MdtPrepDataContainer
  m_rawMdt_nRMdt = m_rawMdt_adc.size() ;

  // fill the muon standalone tracks 
  SG::ReadHandle<TrackCollection> muTrks(m_TrkKey, ctx);
  ATH_MSG_INFO("Number of Muon StandAlone Tracks : "<<muTrks->size());
  //if(!muTrks->size()) continue;  

  m_trk_nTracks = muTrks->size() ;

  // only fill the branches after the nTracks
    //m_nEvent   = eventInfo->nEvent();
  m_runNumber   = eventInfo->runNumber();
  m_eventNumber = eventInfo->eventNumber();
  m_lumiBlock    = eventInfo->lumiBlock();
  m_bcId        = eventInfo->bcid();
  m_timeStamp = eventInfo->timeStamp();
  m_pt = eventInfo->timeStampNSOffset();
  //m_eventTag = eventInfo->eventTag();


  // if tracks were found, print MDT track hits
  for (unsigned int itrk = 0; itrk < muTrks->size(); ++itrk) {
    
    const Trk::Track* trkSA = muTrks->at(itrk) ;

    const DataVector<const Trk::TrackStateOnSurface>* trk_states = trkSA->trackStateOnSurfaces();
    if (!trk_states) {
            ATH_MSG_WARNING(" track without states, discarding track ");
            return StatusCode::FAILURE;
        }

    // load trk branches
    const Trk::FitQuality* fq = trkSA->fitQuality();        
    if (!fq)  {            
      ATH_MSG_WARNING(" was expecting a FitQuality here... ");            
      return StatusCode::FAILURE;        
              }

    // trackSummary
    const Trk::MuonTrackSummary* summary = nullptr;
    // check if the track already has a MuonTrackSummary, if not calculate it using the helper
    const Trk::TrackSummary* trkSummary = trkSA->trackSummary();
    m_trk_nMdtHits.push_back(trkSummary->get(Trk::numberOfMdtHits));
    m_trk_nMdtGoodHits.push_back(trkSummary->get(Trk::numberOfGoodMdtHits));
    //!< number of measurements flaged as outliers in TSOS
    m_trk_nOutliersHits.push_back(trkSummary->get(Trk::numberOfOutliersOnTrack));
    m_trk_nRpcPhiHits.push_back(trkSummary->get(Trk::numberOfRpcPhiHits));
    m_trk_nTgcPhiHits.push_back(trkSummary->get(Trk::numberOfTgcPhiHits));
    m_trk_nRpcEtaHits.push_back(trkSummary->get(Trk::numberOfRpcEtaHits));
    m_trk_nTgcEtaHits.push_back(trkSummary->get(Trk::numberOfTgcEtaHits));
    m_trk_nMdtHoles.push_back(trkSummary->get(Trk::numberOfMdtHoles));
    ATH_MSG_DEBUG("Mdt Hits " << trkSummary->get(Trk::numberOfMdtHits) << " Mdt Good Hits " << trkSummary->get(Trk::numberOfGoodMdtHits) 
                  <<" Mdt Holes "<< trkSummary->get(Trk::numberOfMdtHoles) 
                  << "  outliers " << trkSummary->get(Trk::numberOfOutliersOnTrack) 
                  << " TGC Phi Eta Hits "<<trkSummary->get(Trk::numberOfTgcPhiHits)<<" "<<trkSummary->get(Trk::numberOfTgcEtaHits)
                  << " RPC Phi Eta Hits "<<trkSummary->get(Trk::numberOfRpcPhiHits)<<" "<<trkSummary->get(Trk::numberOfRpcEtaHits));

    if (trkSummary) summary = trkSummary->muonTrackSummary();
    if (!summary) {
	  ATH_MSG_WARNING("No muon summary is present");
                      }
    else  {
    ATH_MSG_DEBUG("Hits: eta " << summary->netaHits() << "  phi " << summary->nphiHits() << "  holes " << summary->nholes()
         << "  outliers " << summary->noutliers() << "  pseudo " << summary->npseudoMeasurements() << "  scatterers "
         << summary->nscatterers() << " close Hits " << summary->ncloseHits());
          }


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
    m_trk_eta.push_back(perigee->eta());
    m_trk_qOverP.push_back(perigee->parameters()[Trk::qOverP]);

    m_trk_chi2.push_back(fq->chiSquared()) ;
    m_trk_ndof.push_back(fq->numberDoF());
    
    // track author (MuonstandaloneTrack) 200	Muon combined track 201	Muon ID track 202	Muon MS track
    m_trk_author.push_back(202);

    // get the track pT info by using the first trackstateonsurface
    const Trk::TrackParameters *trk_pars = trk_states->front()->trackParameters();
    m_trk_pt.push_back(trk_pars->momentum().perp() * 1e-3);

    const DataVector<const Trk::MeasurementBase>* trk_mT = trkSA->measurementsOnTrack();

    int ntm(0), ntr(0), ntt(0), ntc(0),ntmm(0), nts(0);
    //int count = 0;
    for (const Trk::TrackStateOnSurface* trk_state : *trk_states) {

        const Trk::TrackParameters *trackPars = trk_state->trackParameters();
        if (!trackPars) continue;
        
        const Trk::MeasurementBase* it = trk_state->measurementOnTrack();
        if (!it) {
            ATH_MSG_DEBUG("No Measurement!");
            continue;
        }

        const Trk::RIO_OnTrack* rot = dynamic_cast<const Trk::RIO_OnTrack*>(it);
        if (!rot) {
            ATH_MSG_DEBUG("No ROT!");
            // use competingRIO from RPC and TGC
            continue;
        }

        if (rot->identify() != 0){
            Identifier rot_id = rot->identify();
            if (m_MuonIdHelper->isMdt(rot_id))  {
              //ATH_MSG_INFO("MDT Track Hit, ID "<<rot_id);
              ntm++;
              const Muon::MdtDriftCircleOnTrack* mrot = dynamic_cast<const Muon::MdtDriftCircleOnTrack*>(rot);
                  if (!mrot) {
                      ATH_MSG_DEBUG("This is not a  MdtDriftCircleOnTrack!!! ");
                      continue;
                  }
                  //      mdtSegment = true;
                  int hitType = 0;
                  if (trk_state->type(Trk::TrackStateOnSurface::Measurement)) {
                    hitType = 1;
                  }
                  if (trk_state->type(Trk::TrackStateOnSurface::Outlier)) {
                    hitType = 5;
                  }
                  else if (trk_state->type(Trk::TrackStateOnSurface::Hole)) {
                    hitType = 6;
                  }
                  m_trkHit_type.push_back(hitType);
                  // get digit from segment
                  const Muon::MdtPrepData* prd = mrot->prepRawData();

                  // digit identifier
                  Identifier id = prd->identify();
                  int adc = prd->adc();
                  int tdc = prd->tdc();
                  Amg::Vector3D prd_pos = prd->globalPosition();
                  ATH_MSG_DEBUG("PRD ADC "<<adc<<" PRD TDC "<<tdc<<" PRD gPos "<<prd_pos);
                  // add the implement of calibrationTool 
                  MdtCalibInput calibIn{*prd};
                  calibIn.setTrackDirection(trackPars->momentum().unit());
                  const MdtCalibOutput calibResult{m_calibrationTool->calibrate(ctx, calibIn, false)};
                  ATH_MSG_DEBUG("print "<<calibIn  << " calibResult : "<<calibResult);
                  m_trkHit_tubeT0.push_back(calibResult.tubeT0());
                  m_trkHit_tubeMeanAdc.push_back(calibResult.meanAdc());
                  m_trkHit_lorTime.push_back(calibResult.lorentzTime());
                  m_trkHit_slewTime.push_back(calibResult.slewingTime());
                  m_trkHit_propTime.push_back(calibResult.signalPropagationTime());
                  m_trkHit_sagTime.push_back(calibResult.saggingTime());
                  m_trkHit_tempTime.push_back(calibResult.temperatureTime());
                  m_trkHit_bkgTime.push_back(calibResult.backgroundTime());
                  m_trkHit_tof.push_back(calibIn.timeOfFlight());
                  m_trkHit_triggerTime.push_back(calibIn.triggerTime());
                  m_trkHit_calibStatus.push_back(calibResult.status());


                  m_trkHit_adc.push_back(adc) ;
                  m_trkHit_tdc.push_back(tdc) ;
                  m_trkHit_trackIndex.push_back(itrk) ;
                  m_trkHit_driftTime.push_back(mrot->driftTime()) ;
                  //m_trkHit_error.push_back(error) ;
                  
                  float localAngle = mrot->localAngle() ;
                  float driftRadius = mrot->driftRadius() ;
                  //float error = Amg::error(mrot->localCovariance(), Trk::locR)
                  float positionAlongWire = mrot->positionAlongWire();
                  double x = driftRadius*std::sin(localAngle);
                  double y = driftRadius*std::cos(localAngle);
                  Amg::Vector3D locPos(x, y, positionAlongWire);
                  m_trkHit_localAngle.push_back(localAngle);
                  m_trkHit_driftRadius.push_back(driftRadius) ;

                  const MuonGM::MdtReadoutElement* detEl = MuonDetMgr->getMdtReadoutElement(id);
                  if( !detEl ) {
                  ATH_MSG_WARNING( "getGlobalToStation failed to retrieve detEL byebye"  );
                      }

                  // get the 2nd coordinator from the track hit measurement
                  Amg::Vector3D trkHitPos = trackPars->position();
                  Amg::Vector3D trkHitPosLoc = detEl->globalToLocalTransf(id)* trkHitPos;
                  m_trkHit_closestApproach.push_back(trkHitPosLoc);
                  m_trkHit_gClosestApproach.push_back(trkHitPos);

                  // get trkHit local position from measurement 
                  Amg::Vector3D mrot_gPos = mrot->globalPosition();   // equal to Amg::Vector3D mb_pos = it->globalPosition();
                  Amg::Vector3D mrot_pos = detEl->globalToLocalTransf(id)* mrot_gPos;

                  //distion to readout 2nd coordinators 
                  float distRo_det = detEl->distanceFromRO(trkHitPos, id);
                  m_trkHit_distRO.push_back(distRo_det) ;
                  ATH_MSG_DEBUG("trackHit distRO " << distRo_det<<" positionAlongWire "<<mrot->positionAlongWire()<<" tubeLength "<<detEl->tubeLength(id));

                  // save the local postion of hit center for refitting
                  Amg::Vector3D hitCenter = detEl->localTubePos(id);
                  m_trkHit_center.push_back(hitCenter);
                  
                  m_trkHit_gPos.push_back(mrot_gPos);
                  m_trkHit_pos.push_back(mrot_pos);

                  ATH_MSG_DEBUG("detEl tubeLocalCenter " << " x : "<<hitCenter.x()<< " y : " << hitCenter.y()<< " z : "<<hitCenter.z());
                  ATH_MSG_DEBUG("prd hit global position measurement " <<" x : "<< prd_pos.x()<<" y : "<< prd_pos.y()<<" z : "<<prd_pos.z());
                  ATH_MSG_DEBUG("MdtDriftCircleOnTrack hit global position measurement " << " x : " <<mrot_gPos.x()<<" y : "<< mrot_gPos.y() <<" z : "<<mrot_gPos.z());
                  ATH_MSG_DEBUG("trackHitPos from trackPars " << " x : "<<trkHitPos.x() <<" y : "<<trkHitPos.y() <<" z : "<< trkHitPos.z() );
                  ATH_MSG_DEBUG("trackHit Local Pos from trackPars " <<" x : "<< x <<" y : "<< y <<" z : "<< positionAlongWire);
                  ATH_MSG_DEBUG("trackHit Local Pos from globaltolocalCoords " <<" x : "<< trkHitPosLoc.x() <<" y : "<< trkHitPosLoc.y() <<" z : "<< trkHitPosLoc.z());
                  ATH_MSG_DEBUG("mrod Local Pos from globaltolocalCoords " <<" x : "<< mrot_pos.x() <<" y : "<< mrot_pos.y() <<" z : "<< mrot_pos.z());


                  // residual calculator  
                  //const Trk::ResidualPull* resPull = 0;
                  float residualBiased = -999.;
                  float pullBiased = -999.;
                  if( trackPars ) {

                      std::optional<Trk::ResidualPull> resPullBiased = m_pullCalculator->residualPull( it, trackPars, Trk::ResidualPull::Biased );
                      if( resPullBiased.has_value() ){
                        residualBiased = resPullBiased.value().residual().front();
                        pullBiased = resPullBiased.value().pull().front();
                                            }
                                  }
                  // residual definition double residual = measurement->localParameters()[Trk::loc1] - trkPar->parameters()[Trk::loc1];
                  ATH_MSG_DEBUG(" driftRadius from driftRadius func"<<driftRadius<<" rTrk track fit to wire "<<trackPars->parameters()[Trk::loc1]<<" residualBiased " << residualBiased << " pullBiased " << pullBiased);
                  m_trkHit_resi.push_back(residualBiased) ;
                  m_trkHit_pull.push_back(pullBiased) ;
                  m_trkHit_rTrk.push_back(trackPars->parameters()[Trk::loc1]) ;

            //int mdt_segIndex = iseg ;
            
            MuonFixedId fixid = m_idToFixedIdTool->idToFixedId(id);
            m_trkHit_FixedId.push_back(fixid.getIdInt()) ;
            std::string st = fixid.stationNumberToFixedStationString(fixid.stationName());
            int ml = fixid.mdtMultilayer();
            int la = fixid.mdtTubeLayer();
            int tb = fixid.mdtTube();
            ATH_MSG_DEBUG("TrackIndex "<<itrk<<" station " << st << " eta " << fixid.eta() << " phi " << fixid.phi() << " ML " << ml << " Layer " << la
                                      <<" Tube "<<tb<< " MROT drift R " << mrot->driftRadius() << " drift Time "
                                      << mrot->driftTime() <<" ADC "<<adc<<" TDC "<<tdc);
            //ATH_MSG_INFO("TrackIndex "<<itrk<<" MDT Track Hit, ID "<<id<<" ADC "<<adc<<" TDC "<<tdc);

                                  } // finish mdt hit loop
      else if (m_MuonIdHelper->isRpc(rot_id)) {
            ATH_MSG_INFO("ROT ID is RPC "<<rot_id);
            ++ntr;
                                          }
      else if (m_MuonIdHelper->isTgc(rot_id)) {
            ATH_MSG_INFO("ROT ID is TGC "<<rot_id);
            ++ntt;
                                          }
      else if (m_MuonIdHelper->isCsc(rot_id)) {
            ATH_MSG_INFO("ROT ID is CsC "<<rot_id);
            ++ntc;
                                          }
      else if (m_MuonIdHelper->isMM(rot_id)) {
                ATH_MSG_INFO("ROT ID is MM "<<rot_id);
                ++ntmm;
                                          }
      else if (m_MuonIdHelper->issTgc(rot_id)) {
                ATH_MSG_INFO("ROT ID is sTgc "<<rot_id);
                ++nts;
                                          }
      else {
            ATH_MSG_INFO("Couldn't find track ROT ID "<<rot_id);
      }
                                          }// end of id tech check
  //        }// end of id check 
  } // end of track hit loop
  ATH_MSG_DEBUG("Track "<<itrk<<" Total Track Hits : "<<trk_mT->size()<<", nMDT "<<ntm<<", nRPC "<<ntr<<", nTGC "<<ntt<<", nMM "<<ntmm<<", nsTgc "<<nts<<", nCSC "<<ntc) ;

  } // end of track loop
  m_trkHit_nHits = m_trkHit_adc.size() ;

  
  if (!m_tree.fill(ctx)) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}

StatusCode MuonSegmentReader::finalize()
{
  ATH_MSG_INFO("MuonSegmentReader :: Finalize + Matching");
  ATH_CHECK(m_tree.write());
  return StatusCode::SUCCESS;
}


} // namespace MuonCalib
