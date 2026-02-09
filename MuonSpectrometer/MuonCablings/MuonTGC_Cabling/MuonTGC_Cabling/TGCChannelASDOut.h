/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCHANNELASDOUT_H
#define MUONTGC_CABLING_TGCCHANNELASDOUT_H

#include "MuonTGC_Cabling/TGCChannelId.h"

namespace MuonTGC_Cabling {

class TGCChannelASDOut : public TGCChannelId {
   public:
    // Constructor & Destructor
    TGCChannelASDOut(TGCId::SideType side, TGCId::SignalType signal,
                     TGCId::RegionType region, int sector, int layer,
                     int chamber, int channel);

    TGCChannelASDOut(TGCId::SideType side, TGCId::SignalType signal, int octant,
                     int moduleSector, int layer, int chamber, int channel);

    virtual ~TGCChannelASDOut(void) = default;

    virtual bool isValid(void) const;

   private:
    TGCChannelASDOut() = delete;
};

}  // namespace MuonTGC_Cabling

#endif
