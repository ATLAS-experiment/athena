/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMATERIAL_MATERIALMAPPING_H
#define ACTSMATERIAL_MATERIALMAPPING_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsGeometry/RecordedMaterialTrackCollection.h"
#include "Acts/Material/MaterialMapper.hpp"

namespace ActsTrk {
    /// @class MaterialMapping
    ///
    /// @brief Reads the MaterialTracks and produces the material maps
    /// for the Tracking Geometry.
    /// The output map is stored with the ACTS versioned material JSON codec.

    class MaterialMapping : public AthReentrantAlgorithm  {
        public:
            MaterialMapping(const std::string &name, ISvcLocator *pSvcLocator);
            virtual StatusCode initialize() override;
            virtual StatusCode execute (const EventContext& ctx) const override;
            virtual StatusCode finalize() override;
            virtual ~MaterialMapping();

        private:

            Gaudi::Property<std::string> m_fileName{this, "FileName", "material-maps.json.zst", "Material map output filename"};
            Gaudi::Property<int> m_compressionLevel{this, "CompressionLevel", 9, "Zstd compression level"};
            Gaudi::Property<int> m_materialFractionBits{this, "MaterialFractionBits", 23, "Retained material float fraction bits (0-23; 23 is lossless)"};

            /// The material mapper from the ACTS core components
            std::shared_ptr<Acts::MaterialMapper> m_materialMapper;

            /// The material mapping state
            std::unique_ptr<Acts::MaterialMapper::State> m_mappingState ATLAS_THREAD_SAFE = {};

            /// The tracking geometry service to retrive the geometry context and material surfaces
            ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc","ActsTrackingGeometrySvc", "The ACTS geometry service for the geometry context"};

            /// The RecordedMaterialTrackCollection to read
            SG::ReadHandleKey<RecordedMaterialTrackCollection> m_materialTrackCollectionKey {this, "MaterialTrackCollectionKey", "InputMaterialTracks", "Name of the input RecordedMaterialTrackCollection"};
            /// The mapped and unmapped material tracks
            SG::WriteHandleKey<RecordedMaterialTrackCollection> m_mappedMaterialTrackCollectionKey {this, "MappedMaterialTrackCollectionKey", "OutputMappedMaterialTracks", "Name of the output mapped RecordedMaterialTrackCollection"};
            SG::WriteHandleKey<RecordedMaterialTrackCollection> m_unmappedMaterialTrackCollectionKey {this, "UnmappedMaterialTrackCollectionKey", "OutputUnmappedMaterialTracks", "Name of the output unmapped RecordedMaterialTrackCollection"};
    };
}

#endif
