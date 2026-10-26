/**
 **   @file    TrackAnalysis.cxx         
 ** 
 **   @author sutt
 **   @date   Mon 17 Nov 2025 21:09:16 GMT
 **
 **   $Id: TrackAnalysis.cxx, v0.0   Mon 17 Nov 2025 21:09:16 GMT sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#include "TrigInDetAnalysisExample/ChainString.h"

#include "xAODEventInfo/EventInfo.h"

#include "TrackAnalysisStore.h"

#include "MsgMacros.h"

#include "TrackView.h"
#include "VertexView.h"


#include "TrackAnalysis.h"
#include "TBestMatch.h"

#include "TrackRoiSelector.h"

#include "TagNProbe.h"

#include <algorithm>
#include <limits>
#include <cmath>
#include <utility>
#include <cstdlib> 


///----------------------------------------
///------- Parametrized constructor -------
///----------------------------------------
IDTPM2::TrackAnalysis::TrackAnalysis( const std::string& type, const std::string& name, const IInterface* parent ) :
  AthAlgTool( type, name, parent ),
  m_tdt("Trig::TrigDecisionTool/TrigDecisionTool"),
  m_analysis(std::make_unique<AnalysisR4>(name)) 
{ }


 


///----------------------------------
///------- Default destructor -------
///----------------------------------
IDTPM2::TrackAnalysis::~TrackAnalysis() = default;


///--------------------------
///------- Initialize -------
///--------------------------
StatusCode IDTPM2::TrackAnalysis::initialize() {

  MY_MSG_INFO <<  "TrackAnalysis::initialize() name: " << name() << endmsg;

  std::string selectChain = name().substr(name().find(".")+1,name().size());

  if ( selectChain.find(":")==std::string::npos ) selectChain = ":"+selectChain;
  
  ChainString chainName = selectChain; //chainName.head();

  /// do we need these tests ?? if the variable is empty, then the flag
  /// would just be set to empty anyhow
  if ( chainName.head()!="" )     m_triggerchain = chainName.head();
  if ( chainName.tail()!="" )     m_testTracks   = chainName.tail();
  if ( chainName.roi()!="" )      m_rois         = chainName.roi();
  if ( chainName.vtx()!="" )      m_vertices     = chainName.vtx();
  if ( chainName.element()!="" )  m_leg          = std::atoi(chainName.element().c_str());
  if ( chainName.extra()!="" )    m_extra        = chainName.extra();

  /// we want to set these in the initialise
  /// rather than set them for every event
  m_refTracks   = m_reference;

  m_refVertices = m_referenceVertices;

  /// check tag n probe information ...

  MY_MSG_INFO << "analysis:   "  << name() << endmsg;
  MY_MSG_INFO << "trigger:    "  << m_triggerchain << endmsg;
  MY_MSG_INFO << "reference:  "  << m_refTracks << ( m_refTracks_tag.empty() ? "" : "\t"+ m_refTracks_tag ) << endmsg;
  MY_MSG_INFO << "test:       "  << m_testTracks << endmsg;
  if ( !m_vertices.empty() ) { MY_MSG_INFO << "vtx:       " << name() << endmsg; }
  MY_MSG_INFO << endmsg;

  if ( !m_tagChain.empty() ) {
    
    ChainString tagchain = m_tagChain.value();

    if ( m_reference_tag.empty() ) m_refTracks_tag = m_refTracks;
    else                           m_refTracks_tag = m_reference_tag;
        
    MY_MSG_INFO << "Tag and probe: " << tagchain << endmsg;
    
    MY_MSG_INFO << "Tag chain: " << tagchain.head()
		<< "\t" << tagchain.roi()
		<< "\t" << tagchain.element()
		<< "\t" << tagchain.extra()
		<< "\treferences: " << m_reference << "\t" << m_reference_tag 
		<< endmsg;

    m_triggerchain_tag = tagchain.head();
    m_roi_tag          = tagchain.roi();
    m_extra_tag        = tagchain.extra();
    if ( !tagchain.element().empty() ) m_leg_tag = std::atoi(tagchain.element().c_str());

    using PtPdgHelper = PtPdgIdHelper<TrackView,xAOD::TruthParticle>;
    
    if ( m_refPdgId_tag.value() ) m_tnpPredicate = std::make_unique<PtPdgHelper>( m_minTagPT.value(), m_refPdgId_tag.value() ); 
    else                          m_tnpPredicate = std::make_unique<PtHelper<TrackView>>( m_minTagPT.value() ); 

    MY_MSG_INFO << "TagNProbe: " << m_triggerchain << " -> " << m_triggerchain_tag << endmsg;   
    
  }

  
  MY_MSG_DEBUG << "TrackAnalysis::initialize()"
	       << " trigger="    << m_triggerchain
	       << " testTracks=" << m_testTracks
	       << " refTracks="  << m_refTracks
	       << " rois="       << m_rois << endmsg;
  
  ATH_CHECK( m_eventInfoName.initialize() );

  ATH_CHECK( m_beamKeys.initialize() );

  ATH_CHECK( m_refSelector.retrieve() );
  ATH_CHECK( m_testSelector.retrieve() );

  //  ATH_CHECK( m_vertexQualitySelectionTool.retrieve() );

  ATH_CHECK( m_trackMatchingTool.retrieve() );

#if 0
  /// none of this is needed any longer 
  std::unique_ptr<TrackAnalysisDefinition> config = std::make_unique<TrackAnalysisDefinition>();

  config->setTestCollection( m_testTracks );
  config->setReferenceCollection( m_refTracks );

  config->setTestType( "Trigger" );
  config->setReferenceType( "Offline" );

  /// these useTrigger/Offline etc flags are redundant, and should all
  /// be inferred from the track collection names, although they are
  /// not needed at all, the only thing needed is to workout what sort
  /// of tracks they are and then call the ReadHandle with th relevant
  /// type 
  config->setUseTrigger( true );
  config->setUseOffline( true );
  config->setDoTrigNavigation( true );

  /// this should be safe, even though we are taking a pointer
  /// to an automatic object that will go out of scope when this
  /// execute method returns, because we want the entire processing 
  /// of this event to take place from within this function
  m_trkAnaDef = std::move( config );

#endif 
  
  //  m_trackQualitySelectionTool->setAnaDef(m_trkAnaDef.get());

  MY_MSG_DEBUG << "TrackAnalysis::initialize() exitting" << endmsg;

  m_analysis->set_monTool( &m_tool );

  for ( size_t i=m_fillers.size() ; i-- ; ) m_analysis->addFillTool( m_fillers[i] );
      
  m_analysis->initialise();

  m_initialized = true;
  
  return StatusCode::SUCCESS;

}



///------------------------------
///------- bookHistograms -------
///------------------------------
StatusCode IDTPM2::TrackAnalysis::bookHistograms()
{
  return StatusCode::SUCCESS;
}


/// ------------------------------
/// ------- fillHistograms -------
/// ------------------------------
StatusCode IDTPM2::TrackAnalysis::fillHistograms( const EventContext& context ) const { 
  if ( execute( context ) )  return StatusCode::SUCCESS;
  return StatusCode::FAILURE;
}




void printtracks( const std::vector<const TrackView*>& ars, const std::string& sht=""  ) {
  std::cout << "\nSUTT printtracks: " << sht << "\tsize: " << ars.size() << std::endl;
  for ( size_t i=0 ; i<ars.size() && i<10 ; i++ ) std::cout << "\t" << i << "\t" << *ars[i] << std::endl;
  std::cout << std::endl;
}


template<typename T>
std::ostream& operator<<( std::ostream& s, std::vector<T>& v ) {
  for ( size_t i=0 ; i<v.size() ; i++ ) s << "\t" << v[i];
  return s;
}


#include <fenv.h>

bool IDTPM2::TrackAnalysis::execute( const EventContext& context ) const {

  feclearexcept(FE_ALL_EXCEPT);
  fedisableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
 
  MY_MSG_DEBUG << "Filling hists " << name() << "\ttrigger: " << m_triggerchain << " ..." << endmsg;
  
  unsigned decisionType = TrigDefs::Physics; // TrigDefs::includeFailedDecisions;

  if( !m_triggerchain.empty() && !m_tdt->isPassed( m_triggerchain, decisionType ) ) return true;

  MY_MSG_DEBUG <<  "[91;1m PROCESSING ------------------------------------------------------------------------------------[m" << endmsg;
  
  MY_MSG_DEBUG << "TrackAnalysis::execute() " << name()
	       << " refTracks:     " << m_refTracks
	       << "\t testTracks:  " << m_testTracks
	       << "\ttriggerchain: " << m_triggerchain << endmsg;
  

#if 0															    
  if ( m_testTracks.find("HLT_")==std::string::npos && m_testTracks.find("EF_")==std::string::npos ) {
    
    const std::vector<std::string> configuredChains = m_tdt->getListOfTriggers("HLT_.*, EF_.*");

    for ( unsigned i=0 ; i<configuredChains.size() ; i++ ) {
      if( m_tdt->isPassed( configuredChains[i], decisionType ) ) std::cout << i << "\t" << "[91;1m" << "Chain " << configuredChains[i] << " pass   (ACN)[m" << std::endl;
      if ( configuredChains[i] == m_triggerchain ) std::cout << "Mutha ..." << configuredChains[i] << " " <<  m_triggerchain << "\n\n" << std::endl;
    }
    
    MY_MSG_INFO <<  "[91;1m" << configuredChains.size() << " Configured Chains" << "[m"
		 <<"\ttrigger chain: " << m_triggerchain << " :: "
		 << m_tdt->isPassed( m_triggerchain, decisionType ) << endmsg;
  }
#endif
  
  IDTPM2::TrackAnalysisStore trackAnaStore( "_duff", context );

  if ( !m_useDecorations ) trackAnaStore.setBeamKeys( &m_beamKeys );
    
  /// actaully, the offline store does not need the tdt
  //  trackAnaStore.setTDT( &m_tdt );

  RoiSelector roiSelector(&m_tdt);

  trackAnaStore.fillViewRef( m_refTracks );

  //  printtracks( trackAnaStore.reference_tracks(), "\ttrackAnaStore: reference" );  
  
  /// only retrieve the global collection if not a trigger analysis ...

  bool isTrigger = m_testTracks.find("HLT_")!=std::string::npos || m_testTracks.find("EF_")!=std::string::npos; 
  
  if ( !isTrigger ) trackAnaStore.fillViewTest( m_testTracks );

  /// ----------------------------------
  /// ------ RoI getter/selector -------
  /// ----------------------------------
  std::vector<TrigCompositeUtils::LinkInfo<TrigRoiDescriptorCollection> > selectedRois; 

  /// would likely never be set = proably be best localising all TagNProbe
  /// information into the TagNProbe class
  std::vector<TrigCompositeUtils::LinkInfo<TrigRoiDescriptorCollection> > tagRois;  
  
  /// this isn't the way to do it. If you want to use a dummy Roi then add the dummy Roi 
  /// to the Roi vector !!!
  size_t selectedRoisSize = 1; // by default only one "dummy" RoI, i.e. for offline analysis 

  /// actually, we shouldn't need to bother with testing for the "Trigger Navigation"
  /// we should only need to check whether we are requesting a trigger collection / chain
  
  /// doesn't need any configuration - could for instance, get this
  /// to be a track or vertex filter, rather than just for tracks
  TrackRoiSelector roiFilter;

  std::unique_ptr<TagNProbe> tnp = 0;

  /// need to make this mass range, and the pt range, configurable parameters
  /// add to the chain string ? or as normal job properties ?? if normal
  /// then would have to run different types - Z, J/psi etc as different instances,
  /// properties, maybe allow both ?

  if ( !m_tagChain.empty() ) tnp = std::make_unique<TagNProbe>( m_minTagMass, m_maxTagMass );

  /// why check if the Navigation is required when you can just check the trigger chain
  ///  if ( m_trkAnaDef->doTrigNavigation() ) {
  /// actually, the logic is the other way round - if it is a trigger collection
  /// *then* we will need tha navigation
  if ( !m_triggerchain.empty() ) { 
    selectedRois     = roiSelector.retrieveRois( m_triggerchain, m_rois ); 
    selectedRoisSize = selectedRois.size();
  }

  /// at this point, we have the reference objects, so for a tag and probe analysis
  /// with the same reference type - eg electron, mu etc all we need now, if to filter
  /// these to the relevant Rois and store for later use in the selection

  /// make this a unique_ptr so that we only need to actually construct the store
  /// if we are doing a tag n probe analysis with different references
  std::unique_ptr<TrackAnalysisStore> tagStore = 0;

  if ( tnp ) {

      /// get the tag objects for the tag aqnd probe analysis ...

      /// this is needed in case the tag and probe types are not the same in each leg ...
      const std::vector<const TrackView*>*  vref_ptr;

      MY_MSG_DEBUG << "reftracks: " << m_refTracks << "\t:" << m_refTracks_tag << endmsg;
      
      if ( m_refTracks!=m_refTracks_tag ) {
	/// yep, different legs here ...
        tagStore = std::make_unique<TrackAnalysisStore>( "tagstore", context );
	tagStore->fillViewRef( m_refTracks_tag );
	vref_ptr = &tagStore->reference_tracks();
      }
      else {
        vref_ptr = &trackAnaStore.reference_tracks();
      }

      const std::vector<const TrackView*>& vref = *vref_ptr;
      
      tagRois = roiSelector.retrieveRois( m_triggerchain_tag, m_roi_tag, m_leg_tag );

      for ( auto roi : tagRois ) {
        ElementLink<TrigRoiDescriptorCollection> roi_link = roi.link;
        if( ! roi_link.isValid() ) continue;
	std::vector<const TrackView*> v = roiFilter.getTracks( vref, *roi_link.cptr() );
	/// note we can do some basic selection of the tags with the predicate
	tnp->append_tags( v.begin(), v.end(), *m_tnpPredicate.get() );
      }

      MY_MSG_DEBUG << "tnp size() : " << tnp->size() << "\t" << vref.size() << endmsg;

#if 0
      /// check that the tag and probe code actually works ... 
      const std::vector<const TrackView*>& v = trackAnaStore.reference_tracks();
      for ( size_t i=0 ; i<v.size() ; i++ ) {
	bool ok = tnp->select( v[i] );
	std::vector<double> m = tnp->masses( v[i] );
	std::cout << "\ttnp: " << ok << "\tm: " << m.size() << m << "\t " << *v[i] << std::endl;
      }
#endif  

  }
  
  /// So we are using three TrackAnalysisCollections objets ...
  /// - the "Raw" object for the "full" collections,
  /// - the roiStore       for the Roiselected collections, and
  /// - the selectedStore  for the tracks in the rois, after selection
  /// - since it is the selectedStore which is passed into the
  ///   histogram filling, it is that one where we store the
  ///   EventInfo
  ///   NB: we should set a flag for the "first" Roi in that
  ///       store so we can count events etc

  TrackAnalysisStore      roiStore(      "roi", &m_tdt, context );  /// don't actually need these to be named
  TrackAnalysisStore selectedStore( "selected", &m_tdt, context );

  /// no point doin a vertex analysis, if no test, or reefernce vertex has been specified
  if ( !m_vertices.empty() && !m_refVertices.empty() ) selectedStore.fillVertexViewRef( m_refVertices );

  /// this stuff is still being developed and tested, so we need all this left in here ...

  //  std::cout << "TrackAnalysis: ref vtx size: " << selectedStore.refvertices().size() << std::endl;

  //  const std::vector<const VertexView*> vtx = selectedStore.refvertices();
  
  //  for ( int iv=vtx.size() ; iv-- ; ) {
  //   if ( vtx[iv]->vertexType()!=0 ) std::cout << "\t" << iv << "\tvtx[" << iv << "]: " << *vtx[iv] << std::endl;
  //  }
	  
  /// at the moment - for the sake of argument, just assume that the vertices always
  /// live in the Roi - need to fix that for the tau vertices, but for the FS tracking
  /// that will be ok

  
  /// get the Event info for this event ...
  /// NB: only get the EventInfo fir the selectedStore, since
  ///     that is the one which is run over by the analyses
  if ( selectedStore.fillEventInfo( m_eventInfoName.key() ).isFailure() ) return false;

  MY_MSG_DEBUG << "Processing event = " << selectedStore.eventInfo()->eventNumber() << endmsg;

  for( size_t ir=selectedRoisSize ; ir-- ;  ) {

    MY_MSG_DEBUG << "\tRoi: " << ir << endmsg; 
    
    if( !m_triggerchain.empty() ) { 

        /// store the roi index number for the analysis ...
        selectedStore.roiIndex(ir);
      
        /// Getting RoI ElementLink
        ElementLink<TrigRoiDescriptorCollection> roi_link = selectedRois[ir].link;

        /// skip non-valid RoI link
        if( ! roi_link.isValid() ) {
          MY_MSG_WARNING << "Found non-valid RoI ElementLink" << endmsg;
          continue;
        }

	const TrigRoiDescriptor* const* roiptr = roi_link.cptr();

	if ( !roiptr ) {
	  MY_MSG_WARNING << "could not fetch RoiDescriptor: " << m_rois << endmsg;
	  continue;
	}

	/// get the test tracks for this Roi 
	if ( isTrigger ) { 

	  roiStore.fillViewTest( m_testTracks, roi_link  );

	  /// FIXME: this is a bit of a hack - it just adds the vertices to the selected store
	  ///        and really we need to add them to the Roi store, and then implement a
	  ///        vertex selector
	  if ( !m_vertices.empty() ) selectedStore.fillVertexViewTest( m_vertices, roi_link );

	  /// again, being used for vertex analysis development ...
	  //	  const std::vector<const VertexView*> vtx = selectedStore.testvertices();
  
	  //	  for ( int iv=vtx.size() ; iv-- ; ) {
	  //         std::cout << "\t\t" << iv << "\tvtx[" << iv << "]: " << *vtx[iv] << std::endl;
	  //      }
	  
	  //	  printtracks( roiStore.test_tracks(),            "\nroiStore: test tracks" );
	  //	  if ( !m_vertices.empty() ) fillroivertices( roi_link, m_vertices );
	  	    
	  //	    selectedStore.fill<VertexView,xAOD::VertexContainer>( selectedStore.testvertices(), 
	  //								  roi_link, m_vertices );

	}

	/// dbg ...
	//	printtracks( trackAnaStore.test_tracks(), "test (should be ok - roi retrieve II)" );  
	  
	/// NB: in principle, it may be more efficient to use an interface such as
	///         roiFilter.getTracks( const vector<TrackView*>& in, RoiDesciptor* roi, vector<TrackView*>& out );
	///     and then directly clear, and fill the output collection inside the selector
	///     the current implementation has the advantage that the internal collection is
	///     local so can be better optimised, but passing in the collection would mean
	///     that we can make use of the prior space allocated for the collection
	
        roiStore.reference_tracks() = roiFilter.getTracks( trackAnaStore.reference_tracks(), *roiptr );

	///	printtracks( roiStore.reference_tracks(), " filtered reference tracks" );

    }
    else {
      /// maybe we can do something here to avoid having to copy the container
      roiStore.reference_tracks() = trackAnaStore.reference_tracks();
      roiStore.test_tracks()      = trackAnaStore.test_tracks();
    }

    //    printtracks( roiStore.reference_tracks(), "\troiStore: reference" );  
    //    printtracks( roiStore.test_tracks(), "\troiStore: test" );
    
    MY_MSG_DEBUG << "SUTT: TrackSelection ..." << endmsg;

    selectedStore.reference_tracks() = m_refSelector->selectTracks( roiStore.reference_tracks(), tnp.get() );

    //    printtracks( selectedStore.reference_tracks(), "\tselectedStore: reference" );  
    
    /// select the trigger tracks ... 
    selectedStore.test_tracks() = m_testSelector->selectTracks( roiStore.test_tracks() );

    //    printtracks( selectedStore.test_tracks(), "\tselectedStore: test" );
    
    /// store this mastcher access in the tool,     
    // TDeltaRMatcher<const TrackView> matcher( "DeltaRMatcher", 0.1 ); /// FIXME: turn this hard coded parameter in to a job Property

    std::unique_ptr<TIDA::Associator<const TrackView> > matcher = m_trackMatchingTool->uclone();
    
    //    MY_MSG_DEBUG << "n matched: " << matcher->size() << endmsg; 

    /// for Harry's matcher tool
    //	matchertype mt = matcher.uclone(); 
    matcher->match( selectedStore.reference_tracks(), selectedStore.test_tracks() );
    
    // matcher.match( tref, ttest );
    
    //    MY_MSG_DEBUG <<  "SUTT: n matched: " << matcher->size() << endmsg;
    
    selectedStore.set_matcher( matcher.get() );

    TDeltaZMatcher<const VertexView> vertex_matcher( "DeltaZMatcher", 1.0 ); /// vertex

    if ( !m_vertices.empty() ) {
      vertex_matcher.match( selectedStore.refvertices(), selectedStore.testvertices() );
      selectedStore.set_vertex_matcher( &vertex_matcher );
    }

    if ( tnp ) selectedStore.addTagNProbe( tnp.get() );
    
    /// do we want to transfer this using unique_ptr ??
    //    selectedStore.refMap( trackAnaStore );
  
    /// rather than passing this in, perhaps better to simply use the
    /// collections themselves

    m_analysis->execute( selectedStore );

  }

  return true;
  
}


///------------------------------
///------- procHistograms -------
///------------------------------
StatusCode IDTPM2::TrackAnalysis::proc() {

  MY_MSG_DEBUG << "Finalising plots" << endmsg;

  /// do we need the end of run protection ??
  // if( endOfRunFlag() ) m_analysis->finalize()

  m_analysis->finalize();
  
  MY_MSG_DEBUG << "Successfully finalized hists" << endmsg;

  return StatusCode::SUCCESS;
}


