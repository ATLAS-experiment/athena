/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrackSimSGInput_FPGATrackSimRawHistWrapperAlg_h
#define FPGATrackSimSGInput_FPGATrackSimRawHistWrapperAlg_h


#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "FPGATrackSimSGInput/IFPGATrackSimInputTool.h"

class TFile;
class TTree;
class TH2F;
class FPGATrackSimEventInputHeader;
/**
 * @brief Steering algorithm to run IFPGATrackSimInputTool and save the output in plain ROOT tree
 * 
 */
class TrigFPGATrackSimRawHitsWrapperAlg : public AthAlgorithm {
public:
  TrigFPGATrackSimRawHitsWrapperAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TrigFPGATrackSimRawHitsWrapperAlg() {};
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;


private:
  // configuration parameters  
  ToolHandle<IFPGATrackSimInputTool> m_hitInputTool { this, "InputTool", "FPGATrackSimSGToRawHitsTool/FPGATrackSimInputTool", "HitInput Tool" };
  Gaudi::Property<std::string> m_outpath { this, "OutFileName", "httsim_smartwrapper.root", "output path" };
  Gaudi::Property<std::string> m_metaData { this, "WrapperMetaData", "", "meta data" };

  // internal pointers
  FPGATrackSimEventInputHeader* m_eventHeader = nullptr;
  TFile* m_outfile = nullptr;
  TTree* m_EventTree = nullptr;
};

#endif
