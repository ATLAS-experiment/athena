/** emacs: this is -*- c++ -*-
 **
 **   @file  TrackSelector.h
 **
 **   @date    Sun 26 Jul 2026 10:22:51 BST
 **
 **   @class TrackSelector
 **   @brief Uses the GenericSelectorTool and the OfflineSelectionTool
 **          and working points defined there for track selection (how 
 **          working points if we have to specify all the parameters?) 
 **
 **  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#ifndef INDETTRACKPERFMON2_IDTPM2_TRACKSELECTOR_H
#define INDETTRACKPERFMON2_IDTPM2_TRACKSELECTOR_H

// Local includes
#include "GenericTrackSelector.h"

// Framework includes
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"

#include <string>

namespace IDTPM2 {


class TrackSelector : public virtual GenericTrackSelector {

public:

  ASG_TOOL_CLASS( TrackSelector, ITrackSelector );

  TrackSelector( const std::string& name );

  virtual StatusCode initialize() override;

  /// just take the baseclass version ...
  ///  virtual std::vector<TrackView> selectTracks( const std::vector<TrackView>& tv ); 
  
  virtual bool selectTrack( const TrackView& t ) const;

private:

  /// how these vector cuts are handled needs to be tidied up - put in to some genertic templates 
  FloatArrayProperty           m_etaBins    { this, "etaBins",    {}, "Eta bins for reco tracks selections" };
  UnsignedIntegerArrayProperty m_minHitsVec { this, "minHitsVec", {}, "Minimum number of Si hits for reco tracks selections" };
  FloatArrayProperty           m_minPtVec   { this, "minPtVec",   {}, "Minimum pt in eta bins for reco tracks selections" };
  FloatArrayProperty           m_maxD0Vec   { this, "maxD0Vec",   {}, "Maximum d0 in eta bins for reco tracks selections" };
  FloatArrayProperty           m_maxZ0Vec   { this, "maxZ0Vec",   {}, "Maximum z0 in eta bins for reco tracks selections" };

  BooleanProperty              m_useOfflineTool { this, "UseOfflineTool",  false, "decide whether to use the offline selection tool" };

  ToolHandle< InDet::IInDetTrackSelectionTool > m_offlineTool { this, "offlineTool", "", "Instance name of track selection tool" };
  
};

} // namespace IDTPM2

#endif // INDETTRACKPERFMON2_IDTPM2_TRACKSELECTOR_H
