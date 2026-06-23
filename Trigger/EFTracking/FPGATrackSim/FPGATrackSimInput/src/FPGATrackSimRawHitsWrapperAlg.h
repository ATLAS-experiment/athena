/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/


#ifndef FPGATrackSim_RAWHITSWRAPPERALG_H
#define FPGATrackSim_RAWHITSWRAPPERALG_H


#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "FPGATrackSimInput/IFPGATrackSimEventInputHeaderTool.h"

class FPGATrackSimRawHitsWrapperAlg : public AthAlgorithm {
public:
  FPGATrackSimRawHitsWrapperAlg (const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~FPGATrackSimRawHitsWrapperAlg () = default;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx)    override;
  virtual StatusCode finalize()   override;
  StatusCode BookHistograms();


private:
  // configuration parameters  
  //TODO: use input from SG: FPGATrackSimSGToRawHitsTool/IFPGATrackSimInputTool
  ToolHandle<IFPGATrackSimEventInputHeaderTool>    m_readOutputTool  { this, "InputTool",  "FPGATrackSimInputHeaderTool/ReadInputHeaderTool", "Input Tool" };
  ToolHandle<IFPGATrackSimEventInputHeaderTool>    m_writeOutputTool { this, "OutputTool", "FPGATrackSimInputHeaderTool/WriteInputHeaderTool", "Output Tool" };
 
  // some debug counters
  unsigned int m_tot_hits=0;
  unsigned int m_tot_truth=0;
  unsigned int m_tot_oftracks=0;
};

#endif // FPGATrackSimRAWHITSWRAPPERALG_h
