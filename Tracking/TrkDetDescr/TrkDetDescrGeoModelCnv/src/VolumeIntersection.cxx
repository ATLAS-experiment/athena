/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// VolumeConverter.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "TrkDetDescrGeoModelCnv/VolumeIntersection.h"

// Trk
#include "TrkVolumes/BevelledCylinderVolumeBounds.h"
#include "TrkVolumes/BoundarySurface.h"
#include "TrkVolumes/CombinedVolumeBounds.h"
#include "TrkVolumes/CuboidVolumeBounds.h"
#include "TrkVolumes/CylinderVolumeBounds.h"
#include "TrkVolumes/DoubleTrapezoidVolumeBounds.h"
#include "TrkVolumes/PrismVolumeBounds.h"
#include "TrkVolumes/SimplePolygonBrepVolumeBounds.h"
#include "TrkVolumes/SubtractedVolumeBounds.h"
#include "TrkVolumes/TrapezoidVolumeBounds.h"

// GeoModel
#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelKernel/GeoShapeUnion.h"
#include "GeoModelKernel/GeoTrd.h"

// STL
#include <iostream>

// #define DEBUG
#ifdef DEBUG
#define DEBUG_TRACE(a) \
    do {               \
        a              \
    } while (0)
#else
#define DEBUG_TRACE(a) \
    do {               \
    } while (0)
#endif

std::pair<bool, const Trk::Volume*> Trk::VolumeIntersection::intersect(
    const Volume* volA, const Volume* volB) const {

    const Trk::Volume* overlap = nullptr;

    // if combination of shifted polygons, calculable
    Trk::PolygonCache pgA = polygonXY(volA);
    Trk::PolygonCache pgB = polygonXY(volB);

    if (pgA.nVtx > 0 && pgB.nVtx > 0) {
        // check orientation of xy face
        Amg::Vector3D a0 = pgA.vertices[1] - pgA.vertices[0];
        Amg::Vector3D a1 = pgA.vertices[2] - pgA.vertices[1];
        Amg::Vector3D b0 = pgB.vertices[1] - pgB.vertices[0];
        Amg::Vector3D b1 = pgB.vertices[2] - pgB.vertices[1];

        if (std::abs((a0.cross(a1).unit()).dot(b0.cross(b1).unit())) < 1.e-3) {
            if (std::abs((a0.cross(a1).unit()).dot(b1)) < 1.e-3)
                pgB = polygonXY(volB, 1);  // xy -> yz, only if cuboid-like
            else if (std::abs((a0.cross(a1).unit()).dot(b0)) < 1.e-3)
                pgB = polygonXY(volB, 2);  // xy -> zx, only if cuboid-like
            // new check orientation of xy face
            b0 = pgB.vertices[1] - pgB.vertices[0];
            b1 = pgB.vertices[2] - pgB.vertices[1];
        }
        if (std::abs(std::abs((a0.cross(a1).unit()).dot(b0.cross(b1).unit())) -
                     1.) > 1.e-3) {
            return std::pair<bool, const Trk::Volume*>(false, overlap);
        }

        Amg::Transform3D trf{Amg::Transform3D::Identity()};
        Amg::Vector3D norm = a0.cross(a1).unit();
        if (norm.z() != 1.) {  // rotate to align with z axis
            trf = Amg::AngleAxis3D(-norm.theta(), Amg::Vector3D::UnitY()) *
                  Amg::AngleAxis3D(-norm.phi(), Amg::Vector3D::UnitZ());

            for (size_t ia = 0; ia < pgA.vertices.size(); ia++)
                pgA.vertices[ia] = trf * pgA.vertices[ia];
            for (size_t ib = 0; ib < pgB.vertices.size(); ib++)
                pgB.vertices[ib] = trf * pgB.vertices[ib];
            pgA.center = trf * pgA.center;
            pgB.center = trf * pgB.center;
        }
        // overlap in z
        pgA.minZ = std::min(pgA.vertices[0].z(),
                            2 * pgA.center.z() - pgA.vertices[0].z());
        pgA.maxZ = std::max(pgA.vertices[0].z(),
                            2 * pgA.center.z() - pgA.vertices[0].z());
        pgB.minZ = std::min(pgB.vertices[0].z(),
                            2 * pgB.center.z() - pgB.vertices[0].z());
        pgB.maxZ = std::max(pgB.vertices[0].z(),
                            2 * pgB.center.z() - pgB.vertices[0].z());

        if (pgA.minZ > pgB.maxZ || pgB.minZ > pgA.maxZ) {
            return std::pair<bool, const Trk::Volume*>(true, overlap);
        }  // no overlap in z

        Trk::PolygonCache result = intersectPgon(pgA, pgB);
        if (result.nVtx > 0) {
            Trk::SimplePolygonBrepVolumeBounds* spb =
                new Trk::SimplePolygonBrepVolumeBounds(
                    result.xyVertices, 0.5 * (result.maxZ - result.minZ));
            Amg::Transform3D* transf = new Amg::Transform3D(
                trf.inverse() *
                Amg::Translation3D(0., 0., 0.5 * (result.maxZ + result.minZ)));
            overlap = new Trk::Volume(transf, spb);
        }
        return (std::pair<bool, const Trk::Volume*>(true, overlap));
    }  // end shifted polygons

    return (std::pair<bool, const Trk::Volume*>(false, overlap));
}

Trk::PolygonCache Trk::VolumeIntersection::intersectPgon(
    Trk::PolygonCache& pgA, Trk::PolygonCache& pgB) const {

    // retrieve xy vertices (size+1)
    for (auto vtx : pgA.vertices)
        pgA.xyVertices.push_back(std::make_pair(vtx.x(), vtx.y()));
    pgA.xyVertices.push_back(
        std::make_pair(pgA.vertices.front().x(), pgA.vertices.front().y()));
    for (auto vtx : pgB.vertices)
        pgB.xyVertices.push_back(std::make_pair(vtx.x(), vtx.y()));
    pgB.xyVertices.push_back(
        std::make_pair(pgB.vertices.front().x(), pgB.vertices.front().y()));
    // find common
    for (auto vtx : pgA.xyVertices)
        pgA.commonVertices.push_back(inside(vtx, pgB.xyVertices));
    for (auto vtx : pgB.xyVertices)
        pgB.commonVertices.push_back(inside(vtx, pgA.xyVertices));
    // edges
    for (int ia = 0; ia < pgA.nVtx; ia++)
        pgA.edges.push_back(std::make_pair(
            pgA.xyVertices[ia + 1].first - pgA.xyVertices[ia].first,
            pgA.xyVertices[ia + 1].second - pgA.xyVertices[ia].second));
    for (int ib = 0; ib < pgB.nVtx; ib++)
        pgB.edges.push_back(std::make_pair(
            pgB.xyVertices[ib + 1].first - pgB.xyVertices[ib].first,
            pgB.xyVertices[ib + 1].second - pgB.xyVertices[ib].second));
    // edge intersections
    std::vector<Trk::EdgeCross> edge_cross;
    for (int ia = 0; ia < pgA.nVtx; ia++) {
        for (int ib = 0; ib < pgB.nVtx; ib++) {
            double rs = det(pgA.edges[ia], pgB.edges[ib], false);
            double qps = det(pgB.xyVertices[ib], pgB.edges[ib], false) -
                         det(pgA.xyVertices[ia], pgB.edges[ib], false);
            double rpq = det(pgA.edges[ia], pgA.xyVertices[ia], false) -
                         det(pgA.edges[ia], pgB.xyVertices[ib], false);
            if (rs == 0 && rpq == 0) {
                double t0 =
                    det(pgA.edges[ia], pgA.edges[ia], true) > 0
                        ? (det(pgB.xyVertices[ib], pgA.edges[ia], true) -
                           det(pgA.xyVertices[ia], pgA.edges[ia], true)) /
                              det(pgA.edges[ia], pgA.edges[ia], true)
                        : 0;
                double t1 =
                    det(pgA.edges[ia], pgA.edges[ia], true) > 0
                        ? t0 + det(pgB.edges[ib], pgA.edges[ia], true) /
                                   det(pgA.edges[ia], pgA.edges[ia], true)
                        : 0;
                if (t0 > 0 && t0 < 1.)
                    edge_cross.push_back(Trk::EdgeCross(
                        std::make_pair(ia, ib), std::make_pair(t0, -1)));
                if (t1 > 0 && t1 < 1.)
                    edge_cross.push_back(Trk::EdgeCross(
                        std::make_pair(ia, ib), std::make_pair(t1, -1)));
            } else if (rs != 0 && qps / rs > 0 && qps / rs < 1 &&
                       rpq / rs > 0 && rpq / rs < 1) {
                edge_cross.push_back(
                    Trk::EdgeCross(std::make_pair(ia, ib),
                                   std::make_pair(qps / rs, rpq / rs)));
            }
        }
    }
    // collect new vertices : first in edge-crossing format
    std::vector<Trk::EdgeCross> setVtx;
    for (int ia = 0; ia < pgA.nVtx; ia++) {
        if (pgA.commonVertices[ia])
            setVtx.push_back(Trk::EdgeCross(std::make_pair(ia, -1),
                                            std::make_pair(0., -1.)));
        for (auto ie : edge_cross) {
            if (ie.edge_id.first == ia) {
                if (setVtx.size() > 0 && setVtx.back().edge_id.first == ia &&
                    setVtx.back().edge_pos.first > ie.edge_pos.first)
                    setVtx.insert(setVtx.end() - 1, ie);
                else
                    setVtx.push_back(ie);
            }
        }  // loop over edge crossings
    }
    // insert common vertices from polygonB
    for (int ib = 0; ib < pgB.nVtx; ib++) {
        if (pgB.commonVertices[ib]) {
            int nlow = ib == 0 ? pgB.nVtx : ib - 1;
            // find entry with nearest intersection along edge
            std::vector<Trk::EdgeCross>::iterator it = setVtx.begin();
            std::vector<Trk::EdgeCross>::iterator itb = setVtx.end();
            while (it != setVtx.end()) {
                if ((*it).edge_id.second == nlow) {
                    if (itb == setVtx.end() ||
                        (*it).edge_pos.second > (*itb).edge_pos.second)
                        itb = it;
                }
                ++it;
            }
            setVtx.insert(
                itb, Trk::EdgeCross(
                         std::make_pair(ib, -2),
                         std::make_pair(
                             0., -1.)));  // -2 indicates vertex coming from B
        }
    }

    // TODO verify the ordering

    // calculate position of vertices and fill the cache
    Trk::PolygonCache pgon;
    pgon.minZ = std::max(pgA.minZ, pgB.minZ);
    pgon.maxZ = std::min(pgA.maxZ, pgB.maxZ);
    pgon.nVtx = setVtx.size() < 3 ? 0 : setVtx.size();

    if (pgon.nVtx < 3)
        return pgon;

    for (auto vtx : setVtx) {
        if (vtx.edge_id.second == -1)
            pgon.xyVertices.push_back(pgA.xyVertices[vtx.edge_id.first]);
        else if (vtx.edge_id.second == -2)
            pgon.xyVertices.push_back(pgB.xyVertices[vtx.edge_id.first]);
        else {  // calculate intersection
            Amg::Vector2D vpos(pgA.xyVertices[vtx.edge_id.first].first,
                               pgA.xyVertices[vtx.edge_id.first].second);
            Amg::Vector2D vdir(pgA.edges[vtx.edge_id.first].first,
                               pgA.edges[vtx.edge_id.first].second);
            Amg::Vector2D vint = vpos + vtx.edge_pos.first * vdir;
            pgon.xyVertices.push_back(std::make_pair(vint.x(), vint.y()));
        }
    }

    // std::cout <<"new polygon:" << pgon.xyVertices.size() <<":" << pgon.nVtx
    // << std::endl; for (auto xy : pgon.xyVertices ) std::cout <<"pgon vertices
    // " << xy.first <<"," << xy.second <<":" <<  std::endl;

    return pgon;
}

Trk::PolygonCache Trk::VolumeIntersection::polygonXY(const Trk::Volume* vol,
                                                     int swap) const {

    const Trk::CuboidVolumeBounds* box =
        dynamic_cast<const Trk::CuboidVolumeBounds*>(&(vol->volumeBounds()));
    const Trk::TrapezoidVolumeBounds* trd =
        dynamic_cast<const Trk::TrapezoidVolumeBounds*>(&(vol->volumeBounds()));
    const Trk::DoubleTrapezoidVolumeBounds* trdd =
        dynamic_cast<const Trk::DoubleTrapezoidVolumeBounds*>(
            &(vol->volumeBounds()));
    const Trk::PrismVolumeBounds* prism =
        dynamic_cast<const Trk::PrismVolumeBounds*>(&(vol->volumeBounds()));
    const Trk::SimplePolygonBrepVolumeBounds* spb =
        dynamic_cast<const Trk::SimplePolygonBrepVolumeBounds*>(
            &(vol->volumeBounds()));

    bool isPolygon = (box || trd || prism || spb || trdd);

    if (!isPolygon)
        return Trk::PolygonCache();

    Trk::PolygonCache cache;

    double hz = 0.;
    std::vector<Amg::Vector3D> vtxLocal;

    if (swap > 0 &&
        (box || (trd && trd->minHalflengthX() ==
                            trd->maxHalflengthX()))) {  // swapping faces

        if (swap == 1) {
            if (box) {
                hz = box->halflengthX();
                cache.nVtx = 4;
                vtxLocal.push_back(Amg::Vector3D(box->halflengthX(),
                                                 box->halflengthY(),
                                                 box->halflengthZ()));
                vtxLocal.push_back(Amg::Vector3D(box->halflengthX(),
                                                 -box->halflengthY(),
                                                 box->halflengthZ()));
                vtxLocal.push_back(Amg::Vector3D(box->halflengthX(),
                                                 -box->halflengthY(),
                                                 -box->halflengthZ()));
                vtxLocal.push_back(Amg::Vector3D(box->halflengthX(),
                                                 box->halflengthY(),
                                                 -box->halflengthZ()));
            } else if (trd) {
                hz = trd->minHalflengthX();
                cache.nVtx = 4;
                vtxLocal.push_back(Amg::Vector3D(trd->minHalflengthX(),
                                                 trd->halflengthY(),
                                                 trd->halflengthZ()));
                vtxLocal.push_back(Amg::Vector3D(trd->minHalflengthX(),
                                                 -trd->halflengthY(),
                                                 trd->halflengthZ()));
                vtxLocal.push_back(Amg::Vector3D(trd->maxHalflengthX(),
                                                 -trd->halflengthY(),
                                                 -trd->halflengthZ()));
                vtxLocal.push_back(Amg::Vector3D(trd->maxHalflengthX(),
                                                 trd->halflengthY(),
                                                 -trd->halflengthZ()));
            }
        } else if (swap == 2) {
            if (box) {
                hz = box->halflengthY();
                cache.nVtx = 4;
                vtxLocal.push_back(Amg::Vector3D(box->halflengthX(),
                                                 box->halflengthY(),
                                                 box->halflengthZ()));
                vtxLocal.push_back(Amg::Vector3D(-box->halflengthX(),
                                                 box->halflengthY(),
                                                 box->halflengthZ()));
                vtxLocal.push_back(Amg::Vector3D(-box->halflengthX(),
                                                 box->halflengthY(),
                                                 -box->halflengthZ()));
                vtxLocal.push_back(Amg::Vector3D(box->halflengthX(),
                                                 box->halflengthY(),
                                                 -box->halflengthZ()));
            } else if (trd) {
                hz = trd->halflengthY();
                cache.nVtx = 4;
                vtxLocal.push_back(Amg::Vector3D(trd->minHalflengthX(),
                                                 trd->halflengthY(),
                                                 trd->halflengthZ()));
                vtxLocal.push_back(Amg::Vector3D(-trd->minHalflengthX(),
                                                 trd->halflengthY(),
                                                 trd->halflengthZ()));
                vtxLocal.push_back(Amg::Vector3D(-trd->minHalflengthX(),
                                                 trd->halflengthY(),
                                                 -trd->halflengthZ()));
                vtxLocal.push_back(Amg::Vector3D(trd->minHalflengthX(),
                                                 trd->halflengthY(),
                                                 -trd->halflengthZ()));
            }
        }
        cache.hZ = hz;
        cache.center = vol->transform().translation();

        for (auto vtxloc : vtxLocal) {
            Amg::Vector3D vtx = vol->transform() * vtxloc;
            cache.vertices.push_back(vtx);
        }
        return cache;
    }  // end swap

    if (box) {
        hz = box->halflengthZ();
        cache.nVtx = 4;
        vtxLocal.push_back(Amg::Vector3D(box->halflengthX(), box->halflengthY(),
                                         box->halflengthZ()));
        vtxLocal.push_back(Amg::Vector3D(
            -box->halflengthX(), box->halflengthY(), box->halflengthZ()));
        vtxLocal.push_back(Amg::Vector3D(
            -box->halflengthX(), -box->halflengthY(), box->halflengthZ()));
        vtxLocal.push_back(Amg::Vector3D(
            box->halflengthX(), -box->halflengthY(), box->halflengthZ()));
    } else if (trd) {
        hz = trd->halflengthZ();
        cache.nVtx = 4;
        vtxLocal.push_back(Amg::Vector3D(
            trd->minHalflengthX(), -trd->halflengthY(), trd->halflengthZ()));
        vtxLocal.push_back(Amg::Vector3D(
            -trd->minHalflengthX(), -trd->halflengthY(), trd->halflengthZ()));
        vtxLocal.push_back(Amg::Vector3D(
            -trd->maxHalflengthX(), trd->halflengthY(), trd->halflengthZ()));
        vtxLocal.push_back(Amg::Vector3D(
            trd->maxHalflengthX(), trd->halflengthY(), trd->halflengthZ()));
    } else if (trdd) {
        hz = trdd->halflengthZ();
        cache.nVtx = 6;
        vtxLocal.push_back(Amg::Vector3D(trdd->maxHalflengthX(),
                                         2 * trdd->halflengthY2(),
                                         trdd->halflengthZ()));
        vtxLocal.push_back(Amg::Vector3D(-trdd->maxHalflengthX(),
                                         2 * trdd->halflengthY2(),
                                         trdd->halflengthZ()));
        vtxLocal.push_back(
            Amg::Vector3D(-trdd->medHalflengthX(), 0., trdd->halflengthZ()));
        vtxLocal.push_back(Amg::Vector3D(-trdd->minHalflengthX(),
                                         -2 * trdd->halflengthY1(),
                                         trdd->halflengthZ()));
        vtxLocal.push_back(Amg::Vector3D(trdd->minHalflengthX(),
                                         -2 * trdd->halflengthY1(),
                                         trdd->halflengthZ()));
        vtxLocal.push_back(
            Amg::Vector3D(trdd->medHalflengthX(), 0., trdd->halflengthZ()));
    } else if (prism) {
        hz = prism->halflengthZ();
        const std::vector<std::pair<double, double> > vtcs =
            prism->xyVertices();
        for (const auto& vtc : vtcs)
            vtxLocal.push_back(
                Amg::Vector3D(vtc.first, vtc.second, prism->halflengthZ()));
        cache.nVtx = vtcs.size();
    } else if (spb) {
        hz = spb->halflengthZ();
        const std::vector<std::pair<double, double> > vtcs = spb->xyVertices();
        for (const auto& vtc : vtcs)
            vtxLocal.push_back(
                Amg::Vector3D(vtc.first, vtc.second, spb->halflengthZ()));
        cache.nVtx = vtcs.size();
    }

    cache.hZ = hz;
    cache.center = vol->transform().translation();

    for (auto vtxloc : vtxLocal) {
        Amg::Vector3D vtx = vol->transform() * vtxloc;
        cache.vertices.push_back(vtx);
    }

    return cache;
}

bool Trk::VolumeIntersection::inside(
    std::pair<double, double> vtx,
    std::vector<std::pair<double, double> > pgon) const {

    // GM code
    bool in = false;
    size_t nv = pgon.size();
    for (size_t i = 0, k = nv - 1; i < nv; k = i++) {
        if ((pgon[i].second > vtx.second) != (pgon[k].second > vtx.second)) {
            double ctg = (pgon[k].first - pgon[i].first) /
                         (pgon[k].second - pgon[i].second);
            in ^= (vtx.first <
                   (vtx.second - pgon[i].second) * ctg + pgon[i].first);
        }
    }
    return in;
}

double Trk::VolumeIntersection::det(std::pair<double, double> a,
                                    std::pair<double, double> b,
                                    bool dot) const {

    if (dot)
        return (a.first * b.first + a.second * b.second);

    return (a.first * b.second - a.second * b.first);
}

std::pair<bool, const Trk::Volume*>
Trk::VolumeIntersection::intersectApproximative(const Volume* volA,
                                                const Volume* volB) const {

    const Trk::Volume* overlap = nullptr;

    // if combination of shifted polygons, calculable
    Trk::PolygonCache pgA = polygonXY(volA);
    Trk::PolygonCache pgB = polygonXY(volB);

    const Trk::CylinderVolumeBounds* cylA = 0;
    if (pgA.nVtx == 0)
        cylA = dynamic_cast<const Trk::CylinderVolumeBounds*>(
            &(volA->volumeBounds()));
    const Trk::CylinderVolumeBounds* cylB = 0;
    if (pgB.nVtx == 0)
        cylB = dynamic_cast<const Trk::CylinderVolumeBounds*>(
            &(volB->volumeBounds()));

    if (cylA && cylB) {
        double distance_center = (volA->center() - volB->center()).norm();
        if (distance_center >
            (sqrt(pow(cylA->outerRadius(), 2) + pow(cylA->halflengthZ(), 2)) +
             sqrt(pow(cylB->outerRadius(), 2) + pow(cylB->halflengthZ(), 2))))
            return std::pair<bool, const Trk::Volume*>(true, overlap);
    }

    if (pgA.nVtx > 0 && pgB.nVtx > 0) {
        // check orientation of xy face
        Amg::Vector3D a0 = pgA.vertices[1] - pgA.vertices[0];
        Amg::Vector3D a1 = pgA.vertices[2] - pgA.vertices[1];
        Amg::Vector3D b0 = pgB.vertices[1] - pgB.vertices[0];
        Amg::Vector3D b1 = pgB.vertices[2] - pgB.vertices[1];

        if (std::abs((b0.cross(b1).unit()).dot(a0.cross(a1).unit())) < 1.e-3) {
            if (std::abs((b0.cross(b1).unit()).dot(a1)) < 1.e-3)
                pgA = polygonXY(volA, 1);  // xy -> yz, only if cuboid-like
            else if (std::abs((b0.cross(b1).unit()).dot(a0)) < 1.e-3)
                pgA = polygonXY(volA, 2);  // xy -> zx, only if cuboid-like
            // new check orientation of xy face
            a0 = pgA.vertices[1] - pgA.vertices[0];
            a1 = pgA.vertices[2] - pgA.vertices[1];
        }
        if (std::abs(std::abs((b0.cross(b1).unit()).dot(a0.cross(a1).unit())) -
                     1.) > 1.e-3) {
            return std::pair<bool, const Trk::Volume*>(false, overlap);
        }

        Amg::Transform3D trf{Amg::Transform3D::Identity()};
        Amg::Vector3D norm = a0.cross(a1).unit();
        if (norm.z() != 1.) {  // rotate to align with z axis
            trf = Amg::AngleAxis3D(-norm.theta(), Amg::Vector3D::UnitY()) *
                  Amg::AngleAxis3D(-norm.phi(), Amg::Vector3D::UnitZ());

            for (size_t ia = 0; ia < pgA.vertices.size(); ia++)
                pgA.vertices[ia] = trf * pgA.vertices[ia];
            for (size_t ib = 0; ib < pgB.vertices.size(); ib++)
                pgB.vertices[ib] = trf * pgB.vertices[ib];
            pgA.center = trf * pgA.center;
            pgB.center = trf * pgB.center;
        }
        // overlap in z
        pgA.minZ = std::min(pgA.vertices[0].z(),
                            2 * pgA.center.z() - pgA.vertices[0].z());
        pgA.maxZ = std::max(pgA.vertices[0].z(),
                            2 * pgA.center.z() - pgA.vertices[0].z());
        pgB.minZ = std::min(pgB.vertices[0].z(),
                            2 * pgB.center.z() - pgB.vertices[0].z());
        pgB.maxZ = std::max(pgB.vertices[0].z(),
                            2 * pgB.center.z() - pgB.vertices[0].z());

        if (pgA.minZ > pgB.maxZ || pgB.minZ > pgA.maxZ) {
            return std::pair<bool, const Trk::Volume*>(true, overlap);
        }  // no overlap in z

        Trk::PolygonCache result = intersectPgon(pgA, pgB);
        if (result.nVtx > 0) {
            Trk::SimplePolygonBrepVolumeBounds* spb =
                new Trk::SimplePolygonBrepVolumeBounds(
                    result.xyVertices, 0.5 * (result.maxZ - result.minZ));
            Amg::Transform3D* transf = new Amg::Transform3D(
                trf.inverse() *
                Amg::Translation3D(0., 0., 0.5 * (result.maxZ + result.minZ)));
            overlap = new Trk::Volume(transf, spb);
        }
        return (std::pair<bool, const Trk::Volume*>(true, overlap));
    }  // end shifted polygons

    return (std::pair<bool, const Trk::Volume*>(false, overlap));
}
