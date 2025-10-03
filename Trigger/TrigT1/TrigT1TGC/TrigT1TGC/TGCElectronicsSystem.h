/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCElectronicsSystem_hh
#define TGCElectronicsSystem_hh

#include "TrigT1TGC/TGCArguments.h"
#include "TrigT1TGC/TGCReadoutIndex.h"

#include <memory>

namespace LVL1TGC {
class TGCTMDB;
class TGCNSW;
class TGCBIS78;
}

namespace LVL1TGCTrigger {

const int NumberOfOctant = 8;
const int NumberOfModule = 15; 
const int NumberOfSignalTypes = 2;  // 1=WIRE, 2=STRIP
const int NumberOfLayers = 9;       // 0,1,2: Tpl, 3,4: Dbl, 5,6: Inner 8,9
const int NumberOfRegions = 2;      // 1=Forward, 2=ENdcap 

class TGCEvent;
class TGCDatabaseManager;
class TGCSector;

class TGCElectronicsSystem {
 public:
  TGCElectronicsSystem(TGCArguments*, TGCDatabaseManager* database);
  virtual ~TGCElectronicsSystem();

  void distributeSignal(TGCEvent* event);
  int getNumberOfSector() const { return NumberOfOctant*NumberOfModule; }
  int getNumberOfOctant() const { return NumberOfOctant; }
  int getNumberOfModule() const { return NumberOfModule; }

  TGCRegionType getRegionType(int mod) const;
  TGCForwardBackwardType getForwardBackward(int side, int oct, int mod) const;
  int getSectorId(int side, int oct, int mod) const;
  TGCSector* getSector(const TGCReadoutIndex& index) const;
  TGCSector* getSector(int side, int oct, int mod) const { 
    if ( (side<0) || (oct<0) || (mod<0) ) return 0;
    return m_sector[side][oct][mod];
  };
  std::shared_ptr<LVL1TGC::TGCTMDB> getTMDB() const {return m_tmdb;}
  std::shared_ptr<LVL1TGC::TGCNSW> getNSW() const {return m_nsw;}
  std::shared_ptr<LVL1TGC::TGCBIS78> getBIS78() const {return m_bis78;}

  TGCArguments* tgcArgs() { return m_tgcArgs;}
  const TGCArguments* tgcArgs() const { return m_tgcArgs;}
  
 private:
  // hide default/copy constructor and assignment operator
  TGCElectronicsSystem();
  TGCElectronicsSystem(const TGCElectronicsSystem& right) = delete;
  TGCElectronicsSystem& operator=(const TGCElectronicsSystem& right) = delete;

 private:
  TGCDatabaseManager* m_DB;
  TGCSector* m_sector[LVL1TGC::kNSide][NumberOfOctant][NumberOfModule];
  // Other Inner systems
  std::shared_ptr<LVL1TGC::TGCTMDB>  m_tmdb{nullptr};
  std::shared_ptr<LVL1TGC::TGCNSW>   m_nsw{nullptr};
  std::shared_ptr<LVL1TGC::TGCBIS78> m_bis78{nullptr};

  TGCArguments* m_tgcArgs;
};


}  // end of namespace

#endif // TGCElectronicsSystem_hh
