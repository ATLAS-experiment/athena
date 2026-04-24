/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#ifndef DECORATORHELPERS_H
#define DECORATORHELPERS_H

#include <Math/Vector4D.h>
#include <TMath.h>
#include <xAODEventInfo/EventInfo.h>

/**
 * @brief Template function to fill particle information into a given object.
 *
 * This template function assigns a value to a decorator associated with an
 * object.
 *
 * @tparam T Type of the value being decorated (e.g., float, int).
 * @tparam U Type of the value to be assigned.
 * @tparam V Type of the object that will be decorated.
 * @param dec The decorator that will hold the value.
 * @param value The value to be assigned to the decorator.
 * @param object Pointer to the object that will be decorated.
 */
template <typename T, typename U, typename V>
void FillParticleInfo(const SG::Decorator<T>& dec, const U& value,
                      const V* object) {
  dec(*object) = value;
}

/**
 * @brief Template function to fill vector particle information into a given
 * object.
 *
 * @tparam T Type of the value being decorated.
 * @tparam U Type of the value to be assigned.
 * @tparam V Type of the object that will be decorated.
 * @param dec The vector decorator.
 * @param value The value to push back.
 * @param object Pointer to the object that will be decorated.
 */
template <typename T, typename U, typename V>
void FillVectorParticleInfo(const SG::Decorator<std::vector<T>>& dec,
                            const U& value, const V* object) {
  dec(*object).push_back(value);
}

/**
 * @brief Template function to fill basic particle kinematics (pt, eta, phi, m)
 * into an object.
 *
 * @tparam T Type of the value being decorated (e.g., float).
 * @tparam U Type of the object that will be decorated.
 * @param dec_pt Decorator for transverse momentum.
 * @param dec_eta Decorator for pseudorapidity.
 * @param dec_phi Decorator for azimuthal angle.
 * @param dec_m Decorator for mass.
 * @param particle The particle four-vector.
 * @param object Pointer to the object that will be decorated.
 */
template <typename T, typename U>
void FillParticleInfo(const SG::Decorator<T>& dec_pt,
                      const SG::Decorator<T>& dec_eta,
                      const SG::Decorator<T>& dec_phi,
                      const SG::Decorator<T>& dec_m,
                      const ROOT::Math::PtEtaPhiMVector& particle, U* object) {
  FillParticleInfo(dec_pt, particle.Pt(), object);
  FillParticleInfo(dec_eta, particle.Eta(), object);
  FillParticleInfo(dec_phi, particle.Phi(), object);
  FillParticleInfo(dec_m, particle.M(), object);
}

/**
 * @brief Template function to fill basic vector particle kinematics into an
 * object.
 *
 * @tparam T Type of the value being decorated.
 * @tparam U Type of the object that will be decorated.
 */
template <typename T, typename U>
void FillVectorParticleInfo(const SG::Decorator<std::vector<T>>& dec_pt,
                            const SG::Decorator<std::vector<T>>& dec_eta,
                            const SG::Decorator<std::vector<T>>& dec_phi,
                            const SG::Decorator<std::vector<T>>& dec_m,
                            const ROOT::Math::PtEtaPhiMVector& particle,
                            U* object) {
  FillVectorParticleInfo(dec_pt, particle.Pt(), object);
  FillVectorParticleInfo(dec_eta, particle.Eta(), object);
  FillVectorParticleInfo(dec_phi, particle.Phi(), object);
  FillVectorParticleInfo(dec_m, particle.M(), object);
}

/**
 * @brief Template function to fill particle kinematics and PDG ID into an
 * object.
 *
 * @tparam T Type of the value being decorated.
 * @tparam U Type of the object that will be decorated.
 */
template <typename T, typename U>
void FillParticleInfo(const SG::Decorator<T>& dec_pt,
                      const SG::Decorator<T>& dec_eta,
                      const SG::Decorator<T>& dec_phi,
                      const SG::Decorator<T>& dec_m,
                      const SG::Decorator<int>& dec_pdgId,
                      const ROOT::Math::PtEtaPhiMVector& particle, int pdgId,
                      U* object) {
  FillParticleInfo(dec_pdgId, pdgId, object);
  FillParticleInfo(dec_pt, dec_eta, dec_phi, dec_m, particle, object);
}

/**
 * @brief Template function to fill vector particle kinematics and PDG ID into
 * an object.
 *
 * @tparam T Type of the value being decorated.
 * @tparam U Type of the object that will be decorated.
 */
template <typename T, typename U>
void FillVectorParticleInfo(const SG::Decorator<std::vector<T>>& dec_pt,
                            const SG::Decorator<std::vector<T>>& dec_eta,
                            const SG::Decorator<std::vector<T>>& dec_phi,
                            const SG::Decorator<std::vector<T>>& dec_m,
                            const SG::Decorator<std::vector<int>>& dec_pdgId,
                            const ROOT::Math::PtEtaPhiMVector& particle,
                            int pdgId, U* object) {
  FillVectorParticleInfo(dec_pdgId, pdgId, object);
  FillVectorParticleInfo(dec_pt, dec_eta, dec_phi, dec_m, particle, object);
}

/**
 * @brief Template function to fill default (placeholder) particle kinematics
 * into an object.
 *
 * @tparam T Type of the value being decorated.
 * @tparam U Type of the object that will be decorated.
 */
template <typename T, typename U>
void FillDefaultParticleInfo(const SG::Decorator<T>& dec_pt,
                             const SG::Decorator<T>& dec_eta,
                             const SG::Decorator<T>& dec_phi,
                             const SG::Decorator<T>& dec_m, U* object) {
  FillParticleInfo(dec_pt, -1, object);
  FillParticleInfo(dec_eta, -999, object);
  FillParticleInfo(dec_phi, -999, object);
  FillParticleInfo(dec_m, -1, object);
}

/**
 * @brief Template function to fill default (placeholder) vector particle
 * kinematics into an object.
 *
 * @tparam T Type of the value being decorated.
 * @tparam U Type of the object that will be decorated.
 */
template <typename T, typename U>
void FillDefaultVectorParticleInfo(const SG::Decorator<std::vector<T>>& dec_pt,
                                   const SG::Decorator<std::vector<T>>& dec_eta,
                                   const SG::Decorator<std::vector<T>>& dec_phi,
                                   const SG::Decorator<std::vector<T>>& dec_m,
                                   U* object) {
  dec_pt(*object) = {};
  dec_eta(*object) = {};
  dec_phi(*object) = {};
  dec_m(*object) = {};
}

/**
 * @brief Template function to fill default (placeholder) particle kinematics
 * and PDG ID into an object.
 *
 * @tparam T Type of the value being decorated.
 * @tparam U Type of the object that will be decorated.
 */
template <typename T, typename U>
void FillDefaultParticleInfo(const SG::Decorator<T>& dec_pt,
                             const SG::Decorator<T>& dec_eta,
                             const SG::Decorator<T>& dec_phi,
                             const SG::Decorator<T>& dec_m,
                             const SG::Decorator<int>& dec_pdgId, U* object) {
  FillParticleInfo(dec_pdgId, 0, object);
  FillDefaultParticleInfo(dec_pt, dec_eta, dec_phi, dec_m, object);
}

/**
 * @brief Template function to fill default (placeholder) vector particle
 * kinematics and PDG ID into an object.
 *
 * @tparam T Type of the value being decorated.
 * @tparam U Type of the object that will be decorated.
 */
template <typename T, typename U>
void FillDefaultVectorParticleInfo(
    const SG::Decorator<std::vector<T>>& dec_pt,
    const SG::Decorator<std::vector<T>>& dec_eta,
    const SG::Decorator<std::vector<T>>& dec_phi,
    const SG::Decorator<std::vector<T>>& dec_m,
    const SG::Decorator<std::vector<int>>& dec_pdgId, U* object) {
  dec_pdgId(*object) = {};
  FillDefaultVectorParticleInfo(dec_pt, dec_eta, dec_phi, dec_m, object);
}

/**
 * @brief Struct to manage and apply decorators to the EventInfo object.
 *
 * PartonDecorator centralises all decoration of the xAOD::EventInfo for a
 * single parton history instance. Two pieces of per-instance state are set
 * once and reused:
 *
 *  - Prefix (setPrefix): prepended to every decorator name so that multiple
 *    histories running in parallel do not overwrite each other's branches.
 *  - EventInfo (setEventInfo): the target object for all decorate* calls;
 *    set once per event at the start of runHistorySaver.
 *
 * All decorate* methods therefore take only the branch name and the value(s)
 * to write — no object pointer argument is needed at call sites.
 */
struct PartonDecorator {
  std::map<std::string, std::unique_ptr<SG::Decorator<float>>> floatDecorators;
  std::map<std::string, std::unique_ptr<SG::Decorator<std::vector<float>>>>
      vectorfloatDecorators;
  std::map<std::string, std::unique_ptr<SG::Decorator<int>>> intDecorators;
  std::map<std::string, std::unique_ptr<SG::Decorator<std::vector<int>>>>
      vectorintDecorators;

  /**
   * @brief Set the prefix prepended to all decorator names.
   *
   * Must be called before any initialize*() calls. All public methods accept
   * bare names (e.g. "MC_Z_beforeFSR") and the prefix is applied internally.
   *
   * @param prefix The prefix string (e.g. "Tzq"). May be empty.
   */
  void setPrefix(const std::string& prefix) { m_prefix = prefix; }

  /**
   * @brief Set the EventInfo object to decorate for the current event.
   *
   * Must be called once per event before any decorate*() calls. The pointer
   * is not owned and must remain valid for the duration of event processing.
   *
   * @param eventInfo Pointer to the current xAOD::EventInfo.
   */
  void setEventInfo(const xAOD::EventInfo* eventInfo) {
    m_eventInfo = eventInfo;
  }

  // ── Initialize methods ────────────────────────────────────────────────────

  /** @brief Initialize a float decorator by bare name. */
  void initializeFloatDecorator(const std::string& name) {
    const std::string key = fullName(name);
    floatDecorators[key] = std::make_unique<SG::Decorator<float>>(key);
  }

  /** @brief Initialize a vector-of-float decorator by bare name. */
  void initializeVectorFloatDecorator(const std::string& name) {
    const std::string key = fullName(name);
    vectorfloatDecorators[key] =
        std::make_unique<SG::Decorator<std::vector<float>>>(key);
  }

  /** @brief Initialize multiple float decorators by a list of bare names. */
  void initializeFloatDecorator(const std::vector<std::string>& names) {
    for (const auto& name : names)
      initializeFloatDecorator(name);
  }

  /**
   * @brief Initialize pt, eta, phi, m float decorators for a bare prefix.
   *
   * @param prefix Bare prefix (e.g. "MC_Z_beforeFSR"); "_pt" etc. are appended.
   */
  void initializePtEtaPhiMDecorator(const std::string& prefix) {
    initializeFloatDecorator(prefix + "_pt");
    initializeFloatDecorator(prefix + "_eta");
    initializeFloatDecorator(prefix + "_phi");
    initializeFloatDecorator(prefix + "_m");
  }

  /**
   * @brief Initialize vector pt, eta, phi, m float decorators for a bare
   * prefix.
   */
  void initializeVectorPtEtaPhiMDecorator(const std::string& prefix) {
    initializeVectorFloatDecorator(prefix + "_pt");
    initializeVectorFloatDecorator(prefix + "_eta");
    initializeVectorFloatDecorator(prefix + "_phi");
    initializeVectorFloatDecorator(prefix + "_m");
  }

  /** @brief Initialize an integer decorator by bare name. */
  void initializeIntDecorator(const std::string& name) {
    const std::string key = fullName(name);
    intDecorators[key] = std::make_unique<SG::Decorator<int>>(key);
  }

  /** @brief Initialize a vector-of-int decorator by bare name. */
  void initializeVectorIntDecorator(const std::string& name) {
    const std::string key = fullName(name);
    vectorintDecorators[key] =
        std::make_unique<SG::Decorator<std::vector<int>>>(key);
  }

  /** @brief Initialize multiple integer decorators by a list of bare names. */
  void initializeIntDecorator(const std::vector<std::string>& names) {
    for (const auto& name : names)
      initializeIntDecorator(name);
  }

  // ── Getter methods ────────────────────────────────────────────────────────

  /**
   * @brief Retrieve a pointer to an integer decorator by bare name.
   * @throws std::runtime_error if the decorator is not found.
   */
  SG::Decorator<int>* getIntDecorator(const std::string& name) const {
    const std::string key = fullName(name);
    auto it = intDecorators.find(key);
    if (it != intDecorators.end())
      return it->second.get();
    throw std::runtime_error("Decorator with name " + key + " not found.");
  }

  /**
   * @brief Retrieve a pointer to a vector-of-int decorator by bare name.
   * @throws std::runtime_error if the decorator is not found.
   */
  SG::Decorator<std::vector<int>>* getVectorIntDecorator(
      const std::string& name) const {
    const std::string key = fullName(name);
    auto it = vectorintDecorators.find(key);
    if (it != vectorintDecorators.end())
      return it->second.get();
    throw std::runtime_error("Decorator with name " + key + " not found.");
  }

  /**
   * @brief Retrieve a pointer to a float decorator by bare name.
   * @throws std::runtime_error if the decorator is not found.
   */
  SG::Decorator<float>* getFloatDecorator(const std::string& name) const {
    const std::string key = fullName(name);
    auto it = floatDecorators.find(key);
    if (it != floatDecorators.end())
      return it->second.get();
    throw std::runtime_error("Decorator with name " + key + " not found.");
  }

  /**
   * @brief Retrieve a pointer to a vector-of-float decorator by bare name.
   * @throws std::runtime_error if the decorator is not found.
   */
  SG::Decorator<std::vector<float>>* getVectorFloatDecorator(
      const std::string& name) const {
    const std::string key = fullName(name);
    auto it = vectorfloatDecorators.find(key);
    if (it != vectorfloatDecorators.end())
      return it->second.get();
    throw std::runtime_error("Decorator with name " + key + " not found.");
  }

  // ── Decorate methods ──────────────────────────────────────────────────────
  // All write to m_eventInfo. Call setEventInfo() once per event first.

  /**
   * @brief Decorate with default particle kinematics (bare prefix).
   *
   * Writes sentinel values: pt=-1, eta=-999, phi=-999, m=-1, pdgId=0.
   */
  void decorateDefault(const std::string& prefix) {
    FillDefaultParticleInfo(
        *getFloatDecorator(prefix + "_pt"), *getFloatDecorator(prefix + "_eta"),
        *getFloatDecorator(prefix + "_phi"), *getFloatDecorator(prefix + "_m"),
        *getIntDecorator(prefix + "_pdgId"), m_eventInfo);
  }

  /** @brief Decorate with default vector particle kinematics (bare prefix). */
  void decorateVectorDefault(const std::string& prefix) {
    FillDefaultVectorParticleInfo(*getVectorFloatDecorator(prefix + "_pt"),
                                  *getVectorFloatDecorator(prefix + "_eta"),
                                  *getVectorFloatDecorator(prefix + "_phi"),
                                  *getVectorFloatDecorator(prefix + "_m"),
                                  *getVectorIntDecorator(prefix + "_pdgId"),
                                  m_eventInfo);
  }

  /**
   * @brief Decorate with default particle kinematics, no PDG ID (bare prefix).
   */
  void decorateDefaultNoPdgId(const std::string& prefix) {
    FillDefaultParticleInfo(*getFloatDecorator(prefix + "_pt"),
                            *getFloatDecorator(prefix + "_eta"),
                            *getFloatDecorator(prefix + "_phi"),
                            *getFloatDecorator(prefix + "_m"), m_eventInfo);
  }

  /** @brief Decorate with a custom float value (bare name). */
  void decorateCustom(const std::string& name, float value) {
    FillParticleInfo(*getFloatDecorator(name), value, m_eventInfo);
  }

  /** @brief Decorate with a custom integer value (bare name). */
  void decorateCustom(const std::string& name, int value) {
    FillParticleInfo(*getIntDecorator(name), value, m_eventInfo);
  }

  /**
   * @brief Decorate with particle kinematics, no PDG ID (bare prefix).
   *
   * @param prefix Bare decorator prefix.
   * @param p The particle four-vector.
   */
  void decorateParticle(const std::string& prefix,
                        const ROOT::Math::PtEtaPhiMVector& p) {
    FillParticleInfo(*getFloatDecorator(prefix + "_pt"),
                     *getFloatDecorator(prefix + "_eta"),
                     *getFloatDecorator(prefix + "_phi"),
                     *getFloatDecorator(prefix + "_m"), p, m_eventInfo);
  }

  /**
   * @brief Decorate with particle kinematics and PDG ID (bare prefix).
   *
   * @param prefix Bare decorator prefix.
   * @param p The particle four-vector.
   * @param pdgId The PDG ID of the particle.
   */
  void decorateParticle(const std::string& prefix,
                        const ROOT::Math::PtEtaPhiMVector& p, int pdgId) {
    FillParticleInfo(
        *getFloatDecorator(prefix + "_pt"), *getFloatDecorator(prefix + "_eta"),
        *getFloatDecorator(prefix + "_phi"), *getFloatDecorator(prefix + "_m"),
        *getIntDecorator(prefix + "_pdgId"), p, pdgId, m_eventInfo);
  }

  /**
   * @brief Decorate with a vector of particle kinematics and PDG IDs (bare
   * prefix).
   *
   * @param prefix Bare decorator prefix.
   * @param vec_p Vector of particle four-vectors.
   * @param vec_pdgId Vector of PDG IDs.
   */
  void decorateVectorParticle(
      const std::string& prefix,
      const std::vector<ROOT::Math::PtEtaPhiMVector>& vec_p,
      const std::vector<int>& vec_pdgId) {
    for (size_t i = 0; i < vec_p.size(); i++) {
      FillVectorParticleInfo(*getVectorFloatDecorator(prefix + "_pt"),
                             *getVectorFloatDecorator(prefix + "_eta"),
                             *getVectorFloatDecorator(prefix + "_phi"),
                             *getVectorFloatDecorator(prefix + "_m"),
                             *getVectorIntDecorator(prefix + "_pdgId"),
                             vec_p.at(i), vec_pdgId.at(i), m_eventInfo);
    }
  }

 private:
  std::string m_prefix;  ///< Prefix prepended to all decorator names.
  const xAOD::EventInfo* m_eventInfo{
      nullptr};  ///< Target object; set once per event via setEventInfo().

  /**
   * @brief Returns the full (prefixed) decorator key for internal use.
   */
  std::string fullName(const std::string& name) const {
    return m_prefix.empty() ? name : m_prefix + "_" + name;
  }
};

#endif  // DECORATORHELPERS_H
