/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_TRACKTRUTHMATCHINGBASEALG_H
#define ACTSTRK_TRACKTRUTHMATCHINGBASEALG_H 1

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
#include <sstream>
#include <vector>

namespace ActsTrk
{
  constexpr bool TrackFindingValidationDebugHists = false;
  constexpr bool TrackFindingValidationDetailedStat = true;

  class TrackTruthMatchingBaseAlg : public AthReentrantAlgorithm
  {
     template <bool DetailEnabled>
     struct BaseStat;

     template <bool DetailEnabled>
     friend struct BaseStat;

  public:
    TrackTruthMatchingBaseAlg(const std::string &name,
                               ISvcLocator *pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

  protected:
     const TruthParticleHitCounts &getTruthParticleHitCounts(const EventContext &ctx) const {
        SG::ReadHandle<TruthParticleHitCounts> truth_particle_hit_counts_handle = SG::makeHandle(m_truthHitCounts, ctx);
        if (!truth_particle_hit_counts_handle.isValid()) {
           ATH_MSG_ERROR("No truth particle hit count map for key " << m_truthHitCounts.key() );
           std::runtime_error("Failed to get truth particle hit count map");
        }
        return *truth_particle_hit_counts_handle;
     }

     const IAthSelectionTool &truthSelectionTool() const { return *m_truthSelectionTool; }

     std::size_t perEtaSize() const  {
        return m_detailedStat.perEtaSize();
     }
     std::size_t perPdgIdSize() const {
        return m_detailedStat.perPdgIdSize();
     }

     template <bool DetailEnabled>
     struct EventStatBase : public BaseStat<DetailEnabled> {
        static constexpr bool doDetail = DetailEnabled;
        
        EventStatBase(const IAthSelectionTool &truth_selection_tool,
                  std::size_t per_eta_size,
                  std::size_t per_pdg_size,
                  [[maybe_unused]] std::size_t track_to_truth_size)
           : BaseStat<DetailEnabled>(truth_selection_tool, per_eta_size, per_pdg_size),
             m_nTruthCuts(truth_selection_tool.nCuts())
        {
           if constexpr(DetailEnabled) {
              m_truthParticlesWithAssociatedTrack.reserve(track_to_truth_size);
           }

        };
        void fill([[maybe_unused]] unsigned int eta_category_i,
                  [[maybe_unused]] unsigned int pdg_id_category_i,
                  [[maybe_unused]] float hit_efficiency,
                  [[maybe_unused]] float hit_purity,
                  [[maybe_unused]] float match_prob,
                  [[maybe_unused]] const xAOD::TruthParticle *best_match) {
           BaseStat<DetailEnabled>::fill(eta_category_i,
                                         pdg_id_category_i,
                                         hit_efficiency,
                                         hit_purity,
                                         match_prob);
           if constexpr(DetailEnabled) {
              if (!m_truthParticlesWithAssociatedTrack.insert(best_match).second) {
                 // truth particle already had a best match
                 ++this->m_counterPerEta[eta_category_i][kNParticleWithMultipleAssociatedTracks];
                 ++this->m_counterPerPdgId[pdg_id_category_i][kNParticleWithMultipleAssociatedTracks];
              }
              else {
                 ++this->m_counterPerEta[eta_category_i][kNParticleWithAssociatedTrack];
                 ++this->m_counterPerPdgId[pdg_id_category_i][kNParticleWithAssociatedTrack];
              }
           }
        }
        using TruthParticleSet = std::conditional< DetailEnabled,
                                                   std::unordered_set<const xAOD::TruthParticle *>,
                                                   typename BaseStat<DetailEnabled>::Empty >::type;
        TruthParticleSet m_truthParticlesWithAssociatedTrack;

        unsigned int m_nTruthParticleWithoutAssociatedCounts =0u;
        unsigned int m_nTracksWithoutAssociatedTruthParticle =0u;
        unsigned int m_nTracksWithoutSelectedTruthParticle =0u;
        unsigned int m_nTruthParticleNonoiseMismatches=0u;

        unsigned int m_nTruthCuts;
     };
     using EventStat = EventStatBase<TrackFindingValidationDetailedStat>;

     /** Match result returned by @ref analyseTrackTruth
      */
     struct TruthMatchResult {
        const xAOD::TruthParticle *m_truthParticle;  ///< best matching truth particle or nullptr
        float m_matchProbability;                    ///< the matching probability based on weighted hit sums
        float m_hitPurity;                           ///< fraction of hits originting from best match over total reco hits
        float m_hitEfficiency;                       ///< fraction of hits originting from best match over total best match hits
     };
     /**
         @param return tuple containing pointer to best matching truth particle, matching probability, hit purity and hit efficiency.
     */
     TruthMatchResult
     analyseTrackTruth(const TruthParticleHitCounts &truth_particle_hit_counts,
                       const HitCountsPerTrack &track_hit_counts,
                       EventStat &event_stat) const;

     void postProcessEventStat(const TruthParticleHitCounts &truth_particle_hit_counts,
                               std::size_t n_tracks,
                               EventStat &event_stat) const;
     
  private:
     SG::ReadHandleKey<TruthParticleHitCounts>  m_truthHitCounts
        {this, "TruthParticleHitCounts","", "Map from truth particle to hit counts." };

     Gaudi::Property<std::vector<float> > m_weightsForProb
        {this, "MatchWeights", {}, "Weights applied to the counts per measurement type for weighted sums"
                                  " which are used compute the match probability." };
     Gaudi::Property<std::vector<float> > m_weights
        {this, "CountWeights", {}, "Weights applied to the counts per measurement type for weighted sums"
                                   " which are used to compute hit efficiencies and purities." };

     // Empty struct emulating the interface of a Gaudi property to replace optional properties if disabled.
     template <class Base>
     struct DummyProperty {
        template <class OWNER>
        DummyProperty( OWNER*, std::string, Base&&, std::string ) {}
        Base value() const { return Base{}; }
        operator Base() const { return Base{}; }
        
        // some dummy implementations for vector properties:
        std::size_t size() const {return 0u; }
        bool empty() const {return true; }
        auto operator[](std::size_t /*idx*/) { throw std::out_of_range("DummyProperty");}

        // Delegate operator() to the value
        template <class... Args>
        decltype( std::declval<Base>()( std::declval<Args&&>()... ) ) operator()( Args&&... args ) const
           noexcept( noexcept( std::declval<Base>()( std::declval<Args&&>()... ) ) ) {
           return value()( std::forward<Args>( args )... );
        }
     };

     template <class Base>
     using Property = std::conditional< TrackFindingValidationDetailedStat, Gaudi::Property<Base>, DummyProperty<Base> >::type;
     
     Property<std::vector<float>> m_statEtaBins
        {this, "StatisticEtaBins", {-4, -2.6, -2, 0, 2., 2.6, 4}, "Gather statistics separately for these eta bins."};
     Property<std::vector<float>> m_statPtBins
        {this, "StatisticPtBins", {1.e3,2.5e3,10e3, 100e3}, "Gather statistics separately for these pt bins."};
     Property<bool> m_pdgIdCategorisation
        {this, "PdgIdCategorisation", false, "Categorise by pdg id."};
     Property<bool> m_showRawCounts
        {this, "ShowRawCounts", false, "Show all counters."};
     Property<bool> m_printDetails
        {this, "ShowDetailedTables", false, "Show more details; stat. uncert., RMS, entries"};
     Property<bool> m_computeTrackRecoEfficiency
        {this, "ComputeTrackRecoEfficiency", true, "Compute and print track reconstruction efficiency."};

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

     // s_NMeasurementTypes is equal to the number of UncalibMeasType
     constexpr static unsigned int s_NMeasurementTypes = static_cast<unsigned int>(xAOD::UncalibMeasType::nTypes);

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


     bool m_useAbsEtaForStat = false;

     template <bool DetailEnabled>
     struct BaseStat {
        BaseStat() = default;
        BaseStat([[maybe_unused]] const IAthSelectionTool &truth_selection_tool,
                    [[maybe_unused]] std::size_t per_eta_size,
                    [[maybe_unused]] std::size_t per_pdg_size)
           : m_truthSelectionCuts(truth_selection_tool.nCuts()+1, -0.5,truth_selection_tool.nCuts()+.5)
        {
           if constexpr(DetailEnabled) {
              m_counterPerEta.resize(per_eta_size);
              m_counterPerPdgId.resize( per_pdg_size);
              m_statPerEta.resize( per_eta_size );
              m_statPerPdgId.resize( per_pdg_size );
           }

        }
        void reset(const IAthSelectionTool &truth_selection_tool,
                   [[maybe_unused]] std::size_t per_eta_size,
                   [[maybe_unused]] std::size_t per_pdg_size)
        {
           m_truthSelectionCuts.setBinning(truth_selection_tool.nCuts()+1, -0.5,truth_selection_tool.nCuts()+.5);
           if constexpr(DetailEnabled) {
              m_counterPerEta.clear();
              m_counterPerPdgId.clear();
              m_statPerEta.clear();
              m_statPerPdgId.clear();
              m_counterPerEta.resize(per_eta_size);
              m_counterPerPdgId.resize( per_pdg_size);
              m_statPerEta.resize( per_eta_size );
              m_statPerPdgId.resize( per_pdg_size );
           }
        }
        void fill([[maybe_unused]] unsigned int eta_category_i,
                  [[maybe_unused]] unsigned int pdg_id_category_i,
                  [[maybe_unused]] float hit_efficiency,
                  [[maybe_unused]] float hit_purity,
                  [[maybe_unused]] float match_prob) {
           if (DetailEnabled) {
              assert( eta_category_i <m_statPerEta.size());
              m_statPerEta[eta_category_i][kHitEfficiency].add( hit_efficiency);
              m_statPerEta[eta_category_i][kHitPurity].add( hit_purity);
              m_statPerEta[eta_category_i][kMatchProbability].add( match_prob);
              assert( pdg_id_category_i <m_statPerPdgId.size());
              m_statPerPdgId[pdg_id_category_i][kHitEfficiency].add( hit_efficiency);
              m_statPerPdgId[pdg_id_category_i][kHitPurity].add( hit_purity);
              m_statPerPdgId[pdg_id_category_i][kMatchProbability].add( match_prob);
              assert( eta_category_i < m_counterPerEta.size());
              assert( pdg_id_category_i <m_counterPerPdgId.size());
              ++m_counterPerEta[eta_category_i][kNTotalTracks];
              ++m_counterPerPdgId[pdg_id_category_i][kNTotalTracks];
           }
        }
        void incrementTotal([[maybe_unused]] unsigned int eta_category_i,
                            [[maybe_unused]] unsigned int pdg_id_category_i) {
           if constexpr(DetailEnabled) {
              ++m_counterPerEta[eta_category_i][kNTotalParticles];
              ++m_counterPerPdgId[pdg_id_category_i][kNTotalParticles];
           }
        }

        BaseStat<DetailEnabled> &operator+=(const BaseStat<DetailEnabled> &event_stat);
        
        void printStatTables(const TrackTruthMatchingBaseAlg &parent,
                             const std::vector<float> &statPtBins,
                             const std::vector<float> &statEtaBins,
                             std::vector< int > &pdgId,
                             bool printDetails,
                             bool pdgIdCategorisation,
                             bool useAbsEtaForStat);
        

        std::size_t perEtaSize() const  {
           if constexpr(DetailEnabled) { return m_counterPerEta.size(); }
           else { return 0u; }
        }
        std::size_t perPdgIdSize() const {
           if constexpr(DetailEnabled) { return m_counterPerPdgId.size(); }
           else { return 0u; }
        }
        ActsUtils::StatHist                                            m_truthSelectionCuts;
        // per event statistics
        struct Empty {};
        using CounterArrayVec = std::conditional< DetailEnabled,
                                                  std::vector< std::array< std::size_t, kNCategorisedCounter> >,
                                                  Empty >::type;
        using StatArrayVec = std::conditional< DetailEnabled,
                                               std::vector< std::array<ActsUtils::Stat, kNCategorisedStat> >,
                                               Empty >::type;
        CounterArrayVec m_counterPerEta;
        CounterArrayVec m_counterPerPdgId;
        StatArrayVec m_statPerEta;
        StatArrayVec m_statPerPdgId;
        
     };
     

     mutable std::mutex                           m_statMutex ATLAS_THREAD_SAFE;
     mutable std::array< std::size_t, kNCounter > m_counter ATLAS_THREAD_SAFE {};
     mutable std::vector< int >                   m_pdgId ATLAS_THREAD_SAFE;
     mutable BaseStat<TrackFindingValidationDetailedStat> m_detailedStat ATLAS_THREAD_SAFE;

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
