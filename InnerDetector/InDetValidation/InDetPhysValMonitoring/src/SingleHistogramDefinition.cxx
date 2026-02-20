/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
//
//  SingleHistogramDefinition.cpp
//  HDef
//
//  Created by sroe on 13/07/2015.
//

#include "InDetPhysValMonitoring/SingleHistogramDefinition.h"
#include <utility>
#include <limits>
#include <format>
SingleHistogramDefinition::SingleHistogramDefinition() :
  name{},
  histoType{},
  title{},
  nBinsX{},
  nBinsY{},
  nBinsZ{},
  xAxis(std::make_pair(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN())),
  yAxis(std::make_pair(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN())),
  zAxis(std::make_pair(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN())),
  xTitle{},
  yTitle{},
  zTitle{},
  allTitles{},
  folder{},
  m_empty(true) {
}

SingleHistogramDefinition::SingleHistogramDefinition(Titles_t thename, Titles_t thehistoType,
						     Titles_t thetitle, NBins_t nbinsX, 
						     Var_t xLo, Var_t xHi,
						     Titles_t xName, Titles_t yName,
						     Titles_t thefolder) :
  name(thename), histoType(thehistoType), title(thetitle), 
  nBinsX(nbinsX), nBinsY(0), nBinsZ(0),
  xAxis(xLo, xHi),
  yAxis(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()),
  zAxis(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()),
  xTitle(xName), yTitle(yName), zTitle(""), 
  folder(thefolder), m_empty(false) {
  
  allTitles = titleDigest();
}

SingleHistogramDefinition::SingleHistogramDefinition(Titles_t thename, Titles_t thehistoType,
						     Titles_t thetitle, NBins_t nbinsX, NBins_t nbinsY, 
						     Var_t xLo, Var_t xHi, Var_t yLo, Var_t yHi, 
						     Titles_t xName, Titles_t yName, Titles_t thefolder) :
  name(thename), histoType(thehistoType), title(thetitle), 
  nBinsX(nbinsX), nBinsY(nbinsY), nBinsZ(0),
  xAxis(xLo, xHi),
  yAxis(yLo, yHi),
  zAxis(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()),
  xTitle(xName), yTitle(yName), zTitle(""),
  folder(thefolder), m_empty(false) {
  // should do sanity checks here
  
  allTitles = titleDigest();
}

SingleHistogramDefinition::SingleHistogramDefinition(Titles_t thename, Titles_t thehistoType,
                                                     Titles_t thetitle, NBins_t nbinsX, NBins_t nbinsY, NBins_t nbinsZ, 
						     Var_t xLo, Var_t xHi, Var_t yLo, Var_t yHi, Var_t zLo, Var_t zHi, 
						     Titles_t xName, Titles_t yName, Titles_t zName, Titles_t thefolder) :
  name(thename), histoType(thehistoType), title(thetitle), 
  nBinsX(nbinsX), nBinsY(nbinsY), nBinsZ(nbinsZ),
  xAxis(xLo, xHi),
  yAxis(yLo, yHi),
  zAxis(zLo, zHi),
  xTitle(xName), yTitle(yName), zTitle(zName), 
  folder(thefolder), m_empty(false) {
  // should do sanity checks here
  
  allTitles = titleDigest();
}

bool
SingleHistogramDefinition::empty() const {
  return m_empty;
}

std::string
SingleHistogramDefinition::str() const {
  return std::format("{} {} {} {} {} {} {:f} {:f} {:f} {:f} {:f} {:f} {} {} {}",
                     name, histoType, title, nBinsX, nBinsY, nBinsZ,
                     xAxis.first, xAxis.second,
                     yAxis.first, yAxis.second,
                     zAxis.first, zAxis.second,
                     xTitle, yTitle, zTitle);
}

bool
SingleHistogramDefinition::validType() const {
  const std::string signature((histoType.substr(0, 3)));

  return((signature == "TH1")or(signature == "TH2")or(signature == "TH3")or(signature == "TPr") or(signature == "TEf"));
}

bool
SingleHistogramDefinition::isValid() const {
  if (name.empty() or histoType.empty()) {
    return false;
  }
  bool sane(true);
  // note: if yaxis is left undefined, the limits should be NaN, but (NaN != NaN) is always true
  const bool sensibleLimits = (xAxis.first != xAxis.second)and(yAxis.first != yAxis.second)and(zAxis.first != zAxis.second);
  const bool sensibleXBins = (nBinsX != 0);
  const bool sensibleYBins = (nBinsY != 0);
  const bool sensibleZBins = (nBinsZ != 0);
  const bool sensibleTitles = not (title.empty() or xTitle.empty());
  sane = (sensibleLimits and sensibleXBins and sensibleTitles);
  if (histoType.substr(0, 3) == "TH2") {
    sane = (sane and sensibleYBins);
  }
  if (histoType.substr(0, 3) == "TH3") {
    sane = (sane and sensibleYBins and sensibleZBins);
  }
  return sane;
}

std::string
SingleHistogramDefinition::stringIndex() const {
  return stringIndex(name, folder);
}

std::string
SingleHistogramDefinition::titleDigest() const {
  const std::string s(";");

  return title + s + xTitle + s + yTitle + s + zTitle;
}

std::string
SingleHistogramDefinition::stringIndex(const std::string& thisname, const std::string& thisfolder) {
  if (thisfolder.empty()) {
    return thisname;
  }
  const std::string delimiter("/");
  std::string result(thisfolder);
  if ((thisfolder.substr(0, 1)) == delimiter) {
    result = thisfolder.substr(1, thisfolder.size() - 2);// reduce "/myfolder" to "myfolder"
  }
  size_t lastChar(result.size() - 1);
  if ((result.substr(lastChar, 1) != delimiter)) {
    result = result + delimiter; // add a slash: "myfolder" => "myfolder/"
  }
  return result + thisname;
}
