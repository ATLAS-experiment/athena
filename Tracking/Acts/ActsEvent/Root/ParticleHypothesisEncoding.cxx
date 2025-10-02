/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsEvent/ParticleHypothesisEncoding.h"

#include <stdexcept>

#include "CxxUtils/AthUnlikelyMacros.h"

namespace ActsTrk::ParticleHypothesis {

xAOD::ParticleHypothesis convert(Acts::ParticleHypothesis h) {
  if (ATH_LIKELY(h == Acts::ParticleHypothesis::pion())) {
    return xAOD::pion;
  } else if ( h == Acts::ParticleHypothesis::muon()) {
    return xAOD::muon;
  } else if (h == Acts::ParticleHypothesis::electron()) {
    return xAOD::electron;
  } else if (h == Acts::ParticleHypothesis::geantino()) {
    return xAOD::geantino;
  } else {
    throw std::domain_error(
          "ActsTrk::ParticleHypothesis conversion to xAOD does not handle particle of abs(pdg)" + std::to_string(h.absolutePdg()));
  }
  return xAOD::undefined;
}

Acts::ParticleHypothesis convert(Trk::ParticleHypothesis h) {
  switch (h) {
      using enum Trk::ParticleHypothesis;
      case pion:                             
        return Acts::ParticleHypothesis::pion();
      case geantino:
        return Acts::ParticleHypothesis::chargedGeantino();
      case electron:
        return Acts::ParticleHypothesis::electron();
      case muon:
        return Acts::ParticleHypothesis::muon();
      case kaon:
        throw std::domain_error("ActsTrk::ParticleHypothesis conversion to Acts does not handle kaon");
      case proton:
        return Acts::ParticleHypothesis(Acts::PdgParticle::eProton);
      case photon:
        return Acts::ParticleHypothesis::photon();
      case neutron: 
         return Acts::ParticleHypothesis(Acts::PdgParticle::eNeutron);
       case pi0:
        return Acts::ParticleHypothesis::pion0();
      case k0:
        throw std::domain_error("ActsTrk::ParticleHypothesis conversion to Acts does not handle k0");
      case nonInteractingMuon: 
        throw std::domain_error("ActsTrk::ParticleHypothesis conversion to Acts does not handle nonInteractingMuon");
      default:
          break;
   }
   throw std::domain_error("ActsTrk::ParticleHypothesis conversion to Acts failed for" +std::to_string(h));
}

Acts::ParticleHypothesis convert(xAOD::ParticleHypothesis h) {
  switch (h) {
    case xAOD::geantino:
      return Acts::ParticleHypothesis::geantino();
    case xAOD::electron:
      return Acts::ParticleHypothesis::electron();
    case xAOD::muon:
      return Acts::ParticleHypothesis::muon();
    case xAOD::pion:
      return Acts::ParticleHypothesis::pion();
    case xAOD::kaon:
      throw std::domain_error(
          "ActsTrk::ParticleHypothesis conversion to Acts does not handle "
          "kaon");
      //      return Acts::ParticleHypothesis(321); // TODO add in ACTS
    case xAOD::proton:
      return Acts::ParticleHypothesis(Acts::PdgParticle::eProton);
    case xAOD::photon:
      return Acts::ParticleHypothesis::photon();
    case xAOD::neutron:
      throw std::domain_error(
          "ActsTrk::ParticleHypothesis conversion to Acts doe not handle "
          "neutron");
      //      return Acts::ParticleHypothesis(2112); // TODO add in ACTS
    case xAOD::pi0:
      return Acts::ParticleHypothesis::pion0();
    case xAOD::k0:
      throw std::domain_error(
          "ActsTrk::ParticleHypothesis conversion to Acts doe not handle K0");
      //      return Acts::ParticleHypothesis(311); // TODO add in ACTS
    case xAOD::nonInteractingMuon:
      throw std::domain_error(
          "ActsTrk::ParticleHypothesis conversion to Acts does not handle "
          "nonInteractingMuon");
    case xAOD::undefined:
      throw std::domain_error(
          "ActsTrk::ParticleHypothesis conversion to Acts does not handle "
          "undefined/noHypothesis");
    default:
      throw std::domain_error(
          "ActsTrk::ParticleHypothesis conversion to Acts failed for" +
          std::to_string(h));
  }
}
}  // namespace ActsTrk::ParticleHypothesis