/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCMODULEPP_HH
#define MUONTGC_CABLING_TGCMODULEPP_HH

#include "MuonTGC_Cabling/TGCModuleId.h"

namespace MuonTGC_Cabling {

class TGCModulePP : public TGCModuleId {
   public:
    TGCModulePP(TGCId::SideType side, TGCId::ModuleType module,
                TGCId::RegionType region, int sector, int id);
    TGCModulePP(TGCId::SideType side, TGCId::StationType station,
                TGCId::ModuleType module, TGCId::RegionType region,
                int sector, int id);

    virtual ~TGCModulePP() = default;

    virtual bool isValid() const;
};

}  // namespace MuonTGC_Cabling

#endif
