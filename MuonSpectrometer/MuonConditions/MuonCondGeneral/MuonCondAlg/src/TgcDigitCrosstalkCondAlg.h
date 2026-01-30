/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_TGCDIGITCROSSTALKCONDALG_H_
#define MUONCONDALG_TGCDIGITCROSSTALKCONDALG_H_

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCondData/TgcDigitCrosstalkData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

namespace Muon{
class TgcDigitCrosstalkCondAlg : public AthCondAlgorithm
{
 public:
  using AthCondAlgorithm::AthCondAlgorithm;
  virtual ~TgcDigitCrosstalkCondAlg() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  SG::ReadCondHandleKey<CondAttrListCollection> m_readKey{this, "ReadKey", "/TGC/DIGIT/XTALK", "SG key for TGCDIGITXTALK"};
  SG::WriteCondHandleKey<TgcDigitCrosstalkData> m_writeKey{this, "WriteKey", "TGCDigitCrosstalkData", "SG Key of TgcDigitCrosstalk"};
};
}
#endif   // MUONCONDALG_TGCDIGITTIMEOFFSETCONDALG_H_

