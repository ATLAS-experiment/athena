/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#ifndef FPGATrackSim_DUMPOUTPUTSTATALG_H
#define FPGATrackSim_DUMPOUTPUTSTATALG_H


#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "FPGATrackSimInput/FPGATrackSimOutputHeaderTool.h"


class TH2F;

class FPGATrackSimDumpOutputStatAlg : public AthAlgorithm {
public:
  FPGATrackSimDumpOutputStatAlg (const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~FPGATrackSimDumpOutputStatAlg () {};
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;
  StatusCode BookHistograms();


private:
  ToolHandle<FPGATrackSimOutputHeaderTool>    m_readOutputTool  { this, "InputTool",  "FPGATrackSimOutputHeaderTool/ReadOutputHeaderTool", "Input Tool" };
  
  // Make these configurable. If they are set to the empty string branches won't get read.
  Gaudi::Property<std::string> m_inputBranchName { this, "InputBranchName", "", "Branch containing InputHeader to read from ROOT file." };
  Gaudi::Property<std::string> m_outputBranchName { this, "OutputBranchName", "", "Branch containing OutputHeader to read from ROOT file." };

  FPGATrackSimLogicalEventInputHeader* m_eventInputHeader = nullptr;
  FPGATrackSimLogicalEventOutputHeader* m_eventOutputHeader = nullptr;

  // Internal counter.
  unsigned m_event = 0;

  // histograms
  //TH2F*   m_hits_r_vs_z = nullptr;
};

#endif // FPGATrackSim_DUMPOUTPUTSTATALG_H
