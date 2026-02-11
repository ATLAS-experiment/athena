/******************************************
 *
 * Jet selector tool
 *
 * Adapted from xAODAnaHelpers/JetSelector.cxx
 *      G.Facini (gabriel.facini@cern.ch), 
 *      M. Milesi (marco.milesi@cern.ch)
 *      J.Alison (john.alison@cern.ch)
 *
 * Adapted by E. Duffield (eduffield@berkeley.edu)
 * Thur Aug 6 13:16:22 PST 2015
 *
 ******************************************/

// c++ include(s):
#include <iostream>
#include <typeinfo>
#include <sstream>

// EDM include(s):
#include "xAODJet/JetContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODCore/ShallowCopy.h"
#include "AthContainers/ConstDataVector.h"
#include "PATInterfaces/SystematicVariation.h"
#include "PATInterfaces/SystematicRegistry.h"
//#include "PATInterfaces/SystematicCode.h" // deprecated. Replaced by StatusCode

// package include(s):
#include "xAODEventInfo/EventInfo.h"
#include "IPPerformance/JetSelector.h"
#include "IPPerformance/ReturnCheck.h"

// external tools include(s):

// ROOT include(s):
#include "TEnv.h"
#include "TFile.h"
#include "TSystem.h"
#include "TObjArray.h"
#include "TObjString.h"

JetSelector :: JetSelector (const std::string& name,ISvcLocator* pSvcLocator):ETAlgorithm(name,pSvcLocator),
  m_jet_cutflowHist(nullptr)
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().
  Info("JetSelector()", "Calling constructor");

  // read debug flag from .config file
  m_debug         = false;

  // input container to be read from TEvent or TStore
  m_inJetContainerName      = "";
  m_jetScaleType            = "";


  // decorate selected objects that pass the cuts
  m_decorateSelectedObjects = true;
  m_createSelectedContainer = false;
  // if requested, a new container is made using the SG::VIEW_ELEMENTS option
  m_outContainerName        = "";
  m_jetCleanCutLevel        = "LooseBad";
  m_jetCleanUgly            = "false";

  // cuts
  m_cleanJets               = true;
  m_pT_min                  = 1e8;
  m_eta_max                 = 1e8;
  m_e_min                   = 1e8;

  m_doJVT 		              = true;
  m_JVTCut 		              = 0.64;


}

JetSelector::~JetSelector() {
}

StatusCode JetSelector :: initialize ()
{
  // Here you do everything that you need to do after the first input
  // file has been connected and before the first event is processed,
  // e.g. create additional histograms based on which variables are
  // available in the input files.  You can also create all of your
  // histograms and trees in here, but be aware that this method
  // doesn't get called if no events are processed.  So any objects
  // you create here won't be available in the output if you have no
  // input events.
  Info("initialize()", "Calling initialize");

   // *****CUTFLOW**********
    //
    TFile *file =TFile::Open("ctflow","RECREATE");
    file->cd();


    m_jet_cutflowHist  = new TH1D("cutflow_jets", "cutflow_jets", 1, 1, 2);
    ATH_CHECK( histSvc()->regHist("/MYSTREAM/cutflow_jets",m_jet_cutflowHist) );
    m_jet_cutflowHist->SetCanExtend(TH1::kAllAxes);
    m_jet_cutflow_all             = m_jet_cutflowHist->GetXaxis()->FindBin("all");
    m_jet_cutflow_cleaning_cut    = m_jet_cutflowHist->GetXaxis()->FindBin("cleaning");     
    m_jet_cutflow_ptmin_cut       = m_jet_cutflowHist->GetXaxis()->FindBin("pTmin");      
    m_jet_cutflow_eta_cut         = m_jet_cutflowHist->GetXaxis()->FindBin("eta"); 
    m_jet_cutflow_e_cut           = m_jet_cutflowHist->GetXaxis()->FindBin("e");
    m_jet_cutflow_jvt_cut         = m_jet_cutflowHist->GetXaxis()->FindBin("JVT");
 
    //configure()
    setConfig(m_configFileName);
    if(!getConfig().empty()){
    Info("configure()", "Configuing JetSelector Interface. User configuration read from : %s ", getConfig().c_str());

    TEnv* config = new TEnv(getConfig(true).c_str());

    // read debug flag from .config file
    m_debug         = config->GetValue("Debug" ,      m_debug);

    // input container to be read from TEvent or TStore
    m_inJetContainerName      = config->GetValue("InputJetContainer",  m_inJetContainerName.c_str());
    m_jetScaleType            = config->GetValue("JetScaleType",  m_jetScaleType.c_str());
    //If not set, find default from input container name
    if (m_jetScaleType.size() == 0){
      if( m_inJetContainerName.find("EMTopo") != std::string::npos){
        m_jetScaleType = "JetEMScaleMomentum";
      }else{
        m_jetScaleType = "JetConstitScaleMomentum";
      }
    }

    // // name of algo input container comes from - only if running on syst
    // m_inputAlgo               = config->GetValue("InputAlgo",   m_inputAlgo.c_str());
    // m_outputAlgo              = config->GetValue("OutputAlgo",  m_outputAlgo.c_str());

    // decorate selected objects that pass the cuts
    m_decorateSelectedObjects = config->GetValue("DecorateSelectedObjects", m_decorateSelectedObjects);
    m_createSelectedContainer = config->GetValue("CreateSelectedContainer", m_createSelectedContainer);
    // if requested, a new container is made using the SG::VIEW_ELEMENTS option
    m_outContainerName        = config->GetValue("OutputContainer", m_outContainerName.c_str());
    m_jetCleanCutLevel        = config->GetValue("JetCleanCutLevel",        m_jetCleanCutLevel.c_str());
    m_jetCleanUgly            = config->GetValue("JetCleanUgly",            m_jetCleanUgly );


    // cuts
    m_cleanJets               = config->GetValue("CleanJets",   m_cleanJets);
    m_pT_min                  = config->GetValue("pTMin",       m_pT_min);
    m_e_min                   = config->GetValue("eMin",        m_e_min);
    m_eta_max                 = config->GetValue("etaMax",      m_eta_max);
    m_doJVT                           = config->GetValue("DoJVT",       m_doJVT);
    m_JVTCut                          = config->GetValue("JVTCut",      m_JVTCut);


    config->Print();
    Info("configure()", "JetSelector Interface succesfully configured! ");

    delete config; config = nullptr;
  }

  m_isEMjet = m_inJetContainerName.find("EMTopoJets") != std::string::npos;

  if ( m_inJetContainerName.empty() ) {
    Error("configure()", "InputContainer is empty!");
    return StatusCode::FAILURE;
  }

  m_decor   = "passSel";

  if ( m_decorateSelectedObjects ) {
    Info(m_name.c_str()," Decorate Jets with %s", m_decor.c_str());
  }
  //configure() end!


  std::string jc_tool_name = std::string("JetCleaning_") + m_name; //m_name comes from algorithm.cxx
  ANA_CHECK( m_jetCleaning.retrieve() );

  Info("initialize()", "JetSelector Interface succesfully initialized!" );

  RETURN_CHECK("JetSelector::initialize()", service("StoreGateSvc", m_storeGate), "Failed to retrieve StoreGateSvc.");
  return StatusCode::SUCCESS;

  return StatusCode::SUCCESS;
}



StatusCode JetSelector :: finalize ()
{
  // This method is the mirror image of initialize(), meaning it gets
  // called after the last event has been processed on the worker node
  // and allows you to finish up any objects you created in
  // initialize() before they are written to disk.  This is actually
  // fairly rare, since this happens separately for each worker node.
  // Most of the time you want to do your post-processing on the
  // submission node after all your histogram outputs have been
  // merged.  This is different from histFinalize() in that it only
  // gets called on worker nodes that processed input events.

  Info("finalize()", "%s", m_name.c_str());

  return StatusCode::SUCCESS;
}


StatusCode JetSelector :: execute ()
{
  // Here you do everything that needs to be done on every single
  // events, e.g. read input variables, apply cuts, and fill
  // histograms and trees.  This is where most of your actual analysis
  // code will go.
  if ( m_debug ) { Info("execute()", "Applying Jet Selection... "); }

  // retrieve event
  const xAOD::EventInfo* eventInfo = 0;
  ANA_CHECK (evtStore()->retrieve (eventInfo, "EventInfo"));

  const xAOD::JetContainer* inJets = 0;
  if ( !evtStore()->retrieve( inJets, m_inJetContainerName).isSuccess() ){ 
    Error("execute()", "Failed to retrieve Input Jet container. Exiting." );
    return StatusCode::FAILURE;
  }
  Info("initialize()", "Input jet container = %s", m_inJetContainerName.c_str());
  //  //make copy of jets //took out the shallow copy since it really wasn't liking it.
  // std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > inJetShallowCopyPair = xAOD::shallowCopyContainer( *inJets );
  // xAOD::JetContainer* cleanJetcopy = inJetShallowCopyPair.first;
  
    // create output container (if requested)
  ConstDataVector<xAOD::JetContainer>* selectedJets(nullptr);
  if ( m_createSelectedContainer ) {
    selectedJets = new ConstDataVector<xAOD::JetContainer>(SG::VIEW_ELEMENTS);
  }

  // if doing JVF or JVT get PV location
  const xAOD::VertexContainer *vertices = 0;
  if ( m_doJVT ) {
    if ( !evtStore()->retrieve( vertices, "PrimaryVertices").isSuccess() ){ 
    Error("execute()", "Failed to retrieve Input Vertex container from store. Exiting." );
    return StatusCode::FAILURE; }
    m_pvLocation = getPrimaryVertexLocation( vertices );
  }

//Jet Cleaning first. 
  CleanJets(inJets , m_jetCleaning.get());
  

  static SG::AuxElement::Accessor< char > isCleanAcc("cleanJet");
  static SG::AuxElement::Decorator< char > passSelDecor( m_decor );

  for ( auto jet_itr : *inJets ) { 
    int passSel = this->PassCuts( jet_itr );
    if ( m_decorateSelectedObjects ) {
      passSelDecor( *jet_itr ) = passSel;
    }

    if ( passSel ) {
      if ( m_createSelectedContainer ) {
        selectedJets->push_back( jet_itr );
      }
    }
  } // end jet loop


  // add ConstDataVector to TStore
  if ( m_createSelectedContainer ) {
    RETURN_CHECK("JetSelector::execute()",  m_storeGate->record( selectedJets, m_outContainerName ), "Failed to store const data container.");
  }
  return StatusCode::SUCCESS;
}


int JetSelector::PassCuts( const xAOD::Jet* jet ) {
  m_jet_cutflowHist->Fill( m_jet_cutflow_all, 1 );

  // clean jets
  static SG::AuxElement::Accessor< char > isCleanAcc("cleanJet");
  if ( m_cleanJets ) {
    if ( isCleanAcc.isAvailable( *jet ) ) {
      if ( !isCleanAcc( *jet ) ) { return 0; }
    }
  }

  m_jet_cutflowHist->Fill( m_jet_cutflow_cleaning_cut, 1 );        

  // pT      
  if ( m_pT_min != 1e8 ) {
    if ( jet->pt() < m_pT_min ) { return 0; }
  }
  m_jet_cutflowHist->Fill( m_jet_cutflow_ptmin_cut, 1 );        

  // eta
  if ( m_eta_max != 1e8 ) {
    if ( fabs(jet->eta()) > m_eta_max ) { return 0; }
  }
  m_jet_cutflowHist->Fill( m_jet_cutflow_eta_cut, 1 );        

  // e
  if ( m_e_min != 1e8 ) {
    if ( fabs(jet->e()) < m_e_min ) { return 0; }
  }
  m_jet_cutflowHist->Fill( m_jet_cutflow_e_cut, 1 );        

  // JVT  cut
  if ( m_doJVT ){
    if ( jet->getAttribute< float >( "Jvt" )  < m_JVTCut ) {
      return 0;
    }
  } // m_doJVT
  m_jet_cutflowHist->Fill( m_jet_cutflow_jvt_cut, 1 );        
  return 1;
}

void JetSelector::CleanJets(const xAOD::JetContainer* cleanJetcopy , JetCleaningTool* m_jetCleaning) {

   for ( auto jet_itr : *cleanJetcopy ) {

      static SG::AuxElement::Decorator< char > isCleanDecor( "cleanJet" );
      const xAOD::Jet* jetToClean = jet_itr;

      isCleanDecor(*jet_itr) = bool( m_jetCleaning->accept(*jetToClean) );

    } //end cleaning decision

}

int JetSelector::getPrimaryVertexLocation(const xAOD::VertexContainer* vertexContainer) {
  int location(0);
  for( auto vtx_itr : *vertexContainer )
  {
    if(vtx_itr->vertexType() == xAOD::VxType::VertexType::PriVtx) {
      return location;
    }
    location++;
  }
  return -1;
}

