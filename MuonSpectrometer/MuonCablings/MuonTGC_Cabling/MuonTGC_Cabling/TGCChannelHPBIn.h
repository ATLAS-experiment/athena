/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCHANNELHPBIN_HH
#define MUONTGC_CABLING_TGCCHANNELHPBIN_HH

#include "MuonTGC_Cabling/TGCChannelId.h"

namespace MuonTGC_Cabling {

class TGCChannelHPBIn : public TGCChannelId {
   public:
    // Constructor & Destructor
    TGCChannelHPBIn(TGCId::SideType side, TGCId::SignalType signal,
                    TGCId::RegionType region, int sector, int id, int block,
                    int channel);

    virtual ~TGCChannelHPBIn() = default;

    virtual std::unique_ptr<TGCModuleId> getModule() const override;

    virtual bool isValid() const;

   private:
    static const int s_numberOfBlock;
    static const int s_channelInBlock;
    static const int s_slbInBlock;

   public:
    static int getNumberOfBlock();
    static int getChannelInBlock();
    static int getSlbInBlock();

   private:
    TGCChannelHPBIn() {}
};

}  // namespace MuonTGC_Cabling

#endif
