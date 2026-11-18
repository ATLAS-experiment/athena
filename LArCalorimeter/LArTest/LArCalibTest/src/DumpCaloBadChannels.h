/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-
#ifndef LARCALIBTEST_DUMPCALOBADCHANNELS
#define LARCALIBTEST_DUMPCALOBADCHANNELS

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "CaloConditions/ICaloBadChanTool.h"
#include "CaloIdentifier/CaloCell_ID.h"

class DumpCaloBadChannels : public AthAlgorithm
{
 public:
  using AthAlgorithm::AthAlgorithm;
  ~DumpCaloBadChannels();

  StatusCode initialize(); 
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();

 private:
  ToolHandle<ICaloBadChanTool> m_caloBCT{"CaloBadChanTool"};
  Gaudi::Property<std::string> m_fileName{this, "FileName", ""};
  const CaloCell_ID* m_cellID{nullptr};
};

#endif
