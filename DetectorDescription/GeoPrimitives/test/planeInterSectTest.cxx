/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"

#include <stdlib.h>
#include <iostream>

int main (){
    int retCode = EXIT_SUCCESS;
    const Amg::Vector3D pointInPlane{561.,0.,0.};
    const Amg::Vector3D planeNormal{Amg::Vector3D::UnitX()};

    const Amg::Vector3D extPoint{pointInPlane + 50.*planeNormal};
    const Amg::Vector3D lineDir = Amg::Vector3D{1.,5., 0.}.unit();

    const Amg::Vector3D crossPoint = extPoint + Amg::intersect<3>(extPoint, lineDir,planeNormal, planeNormal.dot(pointInPlane)).value_or(-2242) * lineDir;
    
    if (std::abs(pointInPlane.x() - crossPoint.x()) > std::numeric_limits<float>::epsilon()) {
        std::cerr<<"Crossing point is not in the desired yz-plane: "<<Amg::toString(crossPoint)<<std::endl;
        retCode = EXIT_FAILURE;
    }
    if (std::abs(pointInPlane.y() - crossPoint.y() - 250.) > std::numeric_limits<float>::epsilon()) {
        std::cerr<<"Crossing point is not in the desired yz-plane: "<<Amg::toString(crossPoint)<<std::endl;
        retCode = EXIT_FAILURE;
    }
    std::cout<<"Successfully extrapolated "<<Amg::toString(pointInPlane)<<std::endl;
    return retCode;
}