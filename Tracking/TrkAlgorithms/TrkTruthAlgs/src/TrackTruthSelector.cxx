/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkTruthAlgs/TrackTruthSelector.h"

TrackTruthSelector::TrackTruthSelector(const std::string &name,ISvcLocator *pSvcLocator) : AthAlgorithm(name,pSvcLocator) {}

// -----------------------------------------------------------------------------------------------------
StatusCode TrackTruthSelector::initialize()
{
  m_subDetWeights[SubDetHitStatistics::Pixel] = m_weightPixel;
  m_subDetWeights[SubDetHitStatistics::SCT] = m_weightSCT;
  m_subDetWeights[SubDetHitStatistics::TRT] = m_weightTRT;
  m_subDetWeights[SubDetHitStatistics::MDT] = m_weightMDT;
  m_subDetWeights[SubDetHitStatistics::RPC] = m_weightRPC;
  m_subDetWeights[SubDetHitStatistics::TGC] = m_weightTGC;
  m_subDetWeights[SubDetHitStatistics::CSC] = m_weightCSC;
  m_subDetWeights[SubDetHitStatistics::STGC] = m_weightsTGC;
  m_subDetWeights[SubDetHitStatistics::MM] = m_weightMM;

  ATH_CHECK( m_detailedTrackTruthName.initialize() );
  ATH_CHECK( m_outputName.initialize() );
  return StatusCode::SUCCESS;
}

// -----------------------------------------------------------------------------------------------------
StatusCode TrackTruthSelector::execute() {
  ATH_MSG_DEBUG ("TrackTruthSelector::execute()");

  //----------------------------------------------------------------
  // Retrieve the input
  const DetailedTrackTruthCollection *detailed = nullptr;
  SG::ReadHandle<DetailedTrackTruthCollection> rh_detailed(m_detailedTrackTruthName);
  SG::WriteHandle<TrackTruthCollection> wh_output(m_outputName);
  if(!rh_detailed.isValid()){
    ATH_MSG_WARNING ("DetailedTrackTruthCollection "<<m_detailedTrackTruthName.key()<<" NOT found");
    return StatusCode::SUCCESS;
  } else {
    detailed = rh_detailed.cptr();
    ATH_MSG_DEBUG ("Got DetailedTrackTruthCollection "<<m_detailedTrackTruthName.key());
  }


  //----------------------------------------------------------------
  // Produce and store the output.

  TrackTruthCollection *out = new TrackTruthCollection(detailed->trackCollectionLink());

  fillOutput(out,detailed);

  ATH_CHECK(wh_output.record(std::unique_ptr<TrackTruthCollection>(out)));

  return StatusCode::SUCCESS;

}
//================================================================

void TrackTruthSelector::fillOutput(TrackTruthCollection *out, 
				    const DetailedTrackTruthCollection *in)
{
  using Iter = DetailedTrackTruthCollection::const_iterator;
  Iter itrackData=in->begin();
  while(itrackData!=in->end()) {
    std::pair<Iter,Iter> range = in->equal_range(itrackData->first);

    // We KNOW that the range is not empty - no need to check that.
    Iter selected = range.first;
    double bestProb = getProbability(selected->second);
    ATH_MSG_VERBOSE ("track=" << selected->first.index() << " prob=" << bestProb << " link: " << *(selected->second.trajectory().rbegin()));
    for(Iter imatch = ++range.first; imatch != range.second; ++imatch) {
      double prob = getProbability(imatch->second);
      ATH_MSG_VERBOSE ("track=" << imatch->first.index() << " prob=" << prob << " link: " << *(imatch->second.trajectory().rbegin()));
      if(prob>bestProb) {
	bestProb = prob;
	selected = imatch;
      }
    }

    // trajectory[0] is the LAST particle on the trajectory. The first
    // is at trajectory.rbegin(), but different trajectories can have
    // the same first particle.
    //    const HepMcParticleLink& particleLink = selected->second.trajectory()[0];
    const HepMcParticleLink& particleLink = *(selected->second.trajectory().rbegin());

    ATH_MSG_DEBUG ("Truth selected for track=" << selected->first.index() << " prob=" << bestProb << " link: " << particleLink);
    out->insert(std::make_pair(selected->first, TrackTruth(particleLink, bestProb, 0) ));
    itrackData=range.second;
  }

}

//================================================================
double TrackTruthSelector::getProbability(const DetailedTrackTruth& dt) const 
{
  double prd_track=0, prd_common=0;
  for(unsigned i=0; i<SubDetHitStatistics::NUM_SUBDETECTORS; i++) {
    prd_common += m_subDetWeights[i] * dt.statsCommon()[SubDetHitStatistics::SubDetType(i)];
    prd_track += m_subDetWeights[i] * dt.statsTrack()[SubDetHitStatistics::SubDetType(i)];
  }
  return (prd_track>0)? prd_common/prd_track : -1.;
}

//================================================================
