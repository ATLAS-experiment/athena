/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GMTreeBrowser.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "TrkDetDescrGeoModelCnv/GMTreeBrowser.h"
// GeoModel includes
#include <cmath>
#include <iomanip>
#include <iostream>

#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoCons.h"
#include "GeoModelKernel/GeoPara.h"
#include "GeoModelKernel/GeoPcon.h"
#include "GeoModelKernel/GeoPgon.h"
#include "GeoModelKernel/GeoShape.h"
#include "GeoModelKernel/GeoShapeIntersection.h"
#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelKernel/GeoShapeSubtraction.h"
#include "GeoModelKernel/GeoShapeUnion.h"
#include "GeoModelKernel/GeoSimplePolygonBrep.h"
#include "GeoModelKernel/GeoTrap.h"
#include "GeoModelKernel/GeoTrd.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoTubs.h"
#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelUtilities/GeoVisitVolumes.h"

int Trk::GMTreeBrowser::compareGeoVolumes(const GeoVPhysVol* gv1,
                                          const GeoVPhysVol* gv2,
                                          double tolerance, bool dumpInfo,
                                          int level) const {

    int diff = 0;

    // CASE 1: naming difference
    if (gv1->getLogVol()->getName() != gv2->getLogVol()->getName()) {
        diff = 1000 * level + 1;
        if (dumpInfo) {
            std::cout << "CASE 1: names differ at level:" << level << ":"
                      << gv1->getLogVol()->getName() << ":"
                      << gv2->getLogVol()->getName() << std::endl;
        }
        //  else return m_diff;    // the naming change is harmless and can mask
        //  more serious problems, continue the loop
    }
    // CASE 2: material type difference
    if (gv1->getLogVol()->getMaterial()->getName() !=
        gv2->getLogVol()->getMaterial()->getName()) {
        diff = 1000 * level + 2;
        if (dumpInfo) {
            std::cout << "CASE 2: material types differ for volume:"
                      << gv1->getLogVol()->getName() << ":at level:" << level
                      << ":" << gv1->getLogVol()->getMaterial()->getName()
                      << ":differs from:"
                      << gv2->getLogVol()->getMaterial()->getName()
                      << std::endl;
        } else
            return diff;
    }
    //  CASE 3: shape type difference
    if (gv1->getLogVol()->getShape()->typeID() !=
        gv2->getLogVol()->getShape()->typeID()) {
        diff = 1000 * level + 3;
        if (dumpInfo) {
            std::cout << "CASE 3: shape types differ at level:" << level
                      << ":volume name:" << gv1->getLogVol()->getName()
                      << ":shape:" << gv1->getLogVol()->getShape()->type()
                      << ":shape ref:" << gv2->getLogVol()->getShape()->type()
                      << std::endl;
        } else
            return diff;
    }
    //  CASE 4: difference in shape definition
    if (!compareShapes(gv1->getLogVol()->getShape(),
                       gv2->getLogVol()->getShape(), tolerance)) {
        diff = 1000 * level + 4;
        if (dumpInfo) {
            std::cout << "CASE 4: shape definition differ at level:" << level
                      << std::endl;
        } else
            return diff;
    }
    unsigned int nChild1 = gv1->getNChildVols();
    // CASE 5: difference in the number of child volumes
    if (nChild1 != gv2->getNChildVols()) {
        diff = 1000 * level + 5;
        if (dumpInfo) {
            std::cout << "CASE 5: number of child vols differ at level:"
                      << level << ":volume name:" << gv1->getLogVol()->getName()
                      << ":nChildVols:" << gv1->getNChildVols()
                      << ":nChildVols ref:" << gv2->getNChildVols()
                      << std::endl;
        } else
            return diff;
    }

    // CASE 6 & 7: transform to child difference
    // We used to do this with something like
    //  for (unsigned int ic = 0; ic < gv1->getNChildVols(); ic++) {
    //    GeoTrf::Transform3D transf1 = gv1->getXToChildVol(ic);
    //    const GeoVPhysVol* cv1 = &(*(gv1->getChildVol(ic)));
    //
    // But getXToChildVol and getChildVol need to walk all the children
    // until they reach the given index.  So this would be N^2,
    // and each time we repeat it for the transform and the volume.
    // Better to use geoGetVolumes so that we only need do the walk once.
    // And examination of profiling data shows that almost never
    // fail a comparison from here on, so we'll almost always be examining
    // all children anyway.
    // (It would be even better if GeoVPhysVol has some sort of iterator
    // interface.  Maybe we can use a generator with C++23...)
    GeoVolumeVec_t children1 = geoGetVolumes (gv1, 1, nChild1);
    GeoVolumeVec_t children2 = geoGetVolumes (gv2, 1, nChild1);
    assert (children1.size() == nChild1 && children2.size() == nChild1);
    for (unsigned int ic = 0; ic < nChild1; ic++) {
        GeoTrf::Transform3D& transf1 = children1.at(ic).second;
        GeoTrf::Transform3D& transf2 = children2.at(ic).second;

        const GeoVPhysVol* cv1 = children1.at(ic).first;
        const GeoVPhysVol* cv2 = children2.at(ic).first;

        if ((transf1.translation() - transf2.translation()).norm() >
            tolerance) {
            diff = 1000 * level + 10 * ic + 6;
            if (dumpInfo) {
                std::cout << "CASE 6: translation differs at level:" << level
                          << ": between mother and child:"
                          << gv1->getLogVol()->getName() << ":"
                          << cv1->getLogVol()->getName() << std::endl;
                this->printTranslationDiff(transf1, transf2, tolerance);
            } else
                return diff;
        }
        // For rotation matrices, transpose is the same as inverse.
        GeoTrf::RotationMatrix3D rot =
            transf1.rotation() * transf2.rotation().transpose();
        if (std::abs(rot(0, 1)) > tolerance ||
            std::abs(rot(0, 2)) > tolerance ||
            std::abs(rot(1, 2)) > tolerance) {
            diff = 1000 * level + 10 * ic + 7;
            if (dumpInfo) {
                std::cout << "CASE 7: rotation differs at level:" << level
                          << ":between mother and child:"
                          << gv1->getLogVol()->getName() << ":"
                          << cv1->getLogVol()->getName() << std::endl;
                this->printRotationDiff(transf1, transf2, tolerance);
            } else
                return diff;
        }

        int child_comp =
            compareGeoVolumes(cv1, cv2, tolerance, dumpInfo, level + 1);
        if (child_comp != 0)
            return child_comp;
    }

    return diff;
}

bool Trk::GMTreeBrowser::compareShapes(const GeoShape* sh1, const GeoShape* sh2,
                                       double tol) const {

    if (sh1->typeID() != sh2->typeID())
        return false;

    if (sh1->type() == "Pgon") {
        const GeoPgon* pgon1 = static_cast<const GeoPgon*>(sh1);
        const GeoPgon* pgon2 = static_cast<const GeoPgon*>(sh2);
        if (!pgon1 || !pgon2)
            return false;

        if (pgon1->getNPlanes() != pgon2->getNPlanes())
            return false;
        if (pgon1->getNSides() != pgon2->getNSides())
            return false;
        if (std::abs(pgon1->getSPhi() - pgon2->getSPhi()) > tol)
            return false;
        if (std::abs(pgon1->getDPhi() - pgon2->getDPhi()) > tol)
            return false;

        return true;

    } else if (sh1->type() == "Trd") {
        const GeoTrd* trd1 = static_cast<const GeoTrd*>(sh1);
        const GeoTrd* trd2 = static_cast<const GeoTrd*>(sh2);

        if (std::abs(trd1->getXHalfLength1() - trd2->getXHalfLength1()) > tol)
            return false;
        if (std::abs(trd1->getXHalfLength2() - trd2->getXHalfLength2()) > tol)
            return false;
        if (std::abs(trd1->getYHalfLength1() - trd2->getYHalfLength1()) > tol)
            return false;
        if (std::abs(trd1->getYHalfLength2() - trd2->getYHalfLength2()) > tol)
            return false;
        if (std::abs(trd1->getZHalfLength() - trd2->getZHalfLength()) > tol)
            return false;

        return true;

    } else if (sh1->type() == "Box") {
        const GeoBox* box1 = static_cast<const GeoBox*>(sh1);
        const GeoBox* box2 = static_cast<const GeoBox*>(sh2);

        if (std::abs(box1->getXHalfLength() - box2->getXHalfLength()) > tol)
            return false;
        if (std::abs(box1->getYHalfLength() - box2->getYHalfLength()) > tol)
            return false;
        if (std::abs(box1->getZHalfLength() - box2->getZHalfLength()) > tol)
            return false;

        return true;

    } else if (sh1->type() == "Tube") {
        const GeoTube* tube1 = static_cast<const GeoTube*>(sh1);
        const GeoTube* tube2 = static_cast<const GeoTube*>(sh2);

        if (std::abs(tube1->getRMin() - tube2->getRMin()) > tol)
            return false;
        if (std::abs(tube1->getRMax() - tube2->getRMax()) > tol)
            return false;
        if (std::abs(tube1->getZHalfLength() - tube2->getZHalfLength()) > tol)
            return false;

        return true;

    } else if (sh1->type() == "Tubs") {
        const GeoTubs* tubs1 = static_cast<const GeoTubs*>(sh1);
        const GeoTubs* tubs2 = static_cast<const GeoTubs*>(sh2);

        if (std::abs(tubs1->getRMin() - tubs2->getRMin()) > tol)
            return false;
        if (std::abs(tubs1->getRMax() - tubs2->getRMax()) > tol)
            return false;
        if (std::abs(tubs1->getZHalfLength() - tubs2->getZHalfLength()) > tol)
            return false;
        if (std::abs(tubs1->getSPhi() - tubs2->getSPhi()) > tol)
            return false;
        if (std::abs(tubs1->getDPhi() - tubs2->getDPhi()) > tol)
            return false;

        return true;

    } else if (sh1->type() == "Cons") {
        const GeoCons* cons1 = static_cast<const GeoCons*>(sh1);
        const GeoCons* cons2 = static_cast<const GeoCons*>(sh2);

        if (std::abs(cons1->getRMin1() - cons2->getRMin1()) > tol)
            return false;
        if (std::abs(cons1->getRMin2() - cons2->getRMin2()) > tol)
            return false;
        if (std::abs(cons1->getRMax1() - cons2->getRMax1()) > tol)
            return false;
        if (std::abs(cons1->getRMax2() - cons2->getRMax2()) > tol)
            return false;
        if (std::abs(cons1->getDZ() - cons2->getDZ()) > tol)
            return false;
        if (std::abs(cons1->getSPhi() - cons2->getSPhi()) > tol)
            return false;
        if (std::abs(cons1->getDPhi() - cons2->getDPhi()) > tol)
            return false;

        return true;

    } else if (sh1->type() == "SimplePolygonBrep") {
        const GeoSimplePolygonBrep* spb1 =
            static_cast<const GeoSimplePolygonBrep*>(sh1);
        const GeoSimplePolygonBrep* spb2 =
            static_cast<const GeoSimplePolygonBrep*>(sh2);
        if (!spb1 || !spb2)
            return false;

        unsigned int nv1 = spb1->getNVertices();
        unsigned int nv2 = spb2->getNVertices();
        if (nv1 != nv2)
            return false;
        if (std::abs(spb1->getDZ() - spb2->getDZ()) > tol)
            return false;

        for (unsigned int iv = 0; iv < nv1; iv++) {

            if (std::abs(spb1->getXVertex(iv) - spb2->getXVertex(iv)) > tol)
                return false;
            if (std::abs(spb1->getYVertex(iv) - spb2->getYVertex(iv)) > tol)
                return false;
        }

        return true;

    } else if (sh1->type() == "Pcon") {
        const GeoPcon* pc1 = static_cast<const GeoPcon*>(sh1);
        const GeoPcon* pc2 = static_cast<const GeoPcon*>(sh2);
        if (!pc1 || !pc2)
            return false;

        if (std::abs(pc1->getSPhi() - pc2->getSPhi()) > tol)
            return false;
        if (std::abs(pc1->getDPhi() - pc2->getDPhi()) > tol)
            return false;

        unsigned int nv1 = pc1->getNPlanes();
        unsigned int nv2 = pc2->getNPlanes();
        if (nv1 != nv2)
            return false;

        for (unsigned int iv = 0; iv < nv1; iv++) {

            if (std::abs(pc1->getZPlane(iv) - pc2->getZPlane(iv)) > tol)
                return false;
            if (std::abs(pc1->getRMinPlane(iv) - pc2->getRMinPlane(iv)) > tol)
                return false;
            if (std::abs(pc1->getRMaxPlane(iv) - pc2->getRMaxPlane(iv)) > tol)
                return false;
        }

        return true;

    } else if (sh1->type() == "Subtraction") {
        const GeoShapeSubtraction* sub1 =
            static_cast<const GeoShapeSubtraction*>(sh1);
        const GeoShapeSubtraction* sub2 =
            static_cast<const GeoShapeSubtraction*>(sh2);

        if (!sub1 || !sub2)
            return false;

        if (!compareShapes(sub1->getOpA(), sub2->getOpA(), tol))
            return false;
        if (!compareShapes(sub1->getOpB(), sub2->getOpB(), tol))
            return false;

        return true;

    } else if (sh1->type() == "Union") {
        const GeoShapeUnion* sub1 = static_cast<const GeoShapeUnion*>(sh1);
        const GeoShapeUnion* sub2 = static_cast<const GeoShapeUnion*>(sh2);

        if (!sub1 || !sub2)
            return false;

        if (!compareShapes(sub1->getOpA(), sub2->getOpA(), tol))
            return false;
        if (!compareShapes(sub1->getOpB(), sub2->getOpB(), tol))
            return false;

        return true;

    } else if (sh1->type() == "Shift") {
        const GeoShapeShift* shift1 = static_cast<const GeoShapeShift*>(sh1);
        const GeoShapeShift* shift2 = static_cast<const GeoShapeShift*>(sh2);

        if (!shift1 || !shift2)
            return false;

        if (!compareShapes(shift1->getOp(), shift2->getOp(), tol))
            return false;

        const GeoTrf::Transform3D& transf1 = shift1->getX();
        const GeoTrf::Transform3D& transf2 = shift2->getX();

        if ((transf1.translation() - transf2.translation()).norm() > tol)
            return false;

        // For rotation matrices, transpose is the same as inverse.
        if (!identity_check(transf1.rotation() * transf2.rotation().transpose(),
                            tol))
            return false;

        return true;
    }

    std::cout << "unknown shape to compare:" << sh1->type() << std::endl;

    return false;
}

bool Trk::GMTreeBrowser::findNamePattern(const GeoVPhysVol* gv,
                                         std::string_view name) const {

    if (gv->getLogVol()->getName().find(name) != std::string::npos)
        return true;

    for (unsigned int ic = 0; ic < gv->getNChildVols(); ic++) {

        const GeoVPhysVol* cv = &(*(gv->getChildVol(ic)));
        if (this->findNamePattern(cv, name))
            return true;
    }

    return false;
}

const GeoVPhysVol* Trk::GMTreeBrowser::findTopBranch(
    const GeoVPhysVol* gv, std::string_view name) const {

    const GeoVPhysVol* topName = nullptr;

    for (unsigned int ic = 0; ic < gv->getNChildVols(); ic++) {

        const GeoVPhysVol* cv = &(*(gv->getChildVol(ic)));
        const GeoLogVol* clv = cv->getLogVol();
        if (clv && clv->getName().find(name) != std::string::npos)
            return gv;
        topName = this->findTopBranch(cv, name);
        if (topName)
            return topName;
    }

    return topName;
}

bool Trk::GMTreeBrowser::identity_check(GeoTrf::RotationMatrix3D rotation,
                                        double tol) const {

    if (std::abs(rotation(0, 1)) > tol)
        return false;
    if (std::abs(rotation(0, 2)) > tol)
        return false;
    if (std::abs(rotation(1, 2)) > tol)
        return false;

    return true;
}

void Trk::GMTreeBrowser::printTranslationDiff(GeoTrf::Transform3D tr_test,
                                              GeoTrf::Transform3D tr_ref,
                                              double tolerance) const {
    std::ios oldState(nullptr);
    oldState.copyfmt(std::cout);
    //
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "test translation:x:y:z:" << tr_test.translation().x() << ":"
              << tr_test.translation().y() << ":" << tr_test.translation().z()
              << std::endl;
    std::cout << " ref  translation:x:y:z:" << tr_ref.translation().x() << ":"
              << tr_ref.translation().y() << ":" << tr_ref.translation().z()
              << std::endl;
    std::cout << " absolute shift :"
              << (tr_test.translation() - tr_ref.translation()).norm()
              << ": to be compared with the tolerance limit:" << tolerance
              << std::endl;
    std::cout.copyfmt(oldState);  // restore ostream state
}

void Trk::GMTreeBrowser::printRotationDiff(GeoTrf::Transform3D tr_test,
                                           GeoTrf::Transform3D tr_ref,
                                           double tolerance) const {

    GeoTrf::RotationMatrix3D rotest = tr_test.rotation();
    GeoTrf::RotationMatrix3D rotref = tr_ref.rotation();
    GeoTrf::RotationMatrix3D rotid = rotest * rotref.inverse();
    std::ios oldState(nullptr);
    oldState.copyfmt(std::cout);

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "test rotation:" << rotest(0, 0) << ":" << rotest(0, 1) << ":"
              << rotest(0, 2) << std::endl;
    std::cout << "                   " << rotest(1, 0) << ":" << rotest(1, 1)
              << ":" << rotest(1, 2) << std::endl;
    std::cout << "                   " << rotest(2, 0) << ":" << rotest(2, 1)
              << ":" << rotest(2, 2) << std::endl;
    std::cout << " ref rotation:" << rotref(0, 0) << ":" << rotref(0, 1) << ":"
              << rotref(0, 2) << std::endl;
    std::cout << "                   " << rotref(1, 0) << ":" << rotref(1, 1)
              << ":" << rotref(1, 2) << std::endl;
    std::cout << "                   " << rotref(2, 0) << ":" << rotref(2, 1)
              << ":" << rotref(2, 2) << std::endl;
    std::cout << "test*inv(ref):" << rotid(0, 0) << ":" << rotid(0, 1) << ":"
              << rotid(0, 2) << std::endl;
    std::cout << "                   " << rotid(1, 0) << ":" << rotid(1, 1)
              << ":" << rotid(1, 2) << std::endl;
    std::cout << "                   " << rotid(2, 0) << ":" << rotid(2, 1)
              << ":" << rotid(2, 2) << std::endl;
    std::cout << " identity check fails within the tolerance limit:"
              << tolerance << std::endl;
    std::cout.copyfmt(oldState);  // restore ostream state
}
