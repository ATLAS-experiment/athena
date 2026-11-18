/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DUMPLARDIGITS_H
#define DUMPLARDIGITS_H
#include "AthenaBaseComps/AthAlgorithm.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "StoreGate/ReadCondHandleKey.h"

//#include "LArDetDescr/LArDetDescrManager.h"
#include <fstream>

class DumpLArDigits : public AthAlgorithm
{
 public:
  using AthAlgorithm::AthAlgorithm;

  ~DumpLArDigits();

  //standart algorithm methods
  StatusCode initialize();
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();
 private:
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
  int m_count{0};
  int m_chan{0};
  //LArDigitContainer* m_larDigitCont;
  const LArOnlineID*       m_onlineHelper{nullptr};
  const LArEM_ID* m_emId{nullptr};
  std::ofstream m_outfile;
  Gaudi::Property<std::string> m_key{this, "LArDigitContainerName", ""};
  Gaudi::Property<std::string> m_FileName{this, "OutputFileName", "LArDigits.txt"};
 private:
  class mySort {
  public:
    bool operator()(const LArDigit* a, const LArDigit* b);
  };
};

#endif
