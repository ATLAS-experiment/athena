/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "electronSuperClusterBuilder.h"
//
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloUtils/CaloClusterStoreHelper.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"
#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "FourMomUtils/P4Helpers.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <cmath>
#include <memory>


namespace{

bool matchSameTrack(const xAOD::TrackParticle& seedTrack,
                    const egammaRec& sec) {
  const xAOD::TrackParticle* secTrack = sec.trackParticle();
  if (secTrack) {
    // Check that the tracks are the same.
    return seedTrack.index() == secTrack->index();
  }
  return false;
}
}  // namespace

electronSuperClusterBuilder::electronSuperClusterBuilder(
  const std::string& name,
  ISvcLocator* pSvcLocator)
  : egammaSuperClusterBuilderBase(name, pSvcLocator)
{
}

StatusCode
electronSuperClusterBuilder::initialize()
{
  ATH_MSG_DEBUG(" Initializing electronSuperClusterBuilder");

  // Additional Window we search in
  m_maxDelPhi = m_maxDelPhiCells * s_cellPhiSize * 0.5;
  m_maxDelEta = m_maxDelEtaCells * s_cellEtaSize * 0.5;

  // retrieve track match builder
  if (m_doTrackMatching) {
    ATH_CHECK(m_trackMatchBuilder.retrieve());
  }

  return egammaSuperClusterBuilderBase::initialize();
}


bool electronSuperClusterBuilder::egammaRecPassesSelection(const egammaRec *egRec) const {
  // We need tracks
  if (egRec->getNumberOfTrackParticles() == 0) {
    return false;
  }
  const xAOD::TrackParticle *trackParticle = egRec->trackParticle(0);
  using xAOD::EgammaHelpers::summaryValueInt;
  // with possible pixel
  uint8_t nPixelHits = summaryValueInt(*trackParticle, xAOD::numberOfPixelDeadSensors, 0);
  nPixelHits += summaryValueInt(*trackParticle, xAOD::numberOfPixelHits, 0); 
  if (nPixelHits < m_numberOfPixelHits) {
    return false;
  }
  // and with silicon (add SCT to pixel)
  uint8_t nSiHits = nPixelHits;
  nSiHits += summaryValueInt(*trackParticle, xAOD::numberOfSCTHits, 0);
  return nSiHits >= m_numberOfSiHits;
};

xAOD::EgammaParameters::EgammaType 
electronSuperClusterBuilder::getEgammaRecType([[maybe_unused]] const egammaRec *egRec) const {
  return xAOD::EgammaParameters::electron;
}

StatusCode
electronSuperClusterBuilder::redoMatching(
  const EventContext &ctx,
  SG::WriteHandle<EgammaRecContainer> &newEgammaRecs
) const {
  if (m_doTrackMatching) {
    ATH_CHECK(m_trackMatchBuilder->executeRec(ctx, newEgammaRecs.ptr()));
  }

  return StatusCode::SUCCESS;
}

std::vector<std::size_t>
electronSuperClusterBuilder::searchForSecondaryClusters(
  const std::size_t seedIndex,
  const EgammaRecContainer* egammaRecs,
  std::vector<bool>& isUsed) const
{
  // assume egammaRecs != 0, since the ReadHadler is valid
  // assume seed egammaRec has a valid cluster, since it has been already used
  std::vector<std::size_t> secondaryIndices;

  const auto* const seedEgammaRec = (*egammaRecs)[seedIndex];
  const xAOD::CaloCluster* const seedCaloClus = seedEgammaRec->caloCluster();

  const xAOD::TrackParticle* seedTrackParticle = seedEgammaRec->trackParticle();

  // Now loop over the potential secondary clusters
  for (std::size_t i = 0; i < egammaRecs->size(); ++i) {
    // if already used continue
    if (isUsed[i]) {
      continue;
    }

    const auto* const secEgammaRec = (*egammaRecs)[i];
    const xAOD::CaloCluster* const secClus = secEgammaRec->caloCluster();
    // Now perform a number of tests to see if the cluster should be added

    const auto seedSecdEta = std::abs(seedCaloClus->eta() - secClus->eta());
    const auto seedSecdPhi =
      std::abs(P4Helpers::deltaPhi(seedCaloClus->phi(), secClus->phi()));

    const bool addCluster =
      (matchesInWindow(seedCaloClus, secClus) ||
       ((seedSecdEta < m_maxDelEta && seedSecdPhi < m_maxDelPhi) &&
        (matchSameTrack(*seedTrackParticle, *secEgammaRec))));
    // Add it to the list of secondary clusters if it matches.
    if (addCluster) {
      secondaryIndices.push_back(i);
      isUsed[i] = true;
    }
  }
  ATH_MSG_DEBUG("Found: " << secondaryIndices.size() << " secondaries");
  return secondaryIndices;
}


