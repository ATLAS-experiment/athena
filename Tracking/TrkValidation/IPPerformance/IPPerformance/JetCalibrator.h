#ifndef IPPerformance_JetCalibrator_H
#define IPPerformance_JetCalibrator_H

// CP interface includes
#include "PATInterfaces/SystematicRegistry.h"
#include "PATInterfaces/SystematicSet.h"
#include "PATInterfaces/SystematicVariation.h"

// external tools include(s):
#include "JetCalibTools/JetCalibrationTool.h"
#include "JetUncertainties/JetUncertaintiesTool.h"
#include "JetSelectorTools/JetCleaningTool.h"
#include "JetMomentTools/JetVertexTaggerTool.h"
#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/ServiceHandle.h"

// algorithm wrapper
#include "AthenaBaseComps/AthAlgorithm.h"
class JetCalibrator : public AthAlgorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:

  // configuration variables
  SG::ReadHandleKey<xAOD::JetContainer> m_inContainKey{this, "inContainKey", "AntiKt4EMTopoJets"};  
  Gaudi::Property<std::string> m_outContainerName{this, "OutputContainer", "Jets_Calib"};

  Gaudi::Property<std::string> m_jetAlgo{this, "JetAlgorithm", "AntiKt4EMTopo"};
  Gaudi::Property<std::string> m_outputAlgo{this, "OutputAlgo", "Jets_Calib_Algo"};
  std::string m_JESUncertConfig;
  float m_systSigmaVal;
  std::string m_JESJERSyst;

  // systematics
  bool m_runSysts;
  std::string m_systName;
  float       m_systVal;
  std::vector<float> m_systValVector;

private:
  int m_numEvent;         //!

  Gaudi::Property<bool> m_isMC{this, "isMC", false, " whether the data is Monte Carlo"};
  Gaudi::Property<bool> m_isFullSim{this,"isFullSim","false","whether the data is Full Simulation"};
  
    // obtain StoreGateSvc
  StoreGateSvc* m_storeGate = nullptr;

  std::string m_outSCContainerName;     //!
  std::string m_outSCAuxContainerName;  //!

  std::vector<CP::SystematicSet> m_systList; //!
  std::vector<int> m_systType; //!

  // tools
  ToolHandle<JetCalibrationTool>    m_jetCalibration{this,"jetCalibration","JetCalibrationTool"};
  ToolHandle<JetUncertaintiesTool>  m_JESUncertTool{this,"JESUncertTool","JetUncertaintiesTool"};

  ToolHandle<JetCleaningTool>       m_jetCleaning{this,"jetCleaning","JetCleaningTool"};
  std::vector<std::string>  m_decisionNames;    //!
  std::vector< JetCleaningTool* > m_allJetCleaningTools;   //!

public:

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
