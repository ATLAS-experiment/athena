/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTrackEvent/Circle.h"

#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"

namespace MuonR4 {
    std::ostream& Circle::print(std::ostream& ostr) const {
        ostr<<"Circle - center: "<<Amg::toString(center())
            <<", radius: "<<radius()
            <<", normal: "<<Amg::toString(normal());
        return ostr;
    }
    Circle::Circle(const Amg::Vector3D& pointA,
                   const Amg::Vector3D& pointB,
                   const Amg::Vector3D& pointC) {
        
        /// Construct the connection line A -> C 
        const Amg::Vector3D AC = (pointC - pointA).unit();
        /// Construct the connection line A -> B
        const Amg::Vector3D AB = (pointB - pointA).unit();
        /// The circle plane normal is the normalized cross product between the 2
        m_normal = AC.cross(AB).unit();
        /// Calculate the bisector between A + B
        const Amg::Vector3D midAB = 0.5*(pointA + pointB);
        /// Calculate the bisector between A + C
        const Amg::Vector3D midAC = 0.5*(pointA + pointC);

        const Amg::Vector3D normAB = AB.cross(m_normal);
        const Amg::Vector3D normAC = AC.cross(m_normal);

        const std::optional<double> midIsect = Amg::intersect<3>(midAB, normAB,  midAC, normAC);
        if (!midIsect) {
            m_normal.setZero();
            return;
        }
        m_center = midAC + (*midIsect) * normAC;
        m_radius = std::copysign((pointA - m_center).mag(), (*midIsect));
    }
}