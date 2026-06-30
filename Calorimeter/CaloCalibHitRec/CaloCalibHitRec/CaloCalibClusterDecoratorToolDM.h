/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCALIBHITREC_CALOCALIBCLUSTERDECORATORTOOLDM_H
#define CALOCALIBHITREC_CALOCALIBCLUSTERDECORATORTOOLDM_H

class CaloCell_ID;
class CaloDmDescrManager;

#include "GaudiKernel/ToolHandle.h"

#include "src/CaloCalibClusterMomentsMaker2.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "CLHEP/Units/SystemOfUnits.h"

#include <array>
#include <cmath>
#include <atomic>
#include <string>
#include <vector>

/**
 * @class CaloCalibClusterDecoratorToolDM
 *
 * @brief Decorate clusters with dead-material truth-energy contributions.
 *
 * The tool mirrors the ENG_CALIB_DEAD_TOT dead-material sharing logic from
 * CaloCalibClusterMomentsMaker2, but keeps the assigned energy separated by
 * truth-particle uniqueID/barcode.  The final decoration is therefore a sorted
 * vector of (truth ID, dead-material calibration-hit energy) pairs per cluster.
 */
class CaloCalibClusterDecoratorToolDM
  : public AthAlgTool,
    virtual public CaloClusterCollectionProcessor
{
public:
  using CalibHitIPhiIEtaRange =
      CaloCalibClusterMomentsMaker2::CalibHitIPhiIEtaRange;
  using CellInfoSet_t = CaloCalibClusterMomentsMaker2::CellInfoSet_t;
  using ClusInfo_t = CaloCalibClusterMomentsMaker2::ClusInfo_t;
  using ClusList = CaloCalibClusterMomentsMaker2::ClusList;

  CaloCalibClusterDecoratorToolDM(const std::string& type,
                                  const std::string& name,
                                  const IInterface* parent);

  using CaloClusterCollectionProcessor::execute;
  virtual StatusCode execute(const EventContext& ctx,
                             xAOD::CaloClusterContainer* theClusColl) const override;
  virtual StatusCode initialize() override;

private:
  enum keys_dm_energy_sharing {kMatchDmOff, kMatchDmLoose, kMatchDmMedium, kMatchDmTight};

  SG::WriteDecorHandleKey<xAOD::CaloClusterContainer>
    m_caloClusterWriteDecorHandleKeyNLeadingTruthParticlesDM{
      this,
      "CaloClusterWriteDecorHandleKey_NLeadingTruthParticlesDM",
      "CaloTopoClustersNew.calclus_NLeadingTruthParticleBarcodeEnergyPairs_DM",
      "Dead-material truth-particle barcode/energy decoration"};

  Gaudi::Property<unsigned int> m_numTruthParticles{
      this,
      "NumTruthParticles",
      100,
      "Number of truth particles per CaloCluster/PFO for which to store dead-material calibration-hit energy"};

  SG::ReadHandleKeyArray<CaloCalibrationHitContainer> m_CalibrationHitContainerNames{
      this,
      "CalibrationHitContainerNames",
      {},
      "Calibration-hit containers inside the calorimeter volume"};

  SG::ReadHandleKeyArray<CaloCalibrationHitContainer> m_DMCalibrationHitContainerNames{
      this,
      "DMCalibrationHitContainerNames",
      {},
      "Dead-material calibration-hit containers"};

  const CaloCell_ID* m_calo_id{nullptr};
  const CaloDmDescrManager* m_caloDmDescrManager{nullptr};

  int m_n_phi_out{127};
  int m_n_eta_out{127};
  double m_out_phi_max{M_PI};
  double m_out_eta_max{6.0};
  double m_rmaxOut[3]{1.0, 0.5, 0.3};
  std::array<std::vector<std::vector<CalibHitIPhiIEtaRange> >, 3> m_i_phi_eta;

  Gaudi::Property<int> m_MatchDmType{
      this,
      "MatchDmType",
      kMatchDmLoose,
      "Dead-material matching type: 0=off, 1=loose, 2=medium, 3=tight"};

  Gaudi::Property<bool> m_useParticleID{
      this,
      "UseParticleID",
      true,
      "Use calibration-hit particle uniqueID for dead-material truth attribution"};

  Gaudi::Property<float> m_energyMin{
      this,
      "EnergyMin",
      200.0 * CLHEP::MeV,
      "Minimum cluster energy used in dead-material sharing"};

  Gaudi::Property<float> m_energyMinCalib{
      this,
      "EnergyMinCalib",
      20.0 * CLHEP::MeV,
      "Minimum in-cluster calibration energy used in dead-material sharing"};

  Gaudi::Property<float> m_apars_alpha{
      this,
      "AparsAlpha",
      0.5,
      "Power-law exponent for dead-material sharing effective energy"};

  Gaudi::Property<float> m_apars_r0{
      this,
      "AparsR0",
      0.2,
      "Distance scale for dead-material sharing effective energy"};

  template <class AddDeadMaterialEnergy>
  void accumulateDeadMaterialEnergy(
      const std::vector<const CaloCalibrationHitContainer*>& v_dmcchc,
      const xAOD::CaloClusterContainer& theClusColl,
      const ClusInfo_t& clusInfoVec,
      const ClusList& clusList,
      bool useParticleID,
      AddDeadMaterialEnergy&& addDeadMaterialEnergy) const
  {
    for (const CaloCalibrationHitContainer* dmcchc : v_dmcchc) {
      for (const CaloCalibrationHit* hit : *dmcchc) {
        const Identifier myId = hit->cellID();
        if (!m_calo_id->is_lar_dm(myId) && !m_calo_id->is_tile_dm(myId)) {
          continue;
        }

        const CaloDmDescrElement* myCDDE = m_caloDmDescrManager->get_element(myId);
        if (!myCDDE) {
          continue;
        }

        int uniqueID = HepMC::UNDEFINED_ID;
        if (useParticleID) {
          uniqueID = HepMC::uniqueID(hit);
        }

        const int jeO = static_cast<int>(std::floor(m_n_eta_out * (myCDDE->eta() / m_out_eta_max)));
        if (jeO < -m_n_eta_out || jeO >= m_n_eta_out) {
          continue;
        }

        int jpO = static_cast<int>(std::floor(m_n_phi_out * (myCDDE->phi() / m_out_phi_max)));
        if (jpO < -m_n_phi_out) {
          jpO += 2 * m_n_phi_out;
        }
        if (jpO >= m_n_phi_out) {
          jpO -= 2 * m_n_phi_out;
        }

        const int nDmArea = m_caloDmDescrManager->get_dm_area(myId);
        const CaloDmRegion* dmRegion = m_caloDmDescrManager->get_dm_region(myId);
        if (!dmRegion) {
          continue;
        }

        std::vector<int> hitClusIndex;
        std::vector<double> hitClusEffEnergy;
        hitClusIndex.reserve(theClusColl.size());
        hitClusEffEnergy.reserve(theClusColl.size());
        double hitClusNorm = 0.0;

        const std::vector<int>& matchingClusters =
            clusList[(jpO + m_n_phi_out) * (2 * m_n_eta_out + 1) + jeO + m_n_eta_out];

        for (int iClus : matchingClusters) {
          const xAOD::CaloCluster* theCluster = theClusColl.at(iClus);
          const auto& clusInfo = clusInfoVec[iClus];
          auto pos = clusInfo.engCalibParticle.find(uniqueID);
          if (pos == clusInfo.engCalibParticle.end()) {
            continue;
          }

          const double engClusTruthUniqueIDCalib = pos->second.engTot;
          if (engClusTruthUniqueIDCalib <= m_energyMinCalib || theCluster->e() <= m_energyMin) {
            continue;
          }

          double sum_smp_energy = 0.0;
          for (unsigned int i_smp = 0; i_smp < dmRegion->m_CaloSampleNeighbours.size(); ++i_smp) {
            const CaloSampling::CaloSample nsmp =
                static_cast<CaloSampling::CaloSample>(dmRegion->m_CaloSampleNeighbours[i_smp]);
            if ((dmRegion->m_CaloSampleEtaMin[i_smp] - 0.5) <= theCluster->eta() &&
                theCluster->eta() <= (dmRegion->m_CaloSampleEtaMax[i_smp] + 0.5)) {
              sum_smp_energy += pos->second.engSmp[nsmp];
            }
          }
          if (sum_smp_energy <= 0.0) {
            continue;
          }

          double phi_diff = myCDDE->phi() - theCluster->phi();
          if (phi_diff <= -M_PI) {
            phi_diff += 2. * M_PI;
          }
          else if (phi_diff > M_PI) {
            phi_diff -= 2. * M_PI;
          }
          const double eta_diff = myCDDE->eta() - theCluster->eta();
          const float distance = std::sqrt(eta_diff * eta_diff + phi_diff * phi_diff);
          const double effEner = std::pow(sum_smp_energy, m_apars_alpha.value()) * std::exp(-distance / m_apars_r0.value());

          hitClusIndex.push_back(iClus);
          hitClusEffEnergy.push_back(effEner);
          hitClusNorm += effEner;
        }

        if (hitClusNorm <= 0.0) {
          continue;
        }

        const double inv_hitClusNorm = 1.0 / hitClusNorm;
        for (std::size_t i = 0; i < hitClusIndex.size(); ++i) {
          const int iClus = hitClusIndex[i];
          const double dm_weight = hitClusEffEnergy[i] * inv_hitClusNorm;
          addDeadMaterialEnergy(iClus, uniqueID, nDmArea, hit->energyTotal() * dm_weight);
        }
      }
    }
  }

  mutable std::atomic<bool> m_foundAllContainers{false};
};

#endif // CALOCALIBHITREC_CALOCALIBCLUSTERDECORATORTOOLDM_H
