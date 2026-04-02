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
   class StripClusterContainer : public StripClusterContainer_v1 {
   public:
      using base_t = StripClusterContainer_v1;
      using base_t::base_t;
      ModuleIndex<StripClusterContainer> &moduleIndex() { return m_moduleIndex; }
      const ModuleIndex<StripClusterContainer> &moduleIndex() const { return m_moduleIndex; }

      ModuleIndex<StripClusterContainer> m_moduleIndex;
   };
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::StripClusterContainer, 1323510650, 1 )

#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::StripClusterContainer,  xAOD::StripClusterContainer_v1 );

#endif // XAODINDETMEASUREMENT_STRIPCLUSTERCONTAINER_H

