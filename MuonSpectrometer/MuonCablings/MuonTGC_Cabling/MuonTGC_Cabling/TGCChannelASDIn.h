/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCHANNELASDIN_HH
#define MUONTGC_CABLING_TGCCHANNELASDIN_HH

#include "MuonTGC_Cabling/TGCChannelId.h"

namespace MuonTGC_Cabling {

class TGCChannelASDIn : public TGCChannelId {
   public:
    // Constructor & Destructor
    TGCChannelASDIn(TGCId::SideType side, TGCId::SignalType signal,
                    TGCId::RegionType region, int sector, int layer,
                    int chamber, int channel);

    virtual ~TGCChannelASDIn(void) {}

    virtual void setSector(int sector);

    virtual int getSector(void) const;

    virtual bool isValid(void) const;

   private:
    TGCChannelASDIn(void) {}
};

}  // namespace MuonTGC_Cabling

#endif
