#include "ZdcAnalysis/ZDCJSONConfig.h"

#include <iostream>

std::pair<bool, std::string> ZDCJSONConfig::ParseConfig(const JSON& config, const JSONParamList& configParamList)
{
  bool result = true;
  std::string resultString = "success";

  for (auto [key, value] : config.items()) {

    if (value.is_null()) {
      result = false;
      resultString = "Found null value for parameter " + key;
      break;
    }

    //
    // First we look up the parameter in the provided parameter list
    //
    auto iter = configParamList.find(key);
    if (iter == configParamList.end()) {
      result = false;
      resultString = "Found unknown parameter: " + key +": JSON=" + config.dump();
      break;
    }

    auto paramType = std::get<0>(iter->second);
    auto paramSize = std::get<1>(iter->second);
    auto paramSingleOnly = !std::get<2>(iter->second); // the boolean indicates whether the parameter is per-channel settable

    // For single value only, we can do a type check here
    //
    if (paramSingleOnly) {
      if (!checkType(value, paramType, paramSize)) {
	result = false;
	resultString = "Incorrect JSON type for parameter " + key;
	break;
      }

      //
      // Set the value for all channels
      //
      setAllParameter(key, value);

      // Add it to the list of "global" parameters
      //
      m_globalConfig[key] = value; 
    }
    else {
      //
      // If the value is per-channel settable, then either the value is an object with
      //   key, value pairs that provide the per-side or per-channel settings or it's
      //   a "single" value (possible an array) that appplies to all channels 
      //
      if (value.type() == JSON::value_t::object) {
	auto [ppcResult, ppcResultString] = ParsePerChannelParams(key, value, paramType, paramSize); 
	if (!ppcResult) {
	  result = ppcResult;
	  resultString = ppcResultString;
	  break;
	}
      }
      else {
	if (!checkType(value, paramType, paramSize)) {
	  result = false;
	  resultString = "Incorrect JSON type for parameter " + key;
	  break;
	}

	//
	// Set the value for all channels
	//
	setAllParameter(key, value);
      }
    }    
  }

  if (result) m_haveConfig = true;
  return {result, resultString};
}

template<typename T> std::pair<bool, std::string> ZDCJSONConfig::ParsePerChannelParams(const std::string& paramKey, const T& paramValue, JSON::value_t paramType, size_t paramSize)
{
  bool result = true;
  std::string resultString = "success";

  for (auto [key, value] : paramValue.items()) {
    try {
      //
      // Compare the key against the side labels
      //
      //bool validKey = false;
      for (unsigned int side = 0; side < m_nSides; side++) {
	size_t srchIdx = key.find(m_sideLabels[side]);
	if (srchIdx == 0) {
	  //validKey = true;

	  // Check to see whether the key length matches the
	  //   side label length -- if it does, then we either
	  //   have an array specifying the values for all channels
	  //   or we have a single value that applies to all channels
	  //
	  if (key.size() == m_sideLabels[side].size()) {
	    if (value.size() == 1) {
	      //
	      // We have per-side value that we set for all channels on that side,
	      //  unless it already has had a specific value provided (handled by SetSideParameter)
	      //
	      if (!checkType(value, paramType)) {
		result = false;
		resultString = "Incorrect JSON type for parameter " + key;
		break;
	      }

	      setPerSideParameter(side, paramKey, value);
	      break;
	    }
	    else if (paramType == JSON::value_t::array) {

	      size_t elementSize = value[0].size();
	      size_t arrayLength = value.size();
	      
	      if (arrayLength == m_numChannelsPerSide && elementSize == paramSize) {
		for (unsigned int chan = 0; chan < m_numChannelsPerSide; chan++) {
		  setChannelParameter(side, chan, paramKey, value[chan]);
		}
	      }
	      else if (arrayLength == paramSize) {
		//
		// We have a single array that is set for all channels
		//
		setPerSideParameter(side, paramKey, value);
	      }
	      else {
		result = false;
		resultString = "Invalid value format for array parameter " + paramKey + " on side " + m_sideLabels[side];
		break;
	      }
	    }
	    else if (paramType == JSON::value_t::object) {
	      //
	      // For now do nothing -- need to work out how to handle objects
	      //
	    }
	    else {
	      //
	      // If we get here we must have an array of values 
	      //
	      if (value.size() != m_numChannelsPerSide) {
		result = false;
		resultString = "Invalid array length for parameter " + paramKey + " on side " + m_sideLabels[side];
		break;
	      }
	      else {
		//
		// The the values for each channel
		//
		for (unsigned int chan = 0; chan < m_numChannelsPerSide; chan++) {
		  if (!checkType(value[chan], paramType)) {
		    result = false;
		    resultString = "Incorrect JSON type for parameter " + paramKey + "paramType = " +
		      std::to_string((unsigned int) paramType) + ", value type = " + std::to_string((unsigned int) value[chan].type());
		    break;
		  }

		  setChannelParameter(side, chan, paramKey, value[chan]);
		}
	      }
	    }
	  }
	  else {
	    // The key must be longer than the side label which should mean a specific channel
	    //
	    std::string keyRemainder = key.substr(m_sideLabels[side].size(), std::string::npos);
	    try {
	      int chanIndex = std::stoi(keyRemainder);
	      if (chanIndex < 0 || size_t(chanIndex) > m_numChannelsPerSide) {
		result = false;
		resultString = "Invalid channel specifier in values for parameter " + key;
		break;
	      }

	      if (!checkType(value, paramType, paramSize)) {
		result = false;
		resultString = "Incorrect JSON type for parameter " + key;
		break;
	      }

	      setChannelParameter(side, chanIndex, paramKey, value);
	    }
	    catch(...) {
	      result = false;
	      resultString = "Invalid channel specifier for parameter " + key;
	      break;
	    }
	  }
	}
	
      }
    }
    catch (...) {
      result = false;
      resultString = "Exception caught while parsing key " + key;
    }
  }

  return {result, resultString};
}


