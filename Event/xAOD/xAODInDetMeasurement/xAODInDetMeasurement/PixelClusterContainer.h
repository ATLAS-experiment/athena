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
   // class PixelClusterContainerAlt : public PixelClusterContainer_v1 {
   // public:
   //    using base_t = PixelClusterContainer_v1;
   //    using base_t::base_t;
   //    ModuleIndex<PixelClusterContainerAlt> &moduleIndex() { return m_moduleIndex; }
   //    const ModuleIndex<PixelClusterContainerAlt> &moduleIndex() const { return m_moduleIndex; }

   //    ModuleIndex<PixelClusterContainerAlt> m_moduleIndex;
   // };
   typedef PixelClusterContainer_v1 PixelClusterContainer ;
}


// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::PixelClusterContainer, 1318540388, 1 )
//CLASS_DEF( xAOD::PixelClusterContainerAlt, 1258860834, 2 )

#include "xAODInDetMeasurement/PixelClusterContainerAlt.h"

// #include "xAODCore/BaseInfo.h"
// SG_BASE( xAOD::PixelClusterContainerAlt,  xAOD::PixelClusterContainer_v1 );

#endif // XAODINDETMEASUREMENT_PIXELCLUSTERCONTAINER_H

