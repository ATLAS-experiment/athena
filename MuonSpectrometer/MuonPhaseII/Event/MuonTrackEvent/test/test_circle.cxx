/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTrackEvent/Circle.h"

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"

#include "Acts/Definitions/Units.hpp"
#include "Acts/Definitions/Tolerance.hpp"

#include <iostream>

using namespace Acts::UnitLiterals;

constexpr double epsilon = 1.e-9;

#define LOG_MSG(msg)\
    std::cout<<__func__<<" "<<__LINE__<<" - "<<msg<<std::endl;

#define ERROR_MSG(msg) \
    std::cerr<<__func__<<" "<<__LINE__<<" - ERROR: "<<msg<<std::endl; \
    ret_code = EXIT_FAILURE;


#define TEST_POINT(P)                                               \
    {                                                               \
        const Amg::Vector3D d = (P -c1);                            \
        if (std::abs(d.norm() - r)  / r> epsilon){                  \
        ERROR_MSG("The test point "<<Amg::toString(P)               \
            <<" is larger than "<<r<<" separated from the center "  \
            <<Amg::toString(c1)<<" vs. "<<d.norm());                \
        }                                                           \
                                                                    \
        if (std::abs(d.dot(circleNorm)) > epsilon) {                \
            ERROR_MSG("The test point "<<Amg::toString(P)           \
            <<" is not in the plane "<<Amg::toString(circleNorm)    \
            <<"/"<<Amg::toString(c1)<<" "<<d.dot(circleNorm))       \
        }                                                           \
    } 

#define TEST_CIRCLE(C)                                          \
    LOG_MSG("test circle "<<C);                                 \
    if ( (C.center() - c1).norm() > epsilon) {                  \
        ERROR_MSG("The constructed circle point "               \
            <<Amg::toString(C.center())                         \
            <<" does not coincide with the injected center "    \
            <<Amg::toString(c1));                               \
    }                                                           \
                                                                \
    if ( std::abs(std::abs(C.normal().dot(circleNorm)) - 1.) > epsilon) {       \
        ERROR_MSG("The constructed circle normal "<<Amg::toString(C.normal())   \
            <<" does not coincide with the injected normal "                    \
            <<Amg::toString(circleNorm));                                       \
    }                                                                           \
                                                                                \
    if (std::abs(std::abs(C.radius()) - r) /r  > epsilon) {            \
        ERROR_MSG("The constructed radius "<<C.radius()                \
                <<" does not coincide with the injected radius "<<r);  \
    }


int main() {

    int ret_code = EXIT_SUCCESS;

    const Amg::Vector3D c1{1.,2.,3.};

    const Amg::Vector3D circleNorm =  Amg::Vector3D{-1., -1., 1.}.unit();
     
    const Amg::Vector3D refDir = Amg::Vector3D{0., -1., -1.}.unit();

    if (std::abs(refDir.dot(circleNorm)) > epsilon) {
        ERROR_MSG("The norm "<<Amg::toString(circleNorm)<<" and reference dir "
                <<Amg::toString(refDir)<<" are not orthogonal "<<refDir.dot(circleNorm)<<".");
        ret_code = EXIT_FAILURE;
    }

    constexpr double r = 125._mm;


    //// Construct 3 points
    const Amg::Vector3D A = c1 + r*(Amg::AngleAxis3D{30._degree, circleNorm} * refDir);
    const Amg::Vector3D B = c1 + r*(Amg::AngleAxis3D{45._degree, circleNorm} * refDir);
    const Amg::Vector3D C = c1 + r*(Amg::AngleAxis3D{136._degree, circleNorm} * refDir);
    TEST_POINT(A);
    TEST_POINT(B);
    TEST_POINT(C);

    /// Now construct the circle
    const MuonR4::Circle circ{A, B, C};
    TEST_CIRCLE(circ);

    const MuonR4::Circle circ1{B, A, C};
    TEST_CIRCLE(circ1);
    
    const MuonR4::Circle circ2{C, B, A};
    TEST_CIRCLE(circ2);
   
    const Amg::Vector3D AC = (C-A).unit();
    const Amg::Vector3D nAC = AC.cross(circleNorm);
    const Amg::Vector3D cAC = A + (B-A).dot(AC) * AC;
    const Amg::Vector3D D = cAC - (B-cAC).dot(nAC)*nAC;
    
    const MuonR4::Circle circ3{A,D, C};
    LOG_MSG("print circle "<<circ3);
    if (circ3.normal().dot(circ.normal()) > 0.) {
        ERROR_MSG("Circle normals point into same direction "
                <<circ<<" vs. "<<circ3);
    }
    return ret_code;
}