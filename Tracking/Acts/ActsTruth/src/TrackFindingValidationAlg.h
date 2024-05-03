/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_TRACKFINDINGVALIDATIONALG_H
#define ACTSTRK_TRACKFINDINGVALIDATIONALG_H 1

#undef NDEBUG
// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Gaudi includes
#include "Gaudi/Property.h"

// Handle Keys
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "ActsEvent/TrackToTruthParticleAssociation.h"
#include "ActsEvent/TruthParticleHitCounts.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "TrkTruthTrackInterfaces/IAthSelectionTool.h"

#include <mutex>
#include "ActsInterop/StatUtils.h"
#include "ActsInterop/TableUtils.h"

#include <string>
#include <memory>
#include <array>
#include <atomic>
#include <type_traits>

#include <cmath>
#include <iomanip>
#include <ostream>
#include <string>
#include <sstream>
#include <vector>

namespace ActsTrk
{
  constexpr bool TrackFindingValidationDebugHists = false;

  class TrackFindingValidationAlg : public AthReentrantAlgorithm
  {
  public:
    TrackFindingValidationAlg(const std::string &name,
                               ISvcLocator *pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
     SG::ReadHandleKey<TruthParticleHitCounts>  m_truthHitCounts
        {this, "TruthParticleHitCounts","", "Map from truth particle to hit counts." };
     SG::ReadHandleKey<TrackToTruthParticleAssociation>  m_trackToTruth
        {this, "TrackToTruthAssociationMap","", "Association map from tracks to generator particles." };

     Gaudi::Property<std::vector<float> > m_weightsForProb
        {this, "MatchWeights", {}, "Weights applied to the counts per measurement type for weighted sums"
                                  " which are used compute the match probability." };
     Gaudi::Property<std::vector<float> > m_weights
        {this, "CountWeights", {}, "Weights applied to the counts per measurement type for weighted sums"
                                   " which are used to compute hit efficiencies and purities." };

     Gaudi::Property<std::vector<float>> m_statEtaBins
        {this, "StatisticEtaBins", {-4, -2.6, -2, 0, 2., 2.6, 4}, "Gather statistics separately for these eta bins."};
     Gaudi::Property<std::vector<float>> m_statPtBins
        {this, "StatisticPtBins", {1.e3,2.5e3,10e3, 100e3}, "Gather statistics separately for these pt bins."};
     Gaudi::Property<bool> m_pdgIdCategorisation
        {this, "PdgIdCategorisation", false, "Categorise by pdg id."};
     Gaudi::Property<bool> m_showRawCounts
        {this, "ShowRawCounts", false, "Show all counters."};
     Gaudi::Property<bool> m_printDetails
        {this, "ShowDetailedTables", false, "Show more details; stat. uncert., RMS, entries"};

    ToolHandle<IAthSelectionTool> m_truthSelectionTool{this, "TruthSelectionTool","AthTruthSelectionTool", "Truth selection tool (for efficiencies and resolutions)"};

     // helper struct for compile time optional statistics
     template <bool IsDebug>
     struct DebugCounter {
        struct Empty {
           template <typename... T_Args>
           Empty(T_Args... ) {}
        };
        mutable typename std::conditional<IsDebug,
                                          std::mutex,
                                          Empty>::type m_mutex ATLAS_THREAD_SAFE;
        mutable typename std::conditional<IsDebug,
                                          ActsUtils::StatHist,
                                          Empty>::type m_measPerTruthParticleWithoutCounts ATLAS_THREAD_SAFE {20,-.5,20.-.5};
        mutable typename std::conditional<IsDebug,
                                          ActsUtils::StatHist,
                                          Empty>::type m_bestMatchProb ATLAS_THREAD_SAFE {20,0.,1.};
        mutable typename std::conditional<IsDebug,
                                          ActsUtils::StatHist,
                                          Empty>::type m_nextToBestMatchProb ATLAS_THREAD_SAFE {20,0.,1.};

        template <class T_OutStream>
        void dumpStatistics(T_OutStream &out) const;
        void fillMeasForTruthParticleWithoutCount(double weighted_measurement_sum) const;
        void fillTruthMatchProb(const std::array<float,2> &best_match_prob) const;
     };
     DebugCounter<TrackFindingValidationDebugHists> m_debugCounter;

     // s_NMeasurementTypes is equal to the number of UncalibMeasType, but we have to remove the "Other" option, which
     // corresponds to an unknown type
     constexpr static unsigned int s_NMeasurementTypes = static_cast<unsigned int>(xAOD::UncalibMeasType::nTypes) - 1u;

     // statistics counter
     enum ECounter {
        NTracksTotal,
        NTruthWithCountsTotal,
        MissingTruthParticleHitCounts,
        NoAssociatedTruthParticle,
        NoSelectedTruthParticle,
        TruthParticleNoNoiseMismatch,
        kNCounter
     };
     //     mutable std::array< std::atomic<std::size_t>, kNCounter > m_counter ATLAS_THREAD_SAFE {};
     mutable std::array< std::size_t, kNCounter > m_counter ATLAS_THREAD_SAFE {};

     enum ECategorisedCounter {
        kNTotalParticles,
        kNParticleWithAssociatedTrack,
        kNParticleWithMultipleAssociatedTracks,
        kNTotalTracks,
        kNCategorisedCounter
     };
     enum ECategorisedStat {
        kHitEfficiency,
        kHitPurity,
        kMatchProbability,
        kNCategorisedStat
     };

     constexpr static int s_pdgIdMax = 1000000000;  // categorise all truth particles with this or larger PDG ID as "Other"

     mutable std::mutex m_statMutex ATLAS_THREAD_SAFE;

     mutable std::vector< std::array< std::size_t, kNCategorisedCounter> >  m_counterPerEta ATLAS_THREAD_SAFE;
     mutable std::vector< int >                                             m_pdgId ATLAS_THREAD_SAFE;
     mutable std::vector< std::array< std::size_t, kNCategorisedCounter> >  m_counterPerPdgId ATLAS_THREAD_SAFE;
     mutable std::vector< std::array< ActsUtils::Stat, kNCategorisedStat> > m_statPerEta ATLAS_THREAD_SAFE;
     mutable std::vector< std::array< ActsUtils::Stat, kNCategorisedStat> > m_statPerPdgId ATLAS_THREAD_SAFE;
     mutable ActsUtils::StatHist                                            m_truthSelectionCuts ATLAS_THREAD_SAFE;

     bool m_useAbsEtaForStat = false;

     /// @brief check that bins are in increasing order.
     /// will cause a FATAL error if bins are not in increasing order.
     void checkBinOrder( const std::vector<float> &bin_edges, const std::string &bin_label) const;

     /// @brief Return the category based on the provided eta value
     /// @param pt the pt of the truth particle
     /// @param eta the eta of the truth particle
     /// @return a bin assigned to the give eta value
     std::size_t getPtEtaStatCategory(float pt, float eta) const;

     /// @brief Return the category based on the PDG ID
     /// @param pt the pt of the truth particle
     /// @param pdg_id the PDG ID
     /// @return 0 or a slot associated to a single PDG ID (absolut value)
     std::size_t getPtPdgIdStatCategory(float pt, int pdg_id) const;
     void initStatTables();
     void printStatTables() const;
     void printCategories(const std::vector<std::string> &pt_category_labels,
                          const std::vector<std::string> &eta_category_labels,
                          std::vector<std::string> &counter_labels,
                          std::vector< std::array< ActsUtils::Stat, kNCategorisedStat> > &stat_per_category,
                          std::vector< std::array< std::size_t, kNCategorisedCounter> >  &counts_per_category,
                          const std::string &top_left_label,
                          bool print_sub_categories) const;
     void printData2D(const std::vector<std::string> &row_category_labels,
                      const std::vector<std::string> &col_category_labels,
                      const std::string &top_left_label,
                      std::vector< std::array< ActsUtils::Stat, kNCategorisedStat> > &stat_per_category,
                      std::vector< std::array< std::size_t, kNCategorisedCounter> >  &counts_per_category,
                      bool rotate) const;

     StatusCode checkMatchWeights();

     static  double weightedCountSum(const ActsTrk::HitCounterArray &counts,
                                     const std::vector<float> &weights);
     static  double noiseCorrection(const ActsTrk::HitCounterArray &noise_counts,
                                    const std::vector<float> &weights);
  };

} // namespace

#endif
