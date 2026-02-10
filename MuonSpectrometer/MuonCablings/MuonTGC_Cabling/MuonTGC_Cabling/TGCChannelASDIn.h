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

    virtual ~TGCChannelASDIn() = default;

    virtual void setSector(int sector);

    virtual int getSector() const;

    virtual bool isValid() const;
};

}  // namespace MuonTGC_Cabling

#endif
