/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKALIGNGENALGS_ALIGNALG_H
#define TRKALIGNGENALGS_ALIGNALG_H

#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgorithm.h"

#include "TrkAlignInterfaces/ITrackCollectionProvider.h"
#include "TrkAlignInterfaces/IAlignTrackPreProcessor.h"
#include "TrkAlignInterfaces/IAlignTrackCreator.h"
#include "TrkAlignInterfaces/IAlignTrackDresser.h"
#include "TrkAlignInterfaces/IAlignTool.h"
#include "TrkAlignInterfaces/IGeometryManagerTool.h"
#include "TrkAlignInterfaces/ITrkAlignDBTool.h"
#include "TrkAlignInterfaces/IFillNtupleTool.h"


#include <string>
#include <fstream>

/**
   @file AlignAlg.h
   @class AlignAlg

   @brief This class is the main algorithm for the alignment with tracks. 
   The algorithm is used to align the modules of the Inner Detector, the 
   Muon Spectrometer, or both.

   @author roberth@bu.edu
   @author Daniel Kollar <daniel.kollar@cern.ch>
*/  

namespace Trk {
  
  class IFillNtupleTool;

  class AlignAlg : public AthAlgorithm {
    
  public: 

    /** constructor */
    using AthAlgorithm::AthAlgorithm;

    /** destructor */
    virtual ~AlignAlg();
    
    /** initialize method */
    virtual StatusCode initialize();

    /** set up geometry and prepare the tools */
    virtual StatusCode start();

    /** loops over tracks in event, and accumulates information necessary for alignmnet */
    virtual StatusCode execute();

    /** processes information accumulated in execute method to determine alignment parameters */
    virtual StatusCode stop();

    /** finalize method */
    virtual StatusCode finalize();

    /** dumps statistics accumulated in each event */
    void showStatistics();
    
  private:
    
    ToolHandle <ITrackCollectionProvider> m_trackCollectionProvider{
      this, "TrackCollectionProvider", "Trk::TrackCollectionProvider",
	"tool for getting track collection from StoreGate"};

    /** Pointer to AlignTrackPreProcessor, used to select hits on tracks and/or tracks before passing to AlignTrackCreator */
    ToolHandle<IAlignTrackPreProcessor> m_alignTrackPreProcessor{
      this, "AlignTrackPreProcessor", "Trk::AlignTrackPreProcessor",
      "tool for converting Trk::Track to AlignTrack after processing if necessary"};

    /** Pointer to alignTrackCreator, used to convert Trk::Track to vector of AlignTrack */
    ToolHandle <IAlignTrackCreator>  m_alignTrackCreator{
      this, "AlignTrackCreator", "Trk::AlignTrackCreator",
      "tool for creating AlignTSOSCollection to store on AlignTrack"};
    
    /** Pointer to alignTrackDresser, used to add residuals, derivatives, etc. to vector of AlignTrack */
    ToolHandle <IAlignTrackDresser>  m_alignTrackDresser{
      this, "AlignTrackDresser", "Trk::AlignTrackDresser",
      "tool for dressing AlignTrack with residuals, derivatives, etc."};
    
    /** Pointer to alignTool */
    ToolHandle <IAlignTool>  m_alignTool{
      this, "AlignTool", "Trk::GlobalChi2AlignTool",
      "alignment algorithm-specific tool"};
        
    /** Pointer to GeometryManagerTool, used to get lists of chambers for which alignment parameters will be determined */
    PublicToolHandle <IGeometryManagerTool> m_geometryManagerTool{
      this, "GeometryManagerTool", "InDet::InDetGeometryManagerTool",
      "tool for configuring geometry"};
    
    /** Pointer to TrkAlignDBTool, used for reading/writing alignment parameters from/to the database */
    ToolHandle <ITrkAlignDBTool> m_trkAlignDBTool{
      this, "AlignDBTool", "Trk::TrkAlignDBTool", "tool for handling DB stuff"};
    
    /** Pointer to FillNtupleTool, used to write track information to ntuple */
    ToolHandle <IFillNtupleTool> m_fillNtupleTool{
      this, "FillNtupleTool", "",
      "tool for storing Trk::Track information into the ntuple"};

    // various job options
    StringProperty m_filename{this, "FileName", "Align.root", "name of ntuple file"};
    StringProperty m_filepath{this, "FilePath", "./", "path to ntuple file"};

    BooleanProperty m_solveOnly{this, "SolveOnly", false,
      "only do the solving (accumulate from binaries)"};
    BooleanProperty m_writeNtuple{this, "WriteNtuple", true,
      "write track and event information to ntuple"};

    IntegerProperty m_alignSolveLevel{this, "AlignSolveLevel", 3,
      "Set the Alignment Solve Level"};

    TFile*         m_ntuple = nullptr;        //!< output ntuple
    BooleanProperty m_writeLogfile{this, "WriteLogFile", true,
      "write a logfile for solving"};
    StringProperty m_logfileName{this, "LogFileName", "alignlogfile.txt",
      "name of the logfile"};
    std::ostream * m_logStream = nullptr;     //!< logfile output stream

    int m_nevents = 0;    //!< number of processed events
    int m_ntracks = 0;    //!< number of processed tracks
    int m_ntracksSel = 0; //!< number of selected tracks
    int m_ntracksProc = 0;  //!< number of tracks successfully processed
    int m_ntracksDress = 0; //!< number of tracks successfully dressed
    int m_ntracksAccum = 0; //!< number of tracks successfully accumulated

    StringProperty m_alignTracksName{this, "AlignTracksName", "AlignTracks",
      "name of the AlignTrack collection in the StoreGate"};

   };

} // end namespace

#endif // TRKALIGNGENALGS_ALIGNALG_H
