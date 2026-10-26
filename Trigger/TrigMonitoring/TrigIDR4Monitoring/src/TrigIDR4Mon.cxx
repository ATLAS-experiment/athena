/**
 **     @file    TrigIDR4Mon.cxx
 **
 **     @brief   implementation of a AthAlgorithm based monitoring base class
 **
 **     @author  Mark Sutton (sutt@cern.ch)
 **     @date    Tue  29 Sep 2025 09:08:26 GMT
 **
 **     Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#include "TrigInDetAnalysisExample/ChainString.h"
#include "TrigInDetAnalysisExample/TIDAHistogram.h"
#include "TrigDecisionTool/ChainGroup.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

#include "TrigIDR4Mon.h"
#include "Utils.h"

#include "MsgMacros.h"


TrigIDR4Mon::TrigIDR4Mon( const std::string & name, ISvcLocator* pSvcLocator) 
  :  AthMonitorAlgorithm(name, pSvcLocator), 
     m_tdt("Trig::TrigDecisionTool/TrigDecisionTool"),
     m_sliceTag("")
{
# ifdef USE_DATE_TIME_STAMP
  MY_MSG_INFO << "TrigIDR4Mon::TrigIDR4Mon() compiled: " << __DATE__ << " " << __TIME__ << endmsg;
# endif

  declareProperty( "SliceTag",   m_sliceTag = "" );
  
  MY_MSG_DEBUG << "TrigIDR4Mon::TrigIDR4Mon() exiting " << gDirectory->GetName() << endmsg;
  
}



TrigIDR4Mon::~TrigIDR4Mon() { }



StatusCode TrigIDR4Mon::initialize() {

  MY_MSG_DEBUG << " ----- enter initialize() ----- " << endmsg;

  MY_MSG_INFO << "TrigIDR4Mon::initialize() " << gDirectory->GetName() << " " << m_sliceTag << endmsg;

  /// NB: Do NOT create the sequences here - leave it until the book() method, since
  ///     we need to be automatically determine which chains to process, and so need
  ///     the TrigDecisionTool which is niot configured until we have an iov

  MY_MSG_DEBUG << "TrigIDR4Mon::name:      " << name()     << endmsg;
  MY_MSG_DEBUG << "TrigIDR4Mon::SliceTag:  " << m_sliceTag << endmsg;

  ATH_CHECK(m_monTools.retrieve());

  MY_MSG_INFO <<  "TrigIDR4Mon::initialize() retrieved " << m_monTools.size() << " MonTools"  << endmsg;

  ATH_CHECK(m_tdt.retrieve());

  ATH_CHECK(bookHistograms());

  MY_MSG_DEBUG <<  " -----  exit init() ----- "  << endmsg;

  return AthMonitorAlgorithm::initialize();

}


StatusCode TrigIDR4Mon::bookHistograms() {

  MY_MSG_DEBUG <<  "           ----- enter book() ----- "  << endmsg;

  //  MY_MSG_INFO <<  "            TrigIDR4Mon::book() " << gDirectory->GetName()  << endmsg;

  /// useful colour information - leave here ...
  // "^[[91;1m"
  // "^[[m"
 
  MY_MSG_DEBUG <<  "^[[91;1m" << name() << "\t:AnalysisConfig " << "^[[m"  << endmsg;
  
  std::string lastvtx = "";

  /// make a copy of the input truth setting job option, so that we can
  /// change it if required

  std::vector<std::string> chains;
  
  chains.reserve( m_monTools.size() );
  
  /// in the tier 0 analysis the wildcard selection should always 
  /// return one and only one chain

  /// do these need to be tool handles ?? yes ! because they are being allocated
  /// to by the tool handles retrieved from python created tools
  //  std::vector<ToolHandle<GenericMonitoringTool>*> monTools; 
  std::vector<IDTPM2::TrackAnalysis*> monTools; 

  monTools.reserve(m_monTools.size());
  
  ToolHandleArray<IDTPM2::TrackAnalysis>::iterator toolitr = m_monTools.begin();

  std::vector<GenericMonitoringTool*> tools;  // not tool hanldes ?? no, because we are creating these on the fly
  
  // sort out all the chain stuff

  int chaincount = 0;

  
  while ( toolitr!=m_monTools.end() ) {

      std::string toolname = toolitr->name();

      if ( toolname.find(":")==std::string::npos ) toolname = ":"+toolname;
      
      /// get chain
      ChainString chainName = toolname;
      
      /// do offline type analyses first ...
      
      if ( chainName.head() == "" ) { 
	
	std::string selectChain = chainName.raw();

	//     toolitr->setPath( m_sliceTag+"/"+chainName );
	
	/// check this analysis, for this chain, hasn;t been used before ....
	if ( std::find(chains.begin(), chains.end(), selectChain) == chains.end()) { // deduplicate	  
	  MY_MSG_DEBUG <<  "TrigIDR4Mon::bookHistograms() adding offline tool: " << selectChain  << endmsg;
	  chains.push_back( selectChain );
	  monTools.push_back( toolitr->get() );

	  /// add ReadHandleKeys for any offline collections that are needed 
	  if ( (*toolitr)->isInitialized() ) {
	    if ( !(*toolitr)->refTracks().empty()     ) ATH_CHECK( initializeKeys( (*toolitr)->refTracks()     ) );    
	    if ( !(*toolitr)->refTracks_tag().empty() ) ATH_CHECK( initializeKeys( (*toolitr)->refTracks_tag() ) );
	  }
	    
	}
	toolitr++;  //cppcheck-suppress postfixOperator 
      }
      else { 
    
  	/// check for configured chains only ...

	if ( chainName.head().find("HLT_")==std::string::npos && chainName.head().find("EF_")==std::string::npos ) {
	  // with the O2 optimisation that ATLAS uses, unevaluated pre- and postfix operators produce identical code - I prefer the postfix
	  toolitr++;  //cppcheck-suppress postfixOperator 
	  continue;
	}
	
	/// get matching chains
	
	std::string selectChain = chainName.head();
	
	/// for the Run 3 python config based, shoud return one-and-only one chains per item

	MY_MSG_DEBUG <<  "checking chain: " << chainName.head()  << endmsg;
	
	if ( selectChain=="" ) { 
	  MY_MSG_WARNING << "^[[91;1m" << "No chain matched\tchain input " << chainName.head() << "  :  " << chainName.tail() << "^[[m"<< endmsg;
	  // with the O2 optimisation that ATLAS uses, unevaluated pre- and postfix operators produce identical code - I prefer the postfix
	  toolitr++;  //cppcheck-suppress postfixOperator 
	  continue;
	}

	/// check this analysis, with this chain hasn't been booked before ... 
	
	selectChain = chainName.subs( selectChain );
	
	if (  std::find(chains.begin(), chains.end(), selectChain) == chains.end() ) { // deduplicate

	  MY_MSG_DEBUG <<  "TrigIDR4Mon::bookHistograms() adding trigger tool: " << selectChain  << endmsg;

	  chains.push_back( selectChain );

	  //	  (*toolitr)->setTDT( m_tdt );
	  monTools.push_back( toolitr->get() );
	  //  monTools.push_back( new TrackAnalysis( "TrackAnalysis", toolitr->name(), this ) );
	  //  monTools.back()->addHistograms( (*toolitr) );
	  //	  monTools.back()->addHistograms( (*toolitr) );

	  if ( (*toolitr)->isInitialized() ) { 
	    if ( !(*toolitr)->refTracks().empty()     ) ATH_CHECK( initializeKeys( (*toolitr)->refTracks()     ) );
	    if ( !(*toolitr)->refTracks_tag().empty() ) ATH_CHECK( initializeKeys( (*toolitr)->refTracks_tag() ) );
	  }
	  
	  chaincount++;
	  
	}

	// with the O2 optimisation that ATLAS uses, unevaluated pre- and postfix operators produce identical code - I prefer the postfix
	//cppcheck-suppress postfixOperator^
	toolitr++; //cppcheck-suppress postfixOperator 
      }

  }
	
  m_chainNames = chains;

  m_tools = monTools;
  
  MY_MSG_INFO <<  " configured " << chaincount << " (" << m_tools.size() << ") analyses"  << endmsg;
  
  MY_MSG_DEBUG <<   " ----- exit book() ----- "  << endmsg;

  return StatusCode::SUCCESS;
  
}





StatusCode TrigIDR4Mon::execute(const EventContext& context) const {

  MY_MSG_DEBUG <<  " ----- enter fill() ----- "  << endmsg;
  MY_MSG_DEBUG <<  "TrigIDR4Mon::execute() " << IDTPM2::stime() << "\tm_tools.size() = " << m_tools.size()  << endmsg;

  static thread_local long allcounts = 0;

  if ( allcounts && allcounts%1000==0 ) {
    MY_MSG_INFO << "TrigIDR4Mon::execute() " << IDTPM2::stime() << "\tcounts: " << allcounts << "\tm_tools.size() = " << m_tools.size() << endmsg;
  }
  allcounts++;
  
  const Trig::ChainGroup*          chainGroup = m_tdt->getChainGroup( "HLT_.*, EF_.*" );
  const std::vector<std::string> selectChains = chainGroup->getListOfTriggers();
  
  /// three approaches:
  ///   1. loop over all the analyses, get the trigger for each analysis, check whether
  ///      it has passed, and if so call the execute method for the analysis
  ///   2. loop over the chains, if a chain has passed, get back the list of all analyses
  ///      that use that chain, loop over those analyses, calling their execute method
  ///   3. loop over all the analyses, and always call the execute method - this will only work,
  ///      if the analyses themselves perform the check on whether their trigger chain has passed
  ///
  /// So 1. and 3. are essentially the same, they have the same code, but the check that the trigger
  /// chain has passed is moved into the analysis itself.

#if 1
  /// this is method 3.
  for ( size_t itools=m_tools.size() ; itools-- ; ) {
    MY_MSG_DEBUG <<  "TrigIDR4Mon::fillHistograms() calling execute() on tool " << itools << " : " << m_tools[itools]->name()  << endmsg;
    m_tools[itools]->execute(context);
  }
#endif

#if 0
  /// part of method 2.
  
  /// this tests all the chains ...
  
  /// print out all the configured chains if need be
  static std::once_flag flag;
  std::call_once(flag, [&]() {
    for ( unsigned i=0 ; i<selectChains.size() ; i++ ) {
      MY_MSG_DEBUG <<  "\tchain " << selectChains[i] << " from TDT"  << endmsg;
    }
    
    for ( size_t i=selectChains.size() ; i-- ; ) {
      if ( i>5 ) i=5;
      MY_MSG_INFO <<  "^[[91;1m" << "configured chain " << selectChains[i] << "^[[m"  << endmsg;
    }
  });
  
  if (msgLvl(MSG::DEBUG)) {
    const std::vector<bool> isPassed = chainGroup->isPassedForEach();
    for ( unsigned i=0 ; i<selectChains.size() ; i++ ) {
      if ( isPassed[i] ) {
        MY_MSG_DEBUG <<  "chain " << selectChains[i] << "\tpass: " << isPassed[i] << "\tprescale: " << m_tdt->getPrescale(selectChains[i])  << endmsg;
      }
    }
  }
  
#endif
  
  MY_MSG_DEBUG << " ----- exit fill() ----- " << endmsg;

  return StatusCode::SUCCESS;
}





// bool newEventsBlock, bool newLumiBlock, bool newRun are protected varibales
// correctly set before this is called
StatusCode TrigIDR4Mon::finalize() {

  MY_MSG_INFO <<  "finalize() " << name() << endmsg;

  for ( size_t itools=m_tools.size() ; itools-- ; ) {
    MY_MSG_DEBUG <<  "TrigIDR4Mon::finalize() calling finalize() on tool " << itools << " : " << m_tools[itools]->name()  << endmsg;
    if ( (m_tools[itools]->finalize()).isFailure() ) {
      MY_MSG_DEBUG <<  "finalize() failed on tool" << m_tools[itools]->name()  << endmsg;
    }
  }
  
  MY_MSG_DEBUG <<  "finalize() out" << name() << endmsg;
  
  return StatusCode::SUCCESS;
}








