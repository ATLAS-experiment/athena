/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCDatabaseManager_hh
#define TGCDatabaseManager_hh

#include "TrigT1TGC/TGCNumbering.h"
#include "TrigT1TGC/TGCElectronicsSystem.h"
#include "TrigT1TGC/TGCConnectionPPToSB.h"

#include "MuonCondSvc/TGCTriggerData.h"
#include "TGCTriggerCondSvc/TGCTriggerLUTs.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "AthenaBaseComps/AthMessaging.h"

#include <vector>
#include <map>
#include <string>

namespace LVL1TGC {
class BigWheelCoincidenceLUT;
class TGCTileMuCoincidenceLUT;
class TGCGoodMF;
class TGCBIS78CoincidenceMap;
class TGCEIFICoincidenceMap;
}

namespace LVL1TGCTrigger {

class TGCArguments;
class TGCConnectionASDToPP;
class TGCConnectionInPP;
class TGCPatchPanel;
class TGCConnectionPPToSL;
class TGCNSWCoincidenceMap;

class TGCDatabaseManager : public AthMessaging
{
  typedef std::vector<int> PatchPanelIDs; 
  typedef std::vector<const TGCPatchPanel*> PatchPanelPointers; 

 public:
  TGCDatabaseManager();
  TGCDatabaseManager(TGCArguments*,
                     const SG::ReadCondHandleKey<TGCTriggerData>& readCondKey,
		     const SG::ReadCondHandleKey<TGCTriggerLUTs>& readLUTsCondKey);
  virtual ~TGCDatabaseManager();

  TGCDatabaseManager(const TGCDatabaseManager& right);
  TGCDatabaseManager& operator = (const TGCDatabaseManager& right);

  const TGCConnectionPPToSL* getConnectionPPToSL(TGCRegionType type) const;
  const TGCConnectionASDToPP* getConnectionASDToPP(TGCRegionType region, int type, TGCForwardBackwardType forwardBackward) const;
  std::shared_ptr<LVL1TGC::BigWheelCoincidenceLUT> getBigWheelCoincidenceLUT() const;
  const LVL1TGC::TGCEIFICoincidenceMap* getEIFICoincidenceMap(int sideId) const;
  std::shared_ptr<LVL1TGC::TGCTileMuCoincidenceLUT> getTileMuCoincidenceLUT() const;
  std::shared_ptr<TGCNSWCoincidenceMap> getNSWCoincidenceMap(int sideId, int octantId, int moduleId) const;
  std::shared_ptr<LVL1TGC::TGCBIS78CoincidenceMap> getBIS78CoincidenceMap() const;
  std::shared_ptr<LVL1TGC::TGCGoodMF> getGoodMFMap() const;

  TGCConnectionInPP* getConnectionInPP(TGCPatchPanel* patchPanel) const;
  void addConnectionInPP(const TGCPatchPanel* patchPanel, const TGCConnectionInPP* connectionInPP);

  void deleteConnectionPPToSL();

  static std::string getFilename(int type);
  static const std::vector<std::string > splitCW(const std::string& input, char delimiter); 

  TGCArguments* tgcArgs() { return m_tgcArgs; }
  const TGCArguments* tgcArgs() const { return m_tgcArgs; }

 private:
  enum {NumberOfModuleInBW=9};

  std::shared_ptr<LVL1TGC::BigWheelCoincidenceLUT> m_bigWheelLUT;
  LVL1TGC::TGCEIFICoincidenceMap* m_mapEIFI[LVL1TGC::kNSide];
  std::shared_ptr<LVL1TGC::TGCTileMuCoincidenceLUT> m_tileMuLUT;
  std::array<std::array<std::array<std::shared_ptr<TGCNSWCoincidenceMap>, NumberOfModuleInBW>, NumberOfOctant>, LVL1TGC::kNSide> m_mapNSW;
  std::shared_ptr<LVL1TGC::TGCBIS78CoincidenceMap> m_mapBIS78; // temporal
  TGCConnectionPPToSL* m_PPToSL[NumberOfRegionType];
  TGCConnectionASDToPP* m_ASDToPP[NumberOfRegionType][TGCSector::NumberOfPatchPanelType][TotalNumForwardBackwardType];
  std::shared_ptr<LVL1TGC::TGCGoodMF> m_mapGoodMF;

  std::map<PatchPanelIDs, std::pair<const TGCConnectionInPP, PatchPanelPointers> > m_patchPanelToConnectionInPP;
  
  TGCArguments* m_tgcArgs;
};

inline std::shared_ptr<LVL1TGC::BigWheelCoincidenceLUT> TGCDatabaseManager::getBigWheelCoincidenceLUT() const
{
  return m_bigWheelLUT;
}

inline const LVL1TGC::TGCEIFICoincidenceMap* TGCDatabaseManager::getEIFICoincidenceMap(int sideId) const
{
  return m_mapEIFI[sideId];
}

inline std::shared_ptr<LVL1TGC::TGCTileMuCoincidenceLUT> TGCDatabaseManager::getTileMuCoincidenceLUT() const
{
  return m_tileMuLUT;
}

inline std::shared_ptr<TGCNSWCoincidenceMap> TGCDatabaseManager::getNSWCoincidenceMap(int sideId, int octantId, int moduleId) const
{
  if (moduleId >= NumberOfModuleInBW) [[unlikely]] return nullptr;
  return m_mapNSW[sideId][octantId][moduleId];
}

inline std::shared_ptr<LVL1TGC::TGCBIS78CoincidenceMap> TGCDatabaseManager::getBIS78CoincidenceMap() const
{
  return m_mapBIS78;
}

inline std::shared_ptr<LVL1TGC::TGCGoodMF> TGCDatabaseManager::getGoodMFMap() const {
  return m_mapGoodMF;
}

inline const TGCConnectionPPToSL* TGCDatabaseManager::getConnectionPPToSL(TGCRegionType type) const {
  int region = (type == TGCRegionType::FORWARD) ? 0 : 1;
  return m_PPToSL[region];
}

inline const TGCConnectionASDToPP* TGCDatabaseManager::getConnectionASDToPP(TGCRegionType region, int type, TGCForwardBackwardType forwardBackward) const {
  int reg = (region == TGCRegionType::FORWARD) ? 0 : 1;
  return m_ASDToPP[reg][type][forwardBackward];
}

}   // end of LVL1TGCTrigger namespace

#endif // TGCDatabaseManager_hh
