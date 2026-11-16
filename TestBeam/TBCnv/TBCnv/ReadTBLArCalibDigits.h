/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef READLARCALIBDIGITS_H
#define READLARCALIBDIGITS_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "TBEvent/TBLArCalibDigitContainer.h"

#include <fstream>

class LArOnlineID;

class ReadTBLArCalibDigits : public AthAlgorithm
{
 public:
  ReadTBLArCalibDigits(const std::string & name, ISvcLocator * pSvcLocator);

  ~ReadTBLArCalibDigits();

  //standart algorithm methods
  StatusCode initialize();
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();

 private:
  int m_count{0};
  const LArOnlineID* m_onlineHelper{nullptr};
  std::ofstream m_outfile;
  Gaudi::Property<std::string> m_containerKey{this, "ContainerKey", "", "SG key of the TBLArCalibDigitContainer"};
  Gaudi::Property<std::string> m_dumpFile{this, "DumpFile", "", "Name of the text dump file (empty: no dump)"};

};

#endif
