/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCABLINGCHECKER
#define LARCABLINGCHECKER
#include "AthenaBaseComps/AthAlgorithm.h"
#include "LArRawEvent/LArDigitContainer.h"
//#include "CaloIdentifier/LArEM_ID.h"
#include "LArIdentifier/LArOnlineID.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArRecConditions/LArCalibLineMapping.h"

#include <fstream>
#include <vector>

class LArCablingChecker : public AthAlgorithm
{
 public:
  using AthAlgorithm::AthAlgorithm;

  ~LArCablingChecker();

  //standard algorithm methods
  StatusCode initialize();
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();

 private:
  int m_count{0};

  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
  SG::ReadCondHandleKey<LArBadChannelCont> m_BCKey{this, "BadChanKey", "LArBadChannel", "SG bad channels key"};
  SG::ReadCondHandleKey<LArCalibLineMapping>  m_CLKey{this, "CalibLineKey", "LArCalibLineMap", "SG calib line key"};

  const LArOnlineID* m_onlineHelper{nullptr};
  const LArEM_ID* m_emId{nullptr};

  std::ofstream m_outfile;
  Gaudi::Property<std::string> m_key{this, "DigitKey", ""};
  Gaudi::Property<std::string> m_outFileName{this, "FileName", "CablingSummary.txt"};
  //std::vector<unsigned> m_vPattern;

  std::vector<bool> *m_errorcellsThisEvent{nullptr};
  int m_errorCounterThisEvent{0};
  std::vector<bool> *m_errorcellsPreviousEvent{nullptr};
  int m_errorCounterPreviousEvent{0};
  int m_numberOfEventsWithThisErrorState{0};
  LArOnlineID::size_type m_channelHashMax{0};

  // Properties
  Gaudi::Property<bool> m_PrintAllCellsAndEvents{this, "PrintAllCellsAndEvents", false};
  Gaudi::Property<bool> m_PrintDisconnectedCells{this, "PrintDisconnectedCells", true};
  Gaudi::Property<int> m_ADCThreshold{this, "ADCThreshold", 1250};
  Gaudi::Property<int> m_DACHighGainThreshold{this, "DACHighGainThreshold", 100};
  Gaudi::Property<int> m_DACMediumGainThreshold{this, "DACMediumGainThreshold", 1000};
  Gaudi::Property<int> m_DACLowGainThreshold{this, "DACLowGainThreshold", 10000};
  Gaudi::Property<bool> m_printEventSummary{this, "PrintEventSummary", false};
  Gaudi::Property<bool> m_useBadChannelTool{this, "UseBadChannelTool", true};
};

#endif
