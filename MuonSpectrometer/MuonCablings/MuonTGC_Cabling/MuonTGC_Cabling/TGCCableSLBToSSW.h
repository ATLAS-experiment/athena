/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCABLESLBTOSSW_HH
#define MUONTGC_CABLING_TGCCABLESLBTOSSW_HH
 
#include "MuonTGC_Cabling/TGCCable.h"

#include <string>
#include <memory>
#include <array>

namespace MuonTGC_Cabling {

class TGCDatabase;
  
class TGCCableSLBToSSW : public TGCCable {
 public:
  enum {
    SL            = TGCId::MaxModuleType, 
    MaxModuleType = TGCId::MaxModuleType + 1
  };

  TGCCableSLBToSSW(const std::string& filename);
  virtual ~TGCCableSLBToSSW() = default;
  
  virtual TGCModuleMap* getModule(const TGCModuleId* moduleId) const;

 private:
  TGCCableSLBToSSW() {}
  virtual TGCModuleMap* getModuleIn(const TGCModuleId* ssw) const;
  virtual TGCModuleMap* getModuleOut(const TGCModuleId* slb) const;
  std::array<std::array<std::unique_ptr<TGCDatabase>, MaxModuleType>, TGCId::MaxRegionType> m_database{nullptr};
};
  
}  // end of namespace
 
#endif
