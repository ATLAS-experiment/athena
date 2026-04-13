/*
   Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_STRIPCLUSTERCONTAINER_H
#define XAODINDETMEASUREMENT_STRIPCLUSTERCONTAINER_H

#include "xAODInDetMeasurement/StripCluster.h"
#include "xAODInDetMeasurement/versions/StripClusterContainer_v1.h"
#include "xAODInDetMeasurement/ModuleIndex.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
    /// Define the version of the strip cluster container
   class StripClusterContainerAlt : public StripClusterContainer_v1 {
   public:
      using base_t = StripClusterContainer_v1;
      using base_t::base_t;
      ModuleIndex<StripClusterContainerAlt> &moduleIndex() { return m_moduleIndex; }
      const ModuleIndex<StripClusterContainerAlt> &moduleIndex() const { return m_moduleIndex; }

      ModuleIndex<StripClusterContainerAlt> m_moduleIndex;
   };
   typedef StripClusterContainer_v1 StripClusterContainer;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::StripClusterContainer, 1323510650, 1 )
CLASS_DEF( xAOD::StripClusterContainerAlt, 1276663924, 1 )

#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::StripClusterContainerAlt,  xAOD::StripClusterContainer_v1 );

#endif // XAODINDETMEASUREMENT_STRIPCLUSTERCONTAINER_H

