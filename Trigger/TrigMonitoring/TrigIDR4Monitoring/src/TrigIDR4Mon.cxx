/**
 **     @file    TrigIDR4Mon.cxx
 **
 **     @brief   implementation of a AthAlgorithm based monitoring base class
 **
 **     @author  Mark Sutton (sutt@cern.ch)
 **     @date    Tue  29 Sep 2025 09:08:26 GMT
 **
 **     Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 **/

// #include "TrigInDetAnalysis/Filter_AcceptAll.h"

// #include "TrigInDetAnalysisUtils/Filters.h"
// #include "TrigInDetAnalysisUtils/Filter_Track.h"
// #include "TrigInDetAnalysisUtils/TagNProbe.h"

#include "TrigInDetAnalysisExample/ChainString.h"
#include "TrigInDetAnalysisExample/TIDAHistogram.h"

#include "TrigIDR4Mon.h"

// #include <mutex>

TrigIDR4Mon::TrigIDR4Mon( const std::string & name, ISvcLocator* pSvcLocator) 
  :  AthMonitorAlgorithm(name, pSvcLocator), 
     m_tdt("Trig::TrigDecisionTool/TrigDecisionTool"),
     m_buildNtuple(false),
     m_initialisePerRun(true),
     m_firstRun(true),
     m_keepAllEvents(false),
     m_fileopen(false),
     m_useHighestPT(false),
     m_vtxIndex(-1),
     m_runPurity(false),
     m_shifter(false),
     m_sliceTag(""),
     m_containTracks(false),
     m_legacy(true), 
     m_fiducial_radius(32),
     m_requireDecision(false),
     m_filter_on_roi(false)
{
  msg(MSG::INFO) << "TrigIDR4Mon::TrigIDR4Mon() compiled: " << __DATE__ << " " << __TIME__ << endmsg;

  declareProperty( "SliceTag",   m_sliceTag = "" );

  declareProperty( "pTCut",   m_pTCut   = 0 );
  declareProperty( "etaCut",  m_etaCut  = 5 );
  declareProperty( "d0Cut",   m_d0Cut   = 1000 );
  declareProperty( "z0Cut",   m_z0Cut   = 2000 );
  declareProperty( "siHits",  m_siHits  = -1 );
  m_pixHits=0;
  m_sctHits=0;

  declareProperty( "trtHits",   m_trtHits   = -2 );
  declareProperty( "strawHits", m_strawHits = -2 );

  declareProperty( "tauEtCutOffline",   m_tauEtCutOffline = 0 );
  declareProperty( "doTauThreeProng",   m_doTauThreeProng = false);

  declareProperty( "pTCutOffline",      m_pTCutOffline      = 1000 );
  declareProperty( "etaCutOffline",     m_etaCutOffline     = 2.5 );
  declareProperty( "d0CutOffline",      m_d0CutOffline      = 1000 );
  declareProperty( "mind0CutOffline",   m_mind0CutOffline   = 0 );
  declareProperty( "z0CutOffline",      m_z0CutOffline      = 2000 );
  declareProperty( "pixHitsOffline",    m_pixHitsOffline    =  2 ); // 1 <- old value ( 2 degrees of freedom = 1 cluster ) 
  declareProperty( "sctHitsOffline",    m_sctHitsOffline    =  6 ); // 6 <- old value ( 6 clusters = 3 spacepoints )
  declareProperty( "siHitsOffline",     m_siHitsOffline     =  8 );
  declareProperty( "blayerHitsOffline", m_blayerHitsOffline = -1 ); // no requirement - in case IBL is off

  declareProperty( "pixHolesOffline",   m_pixHolesOffline   =  20 ); // essentially no limit
  declareProperty( "sctHolesOffline",   m_sctHolesOffline   =  20 ); // essentially no limit
  declareProperty( "siHolesOffline",    m_siHolesOffline    =  2 );  // npix holes + nsi holes <= 2 ( not degrees of freedom ! )   

  declareProperty( "ContainTracks",     m_containTracks     = false );  // use only basic track containment
  declareProperty( "FiducialRadius",    m_fiducial_radius = 32 );

  declareProperty( "trtHitsOffline",    m_trtHitsOffline    = -2 );
  declareProperty( "strawHitsOffline",  m_strawHitsOffline  = -2 );

  declareProperty( "matchR",   m_matchR   = 0.1 );
  declareProperty( "matchPhi", m_matchPhi = 0.1 );

  declareProperty( "ntupleChainNames",  m_ntupleChainNames );
  declareProperty( "releaseMetaData",   m_releaseMetaData );

  declareProperty( "buildNtuple",   m_buildNtuple = false );

  declareProperty( "AnalysisConfig", m_analysis_config = "Ntuple");

  declareProperty( "SelectTruthPdgId",       m_selectTruthPdgId = 0 );
  declareProperty( "SelectParentTruthPdgId", m_selectParentTruthPdgId = 0 );

  declareProperty( "InitialisePerRun", m_initialisePerRun = false );
  declareProperty( "KeepAllEvents",    m_keepAllEvents = false );
  declareProperty( "UseHighestPT",     m_useHighestPT = false );
  declareProperty( "VtxIndex",         m_vtxIndex = -1 );

  declareProperty( "RunPurity",        m_runPurity = false );
  declareProperty( "Shifter",          m_shifter = false );

  declareProperty( "GenericFlag",      m_genericFlag = true );

  declareProperty( "Legacy",           m_legacy = true );

  declareProperty( "outputFileName",   m_outputFileName = "TrkNtuple.root" );

  declareProperty( "RequireDecision", m_requireDecision = false );

  declareProperty( "FilterOnRoi",     m_filter_on_roi = false );
  
  msg(MSG::INFO) << "TrigIDR4Mon::TrigIDR4Mon() exiting " << gDirectory->GetName() << endmsg;

}



TrigIDR4Mon::~TrigIDR4Mon() {

  // if ( m_fileopen ) { 
  //   for ( unsigned i=0 ; i<m_sequences.size() ; i++ ) m_sequences[i]->finalize();
  //   for ( unsigned i=0 ; i<m_sequences.size() ; i++ ) delete m_sequences[i];
  //   m_fileopen = false;
  // }

}



StatusCode TrigIDR4Mon::initialize() {

  msg(MSG::DEBUG) << " ----- enter initialize() ----- " << endmsg;

  msg(MSG::INFO) << "TrigIDR4Mon::initialize() " << gDirectory->GetName() << " " << m_sliceTag << endmsg;

  /// NB: Do NOT create the sequences here - leave it until the book() method, since
  ///     we need to be automatically determine which chains to process, and so need
  ///     the TrigDecisionTool which is niot configured until we have an iov

#if 0
  std::cout << "TrigIDR4Mon::name              = " << name()     << std::endl;
  std::cout << "TrigIDR4Mon::SliceTag          = " << m_sliceTag << std::endl;
  std::cout << "TrigIDR4Mon::AnalysisConfig    = " << m_analysis_config << std::endl;
  std::cout << "TrigIDR4Mon::Legacy            = " << m_legacy   << std::endl;
#endif

  ATH_CHECK(m_monTools.retrieve());  

  ATH_CHECK(bookHistograms());

  /// pointless setup of ReadHandleKeys because the monitoring appears to
  /// (uneccessarily) be being controlled by the scheduler
  /// We fetch the tracks and vertices in a helper class so don't want to mess 
  /// with this ReadHandleKey - it is only here to placate the scheduler

  ATH_CHECK (m_trackdummykeys.initialize());
  ATH_CHECK (m_vtxdummykeys.initialize());
  
  ATH_MSG_DEBUG( " -----  exit init() ----- " );

  return AthMonitorAlgorithm::initialize();

}


StatusCode TrigIDR4Mon::bookHistograms() {

  ATH_MSG_DEBUG( "           ----- enter book() ----- " );

  ATH_MSG_INFO( "            TrigIDR4Mon::book() " << gDirectory->GetName() );

  /// useful colour information - leave here ...
  // "^[[91;1m"
  // "^[[m"
 
  ATH_MSG_INFO( "^[[91;1m" << name() << "\t:AnalysisConfig " << m_analysis_config << "^[[m" );
  
  ATH_MSG_INFO( "configuring chains: " << m_ntupleChainNames.size() << "\tmonTools: " << m_monTools.size() );
  
  std::string lastvtx = "";

  /// make a copy of the input truth setting job option, so that we can
  /// change it if required

  std::vector<std::string> chains;
  
  chains.reserve( m_monTools.size() );
  
  /// in the tier 0 analysis the wildcard selection should always 
  /// return one and only one chain
  
  std::vector<ToolHandle<GenericMonitoringTool>*> monTools;
  monTools.reserve(m_monTools.size());
  
  ToolHandleArray<GenericMonitoringTool>::iterator toolitr = m_monTools.begin();;


  // sort out all the chain stuff
  
  while ( toolitr!=m_monTools.end() ) {
    
      /// get chain
      ChainString chainName = (toolitr->name());
      
      ATH_MSG_INFO( "configuring chain: " << chainName.head() << "\t: " << chainName.tail() );

      /// do offline type analyses first ...
            
      if ( chainName.head() == "" ) { 
	
	std::string selectChain = chainName.raw();

	/// just get this working - this funtionality isn't really wanted any longer
	
	/// maybe better to set here - leave this in p[lace until we have tried it
	//     toolitr->setPath( m_sliceTag+"/"+chainName );

	if (std::find(chains.begin(), chains.end(), selectChain) == chains.end()) { // deduplicate
  	  chains.push_back( selectChain );
	  monTools.push_back( &(*toolitr) );
	}
      }
      else { 
    
  	/// check for configured chains only ...

	if ( chainName.head().find("HLT_")==std::string::npos ) {
	  // with the O2 optimisation that ATLAS uses, unevaluated pre- and postfix operators produce identical code - I prefer the postfix
	  //cppcheck-suppress postfixOperator 
	  toolitr++;
	  continue;
	}
	
	/// get matching chains
	
	std::string selectChain = chainName.head();
	
	/// for the Run 3 python config based, shoud return one-and-only one chains per item

	ATH_MSG_DEBUG( "checking chain: " << chainName.head() );
	
	if ( selectChain=="" ) { 
	  msg(MSG::WARNING) << "^[[91;1m" << "No chain matched\tchain input " << chainName.head() << "  :  " << chainName.tail() << "^[[m"<< endmsg;
	  ++toolitr;
	  continue;
	}
	  
	std::string mchain = selectChain; //chainName.head();

	if ( chainName.tail()!="" )     mchain += "/"+chainName.tail();
	if ( chainName.roi()!="" )      mchain += "_"+chainName.roi();
	if ( chainName.vtx()!="" )      mchain += "_"+chainName.vtx();
	if ( chainName.element()!="" )  mchain += "_"+chainName.element();
	if ( chainName.extra()!="" )    mchain += "_"+chainName.extra();
	
	selectChain = chainName.subs( selectChain );

	if (  std::find(chains.begin(), chains.end(), selectChain) == chains.end() ) { // deduplicate
	  chains.push_back( selectChain );
	  monTools.push_back( &(*toolitr) );
	}
	     
	++toolitr;
      }

  }
	
  m_chainNames = chains;

  
  //  ATH_MSG_DEBUG( " configured " << m_sequences.size() << " sequences" );
  
  ATH_MSG_DEBUG(  " ----- exit book() ----- " );

  return StatusCode::SUCCESS;
  
}





//StatusCode TrigIDR4Mon::execute() {
StatusCode TrigIDR4Mon::fillHistograms(const EventContext &/*context*/) const {

  ATH_MSG_DEBUG( " ----- enter fill() ----- " );

  const Trig::ChainGroup* chainGroup = m_tdt->getChainGroup( "HLT_e.*" );
  const std::vector<std::string> selectChains = chainGroup->getListOfTriggers();

  /// print out all the configured chains if need be
  static std::once_flag flag;
  std::call_once(flag, [&]() {
    for ( unsigned i=0 ; i<selectChains.size() ; i++ ) {
      ATH_MSG_DEBUG( "\tchain " << selectChains[i] << " from TDT" );
    }
    
    for ( size_t i=selectChains.size() ; i-- ; ) {
      if ( i>5 ) i=5;
      ATH_MSG_INFO( "^[[91;1m" << "configured chain " << selectChains[i] << "^[[m" );
    }
  });
  
  if (msgLvl(MSG::DEBUG)) {
    const std::vector<bool> isPassed = chainGroup->isPassedForEach();
    for ( unsigned i=0 ; i<selectChains.size() ; i++ ) {
      if ( isPassed[i] ) {
        ATH_MSG_DEBUG( "chain " << selectChains[i] << "\tpass: " << isPassed[i] << "\tprescale: " << m_tdt->getPrescale(selectChains[i]) );
      }
    }
  }
  
  //  for ( unsigned i=0 ; i<m_sequences.size() ; i++ ) { 
  //    m_sequences[i]->execute();
  //  }

  ATH_MSG_DEBUG(" ----- exit fill() ----- ");

  return StatusCode::SUCCESS;
}





// bool newEventsBlock, bool newLumiBlock, bool newRun are protected varibales
// correctly set before this is called
StatusCode TrigIDR4Mon::finalize() {

  ATH_MSG_DEBUG( " ====== enter proc() ====== " );

  
  ATH_MSG_DEBUG( " ====== exit proc() ====== " );

  return StatusCode::SUCCESS;
}








