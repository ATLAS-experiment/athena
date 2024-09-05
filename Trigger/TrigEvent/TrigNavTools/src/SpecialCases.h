/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <regex>

namespace SpecialCases {
  // explicitely excluded chains
  const std::vector<std::string> excludedChains{
      "HLT_mu20_msonly_mu6noL1_msonly_nscan05",
      "HLT_mu6_dRl1_mu20_msonly_iloosems_mu6noL1_dRl1_msonly",
      "HLT_g45_loose_6j45_0eta240",
      "HLT_mu20_2mu2noL1_JpsimumuFS",
      "HLT_mu20_2mu4_JpsimumuL2",
      "HLT_mu20_2mu4noL1"
  };

  // config hacks patterns
  const std::regex gammaXeChain{"HLT_g.*_xe.*"};
  const std::regex egammaDiEtcut{".*etcut.*etcut.*"};
  const std::regex egammaEtcut{".*etcut.*"};
  const std::regex egammaCombinedWithEtcut{"HLT_(e|g).*_(e|g).*etcut.*"};
  const std::regex isTopo{".*(Jpsi|Zee).*"};
  const std::regex specialEchain{"HLT_e26_lhmedium_nod0_mu8noL1"};
  const std::regex tauXeChain{"HLT.*tau.*xe.*"};
  const std::regex bjetMuChain{"HLT_mu.*_j.*_split_.*"};
}