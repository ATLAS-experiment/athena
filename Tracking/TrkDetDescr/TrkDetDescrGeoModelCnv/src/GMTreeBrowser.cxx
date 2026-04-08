/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GMTreeBrowser.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "TrkDetDescrGeoModelCnv/GMTreeBrowser.h"

#include "GeoModelHelpers/GeoShapeSorter.h"
#include "GeoModelHelpers/TransformSorter.h"
#include "GeoModelHelpers/GeoMaterialSorter.h"
#include "GeoModelHelpers/getChildNodesWithTrf.h"

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
#include "CxxUtils/inline_hints.h"

#if defined(FLATTEN)
    // We compile this package with optimization, even in debug builds; otherwise,
    // the heavy use of Eigen makes it too slow.  However, from here we may call
    // to out-of-line Eigen code that is linked from other DSOs; in that case,
    // it would not be optimized.  Avoid this by forcing all Eigen code
    // to be inlined here if possible.
    ATH_FLATTEN
#endif
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
    if (GeoMaterialSorter{}.compare(gv1->getLogVol()->getMaterial(),
                                    gv2->getLogVol()->getMaterial()) != 0) {
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
    //  CASE 3: difference in shape definition
    if (!compareShapes(gv1->getLogVol()->getShape(),
                       gv2->getLogVol()->getShape(), tolerance)) {
        diff = 1000 * level + 3;
        if (dumpInfo) {
            std::cout << "CASE 4: shape definition differ at level:" << level
                      << std::endl;
        } else
            return diff;
    }
    unsigned int nChild1 = gv1->getNChildVols();
    // CASE 4: difference in the number of child volumes
    if (nChild1 != gv2->getNChildVols()) {
        diff = 1000 * level + 4;
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


    const auto children1 = getChildrenWithRef(gv1);
    const auto children2 = getChildrenWithRef(gv2);
   
    if (children1.size() != children2.size()) {
        diff = 1000 * level + 5;
        if (dumpInfo) {
            std::cout << "CASE 5: number of child vols differ at level:"
                      << level << ":volume name:" << gv1->getLogVol()->getName()
                      << ":nChildVols:" << gv1->getNChildVols()
                      << ":nChildVols ref:" << gv2->getNChildVols()
                      << std::endl;
        } 
        return diff;
    }
   
    for (std::size_t ic = 0; ic < children1.size(); ++ic) {
        const auto& child1 = children1.at(ic);
        const auto& child2 = children2.at(ic);
        
        if (child1.nCopies != child2.nCopies) {
            diff = 1000 * level + 6;
            if (dumpInfo) {
                std::cout << "CASE 5: number of child vols differ at level:"
                          << level << ":volume name:" << child1.volume->getLogVol()->getName()
                          << std::endl;
            } 
            return diff;
        } 
        if (child1.nCopies > 1 && 
            GeoTrf::TransformSorter{}.compare(child1.inductionRule, child2.inductionRule) !=0) {
            diff = 1000 * level + 7;
            if (dumpInfo) {
                std::cout << "CASE 7: Replication rule  is different at level:"
                          << level << ":volume name:" << child1.volume->getLogVol()->getName()
                          << std::endl;
            } 
            return diff;
        }
        if (GeoTrf::TransformSorter{}.compare(child1.transform, child2.transform) != 0) {
            diff = 1000 * level + 7;
            if (dumpInfo) {
                std::cout << "CASE 7:  Volume transform is different:"
                          << level << ":volume name:" << child1.volume->getLogVol()->getName()
                          << std::endl;
            } 
            return diff;
        }
        int child_comp = compareGeoVolumes(child1.volume, child2.volume, tolerance, dumpInfo, level + 1);
        if (child_comp != 0) {
            return child_comp;
        }
    }

    return diff;
}

#if defined(FLATTEN)
    // We compile this package with optimization, even in debug builds; otherwise,
    // the heavy use of Eigen makes it too slow.  However, from here we may call
    // to out-of-line Eigen code that is linked from other DSOs; in that case,
    // it would not be optimized.  Avoid this by forcing all Eigen code
    // to be inlined here if possible.
    ATH_FLATTEN
#endif
bool Trk::GMTreeBrowser::compareShapes(const GeoShape* sh1, const GeoShape* sh2,
                                       double /*tol*/) const {
    return GeoShapeSorter{}.compare(sh1, sh2) == 0;
}

bool Trk::GMTreeBrowser::findNamePattern(const GeoVPhysVol* gv,
                                         std::string_view name) const {

    if (gv->getLogVol()->getName().find(name) != std::string::npos)
        return true;

    for (unsigned int ic = 0; ic < gv->getNChildVols(); ic++) {
        const GeoVPhysVol* cv = gv->getChildVol(ic);
        if (findNamePattern(cv, name))
            return true;
    }

    return false;
}

namespace {

class GeoFindTopName : public GeoNodeAction
{
public:
  explicit GeoFindTopName (const std::string_view name) : m_name (name)
  {
  }
  virtual void handlePhysVol (const GeoPhysVol* v) override
  { handleVol (v); }
  virtual void handleFullPhysVol (const GeoFullPhysVol* v) override
  { handleVol (v); }
  const GeoVPhysVol* topName() const { return m_topName; }
private:
  void handleVol (const GeoVPhysVol* v)
  {
    const GeoLogVol* clv = v->getLogVol();
    if (clv && clv->getName().find(m_name) != std::string::npos) {
      m_topName = v->getParent();
      this->terminate();
    }
  }
  const std::string_view m_name;
  const GeoVPhysVol* m_topName = nullptr;
};

}

const GeoVPhysVol* Trk::GMTreeBrowser::findTopBranch(
    const GeoVPhysVol* gv, std::string_view name) {

    GeoFindTopName topName (name);
    gv->exec (&topName);
    return topName.topName();
}

bool Trk::GMTreeBrowser::identity_check(GeoTrf::RotationMatrix3D rotation,
                                        double tol) {

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
                                              double tolerance) {
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

void Trk::GMTreeBrowser::printRotationDiff(const GeoTrf::Transform3D& tr_test,
                                           const GeoTrf::Transform3D& tr_ref,
                                           double tolerance) {

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
