#include "HGTD_TrkTimePerformanceStudies.h"
#include "xAODTruth/xAODTruthHelpers.h"

HGTD_TrkTimePerformanceStudies::HGTD_TrkTimePerformanceStudies(
    const std::string& name, ISvcLocator* svc_locator)
    : HGTD_AnalysisAlgBase(name, svc_locator) {}

HGTD_TrkTimePerformanceStudies::~HGTD_TrkTimePerformanceStudies() {}

StatusCode HGTD_TrkTimePerformanceStudies::initialize() {
  ATH_CHECK(HGTD_AnalysisAlgBase::initialize());

  ATH_MSG_INFO("Initializing HGTD_TrkTimePerformanceStudies ...");

  for (auto tool : m_track_sel_tools) {
    CHECK(tool.retrieve());
  }

  for (auto tool : m_track_time_tools) {
    CHECK(tool.retrieve());
  }

  for (auto track_tool : m_track_sel_tools) {
    for (auto time_tool : m_track_time_tools) {

      bookEffSubdir(track_tool->name(), time_tool->name(), "m_eff_vs_eta",
                    ";|#eta| ;frequency", 32, 2.4, 4.0);
      bookEffSubdir(track_tool->name(), time_tool->name(),
                    "m_eff_gt50pcprimes_vs_eta", ";|#eta| ;frequency", 32, 2.4,
                    4.0);
      bookEffSubdir(track_tool->name(), time_tool->name(),
                    "m_eff_gt50pcprimes_vs_eta_mistag", ";|#eta| ;frequency",
                    32, 2.4, 4.0);

      for (const auto& primes_fraction_i : m_primes_fractions) {
        std::string name = "m_eff_vs_eta_primesfrac" + primes_fraction_i;
        bookEffSubdir(track_tool->name(), time_tool->name(), name,
                      ";|#eta| ;frequency", 32, 2.4, 4.0);

        std::string name_res =
            "m_hist_timeres_outlier_cases" + primes_fraction_i;
        bookSubdir<TH1F>(track_tool->name(), time_tool->name(), name_res,
                         ";t_{reco} - t_{truth} [ns]; number of tracks", 200,
                         -.4, 0.4);
      }
    }
  }

  ATH_CHECK(m_track_particles_key.initialize());
  ATH_CHECK(m_truth_event_container_key.initialize());
  ATH_CHECK(m_pileup_truth_container_key.initialize());

  return StatusCode::SUCCESS;
}

StatusCode HGTD_TrkTimePerformanceStudies::execute(const EventContext& ctx) {

  //   ATH_CHECK(evtStore()->retrieve(m_track_particles, m_trkpart_cont_name));

  SG::ReadHandle<xAOD::TrackParticleContainer> track_particles_hdl(
      m_track_particles_key,ctx);
  const xAOD::TrackParticleContainer* track_particles =
      track_particles_hdl.cptr();

  //   if (evtStore()->retrieve(m_truth_event_container, "TruthEvents") ==
  //       StatusCode::FAILURE) {
  //     ATH_MSG_ERROR("Failed to retrieve TruthEvents collection. ");
  //     return StatusCode::SUCCESS;
  //   }

  //   if (not evtStore()
  //               ->retrieve(m_pileup_truth_container, "TruthPileupEvents")
  //               .isSuccess()) {
  //     ATH_MSG_ERROR("Failed to retrieve TruthPileupEvents container. ");
  //     return StatusCode::SUCCESS;
  //   }

  for (const auto* track : *track_particles) { // tack is not a pointer!
    for (const auto& track_tool : m_track_sel_tools) {
      for (const auto& time_tool : m_track_time_tools) {

        const float trk_eta = track->eta();

        if (std::abs(trk_eta) < 2.4) {
          continue;
        }
        if ((track->pt() / 1.e3) < 1.0) {
          continue;
        }

        if (!track_tool->trackPassesSelection(track)) {
          continue;
        }

        const xAOD::TruthParticle* truth_particle =
            xAOD::TruthHelpers::getTruthParticle(*track);

        auto purity = time_tool->fracPrimaryHits(*track);
        int n_potential_primes = time_tool->numberPotentialPrimaryHits(*track);
        bool has_time = time_tool->hasTime(*track);

        PrimesFractions primes_fraction = PrimesFractions::AllPrimes;
        if (purity < 0.01 and n_potential_primes == 0) {
          primes_fraction = PrimesFractions::NoPrimesNoPossiblePrimes;
        } else if (purity < 0.01 and n_potential_primes == 1) {
          primes_fraction = PrimesFractions::NoPrimes1PossiblePrimes;
        } else if (purity < 0.01 and n_potential_primes == 2) {
          primes_fraction = PrimesFractions::NoPrimes2PossiblePrimes;
        } else if (purity < 0.01 and n_potential_primes == 3) {
          primes_fraction = PrimesFractions::NoPrimes3PossiblePrimes;
        } else if (purity < 0.01 and n_potential_primes == 4) {
          primes_fraction = PrimesFractions::NoPrimes4PossiblePrimes;
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
        // FIXME: not used yet, maybe we can replace morethanhalfprimes by it
        // bool atleasthalfprimes =
        //     primes_fraction == PrimesFractions::AllPrimes or
        //     primes_fraction == PrimesFractions::MoreThanHalfPrimes or
        //     primes_fraction == PrimesFractions::HalfPrimesHasPrimes;

        fillEffSubDir(track_tool->name(), time_tool->name(), "m_eff_vs_eta",
                      has_time, std::abs(trk_eta));

        bool count_primesfrac_category_as_good = false;
        for (int i = 0; i < m_number_primes_fractions; i++) {
          if (i == primes_fraction) {
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
            float track_time = time_tool->time(*track);
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

StatusCode HGTD_TrkTimePerformanceStudies::finalize() {
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
  // auto truth_hs_vtx = truth_hs_event->truthVertex(1);
  auto truth_hs_vtx = truth_event_container->at(0)->signalProcessVertex();

  for (int i = 0; i < n_hs_truthparticles; i++) {
    if (not truth_hs_event->truthParticle(i)) {
      continue;
    }
    if (truth_hs_event->truthParticle(i)->uid() < 200000 and
        truth_hs_event->truthParticle(i)->uid() != 0 and
        truth_hs_event->truthParticle(i)->status() == 1 and
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
      if (truth_pu_event->truthParticle(i)->uid() < 200000 and
          truth_pu_event->truthParticle(i)->uid() != 0 and
          truth_pu_event->truthParticle(i)->status() == 1 and
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
