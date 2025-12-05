//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARBADCHANNELCONDALG_H
#define LARBADCHANNELCONDALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArCabling/LArOnOffIdMapping.h"

class LArBadChannelCondAlg: public AthCondAlgorithm {
 public:
  //Delegate to base-class ctor
  using AthCondAlgorithm::AthCondAlgorithm;

  virtual ~LArBadChannelCondAlg()=default;

  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;
  virtual StatusCode finalize() override final {return StatusCode::SUCCESS;}

 private:
  SG::ReadCondHandleKey<CondAttrListCollection> m_BCInputKey{this,"ReadKey","/LAR/BadChannelsOfl/BadChannels",
      "Key of input CDO (AttrListCollection)"}; 
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","Key of cabling CDO"};   
  SG::WriteCondHandleKey<LArBadChannelCont> m_BCOutputKey{this,"WriteKey","LArBadChannel","Key of output CDO"};
  Gaudi::Property<std::string> m_inputFileName{this,"InputFileName","",
      "Optional file containing (supplemental) bad channels"};
  Gaudi::Property<bool> m_isSC{this,"isSC",false,"Set to true to interpret the identifiers in the InputFile as SuperCell IDs"};
  Gaudi::Property<bool> m_reloadEveryEvent{this,"ReloadEveryEvent",false,"Set to true to force DB-cache to be cleared - necessary if running online"};

};



#endif
