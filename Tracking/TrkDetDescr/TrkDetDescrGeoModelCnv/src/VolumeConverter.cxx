/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// VolumeConverter.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "TrkDetDescrGeoModelCnv/VolumeConverter.h"

// Trk
#include "TrkGeometry/TrackingVolume.h"
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
#include "GeoModelKernel/GeoShapeIntersection.h"
#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelKernel/GeoShapeSubtraction.h"
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

Trk::TrackingVolume* Trk::VolumeConverter::translate(
    const GeoVPhysVol* gv, bool simplify, bool blend,
    double blendMassLimit) const {

    const std::string name = gv->getLogVol()->getName();

    Amg::Transform3D ident{Amg::Transform3D::Identity()};
    const Trk::Volume* volGeo = m_geoShapeConverter.translateGeoShape(
        gv->getLogVol()->getShape(), &ident);

    // temporary owner of auxiliary volumes
    std::vector<const Trk::Volume*> vv;

    // resolve volume structure into a set of non-overlapping subtractions from
    // analytically calculable shapes
    std::vector<std::pair<const Trk::Volume*, const Trk::Volume*>>
        constituents = splitComposedVolume(volGeo, vv);

    // material properties
    Trk::Material mat =
        m_materialConverter.convert(gv->getLogVol()->getMaterial());

    // calculate precision of volume estimate  taking into account material
    // properties
    double precision = Trk::VolumeConverter::s_precisionInX0 *
                       mat.X0;  // required precision in mm

    // volume estimate from GeoShape
    double volumeFromGeoShape =
        -1;  // replace with database info when available

    // volume estimate from resolveBoolean
    // double volumeBoolean = calculateVolume(volGeo,false,pow(precision,3)); //
    // TODO : test on inert material

    // volume estimate from Trk::Volume
    double volume = volumeFromGeoShape >= 0 ? volumeFromGeoShape : -1.;
    if ((simplify || blend) && volumeFromGeoShape < 0) {
        double fraction = 0.;
        volume = 0.;
        for (auto cs : constituents) {
            fraction = estimateFraction(cs, precision);
            if (fraction < 0) {
                volume = -1;
                break;
            } else
                volume += fraction * calculateVolume(cs.first);
        }
    }

    // evaluate complexity of shape
    // simple case
    if (constituents.size() == 1 && !constituents[0].second)
        return new Trk::TrackingVolume(*volGeo, mat, nullptr, nullptr, name);

    // build envelope
    const Trk::Volume* envelope = nullptr;
    std::string envName = name;

    Trk::TrackingVolume* trEnv = nullptr;

    bool blended = false;

    if (constituents.size() == 1) {

        envelope =
            new Trk::Volume(*(constituents.front().first), volGeo->transform());
        double volEnv = calculateVolume(constituents.front().first);

        if (blend && volume > 0 && volEnv > 0 &&
            volume * mat.rho < blendMassLimit)
            blended = true;

        if ((simplify || blended) && volume > 0 && volEnv > 0) {
            // simplified material rescales X0, l0 and density
            double fraction = volume / volEnv;
            Trk::Material matScaled(mat.X0 / fraction, mat.L0 / fraction, mat.A,
                                    mat.Z, fraction * mat.rho);
            if (blend && !blended)
                envName = envName + "_PERM";
            trEnv = new Trk::TrackingVolume(*envelope, matScaled, nullptr,
                                            nullptr, envName);
        } else {
            auto confinedVols =
                std::make_unique<std::vector<Trk::TrackingVolume*>>();
            confinedVols->push_back(
                new Trk::TrackingVolume(*volGeo, mat, nullptr, nullptr, name));
            envName = name + "_envelope";
            Trk::Material dummyMaterial(1.e10, 1.e10, 0., 0.,
                                        0.);  // default material properties
            trEnv = new Trk::TrackingVolume(*envelope, dummyMaterial,
                                            confinedVols.release(), envName);
        }

        return trEnv;
    }

    // composed shapes : derive envelope from span
    Amg::Transform3D transf = volGeo->transform();
    std::unique_ptr<Trk::VolumeSpan> span = std::make_unique<Trk::VolumeSpan>(
        *findVolumeSpan(&(volGeo->volumeBounds()), transf, 0., 0.));

    bool isCyl = false;
    for (auto fv : constituents) {
        const Trk::CylinderVolumeBounds* cyl =
            dynamic_cast<const Trk::CylinderVolumeBounds*>(
                &(fv.first->volumeBounds()));
        if (cyl) {
            isCyl = true;
            break;
        }
    }

    DEBUG_TRACE({
        std::cout << "envelope estimate: object contains cylinder:" << name
                  << ":" << isCyl << std::endl;
        std::cout << "complex volume span for envelope:" << name
                  << ":x range:" << (*span).xMin << "," << (*span).xMax
                  << std::endl;
        std::cout << "complex volume span for envelope:" << name
                  << ":y range:" << (*span).yMin << "," << (*span).yMax
                  << std::endl;
        std::cout << "complex volume span for envelope:" << name
                  << ":z range:" << (*span).zMin << "," << (*span).zMax
                  << std::endl;
        std::cout << "complex volume span for envelope:" << name
                  << ":R range:" << (*span).rMin << "," << (*span).rMax
                  << std::endl;
        std::cout << "complex volume span for envelope:" << name
                  << ":phi range:" << (*span).phiMin << "," << (*span).phiMax
                  << std::endl;
    });

    if (!isCyl) {  // cuboid envelope
        envelope = new Trk::Volume(
            new Amg::Transform3D(transf *
                                 Amg::Translation3D(Amg::Vector3D(
                                     0.5 * ((*span).xMin + (*span).xMax),
                                     0.5 * ((*span).yMin + (*span).yMax),
                                     0.5 * ((*span).zMin + (*span).zMax)))),
            new Trk::CuboidVolumeBounds(0.5 * ((*span).xMax - (*span).xMin),
                                        0.5 * ((*span).yMax - (*span).yMin),
                                        0.5 * ((*span).zMax - (*span).zMin)));
    } else {

        double dPhi = (*span).phiMin > (*span).phiMax
                          ? (*span).phiMax - (*span).phiMin + 2 * M_PI
                          : (*span).phiMax - (*span).phiMin;
        Trk::CylinderVolumeBounds* cylBounds = nullptr;
        if (dPhi < 2 * M_PI) {
            double aPhi = 0.5 * ((*span).phiMax + (*span).phiMin);
            cylBounds = new Trk::CylinderVolumeBounds(
                (*span).rMin, (*span).rMax, 0.5 * dPhi,
                0.5 * ((*span).zMax - (*span).zMin));
            const Amg::AngleAxis3D zRotation(aPhi, Amg::Vector3D::UnitZ());
            transf = transf * zRotation;
        } else {
            cylBounds = new Trk::CylinderVolumeBounds(
                (*span).rMin, (*span).rMax,
                0.5 * ((*span).zMax - (*span).zMin));
        }
        envelope = new Trk::Volume(&transf, cylBounds);
    }

    double volEnv = calculateVolume(envelope);

    if (blend && volume > 0 && volEnv > 0 && volume * mat.rho < blendMassLimit)
        blended = true;

    if ((simplify || blended) && volume > 0 && volEnv > 0) {
        double fraction = volume / volEnv;
        Trk::Material matScaled(mat.X0 / fraction, mat.L0 / fraction, mat.A,
                                mat.Z, fraction * mat.rho);
        if (blend && !blended)
            envName = envName + "_PERM";
        trEnv =
            new Trk::TrackingVolume(*envelope, mat, nullptr, nullptr, envName);
    } else {
        auto confinedVols =
            std::make_unique<std::vector<Trk::TrackingVolume*>>();
        confinedVols->push_back(
            new Trk::TrackingVolume(*volGeo, mat, nullptr, nullptr, name));
        envName = envName + "_envelope";
        Trk::Material dummyMaterial(1.e10, 1.e10, 0., 0.,
                                    0.);  // default material properties
        trEnv = new Trk::TrackingVolume(*envelope, dummyMaterial,
                                        confinedVols.release(), envName);
    }

    return trEnv;
}

double Trk::VolumeConverter::resolveBooleanVolume(const Trk::Volume* trVol,
                                                  double tolerance) const {

    std::vector<Trk::VolumePart> constituents;
    Trk::VolumePart inputVol;
    inputVol.parts.push_back(trVol);
    inputVol.sign = 1.;
    // constituents.push_back(Trk::VolumePart( trVol, 1.));
    constituents.push_back(inputVol);
    std::vector<Trk::VolumePart>::iterator sIter = constituents.begin();
    // temporary owner of auxiliary volumes
    std::vector<const Trk::Volume*> vv;
    double volume = 0;
    while (sIter != constituents.end()) {
        bool update = false;
        for (unsigned int ii = 0; ii < (*sIter).parts.size(); ii++) {
            // std::vector<Trk::Volume* >::iterator vIter =
            // (*sIter).parts.begin(); while (vIter != (*sIter).parts.end()) {
            const Trk::CombinedVolumeBounds* comb =
                dynamic_cast<const Trk::CombinedVolumeBounds*>(
                    &((*sIter).parts[ii]->volumeBounds()));
            const Trk::SubtractedVolumeBounds* sub =
                dynamic_cast<const Trk::SubtractedVolumeBounds*>(
                    &((*sIter).parts[ii]->volumeBounds()));
            if (comb) {
                (*sIter).parts[ii] = comb->first();
                Trk::VolumePart vp = (*sIter);
                constituents.push_back(vp);
                constituents.back().parts[ii] = comb->second();
                constituents.push_back(vp);
                constituents.back().parts.push_back(comb->second());
                constituents.back().sign = -1. * constituents.back().sign;
                update = true;
                break;
            } else if (sub) {
                (*sIter).parts[ii] = sub->outer();
                // small components
                double volSub = calculateVolume(sub->inner(), true, tolerance);
                if (volSub < tolerance)
                    volume += -1. * (*sIter).sign * volSub;
                else {
                    constituents.push_back(Trk::VolumePart(*sIter));
                    constituents.back().parts.push_back(sub->inner());
                    constituents.back().sign = -1. * constituents.back().sign;
                }
                update = true;
                break;
            } else {
                // component small, below tolerance
                double volSmall = calculateVolume((*sIter).parts[ii]);
                if (volSmall < tolerance) {
                    constituents.erase(sIter);
                    update = true;
                    break;
                }
            }
        }  //
        if (update)
            sIter = constituents.begin();
        else if ((*sIter).parts.size() == 1) {
            double volSingle = calculateVolume((*sIter).parts[0]);
            volume += (*sIter).sign * volSingle;
            constituents.erase(sIter);
        } else {
            std::vector<const Trk::Volume*>::iterator tit =
                (*sIter).parts.begin();
            bool noovrlp = false;
            while (tit + 1 != (*sIter).parts.end()) {
                std::pair<bool, const Trk::Volume*> overlap =
                    m_intersectionHelper.intersect(*tit, *(tit + 1));
                if (overlap.first && !overlap.second) {
                    constituents.erase(sIter);
                    noovrlp = true;
                    break;
                }  // no intersection
                else if (overlap.first && overlap.second) {
                    (*sIter).parts.erase(tit, tit + 2);
                    (*sIter).parts.push_back(overlap.second);
                    vv.push_back(
                        (std::make_unique<Trk::Volume>(*overlap.second)).get());
                    tit = (*sIter).parts.begin();
                } else {
                    if (calculateVolume(*tit) < tolerance) {
                        constituents.erase(sIter);
                        noovrlp = true;
                        break;
                    }
                    if (calculateVolume(*(tit + 1)) < tolerance) {
                        constituents.erase(sIter);
                        noovrlp = true;
                        break;
                    }
                    std::pair<bool, const Trk::Volume*> overlap =
                        m_intersectionHelper.intersectApproximative(*tit,
                                                                    *(tit + 1));
                    if (overlap.first) {
                        if (overlap.second) {
                            (*sIter).parts.erase(tit, tit + 2);
                            (*sIter).parts.push_back(overlap.second);
                            vv.push_back(
                                (std::make_unique<Trk::Volume>(*overlap.second))
                                    .get());
                            tit = (*sIter).parts.begin();
                        } else {
                            constituents.erase(sIter);
                            noovrlp = true;
                            break;  // no intersection
                        }
                    } else
                        ++tit;
                }
            }
            if (noovrlp) {
            } else if ((*sIter).parts.size() == 1) {
                double volSingle = calculateVolume((*sIter).parts[0]);
                volume += (*sIter).sign * volSingle;
                constituents.erase(sIter);
            } else {
                ++sIter;
            }
        }
        // std::cout << "constituents.size:" << constituents.size() << ":volume
        // estimate so far:" << volume <<  std::endl;
    }

    if (constituents.size())
        std::cout << "boolean volume resolved to " << constituents.size()
                  << " items "
                  << ":volume estimate:" << volume << std::endl;
    return volume;
}

std::vector<std::pair<const Trk::Volume*, const Trk::Volume*>>
Trk::VolumeConverter::splitComposedVolume(
    const Trk::Volume* trVol, std::vector<const Trk::Volume*>& vv) const {

    std::vector<std::pair<const Trk::Volume*, const Trk::Volume*>> constituents;
    constituents.emplace_back(trVol, nullptr);
    std::vector<std::pair<const Trk::Volume*, const Trk::Volume*>>::iterator
        sIter = constituents.begin();
    const Trk::Volume* subVol = nullptr;
    while (sIter != constituents.end()) {
        const Trk::CombinedVolumeBounds* comb =
            dynamic_cast<const Trk::CombinedVolumeBounds*>(
                &((*sIter).first->volumeBounds()));
        const Trk::SubtractedVolumeBounds* sub =
            dynamic_cast<const Trk::SubtractedVolumeBounds*>(
                &((*sIter).first->volumeBounds()));
        if (comb) {
            subVol = (*sIter).second;
            sIter = constituents.erase(sIter);
            if (comb->intersection()) {
                Trk::Volume* newSubVol = new Trk::Volume(
                    nullptr,
                    new Trk::SubtractedVolumeBounds(comb->first()->clone(),
                                                    comb->second()->clone()));
                vv.push_back((std::make_unique<Trk::Volume>(*newSubVol)).get());
                if (subVol) {
                    Trk::Volume* newCSubVol = new Trk::Volume(
                        nullptr, new Trk::CombinedVolumeBounds(
                                     subVol->clone(), newSubVol, false));
                    constituents.insert(
                        sIter,
                        std::pair<const Trk::Volume*, const Trk::Volume*>(
                            comb->first(), newCSubVol));
                    vv.push_back(
                        (std::make_unique<Trk::Volume>(*newCSubVol)).get());
                } else {
                    constituents.insert(
                        sIter,
                        std::pair<const Trk::Volume*, const Trk::Volume*>(
                            comb->first(), newSubVol));
                }
            } else {
                constituents.insert(
                    sIter, std::pair<const Trk::Volume*, const Trk::Volume*>(
                               comb->first(), subVol));
                if (subVol) {
                    Trk::Volume* newSubVol = new Trk::Volume(
                        nullptr,
                        new Trk::CombinedVolumeBounds(
                            subVol->clone(), comb->first()->clone(), false));
                    constituents.insert(
                        sIter,
                        std::pair<const Trk::Volume*, const Trk::Volume*>(
                            comb->second(), newSubVol));
                    vv.push_back(
                        (std::make_unique<Trk::Volume>(*newSubVol)).get());
                } else {
                    constituents.insert(
                        sIter,
                        std::pair<const Trk::Volume*, const Trk::Volume*>(
                            comb->second(), comb->first()));
                }
            }
            //}
            sIter = constituents.begin();
        } else if (sub) {
            subVol = (*sIter).second;
            sIter = constituents.erase(sIter);
            if (subVol) {
                Trk::Volume* newSubVol = new Trk::Volume(
                    nullptr,
                    new Trk::CombinedVolumeBounds(
                        subVol->clone(), sub->inner()->clone(), false));
                constituents.insert(
                    sIter, std::pair<const Trk::Volume*, const Trk::Volume*>(
                               sub->outer(), newSubVol));
                vv.push_back((std::make_unique<Trk::Volume>(*newSubVol)).get());
                //}
            } else {
                constituents.insert(
                    sIter, std::pair<const Trk::Volume*, const Trk::Volume*>(
                               sub->outer(), sub->inner()));
            }
            sIter = constituents.begin();
        } else {
            ++sIter;
        }
    }

    return constituents;
}

const Trk::VolumeSpan* Trk::VolumeConverter::findVolumeSpan(
    const Trk::VolumeBounds* volBounds, const Amg::Transform3D& transform,
    double zTol, double phiTol) const {
    if (!volBounds)
        return nullptr;
    // volume shape
    const Trk::CuboidVolumeBounds* box =
        dynamic_cast<const Trk::CuboidVolumeBounds*>(volBounds);
    const Trk::TrapezoidVolumeBounds* trd =
        dynamic_cast<const Trk::TrapezoidVolumeBounds*>(volBounds);
    const Trk::DoubleTrapezoidVolumeBounds* dtrd =
        dynamic_cast<const Trk::DoubleTrapezoidVolumeBounds*>(volBounds);
    const Trk::BevelledCylinderVolumeBounds* bcyl =
        dynamic_cast<const Trk::BevelledCylinderVolumeBounds*>(volBounds);
    const Trk::CylinderVolumeBounds* cyl =
        dynamic_cast<const Trk::CylinderVolumeBounds*>(volBounds);
    const Trk::SubtractedVolumeBounds* sub =
        dynamic_cast<const Trk::SubtractedVolumeBounds*>(volBounds);
    const Trk::CombinedVolumeBounds* comb =
        dynamic_cast<const Trk::CombinedVolumeBounds*>(volBounds);
    const Trk::SimplePolygonBrepVolumeBounds* spb =
        dynamic_cast<const Trk::SimplePolygonBrepVolumeBounds*>(volBounds);
    const Trk::PrismVolumeBounds* prism =
        dynamic_cast<const Trk::PrismVolumeBounds*>(volBounds);

    double dPhi = 0.;

    if (sub) {
        return findVolumeSpan(&(sub->outer()->volumeBounds()),
                              transform * sub->outer()->transform(), zTol,
                              phiTol);
    }

    if (comb) {
        const Trk::VolumeSpan* s1 = findVolumeSpan(
            &(comb->first()->volumeBounds()),
            transform * comb->first()->transform(), zTol, phiTol);
        const Trk::VolumeSpan* s2 = findVolumeSpan(
            &(comb->second()->volumeBounds()),
            transform * comb->second()->transform(), zTol, phiTol);

        Trk::VolumeSpan scomb;
        scomb.rMin = std::min((*s1).rMin, (*s2).rMin);
        scomb.rMax = std::max((*s1).rMax, (*s2).rMax);
        scomb.xMin = std::min((*s1).xMin, (*s2).xMin);
        scomb.xMax = std::max((*s1).xMax, (*s2).xMax);
        scomb.yMin = std::min((*s1).yMin, (*s2).yMin);
        scomb.yMax = std::max((*s1).yMax, (*s2).yMax);
        scomb.zMin = std::min((*s1).zMin, (*s2).zMin);
        scomb.zMax = std::max((*s1).zMax, (*s2).zMax);
        if ((*s1).phiMin < (*s1).phiMax && (*s2).phiMin < (*s2).phiMax) {
            scomb.phiMin = std::min((*s1).phiMin, (*s2).phiMin);
            scomb.phiMax = std::max((*s1).phiMax, (*s2).phiMax);
        } else if ((*s1).phiMin < (*s1).phiMax && (*s2).phiMin > (*s2).phiMax) {
            if ((*s1).phiMin > (*s2).phiMax) {
                scomb.phiMin = std::min((*s1).phiMin, (*s2).phiMin);
                scomb.phiMax = (*s2).phiMax;
            } else if ((*s1).phiMax < (*s2).phiMin) {
                scomb.phiMin = (*s2).phiMin;
                scomb.phiMax = std::max((*s1).phiMax, (*s2).phiMax);
            } else {
                scomb.phiMin = 0.;
                scomb.phiMax = 2 * M_PI;
            }
        } else if ((*s1).phiMin > (*s1).phiMax && (*s2).phiMin < (*s2).phiMax) {
            if ((*s2).phiMin > (*s1).phiMax) {
                scomb.phiMin = std::min((*s1).phiMin, (*s2).phiMin);
                scomb.phiMax = (*s1).phiMax;
            } else if ((*s2).phiMax < (*s1).phiMin) {
                scomb.phiMin = (*s1).phiMin;
                scomb.phiMax = std::max((*s1).phiMax, (*s2).phiMax);
            } else {
                scomb.phiMin = 0.;
                scomb.phiMax = 2 * M_PI;
            }
        } else {
            scomb.phiMin = std::min((*s1).phiMin, (*s2).phiMin);
            scomb.phiMax = std::max((*s1).phiMax, (*s2).phiMax);
        }
        delete s1;
        delete s2;
        return new Trk::VolumeSpan(scomb);
    }

    //
    double minZ = 1.e6;
    double maxZ = -1.e6;
    double minPhi = 2 * M_PI;
    double maxPhi = 0.;
    double minR = 1.e6;
    double maxR = 0.;
    double minX = 1.e6;
    double maxX = -1.e6;
    double minY = 1.e6;
    double maxY = -1.e6;

    // defined vertices and edges
    std::vector<Amg::Vector3D> vtx;
    std::vector<std::pair<int, int>> edges;
    Trk::VolumeSpan span;

    if (box) {
        vtx.emplace_back(box->halflengthX(), box->halflengthY(),
                         box->halflengthZ());
        vtx.emplace_back(-box->halflengthX(), box->halflengthY(),
                         box->halflengthZ());
        vtx.emplace_back(box->halflengthX(), -box->halflengthY(),
                         box->halflengthZ());
        vtx.emplace_back(-box->halflengthX(), -box->halflengthY(),
                         box->halflengthZ());
        vtx.emplace_back(box->halflengthX(), box->halflengthY(),
                         -box->halflengthZ());
        vtx.emplace_back(-box->halflengthX(), box->halflengthY(),
                         -box->halflengthZ());
        vtx.emplace_back(box->halflengthX(), -box->halflengthY(),
                         -box->halflengthZ());
        vtx.emplace_back(-box->halflengthX(), -box->halflengthY(),
                         -box->halflengthZ());
        edges.emplace_back(std::make_pair(0, 1));
        edges.emplace_back(std::make_pair(0, 2));
        edges.emplace_back(std::make_pair(1, 3));
        edges.emplace_back(std::make_pair(2, 3));
        edges.emplace_back(std::make_pair(4, 5));
        edges.emplace_back(std::make_pair(4, 6));
        edges.emplace_back(std::make_pair(5, 7));
        edges.emplace_back(std::make_pair(6, 7));
        edges.emplace_back(std::make_pair(0, 4));
        edges.emplace_back(std::make_pair(1, 5));
        edges.emplace_back(std::make_pair(2, 6));
        edges.emplace_back(std::make_pair(3, 7));
    }
    if (trd) {
        vtx.emplace_back(trd->maxHalflengthX(), trd->halflengthY(),
                         trd->halflengthZ());
        vtx.emplace_back(-trd->maxHalflengthX(), trd->halflengthY(),
                         trd->halflengthZ());
        vtx.emplace_back(trd->minHalflengthX(), -trd->halflengthY(),
                         trd->halflengthZ());
        vtx.emplace_back(-trd->minHalflengthX(), -trd->halflengthY(),
                         trd->halflengthZ());
        vtx.emplace_back(trd->maxHalflengthX(), trd->halflengthY(),
                         -trd->halflengthZ());
        vtx.emplace_back(-trd->maxHalflengthX(), trd->halflengthY(),
                         -trd->halflengthZ());
        vtx.emplace_back(trd->minHalflengthX(), -trd->halflengthY(),
                         -trd->halflengthZ());
        vtx.emplace_back(-trd->minHalflengthX(), -trd->halflengthY(),
                         -trd->halflengthZ());
        edges.emplace_back(std::make_pair(0, 1));
        edges.emplace_back(std::make_pair(0, 2));
        edges.emplace_back(std::make_pair(1, 3));
        edges.emplace_back(std::make_pair(2, 3));
        edges.emplace_back(std::make_pair(4, 5));
        edges.emplace_back(std::make_pair(4, 6));
        edges.emplace_back(std::make_pair(5, 7));
        edges.emplace_back(std::make_pair(6, 7));
        edges.emplace_back(std::make_pair(0, 4));
        edges.emplace_back(std::make_pair(1, 5));
        edges.emplace_back(std::make_pair(2, 6));
        edges.emplace_back(std::make_pair(3, 7));
    }
    if (dtrd) {
        vtx.emplace_back(dtrd->maxHalflengthX(), 2 * dtrd->halflengthY2(),
                         dtrd->halflengthZ());
        vtx.emplace_back(-dtrd->maxHalflengthX(), 2 * dtrd->halflengthY2(),
                         dtrd->halflengthZ());
        vtx.emplace_back(dtrd->medHalflengthX(), 0., dtrd->halflengthZ());
        vtx.emplace_back(-dtrd->medHalflengthX(), 0., dtrd->halflengthZ());
        vtx.emplace_back(dtrd->minHalflengthX(), -2 * dtrd->halflengthY1(),
                         dtrd->halflengthZ());
        vtx.emplace_back(-dtrd->minHalflengthX(), -2 * dtrd->halflengthY1(),
                         dtrd->halflengthZ());
        vtx.emplace_back(dtrd->maxHalflengthX(), 2 * dtrd->halflengthY2(),
                         -dtrd->halflengthZ());
        vtx.emplace_back(-dtrd->maxHalflengthX(), 2 * dtrd->halflengthY2(),
                         -dtrd->halflengthZ());
        vtx.emplace_back(dtrd->medHalflengthX(), 0., -dtrd->halflengthZ());
        vtx.emplace_back(-dtrd->medHalflengthX(), 0., -dtrd->halflengthZ());
        vtx.emplace_back(dtrd->minHalflengthX(), -2 * dtrd->halflengthY1(),
                         -dtrd->halflengthZ());
        vtx.emplace_back(-dtrd->minHalflengthX(), -2 * dtrd->halflengthY1(),
                         -dtrd->halflengthZ());
        edges.emplace_back(std::make_pair(0, 1));
        edges.emplace_back(std::make_pair(0, 2));
        edges.emplace_back(std::make_pair(1, 3));
        edges.emplace_back(std::make_pair(2, 4));
        edges.emplace_back(std::make_pair(3, 5));
        edges.emplace_back(std::make_pair(4, 5));
        edges.emplace_back(std::make_pair(6, 7));
        edges.emplace_back(std::make_pair(6, 8));
        edges.emplace_back(std::make_pair(7, 9));
        edges.emplace_back(std::make_pair(8, 10));
        edges.emplace_back(std::make_pair(9, 11));
        edges.emplace_back(std::make_pair(10, 11));
        edges.emplace_back(std::make_pair(0, 6));
        edges.emplace_back(std::make_pair(1, 7));
        edges.emplace_back(std::make_pair(2, 8));
        edges.emplace_back(std::make_pair(3, 9));
        edges.emplace_back(std::make_pair(4, 10));
        edges.emplace_back(std::make_pair(5, 11));
    }
    if (bcyl) {
        dPhi = bcyl->halfPhiSector();
        vtx.emplace_back(0., 0., bcyl->halflengthZ());
        vtx.emplace_back(0., 0., -bcyl->halflengthZ());
        edges.emplace_back(std::make_pair(0, 1));
        if (dPhi < M_PI) {
            vtx.emplace_back(bcyl->outerRadius() * cos(dPhi),
                             bcyl->outerRadius() * sin(dPhi),
                             bcyl->halflengthZ());
            vtx.emplace_back(bcyl->innerRadius() * cos(dPhi),
                             bcyl->innerRadius() * sin(dPhi),
                             bcyl->halflengthZ());
            vtx.emplace_back(bcyl->outerRadius() * cos(-dPhi),
                             bcyl->outerRadius() * sin(-dPhi),
                             bcyl->halflengthZ());
            vtx.emplace_back(bcyl->innerRadius() * cos(-dPhi),
                             bcyl->innerRadius() * sin(-dPhi),
                             bcyl->halflengthZ());
            vtx.emplace_back(bcyl->outerRadius() * cos(dPhi),
                             bcyl->outerRadius() * sin(dPhi),
                             -bcyl->halflengthZ());
            vtx.emplace_back(bcyl->innerRadius() * cos(dPhi),
                             bcyl->innerRadius() * sin(dPhi),
                             -bcyl->halflengthZ());
            vtx.emplace_back(bcyl->outerRadius() * cos(-dPhi),
                             bcyl->outerRadius() * sin(-dPhi),
                             -bcyl->halflengthZ());
            vtx.emplace_back(bcyl->innerRadius() * cos(-dPhi),
                             bcyl->innerRadius() * sin(-dPhi),
                             -bcyl->halflengthZ());
            vtx.emplace_back(bcyl->outerRadius(), 0.,
                             0.);  // to distinguish phi intervals for cylinders
                                   // aligned with z axis
            edges.emplace_back(std::make_pair(2, 3));
            edges.emplace_back(std::make_pair(4, 5));
            edges.emplace_back(std::make_pair(6, 7));
            edges.emplace_back(std::make_pair(8, 9));
            if (bcyl->type() == 1 || bcyl->type() == 3) {
                edges.emplace_back(std::make_pair(3, 5));
                edges.emplace_back(std::make_pair(7, 9));
            }
            if (bcyl->type() == 2 || bcyl->type() == 3) {
                edges.emplace_back(std::make_pair(2, 4));
                edges.emplace_back(std::make_pair(6, 8));
            }
        }
    }
    if (cyl) {
        dPhi = cyl->halfPhiSector();
        vtx.emplace_back(0., 0., cyl->halflengthZ());
        vtx.emplace_back(0., 0., -cyl->halflengthZ());
        edges.emplace_back(std::make_pair(0, 1));
        if (dPhi < M_PI) {
            vtx.emplace_back(cyl->outerRadius() * cos(dPhi),
                             cyl->outerRadius() * sin(dPhi),
                             cyl->halflengthZ());
            vtx.emplace_back(cyl->innerRadius() * cos(dPhi),
                             cyl->innerRadius() * sin(dPhi),
                             cyl->halflengthZ());
            vtx.emplace_back(cyl->outerRadius() * cos(-dPhi),
                             cyl->outerRadius() * sin(-dPhi),
                             cyl->halflengthZ());
            vtx.emplace_back(cyl->outerRadius() * cos(-dPhi),
                             cyl->outerRadius() * sin(-dPhi),
                             cyl->halflengthZ());
            vtx.emplace_back(cyl->outerRadius() * cos(dPhi),
                             cyl->outerRadius() * sin(dPhi),
                             -cyl->halflengthZ());
            vtx.emplace_back(cyl->innerRadius() * cos(dPhi),
                             cyl->innerRadius() * sin(dPhi),
                             -cyl->halflengthZ());
            vtx.emplace_back(cyl->outerRadius() * cos(-dPhi),
                             cyl->outerRadius() * sin(-dPhi),
                             -cyl->halflengthZ());
            vtx.emplace_back(cyl->outerRadius() * cos(-dPhi),
                             cyl->outerRadius() * sin(-dPhi),
                             -cyl->halflengthZ());
            vtx.emplace_back(cyl->outerRadius(), 0.,
                             0.);  // to distinguish phi intervals for cylinders
                                   // aligned with z axis
            edges.emplace_back(std::make_pair(2, 3));
            edges.emplace_back(std::make_pair(4, 5));
            edges.emplace_back(std::make_pair(6, 7));
            edges.emplace_back(std::make_pair(8, 9));
        }
    }

    if (spb) {
        const std::vector<std::pair<double, double>> vtcs = spb->xyVertices();
        for (const auto& vtc : vtcs) {
            vtx.emplace_back(vtc.first, vtc.second, spb->halflengthZ());
            vtx.emplace_back(vtc.first, vtc.second, -spb->halflengthZ());
            edges.emplace_back(std::make_pair(vtx.size() - 2, vtx.size() - 1));
            if (vtx.size() > 2) {
                edges.emplace_back(
                    std::make_pair(vtx.size() - 4, vtx.size() - 2));
                edges.emplace_back(
                    std::make_pair(vtx.size() - 3, vtx.size() - 1));
            }
            if (vtx.size() > 4) {  // some diagonals
                edges.emplace_back(std::make_pair(vtx.size() - 2, 1));
                edges.emplace_back(std::make_pair(vtx.size() - 1, 0));
            }
        }
        edges.emplace_back(std::make_pair(0, vtx.size() - 2));
        edges.emplace_back(std::make_pair(1, vtx.size() - 1));
    }

    if (prism) {
        const std::vector<std::pair<double, double>> vtcs = prism->xyVertices();
        for (const auto& vtc : vtcs) {
            vtx.emplace_back(vtc.first, vtc.second, prism->halflengthZ());
            vtx.emplace_back(vtc.first, vtc.second, -prism->halflengthZ());
            edges.emplace_back(std::make_pair(vtx.size() - 2, vtx.size() - 1));
            if (vtx.size() > 2) {
                edges.emplace_back(
                    std::make_pair(vtx.size() - 4, vtx.size() - 2));
                edges.emplace_back(
                    std::make_pair(vtx.size() - 3, vtx.size() - 1));
            }
        }
        edges.emplace_back(std::make_pair(0, vtx.size() - 2));
        edges.emplace_back(std::make_pair(1, vtx.size() - 1));
    }

    std::vector<Amg::Vector3D> vtxt;

    for (unsigned int ie = 0; ie < vtx.size(); ie++) {
        Amg::Vector3D gp = transform * vtx[ie];
        vtxt.push_back(gp);

        double phi = gp.phi() + M_PI;
        double rad = gp.perp();

        // collect limits from vertices
        minX = std::min(minX, gp[0]);
        maxX = std::max(maxX, gp[0]);
        minY = std::min(minY, gp[1]);
        maxY = std::max(maxY, gp[1]);
        minZ = std::min(minZ, gp[2]);
        maxZ = std::max(maxZ, gp[2]);
        minR = std::min(minR, rad);
        maxR = std::max(maxR, rad);
        maxPhi = std::max(maxPhi, phi);
        minPhi = std::min(minPhi, phi);
    }

    if (cyl || bcyl) {

        double ro = cyl ? cyl->outerRadius() : bcyl->outerRadius();
        double ri = cyl ? cyl->innerRadius() : bcyl->innerRadius();
        // z span corrected for theta inclination
        Amg::Vector3D dir =
            (vtxt[edges[0].first] - vtxt[edges[0].second]).unit();
        maxZ += ro * sin(dir.theta());
        minZ += -ro * sin(dir.theta());
        // azimuthal & radial extent
        if (ro < minR) {  // excentric object, phi span driven by z-R extent
            // calculate point of closest approach
            Trk::PerigeeSurface peri;
            Trk::Intersection closest =
                peri.straightLineIntersection(vtxt[1], dir);
            double le = (vtxt[0] - vtxt[1]).norm();
            if ((closest.position - vtxt[0]).norm() < le &&
                (closest.position - vtxt[1]).norm() < le) {
                if (minR > closest.position.perp() - ro)
                    minR = std::max(0., closest.position.perp() - ro);
                // use for phi check
                double phiClosest = closest.position.phi() + M_PI;
                if (phiClosest < minPhi || phiClosest > maxPhi) {
                    double phiTmp = minPhi;
                    minPhi = maxPhi;
                    maxPhi = phiTmp;
                }
            } else
                minR = std::max(0., minR - ro * std::abs(dir.z()));

            minPhi += -atan(ro / minR);
            maxPhi += atan(ro / minR);
            if (minPhi < 0)
                minPhi += 2 * M_PI;
            if (maxPhi > 2 * M_PI)
                maxPhi += -2 * M_PI;

            maxR += ro * std::abs(cos(dir.theta()));
        } else {

            double rAx = std::max(vtxt[0].perp(), vtxt[1].perp());
            if (rAx < ri)
                minR = ri - rAx;
            else
                minR = std::max(0., minR - ro * std::abs(cos(dir.theta())));

            // loop over edges to check inner radial extent
            Trk::PerigeeSurface peri;
            for (unsigned int ie = 0; ie < edges.size(); ie++) {
                Amg::Vector3D dir =
                    (vtxt[edges[ie].first] - vtxt[edges[ie].second]).unit();
                Trk::Intersection closest =
                    peri.straightLineIntersection(vtxt[edges[ie].second], dir);
                double le =
                    (vtxt[edges[ie].first] - vtxt[edges[ie].second]).norm();
                if ((closest.position - vtxt[edges[ie].first]).norm() < le &&
                    (closest.position - vtxt[edges[ie].second]).norm() < le)
                    if (minR > closest.position.perp())
                        minR = closest.position.perp();
            }

            if (vtxt.size() > 10) {  // cylindrical section
                // find spread of phi extent at section (-) boundary : vertices
                // 4,5,8,9
                double phiSecLmin = std::min(
                    std::min(vtxt[4].phi() + M_PI, vtxt[5].phi() + M_PI),
                    std::min(vtxt[8].phi() + M_PI, vtxt[9].phi() + M_PI));
                double phiSecLmax = std::max(
                    std::max(vtxt[4].phi() + M_PI, vtxt[5].phi() + M_PI),
                    std::max(vtxt[8].phi() + M_PI, vtxt[9].phi() + M_PI));
                // find spread of phi extent at section (+) boundary : vertices
                // 2,3,6,7
                double phiSecUmin = std::min(
                    std::min(vtxt[2].phi() + M_PI, vtxt[3].phi() + M_PI),
                    std::min(vtxt[6].phi() + M_PI, vtxt[7].phi() + M_PI));
                double phiSecUmax = std::max(
                    std::max(vtxt[2].phi() + M_PI, vtxt[3].phi() + M_PI),
                    std::max(vtxt[6].phi() + M_PI, vtxt[7].phi() + M_PI));
                minPhi = std::min(std::min(phiSecLmin, phiSecLmax),
                                  std::min(phiSecUmin, phiSecUmax));
                maxPhi = std::max(std::max(phiSecLmin, phiSecLmax),
                                  std::max(phiSecUmin, phiSecUmax));
                if (vtxt[10].phi() + M_PI < minPhi ||
                    vtxt[10].phi() + M_PI > maxPhi) {
                    minPhi = 3 * M_PI;
                    maxPhi = 0.;
                    double phiTmp;
                    for (unsigned int iv = 2; iv < vtxt.size(); iv++) {
                        phiTmp = vtxt[iv].phi() + M_PI;
                        if (phiTmp < M_PI)
                            phiTmp += 2 * M_PI;
                        minPhi = phiTmp < minPhi ? phiTmp : minPhi;
                        maxPhi = phiTmp > maxPhi ? phiTmp : maxPhi;
                    }
                    if (minPhi > 2 * M_PI)
                        minPhi += -2 * M_PI;
                    if (maxPhi > 2 * M_PI)
                        maxPhi += -2 * M_PI;
                }
            } else {
                minPhi = 0.;
                maxPhi = 2 * M_PI;
                maxR += ro * std::abs(std::cos(dir.theta()));
            }
            if (minPhi >= maxPhi && (minPhi - maxPhi) < M_PI) {
                minPhi = 0.;
                maxPhi = 2 * M_PI;
            }
        }
    }  // end cyl & bcyl

    if (!cyl && !bcyl) {
        // loop over edges to check inner radial extent
        Trk::PerigeeSurface peri;
        for (unsigned int ie = 0; ie < edges.size(); ie++) {
            Amg::Vector3D dir =
                (vtxt[edges[ie].first] - vtxt[edges[ie].second]).unit();
            Trk::Intersection closest =
                peri.straightLineIntersection(vtxt[edges[ie].second], dir);
            double le = (vtxt[edges[ie].first] - vtxt[edges[ie].second]).norm();
            if ((closest.position - vtxt[edges[ie].first]).norm() < le &&
                (closest.position - vtxt[edges[ie].second]).norm() < le)
                if (minR > closest.position.perp())
                    minR = closest.position.perp();
        }  // end loop over edges
        // verify phi span - may run across step
        if (std::abs(maxPhi - minPhi) > M_PI) {
            double phiTmp = minPhi;
            minPhi = 3 * M_PI;
            maxPhi = 0.;  // redo the search
            for (unsigned int iv = 0; iv < vtxt.size(); iv++) {
                phiTmp = vtxt[iv].phi() + M_PI;
                if (phiTmp < M_PI)
                    phiTmp += 2 * M_PI;
                minPhi = phiTmp < minPhi ? phiTmp : minPhi;
                maxPhi = phiTmp > maxPhi ? phiTmp : maxPhi;
            }
            if (minPhi > 2 * M_PI)
                minPhi += -2 * M_PI;
            if (maxPhi > 2 * M_PI)
                maxPhi += -2 * M_PI;
            if (minPhi >= maxPhi && (minPhi - maxPhi) < M_PI) {
                minPhi = 0.;
                maxPhi = 2 * M_PI;
            }
        }
    }

    if (cyl || bcyl || box || trd || dtrd || spb || prism) {
        span.zMin = minZ - zTol;
        span.zMax = maxZ - +zTol;
        minPhi = (minPhi - phiTol) < 0 ? minPhi - phiTol + 2 * M_PI
                                       : minPhi - phiTol;
        span.phiMin = minPhi;
        maxPhi = (maxPhi + phiTol) > 2 * M_PI ? maxPhi + phiTol - 2 * M_PI
                                              : maxPhi + phiTol;
        span.phiMax = maxPhi;
        span.rMin = std::max(70.001, minR - zTol);
        span.rMax = maxR + zTol;
        span.xMin = minX - zTol;
        span.xMax = maxX - +zTol;
        span.yMin = minY - zTol;
        span.yMax = maxY - +zTol;
    } else {
        DEBUG_TRACE(std::cout
                    << " Trk::VolumeConverter::volume shape not recognized "
                    << std::endl);
    }
    const Trk::VolumeSpan* newSpan = new Trk::VolumeSpan(span);
    return newSpan;
}

double Trk::VolumeConverter::calculateVolume(const Trk::Volume* vol,
                                             bool nonBooleanOnly,
                                             double precision) const {

    double volume = -1.;

    if (!vol)
        return volume;

    const Trk::CylinderVolumeBounds* cyl =
        dynamic_cast<const Trk::CylinderVolumeBounds*>(&(vol->volumeBounds()));
    const Trk::CuboidVolumeBounds* box =
        dynamic_cast<const Trk::CuboidVolumeBounds*>(&(vol->volumeBounds()));
    const Trk::TrapezoidVolumeBounds* trd =
        dynamic_cast<const Trk::TrapezoidVolumeBounds*>(&(vol->volumeBounds()));
    const Trk::BevelledCylinderVolumeBounds* bcyl =
        dynamic_cast<const Trk::BevelledCylinderVolumeBounds*>(
            &(vol->volumeBounds()));
    const Trk::PrismVolumeBounds* prism =
        dynamic_cast<const Trk::PrismVolumeBounds*>(&(vol->volumeBounds()));
    const Trk::SimplePolygonBrepVolumeBounds* spb =
        dynamic_cast<const Trk::SimplePolygonBrepVolumeBounds*>(
            &(vol->volumeBounds()));
    const Trk::CombinedVolumeBounds* comb =
        dynamic_cast<const Trk::CombinedVolumeBounds*>(&(vol->volumeBounds()));
    const Trk::SubtractedVolumeBounds* sub =
        dynamic_cast<const Trk::SubtractedVolumeBounds*>(
            &(vol->volumeBounds()));

    if (cyl)
        return volume = 2 * cyl->halfPhiSector() *
                        (cyl->outerRadius() * cyl->outerRadius() -
                         cyl->innerRadius() * cyl->innerRadius()) *
                        cyl->halflengthZ();
    if (box)
        return volume = (8 * box->halflengthX() * box->halflengthY() *
                         box->halflengthZ());
    if (trd)
        return volume = (4 * (trd->minHalflengthX() + trd->maxHalflengthX()) *
                         trd->halflengthY() * trd->halflengthZ());
    if (bcyl) {
        int type = bcyl->type();
        if (type < 1)
            return volume = 2 * bcyl->halfPhiSector() *
                            (bcyl->outerRadius() * bcyl->outerRadius() -
                             bcyl->innerRadius() * bcyl->innerRadius()) *
                            bcyl->halflengthZ();
        if (type == 1)
            return volume = 2 * bcyl->halflengthZ() *
                            (bcyl->halfPhiSector() * bcyl->outerRadius() *
                                 bcyl->outerRadius() -
                             bcyl->innerRadius() * bcyl->innerRadius() *
                                 tan(bcyl->halfPhiSector()));
        if (type == 2)
            return volume = 2 * bcyl->halflengthZ() *
                            (-bcyl->halfPhiSector() * bcyl->innerRadius() *
                                 bcyl->innerRadius() +
                             bcyl->outerRadius() * bcyl->outerRadius() *
                                 tan(bcyl->halfPhiSector()));
        if (type == 3)
            return volume = 2 * bcyl->halflengthZ() *
                            tan(bcyl->halfPhiSector()) *
                            (bcyl->outerRadius() * bcyl->outerRadius() -
                             bcyl->innerRadius() * bcyl->innerRadius());
    }
    if (prism) {

        std::vector<std::pair<double, double>> v = prism->xyVertices();
        // replaced by triangle formula
        // double a2 = v[1].first * v[1].first + v[1].second * v[1].second +
        // v[0].first * v[0].first +
        //            v[0].second * v[0].second - 2 * (v[0].first * v[1].first +
        //            v[0].second * v[1].second);
        // double c2 = v[2].first * v[2].first + v[2].second * v[2].second +
        // v[0].first * v[0].first +
        //            v[0].second * v[0].second - 2 * (v[0].first * v[2].first +
        //            v[0].second * v[2].second);
        // double ca = v[1].first * v[2].first + v[1].second * v[2].second +
        // v[0].first * v[0].first +
        //            v[0].second * v[0].second - v[0].first * v[1].first -
        //            v[0].second * v[1].second - v[0].first * v[2].first -
        //            v[0].second * v[2].second;
        // double vv2 = (a2 * c2 - ca * ca);
        // double vv = 0.;
        // fix nans
        // if (vv2 > 0.)  vv = sqrt(vv2);
        // return volume = vv * prism->halflengthZ();

        double vv = v[0].first * (v[1].second - v.back().second);
        for (unsigned int i = 1; i < v.size() - 1; i++)
            vv += v[i].first * (v[i + 1].second - v[i - 1].second);
        vv += v.back().first * (v[0].second - v[v.size() - 2].second);

        return volume = vv * prism->halflengthZ();
    }
    if (spb) {
        std::vector<std::pair<double, double>> v = spb->xyVertices();

        // replaced by triangle formula
        // this should give a set of non-overlapping prisms
        // temporary owner of auxiliary volumes
        // volume = 0.;
        // auto garbage = std::make_unique<std::vector< const Trk::Volume*> >();
        // std::vector<std::pair<const Trk::Volume*, const Trk::Volume* > >
        //  subvols =splitComposedVolume(spb->combinedVolume(), garbage.get());
        // for (auto prism : subvols) {
        //  volume += calculateVolume( prism.first );
        //}
        // for (auto volTmp : *garbage.get()) delete volTmp;
        // return volume;

        double vv = v[0].first * (v[1].second - v.back().second);
        for (unsigned int i = 1; i < v.size() - 1; i++)
            vv += v[i].first * (v[i + 1].second - v[i - 1].second);
        vv += v.back().first * (v[0].second - v[v.size() - 2].second);

        return volume = vv * spb->halflengthZ();
    }

    if (nonBooleanOnly)
        return volume;

    if (comb || sub)
        return resolveBooleanVolume(vol, precision);

    return volume;
}

double Trk::VolumeConverter::estimateFraction(
    std::pair<const Trk::Volume*, const Trk::Volume*> sub,
    double precision) const {

    double fraction = -1.;

    if (!sub.first)
        return fraction = 0.;

    if (sub.first && !sub.second)
        return fraction = 1.;

    std::pair<bool, const Trk::Volume*> overlap =
        m_intersectionHelper.intersect(sub.first, sub.second);

    if (overlap.first && !overlap.second)
        return fraction = 1.;
    else if (overlap.first && overlap.second) {
        fraction = 1. - calculateVolume(overlap.second, true, precision) /
                            calculateVolume(sub.first, true, precision);
        delete overlap.second;
        return fraction;
    }
    //  resolve embedded volumes

    // trivial within required precision
    double volA = calculateVolume(sub.first, true, precision);
    double volB = calculateVolume(sub.second, true, precision);
    if ((volA > 0 && volA < precision) || (volB > 0 && volB < precision))
        return 1.;

    return fraction;
}

void Trk::VolumeConverter::collectMaterial(const GeoVPhysVol* pv,
                                           Trk::MaterialProperties& layMat,
                                           double sf) const {
    // sf is the area of the layer collecting the material

    // solution relying on GeoModel
    // currently involves hit&miss on-fly calculation of boolean volumes
    // GeoModelTools::MaterialComponent  mat =
    // gm_materialHelper.collectMaterial(pv); Trk::Material newMP =
    // convert(mat.first); double d = mat.second / sf; layMat.addMaterial(newMP,
    // d / newMP.x0()); return;

    std::vector<Trk::MaterialComponent> materialContent;
    collectMaterialContent(pv, materialContent);

    for (auto mat : materialContent) {
        if (mat.second < 0)
            continue;  // protection unsolved booleans
        double d = sf > 0 ? mat.second / sf : 0.;
        if (d > 0)
            layMat.addMaterial(mat.first,
                               (mat.first.X0 > 0 ? d / mat.first.X0 : 0.));
    }
}

void Trk::VolumeConverter::collectMaterialContent(
    const GeoVPhysVol* gv,
    std::vector<Trk::MaterialComponent>& materialContent) const {

    // solution relying on GeoModel
    // currently involves hit&miss on-fly calculation of boolean volumes
    // GeoModelTools::MaterialComponent  mat =
    // gm_materialHelper.collectMaterial(pv); Trk::Material newMP =
    // convert(mat.first); materialContent.push_back( Trk::MaterialComponent(
    // newMP, mat.second) ); return;

    const GeoLogVol* lv = gv->getLogVol();
    Trk::Material mat = m_materialConverter.convert(lv->getMaterial());

    double motherVolume = 0.;

    // skip volume calculation for dummy material configuration
    if (!m_materialConverter.dummy_material(lv->getMaterial())) {
        const GeoShape* sh = lv->getShape();
        while (sh && sh->type() == "Shift") {
            const GeoShapeShift* shift = dynamic_cast<const GeoShapeShift*>(sh);
            sh = shift ? shift->getOp() : nullptr;
        }

        bool isBoolean =
            sh && (sh->type() == "Subtraction" || sh->type() == "Union" ||
                   sh->type() == "Intersection");

        if (isBoolean) {
            Amg::Transform3D transf{Amg::Transform3D::Identity()};
            std::unique_ptr<const Trk::Volume> vol =
                std::make_unique<const Trk::Volume>(
                    *m_geoShapeConverter.translateGeoShape(sh, &transf));
            motherVolume =
                calculateVolume(vol.get(), false, pow(1.e-3 * mat.X0, 3));
            if (motherVolume < 0) {
                //  m_geoShapeConverter.decodeShape(sh);
            }
        } else
            motherVolume = lv->getShape()->volume();
    }

    unsigned int nc = gv->getNChildVols();
    double childVol = 0;
    std::string cPrevious = " ";
    size_t nIdentical = 0;
    std::vector<Trk::MaterialComponent> childMat;
    for (unsigned int ic = 0; ic < nc; ic++) {
        const GeoVPhysVol* cv = &(*(gv->getChildVol(ic)));
        std::string cname = cv->getLogVol()->getName();
        if (cname == cPrevious)
            nIdentical++;  // assuming identity for identical name and branching
                           // history
        else {             // scale and collect material from previous item
            for (auto cmat : childMat) {
                materialContent.push_back(Trk::MaterialComponent(
                    cmat.first, nIdentical * cmat.second));
                childVol += materialContent.back().second;
            }
            childMat.clear();  // reset
            nIdentical = 1;    // current
            collectMaterialContent(cv, childMat);
        }
    }
    for (auto cmat : childMat) {
        materialContent.push_back(
            Trk::MaterialComponent(cmat.first, nIdentical * cmat.second));
        childVol += materialContent.back().second;
    }
    if (motherVolume > 0 && childVol > 0)
        motherVolume += -1. * childVol;

    DEBUG_TRACE(std::cout << "collected material:" << lv->getName()
                          << ":made of:" << lv->getMaterial()->getName()
                          << ":density(g/mm3)" << mat.rho
                          << ":mass:" << mat.rho * motherVolume << std::endl;);
    materialContent.push_back(
        std::pair<Trk::Material, double>(mat, motherVolume));
}

double Trk::VolumeConverter::leadingVolume(const GeoShape* sh) const {

    if (sh->type() == "Subtraction") {
        const GeoShapeSubtraction* sub =
            dynamic_cast<const GeoShapeSubtraction*>(sh);
        if (sub)
            return leadingVolume(sub->getOpA());
    }
    if (sh->type() == "Union") {
        const GeoShapeUnion* uni = dynamic_cast<const GeoShapeUnion*>(sh);
        if (uni)
            return leadingVolume(uni->getOpA()) + leadingVolume(uni->getOpB());
    }
    if (sh->type() == "Intersection") {
        const GeoShapeIntersection* intr =
            dynamic_cast<const GeoShapeIntersection*>(sh);
        if (intr)
            return std::min(leadingVolume(intr->getOpA()),
                            leadingVolume(intr->getOpB()));
    }
    if (sh->type() == "Shift") {
        const GeoShapeShift* shift = dynamic_cast<const GeoShapeShift*>(sh);
        if (shift)
            return leadingVolume(shift->getOp());
    }

    return sh->volume();
}
