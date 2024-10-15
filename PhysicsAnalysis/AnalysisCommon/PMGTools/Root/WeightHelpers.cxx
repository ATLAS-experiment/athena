/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak


#include <unordered_map>
#include <algorithm>
#include <string>
#include <vector>
#include "PMGTools/WeightHelpers.h"


namespace PMGTools
{

std::string weightNameCleanup(const std::string &name)
{
  // empty weight is nominal
  if (name.empty())
  {
    return {};
  }

  // Trim trailing whitespace
  size_t start = name.find_first_not_of(" \t\n\r");
  size_t end = name.find_last_not_of(" \t\n\r");
  std::string out = name.substr(start,end-start+1);  
  std::string outLowercase = out;
  std::transform(outLowercase.begin(), outLowercase.end(), outLowercase.begin(),
    [](unsigned char c){ return std::tolower(c); });

  // more cases of nominal weights
  if (outLowercase == "default" // This is a primary weight in newer samples
    || outLowercase == "nominal" // Powheg calls it "nominal"
    || outLowercase == "weight" // Sherpa names the nominal weight just "Weight"
    || outLowercase == "0")
  {
    return {};
  }

  static const std::vector<std::pair<std::string, std::string>> substitutions =
  {
    {" set = ", "_"}, // Powheg
    {" = ", "_"}, // Powheg
    {"+", "plus"},
    {"-", "minus"},
    {"=", ""},
    {",", ""},
    {".", ""},
    {":", ""},
    {" ", "_"},
    {"#", "num"},
    {"\n", "_"},
    {"/", "over"}, // MadGraph
  };

  for (const auto &[before, after] : substitutions)
  {
    size_t location = out.find(before);
    while (location != std::string::npos)
    {
      out.replace(location, before.length(), after);
      location = out.find(before);
    }
  }

  return out;
}

std::string weightNameWithPrefix(const std::string &name)
{
  std::string out = weightNameCleanup(name);
  if (out.empty())
  {
    return out;
  }

  return generatorWeightsPrefix + out;
}

} // namespace MC
