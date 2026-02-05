/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMATERIAL_MATERIALVALIDATION_H
#define ACTSMATERIAL_MATERIALVALIDATION_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsEvent/RecordedMaterialTrackCollection.h"
#include "AthenaKernel/IAthRNGSvc.h"

#include "Acts/Material/MaterialValidater.hpp"

namespace ActsTrk {
    /// @class MaterialValidation
    ///
    /// @brief Reads the MaterialTracks and produces the material maps
    /// for the Tracking Geometry.
    /// The output map is stored in a ROOT file, json, etc.
    /// depending on the configured writers.

    class MaterialValidation : public AthReentrantAlgorithm  {
        public:
            MaterialValidation(const std::string &name, ISvcLocator *pSvcLocator);
            virtual StatusCode initialize() override;
            virtual StatusCode execute (const EventContext& ctx) const override;
            virtual ~MaterialValidation();

        private:
            /// The material validater from the ACTS core components
            std::shared_ptr<Acts::MaterialValidater> m_materialValidater;
            /// The random number service
            ServiceHandle<IAthRNGSvc> m_rndmGenSvc{this, "AthRNGSvc", "AthRNGSvc", "The random number service"};
            /// The number of tracks to use per event
            Gaudi::Property<size_t> m_nTracks{this, "NumberOfTracks", 1000, "Number of tracks to use per event"};
            /// The eta range for track generation
            Gaudi::Property<std::pair<double, double>> m_etaRange{this, "EtaRange", {-4.5, 4.5}, "The eta range for tracks"};

            /// The tracking geometry service to retrive the geometry context and material surfaces
            ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc","ActsTrackingGeometrySvc", "The ACTS geometry service for the geometry context"};

            /// The RecordedMaterialTrackCollection to write
            SG::WriteHandleKey<RecordedMaterialTrackCollection> m_materialTrackCollectionKey {this, "MaterialTrackCollectionKey", "OutputMaterialTracks", "Name of the output RecordedMaterialTrackCollection"};
    };
}

#endif
