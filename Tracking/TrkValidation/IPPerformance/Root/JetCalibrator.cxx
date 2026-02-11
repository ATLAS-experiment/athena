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
#include "xAODEventInfo/EventInfo.h"
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
#include "IPPerformance/ReturnCheck.h"
#include "StoreGate/StoreGateSvc.h"
// ROOT include(s):
#include "TEnv.h"
#include "TSystem.h"
//#include "xAODMetaData/FileMetaData.h"
//#include "xAODMetaData/FileMetaDataAuxInfo.h"
#include "StoreGate/StoreGateSvc.h"
#include "EventInfo/TagInfo.h"
using std::cout;
using std::endl;

// this is needed to distribute the algorithm to the workers

 JetCalibrator :: JetCalibrator (const std::string& name,ISvcLocator* pSvcLocator) :ETAlgorithm(name,pSvcLocator),
  m_runSysts(false)          // gets set later is syst applies to this tool
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().

  Info("JetCalibrator()", "Calling constructor");


  // read debug flag from .config file
  m_debug                   = false;

  m_sort                    = true;
  // input container to be read from TEvent or TStore
  m_inContainerName         = "";
  // shallow copies are made with this output container name
  m_outContainerName        = "";

  // CONFIG parameters for JetCalibrationTool
  m_jetAlgo                 = "";
  m_outputAlgo              = "";

  // when running data "_Insitu" is appended to this string
  m_calibSequence           = "JetArea_Residual_Origin_EtaJES_GSC";
  m_calibSequenceData       = "JetArea_Residual_Origin_EtaJES_GSC_Insitu";
  m_calibConfigFullSim      = "JES_MC15Prerecommendation_April2015.config";
  m_calibConfigAFII         = "JES_Prerecommendation2015_AFII_Apr2015.config";
  m_calibConfigData         = "JES_MC15Prerecommendation_April2015.config";

  // CONFIG parameters for JetUncertaintiesTool
  m_JESUncertConfig         = "";
  m_JESUncertMCType         = "MC16";
  m_JESJERSyst             = "None";
  m_systSigmaVal            = 1.;
  m_setAFII                 = false;

  // CONFIG parameters for JERSmearingTool
//  m_JERUncertConfig         = "";
//  m_JERFullSys              = false;
//  m_JERApplyNominal         = false;

  // CONFIG parameters for JetCleaningTool
  m_jetCleanCutLevel        = "LooseBad";
  m_saveAllCleanDecisions   = false;
  m_jetCleanUgly            = false;
  m_cleanParent             = false;

  //recalculate JVT using calibrated jets
  m_redoJVT                 = false;

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

  Info("initialize()", "Initializing JetCalibrator Interface... ");
  m_runSysts = false; //Ensure this starts false

  // const xAOD::EventInfo* eventInfo(nullptr);
  // RETURN_CHECK("JetCalibrator::execute()", HelperFunctions::retrieve(eventInfo, m_eventInfoContainerName, m_event, m_store, m_verbose) ,"");
  /*const xAOD::EventInfo* eventInfo = 0;
  ANA_CHECK (evtStore()->retrieve (eventInfo, "EventInfo"));*/  //
  //if( ! m_event->retrieve( eventInfo, m_eventInfoContainerName).isSuccess() ){
  //  Error("initialize()", "Failed to retrieve event info collection. Exiting." );
  //  return EL::StatusCode::FAILURE;
  //} 
  //m_isMC = ( eventInfo->eventType( xAOD::EventInfo::IS_SIMULATION ) );
  if( m_isMC ) Info("initialize()", "Running on MC sample.");
  else Info("initialize()", "Running on data sample.");

  if(!m_isMC){
     std::cout<<"Running on data" << std::endl;
   }
  //configure()
  setConfig(m_configFileName);
  if ( !getConfig().empty() ) {

    Info("configure()", "Configuring JetCalibrator Interface. User configuration read from : %s ", getConfig().c_str());

    TEnv* config = new TEnv(getConfig(true).c_str());

    // read debug flag from .config file
    m_debug                   = config->GetValue("Debug" , m_debug);
    m_sort                    = config->GetValue("Sort",            m_sort);
    // input container to be read from TEvent or TStore
    m_inContainerName         = config->GetValue("InputContainer",  m_inContainerName.c_str());
    // shallow copies are made with this output container name
    m_outContainerName        = config->GetValue("OutputContainer", m_outContainerName.c_str());

    // CONFIG parameters for JetCalibrationTool
    m_jetAlgo                 = config->GetValue("JetAlgorithm",    m_jetAlgo.c_str());
    m_outputAlgo              = config->GetValue("OutputAlgo",      m_outputAlgo.c_str());

    // when running data "_Insitu" is appended to this string
    m_calibSequence           = config->GetValue("CalibSequence",           m_calibSequence.c_str());
    m_calibSequenceData       = config->GetValue("CalibSequenceData",       m_calibSequenceData.c_str());
    m_calibArea               = config->GetValue("CalibArea",       m_calibArea.c_str());
    m_calibConfigFullSim      = config->GetValue("configNameFullSim",       m_calibConfigFullSim.c_str());
    m_calibConfigAFII         = config->GetValue("configNameAFII",          m_calibConfigAFII.c_str());
    m_calibConfigData         = config->GetValue("configNameData",          m_calibConfigData.c_str());

    // CONFIG parameters for JetUncertaintiesTool
    m_JESJERSyst             = config->GetValue("JESJERSyst", m_JESJERSyst.c_str());
    m_systSigmaVal            = config->GetValue("systSigmaVal", m_systSigmaVal);
    m_JESUncertConfig         = config->GetValue("JESUncertConfig", m_JESUncertConfig.c_str());
    m_JESUncertMCType         = config->GetValue("JESUncertMCType", m_JESUncertMCType.c_str());
    m_setAFII                 = config->GetValue("SetAFII",  m_setAFII);

    // CONFIG parameters for JERSmearingTool
 //   m_JERUncertConfig         = config->GetValue("JERUncertConfig", m_JERUncertConfig.c_str());
 //   m_JERFullSys              = config->GetValue("JERFullSys",      m_JERFullSys);
 //   m_JERApplyNominal         = config->GetValue("JERApplyNominal", m_JERApplyNominal);

    // CONFIG parameters for JetCleaningTool
    m_jetCleanCutLevel        = config->GetValue("JetCleanCutLevel",        m_jetCleanCutLevel.c_str());
    m_jetCleanUgly            = config->GetValue("JetCleanUgly",            m_jetCleanUgly );
    m_saveAllCleanDecisions   = config->GetValue("SaveAllCleanDecisions",   m_saveAllCleanDecisions);
    m_cleanParent             = config->GetValue("CleanParent",             m_cleanParent);

    m_redoJVT                 = config->GetValue("RedoJVT",         m_redoJVT);

    config->Print();

    delete config; config = nullptr;
  }

  // If there is no InputContainer we must stop
  if ( m_inContainerName.empty() ) {
    Error("configure()", "InputContainer is empty!");
    return StatusCode::FAILURE;
  }

  if ( m_outputAlgo.empty() ) {
    m_outputAlgo = m_jetAlgo + "_Calib_Algo";
  }

  m_JESUncertAlgo = m_jetAlgo;

  m_outSCContainerName      = m_outContainerName + "ShallowCopy";
  m_outSCAuxContainerName   = m_outSCContainerName + "Aux."; // the period is very important!

  if ( !getConfig().empty() )
    Info("configure()", "JetCalibrator Interface succesfully configured! ");
  //configure() end!

  m_numEvent      = 0;

  // Configure jet calibrator
  if ( !m_isMC ) m_calibSequence = m_calibSequenceData;

  //KB: Need to check if the following statement still applies in r22
  //Insitu should not be applied to the trimmed jets, per Jet/Etmiss recommendation
  if ( !m_isMC && m_calibSequence.find("Insitu") == std::string::npos && m_inContainerName.find("AntiKt10LCTopoTrimmedPtFrac5SmallR20") == std::string::npos) m_calibSequence += "_Insitu";

  // Check that correct calibration sequence is used
  if( m_isMC && m_calibSequence.find("Insitu") != std::string::npos){
    Error("initialize()", "Attempting to use an Insitu calibration sequence on MC.  Exiting.");
    return StatusCode::FAILURE;
  }

  if ( !m_isMC ) {
    m_calibConfig = m_calibConfigData;
  }else{
    m_calibConfig = m_calibConfigFullSim;
    // treat as fullsim by default
    //m_isFullSim = true;
    // Check simulation flavour for calibration config - cannot directly read metadata in xAOD otside of Athena!
    //
    // N.B. (Marco) : With SampleHandler, you can define sample metadata in job steering macro!
    //                They will be passed to the EL:;Worker automatically and can be retrieved anywhere in the EL::Algorithm
    //                I reasonably suppose everyone will use SH...
    //

    // KB: The following lines do not seem to work as it uses a custom variable "sim_flav" that is not accessible at run time.
    //const std::string stringMeta = wk()->metaData()->castString("sim_flav"); // NB: needs to be defined as sample metadata in job steering macro. Should be either "AFII" or "FullSim"
    //if ( m_setAFII ) {
    //  Info("initialize()", "Setting simulation flavour to AFII according to config file. Please double check if this is really an AFII sample!");
    //  m_isFullSim = false;
    //}else if ( stringMeta.empty() ) {
    //  Warning("initialize()", "Could not access simulation flavour from EL::Worker. Treating MC as FullSim by default!" );
    //} else {
    //  m_isFullSim = (stringMeta == "AFII") ? false : true;
    //}

    if ( !m_isFullSim ) {
      m_calibConfig = m_calibConfigAFII;
    }
  }

  // initialize jet calibration tool
  std::string jcal_tool_name = std::string("JetCorrectionTool_") + m_name;
  /*
  m_jetCalibration = new JetCalibrationTool(jcal_tool_name.c_str(),
      m_jetAlgo,
      m_calibConfig,
      m_calibSequence,
      !m_isMC);
  */
  cout<<jcal_tool_name.c_str()<<endl;
  cout<<m_jetAlgo<<endl;
  cout<<m_calibConfig<<endl;
  cout<<m_calibSequence<<endl;
  cout<<m_calibArea<<endl;
  cout<<m_isMC<<endl;
  ANA_CHECK( m_jetCalibration.retrieve() );

  // initialize and configure the jet cleaning tool
  //------------------------------------------------
  std::string jc_tool_name = std::string("JetCleaning_") + m_name;
  ANA_CHECK(m_jetCleaning.retrieve());
  if (m_jetCleanUgly){
    ANA_CHECK(m_jetCleaning->setProperty( "DoUgly", true));
  }
 
  //m_saveAllCleanDecisions=false 
  if( m_saveAllCleanDecisions ){
    //std::string m_decisionNames[] = {"LooseBad", TightBad"};
    m_decisionNames.push_back( "LooseBad" );
    m_decisionNames.push_back( "LooseBadUgly" );
    m_decisionNames.push_back( "TightBad" );
    m_decisionNames.push_back( "TightBadUgly" );
    for(unsigned int i=0; i < m_decisionNames.size() ; ++i){
      m_allJetCleaningTools.push_back( new JetCleaningTool((jc_tool_name+"_pass"+m_decisionNames.at(i)).c_str()) );
      if( m_decisionNames.at(i).find("Ugly") != std::string::npos ){
        std::cout << "adding for " << m_decisionNames.at(i).substr(0,m_decisionNames.at(i).size()-4) << std::endl;
        RETURN_CHECK( "JetCalibrator::initialize()", m_allJetCleaningTools.at( i )->setProperty( "CutLevel", m_decisionNames.at(i).substr(0,m_decisionNames.at(i).size()-4) ), "");
        RETURN_CHECK( "JetCalibrator::initialize()", m_allJetCleaningTools.at( i )->setProperty( "DoUgly", true ), "");
      }else{
        RETURN_CHECK( "JetCalibrator::initialize()", m_allJetCleaningTools.at( i )->setProperty( "CutLevel", m_decisionNames.at(i)), "");
      }
      RETURN_CHECK( "JetCalibrator::initialize()", m_allJetCleaningTools.at( i )->initialize(), ("JetCleaning Interface "+m_decisionNames.at(i)+" succesfully initialized!").c_str());
    }
  } 

  // initialize and configure the jet uncertainity tool
  // only initialize if a config file has been given
  //------------------------------------------------
  std::cout << "SystName " << m_systName << std::endl;
  // Set values from EL Algorithm (Algorithm.h) to those from config
  // There may be more elegant ways to do this...
  m_systName = m_JESJERSyst;
  m_systVal = m_systSigmaVal;


  if ( !m_JESUncertConfig.empty() && !m_systName.empty()  && m_systName != "None" ) {
    m_JESUncertConfig = gSystem->ExpandPathName( m_JESUncertConfig.c_str() );
    Info("initialize()","Initialize JES UNCERT with %s", m_JESUncertConfig.c_str());
    std::string ju_tool_name = std::string("JESProvider_") + m_name;
    ANA_CHECK(m_JESUncertTool.retrieve());
    //m_JESUncertTool->msg().setLevel( MSG::ERROR ); // VERBOSE, INFO, DEBUG
    const CP::SystematicSet recSysts = m_JESUncertTool->recommendedSystematics();

    Info("initialize()"," Initializing Jet Systematics :");

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
        Error("initialize()", "Cannot configure JetUncertaintiesTool for systematic %s", m_systName.c_str());
        return StatusCode::FAILURE;
      }
    }
  } // running systematics
  else {
    Info("initialize()", "No JES/JER Uncertainities considered");
    // m_JESUncertTool not streamed so have to do this
    //m_JESUncertTool = nullptr;
  }
 


  // initialize and configure the JVT correction tool
  if(m_redoJVT){
    ANA_CHECK(m_JVTTool.retrieve());
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
    Info("initialize()"," Running with systematic : %s", (syst_it.name()).c_str());
  }

  RETURN_CHECK("JetCalibrator::initialize()", service("StoreGateSvc", m_storeGate), "Failed to retrieve StoreGateSvc.");
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

  Info("finalize()", "Deleting tool instances...");

  return StatusCode::SUCCESS;
}


StatusCode JetCalibrator ::execute ()
{
  if ( m_debug ) { Info("execute()", "Applying Jet Calibration and Cleaning... "); }

  m_numEvent++;

  // get the collection from TEvent or TStore
  // const xAOD::JetContainer* inJets(nullptr);
  // RETURN_CHECK("JetCalibrator::execute()", HelperFunctions::retrieve(inJets, m_inContainerName, m_event, m_store, m_verbose) ,"");

  const xAOD::JetContainer* inJets = 0;
  if ( !evtStore()->retrieve( inJets, m_inContainerName).isSuccess() ){ 
    Error("execute()", "Failed to retrieve Input Jet container. Exiting." );
    return StatusCode::FAILURE;
  } 
  // loop over available systematics - remember syst == "Nominal" --> baseline
  std::vector< std::string >* vecOutContainerNames = new std::vector< std::string >;
  //std::vector< int >
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
    std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > calibJetsSC = xAOD::shallowCopyContainer( *inJets );
    ConstDataVector<xAOD::JetContainer>* calibJetsCDV = new ConstDataVector<xAOD::JetContainer>(SG::VIEW_ELEMENTS);
    calibJetsCDV->reserve( calibJetsSC.first->size() );

    // Nominal calibration for all inputs 
    // In rel22: pass full jet container instead of correcting each jet in for-loop
    if( m_jetCalibration->applyCalibration( *(calibJetsSC.first) ) == StatusCode::FAILURE ){
      Error("execute()", "JetCalibration tool reported a CP::CorrectionCode::Error");
      Error("execute()", "%s", m_name.c_str());
      return StatusCode::FAILURE;
    }//for jets

    //Apply Uncertainties
    if ( m_runSysts ) {

      if ( thisSysType == 1 ){
        // JES/JER Uncertainty Systematic
        if( m_debug ) { std::cout << "Configure JES for systematic variation : " << syst_it.name() << std::endl; }
        if ( m_JESUncertTool->applySystematicVariation(syst_it) != StatusCode::SUCCESS ) {
          Error("execute()", "Cannot configure JetUncertaintiesTool for systematic %s", m_systName.c_str());
          return StatusCode::FAILURE;
        }
        for ( auto jet_itr : *(calibJetsSC.first) ) {
          if ( m_runSysts ) {
            if ( m_JESUncertTool->applyCorrection( *jet_itr ) == CP::CorrectionCode::Error ) {
              Error("execute()", "JetUncertaintiesTool reported a CP::CorrectionCode::Error");
              Error("execute()", "%s", m_name.c_str());
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

      if(m_cleanParent){
	      //ElementLink<xAOD::JetContainer> el_parent = jet_itr->auxdata<ElementLink<xAOD::JetContainer> >("Parent") ;
	      static const SG::AuxElement::Accessor<ElementLink<xAOD::JetContainer>> mAcc_parent("Parent");
              ElementLink<xAOD::JetContainer> el_parent = mAcc_parent(*jet_itr);

	      if(!el_parent.isValid()){
	        Error("jetDecision()", "Could not make jet cleaning decision on the parent! It doesn't exist.");
	      } else {
	        jetToClean = *el_parent;
	      }
      }

      isCleanDecor(*jet_itr) = bool( m_jetCleaning->accept(*jetToClean) );
      // Alternatively could do
      // isCleanDecor(jet) = m_jetCleaning->accept(jet).getCutResult("Cleaning")
      // Could then get result of individual cuts by replacing "Cleaning" by a different string
      // For total decision this is not recommended as it involves time-consuming string comparisons.


      if( m_saveAllCleanDecisions ){
        for(unsigned int i=0; i < m_allJetCleaningTools.size() ; ++i){
	  static const SG::Accessor <char> mAcc_clean_pass("clean_pass"+m_decisionNames.at(i));
	  mAcc_clean_pass(*jet_itr) = bool( m_allJetCleaningTools.at(i)->accept(*jetToClean) );
          //jet_itr->auxdata< char >(("clean_pass"+m_decisionNames.at(i)).c_str()) = bool( m_allJetCleaningTools.at(i)->accept(*jetToClean) );
        }
      }
    } //end cleaning decision

    if ( !xAOD::setOriginalObjectLink(*inJets, *(calibJetsSC.first)) ) {
      Error("execute()  ", "Failed to set original object links -- MET rebuilding cannot proceed.");
    }

    // Recalculate JVT using calibrated Jets
    if(m_redoJVT){
      for ( auto jet_itr : *(calibJetsSC.first) ) {
	static const SG::Accessor<float> mAcc_Jvt("Jvt");
	mAcc_Jvt(*jet_itr) = m_JVTTool->updateJvt(*jet_itr);//*here m_JVTTool has been ToolHandle!
        //jet_itr->auxdata< float >("Jvt") = m_JVTTool->updateJvt(*jet_itr);//*here m_JVTTool has been ToolHandle!
      }
    }

    // save pointers in ConstDataVector with same order
    for ( auto jet_itr : *(calibJetsSC.first) ) {
      calibJetsCDV->push_back( jet_itr );
    }

    // can only sort the CDV - a bit no-no to sort the shallow copies
    // if ( m_sort ) {
    //   std::sort( calibJetsCDV->begin(), calibJetsCDV->end(), sort_pt );
    // }


    // add shallow copy to StoreGate
    RETURN_CHECK( "JetCalibrator::execute()", m_storeGate->record( calibJetsSC.first, outSCContainerName), "Failed to record shallow copy container.");
    RETURN_CHECK( "JetCalibrator::execute()", m_storeGate->record( calibJetsSC.second, outSCAuxContainerName), "Failed to record shallow copy aux container.");

    // add ConstDataVector to StoreGate
    RETURN_CHECK( "JetCalibrator::execute()", m_storeGate->record( calibJetsCDV, outContainerName), "Failed to record const data container.");
  }
  // add vector of systematic names to StoreGate
  RETURN_CHECK( "JetCalibrator::execute()", m_storeGate->record( vecOutContainerNames, m_outputAlgo), "Failed to record vector of output container names.");


  // look what do we have in TStore
  //if ( m_verbose ) { m_store->print(); }
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
    Info("HelperFunctions::getListofSystematics()","  %s", (syst.basename()).c_str());
    if( systName == syst.basename() ) {
      Info("HelperFunctions::getListofSystematics()","Found match! Adding systematic %s", syst.basename().c_str());
      // continuous systematics - can choose at what sigma to evaluate
      if (syst == CP::SystematicVariation (syst.basename(), CP::SystematicVariation::CONTINUOUS)) {
        systList.push_back(CP::SystematicSet());
        if ( systVal == 0 ) {
          Error("HelperFunctions::getListofSystematics()","Setting continuous systematic to 0 is nominal! Please check!");
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
      Info("HelperFunctions::initialize()","Adding systematic %s", syst.basename().c_str());
      // continuous systematics - can choose at what sigma to evaluate
      // add +1 and -1 for when running all
      if (syst == CP::SystematicVariation (syst.basename(), CP::SystematicVariation::CONTINUOUS)) {
        if ( systVal == 0 ) {
          Error("HelperFunctions::getListofSystematics()","Setting continuous systematic to 0 is nominal! Please check!");
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

