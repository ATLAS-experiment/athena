/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGC_NSW_H
#define TGC_NSW_H

#include <memory>

#include "TrigT1TGC/TGCNumbering.h"

#include "AthenaBaseComps/AthMessaging.h"
#include "StoreGate/ReadHandle.h"

#include "MuonRDO/NSW_TrigRawDataContainer.h"

namespace LVL1TGC {

class NSWTrigOut;

class TGCNSW : public AthMessaging {
 public:
  TGCNSW();
  virtual ~TGCNSW() = default;
  TGCNSW(const TGCNSW& right) = delete;
  const TGCNSW& operator = (const TGCNSW& right) = delete;
  bool operator == (const TGCNSW& right) const = delete;
  bool operator != (const TGCNSW& right) const = delete;

  StatusCode retrieve(const SG::ReadHandleKey<Muon::NSW_TrigRawDataContainer>& key, const EventContext& ctx);
  std::shared_ptr<const NSWTrigOut> getOutput(LVL1TGCTrigger::TGCRegionType region ,int side, int TGC_TriggerSector) const;

 private:
  enum { NumberOfNSWTriggerProcesser = 16 };

  void setOutput(int side, int NSWTriggerProcesser,uint8_t NSWeta_8bit,uint8_t NSWphi_6bit,uint8_t NSWDtheta_5bit, bool lowRes, bool phiRes, bool NSWmonitor); //eta:0.005 phi:10mrad Dtheta:1mrad 
  void eraseOutput();

  void print() const;

  std::shared_ptr<NSWTrigOut> m_buffer[LVL1TGC::TGCSide::kNSide][NumberOfNSWTriggerProcesser];  // buffer[Side][NSW TP]
};


} //end of namespace bracket

#endif
