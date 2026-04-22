/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCALIBHITREC_CALOCALIBCLUSTERMOMENTSMAKER2_H
#define CALOCALIBHITREC_CALOCALIBCLUSTERMOMENTSMAKER2_H
/**
 * @class CaloCalibClusterMomentsMaker2
 * @version $Id: CaloCalibClusterMomentsMaker2.h,v 1.8 2009-05-18 16:16:48 pospelov Exp $
 * @author Sven Menke <menke@mppmu.mpg.de>, Gennady Pospelov <guennadi.pospelov@cern.ch>
 * @date 17-June-2008
 * @brief Calculate calibration hit based moments for CaloCluster objects using Primary Particle ID
 */

class CaloCell_ID;
class CaloDM_ID;
class CaloDmDescrManager;
class McEventCollection;
class TruthParticleContainer;

#include "GaudiKernel/ToolHandle.h"

#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include "CaloGeoHelpers/CaloSampling.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloDmDetDescr/CaloDmDescrManager.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "TruthUtils/MagicNumbers.h"

#include <string>
#include <vector>
#include <set>
#include <map>
#include <atomic>
#include <array>
#include <cmath>

class CaloCalibClusterMomentsMaker2: public AthAlgTool, virtual public CaloClusterCollectionProcessor
{
 public:

  class CalibHitIPhiIEtaRange {
   public:
    char iPhi, iEtaMin, iEtaMax;
  };

  class MyCellInfo : public std::vector<std::pair<int, double> > {
   public:
    MyCellInfo(int iClus, double w) { this->emplace_back(iClus, w); }
    void Add(const MyCellInfo& other) { this->insert(this->end(), other.begin(), other.end()); }
  };

  typedef std::map<Identifier, MyCellInfo> CellInfoSet_t;

  class MyClusInfo {
   public:
    class ClusCalibEnergy {
     public:
      double engTot = 0.0;
      std::array<double, CaloSampling::Unknown + 1> engSmp{};
      void Add(double eng, int nsmp)
      {
        engTot += eng;
        engSmp[nsmp] += eng;
      }
    };

    void Add(double eng, int nsmp, int pid = 0)
    {
      engCalibIn.Add(eng, nsmp);
      engCalibParticle[pid].Add(eng, nsmp);
    }

    ClusCalibEnergy engCalibIn;
    double engCalibOut = 0.0;
    double engCalibDead = 0.0;
    std::array<double, CaloDmDescrArea::DMA_MAX> engCalibDeadInArea{};
    std::map<int, ClusCalibEnergy> engCalibParticle{};
  };
  typedef std::vector<MyClusInfo> ClusInfo_t;
  using ClusList = std::vector<std::vector<int> >;

  typedef std::pair<std::string, xAOD::CaloCluster::MomentType> moment_name_pair;
  typedef std::vector<moment_name_pair> moment_name_vector;
  typedef std::set<moment_name_pair> moment_name_set;

  CaloCalibClusterMomentsMaker2(const std::string& type, const std::string& name,
                                const IInterface* parent);

  using CaloClusterCollectionProcessor::execute;
  virtual StatusCode execute(const EventContext& ctx,
                             xAOD::CaloClusterContainer* theClusColl) const override;
  virtual StatusCode initialize() override;

  /**
 * @brief Precompute eta/phi lookup tables for quick out-of-cluster hit association to clusters.
 *
 * Builds, for three search radius definitions (loose/medium/tight), a set
 * of eta-dependent lookup tables that map relative (Δη, Δφ) regions to
 * discrete index ranges. These tables are later used to efficiently find
 * clusters within a given angular distance of a calibration hit.
 *
 * The tables are organized as:
 * [working point][eta bin] -> vector of (phi bin, eta min/max) ranges.
 *
 * @param n_phi_out   Number of bins in φ used for the lookup grid.
 * @param n_eta_out   Number of bins in η used for the lookup grid.
 * @param out_phi_max Maximum |φ| range covered by the lookup grid.
 * @param out_eta_max Maximum |η| range covered by the lookup grid.
 * @param rmaxOut     Array of angular distance cuts (one per working point).
 * @param[out] i_phi_eta Output lookup tables to be filled.
 */
  static void initializeOutOfClusterDistanceTables(
      int n_phi_out,
      int n_eta_out,
      double out_phi_max,
      double out_eta_max,
      const double (&rmaxOut)[3],
      std::array<std::vector<std::vector<CalibHitIPhiIEtaRange> >, 3>& i_phi_eta);
  /**
 * @brief Build a map of calorimeter cells contributing to clusters.
 *
 * Iterates over all clusters and their constituent cells and constructs
 * a map keyed by cell Identifier. For each cell, stores the list of clusters
 * it contributes to together with the corresponding weights.
 *
 * This structure is used to efficiently associate calibration hits with
 * clusters during subsequent accumulation steps.
 *
 * @param theClusColl Input container of calorimeter clusters.
 * @param[out] cellInfo Map from cell Identifier to per-cluster contribution info.
 */
  static void buildCellInfoMap(const xAOD::CaloClusterContainer& theClusColl,
                               CellInfoSet_t& cellInfo);
  
  /**
 * @brief Build lookup lists of clusters for out-of-cluster energy sharing.
 *
 * For each working point (loose/medium/tight), constructs a grid-indexed
 * lookup structure that maps (η, φ) bins to lists of nearby clusters.
 * These lists are later used to distribute calibration hit energy to
 * clusters lying within the configured angular distance.
 *
 * The lookup is based on the precomputed tables from
 * initializeOutOfClusterDistanceTables().
 *
 * @param theClusColl Input container of calorimeter clusters.
 * @param clusInfoVec Per-cluster calibration information.
 * @param n_phi_out   Number of φ bins in the lookup grid.
 * @param n_eta_out   Number of η bins in the lookup grid.
 * @param out_phi_max Maximum |φ| range covered by the grid.
 * @param out_eta_max Maximum |η| range covered by the grid.
 * @param i_phi_eta   Precomputed lookup tables for each working point.
 * @param doOutOfCluster Flags enabling each working point.
 * @param[out] clusLists Output cluster index lists (non-owning pointers).
 */

  static void buildOutOfClusterClusterLists(
      const xAOD::CaloClusterContainer& theClusColl,
      const ClusInfo_t& clusInfoVec,
      int n_phi_out,
      int n_eta_out,
      double out_phi_max,
      double out_eta_max,
      const std::array<std::vector<std::vector<CalibHitIPhiIEtaRange> >, 3>& i_phi_eta,
      const std::array<bool, 3>& doOutOfCluster,
      const std::array<ClusList*, 3>& clusLists);
  
 
    /**
   * @brief Accumulate calibration-hit energy inside clusters.
   *
   * Loops over the input calibration hit containers and, for hits whose cells
   * belong to one or more clusters, adds the corresponding weighted energy to
   * the per-cluster calibration-energy bookkeeping.
   *
   * If @p useParticleID is true, the per-particle contribution map is also
   * filled and hits with undefined particle ID are counted. If false, only
   * the total in-cluster calibration energy and per-sampling contributions are
   * accumulated.
   *
   * @tparam InvalidUniqueIdHandler Callable invoked when an invalid unique ID
   *         is encountered.
   *
   * @param v_cchc Input calibration hit containers.
   * @param cellInfo Map from cell Identifier to per-cluster contribution info.
   * @param calo_id Calorimeter cell identifier helper.
   * @param[out] clusInfoVec Per-cluster calibration information to be updated.
   * @param[out] nHitsTotal Total number of calibration hits processed.
   * @param[out] nHitsWithoutParticleUID Number of hits with undefined particle ID.
   * @param useParticleID Flag controlling whether truth particle IDs are used.
   * @param invalidUniqueIdHandler Callback invoked when an invalid unique ID is found.
   */
  template <class InvalidUniqueIdHandler>
  static void accumulateClusterCalibHits(
      const std::vector<const CaloCalibrationHitContainer*>& v_cchc,
      const CellInfoSet_t& cellInfo,
      const CaloCell_ID& calo_id,
      ClusInfo_t& clusInfoVec,
      unsigned int& nHitsTotal,
      unsigned int& nHitsWithoutParticleUID,
      bool useParticleID,
      InvalidUniqueIdHandler&& invalidUniqueIdHandler)
  {
    for (const CaloCalibrationHitContainer* cchc : v_cchc) {
      for (const CaloCalibrationHit* hit : *cchc) {
        const Identifier myId = hit->cellID();
        typename CellInfoSet_t::const_iterator pos = cellInfo.find(myId);

        if (pos != cellInfo.end()) {
          const CaloSampling::CaloSample nsmp =
              CaloSampling::CaloSample(calo_id.calo_sample(myId));

          for (const std::pair<int, double>& p : pos->second) {
            const int iClus = p.first;
            const double weight = p.second;

            if (useParticleID) {
              const int uniqueID = HepMC::uniqueID(hit);
              if (uniqueID == HepMC::INVALID_PARTICLE_ID) {
                invalidUniqueIdHandler();
                break;
              }
              clusInfoVec[iClus].Add(weight * hit->energyTotal(),
                                     nsmp,
                                     static_cast<unsigned int>(uniqueID));
            }
            else {
              clusInfoVec[iClus].Add(weight * hit->energyTotal(), nsmp);
            }
          }
        }

        if (useParticleID && HepMC::uniqueID(hit) == HepMC::UNDEFINED_ID) {
          ++nHitsWithoutParticleUID;
        }
        ++nHitsTotal;
      }
    }
  }

  template <class AddOutOfClusterEnergy>
  static void accumulateOutOfClusterEnergy(
      const std::vector<const CaloCalibrationHitContainer*>& v_cchc,
      const CellInfoSet_t& cellInfo,
      const CaloDetDescrManager& calo_dd_man,
      const ClusInfo_t& clusInfoVec,
      int n_phi_out,
      int n_eta_out,
      double out_phi_max,
      double out_eta_max,
      const std::array<bool, 3>& doOutOfCluster,
      const std::array<const ClusList*, 3>& clusLists,
      AddOutOfClusterEnergy&& addOutOfClusterEnergy)
  {
    for (const CaloCalibrationHitContainer* cchc : v_cchc) {
      for (const CaloCalibrationHit* hit : *cchc) {
        const Identifier myId = hit->cellID();
        typename CellInfoSet_t::const_iterator pos = cellInfo.find(myId);
        if (pos != cellInfo.end()) {
          continue;
        }

        const CaloDetDescrElement* myCDDE = calo_dd_man.get_element(myId);
        const int uniqueID = HepMC::uniqueID(hit);
        if (!myCDDE) {
          continue;
        }

        const int jeO = static_cast<int>(std::floor(n_eta_out * (myCDDE->eta() / out_eta_max)));
        if (jeO < -n_eta_out || jeO >= n_eta_out) {
          continue;
        }

        int jpO = static_cast<int>(std::floor(n_phi_out * (myCDDE->phi() / out_phi_max)));
        if (jpO < -n_phi_out) jpO += 2 * n_phi_out;
        if (jpO >= n_phi_out) jpO -= 2 * n_phi_out;

        for (unsigned int ii = 0; ii < 3; ++ii) {
          const ClusList* pClusList = clusLists[ii];
          if (!doOutOfCluster[ii] || pClusList == nullptr) {
            continue;
          }

          double hitClusNorm = 0.0;
          std::vector<int> hitClusIndex;
          std::vector<double> hitClusEffEnergy;
          const std::vector<int>& matchingClusters =
              (*pClusList)[(jpO + n_phi_out) * (2 * n_eta_out + 1) + jeO + n_eta_out];

          for (int iClus : matchingClusters) {
            const MyClusInfo& clusInfo = clusInfoVec[iClus];
            auto partPos = clusInfo.engCalibParticle.find(uniqueID);
            if (partPos == clusInfo.engCalibParticle.end()) {
              continue;
            }
            hitClusNorm += partPos->second.engTot;
            hitClusEffEnergy.push_back(partPos->second.engTot);
            hitClusIndex.push_back(iClus);
          }

          if (hitClusNorm <= 0.0) {
            continue;
          }

          const double inv_hitClusNorm = 1.0 / hitClusNorm;
          for (std::size_t i = 0; i < hitClusIndex.size(); ++i) {
            const int iClus = hitClusIndex[i];
            const double w = hitClusEffEnergy[i] * inv_hitClusNorm;
            addOutOfClusterEnergy(ii, iClus, uniqueID, w * hit->energyTotal());
          }
        }
      }
    }
  }

 private:
  std::vector<std::string> m_momentsNames;
  moment_name_vector m_validNames;
  moment_name_set m_validMoments;
  std::vector<std::string> m_momentsNamesAOD;
  std::set<xAOD::CaloCluster::MomentType> m_momentsAOD;
  SG::ReadHandleKeyArray<CaloCalibrationHitContainer> m_CalibrationHitContainerNames;
  SG::ReadHandleKeyArray<CaloCalibrationHitContainer> m_DMCalibrationHitContainerNames;
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleContainerKey{this,"TruthParticles","TruthParticles","ReadHandleKey for truth particle container"};
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloDetDescrMgrKey{this,"CaloDetDescrManager", "CaloDetDescrManager"};
  const CaloCell_ID* m_calo_id;
  const CaloDM_ID* m_caloDM_ID;
  const CaloDmDescrManager* m_caloDmDescrManager;
  int m_n_phi_out;
  int m_n_eta_out;
  double m_out_phi_max;
  double m_out_eta_max;
  double m_rmaxOut[3];
  std::array<std::vector<std::vector<CalibHitIPhiIEtaRange> >, 3> m_i_phi_eta;
  mutable std::atomic<bool> m_foundAllContainers{};
  enum keys_dm_energy_sharing {kMatchDmOff, kMatchDmLoose, kMatchDmMedium, kMatchDmTight};
  enum keys_calib_frac_origin {kCalibFracEM, kCalibFracHAD, kCalibFracREST, kCalibFracMax};
  bool m_doDeadEnergySharing;
  bool m_doOutOfClusterL;
  bool m_doOutOfClusterM;
  bool m_doOutOfClusterT;
  bool m_doDeadL;
  bool m_doDeadM;
  bool m_doDeadT;
  bool m_useParticleID;
  bool m_doCalibFrac;
  float m_energyMin;
  float m_energyMinCalib;
  float m_apars_alpha;
  float m_apars_r0;
  int m_MatchDmType;

  static double angle_mollier_factor(double x);
};

#endif // CALOCALIBCLUSTERMOMENTSMAKER2_H
