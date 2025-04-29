/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file SingleConditionsDatum.h
 * @author Shaun Roe
 * @date April, 2025
 * @brief Class for single value per channel, with defaults, 
 * limits, and invalid value defined
 */

#ifndef SingleConditionsDatum_h
#define SingleConditionsDatum_h

#include <unordered_map>


template<typename T, T LoLimit, T HiLimit, T Default, T NonsenseValue>
class SingleConditionsDatum {
  public:
  void defaultValue(T v){m_defaultValue = v;}
  void useDefaultValue(bool b){m_alwaysUseDefault = b;}
  void setChanToDefault(int chanNum){m_valueMap[chanNum] = m_defaultValue;}
  void setValue(int chanNum, T value) {
    auto beyondLimits = [=](auto v) {return v<LoLimit or v>HiLimit;};
    m_valueMap[chanNum] = beyondLimits(value) ? m_defaultValue : value;
  }
    //
    T getValue(const int chanNum) const {
      if (m_alwaysUseDefault) return m_defaultValue;
      auto itr = m_valueMap.find(chanNum);
      if (itr!=m_valueMap.end()) { return itr->second; }
      return NonsenseValue;
    }
    bool  useDefaultValue() const {return m_alwaysUseDefault;}
    float defaultValue() const {return m_defaultValue;}

  private:
    T m_defaultValue{Default};
    bool m_alwaysUseDefault{};
    std::unordered_map<int, T>  m_valueMap;
};


#endif

