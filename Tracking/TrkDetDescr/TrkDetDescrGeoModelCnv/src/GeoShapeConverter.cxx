/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Trk
#include "TrkDetDescrGeoModelCnv/GeoShapeConverter.h"

#include <algorithm>
#include <cmath>

#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "TrkVolumes/BoundarySurface.h"
#include "TrkVolumes/CombinedVolumeBounds.h"
#include "TrkVolumes/CuboidVolumeBounds.h"
#include "TrkVolumes/CylinderVolumeBounds.h"
#include "TrkVolumes/DoubleTrapezoidVolumeBounds.h"
#include "TrkVolumes/SimplePolygonBrepVolumeBounds.h"
#include "TrkVolumes/SubtractedVolumeBounds.h"
#include "TrkVolumes/TrapezoidVolumeBounds.h"
#include "TrkVolumes/Volume.h"
#include "TrkVolumes/VolumeExcluder.h"
// GeoModelKernel
#include "GaudiKernel/SystemOfUnits.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoCons.h"
#include "GeoModelKernel/GeoDefinitions.h"
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
#include "GeoModelKernel/GeoVolumeCursor.h"

namespace {
// commonly used angles, ±90°, 180°
constexpr double p90deg(90.0 * Gaudi::Units::deg),
                 m90deg(-90.0 * Gaudi::Units::deg), 
                 p180deg(180.0 * Gaudi::Units::deg);
                 
Amg::Transform3D* makeTransform(const Amg::Transform3D& trf) {
    return std::make_unique<Amg::Transform3D>(trf).release();
}
}  // namespace

namespace Trk {
GeoShapeConverter::GeoShapeConverter() : AthMessaging{"GeoShapeConverter"} {}

std::unique_ptr<CylinderVolumeBounds> GeoShapeConverter::convert(const GeoTubs* gtubs) {
    // get the dimensions
    double rMin = gtubs->getRMin();
    double rMax = gtubs->getRMax();
    double halflength = gtubs->getZHalfLength();
    // create the volumeBounds
    return std::make_unique<CylinderVolumeBounds>(rMin, rMax, halflength);
}

std::unique_ptr<CylinderVolumeBounds> GeoShapeConverter::convert(const GeoTube* gtube) {
    // get the dimensions
    double rMin = gtube->getRMin();
    double rMax = gtube->getRMax();
    double halflength = gtube->getZHalfLength();
    // create the volumeBounds
    return std::make_unique<CylinderVolumeBounds>(rMin, rMax, halflength);
}

std::unique_ptr<CylinderVolumeBounds> GeoShapeConverter::convert(const GeoPcon* gpcon, 
                                                                 std::vector<double>& zbounds) {

    // get the pcon igredients ...
    unsigned int numberOfPlanes = gpcon->getNPlanes();
    double rMin{10.e10}, rMax{-10.e10}, zMin{10.e10}, zMax{-10.e10};

    for (unsigned int iplane = 0; iplane < numberOfPlanes; ++iplane) {
        zMin = std::min(gpcon->getZPlane(iplane), zMin);
        zMax = std::max(gpcon->getZPlane(iplane), zMax);
        rMin = std::min(gpcon->getRMinPlane(iplane), rMin);
        rMax = std::max(gpcon->getRMaxPlane(iplane), rMax);
    }
    zbounds.push_back(zMin);
    zbounds.push_back(zMax);

    return std::make_unique<CylinderVolumeBounds>(rMin, rMax,
                                                  0.5 * (zMax - zMin));
}

std::unique_ptr<CuboidVolumeBounds> GeoShapeConverter::convert(const GeoBox* gbox) {
    double halfX = gbox->getXHalfLength();
    double halfY = gbox->getYHalfLength();
    double halfZ = gbox->getZHalfLength();
    return std::make_unique<CuboidVolumeBounds>(halfX, halfY, halfZ);
}

std::unique_ptr<Volume> GeoShapeConverter::translateGeoShape(const GeoShape* sh, 
                                                            const Amg::Transform3D& transf) const {
    std::unique_ptr<Volume> vol{};
    double tol = 0.1;

    ATH_MSG_DEBUG(" translateGeoShape " << sh->type());

    if (sh->type() == "Trap") {
        const GeoTrap* trap = dynamic_cast<const GeoTrap*>(sh);
        std::unique_ptr<TrapezoidVolumeBounds> volBounds{};
        if (trap->getDxdyndzp() < trap->getDxdyndzn()) {
            volBounds = std::make_unique<TrapezoidVolumeBounds>(trap->getDxdyndzp(), 
                                                                trap->getDxdyndzn(), 
                                                                trap->getDydzn(),
                                                                trap->getZHalfLength());
        } else {
            volBounds = std::make_unique<TrapezoidVolumeBounds>(trap->getDxdyndzn(), 
                                                                trap->getDxdyndzp(), 
                                                                trap->getDydzn(),
                                                                trap->getZHalfLength());
        }
        return std::make_unique<Volume>(makeTransform(transf), volBounds.release());
    } else if (sh->type() == "Pgon") {
        const GeoPgon* pgon = dynamic_cast<const GeoPgon*>(sh);
        double hlz = 0.5 * std::abs(pgon->getZPlane(1) - pgon->getZPlane(0));
        double phiH = pgon->getDPhi() / (2. * pgon->getNSides());
        double hly = 0.5 * std::cos(phiH) * (pgon->getRMaxPlane(0) - pgon->getRMinPlane(0));
        double dly = 0.5 * std::cos(phiH) * (pgon->getRMaxPlane(0) + pgon->getRMinPlane(0));
        double hlxmin = pgon->getRMinPlane(0) * std::sin(phiH);
        double hlxmax = pgon->getRMaxPlane(0) * std::sin(phiH);
        if (pgon->getDPhi() == 2 * M_PI) {

           auto volBounds = std::make_unique<CylinderVolumeBounds>(pgon->getRMaxPlane(0), hlz);
           auto subBounds = std::make_unique<CuboidVolumeBounds>(hlxmax + tol, hlxmax + tol,
                                                                 hlz + tol);
            auto volume = std::make_unique<Volume>(makeTransform(transf), volBounds.release());
            auto bVol = std::make_unique<Volume>(nullptr, subBounds.release());
            const unsigned int nsides(pgon->getNSides());
            const double twicePhiH(2.0 * phiH);
            const double xTranslationDistance = hlxmax + std::cos(phiH) * (pgon->getRMaxPlane(0));

            const Amg::Translation3D xTranslation{xTranslationDistance, 0., 0.};
            for (unsigned int i = 0; i < nsides; i++) {
                const double angle = i * twicePhiH;
                Amg::Transform3D totalTransform = transf * Amg::getRotateZ3D(angle) * xTranslation;
                auto volS = std::make_unique<Volume>(*bVol, totalTransform);
                auto combBounds =std::make_unique<SubtractedVolumeBounds>(volume.release(),
                                                                          volS.release());
                volume = std::make_unique<Volume>(nullptr, combBounds.release());
            }
            return volume;
        }

        if (pgon->getNSides() == 1) {
            auto volBounds = std::make_unique<TrapezoidVolumeBounds>(hlxmin, hlxmax, hly, hlz);
            Amg::Transform3D totalTransform = transf *
                                              Amg::getRotateZ3D(m90deg) *
                                              Amg::Translation3D(0., dly, 0.);
            return std::make_unique<Volume>(makeTransform(std::move(totalTransform)), 
                                            volBounds.release());
        }

        if (pgon->getNSides() == 2) {
            auto cylBounds = std::make_unique<CylinderVolumeBounds>(0, dly + hly, hlz);
            return std::make_unique<Volume>(makeTransform(transf),
                                            cylBounds.release());
        }
        return vol;
    } else if (sh->type() == "Trd") {
        const GeoTrd* trd = dynamic_cast<const GeoTrd*>(sh);
        //
        double x1 = trd->getXHalfLength1();
        double x2 = trd->getXHalfLength2();
        double y1 = trd->getYHalfLength1();
        double y2 = trd->getYHalfLength2();
        double z = trd->getZHalfLength();
        //
        ATH_MSG_DEBUG(" Trd x1 " << x1 << " x2 " << x2 << " y1 " << y1 << " y2 "
                                 << y2 << " z " << z);
        // Note this flip comes from the y axis in Tracking -> z axis in
        // Geomodel
        if (y1 == y2) {
            if (x1 <= x2) {
                auto volBounds = std::make_unique<TrapezoidVolumeBounds>(x1, x2, z, y1);
                Amg::Transform3D totalTransform = transf * Amg::getRotateX3D(p90deg);
                vol = std::make_unique<Volume>(makeTransform(std::move(totalTransform)),
                                               volBounds.release());
                
                ATH_MSG_DEBUG(" Trd new volume case 1 Trapezoid minHalflengthX "
                              << volBounds->minHalflengthX()
                              << " maxHalflengthX() "
                              << volBounds->maxHalflengthX());
            } else {

                auto volBounds = std::make_unique<TrapezoidVolumeBounds>(x2, x1, z, y1);
                Amg::Transform3D totalTransform = transf *
                                                  Amg::getRotateY3D(p180deg) *
                                                  Amg::getRotateZ3D(p180deg);
                vol = std::make_unique<Volume>(makeTransform(std::move(totalTransform)),
                                               volBounds.release());

                if (msgLvl(MSG::DEBUG)) {
                    const Amg::Vector3D top{-x1, y1, z}, bottom{-x2, y1, -z},
                                        top2{x1, y1, z}, bottom2{x2, y1, -z};
                    ATH_MSG_DEBUG(" Trd new volume case 2 Trapezoid minHalflengthX "
                        << volBounds->minHalflengthX() << " maxHalflengthX() "
                        << volBounds->maxHalflengthX());
                    ATH_MSG_DEBUG(" Original topLocal " << Amg::toString(top) << " Radius " << top.perp());
                    ATH_MSG_DEBUG(" Original bottomLocal "<< Amg::toString(bottom) << " Radius " << bottom.perp());
                    Amg::Vector3D topG{transf * top}, bottomG{transf * bottom};
                    ATH_MSG_DEBUG(" top Global " << Amg::toString(topG)<< " Radius " << topG.perp());
                    ATH_MSG_DEBUG(" bottom Global " << Amg::toString(bottomG)<< " Radius "<< bottomG.perp());
                    topG = transf * top2;
                    bottomG = transf * bottom2;
                    ATH_MSG_DEBUG(" top2 Global x " << Amg::toString(topG)<< " Radius " << topG.perp());
                    ATH_MSG_DEBUG(" bottom2 Global " << Amg::toString(bottomG) << " Radius: " << bottomG.perp());
                    const Amg::Vector3D topR{-x2, z, y1}, bottomR{-x1, -z, y1},
                                        top2R{x2, z, y1}, bottom2R{x1, -z, y1};
                    topG = totalTransform * topR;
                    bottomG = totalTransform * bottomR;
                    ATH_MSG_DEBUG(" topR Global " << Amg::toString(topG)<< " Radius " << topG.perp());
                    ATH_MSG_DEBUG(" bottomR Global " << Amg::toString(bottomG)<< " Radius "<< bottomG.perp());
                    topG = totalTransform * top2R;
                    bottomG = totalTransform * bottom2R;
                    ATH_MSG_DEBUG(" top2R Global " << Amg::toString(topG) << " Radius "<< topG.perp());
                    ATH_MSG_DEBUG(" bottom2R Global " << Amg::toString(bottomG)<< " Radius "<< bottomG.perp());

                    ATH_MSG_DEBUG(" Original bottomLocal "<< Amg::toString(bottom) << " Radius "<< bottom.perp());
                    ATH_MSG_DEBUG(" topLocal x " << Amg::toString(topR)<< " Radius " << topR.perp());
                    topG = Amg::getRotateY3D(p180deg) * topR;
                    ATH_MSG_DEBUG(" topLocal Y Flip " << Amg::toString(topG)<< " Radius " << topG.perp());
                    topG = Amg::getRotateY3D(p180deg) * topR;
                    ATH_MSG_DEBUG(" topLocal X Flip " << Amg::toString(topG) << " Radius "<< topG.perp());
                    topG = Amg::getRotateZ3D(p90deg) * topR;
                    ATH_MSG_DEBUG(" topLocal XY Flip " << Amg::toString(topG) << " Radius " << topG.perp());
                    topG = Amg::getRotateY3D(p180deg) * topR;
                    ATH_MSG_DEBUG(" topLocal YZ Flip " << Amg::toString(topG) << " Radius " << topG.perp());
                    topG = Amg::getRotateY3D(p90deg) * topR;
                    ATH_MSG_DEBUG(" topLocal XZ Flip " << Amg::toString(topG) << " Radius " << topG.perp());
                    topG = Amg::getRotateY3D(p180deg) * topR;
                    ATH_MSG_DEBUG(" topLocal XY sign Flip  x "<< Amg::toString(topG) << " Radius " << topG.perp());
                }
            }
            return vol;
        } else if (x1 == x2) {
            if (y1 < y2) {
                std::unique_ptr<TrapezoidVolumeBounds> volBounds =
                    std::make_unique<TrapezoidVolumeBounds>(y1, y2, z, x1);
                ATH_MSG_DEBUG(" Trd new volume case 3 Trapezoid minHalflengthX "
                              << volBounds->minHalflengthX()<< " maxHalflengthX() "
                              << volBounds->maxHalflengthX());
                Amg::Transform3D totalTransform = transf *
                                                  Amg::getRotateZ3D(p90deg) *
                                                  Amg::getRotateX3D(p90deg);
                vol = std::make_unique<Volume>(makeTransform(std::move(totalTransform)),
                                               volBounds.release());

            } else {
                auto volBounds = std::make_unique<TrapezoidVolumeBounds>(y2, y1, z, x1);
                ATH_MSG_DEBUG(" Trd new volume case 4 Trapezoid minHalflengthX "
                              << volBounds->minHalflengthX()
                              << " maxHalflengthX() "<< volBounds->maxHalflengthX());

                Amg::Transform3D totalTransform =  transf * 
                                                   Amg::getRotateX3D(p180deg) *
                                                   Amg::getRotateZ3D(p90deg) * 
                                                   Amg::getRotateX3D(p90deg);
                vol = std::make_unique<Volume>(makeTransform(std::move(totalTransform)),
                                               volBounds.release());
            }
            return vol;
        } else {
            ATH_MSG_WARNING("PROBLEM: translating trapezoid: not recognized:"
                            << x1 << "," << x2 << "," << y1 << "," << y2 << ","<< z);
        }
    } else if (sh->type() == "Box") {
        const GeoBox* box = dynamic_cast<const GeoBox*>(sh);
        //
        double x = box->getXHalfLength();
        double y = box->getYHalfLength();
        double z = box->getZHalfLength();
        std::unique_ptr<CuboidVolumeBounds> volBounds = std::make_unique<CuboidVolumeBounds>(x, y, z);
        return std::make_unique<Volume>(makeTransform(transf), volBounds.release());
    } else if (sh->type() == "Para") {
        const GeoPara* para = dynamic_cast<const GeoPara*>(sh);
        //
        double x = para->getXHalfLength();
        double y = para->getYHalfLength();
        double z = para->getZHalfLength();
        auto volBounds = std::make_unique<CuboidVolumeBounds>(x, y, z);
        return std::make_unique<Volume>(makeTransform(transf), volBounds.release());
        //
    } else if (sh->type() == "Tube") {
        const GeoTube* tube = dynamic_cast<const GeoTube*>(sh);
        return std::make_unique<Volume>(makeTransform(transf), convert(tube).release());
    } else if (sh->type() == "Tubs") {  // non-trivial case - transform!
        const GeoTubs* tubs = dynamic_cast<const GeoTubs*>(sh);
        double rMin = tubs->getRMin();
        double rMax = tubs->getRMax();
        double z = tubs->getZHalfLength();
        double aPhi = tubs->getSPhi();
        double dPhi = tubs->getDPhi();
        auto volBounds = std::make_unique<CylinderVolumeBounds>(rMin, rMax, 0.5 * dPhi, z);
        Amg::Transform3D totalTransform(transf * Amg::getRotateZ3D(aPhi + 0.5 * dPhi));
        return std::make_unique<Volume>(makeTransform(std::move(totalTransform)), volBounds.release());
    } else if (sh->type() == "Cons") {
        const GeoCons* cons = dynamic_cast<const GeoCons*>(sh);
        double rMin1 = cons->getRMin1();
        double rMin2 = cons->getRMin2();
        double rMax1 = cons->getRMax1();
        double rMax2 = cons->getRMax2();
        double z = cons->getDZ();
        double aPhi = cons->getSPhi();
        double dPhi = cons->getDPhi();
        // translate into tube with average radius
        if (dPhi == 2 * M_PI) {
            auto volBounds =std::make_unique<CylinderVolumeBounds>(0.5 * (rMin1 + rMin2), 
                                                                   0.5 * (rMax1 + rMax2), z);
            return std::make_unique<Volume>(makeTransform(transf),
                                            volBounds.release());
        } else {
            auto volBounds =std::make_unique<CylinderVolumeBounds>(0.5 * (rMin1 + rMin2),
                                                                   0.5 * (rMax1 + rMax2),
                                                                   0.5 * dPhi, z);
            Amg::Transform3D totalTransform = transf * Amg::getRotateZ3D(aPhi + 0.5 * dPhi);
            return std::make_unique<Volume>(makeTransform(std::move(totalTransform)), 
                                            volBounds.release());
        }
    } else if (sh->type() == "Pcon") {
        const GeoPcon* con = dynamic_cast<const GeoPcon*>(sh);
        std::unique_ptr<CylinderVolumeBounds> volBounds{};
        double aPhi = con->getSPhi();
        double dPhi = con->getDPhi();
        double z1 = con->getZPlane(0);
        double r1 = con->getRMinPlane(0);
        double R1 = con->getRMaxPlane(0);
        std::vector<std::unique_ptr<Volume>> cyls;
        const unsigned int nPlanes = con->getNPlanes();
        ATH_MSG_DEBUG(" convert pcon aPhi " << aPhi << " dPhi " << dPhi << " z1 " << z1 << " r1 "
                      << r1 << " R1 " << R1 << " nPlanes " << nPlanes);
        for (unsigned int iv = 1; iv < nPlanes; iv++) {
            double z2 = con->getZPlane(iv);
            double r2 = con->getRMinPlane(iv);
            double R2 = con->getRMaxPlane(iv);
            double zshift = 0.5 * (z1 + z2);
            double hz = 0.5 * std::abs(z1 - z2);
            double rmin = std::max(r1, r2);
            double rmax = std::sqrt((R1 * R1 + R1 * R2 + R2 * R2 - r1 * r1 - r1 * r2 - r2 * r2) / 3 + rmin * rmin);
            ATH_MSG_DEBUG(" iPlane " << iv << " z2 " << z2 << " r2 " << r2 << " R2 " << R2 << " zshift " << zshift
                                     << " hz " << hz << " rmin " << rmin << " rmax " << rmax);
            double dz = con->getZPlane(iv) - con->getZPlane(iv - 1);
            double drMin = con->getRMinPlane(iv) - con->getRMinPlane(iv - 1);
            double drMax = con->getRMaxPlane(iv) - con->getRMaxPlane(iv - 1);
            int nSteps = 1;
            if (std::abs(dz) > 1 && (std::abs(drMin) > 0.1 * std::abs(dz) ||
                                     std::abs(drMax) > 0.1 * std::abs(dz))) {
                double dMax = std::abs(dz);
                dMax = std::max(std::abs(drMin), dMax);
                dMax = std::max(std::abs(drMax), dMax);
                nSteps = std::clamp(dMax / 50., 2., 20.);
                ATH_MSG_DEBUG(" Now "
                              << nSteps << " cylinders should be created " << dz
                              << " drMin " << drMin << " drMax " << drMax
                              << " splopeMin " << drMin / dz << " slopeMax "
                              << drMax / dz);
            }
            for (int j = 0; j < nSteps; j++) {
                /// divide volume into nStep cylinders
                double zStep = (0.5 + j) * dz / nSteps;
                if (nSteps > 1) {
                    hz = 0.5 * std::abs(z1 - z2) / nSteps;
                    zshift = z1 + zStep;
                    rmin = r1 + drMin * zStep / dz;
                    rmax = R1 + drMax * zStep / dz;
                }
                ATH_MSG_DEBUG(" cylinder " << j << " zshift " << zshift
                                           << " rmin " << rmin << " rmax "
                                           << rmax << " hz " << hz);
                // translate into tube sector
                if (dPhi == 2 * M_PI) {
                    volBounds = std::make_unique<CylinderVolumeBounds>(rmin, rmax, hz);
                    Amg::Transform3D totalTransform = transf * Amg::Translation3D{0., 0., zshift};
                    cyls.emplace_back(std::make_unique<Volume>(makeTransform(std::move(totalTransform)),
                                                               volBounds.release()));
                } else {
                    volBounds = std::make_unique<CylinderVolumeBounds>(rmin, rmax, 0.5 * dPhi, hz);
                    Amg::Transform3D totalTransform = transf * Amg::Translation3D{0., 0., zshift} *
                                                      Amg::getRotateZ3D(aPhi + 0.5 * dPhi);
                    cyls.emplace_back(std::make_unique<Volume>(makeTransform(std::move(totalTransform)),
                                                               volBounds.release()));
                }
            }  // end loop over steps
            z1 = z2;
            r1 = r2;
            R1 = R2;
        }

        if (cyls.size() < 2) {
            return std::move(cyls[0]);
        } else {
            auto comb =std::make_unique<CombinedVolumeBounds>(cyls[0].release(), cyls[1].release(), false);
            std::unique_ptr<Volume> combVol = std::make_unique<Volume>(nullptr, comb.release());
            for (unsigned int ic = 2; ic < cyls.size(); ++ic) {
                comb = std::make_unique<CombinedVolumeBounds>(combVol.release(), cyls[ic].release(), false);
                combVol = std::make_unique<Volume>(nullptr, comb.release());
            }
            return combVol;
        }
    } else if (sh->type() == "SimplePolygonBrep") {
        const GeoSimplePolygonBrep* spb = dynamic_cast<const GeoSimplePolygonBrep*>(sh);
        unsigned int nv = spb->getNVertices();
        std::vector<std::pair<double, double>> ivtx(nv);
        for (unsigned int iv = 0; iv < nv; iv++) {
            ivtx[iv] = std::make_pair(spb->getXVertex(iv), spb->getYVertex(iv));
            ATH_MSG_DEBUG(" SimplePolygonBrep  x "<< spb->getXVertex(iv) << " y " << spb->getYVertex(iv)
                          << " z " << spb->getDZ());
        }
        // translate into trapezoid or double trapezoid if possible
        if (nv == 4 || nv == 6) {
            std::vector<double> xstep;
            std::vector<std::pair<double, double>> ystep;
            bool trdlike = true;
            for (unsigned int iv = 0; iv < nv; ++iv) {
                if (!ystep.size() || spb->getYVertex(iv) > ystep.back().first)
                    ystep.emplace_back(spb->getYVertex(iv),
                                       std::abs(spb->getXVertex(iv)));
                else {
                    std::vector<std::pair<double, double>>::iterator iy = ystep.begin();
                    while (iy + 1 < ystep.end() &&
                           spb->getYVertex(iv) > (*iy).first + 1.e-3) {
                        ++iy;
                    }
                    if (spb->getYVertex(iv) < (*iy).first - 1.e-3) {
                        ystep.insert(iy, std::make_pair(spb->getYVertex(iv), std::abs(spb->getXVertex(iv))));
                    } else if (spb->getYVertex(iv) == (*iy).first &&
                               std::abs(spb->getXVertex(iv)) != (*iy).second) {
                        trdlike = false;
                    }
                }
            }

            if (trdlike) {
                std::unique_ptr<VolumeBounds> volBounds{};
                if (nv == 4) {
                    if (ystep[1].second >=  ystep[0].second) {  // expected ordering
                        volBounds = std::make_unique<TrapezoidVolumeBounds>(ystep[0].second, 
                                                                            ystep[1].second,
                                                                            0.5 * (ystep[1].first - ystep[0].first),
                                                                            spb->getDZ());
                        return std::make_unique<Volume>(makeTransform(transf),
                                                        volBounds.release());
                    }
                }

                if (nv == 6) {
                    if (ystep[1].second >= ystep[0].second &&
                        ystep[2].second >= ystep[1].second) {  // expected ordering
                        volBounds = std::make_unique<DoubleTrapezoidVolumeBounds>(ystep[0].second, 
                                                                                  ystep[1].second,
                                                                                  ystep[2].second,
                                                                                  0.5 * (ystep[1].first - ystep[0].first),
                                                                                  0.5 * (ystep[2].first - ystep[1].first),
                                                                                  spb->getDZ());
                        return std::make_unique<Volume>(makeTransform(transf * Amg::Translation3D(0., ystep[1].first, 0.)),
                                                        volBounds.release());
                    }
                }
            }  //  not trd-like
        }
        auto newBounds =  std::make_unique<SimplePolygonBrepVolumeBounds>(ivtx, spb->getDZ());
        return std::make_unique<Volume>(makeTransform(transf),
                                        newBounds.release());
    }

    else if (sh->type() == "Subtraction") {
        const GeoShapeSubtraction* sub = dynamic_cast<const GeoShapeSubtraction*>(sh);
        
        const GeoShape* shA = sub->getOpA();
        const GeoShape* shB = sub->getOpB();
        std::unique_ptr<Volume> volA = translateGeoShape(shA, transf);
        std::unique_ptr<Volume> volB = translateGeoShape(shB, transf);
        auto volBounds = std::make_unique<SubtractedVolumeBounds>(volA.release(),
                                                                  volB.release());
        return std::make_unique<Volume>(nullptr, volBounds.release());
    } else if (sh->type() == "Union") {
        const GeoShapeUnion* uni = dynamic_cast<const GeoShapeUnion*>(sh);
        const GeoShape* shA = uni->getOpA();
        const GeoShape* shB = uni->getOpB();
        std::unique_ptr<Volume> volA = translateGeoShape(shA, transf);
        std::unique_ptr<Volume> volB = translateGeoShape(shB, transf);
        auto volBounds = std::make_unique<CombinedVolumeBounds>(volA.release(),
                                                                volB.release(), false);
        return std::make_unique<Volume>(nullptr, volBounds.release());
    } else if (sh->type() == "Intersection") {
        const GeoShapeIntersection* intersect = dynamic_cast<const GeoShapeIntersection*>(sh);

        const GeoShape* shA = intersect->getOpA();
        const GeoShape* shB = intersect->getOpB();
        std::unique_ptr<Volume> volA{translateGeoShape(shA, transf)};
        std::unique_ptr<Volume> volB{translateGeoShape(shB, transf)};
        auto volBounds = std::make_unique<CombinedVolumeBounds>(volA.release(),
                                                                volB.release(), true);
        return std::make_unique<Volume>(nullptr, volBounds.release());
    }

    if (sh->type() == "Shift") {
        const GeoShapeShift* shift = dynamic_cast<const GeoShapeShift*>(sh);
        return translateGeoShape(shift->getOp(), transf * shift->getX());
    }
    ATH_MSG_WARNING("shape " << sh->type() << " not recognized, return 0");
    return nullptr;
}

void GeoShapeConverter::decodeShape(const GeoShape* sh) const {
    ATH_MSG_DEBUG("  ");
    ATH_MSG_DEBUG("decoding shape:" << sh->type());

    if (sh->type() == "Pgon") {
        const GeoPgon* pgon = dynamic_cast<const GeoPgon*>(sh);
        if (pgon)
            ATH_MSG_DEBUG("polygon: " << pgon->getNPlanes() << " planes "
                                      << pgon->getSPhi() << " "
                                      << pgon->getDPhi() << " "
                                      << pgon->getNSides());
        else
            ATH_MSG_DEBUG("polygon: WARNING: dynamic_cast failed!");
    }

    if (sh->type() == "Trd") {
        const GeoTrd* trd = dynamic_cast<const GeoTrd*>(sh);
        ATH_MSG_DEBUG("dimensions:" << trd->getXHalfLength1() << ","
                                    << trd->getXHalfLength2() << ","
                                    << trd->getYHalfLength1() << ","
                                    << trd->getYHalfLength2() << ","
                                    << trd->getZHalfLength());
    }
    if (sh->type() == "Box") {
        const GeoBox* box = dynamic_cast<const GeoBox*>(sh);
        ATH_MSG_DEBUG("dimensions:" << box->getXHalfLength() << ","
                                    << box->getYHalfLength() << ","
                                    << box->getZHalfLength());
    }

    if (sh->type() == "Tube") {
        const GeoTube* tube = dynamic_cast<const GeoTube*>(sh);
        ATH_MSG_DEBUG("dimensions:" << tube->getRMin() << "," << tube->getRMax()
                                    << "," << tube->getZHalfLength());
    }

    if (sh->type() == "Tubs") {
        const GeoTubs* tubs = dynamic_cast<const GeoTubs*>(sh);
        ATH_MSG_DEBUG("dimensions:" << tubs->getRMin() << "," << tubs->getRMax()
                                    << "," << tubs->getZHalfLength() << ","
                                    << tubs->getSPhi() << ","
                                    << tubs->getDPhi());
    }

    if (sh->type() == "Cons") {
        const GeoCons* cons = dynamic_cast<const GeoCons*>(sh);
        ATH_MSG_DEBUG("dimensions:"
                      << cons->getRMin1() << "," << cons->getRMin2() << ","
                      << cons->getRMax1() << "," << cons->getRMax2() << ","
                      << cons->getDZ() << "," << cons->getSPhi() << ","
                      << cons->getDPhi());
    }

    if (sh->type() == "Subtraction") {
        const GeoShapeSubtraction* sub =
            dynamic_cast<const GeoShapeSubtraction*>(sh);
        const GeoShape* sha = sub->getOpA();
        const GeoShape* shs = sub->getOpB();
        ATH_MSG_DEBUG("decoding subtracted shape:");
        decodeShape(sha);
        decodeShape(shs);
    }

    if (sh->type() == "Union") {
        const GeoShapeUnion* sub = dynamic_cast<const GeoShapeUnion*>(sh);
        const GeoShape* shA = sub->getOpA();
        const GeoShape* shB = sub->getOpB();
        ATH_MSG_DEBUG("decoding shape A:");
        decodeShape(shA);
        ATH_MSG_DEBUG("decoding shape B:");
        decodeShape(shB);
    }
    if (sh->type() == "Shift") {
        const GeoShapeShift* shift = dynamic_cast<const GeoShapeShift*>(sh);
        const GeoShape* shA = shift->getOp();
        const GeoTrf::Transform3D& transf = shift->getX();
        ATH_MSG_DEBUG("shifted by:transl:"
                      << transf.translation() << ", rot:" << transf(0, 0) << ","
                      << transf(1, 1) << "," << transf(2, 2));
        decodeShape(shA);
    }
}
}  // namespace Trk