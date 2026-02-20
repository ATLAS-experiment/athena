/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCABLE_HH
#define MUONTGC_CABLING_TGCCABLE_HH

#include "MuonTGC_Cabling/TGCChannelId.h"
#include "MuonTGC_Cabling/TGCModuleId.h"
#include "MuonTGC_Cabling/TGCModuleMap.h"

namespace MuonTGC_Cabling {

class TGCCable {
   public:
    enum CableType {
        NoCableType = -1,
        InASD,
        ASDToPP,
        InPP,
        PPToSLB,
        InSLB,
        SLBToHPB,
        HPBToSL,
        SLBToSSW,
        SSWToROD,
        MaxCableType
    };

    // Constructor & Destructor
    TGCCable(CableType type = NoCableType) : m_type{type} {}
    virtual ~TGCCable() = default;

    CableType getCableType() const { return m_type; }

   private:
    CableType m_type{CableType::NoCableType};
};

}  // namespace MuonTGC_Cabling

#endif
