/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMATERIAL_JSONMATERIALWRITERTOOL_H
#define ACTSMATERIAL_JSONMATERIALWRITERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsMaterial/IMaterialWriterTool.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "Acts/Material/TrackingGeometryMaterial.hpp"

namespace ActsTrk {

    /// @brief Material writer dumping the tracking geometry material map to Json/Cbor
    ///
    /// This uses the ACTS Json plugin (Acts::MaterialMapJsonConverter) to serialise the
    /// surface and volume material maps into a Json (and/or Cbor) file. The resulting
    /// file can be read back in e.g. by Acts::JsonMaterialDecorator.
    class JsonMaterialWriterTool : public extends<AthAlgTool, IMaterialWriterTool> {
        public:
            using base_class::base_class;
            ~JsonMaterialWriterTool() override = default;
            virtual void writeMaterial(const ActsTrk::GeometryContext& gctx,
                                       const Acts::TrackingGeometryMaterial& detMaterial) const override;
        private:
            /// The base name of the output file (without extension)
            Gaudi::Property<std::string> m_fileName{this, "FileName", "material-maps", "Output file base name for the Json/Cbor Material Map"};
            /// Write out the map in Json format
            Gaudi::Property<bool> m_writeJson{this, "WriteJson", true, "Write the material map in Json format"};
            /// Write out the map in Cbor format
            Gaudi::Property<bool> m_writeCbor{this, "WriteCbor", false, "Write the material map in Cbor format"};
    };

}

#endif
