/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GMTreeBrowser.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRGEOMODELCNV_GMTREEBROWSER_H
#define TRKDETDESCRGEOMODELCNV_GMTREEBROWSER_H

#include <GeoModelKernel/GeoDefinitions.h>

#include <vector>

class GeoVPhysVol;
class GeoShape;

namespace Trk {

/**
  @class GMTreeBrowser

  A Helper Class that facilitates navigation through GeoModel tree.
  To be replaced by equivalent GeoModel functionality when available.

  @author  Sarka.Todorova@cern.ch
  */

class GMTreeBrowser {

   public:
    /** Default constructor*/
    GMTreeBrowser() = default;

    /** Destructor*/
    ~GMTreeBrowser() = default;

    /** Recursive comparison of trees/branches/volumes :
        in quiet mode (printFullInfo=False) , returns the indicator of first
       encountered difference ( 0 if none), in verbose mode
       (printFullInfo=True), returns the indicator of last encountered
       difference ( 0 if none) level argument is used to find the depth of the
       difference in the tree */
    int compareGeoVolumes(const GeoVPhysVol* gv1, const GeoVPhysVol* gv2,
                          double tolerance, bool printFullInfo = false,
                          int level = 0) const;

    /** shape comparison */
    bool compareShapes(const GeoShape* gs1, const GeoShape* gv2,
                       double tolerance) const;

    /** search of matching name patterns */
    bool findNamePattern(const GeoVPhysVol* gv, std::string_view name) const;

    /** search of top branch : returns mother volume for children matching name
     */
    const GeoVPhysVol* findTopBranch(const GeoVPhysVol* gv,
                                     std::string_view name) const;

   private:
    /** check of rotation invariance */
    bool identity_check(GeoTrf::RotationMatrix3D rotation, double tol) const;
    /** printout diff - unify output */
    void printTranslationDiff(GeoTrf::Transform3D trtest,
                              GeoTrf::Transform3D trref,
                              double tolerance) const;
    void printRotationDiff(GeoTrf::Transform3D trtest,
                           GeoTrf::Transform3D trref, double tolerance) const;
};

}  // end of namespace Trk

#endif
