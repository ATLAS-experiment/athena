#ifndef IPPerformance_JetSelector_H
#define IPPerformance_JetSelector_H

// EDM include(s):
#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"

// ROOT include(s):
#include "TH1D.h"

// algorithm wrapper
#include "IPPerformance/ETAlgorithm.h"
#include "JetSelectorTools/JetCleaningTool.h"

class JetSelector : public ETAlgorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:
  bool m_debug;

  // configuration variables
  std::string m_inJetContainerName;   // input container name
  std::string m_outContainerName;  		// output container name
  std::string m_jetScaleType;    			// Type of Scale Momementum
  std::string m_decor;           		  // The decoration key written to passing objects
  std::string m_jetCleanCutLevel;  // Level of jet cleaning cut
  Gaudi::Property<std::string> m_configFileName{this, "configFileName", "", "config file name"};
  bool m_jetCleanUgly;			        // Whether or not to remove ugly jets
  bool m_decorateSelectedObjects; 		// decorate selected objects? defaul passSel
  bool m_createSelectedContainer; 		// fill using SG::VIEW_ELEMENTS to be light weight
  bool m_cleanJets;               		// require cleanJet decoration to not be set and false
  float m_pT_min;                 		// require pT > pt_max
  float m_eta_max;                		// require eta < eta_max
  float m_e_min;											// require e > e_min
  bool m_doJVT;                   		// check JVT
  float m_JVTCut;                 		// cut value


private:

  int m_pvLocation;       //!

  bool m_isTruthjet;                //!
  bool m_isEMjet;                //!
  bool m_isLCjet;                //!
 
  // obtain StoreGateSvc
  StoreGateSvc* m_storeGate = nullptr;

   // cutflow
  TH1D* m_jet_cutflowHist;  //!
  TH1D* m_cutflowHist;          //!
  int   m_cutflow_bin;          //!

  //JetCleaningTool          * m_jetCleaning;    //!
  ToolHandle<JetCleaningTool>   m_jetCleaning{this,"jetCleaning","JetCleaningTool"};
  /* object-level cutflow */
  
  int   m_jet_cutflow_all;           //! 
  int   m_jet_cutflow_cleaning_cut;  //!
  int   m_jet_cutflow_ptmin_cut;     //!
  int   m_jet_cutflow_eta_cut;       //!
  int   m_jet_cutflow_e_cut;         //!
  int   m_jet_cutflow_jvt_cut;       //!

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

  // these are the functions not inherited from Algorithm

  // added functions not from Algorithm
  // why does this need to be virtual?
  virtual int PassCuts( const xAOD::Jet* jet );
  void CleanJets(const xAOD::JetContainer* cleanJetcopy , JetCleaningTool* m_jetCleaning);
	int getPrimaryVertexLocation(const xAOD::VertexContainer* vertexContainer);

};

#endif
