/**
 ** @file   TrackAnalysisStore.cxx
 **
 ** @author sutt@cern.ch
 **
 **         the rest of the fetching code, particulatly the non template 
 **         pieces, or the conctrete template instantiations 
 **
 **
 **  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#include "TrackAnalysisStore.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

#include "TrackParticleTraits.h"
#include "TruthParticleTraits.h"
#include "ElectronTraits.h"
#include "MuonTraits.h"
#include "TauJetTraits.h"
#include "TrackView.h"

#include "xAODTracking/VertexContainer.h"
#include "xAODTruth/TruthVertexContainer.h"

#include "xAODVertexTraits.h"

#include "VertexView.h"

#include <algorithm> 



/// --------------------
/// --- Constructors ---
/// --------------------
IDTPM2::TrackAnalysisStore::TrackAnalysisStore( const std::string& anaTag, const EventContext& context ) :
    AthMessaging( "TrackAnalysisStore"+anaTag ),
    m_context(context),
    m_tdt(0),
    m_roiIndex(0),
    m_anaTag(anaTag)
{ }

IDTPM2::TrackAnalysisStore::TrackAnalysisStore( const std::string& anaTag,
						const ToolHandle<Trig::TrigDecisionTool>* tdt, const EventContext& context ) : 
    AthMessaging( "TrackAnalysisStore"+anaTag ),
    m_context(context),
    m_tdt(tdt),
    m_roiIndex(0),
    m_anaTag(anaTag)
{ }


/// ------------------
/// --- initialize ---
/// ------------------
/// this was a tool, instantiated, AS NEW for EVERY event, therefor
/// having an initialize() method of this nature is completely
/// pointless - so now everything is done in the constructor
/// and the initialise is actually redundent. Also the DefinitioSvc
/// is not needed, as the required information can simply be passed
/// in from the top level tool. 
StatusCode IDTPM2::TrackAnalysisStore::initialize() {  return StatusCode::SUCCESS;  }



/// track stuff ...

/// this is a bit naughty - using the m_parents directly rather than  passing it in - prevents being able
/// to use this mechanism for the test corrections as well, so perhaps it could be changed, should change
/// this to always pass in both a TrackView and a ParentView vectors   


void IDTPM2::TrackAnalysisStore::fillView( const std::string& key, view_store_t& trackView ) { 
  if ( fillView_internal<xAOD::TrackParticleContainer, TrackView>( key, trackView ) ) return;
  if (  m_beamkeys && fillView_internal<xAOD::TruthParticleContainer, TrackView, TruthView>( key, trackView ) ) return decoratePerigee();
  if ( !m_beamkeys && fillView_internal<xAOD::TruthParticleContainer, TrackView>( key, trackView ) ) return;
  if ( fillView_internal<xAOD::ElectronContainer>( key, m_parents ) ) return; 
  if ( fillView_internal<xAOD::MuonContainer>(     key, m_parents ) ) return;
  if ( fillView_internal<xAOD::TauJetContainer>(   key, m_parents ) ) return;
}



///  must be a better way that to have a duplicated function, but only one I can
///  think of is to pass in the TrackView, and both TrackParticle and TruthParticle
///  contaiuners
void IDTPM2::TrackAnalysisStore::fillView( const std::string& key, const ElementLink<TrigRoiDescriptorCollection>& roi_link, view_store_t& trackView ) { 
    
    /// don't need to bother with TruthParticles as they are not reconstructed in an Roi
    std::pair<xAOD::TrackParticleContainer::const_iterator, xAOD::TrackParticleContainer::const_iterator> itrpair;
    
    if ( fillRoi<xAOD::TrackParticleContainer>( itrpair, key, roi_link ).isSuccess() ) {
      fillv( trackView, itrpair.first, itrpair.second );
    }
    else {
      throw std::runtime_error( "iterator pair not retrieved FFS for key: "+key );
    }
    
}


/// vertex stuff ...
/// now we have the child-parent relationship sorted out, these vertices should really
/// become ParentViews containing the pointers to all their tracks
void IDTPM2::TrackAnalysisStore::fillView( const std::string& key, vtx_store_t& vertexView ) { 
  if ( fillView_internal<xAOD::VertexContainer, VertexView>(      key, vertexView ) ) return;
  if ( fillView_internal<xAOD::TruthVertexContainer, VertexView>( key, vertexView ) ) return;
}



/// this and the roi based TrackView fillView should really be done with a single template
/// eventually, likelyt create Parent and Child VertexViews as from the TrackView, then
/// we can have the vertices own their own tracks and we would be unstoppable I tell you !! 
void IDTPM2::TrackAnalysisStore::fillView( const std::string& key, const ElementLink<TrigRoiDescriptorCollection>& roi_link, vtx_store_t& vertexView ) { 

    /// don't need to bother with TruthVertices as they are not reconstructed in an Roi
    std::pair<xAOD::VertexContainer::const_iterator, xAOD::VertexContainer::const_iterator> itrpair;
    
    if ( fillRoi<xAOD::VertexContainer>( itrpair, key, roi_link ).isSuccess() ) {
      fillv( vertexView, itrpair.first, itrpair.second );
    }
    else {
      throw std::runtime_error( "iterator pair not retrieved FFS for key: "+key );
    }    

}





/// FIXME: the checks whether these are of the right quality are done using strings
///        at the moment so these should be converted to use our own custom enum
///        to get round all the different enums for the different actual EDM types

/// check if this is a good Electron - do we want this in the ElectronTraits some how ??
/// probably not at the moment

template<>
bool IDTPM2::TrackAnalysisStore::isGoodOffline<xAOD::Electron>( const xAOD::Electron& elec, const std::string& quality, const std::string&  ) {
  // no additional selection - this can go into a specific selector probably ...
  if ( quality.empty() ) return true;
  return elec.passSelection(quality); 
}




/// check if it is an good muon ...
/// it is a shame this can't be as simple as the electron code ...

template<>
bool IDTPM2::TrackAnalysisStore::isGoodOffline<xAOD::Muon>(const xAOD::Muon& muon, const std::string& quality, const std::string&  ) {

  if ( quality.empty() ) return true;

  /// should allow this to be set also
  if ( muon.muonType() != xAOD::Muon::Combined ) return false;
 
  bool good_muon = false;

  /// hmmm - it looks like if it passed "Tight" selection, then it *does not* pass medium
  /// selection, as the goups are *exclusive* - this is onsensical !
  if      ( quality=="Tight"     ) good_muon = (muon.quality()<=xAOD::Muon::Quality::Tight);
  else if ( quality=="Medium"    ) good_muon = (muon.quality()<=xAOD::Muon::Quality::Medium);
  else if ( quality=="Loose"     ) good_muon = (muon.quality()<=xAOD::Muon::Quality::Loose);
  else if ( quality=="VeryLoose" ) good_muon = (muon.quality()<=xAOD::Muon::Quality::VeryLoose);
  else                             good_muon = true;

  ///  std::cout << "isGoodOffline: muon " << quality << "\t" << good_muon << std::endl;
  
  return good_muon; 
}



/// and check if it is a good tau

template<>
bool IDTPM2::TrackAnalysisStore::isGoodOffline<xAOD::TauJet>( const xAOD::TauJet& tau, const std::string& quality, const std::string& prongs ) {

   if ( quality.empty() && prongs.empty() ) return true;

#  ifndef XAODTAU_VERSIONS_TAUJET_V3_H
   int Ntracks = tau.nTracks();
#  else
   int Ntracks=0;   tau.detail( xAOD::TauJetParameters::nChargedTracks, Ntracks );
#  endif

   bool good_tau = false;

   /// run 3 selection
   if      ( quality == "Tight"  )          good_tau = tau.isTau(xAOD::TauJetParameters::JetRNNSigTight);
   else if ( quality == "Medium" )          good_tau = tau.isTau(xAOD::TauJetParameters::JetRNNSigMedium);
   else if ( quality == "Loose"  )          good_tau = tau.isTau(xAOD::TauJetParameters::JetRNNSigLoose);
   else if ( quality == "JetRNNSigTight"  ) good_tau = tau.isTau(xAOD::TauJetParameters::JetRNNSigTight);  // so it doesn't confuse anyone
   else if ( quality == "JetRNNSigMedium" ) good_tau = tau.isTau(xAOD::TauJetParameters::JetRNNSigMedium);
   else if ( quality == "JetRNNSigLoose"  ) good_tau = tau.isTau(xAOD::TauJetParameters::JetRNNSigLoose);
   else  good_tau = true;

   //   std::cout << "isGoodOffline: tau " << quality << "\t" << prongs << "\t(" << Ntracks << ") " << good_tau << std::endl;
   
   /// do we want to build this in as well ??
   /// bool not_a_electron = !( tauisTau(xAOD::TauJetParameters::EleBDTMedium) );
   /// bool not_a_muon     = !( tauisTau(xAOD::TauJetParameters::MuonVeto) );
   /// bool best_tau       = good_tau && not_a_electron && not_a_muon;
   if      ( prongs.empty() )                 return good_tau;
   else if ( prongs=="1Prong" && Ntracks==1 ) return good_tau;
   else if ( prongs=="2Prong" && Ntracks==2 ) return good_tau;
   else if ( prongs=="3Prong" && Ntracks==3 ) return good_tau;

   return false;

}


void IDTPM2::TrackAnalysisStore::decoratePerigee() { 

    /// can extrapolate to the beamline perigee, if we don't know where
    /// the beamline is ... 
    if ( !m_beamkeys ) return;

    /// only currently bother with this for actual, Truth/Particles ...
    if ( m_reftracks.empty() || !m_reftracks.front()->is<xAOD::TruthParticle>() ) return;
        
    SG::ReadDecorHandle<xAOD::EventInfo, float> beamPosX((*m_beamkeys)[0], m_context);
    SG::ReadDecorHandle<xAOD::EventInfo, float> beamPosY((*m_beamkeys)[1], m_context);
    SG::ReadDecorHandle<xAOD::EventInfo, float> beamPosZ((*m_beamkeys)[2], m_context);

    /// add perigee parameters ...
    /// NB: could make this add perigee parameters only when actually requested ... 
    for ( size_t i=0 ; i<m_reftracks.size() ; i++ ) {

      TruthView* tv = dynamic_cast<TruthView*>(m_reftracks[i].get());

      if ( !tv ) throw std::runtime_error("not a TruthView");

      const xAOD::TruthParticle* tp = tv->as<xAOD::TruthParticle>();

      if ( std::fabs(tv->pt())<50 ) continue;

      if ( !MC::isStable(tp) ) continue;				   
      
      float charge = tp->charge();

      /// should *always* have a production vertex ...
      if ( !tp->hasProdVtx() ) {
	tv->add_z0(-999);
	tv->add_d0(-999);
	tv->add_phi(tp->phi());
	continue;
      }

      /// all of this would probably be much better, and certainiy more
      /// compact using some vector types ...

      double vx = tp->prodVtx()->x();
      double vy = tp->prodVtx()->y();
      double vz = tp->prodVtx()->z();

      double bx = beamPosX(0);
      double by = beamPosY(0);
      double bz = beamPosZ(0);

      double dx = vx - bx;
      double dy = vy - by;
      double dz = vz - bz;
      
      double eta = tp->eta();
      double phi = tp->phi();

      double theta = 2*std::atan( std::exp( -eta ) );

      double   sinphi = std::sin(phi);
      double   cosphi = std::cos(phi);
      double tantheta = std::tan(theta);

      if ( charge==0 ) { 

	/// do we really want to bother for neutrals ???
	
	/// siimple ultra-fast limear extrapolations ...
	double    z0 = dz - (dy*cosphi + dy*sinphi)/tantheta + bz;
	double    d0 =       dy*cosphi - dx*sinphi; 
	
	tv->add_z0(z0);
	tv->add_d0(d0);
	tv->add_phi(tp->phi());

      }
      else { 

	/// linear ...
	//	double    z0_ = dz - (dy*cosphi + dy*sinphi)/tantheta + bz;
	//	double    d0_ =       dy*cosphi - dx*sinphi; 
	
	// helix approximation ...

	int sign = ( charge<0 ? -1 : 1 );

	const double B = 2;
	const double c = 2.99792458;
	
	double QR = -charge*std::fabs(tv->pt())*10/(B*c);
	
	double nx =  sign*sinphi;
	double ny = -sign*cosphi;
	
	double Cx = vx + QR*nx;
	double Cy = vy + QR*ny;
	
	double wx = Cx - beamPosX(0);
	double wy = Cy - beamPosY(0);
	
	double w = std::sqrt( wx*wx + wy*wy );
	
	double d0   = w - std::fabs(QR);
	
	double rvx = -QR*nx;
	double rvy = -QR*ny;
	
	double rpx = -std::fabs(QR)*wx/w;
	double rpy = -std::fabs(QR)*wy/w; 
	
	double dphi = sign*std::atan2( rvx*rpy - rvy*rpx, rvx*rpx + rvy*rpy );

	/// do we need this ?
	//      double side = dx*wy - dy*wx;
	
	double s = dphi*QR;
	
	double z0 = dz - s/tantheta + bz;

	double phi = tp->phi()-dphi;
	
	tv->add_z0(z0);
	tv->add_d0(d0);
	tv->add_phi(phi);
	
      }
    }

}
