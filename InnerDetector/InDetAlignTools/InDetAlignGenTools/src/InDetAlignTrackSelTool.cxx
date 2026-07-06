/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// InDetAlignTrackSelTool.h

// Sergio Gonzalez Sevilla, started 04/7/05
// Miguel Olivo Gomez, extended 07/6/06

// AlgTool to select high quality tracks for the inner detector 
// (Pixel+SCT) alignment algorithms.
// This AlgTool provides a track selection based on the following cut variables:
// Momentum, pt, number of shared hits, number of holes and chi2 probability. 
// Returns 0 in case a track is not accepted, otherwise 1

#include "InDetAlignTrackSelTool.h"

#include "xAODTracking/TrackParticle.h"
#include "TrkToolInterfaces/ITrackParticleCreatorTool.h"

#include "TrkParameters/TrackParameters.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "CLHEP/GenericFunctions/CumulativeChiSquare.hh"

InDetAlignTrackSelTool::InDetAlignTrackSelTool( const std::string& type
						, const std::string& name
						, const IInterface* parent
						)
  : AthAlgTool(type,name,parent),
    m_particleCreator("Trk::TrackParticleCreatorTool/TrackParticleCreatorTool", this),
    m_minMomentum(0),
    m_minPt(2),
    m_maxShared(0),
    m_maxHoles(1),
    m_minChi2Prob(0.2)
{
  declareInterface<IInDetAlignTrackSelTool>(this);
  declareProperty("MinMomentum"        , m_minMomentum);
  declareProperty("MinPt"              , m_minPt);
  declareProperty("MaxShared"          , m_maxShared);
  declareProperty("MaxHoles"           , m_maxHoles);
  declareProperty("MinChi2Prob"        , m_minChi2Prob);

  // Tools
  declareProperty("TrackParticleCreatorTool", m_particleCreator,
                  "tool to build TrackParticle");
}

InDetAlignTrackSelTool::~InDetAlignTrackSelTool()
{}

////////////////////////////////////////////////////////////////////////////////////////
StatusCode InDetAlignTrackSelTool::initialize(){
  ////////////////////////////////////////////////////////////////////////////////////////

  // get TrackParticleCreatorTool
  ATH_CHECK(m_particleCreator.retrieve());
  
  ATH_MSG_DEBUG( "Cuts selected : min_Momentum(CLHEP::GeV)=" <<  m_minMomentum
		<< " min_pt(CLHEP::GeV)=" <<  m_minPt
		<< " max_shared=" << m_maxShared
		<< " max_holes=" << m_maxHoles
		<< " min_chi2Prob=" << m_minChi2Prob ) ;

  ATH_MSG_DEBUG( "InDetAlignTrackSelTool initialize() successful" ) ;
  return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////
StatusCode InDetAlignTrackSelTool::finalize(){
  ////////////////////////////////////////////////////////////////////////////////////////
  ATH_MSG_DEBUG( "InDetAlignTrackSelTool finalize method called" ) ;
  return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////
double InDetAlignTrackSelTool::Momentum(const Trk::Track& track) const {
  ////////////////////////////////////////////////////////////////////////////////////////
  ATH_MSG_DEBUG( "in Momentum() " ) ;
  double mom=0.;

  // get measured perigee and momentum of track
  const Trk::Perigee* perigee = track.perigeeParameters();

  if ( !perigee->covariance()) {
    ATH_MSG_ERROR( "No measured perigee parameters assigned to the track" ) ; 
    mom = -1e12; // return big value
  }
  else{
    Amg::VectorX perigeeParams = perigee->parameters();
    mom = std::abs(1./perigeeParams[Trk::qOverP]);
    mom /= 1000.; //mom in GeV
  }

  return mom;
}

////////////////////////////////////////////////////////////////////////////////////////
double InDetAlignTrackSelTool::Pt(const Trk::Track& track) const {
  ////////////////////////////////////////////////////////////////////////////////////////
  ATH_MSG_DEBUG( "in Pt() " ) ;
  double pt=0.;

  // get measured perigee and pt of track
  const Trk::Perigee* perigee = track.perigeeParameters();

  if (!perigee->covariance()) {
    ATH_MSG_ERROR( "No measured perigee parameters assigned to the track" ) ; 
    pt = -1e12; // return big value
  }
  else{  
    Amg::VectorX perigeeParams = perigee->parameters();
    pt = std::abs(sin(perigeeParams[Trk::theta])/perigeeParams[Trk::qOverP]);
    pt /= 1000.; // pt in GeV
  }

  return pt;
}

////////////////////////////////////////////////////////////////////////////////////////
int InDetAlignTrackSelTool::nShared(const EventContext& ctx, const Trk::Track& track) const {
  ////////////////////////////////////////////////////////////////////////////////////////
  ATH_MSG_DEBUG( "in nShared()" ) ;
  int nshared=0, nshpix, nshsct;

  xAOD::TrackParticle* trackPart = m_particleCreator->createParticle(ctx, track);
  uint8_t iSummaryValue(0); // Dummy counter to retrieve summary values

  if (not trackPart){
    ATH_MSG_ERROR("Could not get xAOD::TrackParticle");
    nshared = 1000;
  }
  else{
    nshpix = trackPart->summaryValue(iSummaryValue, xAOD::numberOfPixelSharedHits) ? iSummaryValue : 0;
    nshsct = trackPart->summaryValue(iSummaryValue, xAOD::numberOfSCTSharedHits) ? iSummaryValue : 0;
    //At condition nhpix == -1, the following bits of nhsct are known to be unset: 8-31.
    //The condition nhpix == -1 cannot be true.
    //At condition nhsct == -1, the following bits of nhsct are known to be unset: 8-31.
    //The condition nhsct == -1 cannot be true.

    nshared = nshpix + nshsct;
  }
  return nshared;
}

////////////////////////////////////////////////////////////////////////////////////////
int InDetAlignTrackSelTool::nHoles(const EventContext& ctx, const Trk::Track& track) const {
  ////////////////////////////////////////////////////////////////////////////////////////
  ATH_MSG_DEBUG( "in nHoles() " ) ;
  int nholes{}, nhpix{}, nhsct{};

  xAOD::TrackParticle* trackPart = m_particleCreator->createParticle(ctx, track);
  uint8_t iSummaryValue(0); // Dummy counter to retrieve summary values

  if (not trackPart){
    ATH_MSG_ERROR("Could not get xAOD::TrackParticle");
    nholes = 1000;
  }
  else{
    nhpix = trackPart->summaryValue(iSummaryValue, xAOD::numberOfPixelHoles) ? iSummaryValue : 0;
    nhsct = trackPart->summaryValue(iSummaryValue, xAOD::numberOfSCTHoles) ? iSummaryValue : 0;
    //At condition nhpix == -1, the following bits of nhsct are known to be unset: 8-31.
    //The condition nhpix == -1 cannot be true.
    //At condition nhsct == -1, the following bits of nhsct are known to be unset: 8-31.
    //The condition nhsct == -1 cannot be true.
    nholes =  nhpix + nhsct;
  }
  return nholes;
}

////////////////////////////////////////////////////////////////////////////////////////
double InDetAlignTrackSelTool::chi2Prob(const Trk::Track& track) const {
  ////////////////////////////////////////////////////////////////////////////////////////
  ATH_MSG_DEBUG( "in chi2Prob()" ) ;
  double chi2Prob=0.;

  // get fit quality and chi2 probability of track
  // chi2Prob = TMath::Prob(chi2,DoF) ROOT function
  const Trk::FitQuality* fitQual = track.fitQuality();

  if (fitQual==nullptr) {
    ATH_MSG_ERROR( "No fit quality assigned to the track" ) ; 
    chi2Prob = -1e12; // return big value
  }
  else {
    if (fitQual->chiSquared() > 0. && fitQual->numberDoF() > 0) {
      Genfun::CumulativeChiSquare probabilityFunction( fitQual->numberDoF() );
      chi2Prob = 1 - probabilityFunction( fitQual->chiSquared() );
    }
  }

  return chi2Prob;
}

////////////////////////////////////////////////////////////////////////////////////////
int InDetAlignTrackSelTool::getStatus(const EventContext& ctx, const Trk::Track& track) const {
  ////////////////////////////////////////////////////////////////////////////////////////
  ATH_MSG_DEBUG( "in getStatus()" ) ;
  int stat=1, nholes, nshared;
  double mom, pt, chi2prob;

  // momentum
  mom = Momentum(track);
  if (mom < m_minMomentum) {
    stat=0;
  }

  // transverse momentum
  pt = Pt(track);
  if (pt < m_minPt) {
    stat=0; 
  }

  // number of holes
  nholes = nHoles(ctx, track);
  if (nholes > m_maxHoles) {
    stat=0; 
  }

  // number of shared hits
  nshared = nShared(ctx, track);
  if (nshared > m_maxShared) {
    stat=0; 
  }

  // chi2 Probability
  chi2prob = chi2Prob(track);
  if (chi2prob < m_minChi2Prob) {
    stat=0;
  }

  ATH_MSG_DEBUG( " momentum(CLHEP::GeV)=" << mom 
		 << " pt (CLHEP::GeV)=" << pt 
		 << " nshared=" << nshared
		 << " nholes=" <<  nholes 
		 << " chi2Prob=" << chi2prob
		 ) ;

  if(!stat)
    ATH_MSG_DEBUG( "Track not accepted" ) ;
  else
    ATH_MSG_DEBUG( "Track accepted" ) ;

  return stat;
}
