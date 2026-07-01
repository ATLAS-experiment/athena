/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#include "EventSelectionAlgorithms/ObjectKinematicSelectorAlg.h"
#include "Math/Vector4D.h"
#include "TVector2.h"

#include <algorithm>
#include <cmath>

using ROOT::Math::PtEtaPhiEVector;

namespace CP {

  ObjectKinematicSelectorAlg::ObjectKinematicSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator)
  : EL::AnaAlgorithm(name, pSvcLocator)
  {}

  StatusCode ObjectKinematicSelectorAlg::initialize() {
    ANA_CHECK(m_particlesHandles.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_metHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));

    ANA_CHECK(m_preselection.initialize(m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_decoration.initialize(m_systematicsList, m_eventInfoHandle));

    // sanity checks on the configuration (the Python config guarantees these,
    // but we fail early and clearly if the algorithm is driven directly)
    std::size_t nParticleOperands = 0;
    bool usesMET = false;
    for (const std::string &kind : m_operandKinds) {
      if (kind == "PARTICLE") ++nParticleOperands;
      else if (kind == "MET") usesMET = true;
      else {
        ANA_MSG_ERROR("EXPR: unknown operand kind '" << kind << "'");
        return StatusCode::FAILURE;
      }
    }
    if (nParticleOperands != m_particlesHandles.size() ||
        nParticleOperands != m_selectionKeys.size() ||
        nParticleOperands != m_indices.size()) {
      ANA_MSG_ERROR("EXPR: inconsistent operand configuration (particle handles/selections/indices mismatch)");
      return StatusCode::FAILURE;
    }
    if (usesMET && !m_metHandle) {
      ANA_MSG_ERROR("EXPR: a MET operand was requested but no MET container was configured");
      return StatusCode::FAILURE;
    }

    // build one selection handle per particle operand, tied to its container
    m_selections.clear();
    m_selections.reserve(m_particlesHandles.size());
    for (std::size_t i = 0; i < m_particlesHandles.size(); ++i) {
      m_selections.emplace_back(m_selectionKeys[i], this);
      ANA_CHECK(m_selections.back().initialize(m_systematicsList, m_particlesHandles.at(i), SG::AllowEmpty));
    }

    ANA_CHECK(m_systematicsList.initialize());

    m_signEnum = SignEnum::stringToOperator.at( m_sign );

    return StatusCode::SUCCESS;
  }

  StatusCode ObjectKinematicSelectorAlg::execute(const EventContext& ctx) {

    // a minimal kinematic record per operand; eta/e are unused for MET
    struct Kin { double pt{0.}, eta{0.}, phi{0.}, e{0.}; bool isMET{false}; };

    const std::string &var = m_variable.value();

    for (const auto &sys : m_systematicsList.systematicsVector()) {
      // retrieve the EventInfo
      const xAOD::EventInfo *evtInfo = nullptr;
      ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, sys, ctx));

      // default-decorate EventInfo
      m_decoration.setBool(*evtInfo, 0, sys);

      // check the preselection
      if (m_preselection && !m_preselection.getBool(*evtInfo, sys))
        continue;

      // build the per-operand kinematics, in expression order
      std::vector<Kin> ops;
      ops.reserve(m_operandKinds.size());
      std::size_t pIdx = 0; // running index over the particle handle array

      for (const std::string &kind : m_operandKinds) {
        Kin k;
        if (kind == "MET") {
          const xAOD::MissingETContainer *met = nullptr;
          ANA_CHECK(m_metHandle.retrieve(met, sys, ctx));
          const xAOD::MissingET *term = (*met)[m_metTerm.value()];
          if (term == nullptr) {
            ANA_MSG_ERROR("EXPR: the MET term '" << m_metTerm.value() << "' does not exist! Aborting.");
            return StatusCode::FAILURE;
          }
          k.pt = term->met();
          k.phi = term->phi();
          k.isMET = true;
          ops.push_back(k);
          continue;
        }

        // PARTICLE: collect objects passing the selection, pT-order, pick index
        const xAOD::IParticleContainer *cont = nullptr;
        ANA_CHECK(m_particlesHandles.at(pIdx).retrieve(cont, sys, ctx));

        std::vector<const xAOD::IParticle*> passing;
        passing.reserve(cont->size());
        for (const xAOD::IParticle *p : *cont) {
          if (!m_selections.at(pIdx) || m_selections.at(pIdx).getBool(*p, sys))
            passing.push_back(p);
        }
        std::sort(passing.begin(), passing.end(),
                  [](const xAOD::IParticle *a, const xAOD::IParticle *b) {
                    return a->pt() > b->pt();
                  });

        const int requested = m_indices[pIdx];
        if (requested < 0 || static_cast<std::size_t>(requested) >= passing.size()) {
          ANA_MSG_ERROR("EXPR: object index " << requested << " requested but only "
                        << passing.size() << " object(s) pass the selection; this event "
                        "selection cannot run. Add the appropriate multiplicity requirement "
                        "upstream (e.g. JET_N >= ...).");
          return StatusCode::FAILURE;
        }

        const xAOD::IParticle *obj = passing[requested];
        k.pt = obj->pt();
        k.eta = obj->eta();
        k.phi = obj->phi();
        k.e = obj->e();
        ops.push_back(k);
        ++pIdx;
      }

      // compute the requested variable
      double value = 0.;
      if (var == "dR") {
        const double deta = ops[0].eta - ops[1].eta;
        const double dphi = TVector2::Phi_mpi_pi(ops[0].phi - ops[1].phi);
        value = std::sqrt(deta * deta + dphi * dphi);
      } else if (var == "dEta") {
        value = std::abs(ops[0].eta - ops[1].eta);
      } else if (var == "dPhi") {
        value = std::abs(TVector2::Phi_mpi_pi(ops[0].phi - ops[1].phi));
      } else if (var == "eta") {
        value = ops[0].eta;
      } else if (var == "phi") {
        value = ops[0].phi;
      } else if (var == "m" || var == "e" || var == "pt") {
        if (ops.size() == 1 && ops[0].isMET) {
          // only pt(met) reaches here (guaranteed by the config-time validation)
          value = ops[0].pt;
        } else {
          PtEtaPhiEVector sum;
          for (const Kin &k : ops)
            sum += PtEtaPhiEVector(k.pt, k.eta, k.phi, k.e);
          if (var == "m")      value = sum.M();
          else if (var == "e") value = sum.E();
          else                 value = sum.Pt();
        }
      } else {
        ANA_MSG_ERROR("EXPR: unsupported variable '" << var << "'");
        return StatusCode::FAILURE;
      }

      // calculate decision
      bool decision = SignEnum::checkValue(m_refValue.value(), m_signEnum, static_cast<float>(value));
      m_decoration.setBool(*evtInfo, decision, sys);
    }
    return StatusCode::SUCCESS;
  }

} // namespace CP
