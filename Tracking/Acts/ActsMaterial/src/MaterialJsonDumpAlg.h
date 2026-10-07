/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMATERIAL_MATERIALJSONDUMPALG_H
#define ACTSMATERIAL_MATERIALJSONDUMPALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "GaudiKernel/ServiceHandle.h"
#include "AsgTools/ToolHandle.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsMaterial/IMaterialDumperTool.h"

namespace ActsTrk {
    /// @class MaterialJsonDumpAlg
    ///
    /// @brief Dumps the material already assigned to the tracking geometry
    /// (e.g. loaded on the surfaces by the ITkMaterialDecoratorTool from a
    /// Root or Json material map) through the material dumper tool, which is
    /// the same one the material mapping uses at its end.
    ///
    /// No material tracks and no event loop are needed: the material is
    /// collected from the geometry and dumped once, in finalize().
    class MaterialJsonDumpAlg : public AthReentrantAlgorithm {
        public:
            MaterialJsonDumpAlg(const std::string& name, ISvcLocator* pSvcLocator);
            virtual ~MaterialJsonDumpAlg() = default;
            virtual StatusCode initialize() override;
            virtual StatusCode execute(const EventContext& ctx) const override;
            virtual StatusCode finalize() override;

        private:
            /// The material dumper, shared with the material mapping
            ToolHandle<IMaterialDumperTool> m_materialDumper{this, "MaterialDumper", "", "The material dumper tool"};

            /// The tracking geometry service holding the decorated geometry
            ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc", "The ACTS geometry service"};
    };
}

#endif
