/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////
//                                                         
//  Header file for class VP1Alg                           
//                                                         
//                                                         
//  This is the Athena algorithm starting a VP1 GUI        
//
//  Major updates: 
//  - May 2014, riccardo.mria.bianchi
//  - Dec 2023, riccardo.maria.bianchi@cern.ch - migration to CA
//                                                         
/////////////////////////////////////////////////////////////


#ifndef VP1ALGS_VP1ALG
#define VP1ALGS_VP1ALG

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/IIncidentListener.h"
#include "Gaudi/Property.h"

#include <string>
#include <vector>

class VP1Gui;

class VP1Alg: public AthAlgorithm,
              public IIncidentListener
{
 public:
  VP1Alg(const std::string& name, ISvcLocator* pSvcLocator);
  ~VP1Alg();

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

  void handle(const Incident& inc) override;

 private:

  // Properties
  // -- Athena-related
  Gaudi::Property<std::string> m_atlasRelease{this, "AtlasRelease", "", "The current, in use Atlas release"}; 
  // -- MultipleFiles mode
  Gaudi::Property<bool> m_mfOn{this, "MultipleFilesON", false, 
      "Flag to turn the 'Multiple Files' mode ON/OFF. Default OFF"};
  Gaudi::Property<int> m_mfLimit{this, "MFNumFileLimit", 10, 
      "MultipleFiles mode -- Maximum number of local copies to keep"};
  Gaudi::Property<std::string> m_mfSourceDir{this, "MFSourceDir", "", 
      "MultipleFiles mode -- Path of the source directory, where input data files are stored and from which events are taken from"};
  Gaudi::Property<std::string> m_mfLocalCopyDir{this, "MFLocalCopyDir", ".", 
      "MultipleFiles mode -- Path of the directory to keep local copies of processed events. Default '.' (the local run folder)"};
  Gaudi::Property<std::vector<std::string>> m_mfAvailableLocalInputDirectories{this, "MFAvailableLocalInputDirectories", {}, 
      "MultipleFiles mode -- Will only be used if sourcedir is set and local"};
  // -- Cruise mode
  Gaudi::Property<std::string> m_cruiseInitialMode{this, "CruiseInitialMode", "NONE", 
      "Cruise mode -- Initial Cruise mode: 'NONE', 'EVENT', 'TAB', 'BOTH'. Default: 'NONE'"};
  Gaudi::Property<unsigned> m_cruiseInitialUpdateSeconds{this, "CruiseInitialSeconds", 10, "Cruise mode -- Initial Cruise update time, in seconds. Default: 10"};
  // -- Input files
  Gaudi::Property<std::vector<std::string>> m_initialvp1files{
      this, "InitialInputVP1Files", {}, "a vector of input VP1 files"};
  // -- VP1 GUI
  Gaudi::Property<bool> m_noGui{this, "noGUI", false, 
      "Flag to turn OFF the GUI. Default: FALSE (i.e., GUI is ON)"};





  IToolSvc* m_toolSvc;
  VP1Gui * m_vp1gui;
/* 
  // Work In Progress 
  // The variables below are needed in different VP1 configurations, 
  // which still need to be migrated to CA. In particular:
  // - watch mode
  // - cruise mode
  // - batch mode / no GUI
  // - multiple input files
  // - pick input files from a specific directory
  // 
  std::vector<std::string> m_initialvp1files;
  std::string m_initialCruiseMode;//"NONE", "EVENT", "TAB", "BOTH".
  unsigned m_initialCruiseSeconds;

  bool m_noGui;//For testing job-options in RTT

  // Properties for multiple input files (mf)
  bool m_mfOn;                        // Flag to turn multiple files ON/OFF. Default OFF
  std::string m_mfSourceDir;          // Directory to take event files from
  std::string m_mfLocalCopyDir;       // Directory to keep local copies of processed events. Default "."
  int m_mfLimit;                      // Maximum number of local copies to keep
  std::vector<std::string> m_mfAvailableLocalInputDirectories;//Will only be used if sourcedir is set and local
*/

};

#endif
