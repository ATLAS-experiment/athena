/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCABLEPPTOSLB_HH
#define MUONTGC_CABLING_TGCCABLEPPTOSLB_HH

#include <array>
#include <memory>
#include <string>

#include "MuonTGC_Cabling/TGCCable.h"

namespace MuonTGC_Cabling {

class TGCDatabase;

class TGCCablePPToSLB : public TGCCable {
   public:
    TGCCablePPToSLB(const std::string& filename);
    virtual ~TGCCablePPToSLB();

    std::unique_ptr<TGCChannelId> getChannel(const TGCChannelId& channelId,
                                             bool orChannel = false) const;
    TGCModuleMap getModule(const TGCModuleId& moduleId) const;

   private:
    TGCCablePPToSLB() = delete;
    std::unique_ptr<TGCChannelId> getChannelIn(const TGCChannelId& slbin,
                                               bool orChannel = false) const;
    std::unique_ptr<TGCChannelId> getChannelOut(const TGCChannelId& ppout,
                                                bool orChannel = false) const;
    TGCModuleMap getModuleIn(const TGCModuleId& slb) const;
    TGCModuleMap getModuleOut(const TGCModuleId& pp) const;

    std::array<std::array<std::unique_ptr<TGCDatabase>, TGCId::MaxModuleType>,
               TGCId::MaxRegionType>
        m_database;
};

}  // namespace MuonTGC_Cabling

#endif
