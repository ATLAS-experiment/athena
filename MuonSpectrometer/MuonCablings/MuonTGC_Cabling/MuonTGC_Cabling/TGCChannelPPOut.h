/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCHANNELPPOUT_H_
#define MUONTGC_CABLING_TGCCHANNELPPOUT_H_

#include "MuonTGC_Cabling/TGCChannelId.h"

namespace MuonTGC_Cabling {

class TGCChannelPPOut : public TGCChannelId {
 public:
  TGCChannelPPOut(TGCId::SideType side, TGCId::StationType station,
                  TGCId::ModuleType module, TGCId::RegionType region,
                  int sector, int id, int block, int channel);

  virtual ~TGCChannelPPOut() = default;

  virtual std::unique_ptr<TGCModuleId> getModule() const override;

  virtual bool isValid() const override;
};

}  // namespace MuonTGC_Cabling

#endif
