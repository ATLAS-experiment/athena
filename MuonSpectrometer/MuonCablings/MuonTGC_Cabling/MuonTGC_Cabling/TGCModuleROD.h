/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCMODULEROD_HH
#define MUONTGC_CABLING_TGCMODULEROD_HH

#include "MuonTGC_Cabling/TGCModuleId.h"

namespace MuonTGC_Cabling {

class TGCModuleROD : public TGCModuleId {
   public:
    // Constructor & Destructor
    TGCModuleROD(TGCId::SideType side, int readoutSector);

    virtual ~TGCModuleROD() = default;

    virtual bool isValid() const;
};

}  // namespace MuonTGC_Cabling

#endif
