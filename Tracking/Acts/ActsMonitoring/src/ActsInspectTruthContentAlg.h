/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMONITORING_INSPECTTRUTHCONTENTALG_H
#define ACTSMONITORING_INSPECTTRUTHCONTENTALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"
#include "ActsEvent/MeasurementToTruthParticleAssociation.h"
#include "ActsEvent/SeedContainer.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "ActsEvent/TrackContainer.h"

namespace ActsTrk {

  class ActsInspectTruthContentAlg
    : public AthReentrantAlgorithm {
  public:
    ActsInspectTruthContentAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~ActsInspectTruthContentAlg() override = default;
    
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual StatusCode finalize() override;

    // enum defs
    enum class EStatClusters : std::size_t {
      kNTotal,
      kNClustersFromPrimaries,
      kNClustersWithNoBarcode,
      kNClustersWith1Contribution,
      kNClustersWith1ValidContribution,
      kNClustersWith2Contribution,
      kNClustersWith2ValidContribution,
      kNClustersWith3Contribution,
      kNClustersWith3ValidContribution,
      kNClustersWith200kBarcode,
      kNStat
    };

    enum class EStatSeeds : std::size_t {
      kNTotal,
      nKSeedsWith0Matches,
      nKSeedsWith1Matches,
      nKSeedsWith2Matches,
      nKSeedsSame2Matches,
      nKSeedsWith3Matches,
      nKSeedsSame3Matches,
      nKSeedsWith4Matches,
      nKSeedsSame4Matches,
      nKSeedsWith5Matches,
      nKSeedsSame5Matches,
      nKSeedsWith6Matches,
      nKSeedsSame6Matches,
      kNStat
    };

    enum class EStatTracks : std::size_t {
      kNTotal,
      kNFullMatch,
      kNPerfectMatch,
      kNTracks0Holes,
      kNTracks1Holes,
      kNTracks2Holes,
      kNTracks3Holes,
      kNTracks0Outliers,
      kNTracks1Outliers,
      kNTracks2Outliers,
      kNTracks3Outliers,
      kNStat
    };

    enum class SeedType : std::size_t {
      PPP,
      SSS,
      PPS,
      PSS,
      Others,
      nTypes
    };

    enum class TrackType : std::size_t {
      Main,
      nTypes
    };
    
  private:
    static constexpr std::size_t s_nClusterTypes = static_cast<std::size_t>(xAOD::UncalibMeasType::nTypes);
    static constexpr std::size_t s_nSeedTypes = static_cast<std::size_t>(SeedType::nTypes);
    using cluster_stat_t = std::array<
      std::array<std::size_t, s_nClusterTypes>,
      static_cast<std::size_t>(EStatClusters::kNStat)>;
    using seed_stat_t = std::array<
      std::array<std::size_t, s_nSeedTypes>,
      static_cast<std::size_t>(EStatSeeds::kNStat)>;
    using track_stat_t = std::array<
      std::array<std::size_t, 1>,
      static_cast<std::size_t>(EStatTracks::kNStat)>;
    
    StatusCode fillStatClusters(const xAOD::UncalibratedMeasurementContainer& container,
				const ActsTrk::MeasurementToTruthParticleAssociation& truth,
				cluster_stat_t& stat) const;
    StatusCode fillStatSeeds(const ActsTrk::SeedContainer& seeds,
			     std::array<const ActsTrk::MeasurementToTruthParticleAssociation*, s_nClusterTypes>& truths,
			     seed_stat_t& stat) const;
    StatusCode fillStatTracks(const ActsTrk::TrackContainer& tracks,
			      std::array<const ActsTrk::MeasurementToTruthParticleAssociation*, s_nClusterTypes>& truths,
			      track_stat_t& trackStat,
			      cluster_stat_t& onTrackClusterStat) const;
    
    template <typename row_t,
	      typename coll_t,
	      typename stat_t>
    StatusCode printStatTables(const std::string& objectCollectionName,
			       const stat_t& stat) const;
    template <typename stat_t>
    StatusCode copyStatTable(const stat_t& contextual,
			     stat_t& global) const;
    
    SeedType deduceSeedType(const ActsTrk::Seed&) const;
    
    SG::ReadHandleKeyArray<xAOD::UncalibratedMeasurementContainer> m_clusters {this, "Clusters", {}};
    SG::ReadHandleKeyArray<ActsTrk::SeedContainer> m_seeds {this, "Seeds", {}};
    SG::ReadHandleKeyArray<ActsTrk::MeasurementToTruthParticleAssociation> m_associationMap_key {this,"TruthAssociationMaps", {}};
    SG::ReadHandleKeyArray<ActsTrk::TrackContainer> m_tracks {this, "Tracks", {}};
    
  private:
    mutable std::mutex m_mutex ATLAS_THREAD_SAFE {};
    mutable cluster_stat_t m_clusterStat ATLAS_THREAD_SAFE {};
    mutable seed_stat_t m_seedStat ATLAS_THREAD_SAFE {};
    mutable std::vector<std::pair<std::string, track_stat_t>> m_trackStat ATLAS_THREAD_SAFE {};
    mutable std::vector<std::pair<std::string, cluster_stat_t>> m_onTrack_clusterStat ATLAS_THREAD_SAFE {};
    
  private:
    inline std::string to_string(xAOD::UncalibMeasType type) const;
    inline std::string to_string(ActsInspectTruthContentAlg::SeedType type) const;
    inline std::string to_string(ActsInspectTruthContentAlg::TrackType type) const;
    inline std::string to_label(ActsInspectTruthContentAlg::EStatClusters type) const;
    inline std::string to_label(ActsInspectTruthContentAlg::EStatSeeds type) const;
    inline std::string to_label(ActsInspectTruthContentAlg::EStatTracks type) const;
  };

} // namespace

#include "src/ActsInspectTruthContentAlg.icc"

#endif 
