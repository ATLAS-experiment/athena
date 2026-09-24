/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//
// includes
//

#include <JetAnalysisAlgorithms/BJetRegressionAlg.h>

#include <cmath>
#include <memory>
#include <vector>

//
// method implementations
//

namespace CP
{

  StatusCode BJetRegressionAlg ::
  initialize ()
  {
    if (m_regressedPtName.empty())
    {
      ATH_MSG_ERROR ("regressedPtName is not set; it must name the decoration "
                     "holding the absolute regressed pT, e.g. bJR4v01_pt");
      return StatusCode::FAILURE;
    }
    m_doMass = !m_regressedMassName.empty();

    m_regPtAcc = std::make_unique<SG::ConstAccessor<float>> (m_regressedPtName.value());
    if (m_doMass)
      m_regMassAcc = std::make_unique<SG::ConstAccessor<float>> (m_regressedMassName.value());

    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ANA_CHECK (m_jetHandleNominal.initialize (m_systematicsList));
    ANA_CHECK (m_jetPreselection.initialize
               (m_systematicsList, m_jetHandle, SG::AllowEmpty));
    ANA_CHECK (m_ptRatio.initialize (m_systematicsList, m_jetHandle));
    if (m_doMass)
      ANA_CHECK (m_massRatio.initialize (m_systematicsList, m_jetHandle));

    ANA_CHECK (m_systematicsList.initialize());

    ATH_MSG_INFO ("Folding the b-jet regression into the jet four-momentum");
    ATH_MSG_INFO ("  regressed pT decoration:   " << m_regressedPtName.value()
                  << " (absolute, MeV)");
    if (m_doMass)
      ATH_MSG_INFO ("  regressed mass decoration: " << m_regressedMassName.value()
                    << " (absolute, MeV)");
    else
      ATH_MSG_INFO ("  the model predicts pT only; the jet mass is left untouched");
    if (m_onlyDecorate)
      ATH_MSG_INFO ("  onlyDecorate is set: writing the ratios but not modifying the jet");

    return StatusCode::SUCCESS;
  }

  StatusCode BJetRegressionAlg ::
  execute (const EventContext& ctx)
  {
    // Form the ratios ONCE, against the nominal jet, before touching anything.
    //
    // Two reasons this is not done inside the loop below. First, correctness: the
    // regression output has no systematic dependence, so a ratio taken against
    // the jet of the current systematic would cancel that systematic's shift
    // exactly and nothing would propagate. Second, safety: caching here means it
    // cannot matter whether the copy handle below aliases the nominal instance.
    const xAOD::JetContainer *nominalJets = nullptr;
    ANA_CHECK (m_jetHandleNominal.retrieve (nominalJets, CP::SystematicSet(), ctx));

    std::vector<float> ptRatio (nominalJets->size(), 1.f);
    std::vector<float> massRatio (nominalJets->size(), 1.f);
    std::vector<bool> usable (nominalJets->size(), false);
    // The prediction and the nominal jet it is referenced to, kept so that the
    // scaling below can be written as prediction x shift rather than
    // nominal x ratio. The two are algebraically the same, but the former is
    // EXACT in the nominal pass -- where the shift is identically one -- instead
    // of round-tripping the prediction through a float32 ratio.
    std::vector<float> regPtCache (nominalJets->size(), 0.f);
    std::vector<float> regMassCache (nominalJets->size(), 0.f);
    std::vector<float> nomPt (nominalJets->size(), 0.f);
    std::vector<float> nomMass (nominalJets->size(), 0.f);

    for (std::size_t i = 0; i < nominalJets->size(); ++i)
    {
      const xAOD::Jet *jet = nominalJets->at(i);

      // Which jets to correct at all. Read at the nominal systematic for the
      // reason given in the header: a pT-dependent selection evaluated per
      // variation would put a step into the systematic.
      if (!m_jetPreselection.getBool (*jet, CP::SystematicSet()))
        continue;

      if (!m_regPtAcc->isAvailable (*jet))
      {
        ATH_MSG_WARNING ("Regression decoration " << m_regressedPtName.value()
                         << " is not available on this jet; leaving it uncorrected");
        continue;
      }
      const float regPt = (*m_regPtAcc) (*jet);
      // Validity only. There is deliberately no window on the size of the
      // correction: a cut there would put an unphysical discontinuity in the
      // middle of a smooth distribution, and it is the wrong tool for the real
      // question, which is which jets the regression should be applied to at all.
      if (!std::isfinite (regPt) || regPt <= 0.f || !(jet->pt() > 0.))
      {
        ATH_MSG_WARNING ("Regressed pT " << regPt << " against nominal pT "
                         << jet->pt() << " is not usable; leaving this jet uncorrected");
        continue;
      }

      if (m_doMass)
      {
        if (!m_regMassAcc->isAvailable (*jet))
        {
          ATH_MSG_WARNING ("Regression decoration " << m_regressedMassName.value()
                           << " is not available on this jet; leaving it uncorrected");
          continue;
        }
        const float regMass = (*m_regMassAcc) (*jet);
        // A regressed mass of exactly zero is physical for a jet the model thinks
        // has no mass, so only negative and non-finite values are rejected.
        if (!std::isfinite (regMass) || regMass < 0.f)
        {
          ATH_MSG_WARNING ("Regressed mass " << regMass
                           << " is not usable; leaving this jet uncorrected");
          continue;
        }
        // A nominal mass of zero cannot be scaled to a non-zero one. Leave the
        // mass alone in that case rather than writing an infinite ratio.
        massRatio[i] = (jet->m() > 0.) ? regMass / jet->m() : 1.f;
        regMassCache[i] = regMass;
        nomMass[i] = jet->m();
      }

      ptRatio[i] = regPt / jet->pt();
      regPtCache[i] = regPt;
      nomPt[i] = jet->pt();
      usable[i] = true;
    }

    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.getCopy (jets, sys, ctx));

      // The nominal and per-systematic containers are copies of the same input,
      // so they are the same size and in the same order. Refuse rather than
      // silently pair up the wrong jets if that ever stops being true.
      if (jets->size() != nominalJets->size())
      {
        ATH_MSG_ERROR ("Jet container for systematic \"" << sys.name() << "\" has "
                       << jets->size() << " jets but the nominal one has "
                       << nominalJets->size() << "; cannot match them by index");
        return StatusCode::FAILURE;
      }

      for (std::size_t i = 0; i < jets->size(); ++i)
      {
        xAOD::Jet *jet = jets->at(i);

        m_ptRatio.set (*jet, ptRatio[i], sys);
        if (m_doMass)
          m_massRatio.set (*jet, massRatio[i], sys);

        if (m_onlyDecorate || !usable[i])
          continue;

        // pT and mass are scaled independently; eta and phi are untouched because
        // the models do not predict them.
        //
        // Written as prediction x shift, where shift is this systematic's
        // multiplicative change relative to the nominal jet. In the nominal pass
        // the shift is identically one, so the jet comes out as exactly the
        // prediction rather than the prediction round-tripped through a float32
        // ratio.
        const double newPt = regPtCache[i] * (jet->pt() / nomPt[i]);
        const double newMass = (m_doMass && nomMass[i] > 0.)
          ? regMassCache[i] * (jet->m() / nomMass[i])
          : jet->m();
        jet->setJetP4 (xAOD::JetFourMom_t (newPt, jet->eta(), jet->phi(), newMass));
      }
    }

    return StatusCode::SUCCESS;
  }

}
