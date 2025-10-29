/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Muon
#include "MuonTrackingGeometry/MuonStationTypeBuilder.h"
#include "MuonTrackingGeometry/Utils.h"
// MuonSpectrometer include
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
// Amg
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"

// Trk
#include <fstream>

#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoShape.h"
#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelKernel/GeoShapeSubtraction.h"
#include "GeoModelKernel/GeoShapeUnion.h"
#include "GeoModelKernel/GeoSimplePolygonBrep.h"
#include "GeoModelKernel/GeoTrd.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoTubs.h"
#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelUtilities/GeoVisitVolumes.h"
#include "TrkDetDescrGeoModelCnv/GeoMaterialConverter.h"
#include "TrkDetDescrInterfaces/ILayerArrayCreator.h"
#include "TrkDetDescrInterfaces/ILayerBuilder.h"
#include "TrkDetDescrUtils/BinUtility.h"
#include "TrkDetDescrUtils/BinnedArray.h"
#include "TrkDetDescrUtils/BinningType.h"
#include "TrkDetDescrUtils/GeometryStatics.h"
#include "TrkDetDescrUtils/NavBinnedArray1D.h"
#include "TrkGeometry/CylinderLayer.h"
#include "TrkGeometry/DetachedTrackingVolume.h"
#include "TrkGeometry/DiscLayer.h"
#include "TrkGeometry/HomogeneousLayerMaterial.h"
#include "TrkGeometry/LayerMaterialProperties.h"
#include "TrkGeometry/Material.h"
#include "TrkGeometry/MaterialProperties.h"
#include "TrkGeometry/OverlapDescriptor.h"
#include "TrkGeometry/PlaneLayer.h"
#include "TrkGeometry/SubtractedPlaneLayer.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkGeometrySurfaces/SubtractedPlaneSurface.h"
#include "TrkSurfaces/DiamondBounds.h"
#include "TrkSurfaces/DiscBounds.h"
#include "TrkSurfaces/RectangleBounds.h"
#include "TrkSurfaces/RotatedDiamondBounds.h"
#include "TrkSurfaces/RotatedTrapezoidBounds.h"
#include "TrkSurfaces/TrapezoidBounds.h"
#include "TrkVolumes/BoundarySurface.h"
#include "TrkVolumes/CombinedVolumeBounds.h"
#include "TrkVolumes/CuboidVolumeBounds.h"
#include "TrkVolumes/CylinderVolumeBounds.h"
#include "TrkVolumes/DoubleTrapezoidVolumeBounds.h"
#include "TrkVolumes/SimplePolygonBrepVolumeBounds.h"
#include "TrkVolumes/SubtractedVolumeBounds.h"
#include "TrkVolumes/TrapezoidVolumeBounds.h"
#include "TrkVolumes/VolumeExcluder.h"

// stl
#include <cmath>  //for std::abs
#include <map>
#include <memory>

constexpr double tolerance{0.001};

const InterfaceID& Muon::MuonStationTypeBuilder::interfaceID() {
    static const InterfaceID IID_IMuonStationTypeBuilder("MuonStationTypeBuilder", 1, 0);
    return IID_IMuonStationTypeBuilder;
}

// constructor
Muon::MuonStationTypeBuilder::MuonStationTypeBuilder(const std::string& t,
                                                     const std::string& n,
                                                     const IInterface* p)
    : AthAlgTool(t, n, p) {
    declareInterface<Muon::MuonStationTypeBuilder>(this);
}

// Athena standard methods
// initialize
StatusCode Muon::MuonStationTypeBuilder::initialize() {
    // Retrieve the tracking volume array creator
    // -------------------------------------------
    ATH_CHECK(m_trackingVolumeArrayCreator.retrieve());
    ATH_MSG_INFO("Retrieved tool " << m_trackingVolumeArrayCreator);

    // default (trivial) muon material properties
    m_muonMaterial = std::make_unique<Trk::Material>(10e10, 10e10, 0., 0., 0.);
    if (!m_muonMaterial) {
        ATH_MSG_FATAL("Could not create the material in " << name()
                                                          << " initialize()");
        return StatusCode::FAILURE;
    }

    ATH_MSG_INFO( " initialize() successful");

    return StatusCode::SUCCESS;
}

std::vector<std::unique_ptr<Trk::Layer>> Muon::MuonStationTypeBuilder::processBoxComponentsArbitrary(const GeoVPhysVol* mv,
                                                                                                     const Trk::CuboidVolumeBounds& envelope,
                                                                                                     Cache& /*cache*/) const {
    ATH_MSG_DEBUG( " processing arbitrary box station components for "<< mv->getLogVol()->getName());
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    std::vector<std::unique_ptr<Trk::Layer>> lays{};

    // initial solution : single layer collecting all material ; TODO : resolve
    // sensitive layers and spacers use envelope to define layer bounds
    auto layBounds = std::make_shared<Trk::RectangleBounds>(envelope.halflengthY(), envelope.halflengthZ());
    // calculate layer area
    double layArea= area(*layBounds);
    // use area to blend station material
    Trk::MaterialProperties box_mat;
    m_volumeConverter.collectMaterial(mv, box_mat, layArea);
    Trk::HomogeneousLayerMaterial boxMaterial(box_mat, 0.);

    auto layer = std::make_unique<Trk::PlaneLayer>(Amg::Transform3D::Identity(), layBounds,
                                                   boxMaterial, 2 * envelope.halflengthX());

    lays.push_back(std::move(layer));

    return lays;
}

std::unique_ptr<Trk::TrackingVolumeArray>
        Muon::MuonStationTypeBuilder::processBoxStationComponents(const GeoVPhysVol* mv,
                                                                  const Trk::CuboidVolumeBounds& envelope,
                                                                  Cache& cache) const {
    ATH_MSG_DEBUG( " processing box station components for "
                         << mv->getLogVol()->getName());
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    // loop over children volumes: check if compatible with binning in X (
    // detect overlap of sensitive volumes )
    std::vector<std::pair<double, double>> xVol;
    double xpos{0}, xh{0};
    for (const auto& [cv, transf] : geoGetVolumes(mv)) {
        const GeoLogVol* clv = cv->getLogVol();
        // consider sensitive volumes only
        const GeoFullPhysVol* cfv = dynamic_cast<const GeoFullPhysVol*> (cv);
        std::string name = clv->getName();
        if (name.find("MDT") == std::string::npos && name.find("RPC") == std::string::npos && !cfv){
            continue;
        }
        xpos = transf.translation().x();
        if (clv->getShape()->type() == "Trd") {
            const GeoTrd* trd = static_cast<const GeoTrd*>(clv->getShape());
            xh = std::max(trd->getXHalfLength1(), trd->getXHalfLength2());
        } else if (clv->getShape()->type() == "Box") {
            const GeoBox* box = static_cast<const GeoBox*>(clv->getShape());
            xh = box->getXHalfLength();
        } else {
            xh = get_x_size(cv);
        }
        if (xVol.empty() || xpos > xVol.back().first)
            xVol.emplace_back(xpos, xh);
        else {
            std::vector<std::pair<double, double>>::iterator it = xVol.begin();
            while (it != xVol.end() && xpos > (*it).first) {
                ++it;
            }
            xVol.insert(it, std::make_pair(xpos, xh));
        }
    }

    double xl = xVol[0].second;
    double xc = xVol[0].first;
    for (const auto& xb : xVol) {
        if (xb.first > xc && xb.first - xb.second < xc + xl) {
            ATH_MSG_DEBUG("Inconsistent sensitive overlap");
            return nullptr;  //  overlap of sensitive volumes : not suitable for
                             //  x-binned array
        }
        xc = xb.first;
        xl = xb.second;
    }

    // loop over children volumes; ( make sure they do not exceed enveloping
    // volume boundaries ?) split into connected subvolumes ( assume ordering
    // along X unless otherwise )
    std::vector<std::unique_ptr<Trk::Volume>> compVol;
    std::vector<std::string> compName;
    std::vector<const GeoVPhysVol*> compGeo;
    std::vector<Amg::Transform3D> compTransf;
    for (const auto& [cv, transf] : geoGetVolumes(mv)) {
        const GeoLogVol* clv = cv->getLogVol();
        std::shared_ptr<Trk::VolumeBounds> volBounds{};
        std::unique_ptr<Trk::Volume> vol{};
        if (clv->getShape()->type() == "Trd") {
            const GeoTrd* trd = static_cast<const GeoTrd*>(clv->getShape());
            const double halfX1{trd->getXHalfLength1()}, halfX2{trd->getXHalfLength2()},
                         halfY1{trd->getYHalfLength1()}, halfY2{trd->getYHalfLength2()},
                         halfZ{trd->getZHalfLength()};
            volBounds = std::make_shared<Trk::CuboidVolumeBounds>(std::max(halfX1, halfX2), std::max(halfY1, halfY2), halfZ);
        } else if (clv->getShape()->type() == "Box") {
            const GeoBox* box = static_cast<const GeoBox*>(clv->getShape());
            volBounds = Trk::GeoShapeConverter::convert(box);
        } else {
            double xSize = get_x_size(cv);
            ATH_MSG_VERBOSE("subvolume not box nor trapezoid, estimated x size:" << xSize);
            volBounds = std::make_shared<Trk::CuboidVolumeBounds>(xSize, envelope.halflengthY(), envelope.halflengthZ());
        }
        vol = std::make_unique<Trk::Volume>(makeTransform(transf), std::move(volBounds));
        ATH_MSG_VERBOSE("subvolume center:" << Amg::toString(vol->center()));
        std::string cname = clv->getName();
        const std::string& vname = mv->getLogVol()->getName();
        int nameSize = vname.size() - 8;
        if (cname.compare(0, nameSize, vname, 0, nameSize) == 0)
            cname = cname.substr(nameSize, cname.size() - nameSize);
        // order in X
        if (compVol.empty() || vol->center().x() >= compVol.back()->center().x()) {
            compVol.push_back(std::move(vol));
            compName.push_back(cname);
            compGeo.push_back(cv);
            compTransf.push_back(transf);
        } else {
            std::vector<std::unique_ptr<Trk::Volume>>::iterator volIter = compVol.begin();
            std::vector<std::string>::iterator nameIter = compName.begin();
            std::vector<const GeoVPhysVol*>::iterator geoIter = compGeo.begin();
            std::vector<Amg::Transform3D>::iterator transfIter =
                compTransf.begin();
            while (vol->center().x() >= (*volIter)->center().x()) {
                ++volIter;
                ++nameIter;
                ++geoIter;
                ++transfIter;
            }
            compVol.insert(volIter, std::move(vol));
            compName.insert(nameIter, cname);
            compGeo.insert(geoIter, cv);
            compTransf.insert(transfIter, transf);
        }
    }  // loop over components

    // define enveloping volumes for each "technology"
    std::vector<std::shared_ptr<Trk::TrackingVolume>> trkVols{};
    double envX = envelope.halflengthX();
    double envY = envelope.halflengthY();
    double envZ = envelope.halflengthZ();
    double currX = -envX;
    double maxX = envX;
    bool openSpacer{false}, openRpc{false};
    std::vector<const GeoVPhysVol*> geoSpacer{}, geoRpc{};
    std::vector<Amg::Transform3D> transfSpacer{}, transfRpc{};
    double spacerlowXsize{0.}, spaceruppXsize{0.}, rpclowXsize{0.}, rpcuppXsize{0.};
    std::vector<float> volSteps;
    volSteps.push_back(-envX);
    for (unsigned i = 0; i < compVol.size(); ++i) {
        bool comp_processed = false;
        const Trk::VolumeBounds& volBounds = compVol[i]->volumeBounds();
        const Trk::CuboidVolumeBounds* compBounds = dynamic_cast<const Trk::CuboidVolumeBounds*>(&volBounds);
        // check return to comply with coverity
        if (!compBounds) {
            ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" box station component does not return cuboid shape "
                            <<typeid(volBounds).name());
            continue;
        }
        //
        double lowX = compVol[i]->center().x() - compBounds->halflengthX();
        double uppX = compVol[i]->center().x() + compBounds->halflengthX();

        /// BIS78 volumes
        if (lowX < currX - tolerance && (compName[i].compare("RPC28") != 0 && compName[i].compare("RPC29") !=0)) {
            ATH_MSG_WARNING(" clash between components in volume:" << compName[i] << "current:" << currX
                          << ": low edge of next volume:" << lowX);
        }
        if (uppX > maxX + tolerance) {
            ATH_MSG_WARNING(" clash between component and envelope:" << compName[i] << "upper:" << uppX << ">" << maxX);
        }
        // close Rpc if no further component
        if (openRpc && compName[i].find("RPC") == std::string::npos && compName[i].find("Rpc") == std::string::npos && compName[i].compare(0, 3, "Ded") != 0) {
            // low edge of current volume
            double Xcurr = compVol[i]->center().x() - compBounds->halflengthX();
            if (Xcurr + tolerance >= currX + rpclowXsize + rpcuppXsize) {
                auto rpcBounds = std::make_unique<Trk::CuboidVolumeBounds>(0.5 * (Xcurr - currX), envY, envZ);
                Amg::Transform3D rpcTrf{Amg::getTranslateX3D(currX + rpcBounds->halflengthX())};
                auto rpcVol = std::make_unique<Trk::Volume>(makeTransform(rpcTrf),
                                                            std::move(rpcBounds));
                std::unique_ptr<Trk::TrackingVolume> rpcTrkVol = processRpc(*rpcVol, geoRpc, transfRpc, cache);
                trkVols.push_back(std::move(rpcTrkVol));
                volSteps.push_back(Xcurr);
                currX = Xcurr;
                openRpc = false;
            } else {
                ATH_MSG_WARNING("clash in Rpc definition!");
            }
        }
        // close spacer if no further components
        if (openSpacer && compName[i].compare(0, 1, "C") != 0 && compName[i].compare(0, 2, "LB") != 0 && compName[i].find("Spacer")==std::string::npos) {
            // low edge of current volume
            double Xcurr = compVol[i]->center().x() - compBounds->halflengthX();
            if (Xcurr - currX - (spacerlowXsize + spaceruppXsize) >= -tolerance) {
                auto spacerBounds = std::make_shared<Trk::CuboidVolumeBounds>(0.5 * (Xcurr - currX), envY, envZ);
                Amg::Transform3D spacerTrf{Amg::getTranslateX3D(currX + spacerBounds->halflengthX())};
                Trk::Volume spacerVol(makeTransform(spacerTrf), std::move(spacerBounds));
                std::unique_ptr<Trk::TrackingVolume> spacerTrkVol{processSpacer(spacerVol, geoSpacer, transfSpacer)};
                trkVols.emplace_back(std::move(spacerTrkVol));
                volSteps.push_back(Xcurr);
                currX = Xcurr;
                openSpacer = false;
            } else {
                ATH_MSG_WARNING("clash in spacer definition!");
            }
        }
        if (compName[i].find("RPC")!=std::string::npos || compName[i].starts_with("Ded") || compName[i].find("Rpc")!=std::string::npos) {
            if (!openRpc) {
                openRpc = true;
                geoRpc.clear();
                geoRpc.push_back(compGeo[i]);
                transfRpc.clear();
                transfRpc.push_back(compTransf[i]);
                // establish temporary volume size
                rpclowXsize = compVol[i]->center().x() - currX;
                rpcuppXsize = compBounds->halflengthX();
                // check clash at low edge
                if (std::abs(rpclowXsize) < compBounds->halflengthX() - tolerance) {
                    ATH_MSG_WARNING("rpc low edge - not enough space");
                }
            } else {
                geoRpc.push_back(compGeo[i]);
                transfRpc.push_back(compTransf[i]);
                // check temporary volume size
                if (std::abs(compVol[i]->center().x() - currX) < compBounds->halflengthX() - tolerance) {
                    ATH_MSG_WARNING("rpc low edge - not enough space");
                }
                if (compVol[i]->center().x() + compBounds->halflengthX() > currX + rpclowXsize + rpcuppXsize) {
                    rpcuppXsize += (compVol[i]->center().x() + compBounds->halflengthX()) -
                                   (currX + rpclowXsize + rpcuppXsize);
                }
            }
            comp_processed = true;
        }
        if (compName[i].starts_with("C") || compName[i].starts_with("LB") || compName[i].find("Spacer")!=std::string::npos) {
            if (!openSpacer) {
                openSpacer = true;
                geoSpacer.clear();
                geoSpacer.push_back(compGeo[i]);
                transfSpacer.clear();
                transfSpacer.push_back(compTransf[i]);
                // establish temporary volume size
                spacerlowXsize = compVol[i]->center().x() - currX;
                spaceruppXsize = compBounds->halflengthX();
                // check clash at low edge
                if (std::abs(spacerlowXsize) <
                    compBounds->halflengthX() - tolerance) {
                    ATH_MSG_WARNING("spacer low edge - not enough space:current:center:halfSize:"
                                << currX << "," << compVol[i]->center().x() << "," << compBounds->halflengthX());
                }
            } else {
                geoSpacer.push_back(compGeo[i]);
                transfSpacer.push_back(compTransf[i]);
                // check temporary volume size
                if (std::abs(compVol[i]->center().x() - currX) < compBounds->halflengthX() - tolerance) {
                    ATH_MSG_WARNING("spacer low edge - not enough space:current:center:halfSize:"
                        << currX << "," << compVol[i]->center().x() << "," << compBounds->halflengthX());
                }
                if (compVol[i]->center().x() + compBounds->halflengthX() > currX + spacerlowXsize + spaceruppXsize) {
                    spaceruppXsize += (compVol[i]->center().x() +  compBounds->halflengthX()) -
                                      (currX + spacerlowXsize + spaceruppXsize);
                }
            }
            comp_processed = true;
        }
        if (compName[i].compare(0, 3, "MDT") == 0) {
            std::unique_ptr<Trk::Volume> mdtVol;
            // remove z shift in transform !! bugfix !!
            double zShift = compVol[i]->transform().translation().z();
            if (std::abs(zShift) > tolerance) {
                ATH_MSG_DEBUG("unusual z shift for subvolume:" << zShift);
            }
            double boundHalfLengthX{0.};
            if (lowX == currX) {
                auto mdtBounds = std::make_shared<Trk::CuboidVolumeBounds>(compBounds->halflengthX(), envY, envZ);
                boundHalfLengthX = mdtBounds->halflengthX();
                mdtVol = std::make_unique<Trk::Volume>(makeTransform(Amg::getTranslateZ3D(-zShift) *compVol[i]->transform()),
                                                       std::move(mdtBounds));
            } else {
                if (std::abs(lowX - currX) > 0.002) {
                    ATH_MSG_DEBUG("Mdt volume size does not match the envelope:lowX,currX:"<< lowX << "," << currX);
                    ATH_MSG_DEBUG("adjusting Mdt volume ");
                }
                auto mdtBounds = std::make_shared<Trk::CuboidVolumeBounds>(compBounds->halflengthX() + 0.5 * (lowX - currX),
                                                                           envY, envZ);
                boundHalfLengthX = mdtBounds->halflengthX();
                mdtVol = std::make_unique<Trk::Volume>(makeTransform(Amg::getTranslate3D(0.5 * (currX - lowX), 0., -zShift) *
                                                                     compVol[i]->transform()),
                                                        std::move(mdtBounds));
            }
            double shiftSign = 1.;
            if (std::abs(zShift) > 0.) {
                const std::string& stName = mv->getLogVol()->getName();
                if (stName.compare(0, 4, "BIR3") == 0 || stName.compare(0, 4, "BIR5") == 0 ||
                    stName.compare(0, 4, "BIR7") == 0 || stName.compare(0, 5, "BIR10") == 0) {
                    shiftSign = -1.;
                }
            }

            std::unique_ptr<Trk::TrackingVolume> mdtTrkVol{processMdt(*mdtVol, compGeo[i],
                                                                         Amg::getTranslateZ3D(-zShift) * compTransf[i],
                                                                         shiftSign * std::abs(zShift), cache)};
            trkVols.push_back(std::move(mdtTrkVol));
            currX += 2. * boundHalfLengthX;
            volSteps.push_back(currX);
            comp_processed = true;
            zShift = 0.;
        }
        if (!comp_processed) {
            ATH_MSG_WARNING("unknown technology:" << compName[i]);
        }
    }  // end loop over station children

    // there may be a spacer still open
    if (openSpacer) {
        if (maxX - tolerance >= currX + spacerlowXsize + spaceruppXsize) {
            auto spacerBounds = std::make_shared<Trk::CuboidVolumeBounds>(0.5 * (maxX - currX), envY, envZ);
            Amg::Transform3D spacerTrf{Amg::getTranslateX3D(currX + spacerBounds->halflengthX())};
            Trk::Volume spacerVol(makeTransform(spacerTrf),
                                  std::move(spacerBounds));
            std::unique_ptr<Trk::TrackingVolume> spacerTrkVol{processSpacer(spacerVol, geoSpacer, transfSpacer)};
            trkVols.emplace_back(std::move(spacerTrkVol));
            currX = maxX;
            volSteps.push_back(currX);
            openSpacer = false;
        }
    }
    // there may be an Rpc still open
    if (openRpc) {
        if (maxX + tolerance >= currX + rpclowXsize + rpcuppXsize) {
            auto rpcBounds = std::make_shared<Trk::CuboidVolumeBounds>(0.5 * (maxX - currX), envY, envZ);
            Amg::Transform3D rpcTrf{Amg::getTranslateX3D(currX + rpcBounds->halflengthX())};
            auto rpcVol = std::make_unique<Trk::Volume>(makeTransform(rpcTrf),
                                                        std::move(rpcBounds));
            std::unique_ptr<Trk::TrackingVolume> rpcTrkVol{processRpc(*rpcVol, geoRpc, transfRpc, cache)};
            trkVols.push_back(std::move(rpcTrkVol));
            currX = maxX;
            volSteps.push_back(currX);
            openRpc = false;
        } else {
            ATH_MSG_WARNING("clash in Rpc definition!(last volume)");
        }
    }
    // create VolumeArray (1DX)
    Trk::BinUtility binUtility(volSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
    std::unique_ptr<Trk::TrackingVolumeArray> components(m_trackingVolumeArrayCreator->cuboidVolumesArrayNav(trkVols, binUtility));

    return components;
}

std::unique_ptr<Trk::TrackingVolumeArray>
    Muon::MuonStationTypeBuilder::processTrdStationComponents(const GeoVPhysVol* mv,
                                                              const Trk::TrapezoidVolumeBounds& envelope,
                                                              Cache& cache) const {
    ATH_MSG_DEBUG( " processing station components for " << mv->getLogVol()->getName());
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    // loop over children volumes; ( make sure they do not exceed enveloping
    // volume boundaries ?) split into connected subvolumes ( assume ordering
    // along X unless otherwise )
    std::vector<std::unique_ptr<Trk::Volume>> compVol;
    std::vector<std::string> compName;
    std::vector<const GeoVPhysVol*> compGeo;
    std::vector<Amg::Transform3D> compTransf;
    for (const auto& [cv, transf]: geoGetVolumes(mv)) {
        const GeoLogVol* clv = cv->getLogVol();
               // retrieve volumes for components
        std::shared_ptr<Trk::VolumeBounds> bounds{};
        Amg::Transform3D boxTrf{transf};
        if (clv->getShape()->type() == "Trd") {
            const GeoTrd* trd = dynamic_cast<const GeoTrd*>(clv->getShape());
            const double halfX1 = trd->getXHalfLength1();
            const double halfX2 = trd->getXHalfLength2();
            const double halfY1 = trd->getYHalfLength1();
            const double halfY2 = trd->getYHalfLength2();
            const double halfZ = trd->getZHalfLength();
            if (halfX1 == halfX2 && halfY1 == halfY2)
                bounds = std::make_shared<Trk::CuboidVolumeBounds>(std::max(halfX1, halfX2),
                                                                   std::max(halfY1, halfY2), halfZ);
            if (halfX1 == halfX2 && halfY1 != halfY2) {
                boxTrf = boxTrf * Amg::getRotateY3D(M_PI_2) *
                                  Amg::getRotateZ3D(M_PI_2);
                bounds = std::make_shared<Trk::TrapezoidVolumeBounds>(halfY1, halfY2,
                                                                      halfZ, halfX1);
            }
            if (halfX1 != halfX2 && halfY1 == halfY2) {
                bounds = std::make_shared<Trk::TrapezoidVolumeBounds>(halfX1, halfX2,
                                                                      halfY1, halfZ);
            }
            if (!bounds) {
                ATH_MSG_WARNING("volume shape for component not recognized");
            }
        } else if (clv->getShape()->type() == "Box") {
            const GeoBox* box = dynamic_cast<const GeoBox*>(clv->getShape());
            const double halfX1 = box->getXHalfLength();
            const double halfY1 = box->getYHalfLength();
            const double halfZ = box->getZHalfLength();
            bounds = std::make_shared<Trk::CuboidVolumeBounds>(halfX1, halfY1, halfZ);
        } else {
            double xSize = get_x_size(cv);
            // printChildren(cv);
            if (clv->getName().compare(0, 1, "C") != 0 && clv->getName().compare(0, 2, "LB") != 0 && clv->getName().find("Spacer")==std::string::npos) {
                 boxTrf = boxTrf * Amg::getRotateY3D(M_PI_2) *
                                   Amg::getRotateZ3D(M_PI_2);
            }
            bounds = std::make_shared<Trk::TrapezoidVolumeBounds>(envelope.minHalflengthX(),
                                                                  envelope.maxHalflengthX(),
                                                                  envelope.halflengthY(), xSize);
        }
        auto vol = std::make_unique<Trk::Volume>(makeTransform(boxTrf),
                                                 std::move(bounds));
        std::string cname = clv->getName();
        std::string vname = mv->getLogVol()->getName();
        int nameSize = vname.size() - 8;
        if (cname.compare(0, nameSize, vname, 0, nameSize) == 0) {
            cname = cname.substr(nameSize, cname.size() - nameSize);
        }
        // order in X
        if (compVol.empty() || vol->center().x() >= compVol.back()->center().x()) {
            compVol.emplace_back(std::move(vol));
            compName.push_back(cname);
            compGeo.push_back(cv);
            compTransf.push_back(transf);
        } else {
            std::vector<std::unique_ptr<Trk::Volume>>::iterator volIter = compVol.begin();
            std::vector<std::string>::iterator nameIter = compName.begin();
            std::vector<const GeoVPhysVol*>::iterator geoIter = compGeo.begin();
            std::vector<Amg::Transform3D>::iterator transfIter = compTransf.begin();
            while (vol->center().x() >= (*volIter)->center().x()) {
                ++volIter;
                ++nameIter;
                ++geoIter;
                ++transfIter;
            }
            compVol.insert(volIter, std::move(vol));
            compName.insert(nameIter, cname);
            compGeo.insert(geoIter, cv);
            compTransf.insert(transfIter, transf);
        }
    }  // loop over components

    // define enveloping volumes for each "technology"
    std::vector<std::shared_ptr<Trk::TrackingVolume>> trkVols;
    const double envX1{envelope.minHalflengthX()}, envX2{envelope.maxHalflengthX()},
                 envY{envelope.halflengthY()}, envZ{envelope.halflengthZ()};
    //
    double currX{-envZ}, maxX{envZ};
    //
    bool openSpacer = false;
    std::vector<const GeoVPhysVol*> geoSpacer{}, geoRpc{};
    std::vector<Amg::Transform3D> transfSpacer{}, transfRpc{};
    double spacerlowXsize{0.}, spaceruppXsize{0.}, Xcurr{0.}, lowX{0.}, uppX{0.};
    std::vector<float> volSteps;
    volSteps.push_back(-envelope.halflengthZ());
    for (unsigned i = 0; i < compVol.size(); i++) {
        bool comp_processed = false;
        const Trk::VolumeBounds& volBounds = compVol[i]->volumeBounds();
        const Trk::CuboidVolumeBounds* compCubBounds = dynamic_cast<const Trk::CuboidVolumeBounds*>(&volBounds);
        const Trk::TrapezoidVolumeBounds* compTrdBounds = dynamic_cast<const Trk::TrapezoidVolumeBounds*>(&volBounds);
        if (compCubBounds) {
            lowX = compVol[i]->center().x() - compCubBounds->halflengthX();
            uppX = compVol[i]->center().x() + compCubBounds->halflengthX();
        } else if (compTrdBounds) {
            lowX = compVol[i]->center().x() - compTrdBounds->halflengthZ();
            uppX = compVol[i]->center().x() + compTrdBounds->halflengthZ();
        }
         if (lowX + tolerance < currX) {
            ATH_MSG_WARNING("Warning: we have a clash between components here!" << lowX <<":" << currX);
        }
        if (uppX - tolerance > maxX) {
            ATH_MSG_WARNING("we have a clash between component and envelope!" << uppX <<":" << maxX);
        }
        // low edge of current volume
        Xcurr = lowX;
        if (!compCubBounds && !compTrdBounds) {
            ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" Unknown volume shape "<<typeid(volBounds).name());
            return nullptr;
        }
        // close spacer if no further components
        if (openSpacer && compName[i].compare(0, 1, "C") != 0 && compName[i].compare(0, 2, "LB") != 0) {
            if (Xcurr - currX - (spacerlowXsize + spaceruppXsize) >=-tolerance) {
                auto spacerBounds = std::make_shared<Trk::TrapezoidVolumeBounds>(envX1, envX2, envY,
                                                                                 0.5 * (Xcurr - currX));
                Amg::Transform3D tr = Amg::getTranslateX3D(currX + spacerBounds->halflengthZ()) *
                                      Amg::getRotateY3D(M_PI_2) * Amg::getRotateZ3D(M_PI_2);
                Trk::Volume spacerVol(makeTransform(tr), std::move(spacerBounds));
                std::unique_ptr<Trk::TrackingVolume> spacerTrkVol{processSpacer(spacerVol, geoSpacer, transfSpacer)};
                trkVols.push_back(std::move(spacerTrkVol));
                currX = Xcurr;
                volSteps.push_back(Xcurr);
                openSpacer = false;
            } else {
                ATH_MSG_DEBUG(mv->getLogVol()->getName()<< " : clash in spacer definition ");
            }
        }
        if (compName[i].compare(0, 3, "RPC") == 0 || compName[i].compare(0, 3, "Ded") == 0) {
            ATH_MSG_DEBUG(mv->getLogVol()->getName() << ": RPC components in endcaps? ");
        }
        if (compName[i].compare(0, 1, "C") == 0 || compName[i].compare(0, 2, "LB") == 0) {
            if (!openSpacer) {
                openSpacer = true;
                geoSpacer.clear();
                geoSpacer.push_back(compGeo[i]);
                transfSpacer.clear();
                transfSpacer.push_back(compTransf[i]);
                // establish temporary volume size
                spacerlowXsize = compVol[i]->center().x() - currX;
                if (compCubBounds) {
                    spaceruppXsize = compCubBounds->halflengthX();
                    // check clash at low edge
                    if (spacerlowXsize < compCubBounds->halflengthX()){
                        ATH_MSG_DEBUG( mv->getLogVol()->getName() << ",  spacer low edge - not enough space");
                    }
                }
                if (compTrdBounds) {
                    spaceruppXsize = compTrdBounds->halflengthZ();
                    // check clash at low edge
                    if (spacerlowXsize < compTrdBounds->halflengthZ()) {
                        ATH_MSG_DEBUG( mv->getLogVol()->getName() << ",  spacer low edge - not enough space");
                    }
                }
            } else {
                geoSpacer.push_back(compGeo[i]);
                transfSpacer.push_back(compTransf[i]);
                // check temporary volume size
                if (compCubBounds) {
                    if (compVol[i]->center().x() - currX < compCubBounds->halflengthX()) {
                        ATH_MSG_DEBUG( mv->getLogVol()->getName() << ",  spacer low edge - not enough space");
                    }
                    if (compVol[i]->center().x() + compCubBounds->halflengthX() > currX + spacerlowXsize + spaceruppXsize) {
                        spaceruppXsize += (compVol[i]->center().x() + compCubBounds->halflengthX()) -
                                          (currX + spacerlowXsize + spaceruppXsize);
                    }
                } else if (compTrdBounds) {
                    if (compVol[i]->center().x() - currX < compTrdBounds->halflengthZ()) {
                        ATH_MSG_DEBUG(mv->getLogVol()->getName() << ",  spacer low edge - not enough space");
                    }
                    if (compVol[i]->center().x() + compTrdBounds->halflengthZ() > currX + spacerlowXsize + spaceruppXsize) {
                        spaceruppXsize += (compVol[i]->center().x() + compTrdBounds->halflengthZ()) -
                                          (currX + spacerlowXsize + spaceruppXsize);
                    }
                }
            }
            comp_processed = true;
        }
        if (compName[i].compare(0, 3, "MDT") == 0) {
            std::unique_ptr<Trk::Volume> mdtVol{};
            const double dZ =  compTrdBounds? compTrdBounds->halflengthZ() : compCubBounds->halflengthX();
            if (std::abs(lowX - currX) > 0.002) {
                ATH_MSG_DEBUG( "Mdt volume size does not match the envelope:lowX,currX:"<< lowX << "," << currX);
                ATH_MSG_DEBUG("adjusting Mdt volume ");
            }
            auto mdtBounds = std::make_shared<Trk::TrapezoidVolumeBounds>(envX1, envX2, envY, dZ + 0.5 * (lowX - currX));
            const double halfZ = mdtBounds->halflengthZ();
            mdtVol = std::make_unique<Trk::Volume>(makeTransform(Amg::getTranslateZ3D(0.5 * (currX - lowX)) *compVol[i]->transform()),
                                                   std::move(mdtBounds));
            std::unique_ptr<Trk::TrackingVolume> mdtTrkVol{processMdt(*mdtVol, compGeo[i], compTransf[i], 0., cache)};
            trkVols.push_back(std::move(mdtTrkVol));
            currX += 2. * halfZ;
            volSteps.push_back(currX);
            comp_processed = true;
        }
        if (!comp_processed)
            ATH_MSG_DEBUG(mv->getLogVol()->getName()<< ", unknown technology:" << compName[i]);
    }  // end loop over station children

    // there may be a spacer still open
    if (openSpacer) {
        if (maxX >= currX + spacerlowXsize + spaceruppXsize) {
            auto spacerBounds = std::make_shared<Trk::TrapezoidVolumeBounds>(envX1, envX2, envY,
                                                                             0.5 * (maxX - currX));

            Amg::Transform3D spacerTrf = Amg::getRotateY3D(M_PI_2) * Amg::getRotateZ3D(M_PI_2) *
                                         Amg::getTranslateZ3D(currX + spacerBounds->halflengthZ());
            Trk::Volume spacerVol(makeTransform(spacerTrf), std::move(spacerBounds));

            std::unique_ptr<Trk::TrackingVolume> spacerTrkVol{processSpacer(spacerVol, geoSpacer, transfSpacer)};
            trkVols.push_back(std::move(spacerTrkVol));
            currX = maxX;
            volSteps.push_back(currX);
            openSpacer = false;
        } else {
            ATH_MSG_DEBUG(mv->getLogVol()->getName() << ",  clash in spacer definition (last volume)");
        }
    }
    // create VolumeArray (1DX)
    Trk::BinUtility binUtility(volSteps, Trk::BinningOption::open,Trk::BinningValue::binX);
    std::unique_ptr<Trk::TrackingVolumeArray> components{m_trackingVolumeArrayCreator->trapezoidVolumesArrayNav(trkVols,
                                                                                                                binUtility)};

    return components;
}

// finalize
StatusCode Muon::MuonStationTypeBuilder::finalize() {
    ATH_MSG_INFO( " finalize() successful");
    return StatusCode::SUCCESS;
}
//
std::unique_ptr<Trk::TrackingVolume>
    Muon::MuonStationTypeBuilder::processMdt(const Trk::Volume& vol, 
                                                const GeoVPhysVol* gv, 
                                                const Amg::Transform3D& transf,
                                                double zShift, Cache& cache) const {
std::vector<std::unique_ptr<Trk::PlaneLayer>> layers{};
// loop over material content
std::vector<Muon::MaterialInput> mvols;
getMaterialInputs(gv,GeoTrf::Transform3D::Identity(),mvols);
// envelope defines layer surface boundaries
const Trk::CuboidVolumeBounds* box = dynamic_cast<const Trk::CuboidVolumeBounds*>(&(vol.volumeBounds()));
const Trk::TrapezoidVolumeBounds* trd = dynamic_cast<const Trk::TrapezoidVolumeBounds*>(&(vol.volumeBounds()));
std::shared_ptr<Trk::SurfaceBounds> bounds = nullptr;
double area = 0; double hX = 0;
if (trd) { // x-y plane -> y-z plane
  bounds = std::make_shared<Trk::TrapezoidBounds>(trd->minHalflengthX(), trd->maxHalflengthX(), trd->halflengthY() );
  area = 2 * (trd->minHalflengthX() + trd->maxHalflengthX()) * (trd->halflengthY());
  hX = trd->halflengthZ();
 } else if (box) {
  bounds = std::make_shared<Trk::RectangleBounds>(box->halflengthY(), box->halflengthZ());   
  area = 4 * (box->halflengthY()) * (box->halflengthZ());
  hX = box->halflengthX();
} else {
  ATH_MSG_WARNING(name() << ": MDT subvolume of unknown shape, bailing out ");
  return nullptr;
}
// layers
std::unique_ptr<Trk::OverlapDescriptor> od = nullptr;
for (const auto & mi : mvols) {
  ATH_MSG_VERBOSE("MDT material components overview:" << mi.gmVol->getLogVol()->getName()<<":" << mi.nCopies 
                 <<":" << mi.refX.translation().x() <<":made of:" << mi.gmVol->getLogVol()->getMaterial()->getName() 
                 <<":pitch:" << mi.pitch<<":thickness:" << decodeX(mi.gmVol->getLogVol()->getShape()));

  const GeoLogVol* lv = mi.gmVol->getLogVol();
  double thickness = decodeX(lv->getShape());
  int active{0};
  Trk::MaterialProperties* mdtMat = nullptr;
  if ((lv->getName()).starts_with("MDT")) {
    if (!cache.m_mdtTubeMat) {
      const GeoTube* tube = dynamic_cast<const GeoTube*>(lv->getShape());
      double volume = 4 * (tube->getZHalfLength()) * thickness * mi.pitch;
      cache.m_mdtTubeMat = std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(mi.gmVol, volume, 2 * thickness));
    }
    mdtMat = cache.m_mdtTubeMat.get();
    active = 1;
  }
  if ((lv->getName()) == "MultiLayerFoam" || (lv->getName()) == "foamLog") {
    for (auto& i : cache.m_mdtFoamMat) {
      if (std::abs(thickness - 0.5 * i->thickness()) < tolerance) { mdtMat = i.get(); break;}
    }
    if (!mdtMat) {
      cache.m_mdtFoamMat.push_back(std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(mi.gmVol, area * 2 * thickness, 2 * thickness)));
      mdtMat = cache.m_mdtFoamMat.back().get();
    }
  }
  if (!mdtMat) ATH_MSG_WARNING("Undefined MDT layer material");

  Trk::MaterialProperties matLay = mdtMat ? *mdtMat : Trk::MaterialProperties(*m_muonMaterial, thickness);
  Trk::HomogeneousLayerMaterial mdtMaterial(matLay, 0.);
  Amg::Transform3D cTr = transf * Amg::getTranslateZ3D(mi.refX.translation().x());
  auto layer = std::make_unique<Trk::PlaneLayer>(cTr, bounds, mdtMaterial, thickness, std::move(od));
  layer->setLayerType(active);  // make preliminary identification of active layers
  layer->setRef(mi.refX.translation().z() - zShift);
  layers.push_back(std::move(layer));
}

// create the BinnedArray
std::vector<std::shared_ptr<Trk::Layer>> layerOrder;
std::vector<float> binSteps;
//
double minX = transf.translation().x() - hX;
binSteps.push_back(minX);
if (!layers.empty()) {
  double currX = minX;
  for (unsigned int i = 0; i < layers.size(); ++i) {
    const Amg::Transform3D ltransf = layers[i]->transform();
    if (i < layers.size() - 1) {
      currX = ltransf.translation().x() + 0.5 * layers[i]->thickness();
      binSteps.push_back(currX);
    }
    layerOrder.emplace_back(std::move(layers[i]));
  }
}
binSteps.push_back(transf.translation().x() + hX);

Trk::BinUtility binUtility(binSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
auto mdtLayerArray = std::make_unique<Trk::NavBinnedArray1D<Trk::Layer>>(layerOrder, binUtility, Amg::Transform3D(Amg::Transform3D::Identity()));

return std::make_unique<Trk::TrackingVolume>(vol, *m_muonMaterial, std::move(mdtLayerArray), nullptr, "MDT");
}

void Muon::MuonStationTypeBuilder::getMaterialInputs(const GeoVPhysVol* gv, GeoTrf::Transform3D trfRef, std::vector<Muon::MaterialInput>& mVols) const {
   if (!Trk::GeoMaterialConverter::dummy_material(gv->getLogVol()->getMaterial())) {
      const std::string & name = gv->getLogVol()->getName();
      if ( !mVols.empty() && name == mVols.back().gmVol->getLogVol()->getName() ) {
        if (std::abs(trfRef.translation().x() - mVols.back().refX.translation().x())>tolerance ) {
          Muon::MaterialInput mi; mi.gmVol = gv; mi.nCopies++;  mi.refX = trfRef; mVols.push_back(mi);
        } else {
          if (mVols.back().nCopies<2) mVols.back().pitch = (trfRef.translation()-mVols.back().refX.translation()).norm();
          mVols.back().nCopies++;
        }
      } else { Muon::MaterialInput mi; mi.gmVol = gv; mi.nCopies++;  mi.refX = trfRef; mVols.push_back(mi); }
      return;
   }
   for ( const auto& [cv, transfc]: geoGetVolumes(gv)) getMaterialInputs(cv, trfRef*transfc, mVols);
}


std::unique_ptr<Trk::TrackingVolume> Muon::MuonStationTypeBuilder::processRpc(const Trk::Volume& vol,
                                                                              const std::vector<const GeoVPhysVol*>& gv,
                                                                              const std::vector<Amg::Transform3D>& transfc, Cache& cache) const {
    // layers correspond to DedModules and RpcModules; all substructures
    // averaged in material properties
    std::vector<std::unique_ptr<Trk::Layer>> layers{};
    for (unsigned int ic = 0; ic < gv.size(); ++ic) {
        const GeoLogVol* glv = gv[ic]->getLogVol();
        const GeoShape* shape = glv->getShape();
        if (shape->type() != "Box" && shape->type() != "Trd") {
            const GeoShapeSubtraction* sub = dynamic_cast<const GeoShapeSubtraction*>(shape);
            const GeoShape* subt = nullptr;
            while (sub) {
                subt = sub->getOpA();
                sub = dynamic_cast<const GeoShapeSubtraction*>(subt);
            }
            shape = subt;
        }
        if (shape && shape->type() == "Box") {
            const GeoBox* box = static_cast<const GeoBox*>(shape);
            double xs = box->getXHalfLength();
            double ys = box->getYHalfLength();
            double zs = box->getZHalfLength();
            // translating into layer; x dimension defines thickness
            double thickness = 2 * xs;
            std::unique_ptr<Trk::OverlapDescriptor> od = nullptr;
            auto bounds = std::make_shared<Trk::RectangleBounds>(ys, zs);
            Amg::Transform3D cTr = transfc[ic] * Amg::getRotateY3D(M_PI_2) *Amg::getRotateZ3D(M_PI_2);
            Trk::MaterialProperties rpcMat(0., 10.e10, 10.e10, 13., 26., 0.);
            if (glv->getName().compare(0, 3, "Ded") == 0) {
                // find if material exists already
                bool found = false;
                for (auto& i : cache.m_rpcDed) {
                    if (std::abs(thickness - i->thickness()) < 0.001) {
                        rpcMat = Trk::MaterialProperties(*i);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    double volc = 8 * xs * ys * zs;
                    cache.m_rpcDed.push_back(std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(gv[ic], volc, 2 * xs)));
                    rpcMat = Trk::MaterialProperties(*cache.m_rpcDed.back());
                }
            } else {
                 /// Why not 42?? That number would have much more beauty
                if (std::abs(thickness - 46.0) < 0.001) {
                    if (!cache.m_rpc46) {
                        double volc = 8 * xs * ys * zs;
                        cache.m_rpc46 = std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(gv[ic], volc, 2 * xs));
                    }
                    rpcMat = Trk::MaterialProperties(*cache.m_rpc46);
                } else {   
                    ATH_MSG_WARNING( "RPC module thickness different from 46: "<< thickness);
                }
            }

            Trk::HomogeneousLayerMaterial rpcMaterial(rpcMat, 0.);
            auto layer = std::make_unique<Trk::PlaneLayer>(cTr, bounds, rpcMaterial, thickness, std::move(od));

            // make preliminary identification of active layers
            if (!(glv->getName()).starts_with("Ded")) {
                layer->setLayerType(1);
            } else {
                layer->setLayerType(0);
            }
            layers.push_back(std::move(layer));
        } else if (shape && shape->type() == "Trd") {
            const GeoTrd* trd = static_cast<const GeoTrd*>(shape);
            double xs1 = trd->getXHalfLength1();
            double xs2 = trd->getXHalfLength2();
            double ys1 = trd->getYHalfLength1();
            double ys2 = trd->getYHalfLength2();
            double zs = trd->getZHalfLength();
            // translating into layer; x dimension defines thickness
            if (xs1 == xs2 && ys1 == ys2) {
                double thickness = 2 * xs1;
                std::unique_ptr<Trk::OverlapDescriptor> od = nullptr;
                auto bounds =  std::make_shared<Trk::RectangleBounds>(ys1, zs);
                Amg::Transform3D cTr = transfc[ic] * Amg::getRotateY3D(M_PI_2) * Amg::getRotateZ3D(M_PI_2);
                Trk::MaterialProperties rpcMat(0., 10.e10, 10.e10, 13., 26., 0.);  // default
                if ((glv->getName()).starts_with("Ded")) {
                    // find if material exists already
                    bool found = false;
                    for (auto& i : cache.m_rpcDed) {
                        if (std::abs(thickness - i->thickness()) < 0.001) {
                            rpcMat = Trk::MaterialProperties(*i);
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        double volc = 8 * xs1 * ys1 * zs;
                        cache.m_rpcDed.push_back(std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(gv[ic], volc, 2 * xs1)));
                        rpcMat = Trk::MaterialProperties(*cache.m_rpcDed.back());
                    }
                    // create Ded layer
                    Trk::HomogeneousLayerMaterial rpcMaterial(rpcMat, 0.);
                    auto layer = std::make_unique<Trk::PlaneLayer>(cTr, bounds, rpcMaterial, thickness, std::move(od));
                    layer->setLayerType(0);
                    layers.push_back(std::move(layer));
                } else {
                    // RPC layer; step one level below to resolve strip planes
                    unsigned int ngc = gv[ic]->getNChildVols();
                    for (unsigned int igc = 0; igc < ngc; igc++) {
                        Amg::Transform3D trgc(Amg::Transform3D::Identity());
                        if (transfc[ic].rotation().isIdentity())
                            trgc = gv[ic]->getXToChildVol(igc);
                        else
                            trgc = Amg::getRotateZ3D(M_PI) * gv[ic]->getXToChildVol(igc);

                        const GeoVPhysVol* gcv = gv[ic]->getChildVol(igc);
                        const GeoLogVol* gclv = gcv->getLogVol();
                        const GeoShape* lshape = gclv->getShape();
                        while (lshape->type() == "Subtraction") {
                            const GeoShapeSubtraction* sub = dynamic_cast<const GeoShapeSubtraction*>(lshape);
                            lshape = sub->getOpA();
                        }
                        const GeoTrd* gtrd = dynamic_cast<const GeoTrd*>(lshape);
                        double gx = gtrd->getXHalfLength1();
                        double gy = gtrd->getYHalfLength1();
                        double gz = gtrd->getZHalfLength();

                        if ((gclv->getName()).compare(0, 6, "RPC_AL") == 0) {
                            if (std::abs(gx - 5.0) < 0.001) {
                                if (!cache.m_rpcExtPanel) {
                                    double volc = 8 * gx * gy * gz;
                                    cache.m_rpcExtPanel =
                                        std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(gcv, volc, 2 * gx));
                                }
                                rpcMat = Trk::MaterialProperties(*cache.m_rpcExtPanel);
                            } else if (std::abs(gx - 4.3) < 0.001) {
                                if (!cache.m_rpcMidPanel) {
                                    double volc = 8 * gx * gy * gz;
                                    cache.m_rpcMidPanel = 
                                            std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(gcv, volc, 2 * gx));
                                }
                                rpcMat = Trk::MaterialProperties(*cache.m_rpcMidPanel);
                            } else {
                                ATH_MSG_WARNING("unknown RPC panel:" << gx);
                            }
                            // create Rpc panel layers
                            thickness = 2 * gx;
                            Trk::HomogeneousLayerMaterial rpcMaterial(rpcMat, 0.);
                            auto layer = std::make_unique<Trk::PlaneLayer>(Amg::getTranslate3D(trgc.translation()) *cTr,
                                                                         bounds, rpcMaterial, thickness, std::move(od));
                            layer->setLayerType(0);
                            layers.push_back(std::move(layer));
                        } else if ((gclv->getName()) == "Rpclayer") {
                            // two thicknesses allowed for 2/3 gaps RPCs
                            if (std::abs(gx - 6.85) > 0.001 && std::abs(gx - 5.9) >  0.001)  {      
                                ATH_MSG_WARNING("processRpc() - unusual thickness of RPC ("<< glv->getName()<< ") layer :" << 2 * gx);
                            }
                            if (!cache.m_rpcLayer) {
                                double volc = 8 * gx * gy * gz;
                                cache.m_rpcLayer =
                                    std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(gcv, volc,  2 * gx));
                            }
                            rpcMat = Trk::MaterialProperties(*cache.m_rpcLayer);
                            // define 1 layer for 2 strip planes
                            thickness = 2 * gx;
                            Trk::HomogeneousLayerMaterial rpcMaterial(rpcMat, 0.);
                            auto layer = std::make_unique<Trk::PlaneLayer>(Amg::getTranslate3D(trgc.translation()) *cTr,
                                                                           bounds, rpcMaterial, thickness, std::move(od));
                            layer->setLayerType(1);
                            layers.push_back(std::move(layer));
                        } else {
                            ATH_MSG_WARNING( "unknown RPC component? " << gclv->getName());
                        }
                    }
                }
            } else {
                ATH_MSG_WARNING("RPC true trapezoid layer, not coded yet");
            }
        } else {
            ATH_MSG_WARNING( "RPC layer shape not recognized");
        }
    }  // end loop over Modules

    ATH_MSG_DEBUG(" Rpc component volume processed with" << layers.size()<< " layers");
    auto rpcLayers = std::make_unique<std::vector<Trk::Layer*>>(Muon::release(layers));
    return std::make_unique<Trk::TrackingVolume>(vol, *m_muonMaterial, std::move(rpcLayers), "RPC");
}
//

std::unique_ptr<Trk::TrackingVolume> Muon::MuonStationTypeBuilder::processSpacer(const Trk::Volume& vol,
                                                                                 std::vector<const GeoVPhysVol*> gv,
                                                                                 std::vector<Amg::Transform3D> transf) const {
    // spacers: one level below, assumed boxes
    std::vector<std::unique_ptr<Trk::Layer>> layers{};
    // resolve child volumes
    // Don't use iterators; they'll be invalidated by the push_back's.
    size_t idx{0};
    while (idx < gv.size()) {
        const GeoVPhysVol* vol = gv[idx];
        const Amg::Transform3D& tf = transf[idx];
        if (vol->getNChildVols()) {
            for (unsigned int ich = 0; ich < vol->getNChildVols(); ++ich) {
                gv.push_back(vol->getChildVol(ich));
                transf.emplace_back(tf * vol->getXToChildVol(ich));
            }
            gv.erase(gv.begin() + idx);
            transf.erase(transf.begin() + idx);
        } else {
            ++idx;
        }
    }
    // translate into layers
    for (unsigned int ic = 0; ic < gv.size(); ++ic) {
        const GeoLogVol* clv = gv[ic]->getLogVol();
        Trk::Material cmat = Trk::GeoMaterialConverter::convert(clv->getMaterial());
        ATH_MSG_VERBOSE(" spacer material all X0 "
                        << cmat.X0 << " L0 " << cmat.L0 << " A " << cmat.A
                        << " Z " << cmat.Z << " rho " << cmat.rho);
        if (clv->getShape()->type() == "Box") {
            const GeoBox* box = static_cast<const GeoBox*>(clv->getShape());
            double xs = box->getXHalfLength();
            double ys = box->getYHalfLength();
            double zs = box->getZHalfLength();
            // translating into layer; find minimal size
            std::shared_ptr<Trk::SurfaceBounds> bounds = nullptr;
            double thickness{0.};
            Amg::Transform3D cTr{Amg::Transform3D::Identity()};
            if (zs <= xs && zs <= ys) {  // x-y plane
                bounds = std::make_shared<Trk::RectangleBounds>(xs, ys);
                thickness = 2 * zs;
                cTr = transf[ic];
            } else if (xs <= ys && xs <= zs) {  // x-y plane -> y-z plane
                bounds = std::make_shared<Trk::RectangleBounds>(ys, zs);
                thickness = 2 * xs;
                cTr =  transf[ic] * Amg::getRotateY3D(M_PI_2) * Amg::getRotateZ3D(M_PI_2);
            } else {  // x-y plane -> x-z plane
                bounds = std::make_shared<Trk::RectangleBounds>(xs, zs);
                thickness = 2 * ys;
                cTr = transf[ic] * Amg::getRotateX3D(M_PI_2);
            }
            Trk::MaterialProperties material(thickness, cmat.X0, cmat.L0, cmat.A, cmat.Z, cmat.rho);
            Trk::HomogeneousLayerMaterial spacerMaterial(material, 0.);

            auto layer = std::make_unique<Trk::PlaneLayer>(cTr, bounds, spacerMaterial, thickness, nullptr, 0);
            layers.push_back(std::move(layer));
        } else if (clv->getShape()->type() == "Subtraction") {
            const GeoShapeSubtraction* sub =
                static_cast<const GeoShapeSubtraction*>(clv->getShape());
            if (sub->getOpA()->type() == "Box" && sub->getOpB()->type() == "Box") {
                // LB
                const GeoBox* boxA = static_cast<const GeoBox*>(sub->getOpA());
                const GeoBox* boxB = static_cast<const GeoBox*>(sub->getOpB());
                auto bounds = std::make_shared<Trk::RectangleBounds>(boxA->getYHalfLength(), boxA->getZHalfLength());
                double thickness = (boxA->getXHalfLength() - boxB->getXHalfLength());
                double shift = 0.5 * (boxA->getXHalfLength() + boxB->getXHalfLength());
                Trk::MaterialProperties material(0., 10.e10, 10.e10, 13., 26., 0.);
                Trk::HomogeneousLayerMaterial spacerMaterial;
                if (thickness > 0.) {
                    material = Trk::MaterialProperties( thickness, cmat.X0, cmat.L0, cmat.A, cmat.Z, cmat.rho);
                    spacerMaterial = Trk::HomogeneousLayerMaterial(material, 0.);
                    
                    auto  layx = std::make_unique<Trk::PlaneLayer>(transf[ic]  * Amg::getTranslateX3D(shift) *
                                                                   Amg::getRotateY3D(M_PI_2) * Amg::getRotateZ3D(M_PI_2),
                                                                    bounds, spacerMaterial, thickness, nullptr, 0);
                    layers.push_back(std::move(layx));
                    auto layxx =  std::make_unique<Trk::PlaneLayer>(transf[ic] * Amg::getTranslateX3D(-shift) *
                                                                    Amg::getRotateY3D(M_PI_2) * Amg::getRotateZ3D(M_PI_2),
                                                                    bounds, spacerMaterial, thickness, nullptr, 0);
                    layers.push_back(std::move(layxx));
                }
                thickness = (boxA->getYHalfLength() - boxB->getYHalfLength());
                if (thickness > 0.) {
                    material = Trk::MaterialProperties( thickness, cmat.X0, cmat.L0, cmat.A, cmat.Z, cmat.rho);
                    spacerMaterial = Trk::HomogeneousLayerMaterial(material, 0.);
                    shift = 0.5 * (boxA->getYHalfLength() + boxB->getYHalfLength());
                    bounds = std::make_shared<Trk::RectangleBounds>(boxB->getXHalfLength(), boxA->getZHalfLength());
                    auto lay = std::make_unique<Trk::PlaneLayer>(transf[ic] * Amg::getTranslateY3D(shift) * Amg::getRotateX3D(M_PI_2),
                                                                 bounds, spacerMaterial, thickness, nullptr, 0);
                    layers.push_back(std::move(lay));

                    auto layy = std::make_unique<Trk::PlaneLayer>(transf[ic] * Amg::getTranslateY3D(-shift) * Amg::getRotateX3D(M_PI_2),
                                                                 bounds, spacerMaterial, thickness, nullptr, 0);

                    layers.push_back(std::move(layy));
                }
                thickness = (boxA->getZHalfLength() - boxB->getZHalfLength());
                if (thickness > 0.) {
                    material = Trk::MaterialProperties(thickness, cmat.X0, cmat.L0, cmat.A, cmat.Z, cmat.rho);
                    spacerMaterial =Trk::HomogeneousLayerMaterial(material, 0.);
                    shift = 0.5 * (boxA->getZHalfLength() + boxB->getZHalfLength());
                    bounds = std::make_shared<Trk::RectangleBounds>(boxB->getXHalfLength(), boxB->getYHalfLength());
                    auto layz = std::make_unique<Trk::PlaneLayer>(transf[ic] * Amg::getTranslateZ3D(shift),
                                                                  bounds, spacerMaterial, thickness, nullptr, 0);
                    layers.push_back(std::move(layz));
    
                    auto layzz = std::make_unique<Trk::PlaneLayer>(transf[ic] * Amg::getTranslateZ3D(-shift),
                                                                  bounds, spacerMaterial, thickness, nullptr, 0);
                    layers.push_back(std::move(layzz));
                }
            } else {
                std::vector<std::pair<const GeoShape*, Amg::Transform3D>> subVs;
                const GeoShapeShift* shift = dynamic_cast<const GeoShapeShift*>(sub->getOpB());
                if (shift)
                    subVs.emplace_back(shift->getOp(), shift->getX());
                const GeoShape* shape = sub->getOpA();
                while (shape->type() == "Subtraction") {
                    const GeoShapeSubtraction* subtr = static_cast<const GeoShapeSubtraction*>(shape);
                    const GeoShapeShift* shift = dynamic_cast<const GeoShapeShift*>(subtr->getOpB());
                    if (shift)
                        subVs.emplace_back(shift->getOp(), shift->getX());
                    shape = subtr->getOpA();
                }
                const GeoBox* box = dynamic_cast<const GeoBox*>(shape);
                if (box && subVs.size() == 4) {
                    std::unique_ptr<Trk::Volume> v1{}, v2{};
                    std::shared_ptr<const Trk::VolumeExcluder> volExcl = nullptr;
                    const GeoBox* sb1 = dynamic_cast<const GeoBox*>(subVs[0].first);
                    if (sb1) {
                        v1 = std::make_unique<Trk::Volume>(makeTransform(subVs[0].second),
                                                            Trk::GeoShapeConverter::convert(sb1));
                    }
                    const GeoBox* sb2 = dynamic_cast<const GeoBox*>(subVs[1].first);
                    if (sb2) {
                        v2 = std::make_unique<Trk::Volume>(makeTransform(subVs[1].second),
                                                            Trk::GeoShapeConverter::convert(sb2));
                    }
                    const GeoBox* boxB = dynamic_cast<const GeoBox*>(subVs[2].first);
                    if (boxB && v1 && v2) {
                        auto bounds = std::make_shared<Trk::RectangleBounds>(box->getYHalfLength(), box->getZHalfLength());
                        double thickness = (box->getXHalfLength() - boxB->getXHalfLength());
                        double shift{0.5 * (box->getXHalfLength() + boxB->getXHalfLength())};
                        auto combinedBounds = std::make_shared<Trk::CombinedVolumeBounds>(std::move(v1), std::move(v2), false);
                        auto cVol = std::make_unique<Trk::Volume>(makeTransform(Amg::getTranslateX3D(-shift)),
                                                                 std::move(combinedBounds));
                        volExcl = std::make_shared<const Trk::VolumeExcluder>(std::unique_ptr<Trk::Volume>(cVol->clone()));
                        Trk::PlaneSurface surf{transf[ic] * Amg::getTranslateX3D(shift) * Amg::getRotateY3D(M_PI_2) * Amg::getRotateZ3D(M_PI_2), std::move(bounds)};
                        auto subPlane = std::make_unique<Trk::SubtractedPlaneSurface>(std::move(surf), std::move(volExcl), false);
                        auto subPlaneX = std::make_unique<Trk::SubtractedPlaneSurface>(*subPlane, Amg::getTranslateX3D(-2 * shift));

                        Trk::MaterialProperties material(thickness, cmat.X0, cmat.L0, cmat.A, cmat.Z, cmat.rho);
                        Trk::HomogeneousLayerMaterial spacerMaterial(material, 0.);
                        
                        auto layx = std::make_unique<Trk::SubtractedPlaneLayer>(subPlane.get(), spacerMaterial, thickness, nullptr, 0);
                        layers.push_back(std::move(layx));
                       
                        auto layxx = std::make_unique<Trk::SubtractedPlaneLayer>(subPlaneX.get(), spacerMaterial, thickness, nullptr, 0);
                        layers.push_back(std::move(layxx));

                        bounds = std::make_shared<Trk::RectangleBounds>( boxB->getXHalfLength(), box->getZHalfLength());
                        thickness = subVs[2].second.translation().mag();


                        auto volEx = std::make_shared<const Trk::VolumeExcluder>(std::move(std::make_unique<Trk::Volume>(*cVol, Amg::getTranslateX3D(2 * shift))));
                        
                        surf = Trk::PlaneSurface{transf[ic] * Amg::getRotateX3D(M_PI_2), std::move(bounds)};
                        auto subPlaneBis = std::make_unique<Trk::SubtractedPlaneSurface>(std::move(surf), std::move(volEx), false);
                        material = Trk::MaterialProperties(thickness, cmat.X0, cmat.L0, cmat.A, cmat.Z, cmat.rho);
                        spacerMaterial = Trk::HomogeneousLayerMaterial(material, 0.);
                        auto lay = std::make_unique<Trk::SubtractedPlaneLayer>(subPlaneBis.get(), spacerMaterial, thickness, nullptr, 0);
                        layers.push_back(std::move(lay));
                    }
                }
            }
        } else {
            ATH_MSG_DEBUG(clv->getName() << ", unresolved spacer component " << clv->getName());
        }
    }

    std::vector<std::unique_ptr<Trk::Layer>>::iterator lIt = layers.begin();
    for (; lIt != layers.end(); ++lIt)
        if ((*lIt)->thickness() < 0.)
            lIt = layers.erase(lIt);

    auto  spacerLayers = std::make_unique<std::vector<Trk::Layer*>>(Muon::release(layers));
    auto spacer = std::make_unique<Trk::TrackingVolume>(vol, *m_muonMaterial, std::move(spacerLayers), "Spacer");

    if (!m_resolveSpacer) {  // average into a single material layer
        ATH_MSG_VERBOSE(" !m_resolveSpacer createLayerRepresentation ");
        auto laySpacer = createLayerRepresentation(*spacer);
        laySpacer.first->setLayerType(0);
        layers.clear();
        layers.push_back(std::move(laySpacer.first));
        auto spacerLays = std::make_unique<std::vector<Trk::Layer*>>(Muon::release(layers));
        spacer = std::make_unique<Trk::TrackingVolume>(vol, *m_muonMaterial, std::move(spacerLays), "Spacer");
    }

    return spacer;
}

std::unique_ptr<Trk::TrackingVolume> Muon::MuonStationTypeBuilder::processCscStation(const GeoVPhysVol* mv,
                                                                                     const std::string& name,
                                                                                     Cache& cache) const {
    // CSC stations have the particularity of displacement in Z between
    // multilayer and the spacer - the envelope
    //   has to be derived from the component volume shape and component
    //   displacement
    bool isDiamond = false;
    double xMin{0.}, xMed{0.}, xMax{0}, y1{0.}, y2{0}, z{0.};
    // find the shape and dimensions for the first component
    const GeoVPhysVol* cv = &(*(mv->getChildVol(0)));
    const GeoLogVol* clv = cv->getLogVol();
    // Amg::Transform3D transform =
    // Amg::CLHEPTransformToEigen(mv->getXToChildVol(0));
    if (clv->getShape()->type() == "Shift") {
        const GeoShapeShift* shift =
            static_cast<const GeoShapeShift*>(clv->getShape());
        if (shift->getOp()->type() == "Union") {
            // that would be the union making the diamond/double trapezoid
            // shape, let's retrieve the parameters
            isDiamond = true;
            const GeoShapeUnion* uni = static_cast<const GeoShapeUnion*>(shift->getOp());
            if (uni->getOpA()->type() == "Trd") {
                const GeoTrd* trdA = static_cast<const GeoTrd*>(uni->getOpA());
                xMin = trdA->getYHalfLength1();
                xMed = trdA->getYHalfLength2();
                y1 = trdA->getZHalfLength();
                z = trdA->getXHalfLength1();
            }
            if (uni->getOpB()->type() == "Shift") {
                const GeoShapeShift* sh = static_cast<const GeoShapeShift*>(uni->getOpB());
                const GeoTrd* trdB = dynamic_cast<const GeoTrd*>(sh->getOp());
                if (trdB->getYHalfLength1() != xMed || trdB->getXHalfLength1() != z) {
                    ATH_MSG_DEBUG(mv->getLogVol()->getName() <<
                                  ": something is wrong: dimensions of 2 trapezoids do not match");
                }
                xMax = trdB->getYHalfLength2();
                y2 = trdB->getZHalfLength();
            }
        }  // end Union
        if (shift->getOp()->type() == "Trd") {
            // that would be the trapezoid shape, let's retrieve the parameters
            const GeoTrd* trd = static_cast<const GeoTrd*>(shift->getOp());
            xMin = trd->getYHalfLength1();
            xMed = trd->getYHalfLength2();
            y1 = trd->getZHalfLength();
            z = trd->getXHalfLength1();
        }  // end Trd
    } else {
        if (clv->getShape()->type() == "Trd") {
            // that would be the trapezoid shape, let's retrieve the parameters
            const GeoTrd* trd = static_cast<const GeoTrd*>(clv->getShape());
            xMin = trd->getYHalfLength1();
            xMed = trd->getYHalfLength2();
            y1 = trd->getZHalfLength();
            z = trd->getXHalfLength1();
        }
    }
    // then loop over all components to get total Xsize & transforms
    std::vector<Amg::Transform3D> compTransf;
    std::vector<std::string> compName;
    std::vector<const GeoVPhysVol*> compGeoVol;
    std::vector<double> xSizes;
    double xmn = +10000.;
    double xmx = -10000.;
    for (const auto& [cv,  transform]: geoGetVolumes(mv)) {
        const GeoLogVol* clv = cv->getLogVol();
        unsigned int ich = compTransf.size();
        compTransf.push_back(transform);
        compName.push_back(clv->getName());
        compGeoVol.push_back(cv);
        if (clv->getShape()->type() == "Shift") {
            const GeoShapeShift* shift = static_cast<const GeoShapeShift*>(clv->getShape());
            if (shift->getOp()->type() == "Union") {
                // that would be the union making the diamond/double trapezoid
                // shape, let's retrieve the parameters
                const GeoShapeUnion* uni = static_cast<const GeoShapeUnion*>(shift->getOp());
                if (uni->getOpA()->type() == "Trd") {
                    const GeoTrd* trdA = static_cast<const GeoTrd*>(uni->getOpA());
                    double xSize = trdA->getXHalfLength1();
                    if (!xSizes.empty()){
                        xSizes.push_back((std::abs(transform.translation().x() - compTransf[ich - 1].translation().x()) - xSizes.back()));
                    } else {
                        xSizes.push_back(xSize);
                    }
                    double xpos = (transform * shift->getX()).translation().x();
                    xmn = std::min(xmn, xpos - xSizes.back());
                    xmx = std::max(xmx, xpos + xSizes.back());
                }
            }  // end Union
        }      // end Shift
        if (clv->getShape()->type() == "Trd") {
            const GeoTrd* trd = static_cast<const GeoTrd*>(clv->getShape());
            double xSize = trd->getXHalfLength1();
            if (!xSizes.empty()) {
                xSizes.push_back( std::abs(transform.translation().x() -  compTransf[ich - 1].translation().x()) - xSizes.back());
            } else {
                xSizes.push_back(xSize);
            }
            double xpos = transform.translation().x();
            xmn = std::min(xmn, xpos - xSizes.back());
            xmx = std::max(xmx, xpos + xSizes.back());
        }  // end Trd
    }
    // this should be enough to build station envelope
    double xTotal{0.};
    for (double xSize : xSizes)
        xTotal += xSize;
    double xShift{0.5 * (xmx + xmn)}, zShift{0.};
    zShift = std::abs(compTransf.front().translation().z()) + std::abs(compTransf.back().translation().z());
    // calculate displacement with respect to GeoModel station volume
    // one way or the other, the station envelope is double trapezoid
    std::unique_ptr<Trk::Volume> envelope;
    double envXMed = xMed;
    double envY1 = y1;
    double envY2 = y2;
    std::vector<float> volSteps;
    volSteps.push_back(-xTotal + xShift);
    std::vector<std::shared_ptr<Trk::TrackingVolume>> components{};
    if (!isDiamond) {
        xMax = xMed;
        y2 = 0.5 * zShift;
        auto cscBounds = std::make_shared<Trk::TrapezoidVolumeBounds>(xMin, xMax, y1, xTotal);
        // xy -> yz  rotation
        // the center of Volume is shifted by y1-y2 in y
        Amg::Transform3D cTr = Amg::getRotateY3D(M_PI_2) *  Amg::getRotateZ3D(M_PI_2) * Amg::getTranslateZ3D(xShift);
        envelope = std::make_unique<Trk::Volume>(makeTransform(cTr), std::move(cscBounds));
        // components
        double xCurr = -xTotal;
        for (unsigned int ic = 0; ic < xSizes.size(); ic++) {
            // component volumes follow the envelope dimension
            xCurr += xSizes[ic];
            Amg::Transform3D compTr = Amg::getRotateY3D(M_PI_2) * Amg::getRotateZ3D(M_PI_2) * Amg::getTranslateZ3D(xCurr + xShift);
            auto compBounds = std::make_shared<Trk::TrapezoidVolumeBounds>(xMin, xMax, y1, xSizes[ic]);
            std::unique_ptr<Trk::LayerArray> cscLayerArray = processCSCTrdComponent(compGeoVol[ic], *compBounds, compTr, cache);
            std::unique_ptr<Trk::Volume> compVol = std::make_unique<Trk::Volume>(makeTransform(compTr), std::move(compBounds));
            auto compTV =  std::make_shared<Trk::TrackingVolume>(*compVol, *m_muonMaterial, std::move(cscLayerArray), nullptr, compName[ic]); 
            components.push_back(std::move(compTV));
            xCurr += xSizes[ic];
            volSteps.push_back(xCurr + xShift);
        }  // end components
    } else {
        if (xMed != xMin && xMed != xMax) {
            envXMed += zShift / (y1 / (xMed - xMin) + y2 / (xMed - xMax));
            envY1 = y1 * (envXMed - xMin) / (xMed - xMin);
            envY2 = y2 * (envXMed - xMax) / (xMed - xMax);
        }
        auto cscBounds = std::make_shared<Trk::DoubleTrapezoidVolumeBounds>(xMin, envXMed, xMax,  envY1, envY2, xTotal);
        // xy -> yz  rotation
        // the center of DoubleTrapezoidVolume is shifted by (envY1-envY2) in y
        Amg::Transform3D cTr = Amg::getRotateY3D(M_PI_2) * Amg::getRotateZ3D(M_PI_2) * Amg::getTranslate3D(0., envY1 - envY2, xShift);
        envelope = std::make_unique<Trk::Volume>(makeTransform(cTr), std::move(cscBounds));
        // components
        double xCurr = -xTotal;
        for (unsigned int ic = 0; ic < xSizes.size(); ic++) {
            // component volumes follow the envelope dimension
            xCurr += xSizes[ic];
            Amg::Transform3D compTr = Amg::getRotateY3D(M_PI_2) * Amg::getRotateZ3D(M_PI_2) *
                                      Amg::getTranslate3D(0., envY1 - envY2, xCurr + xShift);
            auto compBounds = std::make_shared<Trk::DoubleTrapezoidVolumeBounds>(xMin, envXMed, xMax, envY1, envY2, xSizes[ic]);
            std::unique_ptr<Trk::LayerArray> cscLayerArray{processCSCDiamondComponent(compGeoVol[ic], *compBounds, compTr, cache)};
            std::unique_ptr<Trk::Volume> compVol = std::make_unique<Trk::Volume>(makeTransform(compTr), std::move(compBounds));
            auto compTV = std::make_shared<Trk::TrackingVolume>(*compVol, *m_muonMaterial,
                                                                std::move(cscLayerArray), nullptr, compName[ic]);
            components.push_back(std::move(compTV));
            xCurr += xSizes[ic];
            volSteps.push_back(xCurr + xShift);
        }  // end components
    }

    // convert component volumes into array
    std::unique_ptr<Trk::BinnedArray<Trk::TrackingVolume>> compArray{};
    if (!components.empty() && isDiamond) {
        Trk::BinUtility binUtil(volSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
        compArray = m_trackingVolumeArrayCreator->doubleTrapezoidVolumesArrayNav(components, binUtil);
    } else  if (!components.empty() && !isDiamond) {
        
        Trk::BinUtility binUtil(volSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
        compArray = m_trackingVolumeArrayCreator->trapezoidVolumesArrayNav(components, binUtil);

    }
    // ready to build the station prototype
    return std::make_unique<Trk::TrackingVolume>(*envelope, *m_muonMaterial, nullptr, std::move(compArray), name);
}

std::unique_ptr<Trk::TrackingVolume> Muon::MuonStationTypeBuilder::processTgcStation(const GeoVPhysVol* cv, Cache& cache) const {

    const GeoLogVol* clv = cv->getLogVol();
    const std::string& tgc_name = clv->getName();
    const GeoShape* baseShape = clv->getShape();
    Amg::Transform3D sTr( Amg::Transform3D::Identity());
    if (baseShape->type() == "Shift") {
        const GeoShapeShift* shift = static_cast<const GeoShapeShift*>(baseShape);
        baseShape = shift->getOp();
        sTr = shift->getX();
    }
    if (baseShape->type() == "Subtraction") {
        const GeoShapeSubtraction* sub = static_cast<const GeoShapeSubtraction*>(baseShape);
        if (sub) {
            baseShape = sub->getOpA();
        }
    }

    if (baseShape->type() == "Trd") {
        const GeoTrd* trd = static_cast<const GeoTrd*>(baseShape);
        double x1 = trd->getXHalfLength1();
        double y1 = trd->getYHalfLength1();
        double y2 = trd->getYHalfLength2();
        double z = trd->getZHalfLength();
        // define envelope
        auto tgcBounds = std::make_shared<Trk::TrapezoidVolumeBounds>(y1, y2, z, x1);
        // xy -> yz  rotation
        Amg::Transform3D tTr = sTr * Amg::getRotateY3D(M_PI_2) * Amg::getRotateZ3D(M_PI_2);
        std::unique_ptr<Trk::LayerArray> tgcLayerArray{processTGCComponent(cv, *tgcBounds, tTr, cache)};
        printVolumeBounds("TGC envelope bounds:", *tgcBounds);
        auto envelope = std::make_unique<Trk::Volume>(makeTransform(tTr), std::move(tgcBounds));

        // ready to build the station prototype
        auto tgc_station = std::make_unique<Trk::TrackingVolume>(*envelope, *m_muonMaterial, std::move(tgcLayerArray), nullptr, tgc_name);
        return tgc_station;
    } else {
        ATH_MSG_WARNING( tgc_name << ": TGC component not a trapezoid ?  no prototype built ");
    }
    return nullptr;
}

std::unique_ptr<Trk::DetachedTrackingVolume> Muon::MuonStationTypeBuilder::process_sTGC(const Identifier& nswId,
                                                                                        const GeoVPhysVol* gv,
                                                                                        const  Amg::Transform3D& transf) const {

    std::string vName = gv->getLogVol()->getName();
    ATH_MSG_DEBUG("processing sTGC prototype of " << vName);

    const Amg::Transform3D& tr_env(transf);
    std::unique_ptr<Trk::Volume> envelope = m_geoShapeConverter.translateGeoShape(gv->getLogVol()->getShape(), tr_env);
    if (!envelope) {
        ATH_MSG_WARNING("sTGC prototype for " << vName << " not built ");
        return nullptr;
    }
    double thickness = envelopeThickness(envelope->volumeBounds());  // half thickness
    Amg::Transform3D envelope_trf_local = transf.inverse()*envelope->transform();
 
    // use envelope to define layer bounds
    std::shared_ptr<Trk::SurfaceBounds> layBounds{getLayerBoundsFromEnvelope(*envelope)};
    // calculate layer area
    double layArea = area(*layBounds);
    // use area to blend station material
    Trk::MaterialProperties sTgc_mat;
    m_volumeConverter.collectMaterial(gv, sTgc_mat, layArea);
    Trk::HomogeneousLayerMaterial stgcMaterial(sTgc_mat, 0.);
    const double scale = 1. / gv->getNChildVols();
    Trk::MaterialProperties sTgc_layerMat(sTgc_mat);
    sTgc_layerMat *= scale;  // divide station material between layers
    Trk::HomogeneousLayerMaterial stgcLayMaterial(sTgc_layerMat, 0.);

    // loop over child volumes, check transforms / align with readout geometry
   
    std::vector<std::unique_ptr<Trk::PlaneLayer>> layers{};
    unsigned int ic = 0;
    const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
    for (const auto& [cv, trc] : geoGetVolumes(gv)) {
        auto layer = std::make_unique<Trk::PlaneLayer>(transf * trc * envelope_trf_local, layBounds,
                                                  stgcLayMaterial, sTgc_layerMat.thickness());
        if (nswId!=0) {
            const Identifier id = idHelper.channelID(nswId,idHelper.multilayer(nswId),
                                                 idHelper.gasGap(nswId) + ic, sTgcIdHelper::Wire, 1);
            layer->setLayerType(id.get_identifier32().get_compact());
        } else layer->setLayerType(1);   // generic sensitive
        layers.push_back(std::move(layer));
        ++ic;
    }
    // create the BinnedArray
    std::vector<std::shared_ptr<Trk::Layer>> layerOrder;
    std::vector<float> binSteps;
    binSteps.push_back(-thickness);
    for (unsigned int il=0; il < layers.size(); il++) {
        binSteps.push_back(binSteps.back() + sTgc_layerMat.thickness());
        layerOrder.push_back(std::move(layers[il]));
    }
    if (binSteps.back() > thickness + 1.e-3) {
        ATH_MSG_WARNING("rescale stgc binning:" << binSteps.back() << ">" << thickness);
    }
    binSteps.back() = thickness;
    Trk::BinUtility binUtility(binSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
    auto stgcLayerArray = std::make_unique<Trk::NavBinnedArray1D<Trk::Layer>>(layerOrder, binUtility, 
                                                                              Amg::Transform3D(Amg::Transform3D::Identity()));
    // build tracking volume
    auto sTgc = std::make_unique<Trk::TrackingVolume>(*envelope, *m_muonMaterial, 
                                                      std::move(stgcLayerArray), nullptr, vName);
    // create layer representation
    auto layerRepr = std::make_unique<Trk::PlaneLayer>(transf * envelope_trf_local, layBounds, stgcMaterial, sTgc_mat.thickness());
    // create prototype as detached tracking volume
    return std::make_unique<Trk::DetachedTrackingVolume>(vName, std::move(sTgc), std::move(layerRepr), nullptr);
}

std::unique_ptr<Trk::DetachedTrackingVolume> Muon::MuonStationTypeBuilder::process_MM(const Identifier& nswId,
                                                                                      const GeoVPhysVol* gv,
                                                                                      const Amg::Transform3D& transf) const {

    std::string vName = gv->getLogVol()->getName();

    ATH_MSG_DEBUG("processing MM:" << vName << ":"<< gv->getLogVol()->getShape()->type());

    const Amg::Transform3D& tr_env(transf);
    std::unique_ptr<const Trk::Volume> envelope{m_geoShapeConverter.translateGeoShape(gv->getLogVol()->getShape(), tr_env)};
    if (!envelope) {
        ATH_MSG_WARNING("MM prototype for " << vName << " not built ");
        return nullptr;
    }
    double thickness = envelopeThickness(envelope->volumeBounds());
    printVolumeBounds("MM envelope bounds", envelope->volumeBounds());
    Amg::Transform3D envelope_trf_local = transf.inverse()*envelope->transform();
 
    // use envelope to define layer bounds
    std::shared_ptr<Trk::SurfaceBounds> layBounds = getLayerBoundsFromEnvelope(*envelope);
    // calculate layer area
    double layArea = area(*layBounds);
    // use area to blend station material
    Trk::MaterialProperties mm_mat;
    m_volumeConverter.collectMaterial(gv, mm_mat, layArea);
    Trk::HomogeneousLayerMaterial mmMaterial(mm_mat, 0.);
    double scale = 1. / gv->getNChildVols();
    Trk::MaterialProperties mm_layerMat(mm_mat);
    mm_layerMat *= scale;  // divide station material between layers
    Trk::HomogeneousLayerMaterial mmLayMaterial(mm_layerMat, 0.);

    // loop over child volumes, check transforms / align with readout geometry
    std::vector<std::unique_ptr<Trk::PlaneLayer>> layers;
    unsigned int ic = 0;
    const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
    for (const auto& [cv, trc] : geoGetVolumes(gv)) {
        auto layer = std::make_unique<Trk::PlaneLayer>(transf * trc * envelope_trf_local, layBounds, mmLayMaterial, mm_layerMat.thickness());
        if (nswId!=0) {
            Identifier id = idHelper.channelID(nswId, idHelper.multilayer(nswId), 1 + ic, 1);
            layer->setLayerType(id.get_identifier32().get_compact());
        } else layer->setLayerType(1);     // generic sensitive
        layers.push_back(std::move(layer));
        ic++;
    }

    // create the BinnedArray
    std::vector<std::shared_ptr<Trk::Layer>> layerOrder;
    std::vector<float> binSteps;
    binSteps.push_back(-thickness);
    for (unsigned int il=0; il < layers.size(); il++) {
        binSteps.push_back(binSteps.back() + mm_layerMat.thickness());
        layerOrder.push_back(std::move(layers[il]));
    }
    if (binSteps.back() > thickness + 1.e-3) {
        ATH_MSG_WARNING("rescale mm binning:" << binSteps.back() << ">" << thickness);
    }
    binSteps.back() = thickness;
    Trk::BinUtility binUtility(binSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
    auto mmLayerArray = std::make_unique<Trk::NavBinnedArray1D<Trk::Layer>>(layerOrder, binUtility,
                                                                           Amg::Transform3D(Amg::Transform3D::Identity()));
    // build tracking volume
    auto mM = std::make_unique<Trk::TrackingVolume>(*envelope, *m_muonMaterial, std::move(mmLayerArray), nullptr, vName);
    // create layer representation
    auto layerRepr = std::make_unique<Trk::PlaneLayer>(transf * envelope_trf_local, layBounds, mmMaterial, mm_mat.thickness());
    // create prototype as detached tracking volume
    return std::make_unique<Trk::DetachedTrackingVolume>(vName, std::move(mM), std::move(layerRepr), nullptr);
}

double Muon::MuonStationTypeBuilder::get_x_size(const GeoVPhysVol* pv) const {
    double xlow{0.}, xup{0.};
    // subcomponents
    GeoVolumeVec_t vols = geoGetVolumes(pv);
    if (vols.empty()) {
        return decodeX(pv->getLogVol()->getShape());
    }

    for (const auto& [cv, transf] : vols) {
        const GeoLogVol* clv = cv->getLogVol();
        double xh = decodeX(clv->getShape());
        xlow = std::min(xlow, (transf.translation())[0] - xh);
        xup = std::max(xup, (transf.translation())[0] + xh);
    }

    return std::max(-xlow, xup);
}

Trk::MaterialProperties Muon::MuonStationTypeBuilder::getAveragedLayerMaterial(const GeoVPhysVol* pv,
                                                                              double volume,
                                                                              double thickness) const {
    ATH_MSG_DEBUG( "::getAveragedLayerMaterial:processing ");
    // loop through the whole hierarchy; collect material
    Trk::MaterialProperties sumMat;
    // protect nan
    if (thickness > 0.)
        m_volumeConverter.collectMaterial(pv, sumMat, volume / thickness);

    ATH_MSG_VERBOSE( " combined material thickness: "<< sumMat.thickness());
    ATH_MSG_VERBOSE( " actual layer thickness: " << thickness);

    // scale material properties to the actual layer thickness
    if (sumMat.thickness() != thickness && sumMat.thickness() > 0.) {
        double sf = thickness / sumMat.thickness();
        sumMat.material().X0 /= sf;
        sumMat.material().L0 /= sf;
        sumMat.material().rho *= sf;
        ATH_MSG_VERBOSE("averaged material scale :"<< sf << " sumMat.material().X0() "
                        << sumMat.material().X0 << " sumMat.material().L0 "
                        << sumMat.material().L0 << " sumMat.material().rho "
                        << sumMat.material().rho << " sumMat.material().x0() "
                        << sumMat.material().x0());
        ATH_MSG_VERBOSE("averaged material:d,x0,dInX0:"
                        << sumMat.thickness() << "," << sumMat.material().x0());
        return sumMat;
    }
    return sumMat;
}

std::unique_ptr<Trk::LayerArray> Muon::MuonStationTypeBuilder::processCSCTrdComponent(const GeoVPhysVol* pv,
                                                                                      const Trk::TrapezoidVolumeBounds& compBounds,
                                                                                      const Amg::Transform3D& transf, 
                                                                                      Cache& cache) const {
    // tolerance
    std::string name = pv->getLogVol()->getName();
    std::vector<std::unique_ptr<Trk::PlaneLayer>> layers{};
    std::vector<double> x_array{}, x_thickness{};
    std::vector<Trk::MaterialProperties> x_mat;
    std::vector<int> x_active;
    double currX = -100000;
    // while waiting for better suggestion, define a single material layer
    Trk::MaterialProperties matCSC(0., 10.e10, 10.e10, 13., 26., 0.);
    double thickness = 2 * compBounds.halflengthZ();
    double minX = compBounds.minHalflengthX();
    double maxX = compBounds.maxHalflengthX();
    double halfY = compBounds.halflengthY();
    double halfZ = compBounds.halflengthZ();
    if (name.compare(name.size() - 5, 5, "CSC01") == 0) {
        if (!cache.m_matCSC01) {
            double vol = (minX + maxX) * 2 * halfY * thickness;
            cache.m_matCSC01 = std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(pv, vol, thickness));
        }
        matCSC = Trk::MaterialProperties(*cache.m_matCSC01);
        // retrieve number of gas gaps and their position -> turn them into
        // active layers step 1 level below
        const GeoVPhysVol* cv1 = pv->getChildVol(0);
        for (const auto& [cv, transfc]: geoGetVolumes(cv1)) {
            const GeoLogVol* clv = cv->getLogVol();
            if (clv->getName() == "CscArCO2") {
                double xl = transfc.translation().x();
                if (x_array.empty() || xl >= x_array.back()) {
                    x_array.push_back(xl);
                } else {
                    unsigned int ix = 0;
                    while (ix < x_array.size() && x_array[ix] < xl) {
                        ix++;
                    }
                    x_array.insert(x_array.begin() + ix, xl);
                }
            }
        }
        if (x_array.empty()) {
            x_array.push_back(0.);
            x_mat.push_back(matCSC);
            x_thickness.push_back(thickness);
            x_active.push_back(1);
        } else if (x_array.size() == 1) {
            double xthick = 2 * std::min(x_array[0] + halfZ, halfZ - x_array[0]);
            double scale = xthick / thickness;
            Trk::MaterialProperties xmatCSC(xthick, scale * matCSC.x0(), scale * matCSC.l0(),
                                            matCSC.averageA(), matCSC.averageZ(),
                                            matCSC.averageRho() / scale);
            x_mat.push_back(xmatCSC);
            x_thickness.push_back(xthick);
            x_active.push_back(1);
        } else {
            double currX = -halfZ;
            for (unsigned int il=0; il < x_array.size(); il++) {
                double xthick;
                if (il < x_array.size() - 1) {
                    xthick = 2 * std::min(x_array[il] - currX, 0.5 * (x_array[il + 1] - x_array[il]));
                } else {
                    xthick = 2 * std::min(x_array[il] - currX, halfZ - x_array[il]);
                }
                x_thickness.push_back(xthick);
                x_mat.push_back(matCSC);
                currX = x_array[il] + 0.5 * x_thickness.back();
                x_active.push_back(1);
            }
        }
    }
    if (name == "CSCspacer") {
        if (!cache.m_matCSCspacer1) {
            double vol = (minX + maxX) * 2 * halfY * thickness;
            cache.m_matCSCspacer1 = std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(pv, vol, thickness));
        }
        x_array.push_back(0.);
        x_mat.push_back(*cache.m_matCSCspacer1);
        x_thickness.push_back(thickness);
        x_active.push_back(0);
    }
    // create layers
    std::shared_ptr<Trk::SurfaceBounds>  bounds = std::make_unique<Trk::TrapezoidBounds>(minX, maxX, halfY);
    for (unsigned int iloop = 0; iloop < x_array.size(); iloop++) {
        Amg::Transform3D cTr = transf * Amg::getTranslateZ3D(x_array[iloop]);
        Trk::HomogeneousLayerMaterial cscMaterial(x_mat[iloop], 0.);
        auto layer = std::make_unique<Trk::PlaneLayer>(cTr, bounds, cscMaterial, x_thickness[iloop]);
        // make preliminary identification of active layers
        layer->setLayerType(x_active[iloop]);
        layers.push_back(std::move(layer));
    }

    // create the BinnedArray
    std::vector<std::shared_ptr<Trk::Layer>> layerOrder;
    std::vector<float> binSteps;
    double xShift = transf.translation().x();
    float lowX = -compBounds.halflengthZ() + xShift;
    binSteps.push_back(lowX);

    if (!layers.empty()) {
        currX = lowX - xShift;
        for (unsigned int i = 0; i < layers.size() - 1; i++) {
            currX = x_array[i] + 0.5 * layers[i]->thickness();
            layerOrder.push_back(std::move(layers[i]));
            binSteps.push_back(currX + xShift);
        }
        layerOrder.push_back(std::move(layers.back()));
        binSteps.push_back(compBounds.halflengthZ() + xShift);
    }

    Trk::BinUtility binUtility(binSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
    return std::make_unique<Trk::NavBinnedArray1D<Trk::Layer>>(layerOrder, binUtility,
                                                              Amg::Transform3D(Amg::Transform3D::Identity()));
}

std::unique_ptr<Trk::LayerArray>
    Muon::MuonStationTypeBuilder::processCSCDiamondComponent(const GeoVPhysVol* pv,
                                                             const Trk::DoubleTrapezoidVolumeBounds& compBounds,
                                                             const Amg::Transform3D& transf,
                                                             Cache& cache) const {
    // tolerance
    std::string name = pv->getLogVol()->getName();
    std::vector<std::unique_ptr<Trk::PlaneLayer>> layers{};
    std::vector<double> x_array{}, x_thickness{};
    std::vector<Trk::MaterialProperties> x_mat;
    std::vector<int> x_active;
    double currX = -100000;
    // while waiting for better suggestion, define a single material layer
    Trk::MaterialProperties matCSC(0., 10e8, 10e8, 13., 26., 0.);
    double thickness = 2 * compBounds.halflengthZ();
    double minX = compBounds.minHalflengthX();
    double medX = compBounds.medHalflengthX();
    double maxX = compBounds.maxHalflengthX();
    double halfY1 = compBounds.halflengthY1();
    double halfY2 = compBounds.halflengthY2();
    double halfZ = compBounds.halflengthZ();
    if (name.compare(name.size() - 5, 5, "CSC02") == 0) {
        if (!cache.m_matCSC02) {
            double vol = ((minX + medX) * 2 * halfY1 + (medX + maxX) * 2 * halfY2) * thickness;
            cache.m_matCSC02 = std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(pv, vol, thickness));
        }
        matCSC = Trk::MaterialProperties(*cache.m_matCSC02);
        // retrieve number of gas gaps and their position -> turn them into
        // active layers step 1 level below
        const GeoVPhysVol* cv1 = pv->getChildVol(0);
        for (const auto& [cv, transfc] : geoGetVolumes(cv1)) {
            const GeoLogVol* clv = cv->getLogVol();
            if (clv->getName() == "CscArCO2") {
                double xl = transfc.translation().x();
                if (x_array.empty() || xl >= x_array.back()) {
                    x_array.push_back(xl);
                } else {
                    unsigned int ix = 0;
                    while (ix < x_array.size() && x_array[ix] < xl) {
                        ix++;
                    }
                    x_array.insert(x_array.begin() + ix, xl);
                }
            }
        }
        //
        if (x_array.empty()) {
            x_array.push_back(0.);
            x_mat.push_back(matCSC);
            x_thickness.push_back(thickness);
            x_active.push_back(1);
        } else if (x_array.size() == 1) {
            x_mat.push_back(matCSC);
            x_thickness.push_back(2 *std::min(x_array[0] + halfZ, halfZ - x_array[0]));
            x_active.push_back(1);
        } else {
            double currX = -halfZ;
            for (unsigned int il=0; il < x_array.size(); il++) {
                double xthick{0.};
                if (il < x_array.size() - 1) {
                    xthick = 2 * std::min(x_array[il] - currX, 0.5 * (x_array[il + 1] - x_array[il]));
                    x_thickness.push_back(xthick);
                } else {
                    xthick = 2 * std::min(x_array[il] - currX, halfZ - x_array[il]);
                    x_thickness.push_back(xthick);
                }
                x_mat.push_back(matCSC);
                currX = x_array[il] + 0.5 * x_thickness.back();
                x_active.push_back(1);
            }
        }
    }
    if (name == "CSCspacer") {
        if (!cache.m_matCSCspacer2) {
            double vol = ((minX + medX) * 2 * halfY1 + (medX + maxX) * 2 * halfY2) *thickness;
            cache.m_matCSCspacer2 = std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(pv, vol, thickness));
        }
        matCSC = Trk::MaterialProperties(*cache.m_matCSCspacer2);
        x_array.push_back(0.);
        x_mat.push_back(matCSC);
        x_thickness.push_back(thickness);
        x_active.push_back(0);
    }
    // create layers
    std::shared_ptr<Trk::DiamondBounds> dbounds = std::make_unique<Trk::DiamondBounds>(minX, medX, maxX, halfY1, halfY2);
    for (unsigned int iloop = 0; iloop < x_array.size(); iloop++) {
        Amg::Transform3D cTr = transf * Amg::getTranslateZ3D(x_array[iloop]);
        Trk::HomogeneousLayerMaterial cscMaterial(x_mat[iloop], 0.);
        auto layer = std::make_unique<Trk::PlaneLayer>(cTr, dbounds, cscMaterial, x_thickness[iloop]);
        // make preliminary identification of active layers
        layer->setLayerType(x_active[iloop]);
        layers.push_back(std::move(layer));
    }

    // create the BinnedArray
    std::vector<std::shared_ptr<Trk::Layer>> layerOrder;
    std::vector<float> binSteps;
    double xShift = transf.translation().x();
    double lowX = -compBounds.halflengthZ() + xShift;
    binSteps.push_back(lowX);

    if (!layers.empty()) {
        currX = lowX;
        for (unsigned int i = 0; i < layers.size() - 1; i++) {
            currX = x_array[i] + 0.5 * layers[i]->thickness() + xShift;
            binSteps.push_back(currX);
            layerOrder.push_back(std::move(layers[i]));
        }

        layerOrder.push_back(std::move(layers.back()));
        binSteps.push_back(compBounds.halflengthZ() + xShift);
    }
   
   Trk::BinUtility binUtility(binSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
   return std::make_unique<Trk::NavBinnedArray1D<Trk::Layer>>(layerOrder, binUtility,
                                                               Amg::Transform3D(Amg::Transform3D::Identity()));

}

std::unique_ptr<Trk::LayerArray> Muon::MuonStationTypeBuilder::processTGCComponent(const GeoVPhysVol* pv,
                                                                                  const Trk::TrapezoidVolumeBounds& tgcBounds,
                                                                                  const Amg::Transform3D& transf,
                                                                                  Cache& cache) const {
    // tolerance
    constexpr double tol{0.001};
    std::string name = pv->getLogVol()->getName();
    std::vector<std::unique_ptr<Trk::PlaneLayer>> layers{};
    std::vector<double> x_array{}, x_thickness{};
    std::vector<Trk::MaterialProperties> x_mat;
    double currX = -100000;
    // while waiting for better suggestion, define a single material layer
    Trk::MaterialProperties matTGC(0., 10e8, 10e8, 13., 26., 0.);
    double minX = tgcBounds.minHalflengthX();
    double maxX = tgcBounds.maxHalflengthX();
    double halfY = tgcBounds.halflengthY();
    double halfZ = tgcBounds.halflengthZ();
    double thickness = 2 * halfZ;
    if (std::abs(halfZ - 35.00) < tol) {
        if (!cache.m_matTGC01) {
            double vol = (minX + maxX) * 2 * halfY * thickness;
            cache.m_matTGC01 = std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(pv, vol, thickness));
        }
        matTGC = Trk::MaterialProperties(*cache.m_matTGC01);
    } else if (std::abs(halfZ - 21.85) < tol) {
        if (!cache.m_matTGC06) {
            double vol = (minX + maxX) * 2 * halfY * thickness;
            cache.m_matTGC06 = std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(pv, vol, thickness));
        }
        matTGC = Trk::MaterialProperties(*cache.m_matTGC06);
    } else {
        ATH_MSG_DEBUG("unknown TGC material:" << halfZ);
    }

    for (const auto& [cv, transfc] : geoGetVolumes(pv)) {
        const GeoLogVol* clv = cv->getLogVol();
        if (clv->getName() == "muo::TGCGas") {
            double xl = transfc.translation().x();
            if (x_array.empty() || xl >= x_array.back()) {
                x_array.push_back(xl);
            } else {
                unsigned int ix = 0;
                while (ix < x_array.size() && x_array[ix] < xl) {
                    ix++;
                }
                x_array.insert(x_array.begin() + ix, xl);
            }
        }
    }
    double activeThick{0.};
    if (x_array.empty()) {
        x_array.push_back(0.);
        x_thickness.push_back(thickness);
        activeThick = thickness;
    } else if (x_array.size() == 1) {
        x_thickness.push_back(2 * fmin(x_array[0] + halfZ, halfZ - x_array[0]));
        activeThick += x_thickness.back();
    } else {
        double currX = -halfZ;
        for (unsigned int il=0; il < x_array.size(); ++il) {
            if (il < x_array.size() - 1) {
                x_thickness.push_back(2 * std::min(x_array[il] - currX, 0.5 * (x_array[il + 1] - x_array[il])));
            } else {
                x_thickness.push_back(2 * std::min(x_array[il] - currX, halfZ - x_array[il]));
            }
            currX = x_array[il] + 0.5 * x_thickness.back();
            activeThick += x_thickness.back();
        }
    }
    // rescale material to match the combined thickness of active layers
    double scale = activeThick / thickness;
    matTGC = Trk::MaterialProperties(activeThick, scale * matTGC.x0(), scale * matTGC.l0(),
                                     matTGC.averageA(), matTGC.averageZ(), matTGC.averageRho() / scale);
    // create layers
    std::shared_ptr<Trk::SurfaceBounds> bounds = std::make_unique<Trk::TrapezoidBounds>(minX, maxX, halfY);

    for (unsigned int iloop = 0; iloop < x_array.size(); iloop++) {
        Amg::Transform3D cTr = Amg::getTranslateX3D(x_array[iloop]) * transf;
        Trk::HomogeneousLayerMaterial tgcMaterial(matTGC, 0.);
        auto layer = std::make_unique<Trk::PlaneLayer>(cTr, bounds, tgcMaterial, x_thickness[iloop]);
        // make preliminary identification of active layers
        layer->setLayerType(1);
        layers.push_back(std::move(layer));
    }
    // create the BinnedArray
    std::vector<std::shared_ptr<Trk::Layer>> layerOrder;
    std::vector<float> binSteps;
    //
    float xShift = transf.translation().x();
    float lowX = -halfZ + xShift;
    binSteps.push_back(lowX);
    if (!layers.empty()) {
        currX = lowX;
        for (unsigned int i = 0; i < layers.size() - 1; ++i) {
            currX = x_array[i] + 0.5 * layers[i]->thickness() +  xShift;
            binSteps.push_back(currX);
            layerOrder.push_back(std::move(layers[i]));
         }
        layerOrder.push_back(std::move(layers.back()));
        binSteps.push_back(halfZ + xShift);
    }
    Trk::BinUtility binUtility(binSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
    return std::make_unique<Trk::NavBinnedArray1D<Trk::Layer>>(layerOrder, binUtility,
                                                               Amg::Transform3D(Amg::Transform3D::Identity()));

}

double Muon::MuonStationTypeBuilder::decodeX(const GeoShape* sh) const {
    double xHalf{0.};

    const GeoTrd* trd = dynamic_cast<const GeoTrd*>(sh);
    const GeoBox* box = dynamic_cast<const GeoBox*>(sh);
    const GeoTube* tub = dynamic_cast<const GeoTube*>(sh);
    const GeoTubs* tubs = dynamic_cast<const GeoTubs*>(sh);
    const GeoShapeShift* shift = dynamic_cast<const GeoShapeShift*>(sh);
    const GeoShapeUnion* uni = dynamic_cast<const GeoShapeUnion*>(sh);
    const GeoShapeSubtraction* sub = dynamic_cast<const GeoShapeSubtraction*>(sh);
    const GeoSimplePolygonBrep* spb =  dynamic_cast<const GeoSimplePolygonBrep*>(sh);

    if (!trd && !box && !tub && !tubs && !shift && !uni && !sub && !spb) {
        ATH_MSG_WARNING("decodeX(GeoShape="
                        << sh->type() << "): shape type " << sh->type()
                        << " is unknown, returning xHalf=0");
        return xHalf;
    }

    if (spb) {
        for (unsigned int i = 0; i < spb->getNVertices(); i++) {
            ATH_MSG_DEBUG(" XVertex " << spb->getXVertex(i) << " YVertex "
                                      << spb->getYVertex(i));
            if (spb->getXVertex(i) > xHalf)
                xHalf = spb->getXVertex(i);
        }
        ATH_MSG_DEBUG(" GeoSimplePolygonBrep xHalf " << xHalf);
    }

    if (trd){
        xHalf = std::max(trd->getXHalfLength1(), trd->getXHalfLength2());
    }
    if (box) {
        xHalf = box->getXHalfLength();
    }
    if (tub) {
        xHalf = tub->getRMax();
    }
    if (sub) {
        // be careful to handle properly GeoModel habit of subtracting large
        // volumes from smaller ones
        double xA = decodeX(sub->getOpA());
        xHalf = xA;
    }
    if (uni) {
        xHalf = std::max(decodeX(uni->getOpA()), decodeX(uni->getOpB()));
    }
    if (shift) {
        double xA = decodeX(shift->getOp());
        double xB = shift->getX().translation().x();
        xHalf = xA + std::abs(xB);
    }

    return xHalf;
}

std::pair<std::unique_ptr<Trk::Layer>,std::vector<std::unique_ptr<Trk::Layer>>>
    Muon::MuonStationTypeBuilder::createLayerRepresentation(Trk::TrackingVolume& trVol) const {

    std::unique_ptr<Trk::Layer> layRepr{};


    std::vector<std::unique_ptr<Trk::Layer>> multi{};

    // retrieve volume envelope

    Trk::CuboidVolumeBounds* cubBounds = dynamic_cast<Trk::CuboidVolumeBounds*>(&(trVol.volumeBounds()));
    Trk::TrapezoidVolumeBounds* trdBounds = dynamic_cast<Trk::TrapezoidVolumeBounds*>(&(trVol.volumeBounds()));
    Trk::DoubleTrapezoidVolumeBounds* dtrdBounds =dynamic_cast<Trk::DoubleTrapezoidVolumeBounds*>(&(trVol.volumeBounds()));

    Amg::Transform3D subt{Amg::Transform3D::Identity()};

    Trk::SubtractedVolumeBounds* subBounds = dynamic_cast<Trk::SubtractedVolumeBounds*>(&(trVol.volumeBounds()));
    if (subBounds) {
        subt = Amg::getRotateY3D(M_PI_2) *Amg::getRotateZ3D(M_PI_2);
        while (subBounds) {
            cubBounds = dynamic_cast<Trk::CuboidVolumeBounds*>(&(subBounds->outer()->volumeBounds()));
            trdBounds = dynamic_cast<Trk::TrapezoidVolumeBounds*>(&(subBounds->outer()->volumeBounds()));
            dtrdBounds = dynamic_cast<Trk::DoubleTrapezoidVolumeBounds*>(&(subBounds->outer()->volumeBounds()));
            subBounds = dynamic_cast<Trk::SubtractedVolumeBounds*>(&(subBounds->outer()->volumeBounds()));
        }
    }

    if (cubBounds) {
        double thickness = 2 * cubBounds->halflengthX();
        double sf = 4 * cubBounds->halflengthZ() * cubBounds->halflengthY();
        auto bounds = std::make_shared<Trk::RectangleBounds>(cubBounds->halflengthY(), cubBounds->halflengthZ());
        Trk::MaterialProperties matProp = collectStationMaterial(trVol, sf);
        ATH_MSG_VERBOSE(" collectStationMaterial cub " << matProp);
        if (matProp.thickness() > thickness) {
            ATH_MSG_DEBUG(" thickness of combined station material exceeds station size:" << trVol.volumeName());
        } else if (matProp.thickness() < thickness && matProp.thickness() > 0.) {

            double sf = thickness / matProp.thickness();
            matProp = Trk::MaterialProperties(thickness, sf * matProp.x0(), sf * matProp.l0(),
                                              matProp.averageA(), matProp.averageZ(), matProp.averageRho() / sf);
        }
        Trk::HomogeneousLayerMaterial mat(matProp, 0.);
        layRepr = std::make_unique<Trk::PlaneLayer>(Amg::getRotateY3D(M_PI_2) * Amg::getRotateZ3D(M_PI_2),
                                                    std::make_shared<Trk::RectangleBounds>(*bounds), mat, thickness, nullptr, 1);
        // multilayers
        if (m_multilayerRepresentation && trVol.confinedVolumes()) {
            std::span<Trk::TrackingVolume* const> vols = trVol.confinedVolumes()->arrayObjects();
            if (vols.size() > 1) {
                for (auto* vol : vols) {
                    Trk::MaterialProperties matMulti = collectStationMaterial(*vol, sf);
                    ATH_MSG_VERBOSE(" collectStationMaterial cub matMulti "<< matMulti);
                    multi.emplace_back(std::make_unique<Trk::PlaneLayer>(vol->transform() * Amg::getRotateY3D(M_PI_2) *  Amg::getRotateZ3D(M_PI_2),
                                                                          std::make_shared<Trk::RectangleBounds>(*bounds),
                                                                          Trk::HomogeneousLayerMaterial(matMulti, 0.),
                                                                          matMulti.thickness(), nullptr, 1));
                }
            }
        }
    } else if (trdBounds) {
        double thickness = 2 * trdBounds->halflengthZ();
        double sf = 2 * (trdBounds->minHalflengthX() + trdBounds->maxHalflengthX()) * trdBounds->halflengthY();
        std::vector<std::unique_ptr<Trk::Surface>> surfs = trdBounds->decomposeToSurfaces(Amg::Transform3D::Identity());
        const Trk::TrapezoidBounds* tbounds = dynamic_cast<const Trk::TrapezoidBounds*>(&surfs[0]->bounds());
        std::shared_ptr<Trk::SurfaceBounds> bounds = std::make_shared<Trk::TrapezoidBounds>(*tbounds);
        Trk::MaterialProperties matProp = collectStationMaterial(trVol, sf);
        ATH_MSG_VERBOSE(" collectStationMaterial trd " << matProp << trVol.volumeName());
        if (matProp.thickness() > thickness) {
            ATH_MSG_DEBUG(" thickness of combined station material exceeds station size:" << trVol.volumeName());
        } else if (matProp.thickness() < thickness && matProp.thickness() > 0.) {
            float sf = thickness / matProp.thickness();
            matProp = Trk::MaterialProperties(thickness, sf * matProp.x0(), sf * matProp.l0(),
                                             matProp.averageA(), matProp.averageZ(), matProp.averageRho() / sf);
        }
        Trk::HomogeneousLayerMaterial mat(matProp, 0.);
        layRepr = std::make_unique<Trk::PlaneLayer>(subt * trVol.transform(), bounds, mat, thickness, nullptr, 1);

        // multilayers
        if (m_multilayerRepresentation && trVol.confinedVolumes()) {
            std::span<Trk::TrackingVolume* const> vols = trVol.confinedVolumes()->arrayObjects();
            if (vols.size() > 1) {
                for (auto* vol : vols) {
                    Trk::MaterialProperties matMulti = collectStationMaterial(*vol, sf);
                    ATH_MSG_VERBOSE(" collectStationMaterial trd matMulti  "<< matMulti);
                    multi.emplace_back(std::make_unique<Trk::PlaneLayer>(Amg::Transform3D(vol->transform()), bounds,
                                                                         Trk::HomogeneousLayerMaterial(matMulti, 0.),
                                                                         matMulti.thickness(), nullptr, 1));
                }
            }
        }
    } else if (dtrdBounds) {
        double thickness = 2 * dtrdBounds->halflengthZ();
        double sf = 2 * (dtrdBounds->minHalflengthX() + dtrdBounds->medHalflengthX()) * dtrdBounds->halflengthY1() +
                    2 * (dtrdBounds->medHalflengthX() + dtrdBounds->maxHalflengthX()) * dtrdBounds->halflengthY2();
        std::vector<std::unique_ptr<Trk::Surface>> surfs = dtrdBounds->decomposeToSurfaces(Amg::Transform3D::Identity());
        const Trk::DiamondBounds* dbounds = dynamic_cast<const Trk::DiamondBounds*>(&surfs[0]->bounds());
        std::shared_ptr<Trk::SurfaceBounds> bounds = std::make_shared<Trk::DiamondBounds>(*dbounds);
        Trk::MaterialProperties matProp = collectStationMaterial(trVol, sf);
        ATH_MSG_VERBOSE(" collectStationMaterial dtrd  " << matProp);
        if (matProp.thickness() > thickness) {
            ATH_MSG_DEBUG(" thickness of combined station material exceeds station size:"<< trVol.volumeName());
        } else if (matProp.thickness() < thickness && matProp.thickness() > 0.) {
            float sf = thickness / matProp.thickness();
            matProp = Trk::MaterialProperties(thickness, sf * matProp.x0(), sf * matProp.l0(),
                                              matProp.averageA(), matProp.averageZ(),
                                              matProp.averageRho() / sf);
        }
        Trk::HomogeneousLayerMaterial mat(matProp, 0.);
        layRepr = std::make_unique<Trk::PlaneLayer>(trVol.transform(), bounds, mat, thickness, nullptr, 1);
        // multilayers
        if (m_multilayerRepresentation && trVol.confinedVolumes()) {
            std::span<Trk::TrackingVolume* const> vols = trVol.confinedVolumes()->arrayObjects();
            if (vols.size() > 1) {
                for (auto* vol : vols) {
                    Trk::MaterialProperties matMulti = collectStationMaterial(*vol, sf);
                    ATH_MSG_VERBOSE(" collectStationMaterial dtrd matMulti  " << matMulti);
                    multi.emplace_back(std::make_unique<Trk::PlaneLayer>(vol->transform(), bounds,
                                                                         Trk::HomogeneousLayerMaterial(matMulti, 0.),
                                                                         matMulti.thickness(), nullptr, 1));
                }
            }
        }
    }
    return std::make_pair(std::move(layRepr), std::move(multi));
}

Identifier Muon::MuonStationTypeBuilder::identifyNSW(const std::string& vName,
                                                     const Amg::Transform3D& transf) const {
    Identifier id(0);

    if ((vName[0] == 'Q') || (vName[0] == 'M')) {  // NSW stations
        // station eta
        std::istringstream istr(&vName[1]);
        int iEta;
        if (vName[0] == 'Q') {
            std::istringstream istr2(&vName[2]);
            istr2 >> iEta;
        } else
            istr >> iEta;
        if (transf.translation().z() < 0.)
            iEta *= -1;
        // station Phi
        unsigned int iPhi = 1;
        // if (trVol.center().z()>0.) iPhi += 8;
        // station multilayer
        std::istringstream istm(&vName[3]);
        int iMult;
        istm >> iMult;
        if (vName[0] == 'Q' && vName[3] == 'P')
            iMult = (vName[1] == 'L') ? 1 : 2;
        if (vName[0] == 'Q' && vName[3] == 'C')
            iMult = (vName[1] == 'L') ? 2 : 1;
        // layer
        std::string stl(&vName[vName.size() - 1]);
        std::istringstream istl(stl);
        int iLay;
        istl >> iLay;
        iLay += 1;
        if (vName[0] == 'Q') {
            std::string stName = (vName[1] == 'L') ? "STL" : "STS";
            id = m_idHelperSvc->stgcIdHelper().channelID(
                stName, iEta, iPhi, iMult, iLay, 2, 1);  // wire position
        } else {
            std::string stName = (vName[2] == 'L') ? "MML" : "MMS";
            id = m_idHelperSvc->mmIdHelper().channelID(stName, iEta, iPhi, iMult,
                                                     iLay, 1);
        }
    }

    return id;
}

Trk::MaterialProperties Muon::MuonStationTypeBuilder::collectStationMaterial(const Trk::TrackingVolume& vol, double sf) const {
    Trk::MaterialProperties layMat(0., 10.e10, 10.e10, 13., 26., 0.);

    // sf is surface of the new layer used to calculate the average 'thickness'
    // of components layers
    if (vol.confinedLayers()) {
        std::span<Trk::Layer const* const> lays = vol.confinedLayers()->arrayObjects();
        for (const auto* lay : lays) {
            const Trk::MaterialProperties* mLay =lay->layerMaterialProperties()->fullMaterial(lay->surfaceRepresentation().center());
            // protect nan
            if (mLay && lay->thickness() > 0 && mLay->material().x0() > 0.) {
                layMat.addMaterial(mLay->material(),
                                   lay->thickness() / mLay->material().x0());
                ATH_MSG_VERBOSE(" collectStationMaterial after add confined lay "<< layMat);
            }
        }
    }
    if (!vol.confinedArbitraryLayers().empty()) {
        Trk::ArraySpan<const Trk::Layer* const> lays = vol.confinedArbitraryLayers();
        for (const auto* lay : lays) {
            const Trk::MaterialProperties* mLay = lay->layerMaterialProperties()->fullMaterial(lay->surfaceRepresentation().center());
            // scaling factor
            const Trk::RectangleBounds* rect = dynamic_cast<const Trk::RectangleBounds*>(&(lay->surfaceRepresentation().bounds()));
            const Trk::TrapezoidBounds* trap =dynamic_cast<const Trk::TrapezoidBounds*>(&(lay->surfaceRepresentation().bounds()));
            if ((rect || trap) && mLay) {
                double scale = rect ? 4 * rect->halflengthX() * rect->halflengthY() / sf
                                    : 2 * (trap->minHalflengthX() + trap->maxHalflengthX()) * trap->halflengthY() / sf;
                // protect nan
                if (lay->thickness() > 0 && mLay->material().x0() > 0.) {
                    layMat.addMaterial(mLay->material(),
                                       scale * lay->thickness() / mLay->material().x0());
                    ATH_MSG_VERBOSE(" collectStationMaterial after add confined sub lay "<< layMat);
                }
            }
        }
    }
    // subvolumes
    if (vol.confinedVolumes()) {
        std::span<Trk::TrackingVolume const* const> subVols = vol.confinedVolumes()->arrayObjects();
        for (const auto* subVol : subVols) {
            if (subVol->confinedLayers()) {
                std::span<Trk::Layer const* const> lays = subVol->confinedLayers()->arrayObjects();
                for (const auto* lay : lays) {
                    const Trk::MaterialProperties* mLay = lay->layerMaterialProperties()->fullMaterial(
                                                                            lay->surfaceRepresentation().center());
                    // protect nan
                    if (mLay && lay->thickness() > 0 && mLay->material().x0() > 0.) {
                        layMat.addMaterial(mLay->material(), lay->thickness() / mLay->material().x0());
                        ATH_MSG_VERBOSE(" collectStationMaterial after add confined vol " << layMat);
                    }
                }
            }
            if (!subVol->confinedArbitraryLayers().empty()) {
                Trk::ArraySpan<const Trk::Layer* const> lays = (subVol->confinedArbitraryLayers());
                for (const auto* lay : lays) {
                    const Trk::MaterialProperties* mLay = lay->layerMaterialProperties()->fullMaterial(lay->surfaceRepresentation().center());
                    // scaling factor
                    const Trk::RectangleBounds* rect =
                            dynamic_cast<const Trk::RectangleBounds*>(&(lay->surfaceRepresentation().bounds()));
                    const Trk::TrapezoidBounds* trap =
                            dynamic_cast<const Trk::TrapezoidBounds*>(&(lay->surfaceRepresentation().bounds()));
                    if ((rect || trap) && mLay) {
                        double scale = rect ? 4 * rect->halflengthX() * rect->halflengthY() / sf
                                            : 2 * (trap->minHalflengthX() +  trap->maxHalflengthX()) * trap->halflengthY() / sf;
                        // protect nan
                        if (lay->thickness() > 0 &&  mLay->material().x0() > 0.) {
                            layMat.addMaterial(mLay->material(), scale * lay->thickness() /  mLay->material().x0());
                            ATH_MSG_VERBOSE(" collectStationMaterial after add sub vols " << layMat);
                        }
                    }
                }
            }
        }
    }
    ATH_MSG_VERBOSE(" collectStationMaterial " << layMat);
    return layMat;
}

void Muon::MuonStationTypeBuilder::printVolumeBounds(const std::string & comment, const Trk::VolumeBounds& volBounds) const {

    ATH_MSG_DEBUG(comment);

    const Trk::CuboidVolumeBounds* box =
        dynamic_cast<const Trk::CuboidVolumeBounds*>(&volBounds);
    if (box) {
        ATH_MSG_DEBUG("cuboid:" << box->halflengthX() << ","
                                << box->halflengthY() << ","
                                << box->halflengthZ());
        return;
    }
    const Trk::TrapezoidVolumeBounds* trd =
        dynamic_cast<const Trk::TrapezoidVolumeBounds*>(&volBounds);
    if (trd) {
        ATH_MSG_DEBUG("trapezoid:" << trd->minHalflengthX() << ","
                                   << trd->maxHalflengthX() << ","
                                   << trd->halflengthY() << ","
                                   << trd->halflengthZ());
        return;
    }
    const Trk::DoubleTrapezoidVolumeBounds* dtrd =
        dynamic_cast<const Trk::DoubleTrapezoidVolumeBounds*>(&volBounds);
    if (dtrd) {
        ATH_MSG_DEBUG("double trapezoid:"
                      << dtrd->minHalflengthX() << "," << dtrd->medHalflengthX()
                      << "," << dtrd->maxHalflengthX() << ","
                      << dtrd->halflengthY1() << "," << dtrd->halflengthY2()
                      << "," << dtrd->halflengthZ());
        return;
    }

    const Trk::SimplePolygonBrepVolumeBounds* spb =
        dynamic_cast<const Trk::SimplePolygonBrepVolumeBounds*>(&volBounds);
    if (spb){
      ATH_MSG_DEBUG("SimplePolygonBrep bounds: number of vertices:"
		    << spb->xyVertices().size());
      return;
    }
}

double Muon::MuonStationTypeBuilder::envelopeThickness(const Trk::VolumeBounds& volBounds) {

    const Trk::CuboidVolumeBounds* box = dynamic_cast<const Trk::CuboidVolumeBounds*>(&volBounds);
    if (box)
        return box->halflengthZ();

    const Trk::TrapezoidVolumeBounds* trd = dynamic_cast<const Trk::TrapezoidVolumeBounds*>(&volBounds);
    if (trd)
        return trd->halflengthZ();

    const Trk::DoubleTrapezoidVolumeBounds* dtrd = dynamic_cast<const Trk::DoubleTrapezoidVolumeBounds*>(&volBounds);
    if (dtrd)
        return dtrd->halflengthZ();

    const Trk::SimplePolygonBrepVolumeBounds* spb = dynamic_cast<const Trk::SimplePolygonBrepVolumeBounds*>(&volBounds);
    if (spb)
        return spb->halflengthZ();

    return 0.;
}

std::unique_ptr<Trk::SurfaceBounds>
    Muon::MuonStationTypeBuilder::getLayerBoundsFromEnvelope(const Trk::Volume& envelope) {

    const Trk::CuboidVolumeBounds* box =
        dynamic_cast<const Trk::CuboidVolumeBounds*>(&(envelope.volumeBounds()));
    if (box){
        return std::make_unique<Trk::RectangleBounds>(box->halflengthX(), box->halflengthY());
    }
    const Trk::TrapezoidVolumeBounds* trd =dynamic_cast<const Trk::TrapezoidVolumeBounds*>(&(envelope.volumeBounds()));

    if (trd) {
        return std::make_unique<Trk::TrapezoidBounds>(trd->minHalflengthX(), trd->maxHalflengthX(), trd->halflengthY());
    }
    const Trk::DoubleTrapezoidVolumeBounds* dtrd = dynamic_cast<const Trk::DoubleTrapezoidVolumeBounds*>(&(envelope.volumeBounds()));

    if (dtrd)
        return std::make_unique<Trk::DiamondBounds>(dtrd->minHalflengthX(), dtrd->medHalflengthX(),
                                                    dtrd->maxHalflengthX(), dtrd->halflengthY1(), dtrd->halflengthY2());

    return nullptr;
}

double Muon::MuonStationTypeBuilder::area(const Trk::SurfaceBounds& sb) {

    const Trk::RectangleBounds* box = dynamic_cast<const Trk::RectangleBounds*>(&sb);
    if (box) {
        return 4 * box->halflengthX() * box->halflengthY();
    }
    const Trk::TrapezoidBounds* trd = dynamic_cast<const Trk::TrapezoidBounds*>(&sb);
    if (trd) {
        return 2 * (trd->minHalflengthX() + trd->maxHalflengthX()) * trd->halflengthY();
    }
    const Trk::DiamondBounds* dtrd = dynamic_cast<const Trk::DiamondBounds*>(&sb);
    if (dtrd) {
        return 2 * (dtrd->minHalflengthX() + dtrd->medHalflengthX()) * dtrd->halflengthY1() +
               2 * (dtrd->medHalflengthX() + dtrd->maxHalflengthX()) * dtrd->halflengthY2();
    }
    return 0.;
}
