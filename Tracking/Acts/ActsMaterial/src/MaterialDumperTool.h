/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMATERIAL_MATERIALDUMPERTOOL_H
#define ACTSMATERIAL_MATERIALDUMPERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "AsgTools/ToolHandleArray.h"
#include "ActsMaterial/IMaterialDumperTool.h"
#include "ActsMaterial/IMaterialWriterTool.h"

namespace ActsTrk {

    /// @brief Dumps the material through the configured material writers
    class MaterialDumperTool : public extends<AthAlgTool, IMaterialDumperTool> {
        public:
            using base_class::base_class;
            virtual ~MaterialDumperTool() = default;
            virtual StatusCode initialize() override;
            virtual void dumpMaterial(const ActsTrk::GeometryContext& gctx,
                                      const Acts::TrackingGeometryMaterial& material) const override;
            virtual void dumpGeometryMaterial(const ActsTrk::GeometryContext& gctx,
                                              const Acts::TrackingGeometry& geometry) const override;
        private:
            /// The material writers the dumped maps are handed to
            ToolHandleArray<IMaterialWriterTool> m_materialMapWriters{this, "MaterialMapWriters", {}, "The material map writers"};
    };

}

#endif
