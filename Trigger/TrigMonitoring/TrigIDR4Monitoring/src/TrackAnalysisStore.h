/** emacs: this is -*- c++ -*-
 **
 **   @file     TrackAnalysisStore.h
 **
 **   @brief    lightweight class to retrieve and store collections
 **
 **   @author   sutt@cern.ch
 **   @date     Wed 17 Jun 2026 16:19:44 BST
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#ifndef IDTPM2_TRACKANALYSISSTORE_H
#define IDTPM2_TRACKANALYSISSTORE_H

/// Athena includes
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"

/// EDM includes
#include "xAODEventInfo/EventInfo.h"

#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthPileupEventContainer.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

#include "xAODTracking/VertexContainer.h"
#include "xAODTruth/TruthVertexContainer.h"

#include "TruthUtils/HepMCHelpers.h"

#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

#include "TrigInDetAnalysis/TIDAAssociator.h"


#include "TrackParticleTraits.h"
#include "TruthParticleTraits.h"

#include "xAODVertexTraits.h"

#include "TrigInDetAnalysisExample/ChainString.h"

#include "TrackView.h"
#include "TruthView.h"
#include "ParentView.h"
#include "VertexView.h"

#include "TagNProbe.h"

#include "MsgMacros.h"
#include "Utils.h"



/// stl includes
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <utility>

/// class to do all the loading of the collections, can load any collection
/// and can automatically convert any class which has a custom TrackTraits 
/// so it can be used in the analysis

namespace IDTPM2 {

  
class TrackAnalysisStore : public AthMessaging {

public:
  
  using beam_key_t = SG::ReadDecorHandleKeyArray<xAOD::EventInfo>;

public:
  
  /// Constructor 
  TrackAnalysisStore( const std::string& anaTag, const EventContext& context );
  
  TrackAnalysisStore( const std::string& anaTag, const ToolHandle<Trig::TrigDecisionTool>* tdt, const EventContext& context );

  /// Destructor
  ~TrackAnalysisStore() = default;
  
  /// = operator
  TrackAnalysisStore& operator=( const TrackAnalysisStore& ) = delete;
  
  /// initialize
  /// this should not actually be needed - we should be creating a new instance
  /// whenever this is required, so all the "initialisation" could be moved to
  /// the constructor
  StatusCode initialize();
  
  /// --- Setter methods ---

  /// get TrackAnalysis tag
  const std::string& anaTag() { return m_anaTag; }

  const std::vector<ParentView>&    parents() const { return m_parents; }; 
  
  /// test and reference tracks ...

  /// these are all the accessors you actually should ever need ...
  
  /// local matcher ...

  const TIDA::Associator<const TrackView>* matcher() const { return m_matcher; }

  void  set_matcher( const TIDA::Associator<const TrackView>* m ) { m_matcher = m; };
  
  const  std::vector<const TrackView*>&   reference_tracks() const { return m_reftrackptr;  }
  const  std::vector<const TrackView*>&        test_tracks() const { return m_testtrackptr; }

  std::vector<const TrackView*>&   reference_tracks() { return m_reftrackptr;  }
  std::vector<const TrackView*>&        test_tracks() { return m_testtrackptr; }
  

  /// these are a bit dodgy - ie we pass in a vector<const TrackView*> so we want to construct
  /// a reference veiw_store_t ?? this would mean making copies of the original
  /// TrackView objects, so now we would have two, seperate sets of pointers to copies
  /// of the same objects so we don't want this, and so we clear the raw view
  void reference_tracks( const std::vector<const TrackView*>& v ) {  m_reftrackptr = v; m_reftracks.clear();  }
  void      test_tracks( const std::vector<const TrackView*>& v ) { m_testtrackptr = v; m_testtracks.clear(); }

  /// vertex accessors
  
  const std::vector<const VertexView*>&  refvertices() const { return  m_refvertices; }
  const std::vector<const VertexView*>& testvertices() const { return m_testvertices; }

  std::vector<const VertexView*>&  refvertices() { return m_refvertices;  }
  std::vector<const VertexView*>& testvertices() { return m_testvertices; }

  void  set_vertex_matcher( const TIDA::Associator<const VertexView>* m ) { m_vertex_matcher = m; };

  const TIDA::Associator<const VertexView>* vertex_matcher() const { return m_vertex_matcher; }
  
  /// get event info

  const xAOD::EventInfo*                                 eventInfo() const { return m_eventInfo; }
  const xAOD::TruthEventContainer*             truthEventContainer() const { return m_truthEventContainer; }
  const xAOD::TruthPileupEventContainer* truthPileupEventContainer() const { return m_truthPUEventContainer; }

  
  /// odds n ends ...
  
  int  roiIndex() const { return m_roiIndex; }
  
  void roiIndex( int i ) { m_roiIndex = i; } 
  

  /// filling methods to fetch collections ...

  
  template<typename T>
  StatusCode fillInfo( const T*& t, const std::string& key );
  
  template<typename Collection>
  std::pair<typename Collection::const_iterator, typename Collection::const_iterator>
  getPair( const ElementLink<TrigRoiDescriptorCollection>& roi_link,
	   const std::string& key );
  
  StatusCode fillEventInfo( const std::string& key ) { return fillInfo<xAOD::EventInfo>( m_eventInfo, key ); }
  
#if 0
  StatusCode fillEventInfo( const SG::ReadHandleKey<xAOD::EventInfo>&                 eventInfoHandleKey,
			    const SG::ReadHandleKey<xAOD::TruthEventContainer>&       truthEventHandleKey,
			    const SG::ReadHandleKey<xAOD::TruthPileupEventContainer>& truthPUEventHandleKey );
#endif

  
  /// track fillers ...

  void fillViewRef( const std::string& key )  {
    fillView( key, m_reftracks );
    if ( m_parents.size()>0 ) trackptrs( m_parents,   m_reftrackptr );
    else                      trackptrs( m_reftracks, m_reftrackptr );
  }

  void fillViewTest( const std::string& key ) { fillView( key, m_testtracks ); trackptrs( m_testtracks, m_testtrackptr ); }

  /// Roi filler ...

  void fillViewTest( const std::string& key, const ElementLink<TrigRoiDescriptorCollection>& roi_link ) {
    fillView( key, roi_link, m_testtracks ); trackptrs( m_testtracks, m_testtrackptr );
  }

  
  /// vertex fillers ...
  
  void fillVertexViewRef( const std::string& key  ) { fillView( key, m_refvertices_raw ); vertexptrs( m_refvertices_raw, m_refvertices ); }
  
  void fillVertexViewTest( const std::string& key ) { fillView( key, m_testvertices_raw ); vertexptrs( m_testvertices_raw, m_testvertices ); }

  void fillVertexViewTest( const std::string& key, const ElementLink<TrigRoiDescriptorCollection>& roi_link ) {
    fillView( key, roi_link, m_testvertices_raw ); vertexptrs( m_testvertices_raw, m_testvertices );
  }

  void fillEventInfo( const xAOD::EventInfo* e ) { m_eventInfo = e; }
  
  bool  hasTagNProbe() const { return m_tnp!=0; }

  void  addTagNProbe( const TagNProbe* t ) { m_tnp=t; }

  const TagNProbe*  tagNProbe() const { return m_tnp; }

  using keys_t = SG::ReadDecorHandleKeyArray<xAOD::EventInfo>;

  void setBeamKeys( const beam_key_t* k ) { m_beamkeys = k; }
  
  void decoratePerigee();
  
protected:

  /// polymoprphic master store types
  
  using view_store_t = std::vector<std::unique_ptr<TrackView>>;

  using  vtx_store_t = std::vector<std::unique_ptr<VertexView>>;

protected:

  /// track fillers ...

  void fillView( const std::string& key, view_store_t& trackView );

  void fillView( const std::string& key, const ElementLink<TrigRoiDescriptorCollection>& roi_link, view_store_t& trackView );

  /// track vertices ...

  void fillView( const std::string& key, vtx_store_t& vertexView );
  
  /// this and the roi based TrackView fillView should really be done with a single template
  void fillView( const std::string& key, const ElementLink<TrigRoiDescriptorCollection>& roi_link, vtx_store_t& vertexView );

  /// templates for athena type track and vertex retrieval ...
  
  template<typename T, typename U, typename V=U>
  bool fillView_internal( const std::string& key, std::vector<std::unique_ptr<U>>& outView ); 

  
  //  template<typename T, typename U>
  //  bool fillView_internal( const std::string& key, std::vector<U>& outView ); 

  template<typename T>
  bool fillView_internal( const std::string& key, std::vector<ParentView>& parentView ); 


  
  /// have the eventInfo already ?? then just pass in the pointer

  /// more rational generic fill methods 
  template<typename T>
  StatusCode fill( T const*& container, const std::string& key );

  //  void fillroivertices( const ElementLink<TrigRoiDescriptorCollection>& roi_link,
  //			const std::string& key="" ) { fill<VertexView,xAOD::VertexContainer>( m_testvertices, roi_link, key ); }
  
  template<typename T>
  StatusCode fillRoi( std::pair<typename T::const_iterator, typename T::const_iterator>& itrpair,
		      const std::string& key,
		      const ElementLink<TrigRoiDescriptorCollection>& roi_link );

  

  const EventContext& eventContext() const { return m_context; }

  /// do this with a single template for treacks and vertices
  void trackptrs( const view_store_t& v, std::vector<const TrackView*>& vp )  { fill_upointers( vp, v.begin(), v.end() );  }

  void trackptrs( const std::vector<ParentView>& v, std::vector<const TrackView*>& vp ) {
    vp.clear();
    for ( size_t i=0 ; i<v.size() ; i++ ) append_pointers( vp, v[i].begin(), v[i].end() );
  }
  
  void vertexptrs( const vtx_store_t& v, std::vector<const VertexView*>& vp ) { fill_upointers( vp, v.begin(), v.end() );  }

    
  /// to be over ridden by selection for actual EDM classes
  template<typename T>
  bool isGoodOffline( const T& , const std::string& , const std::string& ="" ) = delete;

protected:
  
  /// tracks
  
  view_store_t  m_reftracks;
  view_store_t  m_testtracks;

  std::vector<ParentView> m_parents;

  std::vector<const TrackView*> m_reftrackptr;
  std::vector<const TrackView*> m_testtrackptr; 

  const EventContext& m_context;
  
  /// store the track matches in an actual simple way

  const TIDA::Associator<const TrackView>* m_matcher = 0;

  ///// vertices and vertex analysis stuff ....

  vtx_store_t  m_refvertices_raw;
  vtx_store_t  m_testvertices_raw; 

  std::vector<const VertexView*> m_refvertices;
  std::vector<const VertexView*> m_testvertices;

  /// store the vertex matches in an actual simple way
  /// NB: a vertex match can be quite complex: two vertices can
  ///     have similar z positions, but different tracks

  const TIDA::Associator<const VertexView>* m_vertex_matcher = 0;

  /// Actually, actually, we don;t even need this AT ALL, it serves
  /// no purpose since we can control everything from the main execute()
  /// method as and when required, so this actually serves no real purpose
  /// any longer.
  /// actually, we don't want to use a service handle for this,
  /// we extract it at the tool level, and pass it in when this
  /// tool is constructed for each event We stick with the Svc
  /// name however for the time being to avoid having to change
  /// the code that much
  /// SmartIF< ITrackAnalysisDefinitionSvc > m_trkAnaDefSvc;
  /// const ITrackAnalysisDefinition*        m_trkAnaDefSvc;

  
  /// TrigDecisionTool - we don;t want to retrieve this tool
  /// ourselves, we can have it passed in by the parent tool
  
  const ToolHandle<Trig::TrigDecisionTool>* m_tdt;

  int m_roiIndex;
  
  /// --- Collections class variables ---
  /// EventInfo, TruthEvent, and TruthPUEvent
  const xAOD::EventInfo*                             m_eventInfo = nullptr;
  const xAOD::TruthEventContainer*         m_truthEventContainer = nullptr;
  const xAOD::TruthPileupEventContainer* m_truthPUEventContainer = nullptr;

  
  /// TrackAnalysis properties
  /// why do ALL these classes have independent m_anaTag variables ? this is very
  /// dangerous, as none of them applear to ever be checked - there should be
  /// one analTag for an analysis, and all the tools in the analysis use the
  /// same thing

  const TagNProbe* m_tnp = 0;
  
  std::string m_anaTag;


  /// enable / diable local truth decoration  handling
  
  const beam_key_t* m_beamkeys = 0;

}; // class TrackAnalysisStore

  
  

template<typename T, typename U, typename V >
bool IDTPM2::TrackAnalysisStore::fillView_internal( const std::string& key, std::vector<std::unique_ptr<U>>& outView ) { 
    const T* collection = 0;
    if ( fill( collection, key ).isSuccess() ) {
      if ( collection ) {
	fillv<U,V>( outView, collection->begin(), collection->end() );
	return true;
      }
    }
    return false;
}


  
/// the specialisation for the ParentView, because we want to retrieve the parent, and
/// then get the child tracks to add to the tracks for the analysis, and then add the
/// lookup back to the parent on those tracks  
template<typename T>
bool IDTPM2::TrackAnalysisStore::fillView_internal( const std::string& key, std::vector<ParentView>& parentView ) {
    ChainString cs(key);
    const T* parents = 0;
    if ( fill( parents, cs.head() ).isSuccess() ) {
      if ( parents ) {
	for ( auto p : *parents ) if ( isGoodOffline( *p, cs.value(0), cs.value(1) ) ) parentView.push_back(p);
        for ( size_t i=parentView.size() ; i-- ; ) parentView[i].claimChildren();
	return true;
      }
    }
    return false;
}


  
/// more rational generic fill method 
template<typename T>
StatusCode TrackAnalysisStore::fill( T const*& container, const std::string& key ) {
      MY_MSG_DEBUG << "fill: collection (ordinary fill)" << key << endmsg;
      SG::ReadHandle<T> collection( key, eventContext() );  
      if( !collection.isValid() ) return StatusCode::FAILURE; // MY_MSG_WARNING <<  "Collection " << key << " could not be loaded"  << endmsg;
      container = collection.cptr();
      return StatusCode::SUCCESS;
}

  
  
  
/// more rational generic Roi fill method 
template<typename T>
StatusCode TrackAnalysisStore::fillRoi( std::pair<typename T::const_iterator, typename T::const_iterator>& itrpair,
					const std::string& key,
					const ElementLink<TrigRoiDescriptorCollection>& roi_link ) {
  
      MY_MSG_DEBUG <<  "fill: collection (roi)" << key << endmsg;
      
      SG::ReadHandle<T> collection( key, eventContext() );

      if( !collection.isValid() ) return StatusCode::FAILURE; // MY_MSG_WARNING <<  "Collection " << key << " could not be loaded"  << endmsg;

      if ( m_tdt ) {
	itrpair = (*m_tdt)->associateToEventView( collection, roi_link );
	return StatusCode::SUCCESS;
      }      
      else return StatusCode::FAILURE;
}  

  
template<typename T>
StatusCode TrackAnalysisStore::fillInfo( const T*& t, const std::string& key ) {
  SG::ReadHandle<T> collection( key, eventContext() );
    if ( ! collection.isValid() ) return StatusCode::FAILURE; // MY_MSG_WARNING <<  "Collection " << key << " could not be loaded"  << endmsg;
    else t = collection.ptr();
    return StatusCode::SUCCESS;
}
  



  
template<typename Collection>
std::pair<typename Collection::const_iterator, typename Collection::const_iterator>
TrackAnalysisStore::getPair( const ElementLink<TrigRoiDescriptorCollection>& roi_link,
				   const std::string& key ) {
    
    /// access by a pair of begin and end iterators 
    std::pair< typename Collection::const_iterator,
	       typename Collection::const_iterator > itrpair;

    SG::ReadHandle<Collection> handle( key, eventContext() );

    //    CLID checkCLID;
    //    const std::string* keyStr = m_provider->evtStore()->keyToString(roi_link.key(), checkCLID);
    //    MY_MSG_DEBUG <<  "Requesting range over (" << key << ") associated to ROI from " << (keyStr == nullptr ? "UNKNOWN" : *keyStr)  << endmsg;

    if ( m_tdt ) itrpair = (*m_tdt)->associateToEventView( handle, roi_link );

    return itrpair;
}

  
  
} // namespace IDTPM2

#endif // > !IDTPM2_TRACKANALYSISSTORE_H
