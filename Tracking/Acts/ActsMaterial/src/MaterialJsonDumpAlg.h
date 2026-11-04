/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMATERIAL_MATERIALJSONDUMPALG_H
#define ACTSMATERIAL_MATERIALJSONDUMPALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "GaudiKernel/ServiceHandle.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"

namespace ActsTrk {
    /// @class MaterialJsonDumpAlg
    ///
    /// @brief Dumps the material already assigned to the tracking geometry
    /// (e.g. loaded on the surfaces by the ITkMaterialDecoratorTool from a
    /// Root or Json material map) through the ACTS versioned material JSON codec.
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
            Gaudi::Property<std::string> m_fileName{this, "FileName", "material-maps.json.zst", "Material map output filename"};
            Gaudi::Property<int> m_compressionLevel{this, "CompressionLevel", 9, "Zstd compression level"};
            Gaudi::Property<int> m_materialFractionBits{this, "MaterialFractionBits", 23, "Retained material float fraction bits (0-23; 23 is lossless)"};

            /// The tracking geometry service holding the decorated geometry
            ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc", "The ACTS geometry service"};
    };
}

#endif
