/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRUTHUTILS_HEPMCHELPERS_H
#define TRUTHUTILS_HEPMCHELPERS_H
#include <vector>
#include <cmath>
#include <algorithm>
#include <array>
#include <cstdlib>
#include <set>
#include "TruthUtils/MagicNumbers.h"

/// @file
///
/// ATLAS-specific HepMC functions

namespace MC
{
 namespace Pythia8
 {
   /// @brief  To be understood
  template <class T> inline bool isConditionA(const T& p)  { return p->status() == 62 || p->status() == 52 || p->status() == 21 || p->status() == 22;}

  template <class T> inline bool isConditionB(const T& p)  { return p->status() == 23;}

  template <class T> inline bool isConditionC(const T& p)  { return p->status() > 30 && p->status() < 40;}
 }

#include "AtlasPID.h"

  /// @brief Identify if the particle with given PDG ID would not interact with the detector, i.e. not a neutrino or WIMP
  template <class T> inline bool isInteracting(const T& p) { return isStrongInteracting<T>(p) || isEMInteracting<T>(p) || isGeantino<T>(p); }

  /// @brief Identify if the particle with given PDG ID would produce ID tracks but not shower in the detector if stable
  template <class T> inline  bool isChargedNonShowering(const T& p) { return (isMuon<T>(p) || isSUSY<T>(p)); }

  /// @brief Identify if the particle is beam particle
  template <class T> inline bool isBeam(const T& p)  { return HepMC::status(p)%HepMC::SIM_STATUS_THRESHOLD == 4;}

  /// @brief Identify if the particle decayed
  template <class T> inline bool isDecayed(const T& p)  { return HepMC::status(p)%HepMC::SIM_STATUS_THRESHOLD == 2;}

  /// @brief Identify if the particle is stable, i.e. has not decayed
  template <class T> inline bool isStable(const T& p)   { return HepMC::status(p)%HepMC::SIM_STATUS_THRESHOLD == 1;}

  /// @brief Identify if the particle is final state particle
  template <class T> inline bool isFinalState(const T& p)   { return HepMC::status(p)%HepMC::SIM_STATUS_THRESHOLD == 1 && !p->end_vertex();}

  /// @brief Identify if the particle is physical, i.e. is stable or decayed 
  template <class T> inline bool isPhysical(const T& p) { return isStable<T>(p) || isDecayed<T>(p); }

  /// @brief Determine if the particle is stable at the generator (not det-sim) level,
  template <class T> inline bool isGenStable(const T& p) { return isStable<T>(p) && !HepMC::is_simulation_particle<T>(p);}

  /// @brief Identify if the particle is considered stable at the post-detector-sim stage
  template <class T> inline bool isSimStable(const T& p) { return  isStable<T>(p) &&  !p->end_vertex() && HepMC::is_simulation_particle<T>(p);}

  /// @brief Identify if the particle could interact with the detector during the simulation, e.g. not a neutrino or WIMP
  template <class T> inline bool isSimInteracting(const T& p) { return isGenStable<T>(p) && isInteracting<T>(p);}

  /// @brief Identify if particle is satble or decayed in simulation. + a pathological case of decayed particle w/o end vertex.
  /// The decayed particles w/o end vertex might occur in case of simulation of long lived particles in Geant stripped off the decay products. 
  /// I.e. those particles should be re-decayed later.
  template <class T> inline bool isStableOrSimDecayed(const T& p) {
    const auto vertex = p->end_vertex();
    return ( isStable<T>(p) || (isDecayed<T>(p) && (!vertex || HepMC::is_simulation_vertex(vertex))));
  }

  /// @brief Identify a photon with zero energy. Probably a workaround for a generator bug.
  template <class T> inline bool isZeroEnergyPhoton(const T&  p) { return isPhoton<T>(p) && p->e() == 0;}

  /// @brief Identify a particlegun particle
  template <class T> inline bool isSingleParticle(const T&  p) { 
#if defined(HEPMC3)  
    return HepMC::barcode(p)  == 3 ||  HepMC::barcode(p)  == 10001;
#else
    return HepMC::barcode(p)  == 10001;
#endif
  }

  /// @brief Identify a special non-interacting particles.
  template <class T> inline bool isSpecialNonInteracting(const T& p) {
    const int apid = std::abs(p->pdg_id());
    if (apid == NU_E || apid == NU_MU || apid == NU_TAU) return true; //< neutrinos
    if (apid == 1000022 || apid == 1000024 || apid == 5100022) return true; // SUSY & KK photon and Z partners
    if (apid == GRAVITON || apid == 1000039 || apid == 5000039) return true; //< gravitons: standard, SUSY and KK
    if (apid == 9000001 || apid == 9000002 || apid == 9000003 || apid == 9000004 || apid == 9000005 || apid == 9000006) return true; //< exotic particles from monotop model
    return false;
  }

  /// @brief Function to get a mother of particle. MCTruthClassifier legacy.
  /** This can be used for HepMC3::GenVertexPtr, HepMC3::ConstGenVertexPtr or xAOD::TruthVertex* */  
  template <class T> T findMother(T thePart) {
    auto partOriVert = thePart->production_vertex();
    if (!partOriVert) return nullptr;

    long partPDG = thePart->pdg_id();
    long MotherPDG(0);

    auto MothOriVert = partOriVert;
    MothOriVert = nullptr;
    T theMoth(nullptr);

    size_t itr = 0;
    do {
      if (itr != 0) partOriVert = MothOriVert;
      auto incoming = partOriVert->particles_in();
      for ( auto p: incoming) {
        theMoth = p;
        if (!theMoth) continue;
        MotherPDG = theMoth->pdg_id();
        MothOriVert = theMoth->production_vertex();
        if (MotherPDG == partPDG) break;
      }
      itr++;
      if (itr > 100) {
        break;
      }
    } while (MothOriVert != nullptr && MotherPDG == partPDG && !HepMC::is_simulation_particle(thePart) &&
           MothOriVert != partOriVert);
    return theMoth;
  }

  /// @brief Function to find a particle in container
  /** This can be used for HepMC3::GenVertexPtr, HepMC3::ConstGenVertexPtr or xAOD::TruthVertex* */  
  template <class C, class T>  T findMatching(C TruthContainer, T p) {
    T ptrPart = nullptr;
    if (!p) return ptrPart;
    if constexpr (std::is_pointer_v<C> || HepMC::is_smart_ptr_v<C>){ //C is ptr
      for (T truthParticle : *TruthContainer) {
        if (HepMC::is_sim_descendant(p,truthParticle)) {
          ptrPart = truthParticle;
          break;
        }
      }
    }
    else {
      for (T truthParticle : TruthContainer) {
        if (HepMC::is_sim_descendant(p,truthParticle)) {
          ptrPart = truthParticle;
          break;
        }
      }
    }
    return ptrPart;
  }
  /// @brief Function to find all ancestors of the particle.
  /** This can be used for HepMC3::GenParticlePtr, HepMC3::ConstGenParticlePtr or xAOD::TruthParticle* */
  template <class T> void findParticleAncestors(T thePart, std::set<T>& allancestors) {
    auto prodVtx = thePart->production_vertex();
    if (!prodVtx) return;
    for (auto theMother: prodVtx->particles_in()) {
      if (!theMother) continue;
      allancestors.insert(theMother);
      findParticleAncestors(theMother, allancestors);
    }
  }

  /// @brief Function to get the particle stable MC daughters.
  /** This can be used for HepMC3::GenParticlePtr, HepMC3::ConstGenParticlePtr or xAOD::TruthParticle* */
  template <class T> void findParticleStableDescendants(T thePart, std::set<T>& allstabledescendants) {
    auto endVtx = thePart->end_vertex();
    if (!endVtx) return;
    for (auto theDaughter: endVtx->particles_out()) {
      if (!theDaughter) continue;  
      if (isStable(theDaughter) && !HepMC::is_simulation_particle(theDaughter)) {
         allstabledescendants.insert(theDaughter);
      }
      findParticleStableDescendants(theDaughter, allstabledescendants);
    }
  }

  /// @brief Function to classify the vertex as hard scattering vertex.
  /// AV: This is MCtruthClassifier legacy. Note that this function willnot capture some cases of the HardScattering vertices.
  /// The function should be improved in the future.
  /** This can be used for HepMC3::GenVertexPtr, HepMC3::ConstGenVertexPtr or xAOD::TruthVertex* */  
  template <class T>  bool isHardScatteringVertex(T pVert) {
    if (pVert == nullptr) return false;
    T pV = pVert;
    int numOfPartIn(0);
    int pdg(0);

    do {
      pVert = pV;
      auto incoming = pVert->particles_in();
      numOfPartIn = incoming.size();
      pdg = numOfPartIn && incoming.front() != nullptr ? incoming.front()->pdg_id() : 0;
      pV = numOfPartIn && incoming.front() != nullptr ? incoming.front()->production_vertex() : nullptr;

    } while (numOfPartIn == 1 && (std::abs(pdg) < 81 || std::abs(pdg) > 100) && pV != nullptr);

    if (numOfPartIn == 2) {
      auto incoming = pVert->particles_in();
      if (incoming.at(0) && incoming.at(1) && (std::abs(incoming.at(0)->pdg_id()) < 7 || incoming.at(0)->pdg_id() == 21) && (std::abs(incoming.at(1)->pdg_id()) < 7 || incoming.at(1)->pdg_id() == 21)) return true;
    }
    return false;
}

  /// @brief Function to classify the particle.
  /// AV: This is MCtruthClassifier legacy.
  /// The function should be improved in the future.
  /** This can be used for HepMC3::GenVertexPtr, HepMC3::ConstGenVertexPtr or xAOD::TruthVertex* */  
  template <class T> bool isFromHadron(T p, T hadron, bool &fromTau, bool &fromBSM) {
    if (isHadron(p)&&!isBeam(p))  return true; // trivial case
    auto vtx = p->production_vertex();
    if (!vtx)  return false;
    bool fromHad = false;
    auto incoming = vtx->particles_in();
    for (auto parent: incoming) {
      if (!parent) continue;
      // should this really go into parton-level territory?
      // probably depends where BSM particles are being decayed
      fromBSM |= isBSM(parent);
      if (!isPhysical(parent))  return false;
      fromTau |= isTau(parent);
      if (isHadron(parent)&&!isBeam(parent)) {
        if (!hadron)  hadron = parent; // assumes linear hadron parentage
        return true;
      }
      fromHad |= isFromHadron(parent, hadron, fromTau, fromBSM);
    }
    return fromHad;
  }

  /// @brief Function to find the end vertex of a particle.
  /// This algorithm allows for 1->1 decays. 
  /** This can be used for HepMC3::GenVertexPtr, HepMC3::ConstGenVertexPtr or xAOD::TruthVertex*  and particle counterparts*/  
  template <class T> auto findSimulatedEndVertex(T thePart) -> decltype(thePart->end_vertex()) {
     decltype(thePart->end_vertex()) EndVert = thePart->end_vertex();
     decltype(thePart->end_vertex()) pVert(nullptr);
    if (EndVert != nullptr) {
      do {
        bool samePart = false;
        pVert = nullptr;
        auto outgoing = EndVert->particles_out();
        auto incoming = EndVert->particles_in();
        for (const auto& itrDaug: outgoing) {
          if (!itrDaug) continue;
          if ((( HepMC::is_same_generator_particle(itrDaug,thePart)) ||
             // brem on generator level for tau
             (outgoing.size() == 1 && incoming.size() == 1 &&
              !HepMC::is_simulation_particle(itrDaug) && !HepMC::is_simulation_particle(thePart))) &&
            itrDaug->pdg_id() == thePart->pdg_id()) {
            samePart = true;
            pVert = itrDaug->end_vertex();
          }
        }
        if (samePart) EndVert = pVert;
      } while (pVert != nullptr && pVert != EndVert); // pVert!=EndVert to prevent Sherpa loop
    }
    return EndVert;
  }

  /// @brief Function to find the stable particle descendants of the given vertex..
  /** This can be used for HepMC3::GenVertexPtr, HepMC3::ConstGenVertexPtr or xAOD::TruthVertex*  and particle counterparts*/  
  template <class V> auto findFinalStateParticles(V theVert) -> decltype(theVert->particles_out()) {
    if (!theVert) return {};
    decltype(theVert->particles_out()) finalStatePart;
    auto outgoing = theVert->particles_out();
    for (const auto& thePart: outgoing) {
      if (!thePart) continue;
      finalStatePart.push_back(thePart);
      if (isStable(thePart)) continue;
      V pVert = findSimulatedEndVertex(thePart);
      if (pVert == theVert) break; // to prevent Sherpa  loop
      if (pVert != nullptr) {
          auto  vecPart = findFinalStateParticles<V>(pVert);
          finalStatePart.insert(finalStatePart.end(),vecPart.begin(),vecPart.end());
      }
    }
    return finalStatePart;
  }

}
#endif
