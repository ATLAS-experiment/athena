/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCABLESLBTOHPB_HH
#define MUONTGC_CABLING_TGCCABLESLBTOHPB_HH
 
#include "MuonTGC_Cabling/TGCCable.h"

#include <string>


namespace MuonTGC_Cabling
{

class TGCDatabase;
  
class TGCCableSLBToHPB : public TGCCable
{
public:

  // Constructor & Destructor
  TGCCableSLBToHPB(const std::string& filename);

  virtual ~TGCCableSLBToHPB(void);
  
  virtual TGCChannelId* getChannel(const TGCChannelId* channelId,
				   bool orChannel=false) const;
  virtual TGCModuleMap* getModule(const TGCModuleId* moduleId) const;

  TGCChannelId* getChannelInforHPB(const TGCChannelId* hpbin,
				   TGCId::ModuleType moduleType,
				   bool orChannel=false) const;

private:
  TGCCableSLBToHPB(void) {}
  virtual TGCChannelId* getChannelIn(const TGCChannelId* hpbin, 
				     bool orChannel=false) const;
  virtual TGCChannelId* getChannelOut(const TGCChannelId* slbout,
				      bool orChannel=false) const;
  virtual TGCModuleMap* getModuleIn(const TGCModuleId* hpb) const;
  virtual TGCModuleMap* getModuleInforHPB(const TGCModuleId* hpb, TGCId::ModuleType moduleType) const;
  virtual TGCModuleMap* getModuleOut(const TGCModuleId* slb) const;
  TGCDatabase* m_database[TGCId::MaxRegionType][TGCId::MaxModuleType]{};
};
  
} // end of namespace
 
#endif
