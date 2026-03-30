/*
   Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_PIXELCLUSTERCONTAINER_H
#define XAODINDETMEASUREMENT_PIXELCLUSTERCONTAINER_H

#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/versions/PixelClusterContainer_v1.h"
#include "xAODInDetMeasurement/ModuleIndex.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
    /// Define the version of the pixel cluster container
   //    typedef PixelClusterContainer_v1 PixelClusterContainer;
   class PixelClusterContainer : public PixelClusterContainer_v1 {
   public:
      using base_t = PixelClusterContainer_v1;
      using base_t::base_t;
      ModuleIndex<PixelClusterContainer> &moduleIndex() { return m_moduleIndex; }
      const ModuleIndex<PixelClusterContainer> &moduleIndex() const { return m_moduleIndex; }

      ModuleIndex<PixelClusterContainer> m_moduleIndex;
   };
}


// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::PixelClusterContainer, 1318540388, 2 )

#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::PixelClusterContainer,  xAOD::PixelClusterContainer_v1 );

#endif // XAODINDETMEASUREMENT_PIXELCLUSTERCONTAINER_H

