/******************************************
 *
 * Interface to Jet calibration tool(s).
 *
 * G.Facini (gabriel.facini@cern.ch), M. Milesi (marco.milesi@cern.ch), J. Dandoy (jeff.dandoy@cern.ch)
 *
 *
 ******************************************/

// c++ include(s):
#include <iostream>

// EDM include(s):
#include "xAODJet/JetContainer.h"
#include "xAODJet/Jet.h"
#include "xAODBase/IParticleHelpers.h"
#include "xAODBase/IParticleContainer.h"
#include "xAODBase/IParticle.h"
#include "AthContainers/ConstDataVector.h"
#include "AthContainers/DataVector.h"
#include "xAODCore/ShallowCopy.h"

// package include(s):
#include "IPPerformance/JetCalibrator.h"
#include "StoreGate/StoreGateSvc.h"
// ROOT include(s):
#include "TEnv.h"
#include "TSystem.h"
#include "StoreGate/StoreGateSvc.h"

// this is needed to distribute the algorithm to the workers

 JetCalibrator :: JetCalibrator (const std::string& name,ISvcLocator* pSvcLocator) :AthAlgorithm(name,pSvcLocator),
  m_runSysts(false)          // gets set later is syst applies to this tool
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().

  ATH_MSG_INFO("JetCalibrator(): Calling constructor");

  // CONFIG parameters for JetUncertaintiesTool
  m_JESUncertConfig         = "";
  m_JESJERSyst             = "None";
  m_systSigmaVal            = 1.;

}

JetCalibrator::~JetCalibrator() {
}


StatusCode JetCalibrator :: initialize ()
{
  // Here you do everything that you need to do after the first input
  // file has been connected and before the first event is processed,
  // e.g. create additional histograms based on which variables are
  // available in the input files.  You can also create all of your
  // histograms and trees in here, but be aware that this method
  // doesn't get called if no events are processed.  So any objects
  // you create here won't be available in the output if you have no
  // input events.

  ATH_MSG_INFO("initialize(): Initializing JetCalibrator Interface... ");
  m_runSysts = false; //Ensure this starts false

  if( m_isMC ) 
    ATH_MSG_INFO("initialize(): Running on MC sample.");
  else ATH_MSG_INFO("initialize(): Running on data sample.");

  // If there is no InputContainer we must stop
  if ( m_inContainKey.empty()) {
    ATH_MSG_ERROR("InputContainer is empty!");
    return StatusCode::FAILURE;
  }
  ATH_CHECK(m_inContainKey.initialize());

  m_outSCContainerName      = m_outContainerName + "ShallowCopy";
  m_outSCAuxContainerName   = m_outSCContainerName + "Aux."; // the period is very important!

  m_numEvent      = 0;

  ANA_CHECK( m_jetCalibration.retrieve() );

  // initialize and configure the jet cleaning tool
  //------------------------------------------------
  ANA_CHECK(m_jetCleaning.retrieve());

  // initialize and configure the jet uncertainity tool
  // only initialize if a config file has been given
  //------------------------------------------------
  ATH_MSG_INFO("SystName " << m_systName);
  // Set values from EL Algorithm (Algorithm.h) to those from config
  // There may be more elegant ways to do this...
  m_systName = m_JESJERSyst;
  m_systVal = m_systSigmaVal;


  if ( !m_JESUncertConfig.empty() && !m_systName.empty()  && m_systName != "None" ) {
    m_JESUncertConfig = gSystem->ExpandPathName( m_JESUncertConfig.c_str() );
    ATH_MSG_INFO("initialize(): Initialize JES UNCERT with " << m_JESUncertConfig);
    std::string ju_tool_name = std::string("JESProvider_") + std::string(name());
    ANA_CHECK(m_JESUncertTool.retrieve());
    const CP::SystematicSet recSysts = m_JESUncertTool->recommendedSystematics();

    ATH_MSG_INFO("initialize():  Initializing Jet Systematics :");

    //If just one systVal, then push it to the vector
    if( m_systValVector.size() == 0)
      m_systValVector.push_back(m_systVal);

    for(unsigned int iSyst=0; iSyst < m_systValVector.size(); ++iSyst){
      m_systVal = m_systValVector.at(iSyst);
      std::vector<CP::SystematicSet> JESSysList = getListofSystematics( recSysts, m_systName, m_systVal );

      //for ( const auto& syst_it : JESSysList ){
      for(unsigned int i=0; i < JESSysList.size(); ++i){
        m_systList.push_back(  JESSysList.at(i) );
        m_systType.push_back(1);
      }
    }

    // Setup the tool for the 1st systematic on the list
    // If running all, the tool will be setup for each syst on each event
    if ( !m_systList.empty() ) {
      m_runSysts = true;
      // setup uncertainity tool for systematic evaluation
      if ( m_JESUncertTool->applySystematicVariation(m_systList.at(0)) != StatusCode::SUCCESS ) {
        ATH_MSG_ERROR("initialize(): Cannot configure JetUncertaintiesTool for systematic :"<< m_systName);
        return StatusCode::FAILURE;
      }
    }
  } // running systematics
  else {
    ATH_MSG_INFO("initialize(): No JES/JER Uncertainities considered");
  }

  // if not running systematics, need the nominal
  // if running systematics, and running them all, need the nominal
  // add it to the front!
  if ( m_systList.empty() || (!m_systList.empty() && m_systName == "All") ) {
    m_systList.insert( m_systList.begin(), CP::SystematicSet() );
    const CP::SystematicVariation nullVar = CP::SystematicVariation(""); // blank = nominal
    m_systList.begin()->insert(nullVar);
    m_systType.insert(m_systType.begin(), 0);
  }

  for ( const auto& syst_it : m_systList ){
    ATH_MSG_INFO("initialize():  Running with systematic : " << syst_it.name());
  }
  ATH_CHECK(service("StoreGateSvc", m_storeGate));
  return StatusCode::SUCCESS;
}

StatusCode JetCalibrator :: finalize ()
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

  ATH_MSG_INFO("finalize(): Deleting tool instances...");

  return StatusCode::SUCCESS;
}


StatusCode JetCalibrator ::execute ()
{
  const EventContext& ctx = Gaudi::Hive::currentContext();
  m_numEvent++;

  SG::ReadHandle<xAOD::JetContainer> inJets{m_inContainKey, ctx};
  if (!inJets.isValid()) {
    ATH_MSG_ERROR ("Couldn't retrieve xAOD::JetContainer with key: " << m_inContainKey.key() );
    return StatusCode::FAILURE;
  }
  // loop over available systematics - remember syst == "Nominal" --> baseline
  std::vector< std::string >* vecOutContainerNames = new std::vector< std::string >;
  for ( const auto& syst_it : m_systList ) {
    unsigned int sysIndex = (&syst_it - &m_systList[0]);
    int thisSysType = m_systType.at(sysIndex);

    std::string outSCContainerName(m_outSCContainerName);
    std::string outSCAuxContainerName(m_outSCAuxContainerName);
    std::string outContainerName(m_outContainerName);

    // always append the name of the variation, including nominal which is an empty string
    outSCContainerName    += syst_it.name();
    outSCAuxContainerName += syst_it.name();
    outContainerName      += syst_it.name();
    vecOutContainerNames->push_back( syst_it.name() );

    // create shallow copy;
    std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > calibJetsSC = xAOD::shallowCopyContainer( *inJets.cptr() );
    ConstDataVector<xAOD::JetContainer>* calibJetsCDV = new ConstDataVector<xAOD::JetContainer>(SG::VIEW_ELEMENTS);
    calibJetsCDV->reserve( calibJetsSC.first->size() );

    // Nominal calibration for all inputs 
    // In rel22: pass full jet container instead of correcting each jet in for-loop
    if( m_jetCalibration->applyCalibration( *(calibJetsSC.first) ) == StatusCode::FAILURE ){
      ATH_MSG_ERROR("execute(): JetCalibration tool reported a CP::CorrectionCode::Error");
      ATH_MSG_ERROR("execute()"<< name());
      return StatusCode::FAILURE;
    }//for jets

    //Apply Uncertainties
    if ( m_runSysts ) {

      if ( thisSysType == 1 ){
        // JES/JER Uncertainty Systematic
        if ( m_JESUncertTool->applySystematicVariation(syst_it) != StatusCode::SUCCESS ) {
          ATH_MSG_ERROR("execute(): Cannot configure JetUncertaintiesTool for systematic :"<<m_systName);
          return StatusCode::FAILURE;
        }
        for ( auto jet_itr : *(calibJetsSC.first) ) {
          if ( m_runSysts ) {
            if ( m_JESUncertTool->applyCorrection( *jet_itr ) == CP::CorrectionCode::Error ) {
              ATH_MSG_ERROR("execute(): JetUncertaintiesTool reported a CP::CorrectionCode::Error");
              ATH_MSG_ERROR("execute():"<< name());
            }
          }
        }//for jets
      }//JES/JER

    }// if m_runSysts

    // decorate with cleaning decision
    for ( auto jet_itr : *(calibJetsSC.first) ) {

      // Decorations are stored as "char" for technical reasons (basically an STL vector of bools does not behave in a normal way)
      static SG::AuxElement::Decorator< char > isCleanDecor( "cleanJet" );
      const xAOD::Jet* jetToClean = jet_itr;

      isCleanDecor(*jet_itr) = bool( m_jetCleaning->accept(*jetToClean) );

    } //end cleaning decision

    if ( !xAOD::setOriginalObjectLink(*inJets.cptr(), *(calibJetsSC.first)) ) {
      ATH_MSG_ERROR("execute() : Failed to set original object links -- MET rebuilding cannot proceed.");
    }

    // save pointers in ConstDataVector with same order
    for ( auto jet_itr : *(calibJetsSC.first) ) {
      calibJetsCDV->push_back( jet_itr );
    }

    // add shallow copy to StoreGate
    ATH_CHECK( m_storeGate->record( calibJetsSC.first, outSCContainerName));
    ATH_CHECK( m_storeGate->record( calibJetsSC.second, outSCAuxContainerName));
    // add ConstDataVector to StoreGate
    ATH_CHECK( m_storeGate->record( calibJetsCDV, outContainerName));
  }
  // add vector of systematic names to StoreGate
  ATH_CHECK( m_storeGate->record( vecOutContainerNames, m_outputAlgo));

  // look what do we have in TStore
  return StatusCode::SUCCESS;
}


bool JetCalibrator::sort_pt(xAOD::IParticle* partA, xAOD::IParticle* partB){
  return partA->pt() > partB->pt();
}

// Get the subset of systematics to consider
// can also return full set if systName = "All"

std::vector< CP::SystematicSet > JetCalibrator::getListofSystematics(const CP::SystematicSet recSysts, std::string systName, float systVal ) {
  std::vector< CP::SystematicSet > systList;
  // loop over recommended systematics
  for( const auto &syst : recSysts ) {
    ATH_MSG_INFO("HelperFunctions::getListofSystematics()" << syst.basename());
    if( systName == syst.basename() ) {
      ATH_MSG_INFO("HelperFunctions::getListofSystematics(): Found match! Adding systematic :"<< syst.basename());
      // continuous systematics - can choose at what sigma to evaluate
      if (syst == CP::SystematicVariation (syst.basename(), CP::SystematicVariation::CONTINUOUS)) {
        systList.push_back(CP::SystematicSet());
        if ( systVal == 0 ) {
          ATH_MSG_ERROR("HelperFunctions::getListofSystematics(): Setting continuous systematic to 0 is nominal! Please check!");
          //RCU_THROW_MSG("Failure");
	  throw std::runtime_error("Failure");
        }
        systList.back().insert(CP::SystematicVariation (syst.basename(), systVal));
      }
      // not a continuous system
      else {
        systList.push_back(CP::SystematicSet());
        systList.back().insert(syst);
      }
    } // found match!
    else if ( systName == "All" ) {
      ATH_MSG_INFO("HelperFunctions::initialize(): Adding systematic :"<< syst.basename());
      // continuous systematics - can choose at what sigma to evaluate
      // add +1 and -1 for when running all
      if (syst == CP::SystematicVariation (syst.basename(), CP::SystematicVariation::CONTINUOUS)) {
        if ( systVal == 0 ) {
          ATH_MSG_ERROR("HelperFunctions::getListofSystematics(): Setting continuous systematic to 0 is nominal! Please check!");
          //RCU_THROW_MSG("Failure");
	  throw std::runtime_error("Failure");
        }
        systList.push_back(CP::SystematicSet());
        systList.back().insert(CP::SystematicVariation (syst.basename(),  fabs(systVal)));
        systList.push_back(CP::SystematicSet());
        systList.back().insert(CP::SystematicVariation (syst.basename(), -1.0*fabs(systVal)));
      }
      // not a continuous systematic
      else {
        systList.push_back(CP::SystematicSet());
        systList.back().insert(syst);
      }
    } // running all
  } // loop over recommended systematics
  return systList;
}

