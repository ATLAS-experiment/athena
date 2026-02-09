/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCABLEHPBTOSL_HH
#define MUONTGC_CABLING_TGCCABLEHPBTOSL_HH

#include <array>
#include <memory>
#include <string>

#include "MuonTGC_Cabling/TGCCable.h"
#include "MuonTGC_Cabling/TGCId.h"

namespace MuonTGC_Cabling {

class TGCDatabase;

class TGCCableHPBToSL : public TGCCable {
   public:
    TGCCableHPBToSL(const std::string& filename);
    virtual ~TGCCableHPBToSL();

    virtual TGCModuleMap* getModule(const TGCModuleId* moduleId) const;

   private:
    TGCCableHPBToSL() = delete;
    virtual TGCModuleMap* getModuleIn(const TGCModuleId* sl) const;
    virtual TGCModuleMap* getModuleOut(const TGCModuleId* hpt) const;
    std::array<std::array<std::unique_ptr<TGCDatabase>, TGCId::MaxSignalType>,
               TGCId::MaxRegionType>
        m_database;
};

}  // namespace MuonTGC_Cabling

#endif
