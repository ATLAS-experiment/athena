/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// EventToTrackLinkNtupleTool.h
//   Header file for EventToTrackLinkNtupleTool
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Wolfgang.Liebig -at- cern.ch
///////////////////////////////////////////////////////////////////

#ifndef TRK_EVENTTRACKLINKNTUPLETOOL_H
#define TRK_EVENTTRACKLINKNTUPLETOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "GaudiKernel/ToolHandle.h"
#include "TrkValInterfaces/IEventPropertyNtupleTool.h"
#include <vector>
class TTree;

/** @class EventToTrackLinkNtupleTool
    @brief document!

    @author Wolfgang.Liebig -at- cern.ch
 */

namespace Trk {

  class EventToTrackLinkNtupleTool : virtual public IEventPropertyNtupleTool, public AthAlgTool {
  public:

    // standard AlgToolmethods
    EventToTrackLinkNtupleTool(const std::string&,const std::string&,const IInterface*);
    ~EventToTrackLinkNtupleTool();

    // standard Athena methods
    StatusCode initialize();
    StatusCode finalize();

    //! see interface for documentation
    virtual void registerTrackCollections( const std::vector<std::string>& collections, bool doTruth);
    //! see interface for documentation
    virtual void setTrackTreeIndices( unsigned int, int, int);
    //! see interface for documentation
    virtual void setGenParticleTreeIndices( int, int);

    /** @brief add branches to the tree
        Should be called once dunring the initialisation phase by the calling algorithm
        (usually Trk::TrackValidationNtupleWriter) */
    virtual StatusCode addNtupleItems ( TTree*, const std::string &);

    //! calculate event-wide data and copy into TTree branches, but don't write the record yet.
    virtual StatusCode fillEventData ( );

    //! reset ntuple variables (mainly for vectors which need to be cleared)
    virtual StatusCode  resetVariables ( );
       
    //! is True if instance is Tool which links events property to Trk::Tracks
   inline  virtual bool isTrackLinkTool( ) const;
    //! is True if instance is Tool which links events property to Rec::TrkParticle
   inline  virtual bool isTrkParticleLinkTool( ) const;
    //! is True if instance is Tool which links events property to Rec::TrkParticle recieved useing TrigDecTool
   inline  virtual bool isTrkParticleTrigLinkTool( ) const;
    //! is True if instance is Tool which links events property to InDetTrack recieved useing TrigDecTool
   inline  virtual bool isInDetTrackTrigLinkTool( ) const;
    //! is True if instance is EventPropertyTool
   inline  virtual bool isEvtPropertyTool( ) const;

  private:
    //! checks if recieved string collectionType corresponds to some collection type names [Trk::Track, Rec::TrackParticle, Rec::TrackParticle_Trig]
    StatusCode checkCollectionType() const;

    std::string                       m_collectionType;
    TTree*                    m_eventLinkTree;         //!< pointer to event-wise ntuple trees (one for all input track collections + truth)
    bool                      m_doTruth;               //!< Switch to turn on / off truth
    std::vector<std::string>  m_trackCollections;      //!< name of the TrackCollections to form tree branch names later

    // --- ntuple items --- 
    std::vector<int>  m_trackIndexBegin;       //!< index-based link from event tree to track tree entry.
    std::vector<int>  m_nTracksPerEvent;       //!< # of tracks per event, to form track loop.
    int               m_genParticleIndexBegin; //!< index-based link from event tree to truth tree entry.
    int               m_nGenParticlesPerEvent; //!< # of true tracks per event, to form truth loop.
    
    static const std::string s_trackTypeName;            //!< denotes instance which deals with Trk::Track
    static const std::string s_trkParticleTypeName;      //!< denotes instance which deals with Rec::TrackParticle
    static const std::string s_trkParticleTrigTypeName;  //!< denotes instance which deals with Rec::TrackParticle obtained as feature using TriDecTool
    static const std::string s_inDetTrackTrigTypeName;   //!< denotes instance which deals with InDetTrack obtained as feature using TriDecTool
  };

}

 
inline bool Trk::EventToTrackLinkNtupleTool::isTrackLinkTool( ) const {
                                                                     if( m_collectionType == s_trackTypeName) return true;
	                					     return false;
                                                                            }
   
inline bool Trk::EventToTrackLinkNtupleTool::isTrkParticleLinkTool( ) const {
                                                                       if( m_collectionType == s_trkParticleTypeName) return true;
								       return false;
                                                                            }
    
inline bool Trk::EventToTrackLinkNtupleTool::isTrkParticleTrigLinkTool( ) const{
                                                                         if( m_collectionType == s_trkParticleTrigTypeName) return true;
									 return false;
                                                                               }

inline bool Trk::EventToTrackLinkNtupleTool::isInDetTrackTrigLinkTool( ) const{
                                                                         if( m_collectionType == s_inDetTrackTrigTypeName) return true;
									 return false;
                                                                               }

inline bool Trk::EventToTrackLinkNtupleTool::isEvtPropertyTool( ) const {
                                                                      return false;
                                                                        } 


#endif // TRK_EVENTTRACKLINKNTUPLETOOL_H
