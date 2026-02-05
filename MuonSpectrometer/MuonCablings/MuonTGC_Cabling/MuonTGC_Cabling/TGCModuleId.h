/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCMODULEID_H
#define MUONTGC_CABLING_TGCMODULEID_H

#include "MuonTGC_Cabling/TGCId.h"

namespace MuonTGC_Cabling {

class TGCModuleId : public TGCId {
   public:
    enum ModuleIdType {
        NoModuleIdType = -1,
        PP,
        SLB,
        HPB,
        SL,
        SSW,
        ROD,
        SROD,
        MaxModuleIdType
    };

    // Constructor & Destructor
    TGCModuleId(ModuleIdType type = NoModuleIdType) : TGCId(IdType::Module) {
        this->m_type = type;
    }
    virtual ~TGCModuleId(void) {}

    enum { NumberOfSReadoutSector = 3 };

    ModuleIdType getModuleIdType(void) const { return m_type; }

    virtual bool operator==(const TGCModuleId& moduleId) const;

    virtual bool isValid(void) const { return true; }

    int getReadoutSector() const { return m_sectorRO; }
    void setReadoutSector(int sector) { m_sectorRO = sector; }
    virtual void setSector(int v_sector);

   private:
    ModuleIdType m_type;
    int m_sectorRO{-1};
};

}  // namespace MuonTGC_Cabling

#endif
