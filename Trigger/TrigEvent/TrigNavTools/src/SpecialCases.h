/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <regex>

namespace SpecialCases {
  // config hacks patterns
  const std::regex gammaXeChain{"HLT_g.*_xe.*"};
  const std::regex egammaDiEtcut{".*etcut.*etcut.*"};
  const std::regex egammaEtcut{".*etcut.*"};
  const std::regex egammaCombinedWithEtcut{"HLT_(e|g).*_(e|g).*etcut.*"};

}