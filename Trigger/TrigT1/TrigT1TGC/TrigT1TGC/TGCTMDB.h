/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigT1TGC_TGCTMDB_H
#define TrigT1TGC_TGCTMDB_H

#include <memory>

#include "AthenaBaseComps/AthMessaging.h"
#include "StoreGate/ReadHandle.h"
#include "TileEvent/TileMuonReceiverContainer.h"

#include "TrigT1TGC/TGCTMDBOut.h"
#include "TrigT1TGC/TGCNumbering.h"

namespace LVL1TGC {

class TGCTMDB : public AthMessaging {
 public:
  TGCTMDB();
  virtual ~TGCTMDB() = default;
  TGCTMDB(const TGCTMDB& right) = delete;
  TGCTMDB& operator = (const TGCTMDB& right) = delete;
  int operator == (const TGCTMDB& right) const = delete;
  int operator != (const TGCTMDB& right) const = delete;

  StatusCode retrieve(const SG::ReadHandleKey<TileMuonReceiverContainer>& key, const EventContext& ctx);

  std::shared_ptr<const TGCTMDBOut> getOutput(const TGCSide side, unsigned int moduleID) const;
  std::shared_ptr<const TGCTMDBOut> getOutput(const TGCSide side, int sectorID, unsigned int mod) const;

  int getInnerTileBits(const TGCSide side, int sectorID) const;

  void print() const;

 private:
  static constexpr unsigned int kNTileModule = 64;

  void  setOutput(const TGCSide side, const unsigned int module, 
                  const TGCTMDBOut::TileModuleHit hit56, const TGCTMDBOut::TileModuleHit hit6);

  void  eraseOutput();

 private:
  std::array<std::array<std::shared_ptr<TGCTMDBOut>, kNTileModule>, TGCSide::kNSide> m_buffer;
};


}   // end of namespace

#endif
