/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCHANNELSLBOUT_HH
#define MUONTGC_CABLING_TGCCHANNELSLBOUT_HH

#include "MuonTGC_Cabling/TGCChannelId.h"

namespace MuonTGC_Cabling {

class TGCChannelSLBOut : public TGCChannelId {
   public:
    // Constructor & Destructor
    TGCChannelSLBOut(TGCId::SideType side, TGCId::ModuleType module,
                     TGCId::RegionType region, int sector, int id, int block,
                     int channel);

    virtual ~TGCChannelSLBOut(void) {}

    virtual TGCModuleId* getModule(void) const;

    virtual bool isValid(void) const;

   private:
    static const int s_numberOfBlockInWD;
    static const int s_numberOfBlockInSD;
    static const int s_numberOfBlockInWT;
    static const int s_numberOfBlockInST;
    static const int s_numberOfLayerInWD;
    static const int s_numberOfLayerInSD;
    static const int s_numberOfLayerInWT;
    static const int s_numberOfLayerInST;
    static const int s_channelInBlockForWD;
    static const int s_channelInBlockForSD;
    static const int s_channelInBlockForWT;
    static const int s_channelInBlockForST;

   public:
    static int getNumberOfBlock(TGCId::ModuleType moduleType);
    static int getNumberOfLayer(TGCId::ModuleType moduleType);
    static int getChannelInBlock(TGCId::ModuleType moduleType);

   private:
    TGCChannelSLBOut(void) {}
};

}  // namespace MuonTGC_Cabling

#endif
