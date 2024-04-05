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

/** scales the vector length without changing the angles */
inline void setMag(Amg::Vector3D& v, double mag) {
    double p = v.mag();
    if (p != 0.0) {
        double scale = mag / p;
        v[0] *= scale;
        v[1] *= scale;
        v[2] *= scale;
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
    double a = v1.eta() - v2.eta();
    double b = deltaPhi(v1,v2);
    return sqrt(a*a + b*b);
}






/**
 * Sets components in cartesian coordinate system.
 */
inline void setVector3DCartesian(Amg::Vector3D& v1, double x1, double y1, double z1) { v1[0] = x1; v1[1] = y1; v1[2] = z1; }
/**
 * Gets magnitude squared of the vector.
 */
inline double mag2Vector3D(const Amg::Vector3D& v1) { return v1.x()*v1.x() + v1.y()*v1.y() + v1.z()*v1.z(); }
/**
 * Gets magnitude of the vector.
 */
inline double magVector3D(const Amg::Vector3D& v1) { return std::sqrt(mag2Vector3D(v1)); }
/**
 * Gets r-component in spherical coordinate system
 */
inline double rVector3D(const Amg::Vector3D& v1) { return magVector3D(v1); }

/**
 * Transform a point from a Trasformation3D
 *
 * from CLHEP::Point3D::transform:
 * http://proj-clhep.web.cern.ch/proj-clhep/doc/CLHEP_2_0_4_7/doxygen/html/Point3D_8cc-source.html#l00032
 */
inline Amg::Vector3D transform( Amg::Vector3D& v, Amg::Transform3D& tr ) {
    Amg::Vector3D vect;
    double vx = v.x(), vy = v.y(), vz = v.z();
    setVector3DCartesian( vect,
            tr(0,0)*vx + tr(0,1)*vy + tr(0,2)*vz + tr(0,3),
            tr(1,0)*vx + tr(1,1)*vy + tr(1,2)*vz + tr(1,3),
            tr(2,0)*vx + tr(2,1)*vy + tr(2,2)*vz + tr(2,3));
    return vect;
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
inline Amg::Rotation3D getRotation3DfromAngleAxis(double angle, Amg::Vector3D& axis)
{
    AngleAxis3D t;
    t = Eigen::AngleAxis<double>(angle,axis);

    Amg::Rotation3D rot;
    rot = t;

    return rot;
}


/**
 * get a rotation transformation around X-axis
 */
inline Amg::Transform3D getRotateX3D(double angle) {
    Amg::Transform3D transf;
    Amg::AngleAxis3D angleaxis(angle, Amg::Vector3D::UnitX());
    transf = angleaxis;
    return transf;
}
/**
 * get a rotation transformation around Y-axis
 */
inline Amg::Transform3D getRotateY3D(double angle) {
    Amg::Transform3D transf;
    Amg::AngleAxis3D angleaxis(angle, Amg::Vector3D::UnitY());
    transf = angleaxis;
    return transf;
}
/**
 * get a rotation transformation around Z-axis
 */
inline Amg::Transform3D getRotateZ3D(double angle) {
    Amg::Transform3D transf;
    Amg::AngleAxis3D angleaxis(angle, Amg::Vector3D::UnitZ());
    transf = angleaxis;
    return transf;
}
/** @brief: Returns a shift transformation along the x-axis*/
inline Amg::Transform3D getTranslateX3D(const double X) {
    return Amg::Transform3D{Amg::Translation3D{X * Amg::Vector3D::UnitX()}};
}
/** @brief: Returns a shift transformation along the y-axis*/
inline Amg::Transform3D getTranslateY3D(const double Y) {
    return Amg::Transform3D{Amg::Translation3D{Y * Amg::Vector3D::UnitY()}};
}
/** @brief: Returns a shift transformation along the z-axis*/
inline Amg::Transform3D getTranslateZ3D(const double Z) {
    return Amg::Transform3D{Amg::Translation3D{Z * Amg::Vector3D::UnitZ()}};
}
/** @brief: Returns a shift transformation along an arbitrary axis */
inline Amg::Transform3D Amg::getTranslate3D(const double X, const double Y, const double Z) {
    return getTranslateX3D(X) * getTranslateY3D(Y) * getTranslateZ3D(Z);
}
/** @brief: Returns a shift transformation along an arbitrary axis */
inline Amg::Transform3D Amg::getTranslate3D(const Amg::Vector3D& v) {
    return Amg::Transform3D{Amg::Translation3D{v}};
}
/// Calculates the closest approach of two lines. 
///    posA: offset point of line A
///    dirA: orientation of line A (unit length)
///    posB: offset point of line B
///    dirB: orientation of line B (unit length)
/// Returns the length to be travelled along line B
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
    ///   -> lambda = (<A-B, dirA> - <A-B, dirB> * <dirA, dirB>) / (1- <dirA,dirB>^2)
    ///   --> mu   =  (<A-B, dirB> - <A-B, dirA> * <dirA, dirB>) / (1- <dirA,dirB>^2)
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
