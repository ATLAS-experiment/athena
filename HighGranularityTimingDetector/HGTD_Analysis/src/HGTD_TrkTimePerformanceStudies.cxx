/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/src/HGTD_TrkTimePerformanceStudies.cxx
 *
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 */

#include "HGTD_TrkTimePerformanceStudies.h"
#include "xAODTruth/xAODTruthHelpers.h"

HGTD_TrkTimePerformanceStudies::HGTD_TrkTimePerformanceStudies(
    const std::string& name, ISvcLocator* svc_locator)
    : HGTD_AnalysisAlgBase(name, svc_locator) {}

HGTD_TrkTimePerformanceStudies::~HGTD_TrkTimePerformanceStudies() {}

StatusCode HGTD_TrkTimePerformanceStudies::initialize() {
  ATH_CHECK(HGTD_AnalysisAlgBase::initialize());

  ATH_MSG_INFO("Initializing HGTD_TrkTimePerformanceStudies ...");

  ATH_CHECK(m_track_sel_tools.retrieve());
  ATH_CHECK(m_track_time_tools.retrieve());
  const std::string titleStr{";|#eta| ;frequency"};
  for (const auto& track_tool : m_track_sel_tools) {
    const auto & tname = track_tool->name();
    for (const auto& time_tool : m_track_time_tools) {
      const auto & timetoolName = time_tool->name();
      static const std::string effVsEta{"m_eff_vs_eta"};
      bookEffSubdir(tname, timetoolName, effVsEta, titleStr, 32, 2.4, 4.0);
      static const std::string primesVsEta{"m_eff_gt50pcprimes_vs_eta"};
      bookEffSubdir(tname, timetoolName, primesVsEta, titleStr, 32, 2.4, 4.0);
      static const std::string primesVsEtaMistag{"m_eff_gt50pcprimes_vs_eta_mistag"};
      bookEffSubdir(tname, timetoolName, primesVsEtaMistag, titleStr, 32, 2.4, 4.0);
      for (const auto& primes_fraction_i : m_primes_fractions) {
        std::string name = "m_eff_vs_eta_primesfrac" + primes_fraction_i;
        bookEffSubdir(tname, timetoolName, name, titleStr, 32, 2.4, 4.0);
        std::string name_res = "m_hist_timeres_outlier_cases" + primes_fraction_i;
        static const std::string thisTitle{";t_{reco} - t_{truth} [ns]; number of tracks"};
        bookSubdir<TH1F>(tname, timetoolName, name_res, thisTitle, 200, -.4, 0.4);
      }
    }
  }

  ATH_CHECK(m_track_particles_key.initialize());
  ATH_CHECK(m_truth_event_container_key.initialize());
  ATH_CHECK(m_pileup_truth_container_key.initialize());

  return StatusCode::SUCCESS;
}

StatusCode HGTD_TrkTimePerformanceStudies::execute(const EventContext& ctx) {

  SG::ReadHandle<xAOD::TrackParticleContainer> track_particles_hdl(
      m_track_particles_key, ctx);
  const xAOD::TrackParticleContainer* track_particles =
      track_particles_hdl.cptr();

  for (const auto* track : *track_particles) {
    for (const auto& track_tool : m_track_sel_tools) {
      for (const auto& time_tool : m_track_time_tools) {

        const float trk_eta = track->eta();

        if (!track_tool->trackPassesSelection(track)) {
          continue;
        }

        const xAOD::TruthParticle* truth_particle =
            xAOD::TruthHelpers::getTruthParticle(*track);

        auto purity = time_tool->fracPrimaryHits(*track);
        int n_potential_primes = time_tool->numberPotentialPrimaryHits(*track);
        bool has_time = time_tool->expertHasTime(*track);

        PrimesFractions primes_fraction = PrimesFractions::AllPrimes;
        if (purity < 0.01) {
          switch (n_potential_primes) {
          case 0:
            primes_fraction = PrimesFractions::NoPrimesNoPossiblePrimes;
            break;
          case 1:
            primes_fraction = PrimesFractions::NoPrimes1PossiblePrimes;
            break;
          case 2:
            primes_fraction = PrimesFractions::NoPrimes2PossiblePrimes;
            break;
          case 3:
            primes_fraction = PrimesFractions::NoPrimes3PossiblePrimes;
            break;
          case 4:
            primes_fraction = PrimesFractions::NoPrimes4PossiblePrimes;
            break;
          }
        } else if (purity > 0.01 and purity < 0.48) {
          primes_fraction = PrimesFractions::LessThanHalfPrimes;
        } else if (purity > 0.48 and purity < 0.52) {
          primes_fraction = PrimesFractions::HalfPrimesHasPrimes;
        } else if (purity > 0.52 and purity < 0.98) {
          primes_fraction = PrimesFractions::MoreThanHalfPrimes;
        } else if (purity > 0.98) {
          primes_fraction = PrimesFractions::AllPrimes;
        }

        bool morethanhalfprimes =
            primes_fraction == PrimesFractions::AllPrimes or
            primes_fraction == PrimesFractions::MoreThanHalfPrimes;
        fillEffSubDir(track_tool->name(), time_tool->name(), "m_eff_vs_eta",
                      has_time, std::abs(trk_eta));

        bool count_primesfrac_category_as_good = false;
        for (size_t i = 0; i < m_primes_fractions.size(); i++) {
          if (i == static_cast<size_t>(primes_fraction)) {
            count_primesfrac_category_as_good = has_time;
          } else {
            count_primesfrac_category_as_good = false;
          }
          fillEffSubDir(track_tool->name(), time_tool->name(),
                        "m_eff_vs_eta_primesfrac" + m_primes_fractions.at(i),
                        count_primesfrac_category_as_good, std::abs(trk_eta));
        }

        if (has_time) {
          auto truth_vertex = getTruthVertex(truth_particle);
          if (truth_particle and truth_vertex) {
            float track_time = time_tool->expertTime(*track);
            float truth_time = time_tool->getTruthTime(*truth_vertex);
            float time_res = track_time - truth_time;
            fillSubdir<TH1F>(track_tool->name(), time_tool->name(),
                             "m_hist_timeres_outlier_cases" +
                                 m_primes_fractions.at(primes_fraction),
                             time_res);
          }
          fillEffSubDir(track_tool->name(), time_tool->name(),
                        "m_eff_gt50pcprimes_vs_eta", morethanhalfprimes,
                        std::abs(trk_eta));
          fillEffSubDir(track_tool->name(), time_tool->name(),
                        "m_eff_gt50pcprimes_vs_eta_mistag",
                        not morethanhalfprimes, std::abs(trk_eta));
        } else {
          fillEffSubDir(track_tool->name(), time_tool->name(),
                        "m_eff_gt50pcprimes_vs_eta", false, std::abs(trk_eta));
          fillEffSubDir(track_tool->name(), time_tool->name(),
                        "m_eff_gt50pcprimes_vs_eta_mistag", false,
                        std::abs(trk_eta));
        }
      } // loop over track-time tools
    }   // loop over track selection tools
  }     // loop over tracks

  return StatusCode::SUCCESS;
}

const xAOD::TruthVertex* HGTD_TrkTimePerformanceStudies::getTruthVertex(
    const xAOD::TruthParticle* truth_particle) {
  if (not truth_particle) {
    return nullptr;
  }

  SG::ReadHandle<xAOD::TruthEventContainer> truth_event_container_hdl(
      m_truth_event_container_key);
  const xAOD::TruthEventContainer* truth_event_container =
      truth_event_container_hdl.cptr();

  if (not truth_event_container or truth_event_container->empty()) {
    ATH_MSG_DEBUG("getTruthVertex: no " << m_truth_event_container_key.key()
                                        << " container in this event");
    return nullptr;
  }

  // check if truth particle is from HS event
  auto truth_hs_event = truth_event_container->at(0);
  if (not truth_hs_event) {
    return nullptr;
  }
  int n_hs_truthparticles = truth_hs_event->nTruthParticles();
  auto truth_hs_vtx = truth_event_container->at(0)->signalProcessVertex();

  for (int i = 0; i < n_hs_truthparticles; i++) {
    if (not truth_hs_event->truthParticle(i)) {
      continue;
    }
    if (not truth_hs_event->truthParticle(i)->isSimulationParticle() and
        truth_hs_event->truthParticle(i)->isStable() and
        truth_hs_event->truthParticle(i)->isCharged() and
        truth_hs_event->truthParticle(i)->index() == truth_particle->index()) {
      return truth_hs_vtx;
    }
  }

  SG::ReadHandle<xAOD::TruthPileupEventContainer> pileup_truth_container_hdl(
      m_pileup_truth_container_key);
  const xAOD::TruthPileupEventContainer* pileup_truth_container =
      pileup_truth_container_hdl.cptr();

  ATH_MSG_DEBUG("getTruthVertex: no HS vertex found");

  // TruthPileupEvents is only written to the AOD for samples digitised with
  // pile-up, so on a no-pile-up sample there is simply no pile-up vertex to
  // associate the track to.
  if (not pileup_truth_container) {
    ATH_MSG_DEBUG("getTruthVertex: no " << m_pileup_truth_container_key.key()
                                        << " container in this event");
    return nullptr;
  }

  // if not, then check if from PU event
  for (size_t pu_event = 0; pu_event < pileup_truth_container->size();
       pu_event++) {
    auto truth_pu_event = pileup_truth_container->at(pu_event);
    auto truth_pu_vertex = truth_pu_event->truthVertex(1);
    int n_pu_truthparticles = truth_pu_event->nTruthParticles();

    for (int i = 0; i < n_pu_truthparticles; i++) {
      if (not truth_pu_event->truthParticle(i)) {
        continue;
      }
      if (not truth_pu_event->truthParticle(i)->isSimulationParticle() and
          truth_pu_event->truthParticle(i)->isStable() and
          truth_pu_event->truthParticle(i)->isCharged() and
          truth_particle->index() ==
              truth_pu_event->truthParticle(i)->index()) {
        return truth_pu_vertex;
      }
    }
  } // LOOP PU events
  // can't be associated to any vertex
  return nullptr;
}
