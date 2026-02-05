/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCABLEINASD_HH
#define MUONTGC_CABLING_TGCCABLEINASD_HH

#include <array>
#include <memory>
#include <string>

#include "MuonTGC_Cabling/TGCCable.h"

namespace MuonTGC_Cabling {

class TGCDatabase;

class TGCCableInASD : public TGCCable {
   public:
    TGCCableInASD(const std::string& filename);
    virtual ~TGCCableInASD();

    virtual TGCChannelId* getChannel(const TGCChannelId* channelId,
                                     bool orChannel = false) const;

   private:
    TGCCableInASD() = delete;
    virtual TGCChannelId* getChannelIn(const TGCChannelId* asdout,
                                       bool orChannel = false) const;
    virtual TGCChannelId* getChannelOut(const TGCChannelId* asdin,
                                        bool orChannel = false) const;
    std::array<std::array<std::unique_ptr<TGCDatabase>, TGCId::MaxModuleType>,
               TGCId::MaxRegionType>
        m_database;
};

}  // namespace MuonTGC_Cabling

#endif
