/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _ZDCJSONConfig_h
#define _ZDCJSONConfig_h

#include <nlohmann/json.hpp>
#include <tuple>
#include <map>
#include <vector>
#include <string>
#include <cmath> //std::abs, std::floor


class ZDCJSONConfig
{
public:
  using JSON = nlohmann::json; 
  using JSONParamDescr = std::tuple<JSON::value_t, unsigned int, bool, bool>;
  using JSONParamList = std::map<std::string, JSONParamDescr>;

private:

  size_t m_nSides{0};
  std::vector<std::string> m_sideLabels;
  size_t m_numChannelsPerSide{0};
  bool m_haveConfig{false};
  JSON m_globalConfig;
  
  std::vector<JSON> m_channelConfig;

  template<typename T> bool 
  checkType(T value, JSON::value_t paramType, int paramSize = -1)
  {
    if (value.is_null()) return false;
    
    if (value.type() == paramType) {
      if (paramSize == -1) return true;
      else if (value.size() == size_t(paramSize)) return true;
      return false;
    }
    
    if (value.is_primitive()) {
      if (paramType == JSON::value_t::number_float) {
	if (value.type() == JSON::value_t::number_integer ||
	    value.type() == JSON::value_t::number_unsigned) return true;
      }
      else if (paramType == JSON::value_t::number_integer &&
	       value.type() == JSON::value_t::number_unsigned) return true;
      else if (paramType == JSON::value_t::number_integer &&
	       value.type() == JSON::value_t::number_float) {
	float fvalue = value;
	if (std::abs(std::floor(fvalue)-fvalue) < 1e-6) return true;
      }
    }

    return false;
  }

  template<typename T> void 
  setAllParameter(const std::string & key, T value)
  {
    for (auto& chanConfig : m_channelConfig) {
      //
      // Look to see if a value is already assigned, if so don't override
      //
      if (chanConfig.find(key) == chanConfig.end()) {
	chanConfig[key] = value;
      }
    }
  }

  template<typename T> void 
  setChannelParameter(unsigned int side, unsigned int chanIndex, const std::string& key, T value)
  {
    unsigned int index = side*m_numChannelsPerSide + chanIndex;
    m_channelConfig[index][key] = value;
  }

  template<typename T> void 
  setPerSideParameter(unsigned int side, std::string key, T value)
  {
    for (unsigned int chan = 0; chan < m_numChannelsPerSide; chan++) {
      unsigned int index = side*m_numChannelsPerSide + chan;
      //
      // Look to see if a value is already assigned, if so don't override
      //
      if (m_channelConfig[index].find(key) == m_channelConfig[index].end()) {
	m_channelConfig[index][key] = value;
      }
    }
  }

  template<typename T> std::pair<bool, std::string>
  ParsePerChannelParams(const std::string& paramKey, const T& paramValue, JSON::value_t paramType, size_t paramSize);

  
public:
  
  ZDCJSONConfig(const std::vector<std::string>& sideNames, size_t numChannelsPerSide) :
    m_nSides(sideNames.size()),
    m_sideLabels(sideNames),
    m_numChannelsPerSide(numChannelsPerSide)
  {
    m_channelConfig.assign(m_nSides*m_numChannelsPerSide, JSON());
  }

  ~ZDCJSONConfig() {}
  
  std::pair<bool, std::string> ParseConfig(const JSON& config, const JSONParamList& JSONConfigParams);

  bool haveConfig() const {return m_haveConfig;}

  template<typename T> bool getGlobalParam(std::string key, T& returnValue)
  {
    if (!haveConfig()) return false;

    try {
      auto iter = m_globalConfig.find(key);
      if (iter == m_globalConfig.end()) return false;
      if (iter.value().is_null()) return false;
      
      returnValue = iter.value().get_to(returnValue);
    }
    catch (...) {
      return false;
    }

    return true;
  }

  auto dumpGlobal() {return m_globalConfig.dump(4);}
  
  const JSON& getChannelConfig(size_t side, size_t channel) const {
    if (!(side < m_nSides && channel < m_numChannelsPerSide)) throw std::runtime_error("ZDCJSONConfig::getChannelConfig(): bad arm or side index");
    size_t index = side*m_numChannelsPerSide + channel;
    return m_channelConfig[index];
  }
};

#endif
