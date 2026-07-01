/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKEVENT_CIRCLE_H
#define MUONTRACKEVENT_CIRCLE_H

#include <GeoPrimitives/GeoPrimitives.h>

#include <ostream>

namespace MuonR4{
    /** @brief Auxiliary class to construct a circle from three arbitrary points in space. 
               The center point, (signed) radius and the normal vector to the plane 
               spanned by the points are evaluated.  */
    class Circle{
        public:
            /** @brief Constructor taking three points in a plane
                       If points are on a line, the plane is set to
                       be the null vector
                @param pointA: First circle point
                @param pointB: Second circle point
                @param pointC: Third circle point */
            explicit Circle(const Amg::Vector3D& pointA,
                            const Amg::Vector3D& pointB,
                            const Amg::Vector3D& pointC);
            
            /** @brief The center point of the circle */
            const Amg::Vector3D& center() const { return m_center; }
            /** @brief The vector that is normal to the circle */
            const Amg::Vector3D& normal() const { return m_normal; }
            /** @brief The radius of the circle */
            double radius() const { return m_radius; }
            /** @brief Define the ostream operator */
            inline friend std::ostream& operator<<(std::ostream& ostr, const Circle& circ) {
                return circ.print(ostr);
            }
        private:
            std::ostream& print(std::ostream& ostr) const;
            /** @brief The center point of the circle */
            Amg::Vector3D m_center{Amg::Vector3D::Zero()};
            /** @brief The plane normal of the circle */
            Amg::Vector3D m_normal{Amg::Vector3D::Zero()};
            /** @brief The radius of the circle */
            double m_radius{0.};


    };
}

#endif