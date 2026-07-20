/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloCalibHitRec/CaloCalibClusterDecoratorToolDM.h"

#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloDmDetDescr/CaloDmDescrManager.h"
#include "CaloSimEvent/CaloCalibrationHit.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "TruthUtils/MagicNumbers.h"

#include <algorithm>
#include <unordered_map>

namespace {

std::vector<std::pair<unsigned int, double> > makeSortedTruthPairs(
    const std::unordered_map<unsigned int, double>& truthMap,
    unsigned int maxTruthParticles)
{
  std::vector<std::pair<unsigned int, double> > truthPairs;
  truthPairs.reserve(truthMap.size());
  for (const auto& [pid, energy] : truthMap) {
    truthPairs.emplace_back(pid, energy);
  }

  std::sort(truthPairs.begin(),
            truthPairs.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });
  if (truthPairs.size() > maxTruthParticles) {
    truthPairs.resize(maxTruthParticles);
  }
  return truthPairs;
}

} // namespace

CaloCalibClusterDecoratorToolDM::CaloCalibClusterDecoratorToolDM(
    const std::string& type,
    const std::string& name,
    const IInterface* parent)
  : AthAlgTool(type, name, parent)
{
  declareInterface<CaloClusterCollectionProcessor>(this);

  for (int im = 0; im < 3; ++im) {
    m_i_phi_eta[im].resize(m_n_eta_out);
  }
}

StatusCode CaloCalibClusterDecoratorToolDM::initialize()
{
  ATH_CHECK(m_caloClusterWriteDecorHandleKeyNLeadingTruthParticlesDM.initialize());
  ATH_CHECK(detStore()->retrieve(m_calo_id, "CaloCell_ID"));
  m_caloDmDescrManager = CaloDmDescrManager::instance();

  CaloCalibClusterMomentsMaker2::initializeOutOfClusterDistanceTables(
      m_n_phi_out,
      m_n_eta_out,
      m_out_phi_max,
      m_out_eta_max,
      m_rmaxOut,
      m_i_phi_eta);

  ATH_CHECK(m_CalibrationHitContainerNames.initialize());
  ATH_CHECK(m_DMCalibrationHitContainerNames.initialize());

  return StatusCode::SUCCESS;
}

StatusCode CaloCalibClusterDecoratorToolDM::execute(
    const EventContext& ctx,
    xAOD::CaloClusterContainer* theClusColl) const
{
  SG::WriteDecorHandle<xAOD::CaloClusterContainer,
                       std::vector<std::pair<unsigned int, double> > >
    caloClusterWriteDecorHandleNLeadingTruthParticlesDM(
        m_caloClusterWriteDecorHandleKeyNLeadingTruthParticlesDM, ctx);

  bool foundAllContainers(true);
  std::vector<const CaloCalibrationHitContainer*> v_cchc;
  std::vector<const CaloCalibrationHitContainer*> v_dmcchc;

  for (const SG::ReadHandleKey<CaloCalibrationHitContainer>& key :
       m_CalibrationHitContainerNames) {
    SG::ReadHandle<CaloCalibrationHitContainer> cchc(key, ctx);
    if (!cchc.isValid()) {
      if (m_foundAllContainers) {
        ATH_MSG_ERROR("SG does not contain calibration hit container " << key.key());
      }
      foundAllContainers = false;
    }
    else {
      v_cchc.push_back(cchc.cptr());
    }
  }

  for (const SG::ReadHandleKey<CaloCalibrationHitContainer>& key :
       m_DMCalibrationHitContainerNames) {
    SG::ReadHandle<CaloCalibrationHitContainer> dmcchc(key, ctx);
    if (!dmcchc.isValid()) {
      if (m_foundAllContainers) {
        ATH_MSG_ERROR("SG does not contain DM calibration hit container " << key.key());
      }
      foundAllContainers = false;
    }
    else {
      v_dmcchc.push_back(dmcchc.cptr());
    }
  }

  if (!m_foundAllContainers && foundAllContainers) {
    m_foundAllContainers = true;
  }

  if (!foundAllContainers) {
    return StatusCode::SUCCESS;
  }

  ClusInfo_t clusInfoVec(theClusColl->size());

  CellInfoSet_t cellInfo;
  CaloCalibClusterMomentsMaker2::buildCellInfoMap(*theClusColl, cellInfo);

  unsigned int nHitsTotal = 0;
  unsigned int nHitsWithoutParticleUID = 0;
  CaloCalibClusterMomentsMaker2::accumulateClusterCalibHits(
      v_cchc,
      cellInfo,
      *m_calo_id,
      clusInfoVec,
      nHitsTotal,
      nHitsWithoutParticleUID,
      m_useParticleID,
      [this]() {
        ATH_MSG_ERROR("Invalid uniqueID detected - this sample cannot be properly analysed.");
      });

  bool useParticleID = m_useParticleID;
  if (m_useParticleID && (nHitsTotal == nHitsWithoutParticleUID)) {
    ATH_MSG_INFO("Calibration hits do not have ParticleUID, ids of particle-caused hits are always 0. Continuing without ParticleID machinery.");
    useParticleID = false;
  }

  std::array<ClusList, 3> clusLists;
  const std::array<bool, 3> doClusterLists{{
      m_MatchDmType == kMatchDmLoose,
      m_MatchDmType == kMatchDmMedium,
      m_MatchDmType == kMatchDmTight}};

  for (unsigned int ii = 0; ii < 3; ++ii) {
    if (doClusterLists[ii]) {
      clusLists[ii].resize((2 * m_n_phi_out + 1) * (2 * m_n_eta_out + 1));
    }
  }

  std::array<ClusList*, 3> clusListPtrs{{&clusLists[0], &clusLists[1], &clusLists[2]}};
  CaloCalibClusterMomentsMaker2::buildOutOfClusterClusterLists(
      *theClusColl,
      clusInfoVec,
      m_n_phi_out,
      m_n_eta_out,
      m_out_phi_max,
      m_out_eta_max,
      m_i_phi_eta,
      doClusterLists,
      clusListPtrs);

  const ClusList* pClusList = nullptr;
  if (m_MatchDmType == kMatchDmLoose) {
    pClusList = &clusLists[0];
  }
  else if (m_MatchDmType == kMatchDmMedium) {
    pClusList = &clusLists[1];
  }
  else if (m_MatchDmType == kMatchDmTight) {
    pClusList = &clusLists[2];
  }

  std::vector<std::unordered_map<unsigned int, double> > engCalibDeadByTruth(theClusColl->size());

  if (pClusList != nullptr) {
    accumulateDeadMaterialEnergy(
        v_dmcchc,
        *theClusColl,
        clusInfoVec,
        *pClusList,
        useParticleID,
        [&engCalibDeadByTruth](int iClus, unsigned int uniqueID, int /*nDmArea*/, double energy) {
          engCalibDeadByTruth[iClus][uniqueID] += energy;
        });
  }

  // Match ENG_CALIB_DEAD_TOT semantics: add the in-cluster dead-like samplings
  // that MomentsMaker2 adds on top of the assigned DMA_ALL dead-material energy.
  for (std::size_t iClus = 0; iClus < clusInfoVec.size(); ++iClus) {
    const auto& particleMap = clusInfoVec[iClus].engCalibParticle;
    for (const auto& [uniqueID, calibEnergy] : particleMap) {
      const double inClusterDeadEnergy =
          calibEnergy.engSmp[CaloSampling::PreSamplerB]
        + calibEnergy.engSmp[CaloSampling::PreSamplerE]
        + calibEnergy.engSmp[CaloSampling::TileGap3];
      if (inClusterDeadEnergy != 0.0) {
        engCalibDeadByTruth[iClus][uniqueID] += inClusterDeadEnergy;
      }
    }
  }

  int clusIdx = -1;
  for (const xAOD::CaloCluster* thisCaloCluster : *theClusColl) {
    ++clusIdx;
    const std::unordered_map<unsigned int, double>& truthMap = engCalibDeadByTruth[clusIdx];
    if (!truthMap.empty()) {
      caloClusterWriteDecorHandleNLeadingTruthParticlesDM(*thisCaloCluster) =
          makeSortedTruthPairs(truthMap, m_numTruthParticles);
    }
  }

  return StatusCode::SUCCESS;
}