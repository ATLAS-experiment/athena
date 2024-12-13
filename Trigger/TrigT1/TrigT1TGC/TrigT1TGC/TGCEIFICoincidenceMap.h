/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigT1TGC_TGCEIFICoincidenceMap_hh
#define TrigT1TGC_TGCEIFICoincidenceMap_hh

#include "TrigT1TGC/TGCInnerTrackletSlot.h"
#include "TrigT1TGC/TGCNumbering.h"
#include "TrigT1TGC/TGCArguments.h"

#include "StoreGate/ReadCondHandle.h"
#include "MuonCondSvc/TGCTriggerData.h"

#include "AthenaBaseComps/AthMessaging.h"

#include <string>
#include <bitset>

namespace LVL1TGC {
 
class TGCEIFICoincidenceMap : public AthMessaging
{
 public:
  TGCEIFICoincidenceMap(LVL1TGCTrigger::TGCArguments*, const SG::ReadCondHandleKey<TGCTriggerData>& readCondKey);
  TGCEIFICoincidenceMap(LVL1TGCTrigger::TGCArguments*, const SG::ReadCondHandleKey<TGCTriggerData>& readCondKey,
                        const std::string& version, int sideId=0);

  TGCEIFICoincidenceMap(const TGCEIFICoincidenceMap& right);
  TGCEIFICoincidenceMap& operator = (const TGCEIFICoincidenceMap& right);

  const LVL1TGCTrigger::TGCInnerTrackletSlot* getInnerTrackletMask(const unsigned int input,
			                                       	   const unsigned int ssc,
					                           const unsigned int sec) const;
  int getFlagPT(const unsigned int pt,
		const unsigned int ssc,
		const unsigned int sec) const;

  int getFlagROI(const unsigned int roi,
		 const unsigned int ssc,
		 const unsigned int sec) const;

  int getTriggerBit(const int slot,
                    const int ssc,
                    const int sec,
                    const int reg,
                    const int read,
                    const int bit) const;

  const std::string&  getVersion() const { return m_verName; }
  int                 getSideId() const { return m_side; }
  bool                isFullCW() const { return m_fullCW; }
  void                setFullCW( bool val) { m_fullCW = val; }

  bool readMap();  
  void                        dumpMap() const;

  const LVL1TGCTrigger::TGCArguments* tgcArgs() const { return m_tgcArgs; }
  LVL1TGCTrigger::TGCArguments* tgcArgs() { return m_tgcArgs; }

 protected:
  static constexpr unsigned int N_INNER_SECTORS = 4;

 private:
  // The flagPT is a flag whether each pT is applied the coincidence or not.
  // In each bit (of kNThresholdsR2 bits), 1 applies the coincidence, and 0 is not.
  // The LSB is the lowest pT threshold, and MSB is the highest pT threshold.
  std::bitset<kNThresholdsR2> m_flagPT[kNMaxSSC][kNEndcapTrigSector];

  int m_flagROI[kNRoiInSSC][kNMaxSSC][kNEndcapTrigSector]; 
  // 1 use; 0: not use; -1: not used for Trigger

  LVL1TGCTrigger::TGCInnerTrackletSlot m_map[N_INNER_SECTORS][kNMaxSSC][kNEndcapTrigSector];

  std::string m_verName;
  int m_side; 
  bool m_fullCW;

  LVL1TGCTrigger::TGCArguments* m_tgcArgs{};

  const SG::ReadCondHandleKey<TGCTriggerData>& m_readCondKey;
};


inline
const LVL1TGCTrigger::TGCInnerTrackletSlot* TGCEIFICoincidenceMap::getInnerTrackletMask(const unsigned int input, 
                                                                                        const unsigned int ssc, 
                                                                                        const unsigned int sec) const
{
  if (input >= N_INNER_SECTORS) return 0;
  if (ssc >= kNMaxSSC) return 0;
  if (sec >= kNEndcapTrigSector) return 0;

  return  &(m_map[input][ssc][sec]);    
}

} //end of namespace bracket

#endif


