/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_TGCDIGITTIMEOFFSETCONDALG_H_
#define MUONCONDALG_TGCDIGITTIMEOFFSETCONDALG_H_

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCondData/TgcDigitTimeOffsetData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"


namespace Muon{
class TgcDigitTimeOffsetCondAlg : public AthCondAlgorithm
{
 public:
  using AthCondAlgorithm::AthCondAlgorithm;
  virtual ~TgcDigitTimeOffsetCondAlg() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  SG::ReadCondHandleKey<CondAttrListCollection> m_readKey{this, "ReadKey", "/TGC/DIGIT/TOFFSET", "SG key for TGCDIGITTOFFSET"};
  SG::WriteCondHandleKey<TgcDigitTimeOffsetData> m_writeKey{this, "WriteKey", "TGCDigitTimeOffsetData", "SG Key of TgcDigitTimeOffset"};
};
}
#endif   // MUONCONDALG_TGCDIGITTIMEOFFSETCONDALG_H_

