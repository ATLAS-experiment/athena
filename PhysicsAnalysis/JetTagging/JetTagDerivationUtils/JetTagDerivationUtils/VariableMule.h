/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef VARIABLE_MULE_H
#define VARIABLE_MULE_H

#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "Gaudi/Algorithm.h"
#include "AthenaBaseComps/AthCheckMacros.h"


template <typename T>
struct MatchedPair
{
  typename T::const_value_type from;
  typename T::const_value_type to;
};

template <typename T, typename C>
class VariableMule
{
public:
  std::map<std::string,std::string> toCopy;
private:
  using RC = SG::ReadDecorHandleKeyArray<C>;
  using WC = SG::WriteDecorHandleKeyArray<C>;
  RC m_fromKeys;
  WC m_toKeys;
  T m_default;
  std::vector<SG::AuxElement::ConstAccessor<T>> m_fromAcc;
public:
  VariableMule(const T& default_value): m_default(default_value) {}
  StatusCode initialize(Gaudi::Algorithm* parent,
                        const std::vector<std::string>& froms,
                        const std::string& to) {
    for (const auto& key: toCopy) {
      std::string fullto = to + "." + key.second;
      std::string doc = "Key to move " + key.first + "->" + key.second;
      for (const auto& from: froms) {
        std::string fullfrom = from + "." + key.first;
        m_fromKeys.emplace_back(parent, "read_" + key.first, fullfrom, doc);
      }
      m_fromAcc.emplace_back(key.first);
      m_toKeys.emplace_back(parent, "write_" + key.second, fullto, doc);
    }
    ATH_CHECK(m_fromKeys.initialize());
    ATH_CHECK(m_toKeys.initialize());
    return StatusCode::SUCCESS;
  }
  void copy(const std::vector<MatchedPair<C>>& pairs, const EventContext& cxt)
    const {
    std::vector<SG::ReadDecorHandle<C,T>> from;
    std::vector<SG::WriteDecorHandle<C,T>> to;
    for (const auto& k: m_fromKeys) {
      from.emplace_back(k, cxt);
    }
    for (const auto& k: m_toKeys) {
      to.emplace_back(k, cxt);
    }
    size_t n_keys = m_fromAcc.size();
    for (const auto& pair: pairs) {
      for (size_t iii = 0; iii < n_keys; iii++) {
        if (pair.from) {
          to.at(iii)(*pair.to) = m_fromAcc.at(iii)(*pair.from);
        } else {
          to.at(iii)(*pair.to) = m_default;
        }
      }
    }
  }
};


#endif
