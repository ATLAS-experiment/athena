/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// IDetachedTrackingVolumeBuilderCond.h (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRINTERFACES_IDETACHEDTRACKINGVOLUMEBUILDERCOND_H
#define TRKDETDESCRINTERFACES_IDETACHEDTRACKINGVOLUMEBUILDERCOND_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/EventIDRange.h"
#include "StoreGate/WriteCondHandle.h"
// STL
#include <vector>

namespace Trk {

  class DetachedTrackingVolume;
  class TrackingGeometry;
 
  /** @class IDetachedTrackingVolumeBuilderCond
    
    Interface class IDetachedTrackingVolumeBuilderCond,
    the DetachedTrackingVolumeBuilder inherits from this one.
        
    @author Andreas.Salzburger@cern.ch, Sarka.Todorova@cern.ch
    */
  class IDetachedTrackingVolumeBuilderCond : virtual public IAlgTool {
    
    public:
    /// Creates the InterfaceID and interfaceID() method
    DeclareInterfaceID(IDetachedTrackingVolumeBuilderCond, 1, 0);

      /**Virtual destructor*/
      virtual ~IDetachedTrackingVolumeBuilderCond() = default;

      using DetachedVolumeVec = std::vector<std::unique_ptr<DetachedTrackingVolume>>; 
      
      virtual DetachedVolumeVec 
              buildDetachedTrackingVolumes(const EventContext& ctx,
                                           SG::WriteCondHandle<TrackingGeometry>& whandle,
                                           bool blend = false) const = 0 ;
    
  };

} // end of namespace


#endif // TRKDETDESCRINTERFACES_IDETACHEDTRACKINGVOLUMEBUILDERCOND_H


