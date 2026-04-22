/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------
// File and Version Information:
// $Id: CaloCalibClusterTruthAttributerToolOOC.cxx,v 1.16 2009-05-18 16:16:49 pospelov Exp $
//
// Description: see CaloCalibClusterTruthAttributerToolOOC.h
//
// Environment:
//      Software developed for the ATLAS Detector at CERN LHC
//
// Author List:
//      Sven Menke
//
//-----------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "CaloCalibHitRec/CaloCalibClusterDecoratorToolOOC.h"

//---------------
// C++ Headers --
//---------------
#include <iterator>
#include <sstream>
#include <set>

#include "CaloEvent/CaloCell.h"
#include "CaloSimEvent/CaloCalibrationHit.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloIdentifier/CaloCell_ID.h"

#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "TruthUtils/MagicNumbers.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include <CLHEP/Vector/LorentzVector.h>
#include <cmath>

using CLHEP::HepLorentzVector;
using CLHEP::MeV;
using CLHEP::cm;

namespace {

/**
 * @brief Convert the per-cluster truth-energy bookkeeping map into a sorted payload.
 *
 * The input map stores, for one reconstructed cluster and one out-of-cluster
 * selection, the accumulated calibration-hit energy attributed to each truth
 * particle. The map key is the truth-particle barcode / uniqueID and the map
 * value is the corresponding accumulated calibration-hit energy.
 */
std::vector<std::pair<unsigned int, double> > makeSortedTruthPairs(
    const std::unordered_map<int, double>& truthMap,
    unsigned int maxTruthParticles)
{
  std::vector<std::pair<unsigned int, double> > truthPairs;
  truthPairs.reserve(truthMap.size());
  for (const auto& [pid, energy] : truthMap) {
    // The internal bookkeeping uses the signed uniqueID/barcode type propagated
    // by the calibration-hit helpers, while the final decoration stores the
    // identifier as unsigned.
    truthPairs.emplace_back(static_cast<unsigned int>(pid), energy);
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

//###############################################################################

CaloCalibClusterDecoratorToolOOC::CaloCalibClusterDecoratorToolOOC(const std::string& type,
                                                                   const std::string& name,
                                                                   const IInterface* parent)
  : AthAlgTool(type, name, parent),
    m_calo_id(nullptr)
{
  declareInterface<CaloClusterCollectionProcessor>(this);

  m_n_phi_out = 127; // not more than 127 since we store indices (-127,...,-1,0,...,126) with 8 bits
  m_n_eta_out = 127;
  m_out_phi_max = M_PI;
  m_out_eta_max = 6.;

  m_rmaxOut[0] = 1.0;
  m_rmaxOut[1] = 0.5;
  m_rmaxOut[2] = 0.3;

  for (int im = 0; im < 3; ++im) {
    m_i_phi_eta[im].resize(m_n_eta_out);
  }
}

//###############################################################################

StatusCode CaloCalibClusterDecoratorToolOOC::initialize()
{
  ATH_CHECK(m_caloClusterWriteDecorHandleKeyNLeadingTruthParticlesL.initialize());
  ATH_CHECK(m_caloClusterWriteDecorHandleKeyNLeadingTruthParticlesT.initialize());
  ATH_CHECK(m_truthParticleContainerKey.initialize());
  ATH_CHECK(detStore()->retrieve(m_calo_id, "CaloCell_ID"));

  CaloCalibClusterMomentsMaker2::initializeOutOfClusterDistanceTables(
      m_n_phi_out,
      m_n_eta_out,
      m_out_phi_max,
      m_out_eta_max,
      m_rmaxOut,
      m_i_phi_eta);

  ATH_CHECK(m_CalibrationHitContainerNames.initialize());
  ATH_CHECK(m_caloDetDescrMgrKey.initialize());

  return StatusCode::SUCCESS;
}

//###############################################################################

StatusCode
CaloCalibClusterDecoratorToolOOC::execute(const EventContext& ctx,
                                          xAOD::CaloClusterContainer* theClusColl) const
{
  SG::WriteDecorHandle<xAOD::CaloClusterContainer,
                       std::vector<std::pair<unsigned int, double> > >
    caloClusterWriteDecorHandleNLeadingTruthParticlesL(
        m_caloClusterWriteDecorHandleKeyNLeadingTruthParticlesL, ctx);
  SG::WriteDecorHandle<xAOD::CaloClusterContainer,
                       std::vector<std::pair<unsigned int, double> > >
    caloClusterWriteDecorHandleNLeadingTruthParticlesT(
        m_caloClusterWriteDecorHandleKeyNLeadingTruthParticlesT, ctx);

  ATH_MSG_DEBUG("Starting CaloCalibClusterTruthAttributerToolOOC::execute");
  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloDetDescrMgrKey, ctx};
  const CaloDetDescrManager* calo_dd_man = *caloMgrHandle;

  bool foundAllContainers(true);
  std::vector<const CaloCalibrationHitContainer*> v_cchc;

  // Only emit an error once the job has previously seen a complete set of
  // calibration-hit containers. This avoids logging one ERROR per event in
  // jobs where the containers are absent throughout, while still flagging a
  // transition from "containers present" to "containers missing".
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

  if (!m_foundAllContainers && foundAllContainers) {
    m_foundAllContainers = true;
  }

  if (!foundAllContainers) {
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG("SG has all containers");

  // Helper to store cluster calibration energies
  ClusInfo_t clusInfoVec(theClusColl->size());

  CaloCalibClusterMomentsMaker2::CellInfoSet_t cellInfo;
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
      true,
      [this]() {
        ATH_MSG_ERROR("Invalid uniqueID detected - this sample cannot be properly analysed.");
      });

  std::array<std::vector<std::unordered_map<int, double> >, 3> engCalibOut;
  std::array<ClusList, 3> clusLists;
  for (auto& clusList : clusLists) {
    clusList.resize((2 * m_n_phi_out + 1) * (2 * m_n_eta_out + 1));
  }
  for (auto& engByTruth : engCalibOut) {
    engByTruth.resize(theClusColl->size());
  }

  // This decorator always evaluates the loose, medium and tight OOC matching
  // definitions when building the helper lookup structures.
  const std::array<bool, 3> doOutOfCluster{{true, true, true}};
  const std::array<ClusList*, 3> clusListPtrs{{
      &clusLists[0], &clusLists[1], &clusLists[2]}};
  const std::array<const ClusList*, 3> constClusListPtrs{{
      &clusLists[0], &clusLists[1], &clusLists[2]}};

  CaloCalibClusterMomentsMaker2::buildOutOfClusterClusterLists(
      *theClusColl,
      clusInfoVec,
      m_n_phi_out,
      m_n_eta_out,
      m_out_phi_max,
      m_out_eta_max,
      m_i_phi_eta,
      doOutOfCluster,
      clusListPtrs);

  CaloCalibClusterMomentsMaker2::accumulateOutOfClusterEnergy(
      v_cchc,
      cellInfo,
      *calo_dd_man,
      clusInfoVec,
      m_n_phi_out,
      m_n_eta_out,
      m_out_phi_max,
      m_out_eta_max,
      doOutOfCluster,
      constClusListPtrs,
      [&engCalibOut](unsigned int ii, int iClus, int uniqueID, double energy) {
        engCalibOut[ii][iClus][uniqueID] += energy;
      });

  int clusIdx = -1;
  for (const xAOD::CaloCluster* thisCaloCluster : *theClusColl) {
    ++clusIdx;

    const std::unordered_map<int, double>& truthMapL = engCalibOut[0][clusIdx];
    if (!truthMapL.empty()) {
      caloClusterWriteDecorHandleNLeadingTruthParticlesL(*thisCaloCluster) =
          makeSortedTruthPairs(truthMapL, m_numTruthParticles);
    }

    const std::unordered_map<int, double>& truthMapT = engCalibOut[2][clusIdx];
    if (!truthMapT.empty()) {
      caloClusterWriteDecorHandleNLeadingTruthParticlesT(*thisCaloCluster) =
          makeSortedTruthPairs(truthMapT, m_numTruthParticles);
    }
  }

  return StatusCode::SUCCESS;
}
