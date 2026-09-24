/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "GeometryEnvelopeTest.h"

#include "Acts/Geometry/TrackingVolume.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/VolumeBounds.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Surfaces/RegularSurface.hpp"
#include "Acts/Surfaces/SurfaceBounds.hpp"
#include "Acts/Definitions/Units.hpp"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"


using namespace ActsTrk::detail::GeoVolIds;
using namespace Acts::UnitLiterals;
namespace ActsTrk{
    StatusCode GeometryEnvelopeTest::initialize() {
        ATH_CHECK(m_trackingGeometrySvc.retrieve());
        return StatusCode::SUCCESS;
    }

    StatusCode GeometryEnvelopeTest::execute(const EventContext& ctx) {
        const Acts::GeometryContext tgContext = m_trackingGeometrySvc->getNominalContext().context();
        
        const Acts::TrackingGeometry* trkGeo = m_trackingGeometrySvc->trackingGeometry().get();
        /// Ensure that all tracking volume are in itself consistent. I.e. the subvolumes
        /// are well contained in their parent volumes
        ATH_CHECK(checkVolume(tgContext, *trkGeo->highestTrackingVolume()));
        /// Check that the system envelope exists and that it has the correct name
        /// The system envelope's name is set externally in the test configuration 
        auto checkEnevelope = [this](const StringProperty& envName, 
                                     const SystemEnvelope env) -> StatusCode {
            if (envName.value().empty()) {
                ATH_MSG_DEBUG("The envelope "<<envName<<" is undefined");
                return StatusCode::SUCCESS;
            }
            const Acts::TrackingVolume* envelope = m_trackingGeometrySvc->getEnvelope(env);
            if (!envelope) {
                ATH_MSG_ERROR("The envelope "<<envName<<"/ "<<env<<" does not exist in the tracking geometry");
                return StatusCode::FAILURE;
            }
            if (envelope->volumeName() != envName) {
                ATH_MSG_ERROR("The envelope "<<envelope->volumeName()<<" "<<envelope->volumeBounds()<<" "
                             <<" has a different name than expected: "<<envName);
                return StatusCode::FAILURE;
            }
            ATH_MSG_INFO("Envelopes match by name "<<envelope->volumeName()<<" "<<envelope->volumeBounds());
            return StatusCode::SUCCESS;
        };
        ATH_CHECK(checkEnevelope(m_ITkExitVolume, SystemEnvelope::ITkExit));
        ATH_CHECK(checkEnevelope(m_caloExitVolume, SystemEnvelope::CaloExit));
        ATH_CHECK(checkEnevelope(m_MsExitVolume, SystemEnvelope::MsExit));

        ATH_MSG_INFO("Test succeeded "<<ctx.eventID());
        return StatusCode::SUCCESS;
    }
    StatusCode GeometryEnvelopeTest::checkVolume(const Acts::GeometryContext& tgContext,
                                                 const Acts::TrackingVolume& volume) const {
        ATH_MSG_VERBOSE("Check whether all volumes are within the envelope defined by "
                        <<volume.volumeName()<<", "<<volume.volumeBounds());
        bool allGood{true};
        for (const Acts::TrackingVolume& childVolume : volume.volumes()) {
            for (const Amg::Vector3D& vert : edges(tgContext, childVolume)) {
                const Amg::Vector3D lVert = volume.globalToLocalTransform(tgContext)* vert;

                if (!volume.volumeBounds().inside(lVert)) {
                    ATH_MSG_ERROR("The vertex "<<Amg::toString(lVert)<<", perp:"<<lVert.perp()
                        <<" is not inside of the volume envelope "<<volume.volumeName()
                        <<" "<<volume.volumeBounds());
                    allGood = false;
                }
            }
        }
        if (!allGood) {
            return StatusCode::FAILURE;
        }
        for (const Acts::Surface& surface : volume.surfaces()) {
            for (const Amg::Vector3D& vert : vertices(tgContext, surface)) {
                const Amg::Vector3D lVert = volume.globalToLocalTransform(tgContext)* vert;
                if (!volume.volumeBounds().inside(lVert, 0.1_mm)) {
                    ATH_MSG_ERROR("The vertex "<<Amg::toString(lVert)<<", perp:"<<lVert.perp()
                        <<" is not inside of the volume envelope "<<volume.volumeName()
                        <<" "<<volume.volumeBounds());
                    allGood = false;
                }
            }
        }
        for (const Acts::TrackingVolume& childVolume : volume.volumes()) {
            ATH_CHECK(checkVolume(tgContext, childVolume));
        }
        return StatusCode::SUCCESS;
    }
     std::vector<Amg::Vector3D> GeometryEnvelopeTest::edges(const Acts::GeometryContext& tgContext,
                                                            const Acts::TrackingVolume& volume) const {
        std::vector<Amg::Vector3D> result{};
        for (const auto& oriented :  volume.volumeBounds().orientedSurfaces(volume.localToGlobalTransform(tgContext))) {
            std::vector<Amg::Vector3D> verts = vertices(tgContext,*oriented.surface);
            result.insert(result.end(), 
                          std::make_move_iterator(verts.begin()),
                          std::make_move_iterator(verts.end()));
        }
        return result;
    }
    std::vector<Amg::Vector3D> GeometryEnvelopeTest::vertices(const Acts::GeometryContext& tgContext,
                                                              const Acts::Surface& surface) const {
        Acts::Polyhedron polyhedron = surface.polyhedronRepresentation(tgContext, 10);
        return polyhedron.vertices;
    }
}
