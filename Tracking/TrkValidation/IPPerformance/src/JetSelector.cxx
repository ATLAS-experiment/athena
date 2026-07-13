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
#include <StoreGate/StoreGateSvc.h>
#include <GaudiKernel/ServiceHandle.h>

// package include(s):
#include "xAODEventInfo/EventInfo.h"
#include "IPPerformance/JetSelector.h"

// ROOT include(s):
#include "TEnv.h"
#include "TFile.h"
#include "TSystem.h"
#include "TObjArray.h"
#include "TObjString.h"

JetSelector :: JetSelector (const std::string& name,ISvcLocator* pSvcLocator):AthAlgorithm(name,pSvcLocator),
  m_jet_cutflowHist(nullptr)
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // initialization code will go into histInitialize() and
  // initialize().

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
  ATH_MSG_INFO("initialize(): Calling initialize");

   // *****CUTFLOW**********
  ServiceHandle<ITHistSvc> histSvc ("THistSvc", "JetSelector");
  ATH_CHECK( histSvc.retrieve() );
  m_jet_cutflowHist  = new TH1D("cutflow_jets", "cutflow_jets", 1, 1, 2);
  ATH_CHECK( histSvc->regHist("/MYSTREAM/cutflow_jets",m_jet_cutflowHist) );
  m_jet_cutflowHist->SetCanExtend(TH1::kAllAxes);
  m_jet_cutflow_all             = m_jet_cutflowHist->GetXaxis()->FindBin("all");
  m_jet_cutflow_cleaning_cut    = m_jet_cutflowHist->GetXaxis()->FindBin("cleaning");     
  m_jet_cutflow_ptmin_cut       = m_jet_cutflowHist->GetXaxis()->FindBin("pTmin");      
  m_jet_cutflow_eta_cut         = m_jet_cutflowHist->GetXaxis()->FindBin("eta"); 
  m_jet_cutflow_e_cut           = m_jet_cutflowHist->GetXaxis()->FindBin("e");
  m_jet_cutflow_jvt_cut         = m_jet_cutflowHist->GetXaxis()->FindBin("JVT");
  
  if (m_jetKey.empty()) {
    ATH_MSG_INFO("configure(): InputJetContainer is empty!");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("configure(): InputJetContainer: "<< m_jetKey.key());
  ATH_CHECK( m_jetKey.initialize() );

  m_decor   = "passSel";

  if ( m_decorateSelectedObjects ) {
    ATH_MSG_INFO(name()<<" Decorate Jets with :"<< m_decor);
  }

  ANA_CHECK( m_jetCleaning.retrieve() );

  ATH_MSG_INFO("initialize(): JetSelector Interface succesfully initialized!" );

  ATH_CHECK(m_storeGate.retrieve());

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

  ATH_MSG_INFO("finalize():"<< name());

  return StatusCode::SUCCESS;
}


StatusCode JetSelector :: execute ()
{
  // Here you do everything that needs to be done on every single
  // events, e.g. read input variables, apply cuts, and fill
  // histograms and trees.  This is where most of your actual analysis
  // code will go.

  // retrieve event
  const EventContext& ctx = Gaudi::Hive::currentContext();
  const xAOD::EventInfo* eventInfo = 0;
  ATH_CHECK (evtStore()->retrieve (eventInfo, "EventInfo"));
  SG::ReadHandle<xAOD::JetContainer> inJets{m_jetKey, ctx};
  if (!inJets.isValid()) {
    ATH_MSG_ERROR ("Couldn't retrieve xAOD::Jet with key: " << m_jetKey.key() );
    return StatusCode::FAILURE;
  }
    // create output container (if requested)
  ConstDataVector<xAOD::JetContainer>* selectedJets(nullptr);
  if ( m_createSelectedContainer ) {
    selectedJets = new ConstDataVector<xAOD::JetContainer>(SG::VIEW_ELEMENTS);
  }

  // if doing JVF or JVT get PV location
  const xAOD::VertexContainer *vertices = 0;
  if ( m_doJVT ) {
    if ( !evtStore()->retrieve( vertices, "PrimaryVertices").isSuccess() ){ 
    ATH_MSG_ERROR("execute(): Failed to retrieve Input Vertex container from store. Exiting." );
    return StatusCode::FAILURE; }
    m_pvLocation = getPrimaryVertexLocation( vertices );
  }

//Jet Cleaning first. 
  CleanJets(inJets.cptr() , m_jetCleaning.get());
  static SG::AuxElement::Accessor< char > isCleanAcc("cleanJet");
  static SG::AuxElement::Decorator< char > passSelDecor( m_decor );
  
  for ( auto jet_itr : *inJets.cptr() ) { 
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
    ATH_CHECK(m_storeGate->record( selectedJets, m_outContainerName ));
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

void JetSelector::CleanJets(const xAOD::JetContainer* cleanJetcopy , JetCleaningTool* jetCleaning) {

  for ( auto jet_itr : *cleanJetcopy ) {

    static SG::AuxElement::Decorator< char > isCleanDecor( "cleanJet" );
    const xAOD::Jet* jetToClean = jet_itr;

    isCleanDecor(*jet_itr) = bool( jetCleaning->accept(*jetToClean) );

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

