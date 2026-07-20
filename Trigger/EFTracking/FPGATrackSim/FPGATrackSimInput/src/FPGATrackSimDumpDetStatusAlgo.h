/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrackSimDumpDetStatusAlgo_h
#define FPGATrackSimDumpDetStatusAlgo_h

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ToolHandle.h"

#include "FPGATrackSimDetectorTool.h" 


/////////////////////////////////////////////////////////////////////////////
class FPGATrackSimDumpDetStatusAlgo: public AthAlgorithm {
public:
  FPGATrackSimDumpDetStatusAlgo (const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~FPGATrackSimDumpDetStatusAlgo () override = default;
  virtual StatusCode initialize() override ;
  virtual StatusCode execute(const EventContext& ctx)    override;


private:
  ToolHandle<FPGATrackSimDetectorTool> m_detectorTool   {this, "FPGATrackSimDetectorTool", "FPGATrackSimDetectorTool/FPGATrackSimDetectorTool"}; 
  Gaudi::Property<bool> m_DumpBadModules       {this, "DumpBadModules", false, "If true enable dump of bad modules for FPGATrackSim"};
  Gaudi::Property<bool> m_DumpModuleIDMap      {this, "DumpModuleIDMap", false, "If true dumps the map of the modules in each tower"};
  Gaudi::Property<bool> m_DumpGlobalToLocalMap {this, "DumpGlobalToLocalMap",false, "True if you want to produce the Global-to-Local map"};
  Gaudi::Property<bool> m_DumpIDMap            {this, "DumpIDMap", false};
  Gaudi::Property<bool> m_DumpModulePositions  {this, "DumpModulePositions",false,"To dump the corner positions of the modules"};

};

#endif // FPGATrackSimDumpDetStatusAlgo_h
