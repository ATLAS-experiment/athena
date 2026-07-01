/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCHANNELPPIN_H_
#define MUONTGC_CABLING_TGCCHANNELPPIN_H_

#include "MuonTGC_Cabling/TGCChannelId.h"

namespace MuonTGC_Cabling {

class TGCChannelPPIn : public TGCChannelId {
 public:
  TGCChannelPPIn(TGCId::SideType side, TGCId::StationType station,
                 TGCId::ModuleType module, TGCId::RegionType region,
                 int sector, int id, int block, int channel);

  virtual ~TGCChannelPPIn() = default;

  virtual std::unique_ptr<TGCModuleId> getModule() const override;

  virtual bool isValid() const override;
};

}  // namespace MuonTGC_Cabling

#endif
