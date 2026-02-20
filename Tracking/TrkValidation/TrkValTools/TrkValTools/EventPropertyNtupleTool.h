/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// EventPropertyNtupleTool.h
//   Header file for EventPropertyNtupleTool
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Wolfgang.Liebig -at- cern.ch
///////////////////////////////////////////////////////////////////

#ifndef TRK_EVENTPROPERTYNTUPLETOOL_H
#define TRK_EVENTPROPERTYNTUPLETOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "GaudiKernel/ToolHandle.h"
#include "TrkValInterfaces/IEventPropertyNtupleTool.h"
#include <vector>
/** @class EventPropertyNtupleTool
    @brief ntuple helper tool to encapsulate filling of general event properties,
           such as event stat, timing, trigger.
    @author Wolfgang.Liebig -at- cern.ch
 */

namespace Trk {

  class EventPropertyNtupleTool : virtual public IEventPropertyNtupleTool, public AthAlgTool {
  public:
    // standard AlgToolmethods
    EventPropertyNtupleTool(const std::string&,const std::string&,const IInterface*);
    ~EventPropertyNtupleTool();

    // standard Athena methods
    StatusCode initialize();
    StatusCode finalize();

    //! method not active in this tool
    virtual void registerTrackCollections( const std::vector<std::string>&, bool){};
    //! method not active in this tool
    virtual void setTrackTreeIndices( unsigned int, int, int){};
    //! method not active in this tool
    virtual void setGenParticleTreeIndices( int, int){};

    /** @brief add branches to the tree
      Should be called once dunring the initialisation phase by the calling algorithm
      (usually Trk::TrackValidationNtupleWriter) */
    virtual StatusCode addNtupleItems ( TTree*, const std::string &);

    //! calculate event-wide data and copy into TTree branches, but don't write the record yet.
    virtual StatusCode fillEventData ( );

    //! reset ntuple variables (mainly for vectors which need to be cleared)
    virtual StatusCode  resetVariables ( );

   //! is True if instance is Tool which links events property to Trk::Tracks
   inline  virtual bool isTrackLinkTool( ) const{ return false;}
    //! is True if instance is Tool which links events property to Rec::TrkParticle
   inline  virtual bool isTrkParticleLinkTool( ) const{return false;}
    //! is True if instance is Tool which links events property to Rec::TrkParticle recieved useing TrigDecTool
   inline  virtual bool isTrkParticleTrigLinkTool( ) const{return false;}
    //! is True if instance is Tool which links events property to Rec::TrkParticle recieved useing TrigDecTool
   inline  virtual bool isInDetTrackTrigLinkTool( ) const{return false;}
    //! is True if instance is EventPropertyTool
   inline  virtual bool isEvtPropertyTool( ) const {return true;}
   
  private:    
    bool m_doTriggerInfo;     //<! job option toggling if to fill trigger bits
    bool m_doTrtPhase;        //<! job option toggling if to fill TRT phase (cosmics)
    std::string m_comTimeName;

    // --- ntuple items --- 
    int               m_runNumber;   //!< run number this event belongs to
    int               m_eventNumber; //!< event number
    float             m_tdc_phase;   //!< event phase in cosmic events (optional) UNIT:nsec

    // more information about level1 trigger 2008 under Atlas.LevelOneCentralTriggerData
    int               m_trigLvl1Type;//!< level 1 trigger type bitted word (2008: equivalent with stream) UNIT:8-bit
    std::vector<unsigned int>* m_trigLvl1TAV; //!< level 1 trigger-after-veto info (256 bits in vector) UNIT:bitted
    int               m_lumiBlock;   //!< lumi block number of current event
    int               m_timeStamp;   //!< time stamp of current event
    int               m_bCID;        //!< bunch crossing ID for current event
  };
}


#endif // TRK_EVENTPROPERTYNTUPLETOOL_H
