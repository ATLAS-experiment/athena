/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCMODULESROD_HH
#define MUONTGC_CABLING_TGCMODULESROD_HH

#include "MuonTGC_Cabling/TGCModuleId.h"

namespace MuonTGC_Cabling {

class TGCModuleSROD : public TGCModuleId {
   public:
    // Constructor & Destructor
    TGCModuleSROD(TGCId::SideType side, int readoutSector);

    virtual ~TGCModuleSROD() = default;

    virtual bool isValid() const;
};

}  // namespace MuonTGC_Cabling

#endif
