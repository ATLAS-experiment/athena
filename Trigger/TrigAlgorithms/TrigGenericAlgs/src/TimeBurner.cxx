/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TimeBurner.h"

#include "AthenaKernel/RNGWrapper.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandLandau.h"
#include "CLHEP/Random/RandomEngine.h"

#include <algorithm>
#include <chrono>
#include <thread>

namespace {
  // MPV of the standard Landau (peak of CLHEP::RandLandau::shoot()).
  constexpr double s_standardLandauMPV = -0.22278298;
}

TimeBurner::TimeBurner(const std::string& name, ISvcLocator* pSvcLocator)
: ::HypoBase(name, pSvcLocator) {}

StatusCode TimeBurner::initialize() {
  if (m_acceptFraction < 0.0 || m_acceptFraction > 1.0) {
    ATH_MSG_ERROR("AcceptFraction must be in [0,1], got " << m_acceptFraction);
    return StatusCode::FAILURE;
  }

  // Map the TimeDistribution string to the internal enum.
  // This would make easy to add new distributions in the future.
  if (m_timeDistribution == "fixed") {
    m_timeDist = TimeDist::Fixed;
  } else if (m_timeDistribution == "landau") {
    m_timeDist = TimeDist::Landau;
    if (m_landauSigma <= 0.0) {
      ATH_MSG_ERROR("LandauSigma must be > 0 for TimeDistribution=\"landau\", got "
                    << m_landauSigma);
      return StatusCode::FAILURE;
    }
  } else {
    ATH_MSG_ERROR("Unknown TimeDistribution \"" << m_timeDistribution
                  << "\". Allowed values: \"fixed\", \"landau\".");
    return StatusCode::FAILURE;
  }

  // we don't actually need the HypoTool
  for (auto& tool : m_hypoTools) tool.disable();

  // Only retrieve the CPU cruncher if we actually use it.
  if (m_burnCPU) {
    ATH_CHECK(m_cpuCrunchSvc.retrieve());
  }

  ATH_CHECK(m_rngSvc.retrieve());

  ATH_MSG_INFO("TimeDistribution  = " << m_timeDistribution);
  if (m_timeDist == TimeDist::Fixed) {
    ATH_MSG_INFO("SleepTimeMillisec = " << m_sleepTimeMillisec << " ms");
  } else {
    ATH_MSG_INFO("LandauMPV         = " << m_landauMPV << " ms");
    ATH_MSG_INFO("LandauSigma       = " << m_landauSigma << " ms");
  }
  ATH_MSG_INFO("AcceptFraction    = " << m_acceptFraction);
  ATH_MSG_INFO("BurnCPU           = " << (m_burnCPU ? "true (busy-wait)" : "false (sleep)"));
  ATH_MSG_INFO("MaxTimeMs         = " << m_maxTimeMs << " ms"
               << (m_maxTimeMs > 0.0 ? "" : " (cap disabled)"));

  return StatusCode::SUCCESS;
}

StatusCode TimeBurner::execute(const EventContext& eventContext) const {
  using namespace TrigCompositeUtils;

  ++m_nSeen;

  // Determine the per-event duration and accept/reject roll.
  ATHRNG::RNGWrapper* rngWrapper = m_rngSvc->getEngine(this);
  rngWrapper->setSeed(name(), eventContext);
  CLHEP::HepRandomEngine* engine = rngWrapper->getEngine(eventContext);

  double sleepMs = 0.0;
  switch (m_timeDist) {
    case TimeDist::Landau:
      // CLHEP::RandLandau samples the standard Landau. Transforming it to MVP and sigma from properties.
      sleepMs = m_landauMPV + m_landauSigma * (CLHEP::RandLandau::shoot(engine) - s_standardLandauMPV);
      break;
    case TimeDist::Fixed:
      sleepMs = static_cast<double>(m_sleepTimeMillisec);
      break;
  }

  bool accept = false;
  if (m_acceptFraction > 0.0) {
    accept = (CLHEP::RandFlat::shoot(engine) < m_acceptFraction);
  }

  // Landau is unbounded below; clip to non-negative.
  sleepMs = std::max(0.0, sleepMs);
  // Optionally cap from above to avoid HLT timeouts.
  if (m_maxTimeMs > 0.0) {
    sleepMs = std::min(sleepMs, m_maxTimeMs.value());
  }
  const auto duration = std::chrono::duration<double, std::milli>(sleepMs);
  if (m_burnCPU) {
    ATH_MSG_DEBUG("Burning CPU for " << sleepMs << " ms");
    m_cpuCrunchSvc->crunch_for(
        std::chrono::duration_cast<std::chrono::milliseconds>(duration));
  } else {
    ATH_MSG_DEBUG("Sleeping for " << sleepMs << " ms");
    std::this_thread::sleep_for(duration);
  }

  // Read the previous-step decisions and create the output decision container.
  // Since TimeBurner does not produce its own physics object, reproduced
  // PEBInfoWriterAlg behaviour: attach a dummy self-link into the output
  // DecisionContainer as the feature, and disable downstream ComboHypo
  // multiplicity checks.
  SG::ReadHandle<DecisionContainer> previousDecisionsHandle(decisionInput(),
                                                            eventContext);
  ATH_CHECK(previousDecisionsHandle.isValid());

  SG::WriteHandle<DecisionContainer> outputHandle =
      createAndStore(decisionOutput(), eventContext);
  DecisionContainer* outputDecisions = outputHandle.ptr();
  for (const Decision* previous : *previousDecisionsHandle) {
    Decision* newD =
        newDecisionIn(outputDecisions, previous, hypoAlgNodeName(), eventContext);

    // Dummy self-link as feature, to satisfy navigation/runtimeValidation.
    ElementLink<DecisionContainer> dummyLink(*outputDecisions,
                                             outputDecisions->size() - 1,
                                             eventContext);
    newD->setObjectLink(featureString(), dummyLink);

    // Disable ComboHypo checks on the output of this terminal step.
    newD->setDetail<int32_t>("noCombo", 1);

    if (accept) {
      insertDecisionIDs(previous, newD);
    }
  }

  if (accept) {
    ++m_nAccepted;
    ATH_MSG_DEBUG("Event accepted");
  } else {
    ++m_nRejected;
    ATH_MSG_DEBUG("Event rejected");
  }

  ATH_CHECK(hypoBaseOutputProcessing(outputHandle));
  return StatusCode::SUCCESS;
}

StatusCode TimeBurner::finalize() {
  ATH_MSG_INFO("Summary: seen=" << m_nSeen.load()
                                << " accepted=" << m_nAccepted.load()
                                << " rejected=" << m_nRejected.load());
  return StatusCode::SUCCESS;
}
