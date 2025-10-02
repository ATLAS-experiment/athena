/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCSector_h
#define TGCSector_h


#include "TrigT1TGC/TGCTimingManager.h"
#include "TrigT1TGC/TGCTMDB.h"
#include "TrigT1TGC/TGCNSW.h"
#include "TrigT1TGC/TGCBIS78.h"
#include <memory>

namespace LVL1TGCTrigger {
class TGCArguments;
class TGCDatabaseManager;
class TGCConnectionPPToSL;
class TGCConnectionHPBToSL;
class TGCConnectionPPToSB;
class TGCConnectionASDToPP;
class TGCConnectionSBToHPB;
class TGCASDOut;
class TGCPatchPanel;
class TGCSlaveBoard;
class TGCHighPtBoard;
class TGCSectorLogic;


class TGCSector
{
 public:
  enum TGCHighPtBoardType { WHPB=0, SHPB, NumberOfHighPtBoardType };
  enum TGCPatchPanelType { NOPP=-1,
                           WTPP=0, WDPP, STPP, SDPP, WIPP, SIPP,
                           NumberOfPatchPanelType };

 public:
  TGCSector(TGCArguments*,
	    int idIn, 
	    TGCRegionType type, 
	    TGCForwardBackwardType forwardBackward, 
	    const TGCDatabaseManager* db,
	    std::shared_ptr<const LVL1TGC::TGCTMDB>  tmdb,
	    std::shared_ptr<const LVL1TGC::TGCNSW>   nsw,
            std::shared_ptr<const LVL1TGC::TGCBIS78> bis78);

  TGCSector();

 private:
  // copy constructor and assignement operator are hidden.
  TGCSector(const TGCSector& right) = delete;
  TGCSector& operator = (const TGCSector& right) = delete;

 public:
  virtual ~TGCSector();

  bool hasHit() const;
  void clearNumberOfHit();
  int distributeSignal(const TGCASDOut* asdOut);

  TGCPatchPanel* getPP(int type, int index) const;
  TGCSlaveBoard* getSB(int type, int index) const;
  TGCHighPtBoard* getHPB(int type, int index) const;
  TGCSectorLogic* getSL() { return m_SL; }

  unsigned int getNumberOfPP(int type) const;
  unsigned int getNumberOfSB(int type) const;
  unsigned int getNumberOfHPB(int type) const;

  TGCRegionType getRegionType() const;
  int getId() const;
  void dumpModule();

  LVL1TGC::TGCSide getSideId() const { return m_sideId; }
  int getOctantId() const { return m_octantId; }
  int getModuleId() const { return m_moduleId; }

  TGCArguments* tgcArgs() { return m_tgcArgs; }
  const TGCArguments* tgcArgs() const { return m_tgcArgs; }

private:
  std::shared_ptr<const LVL1TGC::TGCTMDB> getTMDB() const { return m_TMDB; }
  std::shared_ptr<const LVL1TGC::TGCNSW> getNSW() const{ return m_NSW; }
  std::shared_ptr<const LVL1TGC::TGCBIS78> getBIS78() const{ return m_BIS78; }
 
  int getPatchPanelType(TGCSignalType signal, int layer) const;


  void setModule(const TGCConnectionPPToSL* connection);
  void connectPPToSB(const TGCConnectionPPToSB* connection);
  void connectSBToHPB(const TGCConnectionSBToHPB* connection);
  void connectHPBToSL(const TGCConnectionHPBToSL* connection);
  void connectAdjacentPP();
  void connectAdjacentHPB();

  friend void TGCTimingManager::startPatchPanel(TGCSector* sector, TGCDatabaseManager* db);
  friend void TGCTimingManager::startSlaveBoard(TGCSector* sector);
  friend void TGCTimingManager::startHighPtBoard(TGCSector* sector);
  friend void TGCTimingManager::startSectorLogic(TGCSector* sector);

 private:
  int m_id{0};
  TGCRegionType m_regionType;
  LVL1TGC::TGCSide m_sideId{LVL1TGC::TGCSide::ASIDE};
  int m_numberOfHit;
  int m_octantId;
  int m_moduleId;

  TGCForwardBackwardType m_forwardBackward;
  const TGCConnectionASDToPP* m_ASDToPP[NumberOfPatchPanelType];

  std::vector<TGCPatchPanel*>  m_PP[NumberOfPatchPanelType];
  std::vector<TGCSlaveBoard*>  m_SB[NumberOfSlaveBoardType];
  std::vector<TGCHighPtBoard*> m_HPB[NumberOfHighPtBoardType];

  TGCSectorLogic* m_SL;
  std::shared_ptr<const LVL1TGC::TGCTMDB> m_TMDB;
  std::shared_ptr<const LVL1TGC::TGCNSW>  m_NSW;
  std::shared_ptr<const LVL1TGC::TGCBIS78>  m_BIS78;

  TGCArguments* m_tgcArgs;
  const TGCDatabaseManager* m_dbMgr;
};

inline 
 bool TGCSector::hasHit() const
{
  return (m_numberOfHit>0);
}
inline 
 void TGCSector::clearNumberOfHit()
{ m_numberOfHit = 0;}
inline
 TGCPatchPanel* TGCSector::getPP(int type, int index) const
{ 
  if ((type<0) || (index<0)) return 0;
  return m_PP[type].at(index);
}
inline
 TGCSlaveBoard* TGCSector::getSB(int type, int index) const
{   
  if ((type<0) || (index<0)) return 0;
  return m_SB[type].at(index);
}
inline
 TGCHighPtBoard* TGCSector::getHPB(int type, int index) const
{   
  if ((type<0) || (index<0)) return 0;
  return m_HPB[type].at(index);
}
inline
unsigned int TGCSector::getNumberOfPP(int type) const
{ 
  if (type<0) return -1;
  return m_PP[type].size();
}
inline
unsigned int TGCSector::getNumberOfSB(int type) const
{ if (type<0) return -1;
  return m_SB[type].size();
}
inline
unsigned int TGCSector::getNumberOfHPB(int type) const
{ if (type<0) return -1;
  return m_HPB[type].size();
}
inline
TGCRegionType TGCSector::getRegionType() const
{ return m_regionType;}

inline
int TGCSector::getId() const
{ return m_id;}

inline TGCSector::TGCPatchPanelType operator++(TGCSector::TGCPatchPanelType &rs, int)
{
    return rs = static_cast<TGCSector::TGCPatchPanelType>(rs + 1);
}

} //end of namespace bracket

#endif
