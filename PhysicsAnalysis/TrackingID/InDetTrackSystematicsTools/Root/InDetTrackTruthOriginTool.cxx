/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginDefs.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"
#include "TruthUtils/HepMCHelpers.h"
#include "AthContainers/ConstAccessor.h"

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
      static const SG::ConstAccessor<TruthLink>
        truthParticleLinkAcc ("truthParticleLink");
      if ( !truthParticleLinkAcc.isAvailable(*track) ) { 
        return nullptr;
      }

      // retrieve the link and check its validity
      const TruthLink &link = truthParticleLinkAcc(*track);

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
    if(isFrom(truth, MC::BQUARK)) {
      origin = origin | (0x1 << InDet::TrkOrigin::BHadronDecay);
      isFragmentation = false;
    }

    // from D decay chain?
    if(isFrom(truth, MC::CQUARK)) {
      origin = origin | (0x1 << InDet::TrkOrigin::DHadronDecay);
      isFragmentation = false;
    }

    // from tau decay chain?
    if(isFrom(truth, MC::TAU)) {
      origin = origin | (0x1 << InDet::TrkOrigin::TauDecay);
      isFragmentation = false;
    }

    // Secondary? 
    if (HepMC::is_simulation_particle(truth)) {

      // unknown origin (e.g. parent not in the record)
      if(truth->nParents() != 1) {
        origin = origin | (0x1 << InDet::TrkOrigin::OtherOrigin);
      }

      else {
        const xAOD::TruthParticle *parent = truth->parent(0);

        if(parent == nullptr) {
          origin = origin | (0x1 << InDet::TrkOrigin::OtherOrigin);
        }
        
        // sub-categorize secondaries which have one valid parent
        else {

          int pdgId = truth->pdgId();
          int parentId = parent->pdgId();

          // photon conversion
          if(parent->isPhoton() && truth->isElectron()) {
            origin = origin | (0x1 << InDet::TrkOrigin::GammaConversion);
          }

          // Strange Mesons
          else if(parent->isStrangeMeson() && parent->nChildren() == 2) {
            origin = origin | (0x1 << InDet::TrkOrigin::StrangeMesonDecay);
            // specifically Kshort
            if (abs(pdgId) == 211 && parentId == 310) {
              origin = origin | (0x1 << InDet::TrkOrigin::KshortDecay);
            }
          }

          // Strange Baryons
          else if(parent->isStrangeBaryon() && parent->nChildren() == 2) {
            origin = origin | (0x1 << InDet::TrkOrigin::StrangeBaryonDecay);
            // specifically Lambdas
            if ((abs(pdgId) == MC::PIPLUS || abs(pdgId) == MC::PROTON) && abs(parentId) == MC::LAMBDA0) {
              origin = origin | (0x1 << InDet::TrkOrigin::LambdaDecay);
            }
          }

          // other long living particle decays
          else if(parent->isHadron() && parent->nChildren() == 2) {
            origin = origin | (0x1 << InDet::TrkOrigin::OtherDecay);
          }

          // hadronic interactions
          else if(parent->nChildren() > 2) {
            origin = origin | (0x1 << InDet::TrkOrigin::HadronicInteraction);
          }

          // other secondaries
          else {
            origin = origin | (0x1 << InDet::TrkOrigin::OtherSecondary);
          }
        }
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
    return isFromRec( truth, flav, 0 );
  }

  bool InDetTrackTruthOriginTool::isFromRec(const xAOD::TruthParticle* truth, int flav, int depth) const {

    if ( truth == nullptr ) return false;

    if ( depth > 30 ) return false;

    if( flav != MC::BQUARK && flav != MC::CQUARK && flav != MC::TAU ) return false;

    if( flav == MC::BQUARK && truth->isBottomHadron() ) return true;

    if( flav == MC::CQUARK && truth->isCharmHadron() ) return true;

    if( flav == MC::TAU && MC::isTau(truth) ) return true;


    for(unsigned int p=0; p<truth->nParents(); p++) {
      const xAOD::TruthParticle* parent = truth->parent(p);
      if(parent == truth ) continue ; // avoid infinite recursion
      if( isFromRec(parent, flav, depth+1) ) return true;
    }

    return false;
  }

}
