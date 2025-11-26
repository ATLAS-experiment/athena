/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//Author: Lianyou Shan <lianyou.shan@cern.ch>
// -*- c++ -*-
#ifndef INDETSECVTXTRACKSELECTIONTOOL_H
#define INDETSECVTXTRACKSELECTIONTOOL_H

// Local include(s):
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"

// Framework include(s):
#include "AsgTools/AsgTool.h"
// #include "CxxUtils/checker_macros.h" // ATLAS_THREAD_SAFE
#ifndef XAOD_ANALYSIS
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "TrkToolInterfaces/ITrackSummaryTool.h"
#endif

#include <atomic>
#include <limits>
#include <map>
#include <cstdint>
// #include <mutex>

namespace InDet {

  // forward declaration of helper classes
  class SecVtxTrackAccessor;
  class SecVtxTrackCut;

   /// Implementation of the track selector tool
   ///
  class InDetSecVtxTrackSelectionTool :
    public virtual IInDetTrackSelectionTool, 
    public asg::AsgTool {
    
    friend class SecVtxTrackCut;

    /// Create a proper constructor for Athena
    ASG_TOOL_CLASS2( InDetSecVtxTrackSelectionTool,
		     IAsgSelectionTool,
		     InDet::IInDetTrackSelectionTool )
    
  public:
    /// Constructor for standalone usage
    InDetSecVtxTrackSelectionTool( const std::string& name );

    // The default destructor is OK but it must be defined in the
    // implementation file in order to forward declare with unique_ptr
    ~InDetSecVtxTrackSelectionTool();

    /// @name Function(s) implementing the asg::IAsgTool interface
    /// @{
    
    /// Function initialising the tool
    virtual StatusCode initialize() override;
    /// Function finalizing the tool
    virtual StatusCode finalize() override;
    
    /// @}
    
    /// @name Function(s) implementing the IAsgSelectionTool interface
    /// @{
    
    /// Get an object describing the "selection steps" of the tool
    virtual const asg::AcceptInfo& getAcceptInfo() const override;
    
    /// Get the decision using a generic IParticle pointer
    virtual  asg::AcceptData accept( const xAOD::IParticle* ) const override;
    
    /// @}
    
    /// @name Function(s) implementing the IInDetSecVtxTrackSelectionTool interface
    /// @{
    
    /// Get the decision for a specific track object
    virtual  asg::AcceptData accept( const xAOD::TrackParticle& track,
					 const xAOD::Vertex* vertex = nullptr ) const override;

#ifndef XAOD_ANALYSIS
    virtual  asg::AcceptData accept( const Trk::Track& track,
					   const Trk::Vertex* vertex = nullptr) const override;
#endif
    
    /// @}
    virtual void setCutLevel( InDet::CutLevel level, bool overwrite = true ) override
       __attribute__ ((deprecated("For consistency with the athena interface, the cut level is best set through the \"CutLevel\" property.")));

  private:
    bool m_isInitialized = false; // flag whether or not the tool has been initialized, to check erroneous use cases.
    mutable std::atomic<bool> m_warnInit = false; // flag to keep track of whether we have warned about a lack of initialization

    std::unordered_map< std::string, std::shared_ptr<SecVtxTrackAccessor> > m_trackAccessors; //!< list of the accessors that need to be run for each track

    // first element is cut family, second is the set of cuts
    std::map< std::string, std::vector< std::unique_ptr<SecVtxTrackCut> > > m_trackCuts; //!< First element is the name of the cut family, second element is the set of cuts

    mutable std::atomic<uint64_t> m_numTracksProcessed = 0; //!< a counter of the number of tracks proccessed
    mutable std::atomic<uint64_t> m_numTracksPassed = 0; //!< a counter of the number of tracks that passed all cuts
    //    mutable std::vector<uint64_t> m_numTracksPassedCuts ATLAS_THREAD_SAFE; //!< tracks the number of tracks that passed each cut family Guarded by m_mutex
    //    mutable std::mutex m_mutex;

    constexpr static double LOCAL_MAX_DOUBLE = 1.0e16;
    constexpr static int LOCAL_MAX_INT = std::numeric_limits<int>::max();

    /// Object used to store the last decision
    asg::AcceptInfo m_acceptInfo; //!< Object that stores detailed selection information
    
    DoubleProperty m_minD0{this, "minD0", -1., "Minimum |d0| of tracks"};
    IntegerProperty m_NPixel0TRT
      {this, "minNPixelHitsAtZeroTRT", -1, "Minimum number of Pixel hit upon zero TRT hit"};
    IntegerProperty m_minInDetHits
      {this, "minTotalHits", -1, "Minimum number of Pixel + SCT + TRT hits"};

//    ToolHandle< InDet::IInDetTrackSelectionTool > m_trkFilter ;
#ifndef XAOD_ANALYSIS
    bool m_initTrkTools = false; //!< Whether to initialize the Trk::Track tools
    bool m_trackSumToolAvailable = false; //!< Whether the summary tool is available
#endif // XAOD_ANALYSIS

  }; // class InDetSecVtxTrackSelectionTool

} // namespace InDet

#endif // INDETSECVTXTRACKSELECTIONTOOL_INDETSECVTXTRACKSELECTIONTOOL_H
