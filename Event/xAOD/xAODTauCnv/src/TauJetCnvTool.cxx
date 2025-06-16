///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// TauJetCnvTool.cxx 
// Implementation file for class TauJetCnvTool
// Author: Michel Janus janus@cern.ch
/////////////////////////////////////////////////////////////////// 


// xAODJetCnv includes
#include "TauJetCnvTool.h"

// STL includes

// FrameWork includes
#include "Gaudi/Property.h"
#include "AthenaKernel/errorcheck.h"


/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

namespace xAODMaker {
  // Constructors
  ////////////////
  TauJetCnvTool::TauJetCnvTool( const std::string& type,
				const std::string& name, 
				const IInterface* parent ) : 
    ::AthAlgTool( type, name, parent )
    , m_inDetTrackParticles("InDetTrackParticles")
    , m_jets("AntiKt4LCTopoJets")
  {
    declareInterface< ITauJetCnvTool > (this);
    //
    // Property declaration
    // 
    declareProperty( "TrackContainerName", m_inDetTrackParticles );
    declareProperty( "JetContainerName", m_jets );
  }

  // Destructor
  ///////////////
  TauJetCnvTool::~TauJetCnvTool()
  {}

  // Athena Toolorithm's Hooks
  ////////////////////////////
  StatusCode TauJetCnvTool::initialize()
  {
    ATH_MSG_INFO ("Initializing " << name() << "...");

    return StatusCode::SUCCESS;
  }

  StatusCode TauJetCnvTool::convert(const Analysis::TauJetContainer* inputTaus, xAOD::TauJetContainer* xaodTauJets) const
  {  
    Analysis::TauJetContainer::const_iterator it  = inputTaus->begin();
    Analysis::TauJetContainer::const_iterator itE = inputTaus->end();

    for( ; it!= itE; ++it)
      {
	const Analysis::TauJet * tau = *it;

	ATH_MSG_DEBUG( "trying to convert tau with  pt="<< tau->pt() << " eta=" << tau->eta() << " phi=" << tau->phi() << " m=" << tau->m() );    
      
	xAOD::TauJet* xtaujet = new xAOD::TauJet();
	xaodTauJets->push_back( xtaujet );
    
	xtaujet->setP4( tau->pt(), tau->eta(), tau->phi(), tau->m() );

	ATH_MSG_DEBUG( "converted xtau with  pt="<< xtaujet->pt() << " eta=" << xtaujet->eta() << " phi=" << xtaujet->phi() << " m=" << xtaujet->m() );    

	ATH_MSG_DEBUG( "trying to convert tau jetseed 4-vector with  pt="<< tau->getHLV(TauJetParameters::JetSeed).perp() << " eta=" << tau->getHLV(TauJetParameters::JetSeed).eta() << " phi=" << tau->getHLV(TauJetParameters::JetSeed).phi() << " m=" << tau->getHLV(TauJetParameters::JetSeed).m() );    
     

	xtaujet->setP4(xAOD::TauJetParameters::JetSeed, tau->getHLV(TauJetParameters::JetSeed).perp(), tau->getHLV(TauJetParameters::JetSeed).eta(), tau->getHLV(TauJetParameters::JetSeed).phi(), tau->getHLV(TauJetParameters::JetSeed).m());

	ATH_MSG_DEBUG( "converted xtau jetseed 4-vector with  pt="<< xtaujet->ptJetSeed() << " eta=" << xtaujet->etaJetSeed() << " phi=" << xtaujet->phiJetSeed() << " m=" << xtaujet->mJetSeed() );  

	ATH_MSG_DEBUG( "trying to convert tau detectoraxis 4-vector with  pt="<< tau->getHLV(TauJetParameters::DetectorAxis).perp() << " eta=" << tau->getHLV(TauJetParameters::DetectorAxis).eta() << " phi=" << tau->getHLV(TauJetParameters::DetectorAxis).phi() << " m=" << tau->getHLV(TauJetParameters::DetectorAxis).m() );    
     
	xtaujet->setP4(xAOD::TauJetParameters::DetectorAxis, tau->getHLV(TauJetParameters::DetectorAxis).perp(), tau->getHLV(TauJetParameters::DetectorAxis).eta(), tau->getHLV(TauJetParameters::DetectorAxis).phi(), tau->getHLV(TauJetParameters::DetectorAxis).m());

	ATH_MSG_DEBUG( "converted xtau detectoraxis 4-vector with  pt="<< xtaujet->ptDetectorAxis() << " eta=" << xtaujet->etaDetectorAxis() << " phi=" << xtaujet->phiDetectorAxis() << " m=" << xtaujet->mDetectorAxis() );    


	ATH_MSG_DEBUG( "trying to convert tau intermediateaxis 4-vector with  pt="<< tau->getHLV(TauJetParameters::IntermediateAxis).perp() << " eta=" << tau->getHLV(TauJetParameters::IntermediateAxis).eta() << " phi=" << tau->getHLV(TauJetParameters::IntermediateAxis).phi() << " m=" << tau->getHLV(TauJetParameters::IntermediateAxis).m() );    
     
	xtaujet->setP4(xAOD::TauJetParameters::IntermediateAxis, tau->getHLV(TauJetParameters::IntermediateAxis).perp(), tau->getHLV(TauJetParameters::IntermediateAxis).eta(), tau->getHLV(TauJetParameters::IntermediateAxis).phi(), tau->getHLV(TauJetParameters::IntermediateAxis).m());

	ATH_MSG_DEBUG( "converted xtau intermediateaxis 4-vector with  pt="<< xtaujet->ptIntermediateAxis() << " eta=" << xtaujet->etaIntermediateAxis() << " phi=" << xtaujet->phiIntermediateAxis() << " m=" << xtaujet->mIntermediateAxis() );    


	ATH_MSG_DEBUG( "trying to convert tau tauenergyscale 4-vector with  pt="<< tau->getHLV(TauJetParameters::TauEnergyScale).perp() << " eta=" << tau->getHLV(TauJetParameters::TauEnergyScale).eta() << " phi=" << tau->getHLV(TauJetParameters::TauEnergyScale).phi() << " m=" << tau->getHLV(TauJetParameters::TauEnergyScale).m() );    
     
	xtaujet->setP4(xAOD::TauJetParameters::TauEnergyScale, tau->getHLV(TauJetParameters::TauEnergyScale).perp(), tau->getHLV(TauJetParameters::TauEnergyScale).eta(), tau->getHLV(TauJetParameters::TauEnergyScale).phi(), tau->getHLV(TauJetParameters::TauEnergyScale).m());

	ATH_MSG_DEBUG( "converted xtau tauenergyscale 4-vector with  pt="<< xtaujet->ptTauEnergyScale() << " eta=" << xtaujet->etaTauEnergyScale() << " phi=" << xtaujet->phiTauEnergyScale() << " m=" << xtaujet->mTauEnergyScale() );    

	ATH_MSG_DEBUG( "trying to convert tau intermediateaxis 4-vector with  pt="<< tau->getHLV(TauJetParameters::TauEtaCalib).perp() << " eta=" << tau->getHLV(TauJetParameters::TauEtaCalib).eta() << " phi=" << tau->getHLV(TauJetParameters::TauEtaCalib).phi() << " m=" << tau->getHLV(TauJetParameters::TauEtaCalib).m() );    
     
	xtaujet->setP4(xAOD::TauJetParameters::TauEtaCalib, tau->getHLV(TauJetParameters::TauEtaCalib).perp(), tau->getHLV(TauJetParameters::TauEtaCalib).eta(), tau->getHLV(TauJetParameters::TauEtaCalib).phi(), tau->getHLV(TauJetParameters::TauEtaCalib).m());

	ATH_MSG_DEBUG( "converted xtau intermediateaxis 4-vector with  pt="<< xtaujet->ptTauEtaCalib() << " eta=" << xtaujet->etaTauEtaCalib() << " phi=" << xtaujet->phiTauEtaCalib() << " m=" << xtaujet->mTauEtaCalib() );    

	//trying to set element links
	ATH_MSG_DEBUG( "trying to set element links for tau with  numTrack=" << tau->numTrack() );    

	ATH_MSG_DEBUG( "converted xaod tau with numTrack=" << xtaujet->nTracks() );    


	ATH_MSG_DEBUG( "trying to convert tau with  ROIWord=" << tau->ROIWord() );    
	xtaujet->setROIWord( tau->ROIWord());
	ATH_MSG_DEBUG( "converted xaod tau with ROIWord=" << xtaujet->ROIWord() );    

	ATH_MSG_DEBUG( "trying to convert tau with  charge=" << tau->charge() );    
	xtaujet->setCharge( tau->charge() );
	ATH_MSG_DEBUG( "converted xaod tau with charge=" << xtaujet->charge() );    
      
	//set PID variables

	ATH_MSG_DEBUG( "trying to convert tau with  BDTElescore=" << tau->tauID()->discriminant(TauJetParameters::BDTEleScore)   );
	xtaujet->setDiscriminant(xAOD::TauJetParameters::BDTEleScore    , tau->tauID()->discriminant(TauJetParameters::BDTEleScore) );
	ATH_MSG_DEBUG( "converted xaod tau with BDTEleScore=" <<  xtaujet->discriminant(xAOD::TauJetParameters::BDTEleScore ) );

	xtaujet->setDiscriminant(xAOD::TauJetParameters::BDTJetScoreSigTrans    , tau->tauID()->discriminant(TauJetParameters::BDTJetScoreSigTrans) );
	xtaujet->setDiscriminant(xAOD::TauJetParameters::BDTJetScore    , tau->tauID()->discriminant(TauJetParameters::BDTJetScore) );

	ATH_MSG_DEBUG( "trying to convert tau with  MuonVeto=" << tau->tauID()->isTau(TauJetParameters::MuonVeto)   );
	xtaujet->setIsTau(xAOD::TauJetParameters::MuonVeto           ,   tau->tauID()->isTau(TauJetParameters::MuonVeto) );

	xtaujet->setIsTau(xAOD::TauJetParameters::JetBDTSigLoose     ,   tau->tauID()->isTau(TauJetParameters::JetBDTSigLoose) );
	xtaujet->setIsTau(xAOD::TauJetParameters::JetBDTSigMedium    ,   tau->tauID()->isTau(TauJetParameters::JetBDTSigMedium) );
	xtaujet->setIsTau(xAOD::TauJetParameters::JetBDTSigTight     ,   tau->tauID()->isTau(TauJetParameters::JetBDTSigTight) );
	xtaujet->setIsTau(xAOD::TauJetParameters::EleBDTLoose	   ,   tau->tauID()->isTau(TauJetParameters::EleBDTLoose) );
	xtaujet->setIsTau(xAOD::TauJetParameters::EleBDTMedium       ,   tau->tauID()->isTau(TauJetParameters::EleBDTMedium) );
	xtaujet->setIsTau(xAOD::TauJetParameters::EleBDTTight        ,   tau->tauID()->isTau(TauJetParameters::EleBDTTight) );         

	ATH_MSG_DEBUG( "set individual details variables"  );

	//set individual details variables

	const Analysis::TauCommonDetails* commonDetails(tau->details<Analysis::TauCommonDetails>());
	if(commonDetails != nullptr)
	  {
	    int tempint = 0;

	    ATH_MSG_DEBUG( "tau with nCharged " << tau->numTrack() );
	    xtaujet->setDetail(xAOD::TauJetParameters::nCharged , static_cast<int>( tau->numTrack() ) );			  
	    if( xtaujet->detail( xAOD::TauJetParameters::nCharged, tempint ) )
	      ATH_MSG_DEBUG( "converted xaod tau with nPi0 " << tempint );


	    ATH_MSG_DEBUG( "tau with pi0ConeDR " << commonDetails->Pi0ConeDR() );
	    xtaujet->setPi0ConeDR(      commonDetails->Pi0ConeDR() );			  
	    ATH_MSG_DEBUG( "converted xaod tau with pi0ConeDR " << xtaujet->pi0ConeDR() );

	    ATH_MSG_DEBUG( "tau with trackFilterProngs " << commonDetails->TrackFilterProngs() );
	    xtaujet->setTrackFilterProngs(      commonDetails->TrackFilterProngs() );			  
	    ATH_MSG_DEBUG( "converted xaod tau with trackFilterProngs " << xtaujet->trackFilterProngs() );

	    ATH_MSG_DEBUG( "tau with trackFilterQuality " << commonDetails->TrackFilterQuality() );
	    xtaujet->setTrackFilterQuality(      commonDetails->TrackFilterQuality() );			  
	    ATH_MSG_DEBUG( "converted xaod tau with trackFilterQuality " << xtaujet->trackFilterQuality() );

	    ATH_MSG_DEBUG( "found details container for this tau with SeedCalo_EMRadius " << commonDetails->seedCalo_EMRadius() );

	    xtaujet->setDetail(xAOD::TauJetParameters::ipZ0SinThetaSigLeadTrk ,      static_cast<float>( commonDetails->ipZ0SinThetaSigLeadTrk() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::etOverPtLeadTrk        ,      static_cast<float>( commonDetails->etOverPtLeadTrk() ) );				  
	    xtaujet->setDetail(xAOD::TauJetParameters::leadTrkPt              ,      static_cast<float>( commonDetails->leadTrkPt() ) );				  
	    xtaujet->setDetail(xAOD::TauJetParameters::ipSigLeadTrk           ,      static_cast<float>( commonDetails->ipSigLeadTrk() ) );				  
	    xtaujet->setDetail(xAOD::TauJetParameters::massTrkSys             ,      static_cast<float>( commonDetails->massTrkSys() ) );				  
	    xtaujet->setDetail(xAOD::TauJetParameters::trkWidth2              ,      static_cast<float>( commonDetails->trkWidth2() ) );				  
	    xtaujet->setDetail(xAOD::TauJetParameters::trFlightPathSig        ,      static_cast<float>( commonDetails->trFlightPathSig() ) );				  
	    xtaujet->setDetail(xAOD::TauJetParameters::numCells        ,      static_cast<int>( commonDetails->numCells() ) );				  
	    xtaujet->setDetail(xAOD::TauJetParameters::numTopoClusters        ,      static_cast<int>( commonDetails->numTopoClusters() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::numEffTopoClusters     ,      static_cast<float>( commonDetails->numEffTopoClusters() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::topoInvMass            ,      static_cast<float>( commonDetails->topoInvMass() ) );				  
	    xtaujet->setDetail(xAOD::TauJetParameters::effTopoInvMass         ,      static_cast<float>( commonDetails->effTopoInvMass() ) );				  
	    xtaujet->setDetail(xAOD::TauJetParameters::topoMeanDeltaR         ,      static_cast<float>( commonDetails->topoMeanDeltaR() ) );				  
	    xtaujet->setDetail(xAOD::TauJetParameters::effTopoMeanDeltaR      ,      static_cast<float>( commonDetails->effTopoMeanDeltaR() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::EMRadius      ,      static_cast<float>( commonDetails->seedCalo_EMRadius() ) );			  

	    float tempfloat = 0;
	    if (   xtaujet->detail(xAOD::TauJetParameters::EMRadius, tempfloat) )
	      ATH_MSG_DEBUG( "converted details for this xaod tau with SeedCalo_EMRadius " << tempfloat );
	    else
	      ATH_MSG_DEBUG( "error getting SeedCalo_EMRadius from xaod tau " );

	    xtaujet->setDetail(xAOD::TauJetParameters::hadRadius ,	      static_cast<float>( commonDetails->seedCalo_hadRadius() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::etEMAtEMScale ,      static_cast<float>( commonDetails->seedCalo_etEMAtEMScale() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::etHadAtEMScale ,     static_cast<float>( commonDetails->seedCalo_etHadAtEMScale() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::isolFrac ,	      static_cast<float>( commonDetails->seedCalo_isolFrac() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::centFrac ,	      static_cast<float>( commonDetails->seedCalo_centFrac() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::stripWidth2 ,	      static_cast<float>( commonDetails->seedCalo_stripWidth2() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::nStrip ,	      static_cast<int>( commonDetails->seedCalo_nStrip() ) );				  
	    xtaujet->setDetail(xAOD::TauJetParameters::trkAvgDist ,	      static_cast<float>( commonDetails->seedCalo_trkAvgDist() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::trkRmsDist ,	      static_cast<float>( commonDetails->seedCalo_trkRmsDist() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::lead2ClusterEOverAllClusterE , static_cast<float>( commonDetails->seedCalo_lead2ClusterEOverAllClusterE() ) );	  
	    xtaujet->setDetail(xAOD::TauJetParameters::lead3ClusterEOverAllClusterE , static_cast<float>( commonDetails->seedCalo_lead3ClusterEOverAllClusterE() ) );	  
	    xtaujet->setDetail(xAOD::TauJetParameters::caloIso ,	      static_cast<float>( commonDetails->seedCalo_caloIso() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::caloIsoCorrected ,   static_cast<float>( commonDetails->seedCalo_caloIsoCorrected() ) );		  
	    xtaujet->setDetail(xAOD::TauJetParameters::dRmax ,	      static_cast<float>( commonDetails->seedCalo_dRmax() ) );				  
	    xtaujet->setDetail(xAOD::TauJetParameters::secMaxStripEt  ,	      static_cast<float>( commonDetails->seedTrk_secMaxStripEt() ) );				  
	    xtaujet->setDetail(xAOD::TauJetParameters::sumEMCellEtOverLeadTrkPt  ,   static_cast<float>( commonDetails->seedTrk_sumEMCellEtOverLeadTrkPt() ) );				  
	    xtaujet->setDetail(xAOD::TauJetParameters::hadLeakEt  ,	              static_cast<float>( commonDetails->seedTrk_hadLeakEt() ) );				  

	    xtaujet->setDetail(xAOD::TauJetParameters::cellBasedEnergyRing1 ,	      static_cast<float>( commonDetails->cellBasedEnergyRing1() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::cellBasedEnergyRing2 ,	      static_cast<float>( commonDetails->cellBasedEnergyRing2() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::cellBasedEnergyRing3 ,	      static_cast<float>( commonDetails->cellBasedEnergyRing3() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::cellBasedEnergyRing4 ,	      static_cast<float>( commonDetails->cellBasedEnergyRing4() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::cellBasedEnergyRing5 ,	      static_cast<float>( commonDetails->cellBasedEnergyRing5() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::cellBasedEnergyRing6 ,	      static_cast<float>( commonDetails->cellBasedEnergyRing6() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::cellBasedEnergyRing7 ,	      static_cast<float>( commonDetails->cellBasedEnergyRing7() ) );			  
	    xtaujet->setDetail(xAOD::TauJetParameters::TRT_NHT_OVER_NLT ,	      static_cast<float>( commonDetails->TRT_NHT_OVER_NLT() ) );			  
	  } else {
	  ATH_MSG_WARNING( "there was no TauDetails container found" );
	}

      }
  

    return StatusCode::SUCCESS;
  }

  void TauJetCnvTool::setLinks(const Analysis::TauJet& aodtau, xAOD::TauJet& xaodtau) const {

    // Need to reset links from old TrackParticle to xAOD::TrackParticles
  
    for (unsigned int i = 0; i != aodtau.numTrack(); ++i) 
      {
	ATH_MSG_DEBUG( "track number : " << i << " has pt: " << (aodtau.track(i) ? aodtau.track(i)->pt() : -1111.) );    
      }

    //get common details member, because wide and other tracks are stored there
    const Analysis::TauCommonDetails* commonDetails(aodtau.details<Analysis::TauCommonDetails>());


    for (unsigned int i = 0; i != commonDetails->seedCalo_nWideTrk(); ++i) 
      {
	ATH_MSG_DEBUG( "wide track number : " << i << " has pt: " << commonDetails->seedCalo_wideTrk(i)->pt() );    
      }

    ATH_MSG_DEBUG( "trying to set jet link " );
  
    ATH_MSG_DEBUG( "tau jet seed has pt : " <<  aodtau.jet()->pt() );    

    xaodtau.setJetLink(getNewJetLink(aodtau.jetLink(), m_jets) );

    ATH_MSG_DEBUG( "tau jet seed has pt : " <<  (*xaodtau.jetLink())->pt() );    


  }

  ElementLink<xAOD::TrackParticleContainer> TauJetCnvTool::getNewTrackLink(const ElementLink<Rec::TrackParticleContainer>& oldLink, const std::string& name) const{
    ElementLink<xAOD::TrackParticleContainer> newLink;
    newLink.resetWithKeyAndIndex( name, oldLink.index() );
    return newLink;
  }

  ElementLink<xAOD::JetContainer> TauJetCnvTool::getNewJetLink(const ElementLink<JetCollection>& oldLink, const std::string& name) const{
    ElementLink<xAOD::JetContainer> newLink;
    newLink.resetWithKeyAndIndex( name, oldLink.index() );
    return newLink;
  }

}


//  LocalWords:  tempfloat
