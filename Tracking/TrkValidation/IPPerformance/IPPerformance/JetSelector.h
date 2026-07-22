#ifndef IPPerformance_JetSelector_H
#define IPPerformance_JetSelector_H

// EDM include(s):
#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"

// ROOT include(s):
#include "TH1D.h"

// algorithm wrapper
#include "JetSelectorTools/JetCleaningTool.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ServiceHandle.h"
class JetSelector : public AthAlgorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:

  // configuration variables
  Gaudi::Property<std::string> m_outContainerName{this, "OutContainerName", "AntiKt4EMTopoJets_Selected", "Name of the output container"};
  SG::ReadHandleKey<xAOD::JetContainer> m_jetKey{this, "JetsKey", "Jets_Calib"};
  std::string m_decor;           		  // The decoration key written to passing objects
  Gaudi::Property<bool> m_decorateSelectedObjects{this, "DecorateSelectedObjects", true, "Decorate selected objects (default: passSel)"};
  Gaudi::Property<bool> m_createSelectedContainer{this, "CreateSelectedContainer", true, "Create selected container using SG::VIEW_ELEMENTS (lightweight)"};  

  Gaudi::Property<bool>  m_cleanJets{this, "CleanJets", true};
  Gaudi::Property<float> m_pT_min{this, "pTMin", 20e3};
  Gaudi::Property<float> m_e_min{this, "eMin", 0.0};
  Gaudi::Property<float> m_eta_max{this, "etaMax", 2.5};
  Gaudi::Property<bool>  m_doJVT{this, "DoJVT", true};
  Gaudi::Property<float> m_JVTCut{this, "JVTCut", 0.64};

private:

  int m_pvLocation{};       //!
  ServiceHandle<StoreGateSvc> m_storeGate{this,"StoreGateSvc","StoreGateSvc"};

   // cutflow
  TH1D* m_jet_cutflowHist{};  //!
  TH1D* m_cutflowHist{};          //!

  ToolHandle<JetCleaningTool>   m_jetCleaning{this,"jetCleaning","JetCleaningTool"};
  /* object-level cutflow */
  
  int   m_jet_cutflow_all{};           //! 
  int   m_jet_cutflow_cleaning_cut{};  //!
  int   m_jet_cutflow_ptmin_cut{};     //!
  int   m_jet_cutflow_eta_cut{};       //!
  int   m_jet_cutflow_e_cut{};         //!
  int   m_jet_cutflow_jvt_cut{};       //!

  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
public:

  // this is a standard constructor
  JetSelector (const std::string& name,ISvcLocator* pSvcLocator=nullptr);
  virtual ~JetSelector();

  // these are the functions inherited from Algorithm
  virtual StatusCode initialize ();
  virtual StatusCode execute ();
  virtual StatusCode finalize ();

  // added functions not from Algorithm
  // why does this need to be virtual?
  virtual int PassCuts( const xAOD::Jet* jet );
  void CleanJets(const xAOD::JetContainer* cleanJetcopy , JetCleaningTool* m_jetCleaning);
  int getPrimaryVertexLocation(const xAOD::VertexContainer* vertexContainer);

};

#endif
