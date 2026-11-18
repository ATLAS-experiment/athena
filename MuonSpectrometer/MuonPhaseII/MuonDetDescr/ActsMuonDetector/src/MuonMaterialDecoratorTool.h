/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSMUONDETECTOR_MuonMaterialDecoratorTool_H
#define ACTSMUONDETECTOR_MuonMaterialDecoratorTool_H

#include "GeoPrimitives/GeoPrimitives.h"
//
#include "ActsGeometryInterfaces/IRefineTrackingGeoTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Geometry/TrackingVolume.hpp"
#include "Acts/Material/TrackingGeometryMaterial.hpp"

namespace MuonGMR4{
/** @brief Mutable tracking geometry visitor to load the material on the tracking surfaces
 *         inside the Muon System by visiting the tracking volumes of the active chambers 
          and assign material to the portals and by visiting the surfaces to assign the passive material on  */

  class MuonMaterialDecoratorTool  : public extends<AthAlgTool, ActsTrk::IRefineTrackingGeoTool> {
    public:
        using base_class::base_class;
        virtual ~MuonMaterialDecoratorTool();
        virtual void visitSurface(Acts::Surface& surface) override final;
        virtual StatusCode initialize() override final;
        virtual StatusCode finalize() override final;

    private:
            /// Json/Cbor material map, optionally zstd compressed (read with the ACTS Json plugin;
            /// the encoding and the compression are detected from the file contents). Root maps are not supported.
            Gaudi::Property<std::string> m_materialMapFile{this, "MuonMaterialDbFile", "", ""};
            Gaudi::Property<std::string> m_materialMapFolder{this, "MuonMaterialDbFolder", "", ""};
            Acts::TrackingGeometryMaterial m_material;
  };

}
#endif
