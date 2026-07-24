/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Allows the user to search for particles with specified decay positions
// It will pass if there are at least MinPass many particles with LLP_PDGID
// that decay within the specified (cylindrical) volume
// xAOD/derivation-harmonised version of DecayVolumeFilter, operating on the
// slimmed xAOD::TruthParticleContainer instead of the HepMC event record.

#include <algorithm>
#include <cmath>

#include "GeneratorFilters/xAODDecayVolumeFilter.h"
#include "xAODTruth/TruthVertex.h"

// initialize
StatusCode xAODDecayVolumeFilter::filterInitialize() {
  CHECK(m_truthPartContKey.initialize());

  ATH_MSG_DEBUG("Configured cuts: "<< m_RCutMax<<", " << m_RCutMin<<", "<< m_zCutMax<<", "<< m_zCutMin);

  if (m_nBins>1){
    // initialize the histogram
    float lower_edge=std::min(m_RCutMin, m_zCutMin);
    float upper_edge=std::hypot(1.f*m_RCutMax, 1.f*m_zCutMax)-m_Rmargin;
    float bin_width=(upper_edge-lower_edge)/m_nBins;

    for (unsigned i=0;i<=m_nBins;++i){
      ATH_MSG_DEBUG("Bin " << i << " : ["<<lower_edge+i*bin_width<<" , "<<lower_edge+(i+1)*bin_width<<" )");
      m_bin_edges.push_back(lower_edge+i*bin_width);
      m_decay_radius[i]=0;
    }
    m_maxEntries=m_nEvents/m_nBins;
  }
  m_eventCounter=0;

  return StatusCode::SUCCESS;
}

// finalize
StatusCode xAODDecayVolumeFilter::filterFinalize() {
  ATH_MSG_INFO("Filter statistics");
  for (const auto& pair : m_stats) {
    ATH_MSG_INFO("Events with " << pair.first << " decays within specs : " << pair.second);
  }

  if (m_nBins>1){
    reportBinStatistics();
  }

  return StatusCode::SUCCESS;
}

// flat binning acceptance logic
bool xAODDecayVolumeFilter::accept_flatbinning(float r){
  auto it = std::upper_bound(m_bin_edges.begin(), m_bin_edges.end(), r);
  int bin = std::distance(m_bin_edges.begin(), it)-1;

  if ( bin<0 || bin>static_cast<int>(m_nBins) ) {
    ATH_MSG_VERBOSE("\t failed (bin index "<<bin<<"out of range [0,"<<m_nBins<<"]! ");
    return false;
  }

  if(m_decay_radius[bin]<m_maxEntries){
    ++m_decay_radius[bin];
    ATH_MSG_VERBOSE("\t passed! new bin content for bin "<<bin<<" : "<<m_decay_radius[bin]);
    return true;
  }
  ATH_MSG_VERBOSE("\t failed! bin content for bin "<<bin<<" : "<<m_decay_radius[bin]);
  return false;
}

// core filter logic
StatusCode xAODDecayVolumeFilter::filterEvent(const EventContext& ctx) {
  // Retrieve TruthGen container from xAOD Gen slimmer, contains all particles without barcode_zero and
  // duplicated barcode ones
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainer{m_truthPartContKey, ctx};
  CHECK(xTruthParticleContainer.isValid());

  int nPass = 0;
  // Loop over all particles in the (slimmed) truth particle container
  for (const xAOD::TruthParticle* part : *xTruthParticleContainer) {
    // Check decay position of LLP
    if ( std::abs(part->pdgId()) == m_LLP_PDGID ) {
      ATH_MSG_DEBUG("PDG: "<<part->pdgId()<<" -> "<<std::fabs(part->eta()));

      // Check the eta range
      if ( std::abs(part->eta())>m_MaxAbsEta ) {
        continue;
      }

      const xAOD::TruthVertex* decayVtx = part->decayVtx();
      if (decayVtx) {
        // Count only particles not decaying to themselves
        bool selfDecay = false;
        int nOutgoing = decayVtx->nOutgoingParticles();
        for (int i=0; i<nOutgoing; ++i) {
          const xAOD::TruthParticle* child = decayVtx->outgoingParticle(i);
          if ( child && child->pdgId() == part->pdgId() ) {
            selfDecay = true;
            break;
          }
        }

        if (selfDecay){
          continue;
        }

        float x = decayVtx->x();
        float y = decayVtx->y();
        float z = decayVtx->z();
        float Rdecay = std::sqrt(x*x + y*y);

        // Check if the point is within the outer cylinder
        bool isInOuterCylinder = (Rdecay < m_RCutMax) && (std::abs(z) < m_zCutMax);
        // Check if the point is outside the inner cylinder
        bool isOutsideInnerCylinder = (Rdecay > m_RCutMin) || (std::abs(z) > m_zCutMin);

        if(isInOuterCylinder && isOutsideInnerCylinder) {
          // only test for flat binning if event is not yet accepted
          if (m_nBins<=1 || nPass>=1 || accept_flatbinning(std::sqrt(x*x+y*y+z*z))){
            nPass++;
          }
        }
      }
    }
  }

  setFilterPassed(nPass >= m_MinPass, ctx);

  //fill statistics
  m_stats[nPass]++;
  m_eventCounter++;

  // Print stats every 10k events
  if (m_eventCounter%10000==0){
    reportBinStatistics();

    ATH_MSG_INFO("Success statistics: ");
    for (const auto& pair : m_stats) {
      ATH_MSG_INFO("Events with " << pair.first << " decays within specs: " << pair.second);
    }
  }

  return StatusCode::SUCCESS;
}


// stats printout
void xAODDecayVolumeFilter::reportBinStatistics() const {
  if (m_nBins>1){
    ATH_MSG_INFO("Bin statistics: ");
    for (unsigned i=0;i<=m_nBins;i++){
      ATH_MSG_INFO("Bin "<<i<<" : "<<m_decay_radius.at(i));
    }
  }
}
