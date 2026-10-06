/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSMUONDETECTOR_BOUNDSEXPANDER_H
#define ACTSMUONDETECTOR_BOUNDSEXPANDER_H

#include "GeoPrimitives/GeoPrimitives.h"

#ifndef SIMULATIONBASE
/// Ensure that the Algebra plugin is included after
/// GeoPrimitives
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Utilities/BoundFactory.hpp"
#include "Acts/Utilities/ArrayHelpers.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Utilities/Logger.hpp"


#include <memory>
#include <climits>
#include <span>

namespace MuonGMR4 {
    class MuonReadoutElement;
    class Chamber;
    class SpectrometerSector;
}
namespace Acts{
    class Surface;
    class GeometryContext;
    class Volume;
    class TrackingVolume;
    class VolumeBounds;
}


namespace MuonGMR4 {
    class BoundsExpander {
        public:
            /** @brief Returns the vertices from a volume
             *  @param tgContext: The geometry context to align the volume
             *  @param volume: The volume from which the vertices shall be retrieved */
            static std::vector<Amg::Vector3D> cornerPoints(const Acts::GeometryContext& tgContext, 
                                                           const Acts::Volume& volume);
            /** @brief Returns the vertices from a surface
             *  @param tgContext: The geometry context to align the surface
             *  @param volume: The surface from which the vertices shall be retrieved */
            static std::vector<Amg::Vector3D> cornerPoints(const Acts::GeometryContext& tgContext, 
                                                            const Acts::Surface& surface);
            /** @brief Crate volume bounds that enclose the readout element from the central
              *         localToGlobalTransform
              *  @param readoutElement: The readout element of interest
              *  @param boundFactory: The factory used to create the bounds */
            static std::shared_ptr<Acts::VolumeBounds> 
                makeBounds(const MuonGMR4::MuonReadoutElement& readoutElement,
                           Acts::VolumeBoundFactory& boundFactory);
            /** @brief Abrivation of the transform getter */
            using VolumeAligner_t = std::function<Acts::Transform3(const Acts::GeometryContext& tgContext)>;
            /** @brief Default constructor */
            explicit BoundsExpander() = default;
            /** @brief Constructor with a transform from the global -> local frame
             *         of the volume
             * @param toVolume: The Transform to the volume
             * @param loggerObj: The logger instance */
            explicit BoundsExpander(const Acts::Transform3& toVolume,
                                    std::unique_ptr<const Acts::Logger> loggerObj = nullptr);
            /** @brief Expand the bounds from a chamber
              * @param tgContext: The goemetry contex to position the chamber in space
              * @param chamber: Reference to the chamber from which the vertices are extracted */
            void expand(const Acts::GeometryContext& tgContext,
                        const MuonGMR4::Chamber& chamber);
            /** @brief Expand the bounds from a sector
              * @param tgContext: The goemetry contex to position the sector in space
              * @param sector: Reference to the sector from which the vertices are extracted */
            void expand(const Acts::GeometryContext& tgContext,
                        const MuonGMR4::SpectrometerSector& sector);
            /** @brief Expand the bounds from a readout element
             *  @param tgContext: The geometry context to align the readout element in space
             *  @param readoutElement: The readout element of interest
             *  @param boundFactory: To create the temporary volume bounds enclosing 
             *                       the readout element */
            void expand(const Acts::GeometryContext& tgContext,
                        const MuonGMR4::MuonReadoutElement& readoutElement,
                        Acts::VolumeBoundFactory& boundFactory);

            /** @brief Expand the bound definition from a generic volume
             *  @param tgContext: The geometry context to align the volume in space
             *  @param volume: The volume of interest  */
            virtual void expand(const Acts::GeometryContext& tgContext,
                                const Acts::Volume& volume) = 0;
            /** @brief Make the final bounds after all volumes are passed via
             *         the expand method
             * @param boundFactory: The factory object used to create the final bounds */
            virtual std::shared_ptr<Acts::VolumeBounds> 
                makeBounds(Acts::VolumeBoundFactory& boundFactory) = 0;

            /** @brief Creates an empty tracking volume. The volume will be positioned
             *         at the vertex center 
             * @param factory: The bound factory to create the bounds
             * @param volName: The name of the tracking volume */
            std::unique_ptr<Acts::TrackingVolume> makeEnvelope(Acts::VolumeBoundFactory& factory,
                                                               std::string_view volName);
            /** @brief Returns the transform from the ATLAS frame
             *         to the center of all seen vertices */
            Acts::Transform3 toVertexCenter() const;
            /** @brief Returns the transform from the nominal volume center
             *         to the middle point to all seen vertices */
            const Acts::Transform3& boxMidPoint() const;
            /** @brief Update the center point from a list of global vertices
             *  @param vertices: List of vertices expressed in th ATLAS frame*/
            void reCenter(std::span<const Amg::Vector3D> vertices);
            /** @brief Returns the triplet of the minimum coordinates from the
             *         ensemble of all seen vertices */
            std::span<const double> minima() const;
           /** @brief Returns the triplet of the maximum coordinates from the
             *         ensemble of all seen vertices */
            std::span<const double> maxima() const;
            /** @brief Returns the ACTS logegr object */
            const Acts::Logger& logger() const;
        private:
            std::unique_ptr<const Acts::Logger> m_logger{Acts::getDefaultLogger("BoundsExpander",
                                                                                Acts::Logging::Level::INFO)};
            /** @brief External method to go from ATLAS to the reference frame  */
            Acts::Transform3 m_toVolume{Acts::Transform3::Identity()};
            /** @brief Extra shift to center all the vertices */
            Acts::Transform3 m_shiftTrf{Acts::Transform3::Identity()};
            /** @brief Maximum value seen by the recenter method */
            std::array<double, 3> m_locMax{Acts::filledArray<double, 3>(-2.*Acts::UnitConstants::km)};
            /** @brief Minimum value seen by the recenter method */
            std::array<double, 3> m_locMin{Acts::filledArray<double, 3>(2.*Acts::UnitConstants::km)};
    };

}
#endif
#endif