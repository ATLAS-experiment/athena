/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_GEOMETRYCONTEXT_H
#define ACTSGEOMETRYINTERFACES_GEOMETRYCONTEXT_H

#include <memory>
/// Include the GeoPrimitives which need to be put first
#include "ActsGeometryInterfaces/GeometryDefs.h"
/// If the package is loaded in AthSimulation, the Acts library is not avaialble
#ifndef SIMULATIONBASE 
#   include "Acts/Geometry/GeometryContext.hpp"
#endif

#include "ActsGeometryInterfaces/DetectorAlignStore.h"
#include "GeoModelUtilities/TransientConstSharedPtr.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"

#include "StoreGate/ReadHandleKey.h"

/** @brief ActsGeometry context carries all information related to the aboslute positions of the Readout geometry
 *         Per detector technology (e.g. Mdt), it contains an ActsTrk::DetectorAlignmentStore which carries pointers
 *         to the rigid alignment transfomrations of each FullPhysical volume representing the detector sensor envelope.
 *         Further, it carries the transformations for each tracking layer
 *  
*/
namespace ActsTrk{
class GeometryContext {
public:
    using DetectorType = ActsTrk::DetectorType;
    using AlignmentStore = ActsTrk::DetectorAlignStore;
    using AlignmentStorePtr = GeoModel::TransientConstSharedPtr<AlignmentStore>;
    /** @brief Returns the mutable alignable store for the ATLAS detector type (Pixel, Mdt, etc.) */
    AlignmentStorePtr& getStore(const DetectorType type) { 
        return m_alignmentStores[static_cast<unsigned>(type)]; 
    }
      /** @brief Returns the const alignable store for the ATLAS detector type (Pixel, Mdt, etc.) */
    const AlignmentStorePtr& getStore(const DetectorType type) const {
        return m_alignmentStores[static_cast<unsigned>(type)]; 
    }
    /** @brief Adds the store to the Geometry context */
    void setStore(AlignmentStorePtr store) {
        getStore(store->detType) = store;
    }
#ifndef SIMULATIONBASE 
    Acts::GeometryContext context() const { return Acts::GeometryContext(this); }
#endif

private:
    using SubDetAlignments = std::array<AlignmentStorePtr, static_cast<unsigned>(DetectorType::UnDefined)>;
    SubDetAlignments m_alignmentStores{};
};
}

CLASS_DEF( ActsTrk::GeometryContext , 184998957 , 1 );
CONDCONT_DEF( ActsTrk::GeometryContext , 186862071 );


namespace ActsTrk {
    /** @brief Abrivate the ReadHandleKey to declare the data dependency on the
     *         Geometry context */
    using GeoContextReadKey_t = SG::ReadHandleKey<GeometryContext>;
}


#endif
