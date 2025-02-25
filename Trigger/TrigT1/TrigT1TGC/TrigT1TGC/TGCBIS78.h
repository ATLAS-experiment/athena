/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGC_BIS78_H
#define TGC_BIS78_H

#include <memory>

#include "TrigT1TGC/TGCNumbering.h"

#include "AthenaBaseComps/AthMessaging.h"
#include "StoreGate/ReadHandle.h"
#include "MuonRDO/RpcBis78_TrigRawDataContainer.h"

namespace LVL1TGC {

class BIS78TrigOut;

class TGCBIS78 : public AthMessaging {
 public:
  TGCBIS78();
  virtual ~TGCBIS78() = default;
  TGCBIS78(const TGCBIS78& right) = delete;
  const TGCBIS78& operator = (const TGCBIS78& right) = delete;
  bool operator == (const TGCBIS78& right) const = delete;
  bool operator != (const TGCBIS78& right) const = delete;

  StatusCode retrieve(const SG::ReadHandleKey<Muon::RpcBis78_TrigRawDataContainer>& key,
                      const EventContext& ctx);

  std::shared_ptr<const BIS78TrigOut> getOutput(int TGC_TriggerSector) const;

  void print() const;

 private:
  void  setOutput(unsigned int BIS78PadBoard,uint8_t BIS78eta_6bit,uint8_t BIS78phi_6bit,uint8_t BIS78Deta_3bit,uint8_t BIS78Dphi_3bit,uint8_t BIS78flag3over3eta_1bit,uint8_t BIS78flag3over3phi_1bit);
  void  eraseOutput();

 private:
  static constexpr int kNPadBoards = 8;

 protected:
  std::shared_ptr<BIS78TrigOut> m_buffer[kNPadBoards];//buffer[BIS78 Trigger Processor]
};


}  // end of namespace

#endif
