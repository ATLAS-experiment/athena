/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GeoPrimitives.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef GEOPRIMITIVES_GEOPRIMITIVES_H
#define GEOPRIMITIVES_GEOPRIMITIVES_H

#include "EventPrimitives/EventPrimitives.h"

/** Definition of ATLAS Math & Geometry primitives (Amg) 

    This is based on the Eigen geometry module:
    http://eigen.tuxfamily.org/dox/group__Geometry__Module.html

    @author  Robert Johannes Langenberg <robert.langenberg@cern.ch>
    @author Andreas Salzburger <Andreas.Salzburger@cern.ch>
    @author Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
        
*/

namespace Amg {

    /** element for code readability
        - please use these for access to the member variables if needed, e.g.
            double z  = position[Amg::z];
            double px = momentum[Amg::px];
    */
    enum AxisDefs {
        // position access
        x = 0,
        y = 1,
        z = 2,
        // momentum access
        px = 0,
        py = 1,
        pz = 2
    };

    using Rotation3D            = Eigen::Quaternion<double>;
    using Translation3D         = Eigen::Translation<double, 3>;
    using AngleAxis3D           = Eigen::AngleAxisd;
    using Transform3D           = Eigen::Affine3d;
    using Isometry3D            = Eigen::Isometry3d;
    using Vector3D              = Eigen::Matrix<double, 3, 1>;
    using Vector2D              = Eigen::Matrix<double, 2, 1>;
    using RotationMatrix3D      = Eigen::Matrix<double, 3, 3>;
    
   


}
#endif /* GEOPRIMITIVES_GEOPRIMITIVES_H */
