/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FOLD_DECORATOR_ALG_H
#define FOLD_DECORATOR_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"

namespace FlavorTagInference {

  class FoldDecoratorAlg : public AthReentrantAlgorithm
  {
  public:
    FoldDecoratorAlg(const std::string& name, ISvcLocator* svcloc);
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& cxt) const override;
    // virtual StatusCode finalize() override;
  protected:
    using IPLV = std::vector<ElementLink<xAOD::IParticleContainer>>;
    using JC = xAOD::JetContainer;
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_mcEventNumberKey {
      this, "eventID", "EventInfo.mcEventNumber", "event number key"
    };
    Gaudi::Property<uint32_t> m_salt {
      this, "salt", 0, "hash salt, bitwise or-ed with the eventID"
    };
    Gaudi::Property<std::string> m_jetCollection {
      this, "jetCollection", "", "name of jet collection"
    };
    SG::ReadDecorHandleKeyArray<JC,IPLV> m_jetAssociations {
      this, "associations", {}, "jet accociation counts to use in hash"
    };
    SG::ReadDecorHandleKeyArray<JC,int> m_jetInts {
      this, "ints", {}, "jet ints to use in hash"
    };
    SG::WriteDecorHandleKey<JC> m_hashKey {
      this, "jetFoldHash", "unofficialJetFoldHash", "name for jet fold hash"
    };
    Gaudi::Property<std::map<std::string, uint32_t>> m_jetVarSeeds {
      this, "jetVariableSaltSeeds", {}, "salt jet variables using the these"
    };
    Gaudi::Property<std::string> m_constituentChars {
      this, "constituentChars", "{}", "char variables to use off constituents"
    };
    Gaudi::Property<std::map<std::string, uint32_t>> m_constituentSeeds {
      this, "constituentSaltSeeds", {}, "salts for constituent varaibles"
    };

    // todo, use something more efficient than a string-keyed map here
    std::unordered_map<std::string, uint32_t> m_hashedKeys;

    using CharReader = SG::AuxElement::ConstAccessor<unsigned char>;
    using SaltedCReader = std::pair<uint32_t, CharReader>;
    std::unordered_map<std::string, std::vector<SaltedCReader>> m_chars;
  };

}
#endif
