/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// IDetachedTrackingVolumeBuilder.h (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRINTERFACES_IDETACHEDTRACKINGVOLUMEBUILDER_H
#define TRKDETDESCRINTERFACES_IDETACHEDTRACKINGVOLUMEBUILDER_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
// STL
#include <vector>

namespace Trk {

  class DetachedTrackingVolume;
 
  /** @class IDetachedTrackingVolumeBuilder
    
    Interface class IDetachedTrackingVolumeBuilder,
    the DetachedTrackingVolumeBuilder inherits from this one.
        
    @author Andreas.Salzburger@cern.ch, Sarka.Todorova@cern.ch
    */
  class IDetachedTrackingVolumeBuilder : virtual public IAlgTool {
    
    public:
    /// Creates the InterfaceID and interfaceID() method
    DeclareInterfaceID(IDetachedTrackingVolumeBuilder, 1, 0);

      /**Virtual destructor*/
      virtual ~IDetachedTrackingVolumeBuilder() = default;

      using DetachedVolumeVec = std::vector<std::unique_ptr<DetachedTrackingVolume>>; 
      virtual DetachedVolumeVec buildDetachedTrackingVolumes(bool blend = false) const = 0;
  };

} // end of namespace


#endif // TRKDETDESCRINTERFACES_IDETACHEDTRACKINGVOLUMEBUILDER_H


