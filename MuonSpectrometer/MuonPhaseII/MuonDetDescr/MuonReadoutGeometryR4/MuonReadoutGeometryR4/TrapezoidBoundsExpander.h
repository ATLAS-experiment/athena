/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSMUONDETECTOR_TRAPEZOIDBOUNDSEXPANDER_H
#define ACTSMUONDETECTOR_TRAPEZOIDBOUNDSEXPANDER_H
#ifndef SIMULATIONBASE

#include "MuonReadoutGeometryR4/BoundsExpander.h"

#include <iostream>

namespace MuonGMR4 {
    class TrapezoidBoundsExpander : public BoundsExpander {
        public:
            /** @brief Helper struct to describe a sub trapezoid that is 
             *         not aligned with the center of the coordinate system */
            struct TrapezoidPatch{
                /** @brief The constructor  */
                explicit TrapezoidPatch(const Acts::Transform3& trf,
                                        std::shared_ptr<const Acts::VolumeBounds> volBounds);
                /** @brief Print the TrapezoidPatch */
                void print(std::ostream& ostr) const;
                /** @brief the ostream operator */
                inline friend std::ostream& operator<<(std::ostream& ostr, const TrapezoidPatch& patch) {
                    patch.print(ostr);
                    return ostr;
                }
                /** @brief The transform to move to the outer trapezoid center */
                Acts::Transform3 toCenter{Acts::Transform3::Identity()};
                /** @brief Pointer to the volume bounds  */
                std::shared_ptr<const Acts::VolumeBounds> bounds{}; 
            };
            /** @brief Enum to select the left / right corners of a trapezoid */
            enum class Side : std::int8_t{left = -1, right = 1};
            /** @brief Enum to select the top / bottom corners of a trapezoid */
            enum class Level : std::int8_t { bottom= -1, top = 1 };

            friend std::ostream& operator<<(std::ostream& ostr, const Side side){
                switch (side) {
                    case Side::left: return ostr<<"left";
                    case Side::right: return ostr<<"right";
                }
                return ostr;
            }
            friend std::ostream& operator<<(std::ostream& ostr, const Level lvl) {
                switch (lvl) {
                    case Level::bottom: return ostr<<"bottom";
                    case Level::top: return ostr<<"top";
                }
                return ostr;
            }
            /** @brief Returns the vertex point from a trapezoid
             *  @param trapezoid: Reference to the trapezoid of interest
             *  @param side: Is the vertex on the left [-x] or positive side
             *               of the trapezoid
             * @param lvl: Is the vertex on the upper or the bottom edge of the 
             *             trapezoid */
            static Amg::Vector3D makeVertex(const TrapezoidPatch& trapezoid,
                                            const Side side,
                                            const Level lvl);
            /** @brief Returns the distance to the inclided trapezoidal side 
             *         Positive values imply that the test point is outside 
             *         of the trapezoid edge 
             *  @param vertex: The side vertex of the trapezoid 
             *  @param edge: The direction vector describing the inclined edge
             *  @param testMe: The external point to test
             *  @param side: Flag to toggle whether the edge is on the left or right */
            static double distanceToTrapezoid(const Amg::Vector3D& vertex,
                                              const Amg::Vector3D& edge,
                                              const Amg::Vector3D& testMe,
                                              const Side side);
            /** @brief Construct an index from [0;3] from the side and level */
            static std::uint8_t toIndex(const Side side, const Level lvl);

            /** @brief Use the constructor from the base class */
            using BoundsExpander::BoundsExpander;
            /** @brief Use the expand overloads w.r.t. chamber, sector */
            using BoundsExpander::expand;
            /** @brief Use the makeBounds overload for the readout element */
            using BoundsExpander::makeBounds;
            /** @copydoc BoundExpander::expand */
            virtual void expand(const Acts::GeometryContext& tgContext,
                                 const Acts::Volume& volume) override final;

            /** @copydoc BoundExpander::makeBounds */
            virtual std::shared_ptr<Acts::VolumeBounds> 
                    makeBounds(Acts::VolumeBoundFactory& factory) override final;      
            /** @brief Apply an */
            void setMargin(const double margin);

            void setMargin(const double marginXlow, const double marginXhigh,
                           const double marginY, const double marginZ);
        private:       
            std::vector<TrapezoidPatch> m_constituents{};
            std::shared_ptr<Acts::VolumeBounds> m_finalBounds{};
            std::array<double, 4> m_margin{Acts::filledArray<double, 4>(0.)};
    };
}
#endif
#endif