/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <stdexcept>

// Gaudi/Athena include(s):
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ThreadLocalContext.h"

// EDM include(s):
#include "xAODTau/versions/TauJetContainer_v2.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauTrackContainer.h"
#include "xAODTau/TauTrackAuxContainer.h"
#include "xAODTracking/TrackParticleContainer.h"

// Local include(s):
#include "xAODTauJetAuxContainerCnv_v2.h"

// StoreGateSvc
#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/ServiceHandle.h"


xAODTauJetAuxContainerCnv_v2::xAODTauJetAuxContainerCnv_v2()
{
}


void xAODTauJetAuxContainerCnv_v2::
persToTransWithKey( const xAOD::TauJetAuxContainer_v2* oldObj,
                    xAOD::TauJetAuxContainer* newObj,
                    const std::string& key,
                    MsgStream& log ) const {

  ServiceHandle<StoreGateSvc> evtStore ("StoreGateSvc", "tauJetCnv_v2");
  if (evtStore.retrieve().isFailure()) {
    log << MSG::ERROR << "Cannot get StoreGateHandle" << endmsg;
    return;
  }

   std::string tauTrackContName=key;
   if (tauTrackContName.ends_with ("Aux.")) {
     tauTrackContName.resize (tauTrackContName.size()-4);
   }

   //example names:
   //TauJets : Jets --> Tracks
   //HLT_xAOD__TauJetContainer_TrigTauRecMerged Jet --> Track; +=Tracks
   //HLT_xAOD__TauJetContainer_TrigTauRecPreselection ""
   if(tauTrackContName.find("Jet") != std::string::npos){
     tauTrackContName.replace( tauTrackContName.find("Jet"), 3, "Track" );
     if(tauTrackContName.find("HLT") != std::string::npos) tauTrackContName+="Tracks";
   }
   else if (key.length() > 0) {
     log << MSG::ERROR << "Cannot decipher name TauTrackContainer should have" << endmsg;
     return;
   }

   std::string tauTrackAuxContName=tauTrackContName+"Aux.";

   xAOD::TauTrackContainer* pTracks = nullptr;
   xAOD::TauTrackAuxContainer* pAuxTracks = nullptr; 
   if(key.length()){
     //if reading data, then trigger calls T/P converter directly
     //and the key is not set.  In this case, forget about TauTracks
     pTracks = new xAOD::TauTrackContainer();
     pAuxTracks = new xAOD::TauTrackAuxContainer();
     pTracks->setStore(pAuxTracks);

     if(evtStore->record(pTracks, tauTrackContName).isFailure() ||
        evtStore->record(pAuxTracks, tauTrackAuxContName).isFailure()){
       log << MSG::DEBUG << "Couldn't Record TauTracks" << endmsg;
       return;
     }
   }

   // Clear the transient object:
   newObj->resize( 0 );

   // Set up interface containers on top of them:
   xAOD::TauJetContainer_v2 oldInt;
   for( size_t i = 0; i < oldObj->size(); ++i ) {
     oldInt.push_back( new xAOD::TauJet_v2() );
   }
   oldInt.setStore( oldObj );
   xAOD::TauJetContainer newInt;
   newInt.setStore( newObj );

   // Loop over the interface objects, and do the conversion with their help:
   for( const xAOD::TauJet_v2* oldTau : oldInt ) {

      // Add an object to the output container:
      xAOD::TauJet* newTau = new xAOD::TauJet;
      newInt.push_back( newTau );
      
      //
      // Copy the 4-momentum variables:
      //
      newTau->setP4( oldTau->pt(), oldTau->eta(), oldTau->phi(), oldTau->m() );
      newTau->setP4( xAOD::TauJetParameters::JetSeed, oldTau->ptJetSeed(), oldTau->etaJetSeed(), oldTau->phiJetSeed(), oldTau->mJetSeed() );
      newTau->setP4( xAOD::TauJetParameters::DetectorAxis, oldTau->ptDetectorAxis(), oldTau->etaDetectorAxis(), oldTau->phiDetectorAxis(), oldTau->mDetectorAxis() );
      newTau->setP4( xAOD::TauJetParameters::IntermediateAxis, oldTau->ptIntermediateAxis(), oldTau->etaIntermediateAxis(), oldTau->phiIntermediateAxis(), oldTau->mIntermediateAxis() );
      newTau->setP4( xAOD::TauJetParameters::TauEnergyScale, oldTau->ptTauEnergyScale(), oldTau->etaTauEnergyScale(), oldTau->phiTauEnergyScale(), oldTau->mTauEnergyScale() );
      newTau->setP4( xAOD::TauJetParameters::TauEtaCalib, oldTau->ptTauEtaCalib(), oldTau->etaTauEtaCalib(), oldTau->phiTauEtaCalib(), oldTau->mTauEtaCalib() );
      newTau->setP4( xAOD::TauJetParameters::PanTauCellBased, oldTau->ptPanTauCellBased(), oldTau->etaPanTauCellBased(), oldTau->phiPanTauCellBased(), oldTau->mPanTauCellBased() );
      newTau->setP4( xAOD::TauJetParameters::PanTauCellBasedProto, oldTau->ptPanTauCellBasedProto(), oldTau->etaPanTauCellBasedProto(), oldTau->phiPanTauCellBasedProto(), oldTau->mPanTauCellBasedProto() );

      // ROI and charge
      newTau->setROIWord( oldTau->ROIWord() );
      newTau->setCharge( oldTau->charge() );
      newTau->setTrackFilterProngs( oldTau->trackFilterProngs() );
      newTau->setTrackFilterQuality( oldTau->trackFilterQuality() );
      newTau->setPi0ConeDR( oldTau->pi0ConeDR() );

      //
      //copy PID variables
      //
      newTau->setDiscriminant(xAOD::TauJetParameters::BDTJetScoreSigTrans    , oldTau->discriminant(xAOD::TauJetParameters::BDTJetScoreSigTrans) );
      newTau->setDiscriminant(xAOD::TauJetParameters::BDTJetScore    , oldTau->discriminant(xAOD::TauJetParameters::BDTJetScore) );
      newTau->setDiscriminant(xAOD::TauJetParameters::BDTEleScore    , oldTau->discriminant(xAOD::TauJetParameters::BDTEleScore) );
      newTau->setIsTau(xAOD::TauJetParameters::JetBDTSigLoose     ,   oldTau->isTau(xAOD::TauJetParameters::JetBDTSigLoose) );
      newTau->setIsTau(xAOD::TauJetParameters::JetBDTSigMedium    ,   oldTau->isTau(xAOD::TauJetParameters::JetBDTSigMedium) );
      newTau->setIsTau(xAOD::TauJetParameters::JetBDTSigTight     ,   oldTau->isTau(xAOD::TauJetParameters::JetBDTSigTight) );
      newTau->setIsTau(xAOD::TauJetParameters::EleBDTLoose        ,   oldTau->isTau(xAOD::TauJetParameters::EleBDTLoose) );
      newTau->setIsTau(xAOD::TauJetParameters::EleBDTMedium       ,   oldTau->isTau(xAOD::TauJetParameters::EleBDTMedium) );
      newTau->setIsTau(xAOD::TauJetParameters::EleBDTTight        ,   oldTau->isTau(xAOD::TauJetParameters::EleBDTTight) );         

      //
      //set individual int type details variables
      //
      newTau->setDetail(xAOD::TauJetParameters::nCharged                   , oldTau->detail<int>(xAOD::TauJetParameters::nCharged) );                     
      newTau->setDetail(xAOD::TauJetParameters::numCells               ,      oldTau->detail<int>(xAOD::TauJetParameters::numCells) );				  
      newTau->setDetail(xAOD::TauJetParameters::numTopoClusters        ,      oldTau->detail<int>(xAOD::TauJetParameters::numTopoClusters) );			  
      newTau->setDetail(xAOD::TauJetParameters::nStrip                 ,      oldTau->detail<int>(xAOD::TauJetParameters::nStrip) );				  

      //
      //set individual float type details variables
      //
      newTau->setDetail(xAOD::TauJetParameters::ipZ0SinThetaSigLeadTrk ,       oldTau->detail<float>(xAOD::TauJetParameters::ipZ0SinThetaSigLeadTrk) );			  
      newTau->setDetail(xAOD::TauJetParameters::etOverPtLeadTrk        ,       oldTau->detail<float>(xAOD::TauJetParameters::etOverPtLeadTrk) );				  
      newTau->setDetail(xAOD::TauJetParameters::leadTrkPt              ,       oldTau->detail<float>(xAOD::TauJetParameters::leadTrkPt) );				  
      newTau->setDetail(xAOD::TauJetParameters::ipSigLeadTrk           ,       oldTau->detail<float>(xAOD::TauJetParameters::ipSigLeadTrk) );				  
      newTau->setDetail(xAOD::TauJetParameters::massTrkSys             ,       oldTau->detail<float>(xAOD::TauJetParameters::massTrkSys) );				  
      newTau->setDetail(xAOD::TauJetParameters::trkWidth2              ,       oldTau->detail<float>(xAOD::TauJetParameters::trkWidth2) );				  
      newTau->setDetail(xAOD::TauJetParameters::trFlightPathSig        ,       oldTau->detail<float>(xAOD::TauJetParameters::trFlightPathSig) );				  
      newTau->setDetail(xAOD::TauJetParameters::numEffTopoClusters     ,       oldTau->detail<float>(xAOD::TauJetParameters::numEffTopoClusters) );			  
      newTau->setDetail(xAOD::TauJetParameters::topoInvMass            ,       oldTau->detail<float>(xAOD::TauJetParameters::topoInvMass) );				  
      newTau->setDetail(xAOD::TauJetParameters::effTopoInvMass         ,       oldTau->detail<float>(xAOD::TauJetParameters::effTopoInvMass) );				  
      newTau->setDetail(xAOD::TauJetParameters::topoMeanDeltaR         ,       oldTau->detail<float>(xAOD::TauJetParameters::topoMeanDeltaR) );				  
      newTau->setDetail(xAOD::TauJetParameters::effTopoMeanDeltaR      ,       oldTau->detail<float>(xAOD::TauJetParameters::effTopoMeanDeltaR) );			  
      newTau->setDetail(xAOD::TauJetParameters::EMRadius               ,       oldTau->detail<float>(xAOD::TauJetParameters::EMRadius) );			  
      newTau->setDetail(xAOD::TauJetParameters::hadRadius              ,       oldTau->detail<float>(xAOD::TauJetParameters::hadRadius) );			  
      newTau->setDetail(xAOD::TauJetParameters::etEMAtEMScale          ,       oldTau->detail<float>(xAOD::TauJetParameters::etEMAtEMScale) );			  
      newTau->setDetail(xAOD::TauJetParameters::etHadAtEMScale         ,       oldTau->detail<float>(xAOD::TauJetParameters::etHadAtEMScale) );			  
      newTau->setDetail(xAOD::TauJetParameters::isolFrac               ,       oldTau->detail<float>(xAOD::TauJetParameters::isolFrac) );			  
      newTau->setDetail(xAOD::TauJetParameters::centFrac               ,       oldTau->detail<float>(xAOD::TauJetParameters::centFrac) );			  
      newTau->setDetail(xAOD::TauJetParameters::stripWidth2            ,       oldTau->detail<float>(xAOD::TauJetParameters::stripWidth2) );			  
      newTau->setDetail(xAOD::TauJetParameters::trkAvgDist             ,       oldTau->detail<float>(xAOD::TauJetParameters::trkAvgDist) );			  
      newTau->setDetail(xAOD::TauJetParameters::trkRmsDist             ,       oldTau->detail<float>(xAOD::TauJetParameters::trkRmsDist) );			  
      newTau->setDetail(xAOD::TauJetParameters::lead2ClusterEOverAllClusterE , oldTau->detail<float>(xAOD::TauJetParameters::lead2ClusterEOverAllClusterE) );	  
      newTau->setDetail(xAOD::TauJetParameters::lead3ClusterEOverAllClusterE , oldTau->detail<float>(xAOD::TauJetParameters::lead3ClusterEOverAllClusterE) );	  
      newTau->setDetail(xAOD::TauJetParameters::caloIso ,	               oldTau->detail<float>(xAOD::TauJetParameters::caloIso) );			  
      newTau->setDetail(xAOD::TauJetParameters::caloIsoCorrected ,             oldTau->detail<float>(xAOD::TauJetParameters::caloIsoCorrected) );		  
      newTau->setDetail(xAOD::TauJetParameters::dRmax ,	                       oldTau->detail<float>(xAOD::TauJetParameters::dRmax) );				  
      newTau->setDetail(xAOD::TauJetParameters::secMaxStripEt  ,	       oldTau->detail<float>(xAOD::TauJetParameters::secMaxStripEt) );				  
      newTau->setDetail(xAOD::TauJetParameters::sumEMCellEtOverLeadTrkPt  ,    oldTau->detail<float>(xAOD::TauJetParameters::sumEMCellEtOverLeadTrkPt) );				  
      newTau->setDetail(xAOD::TauJetParameters::hadLeakEt  ,	               oldTau->detail<float>(xAOD::TauJetParameters::hadLeakEt) );				  
      newTau->setDetail(xAOD::TauJetParameters::cellBasedEnergyRing1 ,	       oldTau->detail<float>(xAOD::TauJetParameters::cellBasedEnergyRing1) );			  
      newTau->setDetail(xAOD::TauJetParameters::cellBasedEnergyRing2 ,	       oldTau->detail<float>(xAOD::TauJetParameters::cellBasedEnergyRing2) );			  
      newTau->setDetail(xAOD::TauJetParameters::cellBasedEnergyRing3 ,	       oldTau->detail<float>(xAOD::TauJetParameters::cellBasedEnergyRing3) );			  
      newTau->setDetail(xAOD::TauJetParameters::cellBasedEnergyRing4 ,	       oldTau->detail<float>(xAOD::TauJetParameters::cellBasedEnergyRing4) );			  
      newTau->setDetail(xAOD::TauJetParameters::cellBasedEnergyRing5 ,	       oldTau->detail<float>(xAOD::TauJetParameters::cellBasedEnergyRing5) );			  
      newTau->setDetail(xAOD::TauJetParameters::cellBasedEnergyRing6 ,	       oldTau->detail<float>(xAOD::TauJetParameters::cellBasedEnergyRing6) );			  
      newTau->setDetail(xAOD::TauJetParameters::cellBasedEnergyRing7 ,	       oldTau->detail<float>(xAOD::TauJetParameters::cellBasedEnergyRing7) );			  
      newTau->setDetail(xAOD::TauJetParameters::TRT_NHT_OVER_NLT ,	       oldTau->detail<float>(xAOD::TauJetParameters::TRT_NHT_OVER_NLT) );			  
      newTau->setDetail(xAOD::TauJetParameters::TauJetVtxFraction ,	       oldTau->detail<float>(xAOD::TauJetParameters::TauJetVtxFraction) );			  
      newTau->setDetail(xAOD::TauJetParameters::TauJetVtxFraction ,	       oldTau->detail<float>(xAOD::TauJetParameters::TauJetVtxFraction) );			  
      newTau->setDetail(xAOD::TauJetParameters::PSSFraction ,	               oldTau->detail<float>(xAOD::TauJetParameters::PSSFraction) );			  
      newTau->setDetail(xAOD::TauJetParameters::ChPiEMEOverCaloEME ,	       oldTau->detail<float>(xAOD::TauJetParameters::ChPiEMEOverCaloEME) );			  
      newTau->setDetail(xAOD::TauJetParameters::EMPOverTrkSysP ,	       oldTau->detail<float>(xAOD::TauJetParameters::EMPOverTrkSysP) );			  
      newTau->setDetail(xAOD::TauJetParameters::TESOffset ,	               oldTau->detail<float>(xAOD::TauJetParameters::TESOffset) );			  
      newTau->setDetail(xAOD::TauJetParameters::TESCalibConstant ,	       oldTau->detail<float>(xAOD::TauJetParameters::TESCalibConstant) );			  
      newTau->setDetail(xAOD::TauJetParameters::centFracCorrected ,	       oldTau->detail<float>(xAOD::TauJetParameters::centFracCorrected) );			  
      newTau->setDetail(xAOD::TauJetParameters::etOverPtLeadTrkCorrected ,     oldTau->detail<float>(xAOD::TauJetParameters::etOverPtLeadTrkCorrected) );			  
      newTau->setDetail(xAOD::TauJetParameters::innerTrkAvgDist ,	       oldTau->detail<float>(xAOD::TauJetParameters::innerTrkAvgDist) );			  
      newTau->setDetail(xAOD::TauJetParameters::innerTrkAvgDistCorrected ,     oldTau->detail<float>(xAOD::TauJetParameters::innerTrkAvgDistCorrected) );			  
      newTau->setDetail(xAOD::TauJetParameters::SumPtTrkFrac ,	       oldTau->detail<float>(xAOD::TauJetParameters::SumPtTrkFrac) );			  
      newTau->setDetail(xAOD::TauJetParameters::SumPtTrkFracCorrected ,	       oldTau->detail<float>(xAOD::TauJetParameters::SumPtTrkFracCorrected) );			  
 

      //
      //set pantau details
      //
      newTau->setPanTauDetail(xAOD::TauJetParameters::PanTau_isPanTauCandidate,                           oldTau->panTauDetail<int>(xAOD::TauJetParameters::PanTau_isPanTauCandidate ));
      newTau->setPanTauDetail(xAOD::TauJetParameters::PanTau_DecayModeProto,                              oldTau->panTauDetail<int>(xAOD::TauJetParameters::PanTau_DecayModeProto ));	
      newTau->setPanTauDetail(xAOD::TauJetParameters::PanTau_DecayMode, 	                                oldTau->panTauDetail<int>(xAOD::TauJetParameters::PanTau_DecayMode ));		 
      newTau->setPanTauDetail(xAOD::TauJetParameters::PanTau_BDTValue_1p0n_vs_1p1n,                       oldTau->panTauDetail<float>(xAOD::TauJetParameters::PanTau_BDTValue_1p0n_vs_1p1n ));		  
      newTau->setPanTauDetail(xAOD::TauJetParameters::PanTau_BDTValue_1p1n_vs_1pXn, 			oldTau->panTauDetail<float>(xAOD::TauJetParameters::PanTau_BDTValue_1p1n_vs_1pXn ));	  
      newTau->setPanTauDetail(xAOD::TauJetParameters::PanTau_BDTValue_3p0n_vs_3pXn, 			oldTau->panTauDetail<float>(xAOD::TauJetParameters::PanTau_BDTValue_3p0n_vs_3pXn ));		  
      newTau->setPanTauDetail(xAOD::TauJetParameters::PanTau_BDTVar_Charged_StdDev_Et_WrtEtAllConsts, 	oldTau->panTauDetail<float>(xAOD::TauJetParameters::PanTau_BDTVar_Charged_StdDev_Et_WrtEtAllConsts )); 
      newTau->setPanTauDetail(xAOD::TauJetParameters::PanTau_BDTVar_Neutral_PID_BDTValues_BDTSort_1, 	oldTau->panTauDetail<float>(xAOD::TauJetParameters::PanTau_BDTVar_Neutral_PID_BDTValues_BDTSort_1 ));  
      newTau->setPanTauDetail(xAOD::TauJetParameters::PanTau_BDTVar_Neutral_PID_BDTValues_BDTSort_2, 	oldTau->panTauDetail<float>(xAOD::TauJetParameters::PanTau_BDTVar_Neutral_PID_BDTValues_BDTSort_2 ));  
      newTau->setPanTauDetail(xAOD::TauJetParameters::PanTau_BDTVar_Neutral_Ratio_1stBDTEtOverEtAllConsts,oldTau->panTauDetail<float>(xAOD::TauJetParameters::PanTau_BDTVar_Neutral_Ratio_1stBDTEtOverEtAllConsts )); 
      newTau->setPanTauDetail(xAOD::TauJetParameters::PanTau_BDTVar_Neutral_Ratio_EtOverEtAllConsts, 	oldTau->panTauDetail<float>(xAOD::TauJetParameters::PanTau_BDTVar_Neutral_Ratio_EtOverEtAllConsts ));  
      newTau->setPanTauDetail(xAOD::TauJetParameters::PanTau_BDTVar_Combined_DeltaR1stNeutralTo1stCharged,oldTau->panTauDetail<float>(xAOD::TauJetParameters::PanTau_BDTVar_Combined_DeltaR1stNeutralTo1stCharged ));
      
      //copy element links
      newTau->setJetLink( oldTau->jetLink() );
      newTau->setVertexLink( oldTau->vertexLink() );
      newTau->setSecondaryVertexLink( oldTau->secondaryVertexLink() );

      newTau->setNeutralPFOLinks( oldTau->neutralPFOLinks() );
      newTau->setChargedPFOLinks( oldTau->chargedPFOLinks() );
      newTau->setPi0PFOLinks( oldTau->pi0PFOLinks() );
      newTau->setShotPFOLinks( oldTau->shotPFOLinks() );

      //v2 doesn't have pfo element link with specific type name, so copy cellbased ones into proto
      newTau->setProtoNeutralPFOLinks( oldTau->protoNeutralPFOLinks() );
      newTau->setProtoChargedPFOLinks( oldTau->protoChargedPFOLinks() );
      newTau->setProtoPi0PFOLinks( oldTau->protoPi0PFOLinks() );

      if(key.length()==0) continue;

      // Get context and look up hashed track container key.
      const EventContext& ctx = Gaudi::Hive::currentContext();
      ElementLink<xAOD::TauTrackContainer> dum (tauTrackContName, 0, ctx);
      SG::sgkey_t track_sgkey = dum.key();
      IProxyDict* sg = dum.source();

      for(unsigned int i = 0; i < oldTau->nTracks(); ++i){
	ElementLink< xAOD::TrackParticleContainer > linkToTrackParticle = oldTau->trackLinks()[i];
        linkToTrackParticle.toTransient (sg);
	if(!linkToTrackParticle.isValid()) continue;
	newTau->addTauTrackLink(ElementLink<xAOD::TauTrackContainer>(track_sgkey, pTracks->size(), ctx));
	pTracks->push_back(std::make_unique<xAOD::TauTrack>());
	xAOD::TauTrack& track = *pTracks->back();
	const xAOD::TrackParticle* trackParticle=*linkToTrackParticle;
        track.addTrackLink(linkToTrackParticle);
        track.setP4(trackParticle->pt(), trackParticle->eta(), trackParticle->phi(), trackParticle->m());
	track.setFlag(xAOD::TauJetParameters::TauTrackFlag::coreTrack, true);
        track.setFlag(xAOD::TauJetParameters::TauTrackFlag::passTrkSelector, true);
        track.setFlag(xAOD::TauJetParameters::TauTrackFlag::classifiedCharged, true);
        track.setFlag(xAOD::TauJetParameters::TauTrackFlag::unclassified, true);
        //ElementLink<xAOD::TauTrackContainer> linkToTauTrack;
        //linkToTauTrack.resetWithKeyAndIndex(track_sgkey, pTracks->size()-1, ctx);
        //newTau->addTauTrackLink(linkToTauTrack);
      }

      for(unsigned int i = 0; i < oldTau->nWideTracks(); ++i){
	ElementLink< xAOD::TrackParticleContainer > linkToTrackParticle = oldTau->wideTrackLinks()[i];
        linkToTrackParticle.toTransient (sg);
	if(!linkToTrackParticle.isValid()) continue;
	newTau->addTauTrackLink(ElementLink<xAOD::TauTrackContainer>(track_sgkey, pTracks->size(), ctx));
	pTracks->push_back(std::make_unique<xAOD::TauTrack>());
	xAOD::TauTrack& track = *pTracks->back();
	const xAOD::TrackParticle* trackParticle=*linkToTrackParticle;
        track.addTrackLink(linkToTrackParticle);
        track.setP4(trackParticle->pt(), trackParticle->eta(), trackParticle->phi(), trackParticle->m());
	track.setFlag(xAOD::TauJetParameters::TauTrackFlag::wideTrack, true);
        track.setFlag(xAOD::TauJetParameters::TauTrackFlag::passTrkSelector, true);
        track.setFlag(xAOD::TauJetParameters::TauTrackFlag::classifiedIsolation, true);
        track.setFlag(xAOD::TauJetParameters::TauTrackFlag::modifiedIsolationTrack, true);
        track.setFlag(xAOD::TauJetParameters::TauTrackFlag::unclassified, true);
      }

      for(unsigned int i = 0; i < oldTau->nOtherTracks(); ++i){
	ElementLink< xAOD::TrackParticleContainer > linkToTrackParticle = oldTau->otherTrackLinks()[i];
        linkToTrackParticle.toTransient (sg);
	if(!linkToTrackParticle.isValid()) continue;
	newTau->addTauTrackLink(ElementLink<xAOD::TauTrackContainer>(track_sgkey, pTracks->size(), ctx));
	pTracks->push_back(std::make_unique<xAOD::TauTrack>());
	xAOD::TauTrack& track = *pTracks->back();
	const xAOD::TrackParticle* trackParticle=*linkToTrackParticle;
        track.addTrackLink(linkToTrackParticle);
        track.setP4(trackParticle->pt(), trackParticle->eta(), trackParticle->phi(), trackParticle->m());
	float dR=oldTau->p4(xAOD::TauJetParameters::IntermediateAxis).DeltaR(trackParticle->p4());
	if(dR<=0.2) track.setFlag(xAOD::TauJetParameters::TauTrackFlag::coreTrack, true);
	else track.setFlag(xAOD::TauJetParameters::TauTrackFlag::wideTrack, true);
	track.setFlag(xAOD::TauJetParameters::TauTrackFlag::unclassified, true);
      }


      newTau->setDetail(xAOD::TauJetParameters::nChargedTracks, (int) newTau->nTracks());
      newTau->setDetail(xAOD::TauJetParameters::nIsolatedTracks, (int) newTau->nTracks(xAOD::TauJetParameters::classifiedIsolation));

   }

   return;
}

/// This function should never be called, as we are not supposed to convert
/// object before writing.
///
void xAODTauJetAuxContainerCnv_v2::
transToPersWithKey( const xAOD::TauJetAuxContainer*,
                    xAOD::TauJetAuxContainer_v2*,
                    const std::string& /*key*/,
                    MsgStream& log ) const {

   log << MSG::ERROR
       << "Somebody called xAODTauJetAuxContainerCnv_v2::transToPers"
       << endmsg;
   throw std::runtime_error( "Somebody called xAODTauJetAuxContainerCnv_v2::"
                             "transToPers" );

   return;
}

