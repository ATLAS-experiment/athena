/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FoldDecoratorAlg.h"

#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"

#include <nlohmann/json.hpp>

#include <random>

namespace FlavorTagInference {

  FoldDecoratorAlg::FoldDecoratorAlg(
    const std::string& name, ISvcLocator* svcloc):
    AthReentrantAlgorithm(name, svcloc)
  {
  }

  StatusCode FoldDecoratorAlg::initialize()
  {
    ATH_CHECK(m_mcEventNumberKey.initialize());
    // event info is usually not decorated, renounce the dependency
    renounce(m_mcEventNumberKey);
    if (m_jetCollection.empty()) {
      ATH_MSG_ERROR("jet collection not specified");
      return StatusCode::FAILURE;
    }
    m_hashKey = m_jetCollection + "." + m_hashKey.key();
    ATH_CHECK(m_hashKey.initialize());

    // other entropy sources, mostly counts
    for (auto& key: m_jetAssociations) {
      key = m_jetCollection + "." + key.key();
    }
    ATH_CHECK(m_jetAssociations.initialize());
    // most the associations also aren't decorated, renounce these
    // dependencies too
    renounceArray(m_jetAssociations);
    for (auto& key: m_jetInts) {
      key = m_jetCollection + "." + key.key();
    }
    ATH_CHECK(m_jetInts.initialize());
    // same with jetInts: usually not decorated
    renounceArray(m_jetInts);

    // set up some salts for other sources of entropy
    std::map<std::string, uint32_t> fullSeeds;
    for (const auto& [k, v]: m_jetVarSeeds) {
      fullSeeds[m_jetCollection + "." + k] = v;
    }
    auto addHashKeys = [this, &fullSeeds](auto keys) {
      for (auto& key: keys) {
        uint32_t salt = m_salt;
        if (auto h = fullSeeds.extract(key.key())) {
          salt = std::mt19937(h.mapped())();
        }
        this->m_hashedKeys[key.key()] = salt;
      }
    };
    addHashKeys(m_jetAssociations);
    addHashKeys(m_jetInts);
    if (!fullSeeds.empty()) {
      for (const auto& [k, v]: fullSeeds) {
        ATH_MSG_ERROR("unused salt for jet variable " << k);
      }
      return StatusCode::FAILURE;
    }

    // more sources from constituents
    std::set<std::string> associations;
    for (const auto& key: m_jetAssociations) associations.insert(key.key());
    using charmap_t = std::map<std::string,std::vector<std::string>>;
    auto chars = nlohmann::json::parse(m_constituentChars);
    for (const auto& [k, vlist]: chars.get<charmap_t>()) {
      uint32_t salt = m_salt;
      std::string key = m_jetCollection + "." + k;
      if (!associations.count(key)) {
        ATH_MSG_ERROR("Constituent " << key << " is not read from the jet");
        return StatusCode::FAILURE;
      }
      for (const auto& v: vlist) {
        if (auto h = m_constituentSeeds.value().extract(v)) {
          salt = std::mt19937(h.mapped())();
        }
        m_chars[key].emplace_back(salt,v);
      }
    }
    if (!m_constituentSeeds.empty()) {
      for (const auto& [k, v]: m_constituentSeeds) {
        ATH_MSG_ERROR("unused salt for constituent variable " << k);
      }
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode FoldDecoratorAlg::execute(const EventContext& cxt) const {
    SG::ReadDecorHandle<xAOD::EventInfo, uint64_t> hnumber(
      m_mcEventNumberKey, cxt);
    SG::WriteDecorHandle<xAOD::JetContainer, uint32_t> hhash(
      m_hashKey, cxt);
    auto hjetassoc = m_jetAssociations.makeHandles(cxt);
    auto hjetint = m_jetInts.makeHandles(cxt);
    uint32_t number = hnumber(*hnumber);
    auto event_hash = std::mt19937(number ^ m_salt)();

    // get more entropy from jet variables
    auto getSalt = [this, event_hash](const auto& handle) {
      return this->m_hashedKeys.at(handle.decorKey()) ^ event_hash;
    };
    for (const auto* jet: *hhash) {
      uint32_t jet_hash = 0;
      for (const auto& assoc: hjetassoc) {
        const auto& iplc = assoc(*jet);
        jet_hash ^= std::mt19937( iplc.size() ^ getSalt(assoc))();
        if (m_chars.count(assoc.decorKey())) {
          for (const auto& [salt, acc]: m_chars.at(assoc.decorKey())) {
            unsigned int count = 0;
            for (const auto& lnk: iplc) count += acc(**lnk);
            jet_hash ^= std::mt19937(count ^ salt ^ event_hash)();
          }
        }
      }
      for (const auto& hint: hjetint) {
        jet_hash ^= std::mt19937( hint(*jet) ^ getSalt(hint))();
      }
      hhash(*jet) = event_hash ^ jet_hash;
    }
    return StatusCode::SUCCESS;
  }

} // end namespace FlavorTagInference
