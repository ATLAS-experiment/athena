/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCABLESSWTOROD_HH
#define MUONTGC_CABLING_TGCCABLESSWTOROD_HH

#include <memory>
#include <string>

#include "MuonTGC_Cabling/TGCCable.h"

namespace MuonTGC_Cabling {

class TGCDatabase;

class TGCCableSSWToROD : public TGCCable {
   public:
    TGCCableSSWToROD(const std::string& filename);
    TGCCableSSWToROD(const TGCCableSSWToROD&);
    TGCCableSSWToROD& operator=(const TGCCableSSWToROD&);
    virtual ~TGCCableSSWToROD();

    TGCModuleMap getModule(const TGCModuleId& moduleId) const;

   private:
    TGCCableSSWToROD() = delete;
    TGCModuleMap getModuleIn(const TGCModuleId& rod) const;
    TGCModuleMap getModuleOut(const TGCModuleId& ssw) const;
    std::unique_ptr<TGCDatabase> m_database{nullptr};
};

}  // namespace MuonTGC_Cabling

#endif
