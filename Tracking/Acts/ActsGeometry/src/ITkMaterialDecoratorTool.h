/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRY_ITkMaterialDecoratorTool_H
#define ACTSGEOMETRY_ITkMaterialDecoratorTool_H

#include "ActsGeometryInterfaces/IRefineTrackingGeoTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Material/TrackingGeometryMaterial.hpp"
#include "ActsPlugins/Root/RootMaterialDecorator.hpp"

namespace ActsTrk {
    /** @brief Mutable tracking geometry visitor to load the material on the tracking surfaces
     *         inside the ITk.
     *         The material map is read from a versioned or legacy Json/Cbor file (optionally zstd compressed), or a Root file,
     *         chosen automatically from the extension of "MaterialDbFile" ('.root' selects the
     *         Root reader, anything else - notably '.json' / '.cbor' - selects the Json reader). */
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
            // ROOT compatibility retains its geometry-ID-only matching semantics.
            std::unique_ptr<ActsPlugins::RootMaterialDecorator> m_rootMaterialDecorator{};
    };
}
#endif
