/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"

#include <stdlib.h>
#include <iostream>
int main() {

    const Amg::Vector3D posA{100, 0., 0.};
    const Amg::Vector3D posB{100, 50, 0.};

    const double parallelTest = Amg::lineDistance<3>(posA, Amg::Vector3D::UnitZ(), posB, Amg::Vector3D::UnitZ());
    int retCode = EXIT_SUCCESS;
    if (std::abs(parallelTest - 50) > std::numeric_limits<double>::epsilon()){
        std::cerr<<__FILE__<<":"<<__LINE__<<" - Distance between two lines is "<<parallelTest<<". Bus 50 is expected. "<<std::endl;
        retCode = EXIT_FAILURE;
    }
    std::cout<<__FILE__<<":"<<__LINE__<<" - Passed test between two parallel crossing lines: "<<parallelTest<<std::endl;

    const double orthogonalTest = Amg::lineDistance<3>(posA, Amg::Vector3D::UnitX(), posB, Amg::Vector3D::UnitZ());
    if (std::abs(orthogonalTest - 50) > std::numeric_limits<double>::epsilon()){
        std::cerr<<__FILE__<<":"<<__LINE__<<" - Distance between two lines is "<<parallelTest<<". Bus 50 is expected. "<<std::endl;
        retCode = EXIT_FAILURE;
    }
    std::cout<<__FILE__<<":"<<__LINE__<<" - Passed test between two orthogonal lines: "<<orthogonalTest<<std::endl;

    constexpr double angleXY = 36. * M_PI / 180.;
    const Amg::Vector3D dirInXY{Amg::dirFromAngles(0.,angleXY)};
    const double planeXYTest = Amg::lineDistance(posA, dirInXY, posB, dirInXY);
    if (std::abs(planeXYTest - 50.) > std::numeric_limits<double>::epsilon()) {
        std::cerr<<__FILE__<<":"<<__LINE__<<" - Distance between two lines is "<<planeXYTest<<". Bus 50 is expected. "<<std::endl;
        retCode = EXIT_FAILURE;
    }
    std::cout<<__FILE__<<":"<<__LINE__<<" - Passed test between two parallel lines in the x-y plane: "<<planeXYTest<<std::endl;
    /// Generate a plumb-line 
    const Amg::Vector3D plumbDir{-dirInXY.z(), 0., dirInXY.x()};
    /// Another vector that's perpendicular to the plumb but not parallel to the original line 
    const Amg::Vector3D extDir = (5.* dirInXY + 21. * plumbDir.cross(dirInXY)).unit();

    if (extDir.dot(plumbDir) > std::numeric_limits<float>::epsilon() ||
        plumbDir.dot(dirInXY) > std::numeric_limits<float>::epsilon()) {
        std::cerr<<__FILE__<<":"<<__LINE__<<" - The external dir is not orthogonal "<<Amg::toString(dirInXY)
                 <<";"<<Amg::toString(plumbDir)<<";"<<Amg::toString(extDir)<<" -- "<<extDir.dot(plumbDir)<<" "<<plumbDir.dot(dirInXY)<<std::endl;
        retCode = EXIT_FAILURE;
    }
    /// Create a random external point that's used as reference for the second test line
    const Amg::Vector3D extPoint = posA + 50. * dirInXY + 50. * plumbDir + 400. * extDir;
    /// Recuperate the external point that's closest to the primary line
    const Amg::Vector3D closePointExt = extPoint + Amg::intersect(posA, dirInXY, extPoint, extDir).value_or(-8888.) * extDir;
    if  ( (posA + 50. * dirInXY + 50. * plumbDir - closePointExt).mag() > std::numeric_limits<float>::epsilon()){
      std::cerr<<__FILE__<<":"<<__LINE__<<" - The closest point on the external line "<<Amg::toString(closePointExt)
               <<" is not "<<Amg::toString(posA + 50. * dirInXY + 50. * plumbDir)<<std::endl;
      retCode = EXIT_FAILURE;
    }
    /// Let's get the second closest point
    const Amg::Vector3D closePointXY = posA + Amg::intersect(extPoint, extDir, posA, dirInXY).value_or(-8888.) * dirInXY;
    if  ( (posA + 50. * dirInXY - closePointXY).mag() > std::numeric_limits<float>::epsilon()){
      std::cerr<<__FILE__<<":"<<__LINE__<<" - The closest point on the external line "<<Amg::toString(closePointXY)
               <<" is not "<<Amg::toString(posA + 50. * dirInXY)<<std::endl;
      retCode = EXIT_FAILURE;
    }
    /// Calculate the distance between the two close points
    if ( std::abs((closePointXY - closePointExt).mag() - 50.) > std::numeric_limits<float>::epsilon()) {
      std::cerr<<__FILE__<<":"<<__LINE__<<"The two lines shall have a distance of 50. But they've "<<(closePointXY - closePointExt).mag()<<std::endl;
      retCode = EXIT_FAILURE;
    }

    const double extLineDist = Amg::lineDistance(posA, dirInXY, extPoint, extDir);
    if (std::abs(extLineDist - 50.) > std::numeric_limits<float>::epsilon()) {
        std::cerr<<__FILE__<<":"<<__LINE__<<" - Distance between two lines is "<<extLineDist<<". Bus 50 is expected. "<<std::endl;
        retCode = EXIT_FAILURE;
    }
    /// Finally the case when both lines are crossing
    const double crossLines = Amg::lineDistance<3>(extPoint+ 525 * dirInXY, dirInXY, extPoint, extDir);
    if (std::abs(crossLines) > std::numeric_limits<float>::epsilon()) {
        std::cerr<<__FILE__<<":"<<__LINE__<<" - Expect crossing lines but they are by "<<crossLines<< " apart."<<std::endl;
        retCode = EXIT_FAILURE;
    }
    return retCode;
}