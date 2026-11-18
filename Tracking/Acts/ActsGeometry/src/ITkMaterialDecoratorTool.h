/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRY_ITkMaterialDecoratorTool_H
#define ACTSGEOMETRY_ITkMaterialDecoratorTool_H

#include "ActsGeometryInterfaces/IRefineTrackingGeoTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Material/TrackingGeometryMaterial.hpp"

namespace ActsTrk {
    /** @brief Mutable tracking geometry visitor to load the material on the tracking surfaces
     *         inside the ITk.
     *         The material map is read with the ACTS Json plugin from a versioned or legacy Json/Cbor
     *         file, optionally zstd compressed; the encoding and the compression are detected from the
     *         file contents. Root material maps are not supported. */
    class ITkMaterialDecoratorTool : public extends<AthAlgTool, IRefineTrackingGeoTool> {
        public:
            using base_class::base_class;
            virtual ~ITkMaterialDecoratorTool();
            virtual void visitSurface(Acts::Surface& surface) override final;
            virtual StatusCode initialize() override final;
            virtual StatusCode finalize() override final;
        private:
            Gaudi::Property<std::string> m_materialMapFile{this, "MaterialDbFile", "", ""};
            Gaudi::Property<std::string> m_materialMapFolder{this, "MaterialDbFolder", "", ""};
            Acts::TrackingGeometryMaterial m_material;
    };
}
#endif
