/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigChainNameParserChecker.h"
#include "TrigCompositeUtils/ChainNameParser.h"

StatusCode TrigChainNameParserChecker::initialize() {
  ATH_CHECK( m_HLTMenuKey.initialize() );
  return StatusCode::SUCCESS;
}

StatusCode TrigChainNameParserChecker::execute(const EventContext& ctx) const {

  // Validate the ChainNameParser against the JSON menu.
  // Expecting to run in a trigger production job hence the menu comes from the detector store.

  // Only needs to run once per job, on the first event.
  static const bool error = [&](){

    SG::ReadHandle<TrigConf::HLTMenu> hltMenuHandle(m_HLTMenuKey, ctx);
    const TrigConf::HLTMenu& hltMenu = *hltMenuHandle;
    bool isErr = false;
    for (const auto& chain : hltMenu) {
      const std::vector<size_t> legMultiplicitesA = chain.legMultiplicities();
      const std::vector<int>    legMultiplicitesB = ChainNameParser::multiplicities(chain.name());
      if (!legMultiplicitesB.size()) {
        ATH_MSG_ERROR("Zero parsed legs for chain:" << chain.name());
        isErr = true;
      } else if (legMultiplicitesA.size() != legMultiplicitesB.size()) {
        ATH_MSG_ERROR("Inconsistent N Legs, Menu:" << legMultiplicitesA.size() << " Parser:" << legMultiplicitesB.size() << " chain:" << chain.name());
        isErr = true;
      } else {
        for (size_t i = 0; i < legMultiplicitesA.size(); ++i) {
          if (legMultiplicitesA.at(i) != (size_t)legMultiplicitesB.at(i)) {
            ATH_MSG_ERROR("Inconsistency in Leg " << i << ", Menu multi:" << legMultiplicitesA.at(i) << " Parser multi:" << legMultiplicitesB.at(i) << " chain:" << chain.name());
            isErr = true;
          }
        }
      }
    }

    if (isErr) {
      ATH_MSG_ERROR("One or more chains (above) in the menu were parsed incorrectly by the ChainNameParser with respect to the menu.");
      ATH_MSG_ERROR("Please update the parser in TrigCompositeUtils to bring it into agreement with the menu.");
      ATH_MSG_ERROR("You may need to update the lists of identifiers in allSignatures() or allSignaturePostfixQualifiers().");
    } else {
      ATH_MSG_INFO("TrigChainNameParserChecker did not find any issues over " << hltMenu.size() << " chains.");
    }
    return isErr;

  }();

  return (error ? StatusCode::FAILURE : StatusCode::SUCCESS);

}
