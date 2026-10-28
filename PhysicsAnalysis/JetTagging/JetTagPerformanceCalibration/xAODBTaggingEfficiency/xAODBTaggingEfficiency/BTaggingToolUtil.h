/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BTAGGINGTOOLUTIL_H
#define BTAGGINGTOOLUTIL_H

#include <nlohmann/json.hpp>
#include <string>

class BTaggingToolUtil {

  public:
  // Convert MeV to GeV 
  static double MeVToGeV(const double x){ return x/1000.;}

  static float getExtendedFloat(const nlohmann::json &config);
  static std::string getExtendedString(const nlohmann::json &config);

  // enumaration class to assign integers to different binning variables 
  // to speed up the tools performance
  enum class varType {
    unknown,
    pT,
    mass,
    eta,
    abseta
  };

  // Returns true if two numbers are approximatively equal 
  static bool approxEqual(const float a, const float b, const float eps);

  static varType getVariableType(const std::string& varName); 
  static std::string getVariableName(const varType vType);
};

#endif // BTAGGINGTOOLUTIL_H
