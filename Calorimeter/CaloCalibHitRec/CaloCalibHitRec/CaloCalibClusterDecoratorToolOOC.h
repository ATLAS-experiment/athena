/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCALIBHITREC_CALOCALIBCLUSTERDECORATORTOOLOOC_H
#define CALOCALIBHITREC_CALOCALIBCLUSTERDECORATORTOOLOOC_H

class CaloCell_ID;
class CaloDetDescrManager;

#include "CaloDetDescr/CaloDetDescrManager.h"
#include "GaudiKernel/ToolHandle.h"

#include "src/CaloCalibClusterMomentsMaker2.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODTruth/TruthParticleContainer.h"

#include <string>
#include <vector>
#include <atomic>
#include <array>
#include <unordered_map>

/**
 * @class CaloCalibClusterDecoratorToolOOC
 *
 * @brief Decorate clusters with out-of-cluster truth-energy contributions.
 *
 * The tool attributes calibration-hit energy deposited outside the reconstructed
 * cluster to the contributing truth particles and decorates each cluster with
 * the leading truth-particle barcode/energy pairs for the loose and tight
 * out-of-cluster definitions.
 */
class CaloCalibClusterDecoratorToolOOC
  : public AthAlgTool,
    virtual public CaloClusterCollectionProcessor
{
public:
  using CalibHitIPhiIEtaRange =
      CaloCalibClusterMomentsMaker2::CalibHitIPhiIEtaRange;
  using MyCellInfo = CaloCalibClusterMomentsMaker2::MyCellInfo;
  using CellInfoSet_t = CaloCalibClusterMomentsMaker2::CellInfoSet_t;
  using MyClusInfo = CaloCalibClusterMomentsMaker2::MyClusInfo;
  using ClusInfo_t = CaloCalibClusterMomentsMaker2::ClusInfo_t;
  using ClusList = CaloCalibClusterMomentsMaker2::ClusList;

  CaloCalibClusterDecoratorToolOOC(const std::string& type,
                                   const std::string& name,
                                   const IInterface* parent);

  using CaloClusterCollectionProcessor::execute;
  virtual StatusCode execute(const EventContext& ctx,
                             xAOD::CaloClusterContainer* theClusColl) const override;
  virtual StatusCode initialize() override;

private:
  /**
   * @brief Writehandle for the decoration holding the leading truth-particle barcode/energy pairs
   * for the loose out-of-cluster definition.
   */
  SG::WriteDecorHandleKey<xAOD::CaloClusterContainer>
    m_caloClusterWriteDecorHandleKeyNLeadingTruthParticlesL{
      this,
      "CaloClusterWriteDecorHandleKey_NLeadingTruthParticlesL",
      "CaloTopoClustersNew.calclus_NLeadingTruthParticleBarcodeEnergyPairs_L",
      "Loose OOC truth-particle barcode/energy decoration"};

  /**
   * @brief Writehandle for the decoration holding the leading truth-particle barcode/energy pairs
   * for the tight out-of-cluster definition.
   */
  SG::WriteDecorHandleKey<xAOD::CaloClusterContainer>
    m_caloClusterWriteDecorHandleKeyNLeadingTruthParticlesT{
      this,
      "CaloClusterWriteDecorHandleKey_NLeadingTruthParticlesT",
      "CaloTopoClustersNew.calclus_NLeadingTruthParticleBarcodeEnergyPairs_T",
      "Tight OOC truth-particle barcode/energy decoration"};

  /**
   * @brief Number of leading truth particles to store per cluster.
   */
  Gaudi::Property<unsigned int> m_numTruthParticles{
      this,
      "NumTruthParticles",
      100,
      "Number of truth particles per CaloCluster/PFO for which to store OOC calibration-hit energy"};

  /** 
   * @brief vector of calibration hit container names to use. 
   *
   * The containers specified in this property should hold calibration
   * hits inside the calorimeter systems. */

  SG::ReadHandleKeyArray<CaloCalibrationHitContainer> m_CalibrationHitContainerNames{
      this,
      "CalibrationHitContainerNames",
      {},
      "Calibration-hit containers inside the calorimeter volume"};

  /**
   * @brief Readhandle for Truth-particle container used to validate barcode / uniqueID usage.
   */
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleContainerKey{
      this,
      "TruthParticles",
      "TruthParticles",
      "ReadHandleKey for truth particle container"};

  /**
   * @brief Conditions Handle Key to access the CaloDetDescrManager.
   */
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloDetDescrMgrKey{
      this,
      "CaloDetDescrManager",
      "CaloDetDescrManager",
      "Conditions handle for the CaloDetDescrManager"};

  /** @brief Cached pointer to the calorimeter cell identifier helper. */
  const CaloCell_ID* m_calo_id;

  /**
   * @brief Number of bins in the lookup table for out-of-cluster phi offsets.
   *
   * The table stores offsets in signed 8-bit-compatible index ranges.
   */
  int m_n_phi_out;

  /**
   * @brief Number of bins in the lookup table for out-of-cluster eta offsets.
   */
  int m_n_eta_out;

  /** @brief Maximum |Δphi| covered by the out-of-cluster lookup tables. */
  double m_out_phi_max;

  /** @brief Maximum |Δeta| covered by the out-of-cluster lookup tables. */
  double m_out_eta_max;

  /**
   * @brief Maximum matching radii for the loose/medium/tight OOC definitions.
   */
  double m_rmaxOut[3];

  /**
   * @brief Precomputed eta/phi search windows used by the OOC helper methods.
   */
  std::array<std::vector<std::vector<CalibHitIPhiIEtaRange> >, 3> m_i_phi_eta;

  /**
   * @brief Tracks whether a previous event had the full set of calibration-hit containers.
   *
   * This is used to suppress repeated ERROR messages in jobs where the
   * containers are absent for every event, while still reporting a transition
   * from present to missing containers.
   */
  mutable std::atomic<bool> m_foundAllContainers{false};
};

#endif // CALOCALIBHITREC_CALOCALIBCLUSTERDECORATORTOOLOOC_H
