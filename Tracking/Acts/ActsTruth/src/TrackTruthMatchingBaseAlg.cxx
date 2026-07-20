/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "TrackTruthMatchingBaseAlg.h"

#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthParticle.h"

// for pdg_id -> name
#include "GeneratorModules/GenData.h"

#include <iomanip>
#include <cmath>
#include <type_traits>
#include <typeinfo>
#include <numeric>

namespace {
   template <typename T, std::size_t N>
   void accumulateTo(typename std::vector<std::array<T,N> >::const_iterator src_begin,
                     typename std::vector<std::array<T,N> >::const_iterator src_end,
                     std::array<T,N> &dest) {
      for (typename std::vector<std::array<T,N> >::const_iterator src_iter = src_begin;
           src_iter != src_end;
           ++src_iter) {
         for (unsigned int elm_i=0; elm_i<dest.size(); ++elm_i) {
            assert( elm_i < src_iter->size() );
            dest[elm_i] += (*src_iter)[elm_i];
         }
      }
   }

   template <typename T, bool LastRowOnly=false>
   void accumulateToLastColumnRow(std::size_t n_rows, std::size_t n_cols, std::vector<T> &stat) {
      assert(n_cols > 0);
      assert(n_rows > 0);
      auto stat_begin_iter = stat.begin();
      if (n_rows>1) {
         auto stat_total_row_begin_iter = stat.begin() + (n_rows-1) * n_cols;
         assert(  static_cast<std::size_t>(stat_total_row_begin_iter -  stat_begin_iter) < stat.size());

         for (std::size_t row_i = 0; row_i < n_rows-1; ++row_i) {
            auto stat_end_iter = stat_begin_iter + n_cols - 1 ;
            assert(static_cast<std::size_t>(stat_end_iter - stat.begin()) < stat.size());
            auto stat_total_row_iter = stat_total_row_begin_iter;
            if constexpr(!LastRowOnly) {
               accumulateTo(stat_begin_iter,  stat_end_iter, *stat_end_iter );
            }

            ++stat_end_iter; // now also consider the total eta bin.
            for (; stat_begin_iter != stat_end_iter; ++stat_begin_iter, ++stat_total_row_iter) {
               assert(static_cast<std::size_t>(stat_begin_iter - stat.begin()) < stat.size());
               assert(static_cast<std::size_t>(stat_total_row_iter - stat.begin()) < stat.size());
               for (unsigned int idx=0; idx < stat_total_row_iter->size(); ++idx) {
                  stat_total_row_iter->at(idx) += stat_begin_iter->at(idx);
               }
            }
         }
      }
      else if constexpr(!LastRowOnly) {
         auto stat_end_iter = stat_begin_iter + n_cols - 1 ;
         accumulateTo(stat_begin_iter,  stat_end_iter, *stat_end_iter );
      }
   }

   template <typename T>
   void accumulateToLastRow(std::size_t n_rows, std::size_t n_cols, std::vector<T> &stat) {
      accumulateToLastColumnRow<T,true>(n_rows, n_cols, stat);
   }

   template <typename T, std::size_t N>
   void addStat(const std::vector<std::array<T,N> > &src, std::vector<std::array<T,N> > &dest) {
      assert( src.size() == dest.size());
      unsigned int idx=0;
      for (const std::array<T,N> &src_elm : src) {
         assert( idx < dest.size());
         std::array<T,N> &dest_elm = dest[idx];
         unsigned val_i=0;
         for (const T &val : src_elm) {
            assert( val_i < dest_elm.size());
            dest_elm[val_i] += val;
            ++val_i;
         }
         ++idx;
      }
   }

   inline double sqr(double a) { return a*a; }
   // computate ratio and its statistical uncertainty
   inline std::array<float, 2> computeRatio(unsigned int numerator_counts, unsigned int denominator_counts) {
      double inv_denominator_counts = denominator_counts > 0 ? 1./denominator_counts : 0.;
      return std::array<float, 2> {
         static_cast<float>(numerator_counts * inv_denominator_counts),
         static_cast<float>(sqrt(  numerator_counts * (denominator_counts-numerator_counts)
                                 * inv_denominator_counts * sqr(inv_denominator_counts) ))
      };
   }

   std::string hfill(const std::string &head, const std::string &tail, std::size_t width) {
      width=std::max(width, head.size() + tail.size()) - head.size() - tail.size();
      std::stringstream out;
      out << head << std::setw(width) << " " << tail;
      return out.str();

   }

   std::string dumpCounts(const ActsTrk::HitCounterArray &counts) {
      std::stringstream out;
      for (uint8_t val : counts) {
         out << " " << std::setw(2) << static_cast<int>(val);
      }
      return out.str();
   }
}
namespace ActsTrk
{
  // to dump
  inline MsgStream &operator<<(MsgStream &out, const ActsUtils::Stat &stat) {
     ActsUtils::dumpStat(out, stat);
     return out;
  }

  TrackTruthMatchingBaseAlg::TrackTruthMatchingBaseAlg(const std::string &name,
                                                         ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode TrackTruthMatchingBaseAlg::initialize()
  {
     ATH_CHECK( m_truthSelectionTool.retrieve());
     ATH_CHECK( m_truthHitCounts.initialize() );
     initStatTables();
     return checkMatchWeights();
  }

  template <bool IsDebug>
  template <class T_OutStream>
  inline void TrackTruthMatchingBaseAlg::DebugCounter<IsDebug>::dumpStatistics(T_OutStream &out) const {
     if constexpr(IsDebug) {
        out << "Weighted measurement sum per truth particle without associated counts :" << m_measPerTruthParticleWithoutCounts << std::endl
            << m_measPerTruthParticleWithoutCounts.histogramToString() << std::endl
            << "Match probability of best match :" << m_bestMatchProb << std::endl
            << m_bestMatchProb.histogramToString() << std::endl
            << "Match probability of next-to-best match :" << m_nextToBestMatchProb << std::endl
            << m_nextToBestMatchProb.histogramToString() << std::endl;
     }
  }

  template <bool IsDebug>
  inline void TrackTruthMatchingBaseAlg::DebugCounter<IsDebug>::fillMeasForTruthParticleWithoutCount(double weighted_measurement_sum) const {
     if constexpr(IsDebug) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_measPerTruthParticleWithoutCounts.add(weighted_measurement_sum);
     }
  }
  template <bool IsDebug>
  inline void TrackTruthMatchingBaseAlg::DebugCounter<IsDebug>::fillTruthMatchProb(const std::array<float,2> &best_match_prob) const {
     if constexpr(IsDebug) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_bestMatchProb.add(best_match_prob[0]);
        m_nextToBestMatchProb.add(best_match_prob[1]);
     }
  }


  StatusCode TrackTruthMatchingBaseAlg::finalize()
  {
     if (msgLvl(MSG::INFO)) {
        if constexpr(TrackFindingValidationDebugHists) {
           m_debugCounter.dumpStatistics(msg());
        }
        msg(MSG::INFO) << "Truth selection cuts: " << std::endl;
        unsigned int cut_i=0;
        std::size_t total = std::accumulate ( m_detailedStat.m_truthSelectionCuts.m_histogram.begin(),
                                              m_detailedStat.m_truthSelectionCuts.m_histogram.end(), 0u);
        msg() << std::setw(3) << "" << " " << std::setw(20) << total << " total" << std::endl;
        if (m_detailedStat.m_truthSelectionCuts.m_histogram.at(cut_i) > 0) {
           msg() << std::setw(3) << "" << " "
                 << std::setw(20) << (total - m_detailedStat.m_truthSelectionCuts.m_histogram.at(cut_i))
                 << " underflow" << std::endl;
        }
        total -= m_detailedStat.m_truthSelectionCuts.m_histogram.at(cut_i++);
        for (const std::string &name : m_truthSelectionTool->names()) {
           total -= m_detailedStat.m_truthSelectionCuts.m_histogram.at(cut_i);
           msg() << std::setw(3) << cut_i << " " << std::setw(20) << total  << " " << name << std::endl;
           ++cut_i;
        }
        total -= m_detailedStat.m_truthSelectionCuts.m_histogram.at(cut_i);
        if (total>0) {
           msg() << std::setw(3) << "" << " " << std::setw(20) << total << " overflow" << std::endl;
        }
        if (msgLvl(MSG::DEBUG)) {
           msg() << m_detailedStat.m_truthSelectionCuts.histogramToString();
        }
        msg() << endmsg;
     }
     printStatTables();
     return StatusCode::SUCCESS;
  }

  TrackTruthMatchingBaseAlg::TruthMatchResult
  TrackTruthMatchingBaseAlg::analyseTrackTruth(const TruthParticleHitCounts &truth_particle_hit_counts,
                                               const HitCountsPerTrack &track_hit_counts,
                                               TrackTruthMatchingBaseAlg::EventStat &event_stat) const
  {
       TruthMatchResult ret{};
       std::array<unsigned int,2> best_match_i{std::numeric_limits<unsigned int>::max(),std::numeric_limits<unsigned int>::max()};
       std::array<float,2> best_match_prob {};

       std::array<unsigned int,2> best_match_i_nonoise{std::numeric_limits<unsigned int>::max(),std::numeric_limits<unsigned int>::max()};
       std::array<float,2> best_match_prob_nonoise{};

       const HitCounterArray &total_counts = track_hit_counts.totalCounts();
       const HitCounterArray &noise_counts = track_hit_counts.noiseCounts();

       double total_sum=weightedCountSum(total_counts, m_weights.value() );
       double total_sum_for_prob=weightedCountSum(total_counts, m_weightsForProb.value() );
       double noise_sum=noiseCorrection(noise_counts, m_weightsForProb.value() );
       double total_sum_for_prob_nonoise=total_sum_for_prob;
       total_sum_for_prob += noise_sum;

       if (total_sum_for_prob>0.) {
       // compute total hit count per truth particle and remember the highest and second highest
       // count per truth particle.
       // The match probability is then max_counter / sum_{i in associated truth particles} counts_i
       unsigned int truth_i=0;
       --truth_i;
       for (const std::pair<const xAOD::TruthParticle *, HitCounterArray > &
               hit_counts_for_associated_truth_particle : track_hit_counts.countsPerTruthParticle() ) {
          ++truth_i;
          double truth_sum_for_prob=weightedCountSum(hit_counts_for_associated_truth_particle.second, m_weightsForProb.value() );
          float match_prob_nonoise = truth_sum_for_prob /total_sum_for_prob_nonoise;
          float match_prob = truth_sum_for_prob /total_sum_for_prob;
          if (match_prob>1 || match_prob<0.) {
             ATH_MSG_ERROR("Negative or too large truth match \"probability\". This should not happen."
                           << " Track hits: " << dumpCounts(total_counts)
                           << " noise hits of those: " << dumpCounts(noise_counts)
                           << " truth hits: " << dumpCounts(hit_counts_for_associated_truth_particle.second));
          }          // remember the highest and next-to-highest hit count per truth particle
          if (match_prob>best_match_prob[1]) {
             int dest_i=match_prob<best_match_prob[0];
             best_match_i[1]=best_match_i[0];
             best_match_prob[1]=best_match_prob[0];
             best_match_prob[dest_i]=match_prob;
             best_match_i[dest_i]=truth_i;
          }
          if (match_prob_nonoise>best_match_prob_nonoise[1]) {
             int dest_i=match_prob_nonoise<best_match_prob_nonoise[0];
             best_match_i_nonoise[1]=best_match_i_nonoise[0];
             best_match_prob_nonoise[1]=best_match_prob_nonoise[0];
             best_match_prob_nonoise[dest_i]=match_prob_nonoise;
             best_match_i_nonoise[dest_i]=truth_i;
          }

       }
       }

       if (best_match_i_nonoise[0] != best_match_i[0]) {
          ++event_stat.m_nTruthParticleNonoiseMismatches;
       }
       if (   best_match_i[0] < track_hit_counts.countsPerTruthParticle().size()
           && track_hit_counts.countsPerTruthParticle()[ best_match_i[0] ].first) {
          ret.m_truthParticle  = track_hit_counts.countsPerTruthParticle()[ best_match_i[0] ].first;
          ret.m_matchProbability = best_match_prob[0];

          const xAOD::TruthParticle *best_match = track_hit_counts.countsPerTruthParticle()[ best_match_i[0] ].first;
          const IAthSelectionTool::CutResult accept = m_truthSelectionTool->accept(best_match);
          event_stat.m_truthSelectionCuts.add( event_stat.m_nTruthCuts - accept.missingCuts() );
          if (accept) {

             double common_truth_sum=weightedCountSum(track_hit_counts.countsPerTruthParticle()[ best_match_i[0] ].second, m_weights.value() );

             float hit_efficiency = 0.;
             std::unordered_map<const xAOD::TruthParticle *,HitCounterArray>::const_iterator
                best_truth_particle_counts_iter  = truth_particle_hit_counts.find( best_match );

             if (best_truth_particle_counts_iter != truth_particle_hit_counts.end()) {
                double truth_sum=weightedCountSum(best_truth_particle_counts_iter->second, m_weights.value() );
                // in principle truth_measuremnts should always be larger than 0
                hit_efficiency = truth_sum > 0u ? (common_truth_sum/truth_sum) : 0.;
             }
             else {
                // this can happen if the total number of hits are below threshold for
                // accepting a truth particle
                ++event_stat.m_nTruthParticleWithoutAssociatedCounts;
                m_debugCounter.fillMeasForTruthParticleWithoutCount(total_sum);
             }

             // in principle a track matched to a truth particle should always have n_total > 0
             // but the hits could be filtered out.
             if (total_sum>0.) {
                float hit_purity = common_truth_sum / total_sum;
                ret.m_hitPurity = hit_purity;
                ret.m_hitEfficiency = hit_efficiency;
                m_debugCounter.fillTruthMatchProb(best_match_prob);

                if constexpr(TrackFindingValidationDetailedStat) {
                   float best_match_pt = best_match->pt();
                   std::size_t eta_category_i    = getPtEtaStatCategory(best_match_pt, best_match->eta());
                   std::size_t pdg_id_category_i = getPtPdgIdStatCategory(best_match_pt, best_match->pdg_id());
                   event_stat.fill( eta_category_i, pdg_id_category_i, hit_efficiency, hit_purity, best_match_prob[0], best_match );
                }
             }
          }
          else {
             ++event_stat.m_nTracksWithoutSelectedTruthParticle;
          }
       }
       else {
          // no eta, pdg_id for tracks without associated truth particle
          // could use eta of track but not available to the algorithm

          ++event_stat.m_nTracksWithoutAssociatedTruthParticle;
          if (!track_hit_counts.countsPerTruthParticle().empty()) {
             ATH_MSG_ERROR("Failed to select best matching truth particle out of " << track_hit_counts.countsPerTruthParticle().size()
                           << ". This should not happen." );
          }
       }
       return ret;
    }



  void TrackTruthMatchingBaseAlg::postProcessEventStat(const TruthParticleHitCounts &truth_particle_hit_counts,
                                                       std::size_t n_tracks,
                                                       TrackTruthMatchingBaseAlg::EventStat &event_stat) const
  {
     if constexpr(EventStat::doDetail) {
        if (m_computeTrackRecoEfficiency.value()) {
           for(const std::pair<const xAOD::TruthParticle * const,ActsTrk::HitCounterArray> &truth_particle : truth_particle_hit_counts) {
              const IAthSelectionTool::CutResult accept = m_truthSelectionTool->accept(truth_particle.first);
              if (accept) {
                 double truth_sum=weightedCountSum(truth_particle.second, m_weights.value() );
                 if (truth_sum>0.) {
                    float truth_particle_pt = truth_particle.first->pt();
                    std::size_t eta_category_i =  getPtEtaStatCategory(truth_particle_pt, truth_particle.first->eta());
                    std::size_t pdg_id_category_i =  getPtPdgIdStatCategory(truth_particle_pt, truth_particle.first->pdg_id());
                    event_stat.incrementTotal(eta_category_i, pdg_id_category_i);
                 }
              }
           }
        }
     }

    // update total statistic counter
    {
       std::lock_guard<std::mutex> lock(m_statMutex);
       m_counter[MissingTruthParticleHitCounts] += event_stat.m_nTruthParticleWithoutAssociatedCounts;
       m_counter[NoAssociatedTruthParticle] += event_stat.m_nTracksWithoutAssociatedTruthParticle;
       m_counter[NoSelectedTruthParticle] += event_stat.m_nTracksWithoutSelectedTruthParticle;
       m_counter[TruthParticleNoNoiseMismatch]+=event_stat.m_nTruthParticleNonoiseMismatches;
       m_counter[NTracksTotal]+=n_tracks;
       m_counter[NTruthWithCountsTotal]+=truth_particle_hit_counts.size();

       if constexpr(EventStat::doDetail) {
          m_detailedStat += event_stat;
       }
    }
  }

  inline std::size_t TrackTruthMatchingBaseAlg::getPtEtaStatCategory(float pt, float eta) const
  {
     std::vector<float>::const_iterator pt_bin_iter = std::upper_bound(m_statPtBins.begin(),
                                                                       m_statPtBins.end(),
                                                                       pt);
     std::vector<float>::const_iterator eta_bin_iter = std::upper_bound(m_statEtaBins.begin(),
                                                                        m_statEtaBins.end(),
                                                                        m_useAbsEtaForStat ? std::abs(eta) : eta);
     return   (m_statEtaBins.size()+2u) * static_cast<std::size_t>(pt_bin_iter - m_statPtBins.begin())
            + static_cast<std::size_t>(eta_bin_iter - m_statEtaBins.begin());
  }
  std::size_t TrackTruthMatchingBaseAlg::getPtPdgIdStatCategory(float pt, int pdg_id) const {
     std::vector<float>::const_iterator pt_bin_iter = std::upper_bound(m_statPtBins.begin(),
                                                                       m_statPtBins.end(),
                                                                       pt);
     int abs_pdg_id = std::min(std::abs(pdg_id), s_pdgIdMax);
     std::vector< int >::const_iterator iter = std::find(m_pdgId.begin(), m_pdgId.end(), abs_pdg_id);
     if (iter == m_pdgId.end()){
        if (m_pdgId.size() < m_pdgId.capacity()) {
           std::lock_guard<std::mutex> lock(m_statMutex); // @TODO pdg id list specific mutex ?
           // make sure that the pdg id still does not exist.
           iter = std::find(m_pdgId.begin(), m_pdgId.end(), abs_pdg_id);
           if (iter == m_pdgId.end()){
              m_pdgId.push_back(abs_pdg_id);
              iter = m_pdgId.end()-1;
           }
        }
        else {
           iter=m_pdgId.begin();
        }
     }
     return   (m_pdgId.capacity()) * static_cast<std::size_t>(pt_bin_iter - m_statPtBins.begin())
            + (iter - m_pdgId.begin());
  }

  void TrackTruthMatchingBaseAlg::checkBinOrder( const std::vector<float> &bin_edges, const std::string &bin_label) const {
    if (!bin_edges.empty())
    {
      float last_eta = bin_edges[0];
      for (float eta : bin_edges)
      {
        if (eta < last_eta)
        {
          ATH_MSG_FATAL(bin_label + " bins for statistics counter not in ascending order.");
        }
        last_eta = eta;
      }
    }
  }
  void TrackTruthMatchingBaseAlg::initStatTables()
  {
    if (!m_statEtaBins.empty())
    {
      m_useAbsEtaForStat = (m_statEtaBins[0] > 0.);
      checkBinOrder(m_statEtaBins.value(),"Eta");
    }
    checkBinOrder(m_statPtBins.value(),"Pt");


    unsigned int max_pdg_id_slots=( m_pdgIdCategorisation.value() ? 20 : 1 );

    // last element in statPerEta and counterPerEta will be used accumulate statistics of all eta bins
    m_detailedStat.reset( *m_truthSelectionTool,
                          (m_statPtBins.size() + 2) * (m_statEtaBins.size() + 2),
                          (m_statPtBins.size() + 2) * max_pdg_id_slots);

    m_pdgId.reserve(max_pdg_id_slots);
    m_pdgId.push_back(1000000000);
  }

  template <bool DetailEnabled>
  inline TrackTruthMatchingBaseAlg::BaseStat<DetailEnabled> &
  TrackTruthMatchingBaseAlg::BaseStat<DetailEnabled>::operator+=(const TrackTruthMatchingBaseAlg::BaseStat<DetailEnabled> &event_stat) {
     if constexpr(DetailEnabled) {
        addStat(event_stat.m_counterPerEta,m_counterPerEta);
        addStat(event_stat.m_statPerEta, m_statPerEta);
        addStat(event_stat.m_counterPerPdgId,m_counterPerPdgId);
        addStat(event_stat.m_statPerPdgId, m_statPerPdgId);
     }
     m_truthSelectionCuts += event_stat.m_truthSelectionCuts;
     return *this;
  }

  template <bool DetailEnabled>
  void TrackTruthMatchingBaseAlg::BaseStat<DetailEnabled>::printStatTables(const TrackTruthMatchingBaseAlg &parent,
                                                                           const std::vector<float> &statPtBins,
                                                                           const std::vector<float> &statEtaBins,
                                                                           std::vector< int > &pdgId,
                                                                           bool printDetails,
                                                                           bool pdgIdCategorisation,
                                                                           bool useAbsEtaForStat) {
    if constexpr(DetailEnabled) {
       static constexpr bool rotate=true;// row : eta/PDG ID; column: pt
       std::vector<std::string> counter_labels { std::string("Truth particles"),
                                                 std::string("with asso. track"),
                                                 std::string("with >1 asso. tracks"),
                                                 std::string("total tracks")};
       std::vector<std::string> pt_labels;
       pt_labels.reserve(statPtBins.size() + 2);
       unsigned int pt_precision=0;
       for (float pt : statPtBins) {
          if (pt<1.) {
             pt_precision=1;
             break;
          }
       }
       for (std::size_t bin_i = 0; bin_i < statPtBins.size() + 2; ++bin_i) {
          pt_labels.push_back(TableUtils::makeBinLabel("pt",statPtBins, bin_i, true, pt_precision));
       }
       // statistics eta-bins
       {
          std::vector<std::string> eta_labels;
          eta_labels.reserve(statEtaBins.size() + 2);
          for (std::size_t eta_bin_i = 0; eta_bin_i < statEtaBins.size() + 2; ++eta_bin_i) {
             eta_labels.push_back(TableUtils::makeEtaBinLabel(statEtaBins, eta_bin_i, useAbsEtaForStat));
          }

          accumulateToLastColumnRow(statPtBins.size()+2,statEtaBins.size()+2, m_statPerEta);
          accumulateToLastColumnRow(statPtBins.size()+2,statEtaBins.size()+2, m_counterPerEta);

          if (statPtBins.empty() || printDetails) {
             parent.printCategories(pt_labels, eta_labels, counter_labels, m_statPerEta, m_counterPerEta,
                                    (!statPtBins.empty()
                                     ? hfill("pt ",
                                             "eta",
                                             TableUtils::maxLabelWidth(pt_labels)
                                             +TableUtils::maxLabelWidth(eta_labels))
                                     : std::string("eta") ),
                                    !statPtBins.empty());
          }
          if (!statPtBins.empty()) {
             parent.printData2D(pt_labels, eta_labels,
                                rotate
                                ? hfill("eta","\\ pt", TableUtils::maxLabelWidth(eta_labels))
                                : hfill("pt","\\ eta", TableUtils::maxLabelWidth(pt_labels)),
                                m_statPerEta,
                                m_counterPerEta,
                                rotate);
          }
       }

       // statistics in PDG ID bins.
       if (pdgIdCategorisation) {
          std::vector<std::string> pdg_id_labels;
          pdg_id_labels.reserve( pdgId.size());
          pdg_id_labels.push_back("Other");
          auto gendata = std::make_unique<GenData>();
          for (unsigned int pdg_i=1; pdg_i < pdgId.size(); ++pdg_i) {
             std::stringstream a_label;
             a_label << gendata->particleName(pdgId[pdg_i]).value()  << " [" << pdgId[pdg_i] << "]";
             pdg_id_labels.push_back(  a_label.str() );
          }
          unsigned int max_pdg_id_slots=m_statPerPdgId.size()/(statPtBins.size()+2);
          assert( m_statPerPdgId.size() % (statPtBins.size()+2) == 0 );
          // also the unused columns are projected, but that does not harm :
          accumulateToLastRow(statPtBins.size()+2,max_pdg_id_slots, m_statPerPdgId);
          accumulateToLastRow(statPtBins.size()+2,max_pdg_id_slots, m_counterPerPdgId);

          if (statPtBins.empty() || printDetails) {
             parent.printCategories(pt_labels, pdg_id_labels, counter_labels, m_statPerPdgId, m_counterPerPdgId,
                             (!statPtBins.empty()
                              ? hfill("pt ",
                                      "PDG-id",
                                      TableUtils::maxLabelWidth(pt_labels)
                                      +TableUtils::maxLabelWidth(pdg_id_labels))
                              : std::string("eta")),
                             !statPtBins.empty());
          }
          if (!statPtBins.empty()) {
             parent.printData2D(pt_labels, pdg_id_labels,
                                rotate
                                ? hfill("PDG ID","\\ pt", TableUtils::maxLabelWidth(pdg_id_labels))
                                : hfill("pt","\\ PDG ID", TableUtils::maxLabelWidth(pt_labels)),
                                m_statPerPdgId,
                                m_counterPerPdgId,
                                rotate);
          }
       }
    }
  }

  void TrackTruthMatchingBaseAlg::printStatTables() const
  {
    if (msgLvl(MSG::INFO))
    {
       msg() << MSG::INFO << std::endl;
       m_detailedStat.printStatTables( *this, m_statPtBins, m_statEtaBins, m_pdgId, m_printDetails.value(), m_pdgIdCategorisation.value(), m_useAbsEtaForStat );
      {
          std::array<std::string, kNCounter> counter_labels { std::string("Number of tracks"),
                                                              std::string("Number of truth particles with hit counts"),
                                                              std::string("Associated truth particles without hit counts"),
                                                              std::string("Tracks without associated truth particle"),
                                                              std::string("Tracks without selected, associated truth particle"),
                                                              std::string("Best truth particle without noise correction mismatch")
          };
          msg() << makeTable( m_counter, counter_labels) << std::endl;
      }
      msg() << endmsg;
    }
  }



  void TrackTruthMatchingBaseAlg::printCategories(const std::vector<std::string> &row_category_labels,
                                                  const std::vector<std::string> &col_category_labels,
                                                  std::vector<std::string> &counter_labels,
                                                  std::vector< std::array< ActsUtils::Stat, kNCategorisedStat> > &stat_per_category,
                                                  std::vector< std::array< std::size_t, kNCategorisedCounter> >  &counts_per_category,
                                                  const std::string &top_left,
                                                  bool print_sub_categories) const {
     if (!row_category_labels.empty() && !col_category_labels.empty()) {
        if (row_category_labels.size() * col_category_labels.size() > counts_per_category.size() ) {
           ATH_MSG_ERROR( "Mismatch between category labels and number of counters (logic error -> fix needed):"
                          << row_category_labels.size() << " * " << col_category_labels.size()
                          << " > " << counts_per_category.size() );
        }
        constexpr std::size_t stat_column_width=14*4 + 3*3+4 + 9; // floats + seperators + integer of Stat output
        assert( stat_per_category.size() == counts_per_category.size());
        const unsigned int n_rows = row_category_labels.size();
        const unsigned int n_cols = stat_per_category.size() / n_rows; // some columns at the end of a row might not have labels
                                                                       // and are to be ignored

        assert( stat_per_category.size() % n_rows == 0 );
        for(unsigned int row_i=(print_sub_categories ? 0 : n_rows-1); row_i<n_rows; ++row_i) {
           {
              std::vector<std::string> stat_labels { std::string("Hit Efficiency") };
              msg() << makeTable( stat_per_category, row_i*n_cols, kNCategorisedStat, kHitEfficiency, 1u, col_category_labels, stat_labels, top_left)
                         .columnWidth(stat_column_width)
                         .labelPrefix(row_category_labels.at(row_i)+" ")
                         .precision(std::vector<unsigned int>{3})
                    << std::endl;
           }
           {
              std::vector<std::string> stat_labels { std::string("Hit Purity") };
              msg() << makeTable( stat_per_category, row_i*n_cols, kNCategorisedStat, kHitPurity, 1u, col_category_labels, stat_labels, top_left)
                          .columnWidth(stat_column_width)
                          .labelPrefix(row_category_labels.at(row_i)+" ")
                          .precision(std::vector<unsigned int>{3})
                    << std::endl;
           }
           {
              std::vector<std::string> stat_labels { std::string("Match probability") };
              msg() << makeTable( stat_per_category, row_i*n_cols, kNCategorisedStat, kMatchProbability, 1u, col_category_labels, stat_labels, top_left)
                          .columnWidth(stat_column_width)
                          .labelPrefix(row_category_labels.at(row_i)+" ")
                          .precision(std::vector<unsigned int>{3})
                    << std::endl;
           }
           if (m_showRawCounts.value()) {
              msg() << makeTable( counts_per_category, row_i*n_cols, kNCategorisedCounter, 0u, 1u, col_category_labels, counter_labels, top_left)
                          .labelPrefix(row_category_labels.at(row_i)+" ")
                    << std::endl;
           }

           if (m_computeTrackRecoEfficiency.value()) {
              std::vector< std::array< float, 2> > eff;
              eff.reserve(m_pdgId.size());
              for (unsigned int category_i=0; category_i< col_category_labels.size(); ++category_i) {
                 eff.push_back( computeRatio( counts_per_category[category_i+row_i*n_cols][kNParticleWithAssociatedTrack],
                                              counts_per_category[category_i+row_i*n_cols][kNTotalParticles] ) );
              }
              std::vector<std::string> eff_labels { std::string("reco efficiency"),
                 std::string("stat. uncertainty") };
              msg() << makeTable( eff, 0u, eff.begin()->size(),0u,1u, col_category_labels, eff_labels, top_left)
                          .labelPrefix(row_category_labels.at(row_i)+" ")
                          .precision(std::vector<unsigned int>{3,3})
                    << std::endl;
           }
        }
     }
  }

  namespace {
     // helper to prevent temporary table data from beeing destructed too early
     template <typename T>
     struct TablePlusData {
        TablePlusData(std::vector<T> &&values,
                      const std::vector<std::string> &row_labels,
                      const std::vector<std::string> &col_labels,
                      const std::string &top_left_label)
           : m_data(std::move(values)),
             m_assocTable({
                   TableUtils::Range2D<T>{m_data.data(),
                                          row_labels.size(),  // n-rows
                                          col_labels.size(),  // n-columns
                                          col_labels.size(),  // offset between rows
                                          0u,                 // first column index
                                          1u},                // offset between columns
                   TableUtils::Range<std::string> {row_labels.data(), row_labels.size()},
                   TableUtils::Range<std::string> {col_labels.data(), col_labels.size()},
                   top_left_label
                })
        {}
        std::vector<T>                   m_data;
        TableUtils::MultiColumnTable<T>  m_assocTable;
        TablePlusData &columnWidth(std::size_t value) { m_assocTable.columnWidth(value); return *this;}
        TablePlusData &minLabelWidth(std::size_t value) { m_assocTable.minLabelWidth(value); return *this;}
        TablePlusData &dumpHeader(bool value=true) { m_assocTable.dumpHeader(value); return *this;}
        TablePlusData &dumpFooter(bool value=true) { m_assocTable.dumpFooter(value); return *this;}
        TablePlusData &separateLastRow(bool value=true) { m_assocTable.separateLastRow(value); return *this;}
        TablePlusData &labelPrefix(const std::string& value) { m_assocTable.labelPrefix(value); return *this;}
        TablePlusData &precision(std::vector<unsigned int> &&precision) { m_assocTable.precision(std::move(precision)); return *this;}
     };

     template <typename T>
     inline MsgStream &operator<<(MsgStream &out, const TablePlusData<T> &table) {
        out << table.m_assocTable;
        return out;
     }

     template <typename T>
     inline std::ostream &operator<<(std::ostream &out, const TablePlusData<T> &table) {
        out << table.m_assocTable;
        return out;
     }

     template <class T_Container, class T_Function, typename T=double>
     TablePlusData<T>
     create2DTable(const std::vector<std::string> &row_category_labels,
                   const std::vector<std::string> &col_category_labels,
                   const std::string &top_left_label,
                   T_Container container,
                   T_Function function,
                   bool rotate) {
        const unsigned int n_rows = row_category_labels.size();
        const unsigned int n_cols = col_category_labels.size();
        const unsigned int n_cols_total = container.size() / n_rows; // some columns at the end of a row might not have labels
        std::vector< T > values;
        values.reserve( n_rows * n_cols );
        if (rotate) {
           for (unsigned int col_i=0; col_i< n_cols; ++col_i) {
              for (unsigned int row_i=0; row_i< n_rows; ++row_i) {
                 values.push_back( function(container.at( row_i * n_cols_total + col_i )) );
              }
           }
        }
        else {
           for (unsigned int row_i=0; row_i< n_rows; ++row_i) {
              for (unsigned int col_i=0; col_i< n_cols; ++col_i) {
                 values.push_back( function(container.at( row_i * n_cols_total + col_i )) );
              }
           }
        }
        return TablePlusData<T>(std::move(values),
                                rotate ? col_category_labels : row_category_labels,  // rows
                                rotate ? row_category_labels : col_category_labels,  // columns
                                top_left_label);
     }
  }

   void TrackTruthMatchingBaseAlg::printData2D(const std::vector<std::string> &row_category_labels,
                                               const std::vector<std::string> &col_category_labels,
                                               const std::string &top_left_label,
                                               std::vector< std::array< ActsUtils::Stat, kNCategorisedStat> > &stat_per_category,
                                               std::vector< std::array< std::size_t, kNCategorisedCounter> >  &counts_per_category,
                                               bool rotate) const
   {
     if (!row_category_labels.empty() && !col_category_labels.empty()) {
        if (row_category_labels.size() * col_category_labels.size() > counts_per_category.size() ) {
           ATH_MSG_ERROR( "Mismatch between category labels and number of counters (logic error -> fix needed):"
                          << row_category_labels.size() << " * " << col_category_labels.size()
                          << " > " << counts_per_category.size() );
        }
        std::vector<unsigned int> column_precision;
        column_precision.resize( rotate ? row_category_labels.size() : col_category_labels.size(), 3u);
        assert( stat_per_category.size() == counts_per_category.size());
        msg() << "Hit efficiency : contributing hits over all hits of best matching truth particle" << std::endl
              << create2DTable( row_category_labels, col_category_labels, top_left_label, stat_per_category,
                                [](const std::array< ActsUtils::Stat, kNCategorisedStat> &stat) {
                                   return stat.at(kHitEfficiency).mean();
                                },
                                rotate)
                    .columnWidth(10)
                    .precision(std::vector<unsigned int>(column_precision))
              << std::endl;
        msg() << "Hit purity : contributing hits of best matching truth particle over all hits on track" << std::endl
              << create2DTable( row_category_labels, col_category_labels, top_left_label, stat_per_category,
                                [](const std::array< ActsUtils::Stat, kNCategorisedStat> &stat) {
                                   return stat.at(kHitPurity).mean();
                                },
                                rotate)
                    .columnWidth(10)
                   .precision(std::vector<unsigned int>(column_precision))
              << std::endl;
        msg() << "Match probability : weighted common hit sum of best matching truth particle over total track weighted hit sum" << std::endl
              << create2DTable( row_category_labels, col_category_labels, top_left_label, stat_per_category,
                                [](const std::array< ActsUtils::Stat, kNCategorisedStat> &stat) {
                                   return stat.at(kMatchProbability).mean();
                                },
                                rotate)
                    .columnWidth(10)
                   .precision(std::vector<unsigned int>(column_precision))
              << std::endl;

        if (m_computeTrackRecoEfficiency.value()) {
           msg() << "Reco efficiency : tracks with assoc. truth particle over all selected truth particles with assoc. measurements."
                 << std::endl
                 << create2DTable( row_category_labels, col_category_labels, top_left_label, counts_per_category,
                                   [](const std::array< std::size_t, kNCategorisedCounter> &counter) {
                                      return computeRatio( counter[kNParticleWithAssociatedTrack],
                                                           counter[kNTotalParticles] )[0];
                                   },
                                   rotate)
                     .columnWidth(10)
                     .precision(std::move(column_precision))
                 << std::endl;
        }
     }
  }

  StatusCode TrackTruthMatchingBaseAlg::checkMatchWeights() {
     if (m_weights.size() != s_NMeasurementTypes) {
        ATH_MSG_FATAL( "There must be exactly one weight per measurement type. But got "
                       << m_weights.size() << " != " << s_NMeasurementTypes);
        return StatusCode::FAILURE;
     }
     if (m_weightsForProb.size() != s_NMeasurementTypes) {
        ATH_MSG_FATAL( "There must be exactly one weight for computing the matching probability per measurement type. But got "
                       << m_weightsForProb.size() << " != " << s_NMeasurementTypes);
        return StatusCode::FAILURE;
     }
     for (unsigned int type_i=0; type_i<s_NMeasurementTypes; ++type_i) {
        if (m_weightsForProb[type_i]<0. || m_weights[type_i]<0. || (m_weights[type_i]>0) != (m_weightsForProb[type_i]>0.)) {
           ATH_MSG_FATAL( "Invalid weights (should be positive) or inconsistency of weights which are zero (match prob. weights, weights):"
                          << m_weightsForProb[type_i] << " vs " << m_weights[type_i]);
           return StatusCode::FAILURE;
        }
     }
     return StatusCode::SUCCESS;
  }

  inline double TrackTruthMatchingBaseAlg::weightedCountSum(const ActsTrk::HitCounterArray &counts,
                                                            const std::vector<float> &weights) {
     assert( weights.size() == counts.size());
     double sum=0.;
     for (unsigned int count_i=0; count_i < counts.size(); ++count_i) {
        sum += counts[count_i] * weights[count_i];
     }
     return sum;
  }

  inline double TrackTruthMatchingBaseAlg::noiseCorrection(const ActsTrk::HitCounterArray &noise_counts,
                                                           const std::vector<float> &weights) {
     assert( weights.size() == noise_counts.size());
     double sum=0.;
     for (unsigned int count_i=0; count_i < noise_counts.size(); ++count_i) {
        sum -= weights[count_i] * noise_counts[count_i] - noise_counts[count_i];
     }
     return sum;
  }
}
