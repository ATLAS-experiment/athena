/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SCT_CalibUtilities.cxx
 * implementation file for the SCTCalibUtilities
 *
 * @author Shaun Roe shaun.roe@cern.ch
 **/

#include "SCT_CalibUtilities.h"

//gaudi
#include "InDetIdentifier/SCT_ID.h"
#include "Identifier/Identifier.h"
#include <algorithm> //std::unique

namespace SCT_CalibAlgs {
  std::string
  normalizeList(std::string s) {
    auto binaryPredicate=[](char s0, char s1){
    return (s0==' ' and s0==s1);
  };
  auto normalised= std::unique(s.begin(), s.end(), binaryPredicate);
  s.erase(normalised,s.end());
  size_t first=0;
  size_t last=s.size()-1;
  if (s[first] == ' ') first =1;
  if (s[last] == ' ') last-=1;
  return s.substr(first,last);
}


std::string
formatPosition(const Identifier& waferId, const SCT_ID* helper,
    std::string_view delimiter, const bool includeSide) {
  std::string result{std::to_string(helper->barrel_ec(waferId))};
  result.append(delimiter);
  result += std::to_string(helper->layer_disk(waferId));
  //
  result.append(delimiter);
  result += std::to_string(helper->phi_module(waferId));
  //
  result.append(delimiter);
  result += std::to_string(helper->eta_module(waferId));
  //
  if (includeSide) {
    result.append(delimiter);
    result += std::to_string(helper->side(waferId));
  }

  return result;
}

std::string
chipList2LinkList(const std::string& chipList) {
   if (chipList == "0-5  6-11") {
      return "0 1";
   } else if (chipList == "0-5") {
      return "0";
   } else if (chipList == "6-11") {
      return "1";
   } else {
      return "";
   }
}

unsigned int
bec2Index(const int bec) {
   return static_cast<unsigned int>((bec/2) + 1);
}

std::string
xmlCloseChannel() {
   return "</channel>";
}
} //end of namespace
