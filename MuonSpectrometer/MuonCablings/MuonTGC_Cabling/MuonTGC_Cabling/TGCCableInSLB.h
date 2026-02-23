/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCABLEINSLB_HH
#define MUONTGC_CABLING_TGCCABLEINSLB_HH

#include "MuonTGC_Cabling/TGCCable.h"

namespace MuonTGC_Cabling {

class TGCCableInSLB : public TGCCable {
   public:
    TGCCableInSLB() : TGCCable(TGCCable::InSLB) {}

    virtual ~TGCCableInSLB() = default;

    std::unique_ptr<TGCChannelId> getChannel(const TGCChannelId& channelId,
                                             bool orChannel = false) const;

   private:
    std::unique_ptr<TGCChannelId> getChannelIn(const TGCChannelId& slbout,
                                               bool orChannel = false) const;
    std::unique_ptr<TGCChannelId> getChannelOut(const TGCChannelId& slbin,
                                                bool orChannel = false) const;
};

}  // namespace MuonTGC_Cabling

#endif
