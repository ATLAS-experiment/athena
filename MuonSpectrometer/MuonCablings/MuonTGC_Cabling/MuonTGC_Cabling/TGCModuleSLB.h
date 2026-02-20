/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCMODULESLB_HH
#define MUONTGC_CABLING_TGCMODULESLB_HH

#include "MuonTGC_Cabling/TGCModuleId.h"

namespace MuonTGC_Cabling {

class TGCModuleSLB : public TGCModuleId {
   public:
    // Constructor & Destructor
    TGCModuleSLB(TGCId::SideType side, TGCId::ModuleType module,
                 TGCId::RegionType region, int sector, int id, int sbLoc = -1,
                 int slbAddr = -1);

    virtual ~TGCModuleSLB() = default;

    virtual bool isValid() const;

    // special method for SLB
    int getSBLoc() const { return m_sbLoc; }
    int getSlbAddr() const { return m_slbAddr; }

   private:
    int m_sbLoc{0};
    int m_slbAddr{0};
};

}  // namespace MuonTGC_Cabling

#endif
