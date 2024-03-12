/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// VolumeIntersection.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRGEOMODELCNV_VOLUMEINTERSECTION_H
#define TRKDETDESCRGEOMODELCNV_VOLUMEINTERSECTION_H

#include <memory>
#include <utility>  //for std::pair
#include <vector>

#include "TrkDetDescrGeoModelCnv/GeoShapeConverter.h"

namespace Trk {
class Volume;
}

namespace Trk {

struct PolygonCache {
    double hZ{0.};
    double minZ{0.};
    double maxZ{0.};
    Amg::Vector3D center{Amg::Vector3D::Zero()};
    int nVtx{0};
    std::vector<Amg::Vector3D> vertices{};
    std::vector<std::pair<double, double>> xyVertices{};  // size+1
    std::vector<bool> commonVertices{};
    std::vector<std::pair<double, double>> edges{};
    PolygonCache() = default;
};

struct EdgeCross {

    std::pair<int, int> edge_id{};
    std::pair<double, double> edge_pos{};
    bool used{false};
    EdgeCross(std::pair<int, int> ei, std::pair<double, double> ep)
        : edge_id(std::move(ei)), edge_pos(std::move(ep)){};
};

/**
  @class VolumeIntersection

  A Simple Helper Class that collects methods for calculation of overlap of two
  geometrical objects.

  @author sarka.todorova@cern.ch
  */

class VolumeIntersection {

   public:
    std::pair<bool, std::unique_ptr<Trk::Volume>> intersect(
        const Volume& volA, const Volume& volB) const;

    std::pair<bool, std::unique_ptr<Trk::Volume>> intersectApproximative(
        const Volume& volA, const Volume& volB) const;

   private:
    PolygonCache polygonXY(const Volume& inVol, int swap = 0) const;
    Trk::PolygonCache intersectPgon(Trk::PolygonCache&,
                                    Trk::PolygonCache&) const;

    bool inside(const std::pair<double, double>& vtx,
                const std::vector<std::pair<double, double>>& pgon) const;

    double det(const std::pair<double, double>& a,
               const std::pair<double, double>& b, bool) const;
};

}  // end of namespace Trk

#endif
