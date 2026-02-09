/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCABLEINPP_HH
#define MUONTGC_CABLING_TGCCABLEINPP_HH

#include <array>
#include <memory>
#include <string>

#include "MuonTGC_Cabling/TGCCable.h"
#include "MuonTGC_Cabling/TGCId.h"

namespace MuonTGC_Cabling {

class TGCDatabase;

class TGCCableInPP : public TGCCable {
   public:
    TGCCableInPP(const std::string& filename);
    virtual ~TGCCableInPP();

    virtual TGCChannelId* getChannel(const TGCChannelId* channelId,
                                     const bool orChannel = false) const;

   private:
    TGCCableInPP(void) = delete;
    virtual TGCChannelId* getChannelIn(const TGCChannelId* ppout,
                                       const bool orChannel = false) const;
    virtual TGCChannelId* getChannelOut(const TGCChannelId* ppin,
                                        const bool orChannel = false) const;

    std::array<std::array<std::unique_ptr<TGCDatabase>, TGCId::MaxModuleType>,
               TGCId::MaxRegionType>
        m_database;
};

}  // namespace MuonTGC_Cabling

#endif
