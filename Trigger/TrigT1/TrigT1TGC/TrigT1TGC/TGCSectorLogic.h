/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGGSectorLogic_hh
#define TGGSectorLogic_hh



#include "TrigT1TGC/TGCRPhiCoincidenceMatrix.h"
#include "TrigT1TGC/TGCSSCController.h"
#include "TrigT1TGC/TGCInnerTrackletSlotHolder.h"
#include "TrigT1TGC/TGCTrackSelector.h"
#include "StoreGate/ReadCondHandle.h"
#include "MuonCondSvc/TGCTriggerData.h"
#include <memory>

namespace LVL1TGC {
class TGCTileMuCoincidenceLUT;
class TGCGoodMF;
class TGCNSW;
class TGCBIS78;
class TGCBIS78CoincidenceMap;
class TGCEIFICoincidenceMap;

}

namespace LVL1TGCTrigger {

class TGCDatabaseManager;
class TGCHighPtBoard;
class TGCHighPtChipOut;
class TGCTMDB;
class TGCNSWCoincidenceMap;
class TGCRPhiCoincidenceOut;
class TGCTrackSelectorOut;
class TGCArguments;


class TGCSectorLogic
{
 public:
  TGCSectorLogic(TGCArguments*, const TGCDatabaseManager* db,
                 TGCRegionType regionIn, int id);
  virtual ~TGCSectorLogic();

  TGCSectorLogic(const TGCSectorLogic& right);

  void getTrackSelectorOutput(std::shared_ptr<TGCTrackSelectorOut> &trackSelectorOut) const;

  int  getTileMuonWord() const;
  int  getInnerStationWord() const;

  void clockIn(const SG::ReadCondHandleKey<TGCTriggerData>& readCondKey,
               int bidIn, bool process=true);

  int getId() const;
  int getModuleID() const;
  int getOctantID() const;
  int getSideID() const;
  TGCRegionType getRegion() const { return m_region;};

  void setWireHighPtBoard(int port, TGCHighPtBoard* highPtBoard);
  void setStripHighPtBoard(TGCHighPtBoard* highPtBoard);
  int  getNumberOfWireHighPtBoard() const;
  void deleteHPBOut();

  TGCSSCController* getSSCController(){return &m_SSCController;};

  void setTMDB(std::shared_ptr<const LVL1TGC::TGCTMDB> tmdb);
  void setNSW(std::shared_ptr<const LVL1TGC::TGCNSW> nsw);
  void setBIS78(std::shared_ptr<const LVL1TGC::TGCBIS78> bis78);
  void showResult();
 
  int getNumberOfSubSectorCluster() const; 
  int getNumberOfSubSector() const; 

  void dec2bin(int dec, char* binstr, int length);

  int getBid() const {return m_bid;};

  void setInnerTrackletSlots(const TGCInnerTrackletSlot* innerTrackletSlots[]);
  
  TGCArguments* tgcArgs() { return m_tgcArgs; }
  const TGCArguments* tgcArgs() const { return m_tgcArgs; }

protected: 
  enum {MaxNumberOfWireHighPtBoard =2};
  void collectInput();

  void doInnerCoincidence(const SG::ReadCondHandleKey<TGCTriggerData>& readCondKey,
                          int SSCId,  TGCRPhiCoincidenceOut* coincidenceOut);

  void doInnerCoincidence(int SSCId,  TGCRPhiCoincidenceOut* coincidenceOut);

  void doTGCNSWCoincidence(TGCRPhiCoincidenceOut* coincidenceOut); 
  bool doTGCBIS78Coincidence(TGCRPhiCoincidenceOut* coincidenceOut); 
  bool doTILECoincidence(TGCRPhiCoincidenceOut* coincidenceOut); 
  bool doTGCEICoincidence(TGCRPhiCoincidenceOut* coincidenceOut);
  bool doTGCFICoincidence(TGCRPhiCoincidenceOut* coincidenceOut);

private:
  TGCSectorLogic& operator=(const TGCSectorLogic& right);

private:
  bool hitTileMu(const uint8_t& mask, const uint8_t& hit6, const uint8_t& hit56) const;

  int m_bid;

  int m_id;
  LVL1TGC::TGCSide m_sideId;
  int m_sectorId, m_moduleId;
  int m_octantId;
  TGCRegionType m_region;
  int  m_NumberOfWireHighPtBoard;

  bool m_useEIFI{false};
  bool m_useTileMu{false};
  bool m_useGoodMF{false};
  bool m_nswSide{false};

  TGCSSCController m_SSCController;

  // Run-3 and Run-2 Coincidence LUTs and Window maps
  TGCRPhiCoincidenceMatrix m_matrix;
  const LVL1TGC::TGCEIFICoincidenceMap*  m_mapEIFI;
  std::shared_ptr<const LVL1TGC::TGCTileMuCoincidenceLUT> m_tileMuLUT;
  std::shared_ptr<const LVL1TGC::TGCTMDB>      m_pTMDB;
  std::shared_ptr<const LVL1TGC::TGCNSW>     m_nsw;
  std::shared_ptr<const TGCNSWCoincidenceMap> m_mapNSW;
  std::shared_ptr<const LVL1TGC::TGCBIS78>    m_bis78;
  std::shared_ptr<const LVL1TGC::TGCBIS78CoincidenceMap> m_mapBIS78;
  std::shared_ptr<const LVL1TGC::TGCGoodMF> m_mapGoodMF;

  TGCTrackSelector m_trackSelector;
  std::shared_ptr<TGCTrackSelectorOut> m_trackSelectorOut;
 
  int m_wordTileMuon;
  int m_wordInnerStation;

  TGCHighPtBoard* m_wireHighPtBoard[MaxNumberOfWireHighPtBoard];
  TGCHighPtChipOut* m_wireHighPtChipOut[MaxNumberOfWireHighPtBoard];
  TGCHighPtBoard* m_stripHighPtBoard;
  TGCHighPtChipOut* m_stripHighPtChipOut;

  // for inner trigger
  const TGCInnerTrackletSlot* m_innerTrackletSlots[TGCInnerTrackletSlotHolder::NUMBER_OF_SLOTS_PER_TRIGGER_SECTOR];
  TGCArguments* m_tgcArgs;
};

inline
 int TGCSectorLogic::getNumberOfSubSectorCluster() const
{
  return (m_region == TGCRegionType::FORWARD) ? 8 : LVL1TGC::kNMaxSSC;
}

inline
 int TGCSectorLogic::getNumberOfSubSector() const
{
  return (m_region == TGCRegionType::FORWARD) ? 8 * LVL1TGC::kNRoiInSSC : LVL1TGC::kNumberOfEndcapRoI;
}

inline
 int  TGCSectorLogic::getNumberOfWireHighPtBoard() const
{
  return m_NumberOfWireHighPtBoard; 
}


inline 
 int TGCSectorLogic::getId() const
{
  return m_id;
}

inline
 int TGCSectorLogic::getModuleID() const
{
  return m_moduleId;
}


inline 
 int TGCSectorLogic::getOctantID() const
{
  return m_octantId;
}

inline 
 int TGCSectorLogic::getSideID() const
{
  return m_sideId;
}

inline   
 int  TGCSectorLogic::getTileMuonWord() const
{
  return m_wordTileMuon;
}

inline   
 int  TGCSectorLogic::getInnerStationWord() const
{
  return m_wordInnerStation;
}

} //end of namespace bracket

#endif
