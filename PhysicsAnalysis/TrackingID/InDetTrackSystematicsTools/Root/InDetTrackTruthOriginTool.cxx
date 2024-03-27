/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginDefs.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"
#include "TruthUtils/MagicNumbers.h"

#include <math.h>

namespace InDet {

  InDetTrackTruthOriginTool::InDetTrackTruthOriginTool(const std::string& name) :
    asg::AsgTool(name)
  {

#ifndef XAOD_STANDALONE
    declareInterface<IInDetTrackTruthOriginTool>(this);
#endif

    declareProperty("matchingProbabilityCut", m_matchingProbabilityCut = 0.5);

    declareProperty("truthParticleLinkName", m_truthParticleLinkName = "truthParticleLink");
    declareProperty("truthMatchProbabilityAuxName", m_truthMatchProbabilityAuxName = "truthMatchProbability");
    declareProperty("isFullPileUpTruth", m_isFullPileupTruth = false);

  }

  InDetTrackTruthOriginTool::~InDetTrackTruthOriginTool() = default;

  StatusCode InDetTrackTruthOriginTool::initialize() {
    if(m_isFullPileupTruth) ATH_MSG_INFO("InDetTrackTruthOriginTool configured for full pile-up truth sample");
    return StatusCode::SUCCESS;
  }

  const xAOD::TruthParticle* InDetTrackTruthOriginTool::getTruth( const xAOD::TrackParticle* track ) const {

      // if the track doesnt't have a valid truth link, skip to the next track
      // in practice, all tracks seem to have a truth link, but we need to also
      // check whether it's valid
      typedef ElementLink<xAOD::TruthParticleContainer> TruthLink;
      if ( !track->isAvailable<TruthLink>("truthParticleLink") ) { 
        return nullptr;
      }

      // retrieve the link and check its validity
      const TruthLink &link = track->auxdata<TruthLink>("truthParticleLink");

      // a missing or invalid link implies truth particle has been dropped from 
      // the truth record at some stage - probably it was from pilup which by
      // default we don't store truth information for
      if(!link or !link.isValid()) {
        return nullptr;
      }

      // seems safe to access and return the linked truth particle
      return *link;
  }

  int InDetTrackTruthOriginTool::getTruthOrigin(const xAOD::TruthParticle* truth) const {

    int origin = 0;

    // is it fragmentation? yes until proven otherwise
    bool isFragmentation = true;

    // from B decay chain?
    if(isFrom(truth, 5)) {
      origin = origin | (0x1 << InDet::TrkOrigin::BHadronDecay);
      isFragmentation = false;
    }

    // from D decay chain?
    if(isFrom(truth, 4)) {
      origin = origin | (0x1 << InDet::TrkOrigin::DHadronDecay);
      isFragmentation = false;
    }

    // from tau decay chain?
    if(isFrom(truth, 15)) {
      origin = origin | (0x1 << InDet::TrkOrigin::TauDecay);
      isFragmentation = false;
    }

    // Secondary? 
    if (HepMC::is_simulation_particle(truth)) {
      // sub-categorize secondaries...
      int parentID = getParentID(truth);

      // photon conversions
      if(parentID == 22) {
        origin = origin | (0x1 << InDet::TrkOrigin::GammaConversion);
      }

      // K-short
      else if(parentID == 310) {
        origin = origin | (0x1 << InDet::TrkOrigin::KshortDecay);
      }

      // Lambda
      else if(std::abs(parentID) == 3122){
        origin = origin | (0x1 << InDet::TrkOrigin::LambdaDecay);
      }

      // other long living particle decays
      else if(std::abs(parentID) > 3) {
        origin = origin | (0x1 << InDet::TrkOrigin::OtherDecay);
      }

      // hadronic interactions
      else if(parentID == -1) {
        origin = origin | (0x1 << InDet::TrkOrigin::HadronicInteraction);
      }

      // other secondaries? 
      //  ---> Not sure what if anything should be here...
      else if(parentID == -2) {
        origin = origin | (0x1 << InDet::TrkOrigin::OtherSecondary);
      }

      // other unknown origin (e.g. parent not in the record:) 
      //  ---> Not sure what if anything should be here...
      else{
        origin = origin | (0x1 << InDet::TrkOrigin::OtherOrigin);
      }

      isFragmentation = false;
    }

    // uncategorized: call it fragmentation
    if(isFragmentation) {
      origin = origin | (0x1 << InDet::TrkOrigin::Fragmentation);
    }

    return origin;
  }

  int InDetTrackTruthOriginTool::getTrackOrigin(const xAOD::TrackParticle* track) const {

    // get truth particle
    const xAOD::TruthParticle* truth = getTruth( track );

    // get track TMP
    static const SG::AuxElement::ConstAccessor< float > tmpAcc( m_truthMatchProbabilityAuxName.data() );
    float truthProb = tmpAcc( *track );

    int origin = 0;

    // truth link is broken: call it from pileup (not true for < 100 MeV and some specialised samples!)
    if (!truth){
      origin = origin | (0x1 << InDet::TrkOrigin::Pileup);
    }
    
    // For full pile-up truth samples, check explicitly if truth record is present among hard-scatter collection to assign Pileup origin
    else if(m_isFullPileupTruth){
      const xAOD::TruthEventContainer* truthEventContainer(nullptr);
      if(evtStore()->retrieve(truthEventContainer, "TruthEvents").isFailure()){
	ATH_MSG_ERROR("InDetTrackTruthOriginTool configured for full pile-up truth but could not retrieve TruthEvents container");
      }
      const xAOD::TruthEvent* event = truthEventContainer ? truthEventContainer->at(0) : nullptr;

      if(event){
	const auto& links = event->truthParticleLinks();

	bool isFromHSProdVtx = false;
	for (const auto& link : links){
	  if(link.isValid() && truth == *link){
	    isFromHSProdVtx = true;
	    break;
	  }
	}

	if(!isFromHSProdVtx) origin = origin | (0x1 << InDet::TrkOrigin::Pileup);
      }
    }

    // low TruthMatchProbability: call it fake (also includes poorly reconstructed tracks)
    if(truthProb < m_matchingProbabilityCut) {
      origin = origin | (0x1 << InDet::TrkOrigin::Fake);
    }

    // truth link is present: find truth origin
    if (truth) {
      origin = origin | getTruthOrigin(truth);
    }

    return origin;
  }

  bool InDetTrackTruthOriginTool::isFrom(const xAOD::TruthParticle* truth, int flav) const {

    if ( truth == nullptr ) return false;

    if( flav != 5 && flav != 4 && flav != 15 ) return false;

    if( flav == 5 && truth->isBottomHadron() ) return true;

    if( flav == 4 && truth->isCharmHadron() ) return true;

    if( flav == 15 && abs(truth->pdgId()) == 15 ) return true;


    for(unsigned int p=0; p<truth->nParents(); p++) {
      const xAOD::TruthParticle* parent = truth->parent(p);
      if(parent == truth ) continue ; // avoid infinite recursion
      if( isFrom(parent, flav) ) return true;
    }

    return false;
  }

  int InDetTrackTruthOriginTool::getParentID(const xAOD::TruthParticle* truth) const {

    // no parents? is anything even there?
    if(truth->nParents() != 1) return -3;

    int pdgId = truth->pdgId();

    const xAOD::TruthParticle *parent = truth->parent(0);

    // in some files, the particle has a parent but that parent's barcode is zero.
    // treat this as "other" (anything less than -2)
    if (parent == nullptr) return -4;

    int parentId = parent->pdgId();

    // photon conversions:
    if(parent->isPhoton() && truth->isElectron()) return parentId;

    // Kshort:
    if(abs(pdgId) == 211 && parentId == 310 && parent->nChildren() == 2) return parentId;

    // Lambdas
    if((abs(pdgId) == 211 || abs(pdgId) == 2212) && abs(parentId) == 3122 && parent->nChildren() == 2) return parentId;

    // other decays:
    if(parent->isHadron() && parent->nChildren() == 2) return parentId;

    // hadronic interactions
    if(parent->nChildren() > 2) return -1;

    // other secondaries: is anything even there??
    return -2;
  }

}
