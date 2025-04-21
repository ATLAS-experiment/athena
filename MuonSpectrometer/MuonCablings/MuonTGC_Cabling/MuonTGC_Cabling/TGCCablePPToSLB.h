/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCABLEPPTOSLB_HH
#define MUONTGC_CABLING_TGCCABLEPPTOSLB_HH
 
#include "MuonTGC_Cabling/TGCCable.h"

#include <string>
#include <memory>
#include <array>

namespace MuonTGC_Cabling {

class TGCDatabase;
  
class TGCCablePPToSLB : public TGCCable
{
public:
  TGCCablePPToSLB(const std::string& filename);
  virtual ~TGCCablePPToSLB() = default;
  
  virtual TGCChannelId* getChannel(const TGCChannelId* channelId,
				   bool orChannel=false) const;
  virtual TGCModuleMap* getModule(const TGCModuleId* moduleId) const;
  
private:
  TGCCablePPToSLB(void) {}
  virtual TGCChannelId* getChannelIn(const TGCChannelId* slbin, 
				     bool orChannel=false) const;
  virtual TGCChannelId* getChannelOut(const TGCChannelId* ppout,
				      bool orChannel=false) const;
  virtual TGCModuleMap* getModuleIn(const TGCModuleId* slb) const;
  virtual TGCModuleMap* getModuleOut(const TGCModuleId* pp) const;

  std::array<std::array<std::unique_ptr<TGCDatabase>, TGCId::MaxModuleType>, TGCId::MaxRegionType> m_database{{{nullptr}}};
};
  
}  // end of namespace
 
#endif
