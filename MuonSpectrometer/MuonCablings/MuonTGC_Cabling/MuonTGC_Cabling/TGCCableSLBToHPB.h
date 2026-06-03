/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCABLESLBTOHPB_HH
#define MUONTGC_CABLING_TGCCABLESLBTOHPB_HH

#include <array>
#include <memory>
#include <string>

#include "MuonTGC_Cabling/TGCCable.h"

namespace MuonTGC_Cabling {

class TGCDatabase;

class TGCCableSLBToHPB : public TGCCable {
   public:
    TGCCableSLBToHPB(const std::string& filename);
    virtual ~TGCCableSLBToHPB();

    std::unique_ptr<TGCChannelId> getChannel(const TGCChannelId& channelId,
                                             bool orChannel = false) const;
    TGCModuleMap getModule(const TGCModuleId& moduleId) const;

    std::unique_ptr<TGCChannelId> getChannelInforHPB(
        const TGCChannelId& hpbin, TGCId::ModuleType moduleType,
        bool orChannel = false) const;

   private:
    TGCCableSLBToHPB() = delete;
    std::unique_ptr<TGCChannelId> getChannelIn(const TGCChannelId& hpbin,
                                               bool orChannel = false) const;
    std::unique_ptr<TGCChannelId> getChannelOut(const TGCChannelId& slbout,
                                                bool orChannel = false) const;
    TGCModuleMap getModuleIn(const TGCModuleId& hpb) const;
    TGCModuleMap getModuleInforHPB(const TGCModuleId& hpb,
                                   TGCId::ModuleType moduleType) const;
    TGCModuleMap getModuleOut(const TGCModuleId& slb) const;
    std::array<std::array<std::unique_ptr<TGCDatabase>, +TGCId::ModuleType::MaxModuleType>,
               +TGCId::RegionType::MaxRegionType>
        m_database;
};

}  // namespace MuonTGC_Cabling

#endif
