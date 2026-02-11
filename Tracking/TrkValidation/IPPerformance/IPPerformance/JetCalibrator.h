#ifndef IPPerformance_JetCalibrator_H
#define IPPerformance_JetCalibrator_H

// CP interface includes
#include "PATInterfaces/SystematicRegistry.h"
#include "PATInterfaces/SystematicSet.h"
#include "PATInterfaces/SystematicVariation.h"

// external tools include(s):
#include "JetCalibTools/JetCalibrationTool.h"
#include "JetUncertainties/JetUncertaintiesTool.h"
//#include "JetResolution/JERTool.h" // deprecated
//#include "JetResolution/JERSmearingTool.h" // deprecated
#include "JetSelectorTools/JetCleaningTool.h"
#include "JetMomentTools/JetVertexTaggerTool.h"
//#include "xAODMetaData/FileMetaData.h"
//#include "xAODMetaData/FileMetaDataAuxInfo.h"
#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/ServiceHandle.h"

// algorithm wrapper
#include "IPPerformance/ETAlgorithm.h"

class JetCalibrator : public ETAlgorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:
  bool m_DC14;

  // configuration variables
  std::string m_inContainerName;
  std::string m_outContainerName;

  std::string m_jetAlgo;
  std::string m_outputAlgo;
  std::string m_calibConfigData;
  std::string m_calibConfigFullSim;
  std::string m_calibConfigAFII;
  std::string m_calibConfig;
  std::string m_calibSequence;
  std::string m_calibSequenceData;
  std::string m_calibArea;
  std::string m_JESUncertConfig;
  std::string m_JESUncertMCType;
  Gaudi::Property<std::string> m_configFileName{this, "configFileName", "", "config file name"};
  float m_systSigmaVal;
  std::string m_JESJERSyst;
  bool m_setAFII;

//  std::string m_JERUncertConfig;
//  bool m_JERFullSys;
//  bool m_JERApplyNominal;

  std::string m_jetCleanCutLevel;
  bool m_saveAllCleanDecisions;
  bool m_jetCleanUgly;
  bool m_redoJVT;
  // sort after calibration
  bool    m_sort;
  //Apply jet cleaning to parent jet
  bool    m_cleanParent;

  // systematics
  bool m_runSysts;

private:
  int m_numEvent;         //!
  int m_numObject;        //!

  //bool m_isMC;            //!
  Gaudi::Property<bool> m_isMC{this, "isMC", false, " whether the data is Monte Carlo"};
  //bool m_isFullSim;       //!
  Gaudi::Property<bool> m_isFullSim{this,"isFullSim","false","whether the data is Full Simulation"};
  
    // obtain StoreGateSvc
  StoreGateSvc* m_storeGate = nullptr;

  std::string m_JESUncertAlgo;          //!

  std::string m_outSCContainerName;     //!
  std::string m_outSCAuxContainerName;  //!
  Gaudi::Property<std::string> m_filesInput{this, "filesInput", "", "Input File Path"};

  std::vector<CP::SystematicSet> m_systList; //!
  std::vector<int> m_systType; //!

  // tools
  ToolHandle<JetCalibrationTool>    m_jetCalibration{this,"jetCalibration","JetCalibrationTool"};
  ToolHandle<JetUncertaintiesTool>  m_JESUncertTool{this,"JESUncertTool","JetUncertaintiesTool"};

//  JERTool                  * m_JERTool;        //!
//  JERSmearingTool          * m_JERSmearTool;   //!
//  ToolHandle<IJERTool>       m_JERToolHandle;  //!

  //ToolHandle<IJetUpdateJvt>         m_JVTToolHandle;  //! //*don't need
  ToolHandle<JetVertexTaggerTool>   m_JVTTool{this,"JVTTool","JetVertexTaggerTool"};
  ToolHandle<JetCleaningTool>       m_jetCleaning{this,"jetCleaning","JetCleaningTool"};
  std::vector<std::string>  m_decisionNames;    //!
  std::vector< JetCleaningTool* > m_allJetCleaningTools;   //!
  //ServiceHandle<StoreGateSvc> m_metaDataStore{this,"MetaDataStore", "MetaDataStore"};
  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
public:
  // Tree *myTree; //!
  // TH1 *myHist; //!

  // this is a standard constructor
  JetCalibrator (const std::string& name,ISvcLocator* pSvcLocator=nullptr);
  virtual ~JetCalibrator();

  // these are the functions inherited from Algorithm
  virtual StatusCode initialize ();
  virtual StatusCode execute ();
  virtual StatusCode finalize ();

  // added functions not from Algorithm
  bool sort_pt(xAOD::IParticle* partA, xAOD::IParticle* partB);
  std::vector< CP::SystematicSet > getListofSystematics(const CP::SystematicSet recSysts, std::string systName, float systVal );

};

#endif
