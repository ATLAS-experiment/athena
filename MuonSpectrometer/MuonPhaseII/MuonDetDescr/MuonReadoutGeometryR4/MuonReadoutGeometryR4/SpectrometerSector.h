/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_SPECTROMETERSECTOR_H
#define MUONREADOUTGEOMETRYR4_SPECTROMETERSECTOR_H

#ifndef SIMULATIONBASE

#include <MuonReadoutGeometryR4/MuonReadoutElement.h>
#include <MuonReadoutGeometryR4/Chamber.h>
#include <ActsGeometryInterfaces/GeometryDefs.h>
#include <Acts/Geometry/TrapezoidVolumeBounds.hpp>


#include <set>

namespace Acts {
    class Volume;
    class PlaneSurface;
}
namespace MuonGMR4{
    class SpectrometerSector;
    class Chamber;
}

namespace MuonGMR4 {
    /**
     *  A spectrometer sector forms the envelope of all chambers that are placed in the same MS sector & layer
     *  E.g. sector 5 in BI on the A-side. The spectrometer sector defines a coordinate system which is primarly used
     *  in the pattern recognition stage of the muon reconstruction
     *
     *          x-axis: Points along the Mdt tubes & is hence sensitive to the phi direction
     *          y-axis: Points to the next Mdt tube in the layer or in other words to the big wheel
     *                  in the sector, if the chamber is in the barrel or outside in radial direction
     *          z-axis: Points radially outwards if the chamber is in the barrel or in towards the cavern
     *                  wall if it's an endcap chamber
     *
     *   The origin of the spectrometer sector is placed in the nominal centre of the envelope.
     *
     *  The spectrometer contains pointer to all chambers & readout elements that are enclosed by it. */

    class SpectrometerSector {
        public:
            using ChamberPtr = GeoModel::TransientConstSharedPtr<Chamber>;
            using ChamberSet = std::vector<ChamberPtr>;

            /// @brief: Helper struct for fast approximate navigation. 
            /// Maps regions instrumented by chambers in the y-z frame. 
            /// Used in pattern recognition. 
            struct chamberLocation{
                public:
                    using enum Acts::TrapezoidVolumeBounds::BoundValues;
                    using BoundPtr_t = std::shared_ptr<const Acts::TrapezoidVolumeBounds>;
                    /** @brief Standard constructor taking the position of the readout element inside the 
                     *         sector frame, the pointer to the readout element itself and the volume
                     *          bounds enclosing the element volume.
                     *  @param origin: Position of the readout element centre expressed in the chamber frame
                     *  @param reEle: Pointer to the readout element of interest
                     *  @param bounds: Pointer to the bounds of interest. */
                    chamberLocation(const Amg::Vector3D& origin,
                                    const MuonReadoutElement* reEle,
                                    BoundPtr_t bounds):
                        m_origin{origin}, m_reEle{reEle}, m_bounds{std::move(bounds)}{}
                    /** @brief Copy constructor */
                    chamberLocation(const chamberLocation& other) = default;
                    /** @brief Move constructor */
                    chamberLocation(chamberLocation&& other) = default;
                    /** @brief Copy assignment */
                    chamberLocation& operator=(const chamberLocation& other) = default;
                    /** @brief Move assignment */
                    chamberLocation& operator=(chamberLocation&& other) = default;
                    /** @brief Returns whether the external position is inside the boundaries 
                     *         in the y-z plane.
                     *  @param pos: Position to check */
                    bool insideYZ(const Amg::Vector3D& pos) const {
                      return minY() <= pos.y() &&  maxY() >= pos.y() &&
                             minZ() <= pos.z() &&  maxZ() >= pos.z();
                    }
                     /** @brief Returns the minimum y covered by the chamber location */
                     double minY() const { return m_origin.y() - m_bounds->get(eHalfLengthY); }
                     /** @brief Returns the maximum u covered by the chamber location */
                     double maxY() const { return m_origin.y() + m_bounds->get(eHalfLengthY); }
                     /** @brief Returns the minimum y covered by the chamber location */
                     double minZ() const { return m_origin.z() - m_bounds->get(eHalfLengthZ); }
                     /** @brief Returns the maximum u covered by the chamber location */
                     double maxZ() const { return m_origin.z() + m_bounds->get(eHalfLengthZ); }
                     /** @brief Calculate the strip / tube length at a given position in the y-z plane */
                     double width(const double y0) const {
                        const double tanPhiHalf = 0.5*(m_bounds->get(eHalfLengthXposY) - m_bounds->get(eHalfLengthXnegY)) 
                                                / m_bounds->get(eHalfLengthY); 
                        return m_bounds->get(eHalfLengthXnegY) + tanPhiHalf * (y0 - minY());
                     }
                     /** @brief Returns the poter to the associate readout element  */
                     const MuonReadoutElement* readoutEle() const { return m_reEle; }
                     /** @brief Returns the pointer to the associate bounds */
                    const BoundPtr_t& bounds() const { return m_bounds; }
                    /** @brief Returns the location */
                    const Amg::Vector3D& location() const { return m_origin; }
                private:
                    /** @brief Origin vector of the readout element inside the spectrometer frame */
                    Amg::Vector3D m_origin{Amg::Vector3D::Zero()};
                    /** @brief Associated readout element */
                    const MuonReadoutElement* m_reEle{nullptr};
                    /** @brief Pointer to the associated bounds */
                    BoundPtr_t m_bounds{};
            };

            struct defineArgs{
                /// List of readout elements in the chamber
                ChamberSet chambers{};
                /** @brief Surrouding box chamber bounds */
                std::shared_ptr<Acts::TrapezoidVolumeBounds> bounds{};
                /// Surface in the centre of the chamber plane
                std::shared_ptr<const Acts::PlaneSurface> surface{};
                
                std::vector<chamberLocation> detectorLocs{}; 
            };

            /** @brief Standard constructor taking the defining parameters */
            SpectrometerSector(defineArgs&& args);
            /** @brief Delete the copy constructor and copy assignment */
            SpectrometerSector(const SpectrometerSector& other) = delete;
            SpectrometerSector& operator=(const SpectrometerSector& other) = delete;

            bool operator<(const SpectrometerSector& other) const;

            /// Returns a string encoding the chamber index & the sector of the MS sector
            std::string identString() const;
            /** @brief Returns the IdHelpeSvc */
            const Muon::IMuonIdHelperSvc* idHelperSvc() const;
            /** @brief Returns the chamber index scheme */
            Muon::MuonStationIndex::ChIndex chamberIndex() const;
            /** @brief: Returns the station phi of the sector */
            int stationPhi() const;
            /**  @brief Returns the sector of the MS-sector */
            int sector() const;
            /** @brief Returns the side of the MS-sector 1 -> A side ; -1 -> C side */
            int8_t side() const;
            /** @brief Returns whether the sector is placed in the barrel */
            bool barrel() const;
            /** @brief  Returns the local -> global tarnsformation from the sector
              * @param gctx: Geometry context carrrying the alignment transformations */
            const Amg::Transform3D& localToGlobalTrans(const ActsGeometryContext& gctx) const;
            /** @brief Returns the global -> local transformation from the ATLAS global */
            Amg::Transform3D globalToLocalTrans(const ActsGeometryContext& gctx) const;
            /** @brief Returns the associated surface */
            const Acts::PlaneSurface& surface() const;
            /** @brief Returns the associated chambers with this sector */
            const ChamberSet& chambers() const;
            /** @brief Long-extend of the chamber in the x-direction at positive Y */
            double halfXLong() const;
            /** @brief Short extend of the chamber in the x-direction at negative Y*/
            double halfXShort() const;
            /** @brief Extend of the chamber in the y-direction */
            double halfY() const;
            /** @brief Thickness of the chamber in the z-direction */
            double halfZ() const;  
            /** @brief Returns the reference to the defining parameters of the sector */
            const defineArgs& parameters() const;
            /** @brief Returns the Acts::Volume representation of the sector.
              * @param gctx: Geometry context carrrying the alignment transformations */
            std::shared_ptr<Acts::Volume> boundingVolume(const ActsGeometryContext& gctx) const;
            /** @brief Returns the volume bounds */
            std::shared_ptr<Acts::TrapezoidVolumeBounds> bounds() const;
            /** @brief Returns the list of all associated readout elements */
            Chamber::ReadoutSet readoutEles() const;
            /// returns the list of all MDT chambers in the sector for fast navigation
            const std::vector<chamberLocation> & chamberLocations() const; 


        private:
           defineArgs m_args{};
    };
    
    std::ostream& operator<<(std::ostream& ostr,
                             const SpectrometerSector::defineArgs& args);

    std::ostream& operator<<(std::ostream& ostr,
                             const SpectrometerSector& chamber);

}


#endif
#endif
