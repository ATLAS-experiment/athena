/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GeoPrimitivesHelpers.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef GEOPRIMITIVES_GEOPRIMITIVESHELPERS_H
#define GEOPRIMITIVES_GEOPRIMITIVESHELPERS_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "GeoPrimitives/GeoPrimitivesCompare.h"
#include "CxxUtils/sincos.h"

#include "cmath"
#include <sstream>
#include <stdexcept>

#include <vector>
#include <optional>
#include <set>
#include <iostream>


/** Geometry primitives helper functions
 @author  Niels van Eldik
 @author  Robert Johannes Langenberg <robert.langenberg@cern.ch>
 @author Andreas Salzburger <Andreas.Salzburger@cern.ch>

 */

namespace Amg {



using  SetVector3D = std::set<Amg::Vector3D, Vector3DComparer>;
using  SetVectorVector3D = std::set< std::vector< Amg::Vector3D>, VectorVector3DComparer>;



/** calculates the opening angle between two vectors */
inline double angle(const Amg::Vector3D& v1, const Amg::Vector3D& v2) {
    const double dp = std::clamp(v1.dot(v2) / (v1.mag() * v2.mag()), -1. ,1.);
    return std::acos(dp);
}


/** calculates the squared distance between two point in 3D space */
inline float distance2(const Amg::Vector3D& p1, const Amg::Vector3D& p2) {
    float dx = p2.x()-p1.x(), dy = p2.y()-p1.y(), dz = p2.z()-p1.z();
    return dx*dx + dy*dy + dz*dz;
}

/** calculates the distance between two point in 3D space */
inline float distance(const Amg::Vector3D& p1, const Amg::Vector3D& p2) {
    return std::sqrt( distance2(p1, p2) );
}




/** sets the phi angle of a vector without changing theta nor the magnitude */
inline void setPhi(Amg::Vector3D& v, double phi) {
    double xy = v.perp();
    CxxUtils::sincos sc(phi);
    v[0] = xy * sc.cs;
    v[1] = xy * sc.sn;
}

/** sets the theta and phi angle of a vector without changing the magnitude */
inline void setThetaPhi(Amg::Vector3D& v, double theta, double phi) {
    double mag = v.mag();
    CxxUtils::sincos sc(phi);
    CxxUtils::sincos sct(theta);
    v[0] = mag * sct.sn * sc.cs;
    v[1] = mag * sct.sn * sc.sn;
    v[2] = mag * sct.cs;
}

/** sets radius, the theta and phi angle of a vector. Angles are measured in RADIANS */
inline void setRThetaPhi(Amg::Vector3D& v, double r, double theta, double phi) {
    CxxUtils::sincos sc(phi);
    CxxUtils::sincos sct(theta);
    v[0] = r * sct.sn * sc.cs;
    v[1] = r * sct.sn * sc.sn;
    v[2] = r * sct.cs;
}

/** sets the theta of a vector without changing phi nor the magnitude */
inline void setTheta(Amg::Vector3D& v, double theta) {
    setThetaPhi(v, theta, v.phi());
}

/** scales the vector in the xy plane without changing the z coordinate nor the angles */
inline void setPerp(Amg::Vector3D& v, double perp) {
    double p = v.perp();
    if (p != 0.0) {
        double scale = perp / p;
        v[0] *= scale;
        v[1] *= scale;
    }
}

inline double deltaPhi(const Amg::Vector3D& v1, const Amg::Vector3D& v2) {
    double dphi = v2.phi() - v1.phi();
    if (dphi > M_PI) {
        dphi -= M_PI*2;
    } else if (dphi <= -M_PI) {
        dphi += M_PI*2;
    }
    return dphi;
}
inline double deltaR(const Amg::Vector3D& v1, const Amg::Vector3D& v2){
    return std::hypot(v1.eta() - v2.eta(), deltaPhi(v1,v2));
}

/*
 * the analogous to CLHEP HepGeom::Transform3D trans (localRot, theSurface.transform().translation());
 */
inline Amg::Transform3D getTransformFromRotTransl(Amg::RotationMatrix3D rot, Amg::Vector3D transl_vec )
{
    Amg::Transform3D trans = Amg::Transform3D::Identity();
    trans = trans * rot;
    trans.translation() = transl_vec;
    return trans;
}

/*
 * Replacing the CLHEP::HepRotation::getAngleAxis() functionality
 *
 * Note:
 * CLHEP has a 'HepRotation::getAngleAxis()' function, e.g.:
 * ---
 * CLHEP::HepRotation rotation   = transform.getRotation();
 * CLHEP::Hep3Vector  rotationAxis;
 * double      rotationAngle;
 * rotation.getAngleAxis(rotationAngle,rotationAxis);
 * ---
 */
inline void getAngleAxisFromRotation(Amg::RotationMatrix3D& rotation, double& rotationAngle, Amg::Vector3D& rotationAxis)
{
    rotationAngle = 0.;

    double xx = rotation(0,0);
    double yy = rotation(1,1);
    double zz = rotation(2,2);

    double cosa  = 0.5 * (xx + yy + zz - 1);
    double cosa1 = 1 - cosa;

    if (cosa1 <= 0) {
        rotationAngle = 0;
        rotationAxis  = Amg::Vector3D(0,0,1);
    }
    else{
        double x=0, y=0, z=0;
        if (xx > cosa) x = std::sqrt((xx-cosa)/cosa1);
        if (yy > cosa) y = std::sqrt((yy-cosa)/cosa1);
        if (zz > cosa) z = std::sqrt((zz-cosa)/cosa1);
        if (rotation(2,1) < rotation(1,2)) x = -x;
        if (rotation(0,2) < rotation(2,0)) y = -y;
        if (rotation(1,0) < rotation(0,1)) z = -z;
        rotationAngle = (cosa < -1.) ? std::acos(-1.) : std::acos(cosa);
        rotationAxis  = Amg::Vector3D(x,y,z);
    }

    return;
}

/**
 * Get the Translation vector out of a Transformation
 */
inline Amg::Vector3D getTranslationVectorFromTransform(const Amg::Transform3D& tr) {
    return Amg::Vector3D(tr(0,3),tr(1,3),tr(2,3));
} // TODO: check! it's perhaps useless, you acn use the transform.translation() method



/**
 * get a AngleAxis from an angle and an axis.
 *
 * to replace the CLHEP constructor:
 * CLHEP::Rotate3D::Rotate3D(double a, cconst Vector3D< double > & v)
 */
inline Amg::Rotation3D getRotation3DfromAngleAxis(double angle, Amg::Vector3D& axis) {
    return Amg::Rotation3D{Amg::AngleAxis3D{angle, axis}};
}


/** @brief Rotate the coordinate system by an angle around the x-axis
 *  @param angle: The roation angle around x. */
inline Amg::Isometry3D getRotateX3D(double angle) {
    return Amg::Isometry3D{Amg::AngleAxis3D{angle, Amg::Vector3D::UnitX()}};
}
/** @brief Rotate the coordinate system by an angle around the z-axis
 *  @param angle: The roation angle around y. */
inline Amg::Isometry3D getRotateY3D(double angle) {
    return Amg::Isometry3D{Amg::AngleAxis3D{angle, Amg::Vector3D::UnitY()}};
}
/** @brief Rotate the coordinate system by an angle around the z-axis
 *  @param angle: The roation angle around z. */
inline Amg::Isometry3D getRotateZ3D(double angle) {
    return Amg::Isometry3D{Amg::AngleAxis3D{angle, Amg::Vector3D::UnitZ()}};
}
/** @brief: Returns a shift transformation along the x-axis*/
inline Amg::Isometry3D getTranslateX3D(const double X) {
    return Amg::Isometry3D{Amg::Translation3D{X * Amg::Vector3D::UnitX()}};
}
/** @brief: Returns a shift transformation along the y-axis*/
inline Amg::Isometry3D getTranslateY3D(const double Y) {
    return Amg::Isometry3D{Amg::Translation3D{Y * Amg::Vector3D::UnitY()}};
}
/** @brief: Returns a shift transformation along the z-axis*/
inline Amg::Isometry3D getTranslateZ3D(const double Z) {
    return Amg::Isometry3D{Amg::Translation3D{Z * Amg::Vector3D::UnitZ()}};
}
/** @brief: Returns a shift transformation along an arbitrary axis */
inline Amg::Isometry3D getTranslate3D(const double X, const double Y, const double Z) {
    return getTranslateX3D(X) * getTranslateY3D(Y) * getTranslateZ3D(Z);
}
/** @brief: Returns a shift transformation along an arbitrary axis */
inline Amg::Isometry3D getTranslate3D(const Amg::Vector3D& v) {
    return Amg::Isometry3D{Amg::Translation3D{v}};
}
/** @brief Convert a general transform into an isometric one, e.g. a GeoModel placement.
 *         GeoModel builds its placements from rotations and translations, but the
 *         type allows scaling and shearing, which the tracking geometry does not.
 *  @param trf: The transform to convert
 *  @param tolerance: Tolerance of the orthogonality check */
inline Amg::Isometry3D toIsometry3D(const Amg::Transform3D& trf,
                                    const double tolerance = 1.e-9) {
    const double deviation = (trf.linear() * trf.linear().transpose() -
                              Amg::RotationMatrix3D::Identity()).cwiseAbs().maxCoeff();
    if (deviation > tolerance) {
        std::stringstream msg{};
        msg<<__FILE__<<":"<<__LINE__<<" --- Transform is not isometric, its linear part "
           <<"deviates from orthogonal by "<<deviation<<".";
        throw std::runtime_error(msg.str());
    }
    Amg::Isometry3D iso{Amg::Isometry3D::Identity()};
    iso.linear() = trf.linear();
    iso.translation() = trf.translation();
    return iso;
}

/** @brief Constructs a direction vector from the azimuthal & polar angles
 *  @param phi: Polar angle in the x-y plane
 *  @param theta: Azimuthal angle in the r-z plane */
inline Amg::Vector3D dirFromAngles(const double phi, const double theta) {
    const CxxUtils::sincos thetaCS{theta}, phiCS{phi};
    return Amg::Vector3D{phiCS.cs * thetaCS.sn, phiCS.sn* thetaCS.sn, thetaCS.cs};
}

/** @brief Project the direction vector onto the plane and renormalize
  *         to unity
  * @param direction: The direction vector to project
  * @param planeNorm: The normal vector representing the plane */
inline Amg::Vector3D projectDirOntoPlane(const Amg::Vector3D& direction,
                                         const Amg::Vector3D& planeNorm) {
    return (direction - planeNorm.dot(direction) * planeNorm).unit();
}

/** @brief: Calculates the shortest distance between two lines
    @param posA: offset point of line A
    @param dirA: orientation of line A (unit length)
    @param posB: offset point of line B
    @param dirB: orientation of line B (unit length)  */
template<int N> double lineDistance(const AmgVector(N)& posA,
                                    const AmgVector(N)& dirA,
                                    const AmgVector(N)& posB,
                                    const AmgVector(N)& dirB) {

    const double dirDots = dirA.dot(dirB);
    const double divisor = (1. - dirDots * dirDots);
    const AmgVector(N) AminusB = posA - posB;
    if (std::abs(divisor) < std::numeric_limits<double>::epsilon()) {
        const AmgVector(N) d = AminusB - dirA.dot(AminusB)*dirA;
        return std::sqrt(d.dot(d));
    }
    const AmgVector(N) lineTravel = AminusB.dot(dirA) * dirA -
                                    AminusB.dot(dirB) * dirB;
    return std::sqrt(std::max(0., AminusB.dot(AminusB) - lineTravel.dot(lineTravel) / divisor));
}
/** @brief Calculates the signed distance between two lines in 3D space 
    @param posA: offset point of line A
    @param dirA: orientation of line A (unit length)
    @param posB: offset point of line B
    @param dirB: orientation of line B (unit length)  */
inline double signedDistance(const Amg::Vector3D& posA,
                             const Amg::Vector3D& dirA,
                             const Amg::Vector3D& posB,
                             const Amg::Vector3D& dirB) {
    /** Project the first direction onto the second & renormalize to a unit vector */
    const double dirDots = dirA.dot(dirB);
    const Amg::Vector3D AminusB = posA - posB;
    if (std::abs(dirDots -1.) < std::numeric_limits<float>::epsilon()){
        return (AminusB - dirA.dot(AminusB)*dirA).mag();
    }
    const Amg::Vector3D projDir = (dirA - dirDots*dirB).unit();
    return AminusB.cross(dirB).dot(projDir);
}  
/** @brief Calculates the point B' along the line B that's closest to a second line A 
    @param posA: offset point of line A
    @param dirA: orientation of line A (unit length)
    @param posB: offset point of line B
    @param dirB: orientation of line B (unit length) */
template <int N> std::optional<double> intersect(const AmgVector(N)& posA, 
                                                 const AmgVector(N)& dirA, 
                                                 const AmgVector(N)& posB, 
                                                 const AmgVector(N)& dirB) {
    //// Use the formula
    ///    A + lambda dirA  = B + mu dirB
    ///    (A-B) + lambda dirA = mu dirB
    ///    <A-B, dirB> + lambda <dirA,dirB> = mu
    ///     A + lambda dirA = B + (<A-B, dirB> + lambda <dirA,dirB>)dirB
    ///     <A-B,dirA> + lambda <dirA, dirA> = <A-B, dirB><dirA,dirB> + lamda<dirA,dirB><dirA,dirB>
    ///   -> lambda = -(<A-B, dirA> - <A-B, dirB> * <dirA, dirB>) / (1- <dirA,dirB>^2)
    ///   --> mu    =  (<A-B, dirB> - <A-B, dirA> * <dirA, dirB>) / (1- <dirA,dirB>^2)
    const double dirDots = dirA.dot(dirB);
    const double divisor = (1. - dirDots * dirDots);
    /// If the two directions are parallel to each other there's no way of intersection
    if (std::abs(divisor) < std::numeric_limits<double>::epsilon()) return std::nullopt;
    const AmgVector(N) AminusB = posA - posB;
    return (AminusB.dot(dirB) - AminusB.dot(dirA) * dirDots) / divisor;
}
/// Intersects a line parametrized as A + lambda * B with the (N-1) dimensional
/// hyperplane that's given in the Hesse normal form <P, N> - C = 0 
template <int N>
std::optional<double> intersect(const AmgVector(N)& pos, 
                                const AmgVector(N)& dir, 
                                const AmgVector(N)& planeNorm, 
                                const double offset) {
    ///  <P, N> - C = 0
    /// --> <A + lambda *B , N> - C = 0
    /// --> lambda = (C - <A,N> ) / <N, B>
    const double normDot = planeNorm.dot(dir); 
    if (std::abs(normDot) < std::numeric_limits<double>::epsilon()) return std::nullopt;
    return (offset - pos.dot(planeNorm)) / normDot;
}

/// Checks whether the linear part of the transformation rotates or stetches
/// any of the basis vectors.
inline bool doesNotDeform(const Amg::Transform3D& trans) {
    for (unsigned int d = 0; d < 3 ; ++d) {
        const double defLength = Amg::Vector3D::Unit(d).dot(trans.linear() * Amg::Vector3D::Unit(d));
        if (std::abs(defLength - 1.) > std::numeric_limits<float>::epsilon()) {
            return false;
        }
    }
    return true;
}
/// Checks whether the transformation is the Identity transformation
inline bool isIdentity(const Amg::Transform3D& trans) {
    return doesNotDeform(trans) && 
           trans.translation().mag() < std::numeric_limits<float>::epsilon();
}


} // end of Amg namespace

#endif
