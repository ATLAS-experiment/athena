/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_GEOMETRYENVELOPETEST_H
#define ACTSGEOMETRY_GEOMETRYENVELOPETEST_H

#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "AthenaBaseComps/AthAlgorithm.h"


namespace ActsTrk{
    class GeometryEnvelopeTest: public AthAlgorithm {
        public:
            using AthAlgorithm::AthAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) override final;
        private:
            /** @brief Checks whether all subvolumes are contained in the volume
             *         Method recursively called for each subvolume
             *  @param tgContext: The Geometry context to align the volumes and surfaces
             *  @param volume: The envelope of interest to be checked */
            StatusCode checkVolume(const Acts::GeometryContext& tgContext,
                                   const Acts::TrackingVolume& volume) const;
            /** @brief Extracts the vertices form the TrackingVolume marking its edges
             *  @param tgContext: The geometry context to position the volume
             *  @param volume: The envelope from which the vertex points shall be extracted */
            std::vector<Amg::Vector3D> edges(const Acts::GeometryContext& tgContext,
                                             const Acts::TrackingVolume& volume) const;
            /** @brief The handle to the tracking geometry service */
            ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};

            Gaudi::Property<std::string> m_ITkExitVolume{this, "ITkExitVolume", ""};
    
            Gaudi::Property<std::string> m_caloExitVolume{this, "CaloExitVolume", ""};
            
            Gaudi::Property<std::string> m_MsExitVolume{this, "MsExitVolume", ""};
            
    };
}

#endif