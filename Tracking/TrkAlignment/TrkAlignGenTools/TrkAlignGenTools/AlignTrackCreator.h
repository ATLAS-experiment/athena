/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKALIGNGENTOOLS_ALIGN_TRACK_CREATOR_H
#define TRKALIGNGENTOOLS_ALIGN_TRACK_CREATOR_H

#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "TrkAlignInterfaces/IAlignTrackCreator.h"
#include "TrkAlignInterfaces/IAlignResidualCalculator.h"
#include "TrkAlignInterfaces/IAlignModuleTool.h"

#include <vector>

/**
   @file AlignTrackCreator.h
   @class AlignTrackCreator
   
   @brief Tool used to create an AlignTrack containing all TSOS on a track, including scatterers.

   @author Robert Harrington <roberth@bu.edu>
   @date 1/5/08
*/

class AtlasDetectorID;

namespace Trk {

  class AlignTrack;
  class AlignTSOS;
  class MeasurementTypeID;
  
  class AlignTrackCreator : virtual public IAlignTrackCreator, public AthAlgTool {

  public:
    AlignTrackCreator(const std::string& type, const std::string& name,
		     const IInterface* parent);

    StatusCode initialize();
    StatusCode finalize();

    /** creates AlignTrack containing all TSOS on track */
    bool processAlignTrack(AlignTrack* track);
   
  private:

    // private variables
    ToolHandle<IAlignModuleTool> m_alignModuleTool{this, "AlignModuleTool", ""};
    ToolHandle<IAlignResidualCalculator> m_residualCalculator{
      this, "ResidualCalculator", "Trk::AlignResidualCalculator/ResidualCalculator"};

    const AtlasDetectorID*   m_idHelper = nullptr;
    MeasurementTypeID* m_measTypeIdHelper = nullptr;

    std::vector< std::pair<int,int> > m_goodEventList; //!> good events read in from ASCII file

    StringProperty m_eventListName{this, "EventList", "goodEvents.txt",
      "name of event list ASCII file"};
    BooleanProperty m_writeEventList{this, "WriteEventList", false,
      "write selected events to event list ASCII file"};
    BooleanProperty m_requireOverlap{this, "RequireOverlap", false,
      "keep only tracks that pass through 2 or more AlignModules"};
    BooleanProperty m_removeATSOSNotInAlignModule{
      this, "RemoveATSOSNotInAlignModule", true,
      "remove AlignTSOS not in AlignModules"};
    BooleanProperty m_includeScatterers{this, "IncludeScatterers", true,
      "includes scatterers on track"};

  }; // end class

} // end namespace

#endif // TRKALIGNGENTOOLS_ALIGN_TRACK_CREATOR_H
