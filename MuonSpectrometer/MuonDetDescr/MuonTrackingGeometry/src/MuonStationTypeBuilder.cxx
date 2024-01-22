/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Muon
#include "MuonTrackingGeometry/MuonStationTypeBuilder.h"
// MuonSpectrometer include
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
// Amg
#include "GeoPrimitives/GeoPrimitives.h"
// Trk
#include <fstream>

#include "TrkDetDescrInterfaces/ILayerArrayCreator.h"
#include "TrkDetDescrInterfaces/ILayerBuilder.h"
#include "TrkDetDescrUtils/BinUtility.h"
#include "TrkDetDescrUtils/BinnedArray.h"
#include "TrkDetDescrUtils/BinningType.h"
#include "TrkDetDescrUtils/GeometryStatics.h"
#include "TrkDetDescrUtils/NavBinnedArray1D.h"
#include "TrkDetDescrUtils/SharedObject.h"
#include "TrkDetDescrGeoModelCnv/GeoMaterialConverter.h"


#include "TrkGeometry/Material.h"
#include "TrkGeometry/MaterialProperties.h"
#include "TrkGeometry/CylinderLayer.h"
#include "TrkGeometry/DetachedTrackingVolume.h"
#include "TrkGeometry/DiscLayer.h"
#include "TrkGeometry/HomogeneousLayerMaterial.h"
#include "TrkGeometry/LayerMaterialProperties.h"
#include "TrkGeometry/OverlapDescriptor.h"
#include "TrkGeometry/PlaneLayer.h"
#include "TrkGeometry/SubtractedPlaneLayer.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkGeometrySurfaces/SubtractedPlaneSurface.h"


#include "TrkSurfaces/DiamondBounds.h"
#include "TrkSurfaces/RotatedDiamondBounds.h"
#include "TrkSurfaces/DiscBounds.h"
#include "TrkSurfaces/RectangleBounds.h"
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



#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoShape.h"
#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelKernel/GeoShapeSubtraction.h"
#include "GeoModelKernel/GeoShapeUnion.h"
#include "GeoModelKernel/GeoSimplePolygonBrep.h"
#include "GeoModelKernel/GeoTrd.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoTubs.h"
#include "GeoModelUtilities/GeoVisitVolumes.h"

// stl
#include <map>
#include <cmath> //for std::abs

static const InterfaceID IID_IMuonStationTypeBuilder("MuonStationTypeBuilder", 1, 0);

const InterfaceID& Muon::MuonStationTypeBuilder::interfaceID() { return IID_IMuonStationTypeBuilder; }

// constructor
Muon::MuonStationTypeBuilder::MuonStationTypeBuilder(const std::string& t, const std::string& n, const IInterface* p) :
    AthAlgTool(t, n, p) {
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
        ATH_MSG_FATAL("Could not create the material in " << name() << " initialize()");
        return StatusCode::FAILURE;
    }

    ATH_MSG_INFO(name() << " initialize() successful");

    return StatusCode::SUCCESS;
}

std::vector< Trk::Layer* >* Muon::MuonStationTypeBuilder::processBoxComponentsArbitrary(const GeoVPhysVol* mv,
                                                                                    Trk::CuboidVolumeBounds* envelope,
											Cache& /*cache*/) const {
    ATH_MSG_DEBUG(name() << " processing station components for " << mv->getLogVol()->getName());
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    std::vector< Trk::Layer*> lays;

    // initial solution : single layer collecting all material ; TODO : resolve sensitive layers and spacers
    // use envelope to define layer bounds
    Trk::RectangleBounds*  layBounds = new Trk::RectangleBounds(envelope->halflengthY(), envelope->halflengthZ());
    // calculate layer area
    double layArea = 0.;
    if (layBounds) layArea = area( layBounds );  // TODO this should be a functionality of SurfaceBounds
    // use area to blend station material
    Trk::MaterialProperties box_mat;
    m_volumeConverter.collectMaterial(mv,box_mat, layArea);
    Trk::HomogeneousLayerMaterial boxMaterial(box_mat, 0.);  

    Trk::PlaneLayer*  layer = new Trk::PlaneLayer(Amg::Transform3D::Identity(), layBounds, boxMaterial, 2*envelope->halflengthX());

    lays.push_back(layer);

    return new std::vector<Trk::Layer*>(lays);    
}

Trk::TrackingVolumeArray* Muon::MuonStationTypeBuilder::processBoxStationComponents(const GeoVPhysVol* mv,
                                                                                    Trk::CuboidVolumeBounds* envelope,
                                                                                    Cache& cache) const {
    ATH_MSG_DEBUG(name() << " processing station components for " << mv->getLogVol()->getName());
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    double tolerance = 0.001;

    // loop over children volumes: check if compatible with binning in X ( detect overlap of sensitive volumes )
    std::vector<std::pair<double,double>>  xVol;
    double xpos, xh;
    for (const GeoVolumeVec_t::value_type& p : geoGetVolumes(mv)) {
        const GeoVPhysVol* cv = p.first;
        const GeoLogVol* clv = cv->getLogVol();
        const Amg::Transform3D& transf = p.second;
	// consider sensitive volumes only
	std::string name = clv->getName(); 
	if ( name.find("MDT")==std::string::npos && name.find("RPC")==std::string::npos ) continue;
	xpos = transf.translation().x();
        if (clv->getShape()->type() == "Trd") {
           const GeoTrd* trd = dynamic_cast<const GeoTrd*>(clv->getShape());
           xh = std::max(trd->getXHalfLength1(), trd->getXHalfLength2());
        } else if (clv->getShape()->type() == "Box") {
	  const GeoBox* box = dynamic_cast<const GeoBox*>(clv->getShape());
	  xh = box->getXHalfLength();
        } else {
          xh = get_x_size(cv);
        }
	if (!xVol.size() || xpos>xVol.back().first) xVol.push_back(std::make_pair(xpos,xh));
	else {
	  std::vector<std::pair<double,double>>::iterator it=xVol.begin();
	  while ( it != xVol.end() && xpos > (*it).first) {it++;} xVol.insert(it,std::make_pair(xpos,xh));
	} 
   }

    double xl = xVol[0].second;
    double xc = xVol[0].first;
    for ( auto xb : xVol) {
      if ( xb.first > xc && xb.first-xb.second < xc+xl) return nullptr;  //  overlap of sensitive volumes : not suitable for x-binned array
     xc = xb.first;  xl = xb.second;
    }
    
    // loop over children volumes; ( make sure they do not exceed enveloping
    // volume boundaries ?) split into connected subvolumes ( assume ordering
    // along X unless otherwise )
    std::vector<std::unique_ptr<Trk::Volume> > compVol;
    std::vector<std::string> compName;
    std::vector<const GeoVPhysVol*> compGeo;
    std::vector<Amg::Transform3D> compTransf;
    double halfZ = 0.;
    double halfX1 = 0.;
    double halfX2 = 0.;
    double halfY1 = 0.;
    double halfY2 = 0.;
    for (const GeoVolumeVec_t::value_type& p : geoGetVolumes(mv)) {
        const GeoVPhysVol* cv = p.first;
        const GeoLogVol* clv = cv->getLogVol();
        const Amg::Transform3D& transf = p.second;
        Trk::VolumeBounds* volBounds = nullptr;
        Trk::Volume* vol;
        if (clv->getShape()->type() == "Trd") {
            const GeoTrd* trd = dynamic_cast<const GeoTrd*>(clv->getShape());
            halfX1 = trd->getXHalfLength1();
            halfX2 = trd->getXHalfLength2();
            halfY1 = trd->getYHalfLength1();
            halfY2 = trd->getYHalfLength2();
            halfZ = trd->getZHalfLength();
            volBounds = new Trk::CuboidVolumeBounds(fmax(halfX1, halfX2), fmax(halfY1, halfY2), halfZ);
        } else if (clv->getShape()->type() == "Box") {
            const GeoBox* box = dynamic_cast<const GeoBox*>(clv->getShape());
            halfX1 = box->getXHalfLength();
            halfX2 = halfX1;
            halfY1 = box->getYHalfLength();
            halfY2 = halfY1;
            halfZ = box->getZHalfLength();
            volBounds = new Trk::CuboidVolumeBounds(halfX1, halfY1, halfZ);
        } else {
            double xSize = get_x_size(cv);
            ATH_MSG_VERBOSE("subvolume not box nor trapezoid, estimated x size:" << xSize);
            // printChildren(cv);
            volBounds = new Trk::CuboidVolumeBounds(xSize, envelope->halflengthY(), envelope->halflengthZ());
        }
        vol = new Trk::Volume(new Amg::Transform3D(transf), volBounds);
        ATH_MSG_VERBOSE("subvolume center:" << vol->center().x() << "," << vol->center().y() << "," << vol->center().z() <<":halfX:" << halfX1);
        std::string cname = clv->getName();
        const std::string &vname = mv->getLogVol()->getName();
        int nameSize = vname.size() - 8;
        if (cname.compare(0, nameSize, vname, 0, nameSize) == 0) cname = cname.substr(nameSize, cname.size() - nameSize);
        // order in X
        if (compVol.empty() || vol->center()[0] >= compVol.back()->center()[0]) {
	  compVol.push_back(std::make_unique<Trk::Volume>(*vol));
            compName.push_back(cname);
            compGeo.push_back(cv);
            compTransf.push_back(transf);
        } else {
	  std::vector<std::unique_ptr<Trk::Volume> >::iterator volIter = compVol.begin();
            std::vector<std::string>::iterator nameIter = compName.begin();
            std::vector<const GeoVPhysVol*>::iterator geoIter = compGeo.begin();
            std::vector<Amg::Transform3D>::iterator transfIter = compTransf.begin();
            while (vol->center()[0] >= (*volIter).get()->center()[0]) {
                ++volIter;
                ++nameIter;
                ++geoIter;
                ++transfIter;
            }
            compVol.insert(volIter, std::make_unique<Trk::Volume>(*vol));
            compName.insert(nameIter, cname);
            compGeo.insert(geoIter, cv);
            compTransf.insert(transfIter, transf);
        }
    }  // loop over components

   // define enveloping volumes for each "technology"
    std::vector<Trk::TrackingVolume*> trkVols;
    double envX = envelope->halflengthX();
    double envY = envelope->halflengthY();
    double envZ = envelope->halflengthZ();
    double currX = -envX;
    double maxX = envX;
    bool openSpacer = false;
    bool openRpc = false;
    std::vector<const GeoVPhysVol*> geoSpacer;
    std::vector<const GeoVPhysVol*> geoRpc;
    std::vector<Amg::Transform3D> transfSpacer;
    std::vector<Amg::Transform3D> transfRpc;
    double spacerlowXsize = 0;
    double spaceruppXsize = 0;
    double rpclowXsize = 0;
    double rpcuppXsize = 0;
    std::vector<float> volSteps;
    volSteps.push_back(-envX);
    for (unsigned i = 0; i < compVol.size(); i++) {
        bool comp_processed = false;
        const Trk::CuboidVolumeBounds* compBounds = dynamic_cast<const Trk::CuboidVolumeBounds*>(&(compVol[i]->volumeBounds()));
        // check return to comply with coverity
        if (!compBounds) {
            ATH_MSG_ERROR("box station component does not return cuboid shape");
            continue;
        }
        //
        double lowX = compVol[i].get()->center()[0] - compBounds->halflengthX();
        double uppX = compVol[i].get()->center()[0] + compBounds->halflengthX();

        if (lowX < currX)
            if (compName[i].compare("RPC28") != 0 && compName[i].compare("RPC29") != 0)  // exclude BIS RPCs from the check sice they overlap in x with other volumes
	       ATH_MSG_WARNING(" clash between components in volume:" << compName[i] << "current:" << currX
                                                                   << ": low edge of next volume:" << lowX);
        if (uppX > maxX) ATH_MSG_WARNING(" clash between component and envelope:" << compName[i] << "upper:" << uppX << ">" << maxX);

        // close Rpc if no further components
        if (openRpc && compName[i].compare(0, 3, "RPC") != 0 && compName[i].compare(0, 3, "Ded") != 0) {
            // low edge of current volume
	  double Xcurr = compVol[i].get()->center()[0] - compBounds->halflengthX();
            if (Xcurr >= currX + rpclowXsize + rpcuppXsize) {
                Trk::CuboidVolumeBounds* rpcBounds = new Trk::CuboidVolumeBounds(0.5 * (Xcurr - currX), envY, envZ);
		std::unique_ptr<Trk::Volume> rpcVol =
		  std::make_unique<Trk::Volume>(new Amg::Transform3D(Amg::Translation3D(currX + rpcBounds->halflengthX(), 0., 0.)), rpcBounds);
                Trk::TrackingVolume* rpcTrkVol = processRpc(rpcVol.get(), geoRpc, transfRpc, cache);
                trkVols.push_back(rpcTrkVol);
                volSteps.push_back(Xcurr);
                currX = Xcurr;
                openRpc = false;
            } else {
                ATH_MSG_WARNING("clash in Rpc definition!");
            }
        }
        // close spacer if no further components
        if (openSpacer && compName[i].compare(0, 1, "C") != 0 && compName[i].compare(0, 2, "LB") != 0) {
            // low edge of current volume
	  double Xcurr = compVol[i].get()->center()[0] - compBounds->halflengthX();
            if (Xcurr - currX - (spacerlowXsize + spaceruppXsize) >= -tolerance) {
                Trk::CuboidVolumeBounds* spacerBounds = new Trk::CuboidVolumeBounds(0.5 * (Xcurr - currX), envY, envZ);
                Trk::Volume spacerVol(new Amg::Transform3D(Amg::Translation3D(currX + spacerBounds->halflengthX(), 0., 0.)), spacerBounds);
                Trk::TrackingVolume* spacerTrkVol = processSpacer(spacerVol, geoSpacer, transfSpacer);
                trkVols.push_back(spacerTrkVol);
                volSteps.push_back(Xcurr);
                currX = Xcurr;
                openSpacer = false;
            } else {
                ATH_MSG_WARNING("clash in spacer definition!");
            }
        }
        if (compName[i].compare(0, 3, "RPC") == 0 || compName[i].compare(0, 3, "Ded") == 0) {
            if (!openRpc) {
                openRpc = true;
                geoRpc.clear();
                geoRpc.push_back(compGeo[i]);
                transfRpc.clear();
                transfRpc.push_back(compTransf[i]);
                // establish temporary volume size
                rpclowXsize = compVol[i].get()->center()[0] - currX;
                rpcuppXsize = compBounds->halflengthX();
                // check clash at low edge
                if (std::abs(rpclowXsize) < compBounds->halflengthX() - tolerance) ATH_MSG_WARNING("rpc low edge - not enough space");
            } else {
                geoRpc.push_back(compGeo[i]);
                transfRpc.push_back(compTransf[i]);
                // check temporary volume size
                if (std::abs(compVol[i].get()->center()[0] - currX) < compBounds->halflengthX() - tolerance)
                    ATH_MSG_WARNING("rpc low edge - not enough space");
                if (compVol[i].get()->center()[0] + compBounds->halflengthX() > currX + rpclowXsize + rpcuppXsize)
		  rpcuppXsize += (compVol[i].get()->center()[0] + compBounds->halflengthX()) - (currX + rpclowXsize + rpcuppXsize);
            }
            comp_processed = true;
        }
        if (compName[i].compare(0, 1, "C") == 0 || compName[i].compare(0, 2, "LB") == 0) {
            if (!openSpacer) {
                openSpacer = true;
                geoSpacer.clear();
                geoSpacer.push_back(compGeo[i]);
                transfSpacer.clear();
                transfSpacer.push_back(compTransf[i]);
                // establish temporary volume size
                spacerlowXsize = compVol[i].get()->center()[0] - currX;
                spaceruppXsize = compBounds->halflengthX();
                // check clash at low edge
                if (std::abs(spacerlowXsize) < compBounds->halflengthX() - tolerance) {
		  ATH_MSG_WARNING("spacer low edge - not enough space:current:center:halfSize:" << currX << "," << compVol[i].get()->center()[0]
                                                                                                  << "," << compBounds->halflengthX());
                }
            } else {
                geoSpacer.push_back(compGeo[i]);
                transfSpacer.push_back(compTransf[i]);
                // check temporary volume size
                if (std::abs(compVol[i].get()->center()[0] - currX) < compBounds->halflengthX() - tolerance) {
		  ATH_MSG_WARNING("spacer low edge - not enough space:current:center:halfSize:" << currX << "," << compVol[i].get()->center()[0]
                                                                                                  << "," << compBounds->halflengthX());
                }
                if (compVol[i].get()->center()[0] + compBounds->halflengthX() > currX + spacerlowXsize + spaceruppXsize)
		  spaceruppXsize += (compVol[i].get()->center()[0] + compBounds->halflengthX()) - (currX + spacerlowXsize + spaceruppXsize);
            }
            comp_processed = true;
        }
        if (compName[i].compare(0, 3, "MDT") == 0) {
	  std::unique_ptr<Trk::Volume> mdtVol;
	  Trk::CuboidVolumeBounds* mdtBounds = nullptr;
	  // remove z shift in transform !! bugfix !!
	  double zShift = compVol[i].get()->transform().translation()[2];
	  if (std::abs(zShift) > 0) { ATH_MSG_DEBUG("unusual z shift for subvolume:" << zShift); }
	  //                                 (HepGeom::TranslateZ3D(-zShift)*(*compTransf[i])).getTranslation()
	  //                                 <<std::endl;
	  if (lowX == currX) {
	    mdtBounds = new Trk::CuboidVolumeBounds(compBounds->halflengthX(), envY, envZ);
	    mdtVol = std::make_unique<Trk::Volume>(new Amg::Transform3D(Amg::Translation3D(0., 0., -zShift) * compVol[i].get()->transform()), mdtBounds);
	  } else {
	    if (std::abs(lowX - currX) > 0.002) {
	      ATH_MSG_DEBUG("Mdt volume size does not match the envelope:lowX,currX:" << lowX << "," << currX);
                    ATH_MSG_DEBUG("adjusting Mdt volume ");
                }
                mdtBounds = new Trk::CuboidVolumeBounds(compBounds->halflengthX() + 0.5 * (lowX - currX), envY, envZ);
                mdtVol = std::make_unique<Trk::Volume>(new Amg::Transform3D(Amg::Translation3D(0.5 * (currX - lowX), 0., 0.) *
									    Amg::Translation3D(0., 0., -zShift) * compVol[i].get()->transform()),
                                         mdtBounds);
            }
            double shiftSign = 1.;
            if (std::abs(zShift) > 0.) {
                const std::string &stName = mv->getLogVol()->getName();
                if (stName.compare(0, 4, "BIR3") == 0 || stName.compare(0, 4, "BIR5") == 0 || stName.compare(0, 4, "BIR7") == 0 ||
                    stName.compare(0, 5, "BIR10") == 0)
                    shiftSign = -1.;
            }
            Trk::TrackingVolume* mdtTrkVol =
	      processMdtBox(mdtVol.get(), compGeo[i], (std::make_unique< Amg::Transform3D>(Amg::Translation3D(0., 0., -zShift) * compTransf[i])).get(),
                              shiftSign * std::abs(zShift), cache);
            trkVols.push_back(mdtTrkVol);
            currX += 2. * mdtBounds->halflengthX();
            volSteps.push_back(currX);
            comp_processed = true;
            zShift = 0.;
        }
        if (!comp_processed) ATH_MSG_DEBUG( "unknown technology:" << compName[i] );
    }  // end loop over station children

    // there may be a spacer still open
    if (openSpacer) {
        if (maxX >= currX + spacerlowXsize + spaceruppXsize) {
            Trk::CuboidVolumeBounds* spacerBounds = new Trk::CuboidVolumeBounds(0.5 * (maxX - currX), envY, envZ);
            Trk::Volume spacerVol(new Amg::Transform3D(Amg::Translation3D(currX + spacerBounds->halflengthX(), 0., 0.)), spacerBounds);
            Trk::TrackingVolume* spacerTrkVol = processSpacer(spacerVol, geoSpacer, transfSpacer);
            trkVols.push_back(spacerTrkVol);
            currX = maxX;
            volSteps.push_back(currX);
            openSpacer = false;
        } else {
        }
    }
    // there may be an Rpc still open
    if (openRpc) {
        if (maxX >= currX + rpclowXsize + rpcuppXsize) {
            Trk::CuboidVolumeBounds* rpcBounds = new Trk::CuboidVolumeBounds(0.5 * (maxX - currX), envY, envZ);
	    std::unique_ptr<Trk::Volume> rpcVol = std::make_unique<Trk::Volume>
                (new Amg::Transform3D(Amg::Translation3D(currX + rpcBounds->halflengthX(), 0., 0.)), rpcBounds);
            Trk::TrackingVolume* rpcTrkVol = processRpc(rpcVol.get(), geoRpc, transfRpc, cache);
            trkVols.push_back(rpcTrkVol);
            currX = maxX;
            volSteps.push_back(currX);
            openRpc = false;
        } else {
	  ATH_MSG_WARNING( "clash in Rpc definition!(last volume)" );
        }
    }
    // create VolumeArray (1DX)
    Trk::TrackingVolumeArray* components = nullptr;
    if (m_trackingVolumeArrayCreator) {
        Trk::BinUtility* binUtility = new Trk::BinUtility(volSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
        components = m_trackingVolumeArrayCreator->cuboidVolumesArrayNav(trkVols, binUtility, false);
    }

    return components;
}

Trk::TrackingVolumeArray* Muon::MuonStationTypeBuilder::processTrdStationComponents(const GeoVPhysVol* mv,
                                                                                    Trk::TrapezoidVolumeBounds* envelope,
                                                                                    Cache& cache) const {
    ATH_MSG_DEBUG(name() << " processing station components for " << mv->getLogVol()->getName());
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    double tolerance = 0.0001;

    // loop over children volumes; ( make sure they do not exceed enveloping
    // volume boundaries ?) split into connected subvolumes ( assume ordering
    // along X unless otherwise )
    std::vector<std::unique_ptr<Trk::Volume> > compVol;
    std::vector<std::string> compName;
    std::vector<const GeoVPhysVol*> compGeo;
    std::vector<Amg::Transform3D> compTransf;
    double halfZ = 0.;
    double halfX1 = 0.;
    double halfX2 = 0.;
    double halfY1 = 0.;
    double halfY2 = 0.;
    for (const GeoVolumeVec_t::value_type& p : geoGetVolumes(mv)) {
        const GeoVPhysVol* cv = p.first;
        const GeoLogVol* clv = cv->getLogVol();
        Amg::Transform3D transf = p.second;
        // retrieve volumes for components
        Trk::VolumeBounds* volBounds = nullptr;
        Trk::Volume* vol;
        if (clv->getShape()->type() == "Trd") {
            const GeoTrd* trd = dynamic_cast<const GeoTrd*>(clv->getShape());
            halfX1 = trd->getXHalfLength1();
            halfX2 = trd->getXHalfLength2();
            halfY1 = trd->getYHalfLength1();
            halfY2 = trd->getYHalfLength2();
            halfZ = trd->getZHalfLength();
            if (halfX1 == halfX2 && halfY1 == halfY2)
                volBounds = new Trk::CuboidVolumeBounds(fmax(halfX1, halfX2), fmax(halfY1, halfY2), halfZ);
            if (halfX1 == halfX2 && halfY1 != halfY2) {
                transf *= Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 1., 0.)) * Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 0., 1.));
                volBounds = new Trk::TrapezoidVolumeBounds(halfY1, halfY2, halfZ, halfX1);
            }
            if (halfX1 != halfX2 && halfY1 == halfY2) { volBounds = new Trk::TrapezoidVolumeBounds(halfX1, halfX2, halfY1, halfZ); }
            if (!volBounds) ATH_MSG_WARNING( "volume shape for component not recognized" );
        } else if (clv->getShape()->type() == "Box") {
            const GeoBox* box = dynamic_cast<const GeoBox*>(clv->getShape());
            halfX1 = box->getXHalfLength();
            // halfX2 = halfX1; //neither halfX2 nor halfY2 are ever used after this
            // assignment
            halfY1 = box->getYHalfLength();
            // halfY2 = halfY1;
            halfZ = box->getZHalfLength();
            volBounds = new Trk::CuboidVolumeBounds(halfX1, halfY1, halfZ);
        } else {
            double xSize = get_x_size(cv);
            // printChildren(cv);
            if (clv->getName().compare(0, 1, "C") != 0 && clv->getName().compare(0, 2, "LB") != 0)
                transf *= Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 1., 0.)) * Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 0., 1.));
            volBounds =
                new Trk::TrapezoidVolumeBounds(envelope->minHalflengthX(), envelope->maxHalflengthX(), envelope->halflengthY(), xSize);
        }
        vol = new Trk::Volume(new Amg::Transform3D(transf), volBounds);
        std::string cname = clv->getName();
        std::string vname = mv->getLogVol()->getName();
        int nameSize = vname.size() - 8;
        if (cname.compare(0, nameSize, vname, 0, nameSize) == 0) cname = cname.substr(nameSize, cname.size() - nameSize);
        // order in X
        if (compVol.empty() || vol->center()[0] >= compVol.back()->center()[0]) {
	  compVol.push_back(std::make_unique<Trk::Volume>(*vol));
            compName.push_back(cname);
            compGeo.push_back(cv);
            compTransf.push_back(transf);
        } else {
	  std::vector<std::unique_ptr<Trk::Volume> >::iterator volIter = compVol.begin();
            std::vector<std::string>::iterator nameIter = compName.begin();
            std::vector<const GeoVPhysVol*>::iterator geoIter = compGeo.begin();
            std::vector<Amg::Transform3D>::iterator transfIter = compTransf.begin();
            while (vol->center()[0] >= (*volIter)->center()[0]) {
                ++volIter;
                ++nameIter;
                ++geoIter;
                ++transfIter;
            }
            compVol.insert(volIter, std::make_unique<Trk::Volume>(*vol));
            compName.insert(nameIter, cname);
            compGeo.insert(geoIter, cv);
            compTransf.insert(transfIter, transf);
        }
    }  // loop over components
    // define enveloping volumes for each "technology"
    std::vector<Trk::TrackingVolume*> trkVols;
    double envX1 = envelope->minHalflengthX();
    double envX2 = envelope->maxHalflengthX();
    double envY = envelope->halflengthY();
    double envZ = envelope->halflengthZ();
    //
    double currX = -envZ;
    double maxX = envZ;
    //
    bool openSpacer = false;
    std::vector<const GeoVPhysVol*> geoSpacer;
    std::vector<const GeoVPhysVol*> geoRpc;
    std::vector<Amg::Transform3D> transfSpacer;
    std::vector<Amg::Transform3D> transfRpc;
    double spacerlowXsize = 0;
    double spaceruppXsize = 0;
    double Xcurr = 0;
    double lowX = 0.;
    double uppX = 0.;
    std::vector<float> volSteps;
    volSteps.push_back(-envelope->halflengthZ());
    for (unsigned i = 0; i < compVol.size(); i++) {
        bool comp_processed = false;
        const Trk::CuboidVolumeBounds* compCubBounds = dynamic_cast<const Trk::CuboidVolumeBounds*>(&(compVol[i].get()->volumeBounds()));
        const Trk::TrapezoidVolumeBounds* compTrdBounds = dynamic_cast<const Trk::TrapezoidVolumeBounds*>(&(compVol[i].get()->volumeBounds()));
        if (compCubBounds) {
	  lowX = compVol[i].get()->center()[0] - compCubBounds->halflengthX();
	  uppX = compVol[i].get()->center()[0] + compCubBounds->halflengthX();
	  if (lowX < currX) ATH_MSG_WARNING( "Warning: we have a clash between components here!" );
	  if (uppX > maxX) ATH_MSG_WARNING( "Warning: we have a clash between component and envelope!" );
	  // low edge of current volume
	  Xcurr = compVol[i].get()->center()[0] - compCubBounds->halflengthX();
        }
        if (compTrdBounds) {
	  lowX = compVol[i].get()->center()[0] - compTrdBounds->halflengthZ();
	  uppX = compVol[i].get()->center()[0] + compTrdBounds->halflengthZ();
	  if (lowX < currX) ATH_MSG_WARNING( "Warning: we have a clash between components here!" );
	  if (uppX > maxX) ATH_MSG_WARNING( "Warning: we have a clash between component and envelope!" );
	  // low edge of current volume
	  Xcurr = compVol[i].get()->center()[0] - compTrdBounds->halflengthZ();
        }
        if (!compCubBounds && !compTrdBounds) {
	  ATH_MSG_WARNING( "unknown volume shape") ;
            return nullptr;
        }
        // close spacer if no further components
        if (openSpacer && compName[i].compare(0, 1, "C") !=0  && compName[i].compare(0, 2, "LB") != 0) {
            if (Xcurr - currX - (spacerlowXsize + spaceruppXsize) >= -tolerance) {
                Trk::TrapezoidVolumeBounds* spacerBounds = new Trk::TrapezoidVolumeBounds(envX1, envX2, envY, 0.5 * (Xcurr - currX));
                Amg::Transform3D tr(Amg::Translation3D(currX + spacerBounds->halflengthZ(), 0., 0.) *
                                    Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 1., 0.)) *
                                    Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 0., 1.)));

                Trk::Volume spacerVol(new Amg::Transform3D(tr), spacerBounds);
                Trk::TrackingVolume* spacerTrkVol = processSpacer(spacerVol, geoSpacer, transfSpacer);
                trkVols.push_back(spacerTrkVol);
                currX = Xcurr;
                volSteps.push_back(Xcurr);
                openSpacer = false;
            } else {
                 ATH_MSG_DEBUG( mv->getLogVol()->getName()<<" : clash in spacer definition " );
            }
        }
        if (compName[i].compare(0, 3, "RPC") == 0 || compName[i].compare(0, 3, "Ded") == 0) {
	  ATH_MSG_DEBUG( mv->getLogVol()->getName() <<": RPC components in endcaps? ");
        }
        if (compName[i].compare(0, 1, "C") == 0 || compName[i].compare(0, 2, "LB") == 0) {
            if (!openSpacer) {
                openSpacer = true;
                geoSpacer.clear();
                geoSpacer.push_back(compGeo[i]);
                transfSpacer.clear();
                transfSpacer.push_back(compTransf[i]);
                // establish temporary volume size
                spacerlowXsize = compVol[i].get()->center()[0] - currX;
                if (compCubBounds) {
                    spaceruppXsize = compCubBounds->halflengthX();
                    // check clash at low edge
                    if (spacerlowXsize < compCubBounds->halflengthX())
		      ATH_MSG_DEBUG( mv->getLogVol()->getName()<<",  spacer low edge - not enough space" );
                }
                if (compTrdBounds) {
                    spaceruppXsize = compTrdBounds->halflengthZ();
                    // check clash at low edge
                    if (spacerlowXsize < compTrdBounds->halflengthZ())
 		      ATH_MSG_DEBUG( mv->getLogVol()->getName()<<",  spacer low edge - not enough space" );
                }
            } else {
                geoSpacer.push_back(compGeo[i]);
                transfSpacer.push_back(compTransf[i]);
                // check temporary volume size
                if (compCubBounds) {
		  if (compVol[i].get()->center()[0] - currX < compCubBounds->halflengthX())
		      ATH_MSG_DEBUG( mv->getLogVol()->getName()<<",  spacer low edge - not enough space" );
		  if (compVol[i].get()->center()[0] + compCubBounds->halflengthX() > currX + spacerlowXsize + spaceruppXsize)
                        spaceruppXsize +=
			  (compVol[i].get()->center()[0] + compCubBounds->halflengthX()) - (currX + spacerlowXsize + spaceruppXsize);
                }
                if (compTrdBounds) {
		  if (compVol[i].get()->center()[0] - currX < compTrdBounds->halflengthZ())
 		      ATH_MSG_DEBUG( mv->getLogVol()->getName()<<",  spacer low edge - not enough space" );
		  if (compVol[i].get()->center()[0] + compTrdBounds->halflengthZ() > currX + spacerlowXsize + spaceruppXsize)
                        spaceruppXsize +=
			  (compVol[i].get()->center()[0] + compTrdBounds->halflengthZ()) - (currX + spacerlowXsize + spaceruppXsize);
                }
            }
            comp_processed = true;
        }
        if (compName[i].compare(0, 3, "MDT") == 0) {
	  std::unique_ptr<Trk::Volume> mdtVol = nullptr;
            Trk::TrapezoidVolumeBounds* mdtBounds = nullptr;
            if (lowX == currX) {
                mdtBounds = compTrdBounds ? new Trk::TrapezoidVolumeBounds(envX1, envX2, envY, compTrdBounds->halflengthZ())
                                          : new Trk::TrapezoidVolumeBounds(envX1, envX2, envY, compCubBounds->halflengthX());
                mdtVol = std::make_unique< Trk::Volume>(new Amg::Transform3D(compVol[i].get()->transform()), mdtBounds);
            } else {
                if (std::abs(lowX - currX) > 0.002) {
                    ATH_MSG_DEBUG("Mdt volume size does not match the envelope:lowX,currX:" << lowX << "," << currX);
                    ATH_MSG_DEBUG("adjusting Mdt volume ");
                }
                mdtBounds = compTrdBounds
                                ? new Trk::TrapezoidVolumeBounds(envX1, envX2, envY, compTrdBounds->halflengthZ() + 0.5 * (lowX - currX))
                                : new Trk::TrapezoidVolumeBounds(envX1, envX2, envY, compCubBounds->halflengthX() + 0.5 * (lowX - currX));
                mdtVol = std::make_unique< Trk::Volume>(new Amg::Transform3D(Amg::Translation3D(0., 0., 0.5 * (currX - lowX)) * compVol[i].get()->transform()),
                                         mdtBounds);
            }
            Trk::TrackingVolume* mdtTrkVol = processMdtTrd(mdtVol.get(), compGeo[i], &compTransf[i], cache);
            trkVols.push_back(mdtTrkVol);
            currX += 2. * mdtBounds->halflengthZ();
            volSteps.push_back(currX);
            comp_processed = true;
        }
        if (!comp_processed) ATH_MSG_DEBUG(mv->getLogVol()->getName()<< ", unknown technology:" << compName[i] );
    }  // end loop over station children

    // there may be a spacer still open
    if (openSpacer) {
        if (maxX >= currX + spacerlowXsize + spaceruppXsize) {
            Trk::TrapezoidVolumeBounds* spacerBounds = new Trk::TrapezoidVolumeBounds(envX1, envX2, envY, 0.5 * (maxX - currX));

            Trk::Volume spacerVol(new Amg::Transform3D(Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 1., 0.)) *
                                                       Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 0., 1.)) *
                                                       Amg::Translation3D(0., 0., currX + spacerBounds->halflengthZ())),
                                  spacerBounds);
            Trk::TrackingVolume* spacerTrkVol = processSpacer(spacerVol, geoSpacer, transfSpacer);
            trkVols.push_back(spacerTrkVol);
            currX = maxX;
            volSteps.push_back(currX);
            openSpacer = false;
        } else {
	  ATH_MSG_DEBUG( mv->getLogVol()->getName()<<",  clash in spacer definition (last volume)" );
        }
    }
    // create VolumeArray (1DX)
    Trk::TrackingVolumeArray* components = nullptr;
    // Trk::BinUtility* binUtility = new Trk::BinUtility1DX( -envelope->halflengthZ() ), volSteps);
    if (m_trackingVolumeArrayCreator) {
        Trk::BinUtility* binUtility = new Trk::BinUtility(volSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
        components = m_trackingVolumeArrayCreator->trapezoidVolumesArrayNav(trkVols, binUtility, false);
    }

    return components;
}

// finalize
StatusCode Muon::MuonStationTypeBuilder::finalize() {
    ATH_MSG_INFO(name() << " finalize() successful");
    return StatusCode::SUCCESS;
}
//
Trk::TrackingVolume* Muon::MuonStationTypeBuilder::processMdtBox(Trk::Volume* vol, const GeoVPhysVol*& gv, Amg::Transform3D* transf,
                                                                       double zShift, Cache& cache) const {
    std::vector<Trk::PlaneLayer*> layers;
    std::vector<double> x_array;
    std::vector<double> x_ref;
    std::vector<Trk::MaterialProperties*> x_mat;
    std::vector<double> x_thickness;
    std::vector<int> x_active;
    double currX = -100000;
    // here one could save time by not reading all tubes
    for (const GeoVolumeVec_t::value_type& p : geoGetVolumes(gv)) {
        const GeoVPhysVol* cv = p.first;
        const GeoLogVol* clv = cv->getLogVol();
        const Amg::Transform3D& transfc = p.second;
        // printChildren(cv);
        Trk::MaterialProperties* mdtMat = nullptr;
        double xv = 0.;
        int active = 0;
        if ((clv->getName()).compare(0, 3, "MDT") == 0) {
            xv = 13.0055;  // the half-thickness
            if (!cache.m_mdtTubeMat) {
                const GeoTube* tube = dynamic_cast<const GeoTube*>(clv->getShape());
                if (!tube) {
                    ATH_MSG_ERROR("tube component does not return tube shape");
                } else {
                    double volume = 8 * (tube->getRMax()) * (tube->getZHalfLength()) * xv;
                    cache.m_mdtTubeMat = std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(cv, volume, 2 * xv));
                }
            }
            mdtMat = cache.m_mdtTubeMat.get();
            active = 1;
        }
        if ((clv->getName()) == "MultiLayerFoam") {
            xv = decodeX(clv->getShape());
            for (auto & i : cache.m_mdtFoamMat) {
                if (std::abs(xv - 0.5 * i->thickness()) < 0.001) {
                    mdtMat = i.get();
                    break;
                }
            }
            if (!mdtMat) {
                const Trk::CuboidVolumeBounds* cub = dynamic_cast<const Trk::CuboidVolumeBounds*>(&(vol->volumeBounds()));
                if (!cub) {
                    ATH_MSG_ERROR("box station component does not return cuboid shape");
                } else {
                    double volume = 8 * (cub->halflengthY()) * (cub->halflengthZ()) * xv;
                    cache.m_mdtFoamMat.push_back(std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(cv, volume, 2 * xv)));
                }
                if (!cache.m_mdtFoamMat.empty()) mdtMat = cache.m_mdtFoamMat.back().get();
            }
        }
        if (transfc.translation()[0] != currX) {
            if (x_array.empty() || transfc.translation()[0] > x_array.back()) {
                x_array.push_back(transfc.translation()[0]);
                x_mat.push_back(mdtMat);
                x_thickness.push_back(2 * xv);
                x_active.push_back(active);
                currX = transfc.translation()[0];
                if (std::abs(transfc.translation()[1]) > 0.001) {
                    // code 2.corrdinate shift
                    double ref = transfc.translation()[2] + 1e5;
                    ref += int(1000 * transfc.translation()[1]) * 10e6;
                    x_ref.push_back(ref);
                } else {
                    x_ref.push_back(transfc.translation()[2]);
                }
            } else {
                std::vector<double>::iterator xIter = x_array.begin();
                std::vector<Trk::MaterialProperties*>::iterator mIter = x_mat.begin();
                std::vector<double>::iterator tIter = x_thickness.begin();
                std::vector<double>::iterator rIter = x_ref.begin();
                std::vector<int>::iterator aIter = x_active.begin();
                while (transfc.translation()[0] > *xIter) {
                    ++xIter;
                    ++mIter;
                    ++rIter;
                }
                x_array.insert(xIter, transfc.translation()[0]);
                x_mat.insert(mIter, mdtMat);
                x_thickness.insert(tIter, 2 * xv);
                x_active.insert(aIter, active);
                if (std::abs(transfc.translation()[1]) > 0.001) {
                    // code 2.corrdinate shift
                    double sign = (transfc.translation()[1] > 0.) ? 1. : -1.;
                    double ref = transfc.translation()[2] + sign * 1e5;
                    ref += int(1000 * transfc.translation()[1]) * 10e6;
                    x_ref.insert(rIter, ref);
                } else {
                    x_ref.insert(rIter, transfc.translation()[2]);
                }
                currX = transfc.translation()[0];
            }
        }
    }
    // create layers //
    Trk::PlaneLayer* layer;
    double thickness = 0.;
    std::unique_ptr<Trk::OverlapDescriptor> od = nullptr;
    const Trk::CuboidVolumeBounds* volBounds = dynamic_cast<const Trk::CuboidVolumeBounds*>(&(vol->volumeBounds()));
    float minX=0.0;
    if (volBounds) {
        double yv = volBounds->halflengthY();
        double zv = volBounds->halflengthZ();
        const auto bounds = std::make_shared<Trk::RectangleBounds>(yv, zv);
        for (unsigned int iloop = 0; iloop < x_array.size(); iloop++) {
            // x-y plane -> y-z plane
            thickness = x_thickness[iloop];
            Amg::Transform3D cTr((*transf) * Amg::Translation3D(x_array[iloop], 0., 0.) *
                                 Amg::AngleAxis3D(M_PI_2, Amg::Vector3D(0., 1., 0.)) * Amg::AngleAxis3D(M_PI_2, Amg::Vector3D(0., 0., 1.)));

            if (!x_mat[iloop]) ATH_MSG_WARNING("Undefined MDT layer material");
            Trk::MaterialProperties matLay = x_mat[iloop] ? *(x_mat[iloop]) : Trk::MaterialProperties(*m_muonMaterial, thickness);
            Trk::HomogeneousLayerMaterial mdtMaterial(matLay, 0.);
            layer = new Trk::PlaneLayer(cTr, bounds, mdtMaterial, thickness, std::move(od));
            layer->setRef(x_ref[iloop] - zShift);
            // make preliminary identification of active layers
            layer->setLayerType(x_active[iloop]);
            layers.push_back(layer);
       }
       // fix lower and upper bound of step vector to volume boundary
       minX = transf->translation()[0] - volBounds->halflengthX();
    }
    // create the BinnedArray
    std::vector<Trk::SharedObject<Trk::Layer>> layerOrder;
    std::vector<float> binSteps;
    // check if additional (navigation) layers needed
    
    binSteps.push_back(minX);
    if (!layers.empty()) {
        currX = minX;
        for (unsigned int i = 0; i < layers.size(); i++) {
            const Amg::Transform3D ltransf = layers[i]->transform();
            layerOrder.emplace_back(layers[i]);
            if (i < layers.size() - 1) {
                currX = ltransf.translation()[0] + 0.5 * layers[i]->thickness();
                binSteps.push_back(currX);
            }
        }
        binSteps.push_back(transf->translation()[0] + volBounds->halflengthX());
    }
    
    Trk::BinUtility* binUtility = new Trk::BinUtility(binSteps, Trk::BinningOption::open, Trk::BinningValue::binX);

    Trk::LayerArray* mdtLayerArray = nullptr;
    mdtLayerArray = new Trk::NavBinnedArray1D<Trk::Layer>(layerOrder, binUtility, new Amg::Transform3D(Trk::s_idTransform));
    std::string name = "MDT";
    Trk::TrackingVolume* mdt = new Trk::TrackingVolume(*vol, *m_muonMaterial, mdtLayerArray, nullptr, name);

    return mdt;
}
//
Trk::TrackingVolume* Muon::MuonStationTypeBuilder::processMdtTrd(Trk::Volume* vol, const GeoVPhysVol*& gv, Amg::Transform3D* transf,
                                                                       Cache& cache) const {
    std::vector<Trk::PlaneLayer*> layers;
    std::vector<double> x_array;
    std::vector<Trk::MaterialProperties*> x_mat;
    std::vector<double> x_thickness;
    std::vector<double> x_ref;
    std::vector<int> x_active;
    double currX = -100000;
    for (const GeoVolumeVec_t::value_type& p : geoGetVolumes(gv)) {
        const GeoVPhysVol* cv = p.first;
        const GeoLogVol* clv = cv->getLogVol();
        const Amg::Transform3D& transfc = p.second;
        double xv = 0.;
        int active = 0;
        if (clv->getShape()->type() == "Trd") {
            const GeoTrd* trd = dynamic_cast<const GeoTrd*>(clv->getShape());
            double x1v = trd->getXHalfLength1();
            double x2v = trd->getXHalfLength2();
            if (x1v == x2v) xv = x1v;
        }
        Trk::MaterialProperties* mdtMat = nullptr;
        if ((clv->getName()).compare(0, 3, "MDT") == 0) {
            xv = 13.0055;  // the half-thickness
            if (!cache.m_mdtTubeMat) {
                const GeoTube* tube = dynamic_cast<const GeoTube*>(clv->getShape());
                double volume = 8 * (tube->getRMax()) * (tube->getZHalfLength()) * xv;
                cache.m_mdtTubeMat = std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(cv, volume, 2 * xv));
            }
            mdtMat = cache.m_mdtTubeMat.get();
            active = 1;
        }
        if ((clv->getName()) == "MultiLayerFoam") {
            xv = decodeX(clv->getShape());
            for (auto & i : cache.m_mdtFoamMat) {
                if (std::abs(xv - 0.5 * i->thickness()) < 0.001) {
                    mdtMat = i.get();
                    break;
                }
            }
            if (!mdtMat) {
                const Trk::TrapezoidVolumeBounds* trd = dynamic_cast<const Trk::TrapezoidVolumeBounds*>(&(vol->volumeBounds()));
                // check return to comply with coverity
                if (!trd) {
                    ATH_MSG_ERROR("trd station component does not return trapezoid shape");
                } else {
                    double volume = 4 * (trd->minHalflengthX() + trd->maxHalflengthX()) * (trd->halflengthY()) * xv;
                    cache.m_mdtFoamMat.push_back(std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(cv, volume, 2 * xv)));
                }
                if (!cache.m_mdtFoamMat.empty()) mdtMat = cache.m_mdtFoamMat.back().get();
            }
        }

        if (transfc.translation()[0] != currX) {
            if (x_array.empty() || transfc.translation()[0] > x_array.back()) {
                x_array.push_back(transfc.translation()[0]);
                x_mat.push_back(mdtMat);
                x_thickness.push_back(2 * xv);
                x_ref.push_back(transfc.translation()[2]);
                currX = transfc.translation()[0];
                x_active.push_back(active);
            } else {
                std::vector<double>::iterator xIter = x_array.begin();
                std::vector<Trk::MaterialProperties*>::iterator mIter = x_mat.begin();
                std::vector<double>::iterator tIter = x_thickness.begin();
                std::vector<double>::iterator rIter = x_ref.begin();
                std::vector<int>::iterator aIter = x_active.begin();
                while (transfc.translation()[0] > *xIter) {
                    ++xIter;
                    ++mIter;
                    ++rIter;
                }
                x_array.insert(xIter, transfc.translation()[0]);
                x_mat.insert(mIter, mdtMat);
                x_thickness.insert(tIter, 2 * xv);
                x_ref.insert(rIter, transfc.translation()[2]);
                x_active.insert(aIter, active);
                currX = transfc.translation()[0];
            }
        }
    }
    // create layers //
    Trk::PlaneLayer* layer;
    double thickness = 0.;
    std::unique_ptr<Trk::OverlapDescriptor> od = nullptr;
    const Trk::TrapezoidVolumeBounds* volBounds = dynamic_cast<const Trk::TrapezoidVolumeBounds*>(&(vol->volumeBounds()));
    if (volBounds) {
        double x1v = volBounds->minHalflengthX();
        double x2v = volBounds->maxHalflengthX();
        double yv = volBounds->halflengthY();
        // x-y plane -> y-z plane
        auto bounds = std::make_shared<const Trk::TrapezoidBounds>(x1v, x2v, yv);
        for (unsigned int iloop = 0; iloop < x_array.size(); iloop++) {
            thickness = x_thickness[iloop];
            if (!x_mat[iloop]) ATH_MSG_WARNING("Undefined MDT layer material");
            Trk::MaterialProperties matLay = x_mat[iloop] ? *(x_mat[iloop]) : Trk::MaterialProperties(*m_muonMaterial, thickness);
            Trk::HomogeneousLayerMaterial mdtMaterial(matLay, 0.);
            Amg::Transform3D cTr((*transf) * Amg::Translation3D(0., 0., x_array[iloop]));
            layer = new Trk::PlaneLayer(cTr, bounds, mdtMaterial, thickness, std::move(od));
            // make preliminary identification of active layers
            layer->setLayerType(x_active[iloop]);
            layer->setRef(x_ref[iloop]);
            layers.push_back(layer);
       }

        // create the BinnedArray
        std::vector<Trk::SharedObject<Trk::Layer>> layerOrder;
        std::vector<float> binSteps;
        //
        double minX = transf->translation()[0] - volBounds->halflengthZ();
        binSteps.push_back(minX);
        if (!layers.empty()) {
            currX = minX;
            for (unsigned int i = 0; i < layers.size(); i++) {
                const Amg::Transform3D ltransf = layers[i]->transform();
                layerOrder.emplace_back(layers[i]);
                if (i < layers.size() - 1) {
                    currX = ltransf.translation()[0] + 0.5 * layers[i]->thickness();
                    binSteps.push_back(currX);
                }
            }
            binSteps.push_back(transf->translation()[0] + volBounds->halflengthZ());
        }
        Trk::BinUtility* binUtility = new Trk::BinUtility(binSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
        Trk::LayerArray* mdtLayerArray = nullptr;
        mdtLayerArray = new Trk::NavBinnedArray1D<Trk::Layer>(layerOrder, binUtility, new Amg::Transform3D(Trk::s_idTransform));
        std::string name = "MDT";
        Trk::TrackingVolume* mdt = new Trk::TrackingVolume(*vol, *m_muonMaterial, mdtLayerArray, nullptr, name);
        return mdt;
    }
    return nullptr;
}
Trk::TrackingVolume* Muon::MuonStationTypeBuilder::processRpc(Trk::Volume* vol, std::vector<const GeoVPhysVol*> gv,
                                                                    std::vector<Amg::Transform3D> transfc, Cache& cache) const {
    // layers correspond to DedModules and RpcModules; all substructures averaged
    // in material properties
    std::vector<Trk::Layer*> layers;
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
            const GeoBox* box = dynamic_cast<const GeoBox*>(shape);
            double xs = box->getXHalfLength();
            double ys = box->getYHalfLength();
            double zs = box->getZHalfLength();
            // translating into layer; x dimension defines thickness
            Trk::PlaneLayer* layer;
            double thickness = 2 * xs;
            std::unique_ptr<Trk::OverlapDescriptor> od = nullptr;
            auto bounds = std::make_shared<const Trk::RectangleBounds>(ys, zs);
            Amg::Transform3D cTr(transfc[ic] * Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 1., 0.)) *
                                 Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 0., 1.)));
            Trk::MaterialProperties rpcMat(0., 10.e10, 10.e10, 13., 26., 0.);  // default
            if ((glv->getName()).compare(0, 3, "Ded") == 0) {
                // find if material exists already
                bool found = false;
                for (auto & i : cache.m_rpcDed) {
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
                if (std::abs(thickness - 46.0) < 0.001) {
                    if (!cache.m_rpc46) {
                        double volc = 8 * xs * ys * zs;
                        cache.m_rpc46 = std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(gv[ic], volc, 2 * xs));
                    }
                    rpcMat = Trk::MaterialProperties(*cache.m_rpc46);
                } else {
                    ATH_MSG_WARNING(name() << "RPC module thickness different from 46:" << thickness);
                }
            }

            Trk::HomogeneousLayerMaterial rpcMaterial(rpcMat, 0.);
            layer = new Trk::PlaneLayer(cTr, bounds, rpcMaterial, thickness, std::move(od));
            layers.push_back(layer);
            // make preliminary identification of active layers
            if ((glv->getName()).compare(0, 3, "Ded") != 0) {
                layer->setLayerType(1);
            } else {
                layer->setLayerType(0);
            }
        } else if (shape && shape->type() == "Trd") {
            const GeoTrd* trd = dynamic_cast<const GeoTrd*>(shape);
            double xs1 = trd->getXHalfLength1();
            double xs2 = trd->getXHalfLength2();
            double ys1 = trd->getYHalfLength1();
            double ys2 = trd->getYHalfLength2();
            double zs = trd->getZHalfLength();
            // translating into layer; x dimension defines thickness
            if (xs1 == xs2 && ys1 == ys2) {
                Trk::PlaneLayer* layer;
                double thickness = 2 * xs1;
                std::unique_ptr<Trk::OverlapDescriptor> od = nullptr;
                auto bounds = std::make_shared<const Trk::RectangleBounds>(ys1, zs);
                Amg::Transform3D cTr(transfc[ic] * Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 1., 0.)) *
                                     Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 0., 1.)));
                Trk::MaterialProperties rpcMat(0., 10.e10, 10.e10, 13., 26., 0.);  // default
                if ((glv->getName()).compare(0, 3, "Ded") == 0) {
                    // find if material exists already
                    bool found = false;
                    for (auto & i : cache.m_rpcDed) {
                        if (std::abs(thickness - i->thickness()) < 0.001) {
                            rpcMat = Trk::MaterialProperties(*i);
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        double volc = 8 * xs1 * ys1 * zs;
                        cache.m_rpcDed.push_back(
                            std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(gv[ic], volc, 2 * xs1)));
                        rpcMat = Trk::MaterialProperties(*cache.m_rpcDed.back());
                    }
                    // create Ded layer
                    Trk::HomogeneousLayerMaterial rpcMaterial(rpcMat, 0.);
                    layer = new Trk::PlaneLayer(cTr, bounds, rpcMaterial, thickness, std::move(od));
                    layer->setLayerType(0);
                    layers.push_back(layer);
                } else {
                    // RPC layer; step one level below to resolve strip planes
                    // printChildren(gv[ic]);
                    unsigned int ngc = gv[ic]->getNChildVols();
                    for (unsigned int igc = 0; igc < ngc; igc++) {
                        Amg::Transform3D trgc(Trk::s_idTransform);
                        if (transfc[ic].rotation().isIdentity())
                            trgc = gv[ic]->getXToChildVol(igc);
                        else
                            trgc = Amg::AngleAxis3D(M_PI, Amg::Vector3D(0., 0., 1.)) * gv[ic]->getXToChildVol(igc);

                        const GeoVPhysVol* gcv = &(*(gv[ic]->getChildVol(igc)));
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
                                ATH_MSG_WARNING(name() << "unknown RPC panel:" << gx);
                            }
                            // create Rpc panel layers
                            thickness = 2 * gx;
                            Trk::HomogeneousLayerMaterial rpcMaterial(rpcMat, 0.);
                            layer = new Trk::PlaneLayer(Amg::Transform3D(Amg::Translation3D(trgc.translation()) * (cTr)), bounds,
                                                        rpcMaterial, thickness,  std::move(od));
                            layer->setLayerType(0);
                            layers.push_back(layer);
                        } else if ((gclv->getName()) == "Rpclayer") {
                            if (std::abs(gx - 6.85) > 0.001 && std::abs(gx - 5.9) > 0.001)  // two thicknesses allowed for 2/3 gaps RPCs
                                ATH_MSG_WARNING("processRpc() - unusual thickness of RPC (" << glv->getName() << ") layer :" << 2 * gx);
                            if (!cache.m_rpcLayer) {
                                double volc = 8 * gx * gy * gz;
                                // material allocated to two strip planes ( gas volume
                                // suppressed )
                                cache.m_rpcLayer = std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(gcv, volc, 2 * gx));
                            }
                            rpcMat = Trk::MaterialProperties(*cache.m_rpcLayer);
                            // define 1 layer for 2 strip planes
                            thickness = 2 * gx;
                            Trk::HomogeneousLayerMaterial rpcMaterial(rpcMat, 0.);
                            layer = new Trk::PlaneLayer(Amg::Transform3D(Amg::Translation3D(trgc.translation()) * (cTr)), bounds,
                                                        rpcMaterial, thickness, std::move(od));
                            layer->setLayerType(1);
                            layers.push_back(layer);
                        } else {
                            ATH_MSG_WARNING(name() << "unknown RPC component? " << gclv->getName());
                        }
                    }
                }
            } else {
                ATH_MSG_WARNING(name() << "RPC true trapezoid layer, not coded yet");
            }
        } else {
            ATH_MSG_WARNING(name() << "RPC layer shape not recognized");
        }
    }  // end loop over Modules

    std::vector<Trk::Layer*>* rpcLayers = new std::vector<Trk::Layer*>(layers);
    std::string name = "RPC";
    Trk::TrackingVolume* rpc = new Trk::TrackingVolume(*vol, *m_muonMaterial, rpcLayers, name);
    ATH_MSG_DEBUG(" Rpc component volume processed with" << layers.size() << " layers");
    return rpc;
}
//

Trk::TrackingVolume* Muon::MuonStationTypeBuilder::processSpacer(Trk::Volume& vol, std::vector<const GeoVPhysVol*> gv,
                                                                       std::vector<Amg::Transform3D> transf) const {
    // spacers: one level below, assumed boxes
    std::vector<Trk::Layer*> layers;
    // resolve child volumes
    // Don't use iterators; they'll be invalidated by the push_back's.
    size_t idx = 0;
    while (idx < gv.size()) {
        const GeoVPhysVol* vol = gv[idx];
        const Amg::Transform3D& tf = transf[idx];
        if (vol->getNChildVols()) {
            for (unsigned int ich = 0; ich < vol->getNChildVols(); ++ich) {
                gv.push_back(&(*(vol->getChildVol(ich))));
                transf.emplace_back(tf * vol->getXToChildVol(ich));
            }
            gv.erase(gv.begin() + idx);
            transf.erase (transf.begin() + idx);
        } else {
            ++idx;
        }
    }
    // translate into layers
    for (unsigned int ic = 0; ic < gv.size(); ++ic) {
        const GeoLogVol* clv = gv[ic]->getLogVol();
        Trk::Material cmat = m_materialConverter.convert(clv->getMaterial());
        ATH_MSG_VERBOSE(" spacer material all X0 " << cmat.X0 << " L0 " << cmat.L0 << " A " << cmat.A << " Z " << cmat.Z << " rho "
                                                   << cmat.rho);
        if (clv->getShape()->type() == "Box") {
            const GeoBox* box = dynamic_cast<const GeoBox*>(clv->getShape());
            double xs = box->getXHalfLength();
            double ys = box->getYHalfLength();
            double zs = box->getZHalfLength();
            // translating into layer; find minimal size
            Trk::PlaneLayer* layer;
            Trk::SharedObject<const Trk::SurfaceBounds> bounds = nullptr;
            double thickness = 0.;
            Amg::Transform3D cTr;
            if (zs <= xs && zs <= ys) {  // x-y plane
                bounds = std::make_shared<const Trk::RectangleBounds>(xs, ys);
                thickness = 2 * zs;
                cTr = Amg::Transform3D(transf[ic]);
            } else if (xs <= ys && xs <= zs) {  // x-y plane -> y-z plane
                bounds = std::make_shared<Trk::RectangleBounds>(ys, zs);
                thickness = 2 * xs;
                cTr = Amg::Transform3D(transf[ic] * Amg::AngleAxis3D(M_PI_2, Amg::Vector3D(0., 1., 0.)) *
                                       Amg::AngleAxis3D(M_PI_2, Amg::Vector3D(0., 0., 1.)));
            } else {  // x-y plane -> x-z plane
                bounds = std::make_shared<Trk::RectangleBounds>(xs, zs);
                thickness = 2 * ys;
                cTr = Amg::Transform3D(transf[ic] * Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(1., 0., 0.)));
            }
            Trk::MaterialProperties material(thickness, cmat.X0, cmat.L0, cmat.A, cmat.Z, cmat.rho);
            Trk::HomogeneousLayerMaterial spacerMaterial(material, 0.);
            layer = new Trk::PlaneLayer(cTr, bounds, spacerMaterial, thickness, nullptr, 0);
            layers.push_back(layer);
        } else if (clv->getShape()->type() == "Subtraction") {
            const GeoShapeSubtraction* sub = dynamic_cast<const GeoShapeSubtraction*>(clv->getShape());
            if (sub && sub->getOpA()->type() == "Box" && sub->getOpB()->type() == "Box") {
                // LB
                const GeoBox* boxA = dynamic_cast<const GeoBox*>(sub->getOpA());
                const GeoBox* boxB = dynamic_cast<const GeoBox*>(sub->getOpB());
                auto bounds = std::make_shared<const Trk::RectangleBounds>(boxA->getYHalfLength(), boxA->getZHalfLength());
                double thickness = (boxA->getXHalfLength() - boxB->getXHalfLength());
                double shift = 0.5 * (boxA->getXHalfLength() + boxB->getXHalfLength());
                Trk::MaterialProperties material(0., 10.e10, 10.e10, 13., 26., 0.);
                Trk::HomogeneousLayerMaterial spacerMaterial;
                if (thickness > 0.) {
                    material = Trk::MaterialProperties(thickness, cmat.X0, cmat.L0, cmat.A, cmat.Z, cmat.rho);
                    spacerMaterial = Trk::HomogeneousLayerMaterial(material, 0.);
                    Trk::PlaneLayer* layx = new Trk::PlaneLayer(Amg::Transform3D(transf[ic] * Amg::Translation3D(shift, 0., 0.) *
                                                                                 Amg::AngleAxis3D(M_PI_2, Amg::Vector3D(0., 1., 0.)) *
                                                                                 Amg::AngleAxis3D(M_PI_2, Amg::Vector3D(0., 0., 1.))),
                                                                bounds, spacerMaterial, thickness, nullptr, 0);
                    layers.push_back(layx);
                    Trk::SharedObject<const Trk::SurfaceBounds> bounds2(bounds);
                    Trk::PlaneLayer* layxx = new Trk::PlaneLayer(Amg::Transform3D(transf[ic] * Amg::Translation3D(-shift, 0., 0.) *
                                                                                  Amg::AngleAxis3D(M_PI_2, Amg::Vector3D(0., 1., 0.)) *
                                                                                  Amg::AngleAxis3D(M_PI_2, Amg::Vector3D(0., 0., 1.))),
                                                                 bounds2, spacerMaterial, thickness, nullptr, 0);
                    layers.push_back(layxx);
                }
                thickness = (boxA->getYHalfLength() - boxB->getYHalfLength());
                if (thickness > 0.) {
                    material = Trk::MaterialProperties(thickness, cmat.X0, cmat.L0, cmat.A, cmat.Z, cmat.rho);
                    spacerMaterial = Trk::HomogeneousLayerMaterial(material, 0.);
                    shift = 0.5 * (boxA->getYHalfLength() + boxB->getYHalfLength());
                    bounds = std::make_shared<const Trk::RectangleBounds>(boxB->getXHalfLength(), boxA->getZHalfLength());
                    Trk::PlaneLayer* lay = new Trk::PlaneLayer(Amg::Transform3D(transf[ic] * Amg::Translation3D(0., shift, 0.) *
                                                                                Amg::AngleAxis3D(M_PI_2, Amg::Vector3D(1., 0., 0.))),
                                                               bounds, spacerMaterial, thickness, nullptr, 0);
                    layers.push_back(lay);
                    Trk::SharedObject<const Trk::SurfaceBounds> bounds2(bounds);
                    Trk::PlaneLayer* layy = new Trk::PlaneLayer(Amg::Transform3D(transf[ic] * Amg::Translation3D(0., -shift, 0.) *
                                                                                 Amg::AngleAxis3D(M_PI_2, Amg::Vector3D(1., 0., 0.))),
                                                                bounds2, spacerMaterial, thickness, nullptr, 0);
                    layers.push_back(layy);
                }
                thickness = (boxA->getZHalfLength() - boxB->getZHalfLength());
                if (thickness > 0.) {
                    material = Trk::MaterialProperties(thickness, cmat.X0, cmat.L0, cmat.A, cmat.Z, cmat.rho);
                    spacerMaterial = Trk::HomogeneousLayerMaterial(material, 0.);
                    shift = 0.5 * (boxA->getZHalfLength() + boxB->getZHalfLength());
                    bounds = std::make_shared<const Trk::RectangleBounds>(boxB->getXHalfLength(), boxB->getYHalfLength());
                    Trk::PlaneLayer* layz = new Trk::PlaneLayer(Amg::Transform3D(transf[ic] * Amg::Translation3D(0., 0., shift)), bounds,
                                                                spacerMaterial, thickness, nullptr, 0);
                    layers.push_back(layz);
                    Trk::SharedObject<const Trk::SurfaceBounds> bounds2(bounds);
                    Trk::PlaneLayer* layzz = new Trk::PlaneLayer(Amg::Transform3D(transf[ic] * Amg::Translation3D(0., 0., -shift)), bounds2,
                                                                 spacerMaterial, thickness, nullptr, 0);
                    layers.push_back(layzz);
                }
            } else if (sub) {
                std::vector<std::pair<const GeoShape*, Amg::Transform3D>> subVs;
                const GeoShapeShift* shift = dynamic_cast<const GeoShapeShift*>(sub->getOpB());
                if (shift) subVs.emplace_back(shift->getOp(), shift->getX());
                const GeoShape* shape = sub->getOpA();
                while (shape->type() == "Subtraction") {
                    const GeoShapeSubtraction* subtr = dynamic_cast<const GeoShapeSubtraction*>(shape);
                    const GeoShapeShift* shift = dynamic_cast<const GeoShapeShift*>(subtr->getOpB());
                    if (shift) subVs.emplace_back(shift->getOp(), shift->getX());
                    shape = subtr->getOpA();
                }
                const GeoBox* box = dynamic_cast<const GeoBox*>(shape);
                if (box && subVs.size() == 4) {
                    Trk::Volume* v1 = nullptr;
                    Trk::Volume* v2 = nullptr;
                    Trk::VolumeExcluder* volExcl = nullptr;
                    const GeoBox* sb1 = dynamic_cast<const GeoBox*>(subVs[0].first);
                    if (sb1)
                        v1 = new Trk::Volume(
                            new Amg::Transform3D(subVs[0].second),
                            new Trk::CuboidVolumeBounds(sb1->getXHalfLength(), sb1->getYHalfLength(), sb1->getZHalfLength()));
                    const GeoBox* sb2 = dynamic_cast<const GeoBox*>(subVs[1].first);
                    if (sb2)
                        v2 = new Trk::Volume(
                            new Amg::Transform3D(subVs[1].second),
                            new Trk::CuboidVolumeBounds(sb2->getXHalfLength(), sb2->getYHalfLength(), sb2->getZHalfLength()));

                    const GeoBox* boxB = dynamic_cast<const GeoBox*>(subVs[2].first);
                    if (boxB && v1 && v2) {
                        auto bounds = std::make_shared<const Trk::RectangleBounds>(box->getYHalfLength(), box->getZHalfLength());
                        double thickness = (box->getXHalfLength() - boxB->getXHalfLength());
                        double shift = 0.5 * (box->getXHalfLength() + boxB->getXHalfLength());
                        Trk::Volume* cVol = new Trk::Volume(new Amg::Transform3D(Amg::Translation3D(-shift, 0., 0.)),
                                                            new Trk::CombinedVolumeBounds(v1, v2, false));
                        volExcl = new Trk::VolumeExcluder(cVol);
			std::unique_ptr<Trk::SubtractedPlaneSurface> subPlane =std::make_unique<Trk::SubtractedPlaneSurface>(
                            Trk::PlaneSurface(Amg::Transform3D(transf[ic] * Amg::Translation3D(shift, 0., 0.) *
                                                               Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 1., 0.)) *
                                                               Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 0., 1.))),
                                              bounds),  volExcl, false);
                        Trk::MaterialProperties material(thickness, cmat.X0, cmat.L0, cmat.A, cmat.Z, cmat.rho);
                        Trk::HomogeneousLayerMaterial spacerMaterial(material, 0.);
                        Trk::SubtractedPlaneLayer* layx = new Trk::SubtractedPlaneLayer(subPlane.get(), spacerMaterial, thickness, nullptr, 0);
                        layers.push_back(layx);
                        // Trk::SubtractedPlaneSurface* subPlaneX
                        //        = new
                        //        Trk::SubtractedPlaneSurface(*subPlane,Amg::Transform3D(Amg::Translation3D(-2*shift,0.,0.)));
                        std::unique_ptr<Trk::SubtractedPlaneSurface> subPlaneX(
                            new Trk::SubtractedPlaneSurface(*subPlane, Amg::Transform3D(Amg::Translation3D(-2 * shift, 0., 0.))));
                        Trk::SubtractedPlaneLayer* layxx = new Trk::SubtractedPlaneLayer(subPlaneX.get(), spacerMaterial, thickness, nullptr, 0);
                        layers.push_back(layxx);

                        bounds = std::make_shared<const Trk::RectangleBounds>(boxB->getXHalfLength(), box->getZHalfLength());
                        thickness = subVs[2].second.translation().mag();
                        Trk::VolumeExcluder* volEx =
                            new Trk::VolumeExcluder(new Trk::Volume(*cVol, Amg::Transform3D(Amg::Translation3D(2 * shift, 0., 0.))));
                        std::unique_ptr<Trk::SubtractedPlaneSurface> subPlaneBis =std::make_unique<Trk::SubtractedPlaneSurface>(
                            Trk::PlaneSurface(Amg::Transform3D(transf[ic] * Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(1., 0., 0.))),
                                              bounds), volEx, false);
                        material = Trk::MaterialProperties(thickness, cmat.X0, cmat.L0, cmat.A, cmat.Z, cmat.rho);
                        spacerMaterial = Trk::HomogeneousLayerMaterial(material, 0.);
                        Trk::SubtractedPlaneLayer* lay = new Trk::SubtractedPlaneLayer(subPlaneBis.get(), spacerMaterial, thickness, nullptr, 0);
                        layers.push_back(lay);
                    } else {
                        delete v1;
                        delete v2;
                    }
                }
            } else {
	      ATH_MSG_DEBUG( clv->getName() << ", unresolved spacer component " << clv->getName());
            }
        } else {
	  ATH_MSG_DEBUG( clv->getName() << ", unresolved spacer component " << clv->getName());
        }
    }

    std::vector<Trk::Layer*>::iterator lIt = layers.begin();
    for (; lIt != layers.end(); ++lIt)
        if ((*lIt)->thickness() < 0.) lIt = layers.erase(lIt);

    std::vector<Trk::Layer*>* spacerLayers = new std::vector<Trk::Layer*>(layers);
    std::string name = "Spacer";
    Trk::TrackingVolume* spacer =  new Trk::TrackingVolume(vol, *m_muonMaterial, spacerLayers, name);

    if (!m_resolveSpacer) {  // average into a single material layer
        ATH_MSG_VERBOSE(" !m_resolveSpacer createLayerRepresentation ");
        std::pair<Trk::Layer*, const std::vector<Trk::Layer*>*> laySpacer = createLayerRepresentation(spacer);
        delete spacer;
        laySpacer.first->setLayerType(0);
        layers.clear();
        layers.push_back(laySpacer.first);
        std::vector<Trk::Layer*>* spacerLays = new std::vector<Trk::Layer*>(layers);
        spacer = new Trk::TrackingVolume(vol, *m_muonMaterial, spacerLays, name);
    }
    
    return spacer;
}

Trk::TrackingVolume* Muon::MuonStationTypeBuilder::processNSW(const MuonGM::MuonDetectorManager* muonDetMgr,
                                                              const std::vector<Trk::Layer*>& layers) const {
    ATH_MSG_DEBUG(name() << " processing NSW station components " << layers.size());
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    // double tolerance = 0.0001;

    Trk::TrackingVolume* trVol = nullptr;

    Amg::Transform3D transf = layers[0]->surfaceRepresentation().transform();

    // loop over layers and retrieve boundaries
    double zMin{25000.}, zMax{-25000.}, rMin{13000.}, rMed{0.}, rMax{0.}, hMin{0.}, hMed{0.}, hMax{0.};

    for (auto *layer : layers) {
        zMin = fmin(zMin, (layer->surfaceRepresentation().center().z()) - 0.5 * layer->thickness());
        zMax = fmax(zMax, (layer->surfaceRepresentation().center().z()) + 0.5 * layer->thickness());

        const Trk::TrapezoidBounds* trdBounds = dynamic_cast<const Trk::TrapezoidBounds*>(&(layer->surfaceRepresentation().bounds()));

        if (trdBounds) {
            rMin = fmin(rMin, (layer->surfaceRepresentation().center().perp()) - trdBounds->halflengthY());
            rMax = fmax(rMax, (layer->surfaceRepresentation().center().perp()) + trdBounds->halflengthY());

            // hMin taken from MM, ring 0
            Identifier id(layer->layerType());
            if (muonDetMgr->mmIdHelper()->is_mm(id)) {
                if (std::abs(muonDetMgr->mmIdHelper()->stationEta(id)) == 1) hMin = trdBounds->minHalflengthX();
                // hMed taken from MM, ring 1
                if (std::abs(muonDetMgr->mmIdHelper()->stationEta(id)) == 2) {
                    hMed = trdBounds->minHalflengthX();
                    rMed = layer->surfaceRepresentation().center().perp() - trdBounds->halflengthY();
                }
                // hMax taken from MM, ring 3
                if (std::abs(muonDetMgr->mmIdHelper()->stationEta(id)) == 4) hMax = trdBounds->maxHalflengthX();
            }
        }
    }

    double c1 = 0;
    if ((rMed - rMin) != 0)
        c1 = (hMed - hMin) / (rMed - rMin);
    else
        ATH_MSG_WARNING("processNSW() - rMed=" << rMed << ", rMin=" << rMin << ", setting c1=0");
    double c2 = 0;
    if ((rMax - rMed) != 0)
        c2 = (hMax - hMed) / (rMax - rMed);
    else
        ATH_MSG_WARNING("processNSW() - rMax=" << rMax << ", rMed=" << rMed << ", setting c2=0");

    double r = 0.5 * (rMin + rMax);
    double z = 0.5 * (zMin + zMax);
    Amg::Vector3D center(r * cos(transf.translation().phi()), r * sin(transf.translation().phi()), z);
    Amg::Transform3D* cTr = new Amg::Transform3D(Amg::Translation3D(center - transf.translation()) * transf);

    if (std::abs(c1 - c2) > 0.1) {  // combined volume bounds needed but enlarge Trd instead (
                                    // otherwise layer representation not created )

        hMax = (c1 > c2 ? c1 : c2) * (rMax - rMin) + hMin;
    }

    Trk::TrapezoidVolumeBounds* trdVolBounds = new Trk::TrapezoidVolumeBounds(hMin, hMax, 0.5 * (rMax - rMin), 0.5 * std::abs(zMax - zMin));
    Trk::Volume envelope(cTr, trdVolBounds);

    std::vector<Trk::Layer*>* nswLayers = new std::vector<Trk::Layer*>(layers);
    std::string name = "NSW";
    trVol = new Trk::TrackingVolume(envelope, *m_muonMaterial, nswLayers, name);

    ATH_MSG_DEBUG(" NSW component volume processed with" << layers.size() << " layers");
    return trVol;
}

Trk::TrackingVolume* Muon::MuonStationTypeBuilder::processCscStation(const GeoVPhysVol* mv, const std::string& name, Cache& cache) const {
    // CSC stations have the particularity of displacement in Z between multilayer
    // and the spacer - the envelope
    //   has to be derived from the component volume shape and component
    //   displacement
    bool isDiamond = false;
    double xMin{0.}, xMed{0.}, xMax{0}, y1{0.}, y2{0}, z{0.};
    // printChildren(mv);
    // find the shape and dimensions for the first component
    const GeoVPhysVol* cv = &(*(mv->getChildVol(0)));
    const GeoLogVol* clv = cv->getLogVol();
    // Amg::Transform3D transform =
    // Amg::CLHEPTransformToEigen(mv->getXToChildVol(0));
    if (clv->getShape()->type() == "Shift") {
        const GeoShapeShift* shift = dynamic_cast<const GeoShapeShift*>(clv->getShape());
        if (shift->getOp()->type() == "Union") {
            // that would be the union making the diamond/double trapezoid shape,
            // let's retrieve the parameters
            isDiamond = true;
            const GeoShapeUnion* uni = dynamic_cast<const GeoShapeUnion*>(shift->getOp());
            if (uni->getOpA()->type() == "Trd") {
                const GeoTrd* trdA = dynamic_cast<const GeoTrd*>(uni->getOpA());
                xMin = trdA->getYHalfLength1();
                xMed = trdA->getYHalfLength2();
                y1 = trdA->getZHalfLength();
                z = trdA->getXHalfLength1();
            }
            if (uni->getOpB()->type() == "Shift") {
                const GeoShapeShift* sh = dynamic_cast<const GeoShapeShift*>(uni->getOpB());
                const GeoTrd* trdB = dynamic_cast<const GeoTrd*>(sh->getOp());
                if (trdB->getYHalfLength1() != xMed || trdB->getXHalfLength1() != z)
		  ATH_MSG_DEBUG(mv->getLogVol()->getName() << ": something is wrong: dimensions of 2 trapezoids do not match" );
                xMax = trdB->getYHalfLength2();
                y2 = trdB->getZHalfLength();
            }
        }  // end Union
        if (shift->getOp()->type() == "Trd") {
            // that would be the trapezoid shape, let's retrieve the parameters
            const GeoTrd* trd = dynamic_cast<const GeoTrd*>(shift->getOp());
            xMin = trd->getYHalfLength1();
            xMed = trd->getYHalfLength2();
            y1 = trd->getZHalfLength();
            z = trd->getXHalfLength1();
        }  // end Trd
    } else {
        if (clv->getShape()->type() == "Trd") {
            // that would be the trapezoid shape, let's retrieve the parameters
            const GeoTrd* trd = dynamic_cast<const GeoTrd*>(clv->getShape());
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
    for (const GeoVolumeVec_t::value_type& p : geoGetVolumes(mv)) {
        const GeoVPhysVol* cv = p.first;
        const GeoLogVol* clv = cv->getLogVol();
        const Amg::Transform3D& transform = p.second;
        unsigned int ich = compTransf.size();
        compTransf.push_back(transform);
        compName.push_back(clv->getName());
        compGeoVol.push_back(cv);
        if (clv->getShape()->type() == "Shift") {
            const GeoShapeShift* shift = dynamic_cast<const GeoShapeShift*>(clv->getShape());
            if (shift->getOp()->type() == "Union") {
                // that would be the union making the diamond/double trapezoid shape,
                // let's retrieve the parameters
                const GeoShapeUnion* uni = dynamic_cast<const GeoShapeUnion*>(shift->getOp());
                if (uni->getOpA()->type() == "Trd") {
                    const GeoTrd* trdA = dynamic_cast<const GeoTrd*>(uni->getOpA());
                    double xSize = trdA->getXHalfLength1();
                    if (!xSizes.empty())
                        xSizes.push_back((std::abs(transform.translation()[0] - compTransf[ich - 1].translation()[0]) - xSizes.back()));
                    else
                        xSizes.push_back(xSize);
                    double xpos = (transform * shift->getX()).translation()[0];
                    if (xpos - xSize < xmn) xmn = xpos - xSizes.back();
                    if (xpos + xSize > xmx) xmx = xpos + xSizes.back();
                }
            }  // end Union
        }      // end Shift
        if (clv->getShape()->type() == "Trd") {
            const GeoTrd* trd = dynamic_cast<const GeoTrd*>(clv->getShape());
            double xSize = trd->getXHalfLength1();
            if (!xSizes.empty())
                xSizes.push_back(std::abs(transform.translation()[0] - compTransf[ich - 1].translation()[0]) - xSizes.back());
            else
                xSizes.push_back(xSize);
            double xpos = transform.translation()[0];
            if (xpos - xSize < xmn) xmn = xpos - xSizes.back();
            if (xpos + xSize > xmx) xmx = xpos + xSizes.back();
        }  // end Trd
           // printChildren(cv);
    }
    // this should be enough to build station envelope
    double xTotal = 0;
    for (double xSize : xSizes) xTotal += xSize;
    double xShift = 0.5 * (xmx + xmn);
    double zShift = 0;
    zShift = std::abs(((compTransf.front()).translation())[2]) + std::abs(((compTransf.back()).translation())[2]);
    // calculate displacement with respect to GeoModel station volume
    // one way or the other, the station envelope is double trapezoid
    std::unique_ptr<Trk::Volume> envelope;
    double envXMed = xMed;
    double envY1 = y1;
    double envY2 = y2;
    std::vector<float> volSteps;
    volSteps.push_back(-xTotal + xShift);
    std::vector<Trk::TrackingVolume*> components;
    if (!isDiamond) {
        Trk::TrapezoidVolumeBounds* cscBounds = nullptr;
        Trk::TrapezoidVolumeBounds* compBounds = nullptr;
        xMax = xMed;
        y2 = 0.5 * zShift;
        cscBounds = new Trk::TrapezoidVolumeBounds(xMin, xMax, y1, xTotal);
        // xy -> yz  rotation
        // the center of Volume is shifted by y1-y2 in y
        Amg::Transform3D* cTr =
            new Amg::Transform3D(Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 1., 0.)) *
                                 Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 0., 1.)) * Amg::Translation3D(0., 0., xShift));
        envelope = std::make_unique<Trk::Volume>(cTr, cscBounds);
        // components
        double xCurr = -xTotal;
        for (unsigned int ic = 0; ic < xSizes.size(); ic++) {
            // component volumes follow the envelope dimension
            xCurr += xSizes[ic];
            Amg::Transform3D* compTr =
                new Amg::Transform3D(Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 1., 0.)) *
                                     Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 0., 1.)) * Amg::Translation3D(0., 0., xCurr + xShift));
            compBounds = new Trk::TrapezoidVolumeBounds(xMin, xMax, y1, xSizes[ic]);
            Trk::LayerArray* cscLayerArray = processCSCTrdComponent(compGeoVol[ic], compBounds, compTr, cache);
	    std::unique_ptr<Trk::Volume> compVol = std::make_unique<Trk::Volume>(compTr, compBounds);
            Trk::TrackingVolume* compTV = new Trk::TrackingVolume(*compVol, *m_muonMaterial, cscLayerArray, nullptr, compName[ic]);
            components.push_back(compTV);
            xCurr += xSizes[ic];
            volSteps.push_back(xCurr + xShift);
        }  // end components
    } else {
        Trk::DoubleTrapezoidVolumeBounds* cscBounds = nullptr;
        Trk::DoubleTrapezoidVolumeBounds* compBounds = nullptr;
        if (xMed != xMin && xMed != xMax) {
            envXMed += zShift / (y1 / (xMed - xMin) + y2 / (xMed - xMax));
            envY1 = y1 * (envXMed - xMin) / (xMed - xMin);
            envY2 = y2 * (envXMed - xMax) / (xMed - xMax);
        }
        cscBounds = new Trk::DoubleTrapezoidVolumeBounds(xMin, envXMed, xMax, envY1, envY2, xTotal);
        // xy -> yz  rotation
        // the center of DoubleTrapezoidVolume is shifted by (envY1-envY2) in y
        Amg::Transform3D* cTr = new Amg::Transform3D(Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 1., 0.)) *
                                                     Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 0., 1.)) *
                                                     Amg::Translation3D(0., envY1 - envY2, 0.) * Amg::Translation3D(0., 0., xShift));
        envelope = std::make_unique<Trk::Volume>(cTr, cscBounds);
        // components
        double xCurr = -xTotal;
        for (unsigned int ic = 0; ic < xSizes.size(); ic++) {
            // component volumes follow the envelope dimension
            xCurr += xSizes[ic];
            Amg::Transform3D* compTr = new Amg::Transform3D(
                Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 1., 0.)) * Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 0., 1.)) *
                Amg::Translation3D(0., envY1 - envY2, 0.) * Amg::Translation3D(0., 0., xCurr + xShift));
            compBounds = new Trk::DoubleTrapezoidVolumeBounds(xMin, envXMed, xMax, envY1, envY2, xSizes[ic]);
            Trk::LayerArray* cscLayerArray = processCSCDiamondComponent(compGeoVol[ic], compBounds, compTr, cache);
	    std::unique_ptr<Trk::Volume> compVol =std::make_unique<Trk::Volume>(compTr, compBounds);
            Trk::TrackingVolume* compTV = new Trk::TrackingVolume(*compVol, *m_muonMaterial, cscLayerArray, nullptr, compName[ic]);
            components.push_back(compTV);
            xCurr += xSizes[ic];
            volSteps.push_back(xCurr + xShift);
        }  // end components
    }

    // convert component volumes into array
    Trk::BinnedArray<Trk::TrackingVolume>* compArray = nullptr;
    if (!components.empty() && isDiamond) {
        if (m_trackingVolumeArrayCreator) {
            Trk::BinUtility* binUtil = new Trk::BinUtility(volSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
            compArray = m_trackingVolumeArrayCreator->doubleTrapezoidVolumesArrayNav(components, binUtil, false);
        }
    }
    if (!components.empty() && !isDiamond) {
        if (m_trackingVolumeArrayCreator) {
            Trk::BinUtility* binUtil = new Trk::BinUtility(volSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
            compArray = m_trackingVolumeArrayCreator->trapezoidVolumesArrayNav(components, binUtil, false);
        }
    }
    // ready to build the station prototype
    Trk::TrackingVolume* csc_station = new Trk::TrackingVolume(*envelope, *m_muonMaterial, nullptr, compArray, name);
    return csc_station;
}

Trk::TrackingVolume* Muon::MuonStationTypeBuilder::processTgcStation(const GeoVPhysVol* cv, Cache& cache) const {
 
    const GeoLogVol* clv = cv->getLogVol();
    const std::string& tgc_name = clv->getName();
    const GeoShape* baseShape = clv->getShape();
    if (baseShape->type() == "Subtraction") {
      const GeoShapeSubtraction* sub = dynamic_cast<const GeoShapeSubtraction*>(baseShape);
      if (sub) baseShape = sub->getOpA();
    }
    
    if (baseShape->type() == "Trd") {
      const GeoTrd* trd = dynamic_cast<const GeoTrd*>(baseShape);
      double x1 = trd->getXHalfLength1();
      double y1 = trd->getYHalfLength1();
      double y2 = trd->getYHalfLength2();
      double z = trd->getZHalfLength();
      // define envelope
      Trk::TrapezoidVolumeBounds* tgcBounds = new Trk::TrapezoidVolumeBounds(y1, y2, z, x1);
      // xy -> yz  rotation
      Amg::Transform3D* tTr = new Amg::Transform3D(Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 1., 0.)) *
						   Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 0., 1.)));
      std::unique_ptr<Trk::Volume> envelope = std::make_unique<Trk::Volume>(tTr, tgcBounds);
      Trk::LayerArray* tgcLayerArray = processTGCComponent(cv, tgcBounds, tTr, cache);
      // ready to build the station prototype
      Trk::TrackingVolume* tgc_station = new Trk::TrackingVolume(*envelope, *m_muonMaterial, tgcLayerArray, nullptr, tgc_name);
           
      printVolumeBounds("TGC envelope bounds:", *tgcBounds);
      printTransform("TGC prototype transform", tgc_station->transform());
      return tgc_station;
    } else {
      ATH_MSG_WARNING( tgc_name << ": TGC component not a trapezoid ?  no prototype built " );
    }
    return nullptr;
}

std::unique_ptr<Trk::DetachedTrackingVolume> Muon::MuonStationTypeBuilder::process_sTGC(const MuonGM::MuonDetectorManager* muonMgr,
											const MuonGM::sTgcReadoutElement* stgc,	Identifier nswId,
											const GeoVPhysVol* gv, Amg::Transform3D transf) const {

  std::string vName= gv->getLogVol()->getName();  
  ATH_MSG_DEBUG( "processing sTGC prototype of " <<vName);
  // printChildren(gv);
  
  std::unique_ptr<const Trk::Volume> envelope = std::unique_ptr<const Trk::Volume>(m_geoShapeConverter.translateGeoShape(gv->getLogVol()->getShape(), &transf));
  if (!envelope.get())  {
    ATH_MSG_WARNING("sTGC prototype for " << vName <<" not built ");
    return nullptr;
  }
  double thickness = envelopeThickness( envelope->volumeBounds());    // half thickness
  //printTransform("sTgc envelope trf", envelope->transform()); 
  //printVolumeBounds("sTgc envelope bounds", envelope->volumeBounds());
  
  // use envelope to define layer bounds
  Trk::SurfaceBounds*  layBounds = nullptr;
  layBounds = getLayerBoundsFromEnvelope( envelope.get() );
  // calculate layer area
  double layArea = 0.;
  if (layBounds) layArea = area( layBounds );  // TODO this should be a functionality of SurfaceBounds
  // use area to blend station material
  Trk::MaterialProperties sTgc_mat;
  m_volumeConverter.collectMaterial(gv,sTgc_mat, layArea);
  Trk::HomogeneousLayerMaterial stgcMaterial(sTgc_mat, 0.);
  float scale =1./ gv->getNChildVols();
  Trk::MaterialProperties sTgc_layerMat(sTgc_mat);
  sTgc_layerMat *= scale;    // divide station material between layers
  Trk::HomogeneousLayerMaterial stgcLayMaterial(sTgc_layerMat, 0.);   // TODO implement frame-like material binning and rotate to match readout geometry

  // loop over child volumes, check transforms / align with readout geometry
  Trk::SharedObject<const Trk::SurfaceBounds> bounds(layBounds);
  std::vector<Trk::PlaneLayer*> layers;
  unsigned int ic=0;
  for (const GeoVolumeVec_t::value_type& p : geoGetVolumes(gv)) {
    //const GeoVPhysVol* cv = p.first;
    const Amg::Transform3D& trc = p.second;
    Trk::PlaneLayer*  layer = new Trk::PlaneLayer(envelope.get()->transform()*trc, bounds, stgcLayMaterial, sTgc_layerMat.thickness());
    
    if (stgc) {
      Identifier id = muonMgr->stgcIdHelper()->channelID(muonMgr->stgcIdHelper()->stationName(nswId),
							 muonMgr->stgcIdHelper()->stationEta(nswId),
							 muonMgr->stgcIdHelper()->stationPhi(nswId),
							 muonMgr->stgcIdHelper()->multilayer(nswId),
							 muonMgr->stgcIdHelper()->gasGap(nswId)+ic, 2, 1); 
      //Amg::Transform3D layTransf = stgc->transform(id);
      //printTransform("subcomponent " , trc ); 
      //printTransform("layer  wire" , transf.inverse()*layTransf );
      unsigned int layType = id.get_identifier32().get_compact();
      layer->setLayerType(layType);
    }
    layers.push_back(layer);      
    ic++;
  }
  // create the BinnedArray
  std::vector<Trk::SharedObject<Trk::Layer>> layerOrder;
  std::vector<float> binSteps;
  binSteps.push_back(-thickness);
  for (unsigned int il=0; il< layers.size(); il++) {
    binSteps.push_back( binSteps.back()+sTgc_layerMat.thickness());
    layerOrder.push_back(Trk::SharedObject<Trk::Layer>(layers[il]));
  }
  if (binSteps.back()>thickness+1.e-3) ATH_MSG_WARNING("rescale stgc binning:" <<binSteps.back() << ">" <<thickness );
  binSteps.back()=thickness;
  Trk::BinUtility* binUtility = new Trk::BinUtility(binSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
  Trk::LayerArray* stgcLayerArray = new Trk::NavBinnedArray1D<Trk::Layer>(layerOrder, binUtility, new Amg::Transform3D(Trk::s_idTransform));
  // build tracking volume
  Trk::TrackingVolume* sTgc = new Trk::TrackingVolume(*envelope, *m_muonMaterial, stgcLayerArray, nullptr, vName);
  // create layer representation
  Trk::PlaneLayer*  layerRepr = new Trk::PlaneLayer(transf, bounds, stgcMaterial, sTgc_mat.thickness());
  // create prototype as detached tracking volume
  std::unique_ptr<Trk::DetachedTrackingVolume> stgcType{sTgc ? new Trk::DetachedTrackingVolume( vName, sTgc, layerRepr, nullptr) : nullptr};
  
  return stgcType;
}

std::unique_ptr<Trk::DetachedTrackingVolume> Muon::MuonStationTypeBuilder::process_MM(const MuonGM::MuonDetectorManager* muonMgr,
										      const MuonGM::MMReadoutElement* mm, Identifier nswId,
										      const GeoVPhysVol* gv, Amg::Transform3D transf) const {

  std::string vName = gv->getLogVol()->getName();
  
  ATH_MSG_DEBUG( "processing MM:" << vName << ":" << gv->getLogVol()->getShape()->type());
  //printChildren(gv); 

  std::unique_ptr<const Trk::Volume> envelope = std::unique_ptr<const Trk::Volume>(m_geoShapeConverter.translateGeoShape(gv->getLogVol()->getShape(), &transf));
  if (!envelope.get())  {
    ATH_MSG_WARNING("MM prototype for " << vName <<" not built ");
    return nullptr;
  }
  double thickness = envelopeThickness( envelope->volumeBounds());    // half thickness
  printTransform("MM envelope trf", envelope->transform()); 
  printVolumeBounds("MM envelope bounds", envelope->volumeBounds());
  
  // use envelope to define layer bounds
  Trk::SurfaceBounds*  layBounds = nullptr;
  layBounds = getLayerBoundsFromEnvelope( envelope.get());
  // calculate layer area
  double layArea = 0.;
  if (layBounds) layArea = area( layBounds );  // TODO this should be a functionality of SurfaceBounds
  // use area to blend station material
  Trk::MaterialProperties mm_mat;
  m_volumeConverter.collectMaterial(gv,mm_mat, layArea);
  Trk::HomogeneousLayerMaterial mmMaterial(mm_mat, 0.);  
  float scale =1./ gv->getNChildVols();
  Trk::MaterialProperties mm_layerMat(mm_mat);
  mm_layerMat *= scale;    // divide station material between layers
  Trk::HomogeneousLayerMaterial mmLayMaterial(mm_layerMat, 0.);   // TODO implement frame-like material binning and rotate to match readout geometry

  // loop over child volumes, check transforms / align with readout geometry
  Trk::SharedObject<const Trk::SurfaceBounds> bounds(layBounds);
  std::vector<Trk::PlaneLayer*> layers;
  unsigned int ic=0;
  for (const GeoVolumeVec_t::value_type& p : geoGetVolumes(gv)) {
    //const GeoVPhysVol* cv = p.first;
    const Amg::Transform3D& trc = p.second;
    Trk::PlaneLayer*  layer = new Trk::PlaneLayer(transf*trc, bounds, mmLayMaterial, mm_layerMat.thickness());
    
    if (mm) {
      Identifier id = muonMgr->mmIdHelper()->channelID(muonMgr->mmIdHelper()->stationName(nswId),
							 muonMgr->mmIdHelper()->stationEta(nswId),
							 muonMgr->mmIdHelper()->stationPhi(nswId),
							 muonMgr->mmIdHelper()->multilayer(nswId),
							 1+ic, 1); 
      //Amg::Transform3D layTransf = mm->transform(id);
      //printTransform("subcomponent " , trc ); 
      //printTransform("layer  mm" , transf.inverse()*layTransf );
      unsigned int layType = id.get_identifier32().get_compact();
      layer->setLayerType(layType);
    }
    layers.push_back(layer);      
    ic++;
  }

  // create the BinnedArray
  std::vector<Trk::SharedObject<Trk::Layer>> layerOrder;
  std::vector<float> binSteps;
  binSteps.push_back(-thickness);
  for (unsigned int il=0; il< layers.size(); il++) {
    binSteps.push_back( binSteps.back()+mm_layerMat.thickness());
    layerOrder.push_back(Trk::SharedObject<Trk::Layer>(layers[il]));
  }
  if (binSteps.back()>thickness+1.e-3) ATH_MSG_WARNING("rescale mm binning:" <<binSteps.back() << ">" <<thickness );
  binSteps.back()=thickness;
  Trk::BinUtility* binUtility = new Trk::BinUtility(binSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
  Trk::LayerArray* mmLayerArray = new Trk::NavBinnedArray1D<Trk::Layer>(layerOrder, binUtility, new Amg::Transform3D(Trk::s_idTransform));
  // build tracking volume
  Trk::TrackingVolume* mM = new Trk::TrackingVolume(*envelope, *m_muonMaterial, mmLayerArray, nullptr, vName); 
  // create layer representation
  Trk::PlaneLayer*  layerRepr = new Trk::PlaneLayer(transf, bounds, mmMaterial, mm_mat.thickness());
  // create prototype as detached tracking volume
  std::unique_ptr<Trk::DetachedTrackingVolume> mmType{mM ? new Trk::DetachedTrackingVolume( vName, mM, layerRepr, nullptr) : nullptr};
  
  return mmType;
}

void Muon::MuonStationTypeBuilder::printChildren(const GeoVPhysVol* pv, int level) const {
    // subcomponents
    int ic=0;
    for (const GeoVolumeVec_t::value_type& p : geoGetVolumes(pv)) {
        const GeoVPhysVol* cv = p.first;
        const GeoLogVol* clv = cv->getLogVol();
        const Amg::Transform3D& transf = p.second;
        ATH_MSG_DEBUG( level<<":");
        ATH_MSG_DEBUG( "subcomponent:" <<ic <<":"<< clv->getName() << ", made of" << clv->getMaterial()->getName() << "," << clv->getShape()->type()
		       << ":" << transf.translation().x() <<"," << transf.translation().y() <<"," <<transf.translation().z() );

        if (clv->getShape()->type() == "Shift") {
            const GeoShapeShift * shift = dynamic_cast<const GeoShapeShift*>(clv->getShape());
            //
	    if (shift->getOp()->type()=="SimplePolygonBrep") {
	      const GeoSimplePolygonBrep* spb = dynamic_cast<const GeoSimplePolygonBrep*> (shift->getOp());
	      unsigned int nv = spb->getNVertices();
	      std::vector<std::pair<double,double> > ivtx(nv);
	      for (unsigned int iv = 0; iv < nv; iv++) {
		ivtx[iv]=std::pair<double,double>(spb->getXVertex(iv),spb->getYVertex(iv));
                ATH_MSG_DEBUG( " SimplePolygonBrep  x " << spb->getXVertex(iv) << " y " << spb->getYVertex(iv) << " z " << spb->getDZ() );		
	      }   
	      //return new Trk::Volume(new Amg::Transform3D(*transf),new Trk::SimplePolygonBrepVolumeBounds(ivtx,spb->getDZ()));
	    } else {
	      ATH_MSG_DEBUG("geoshape type:" <<  shift->getOp()->type() );
	    }
            ATH_MSG_DEBUG( "shift:" << shift->getX().translation().x() << "," << shift->getX().translation().y()  << ","  << shift->getX().translation().z() );
	    ATH_MSG_DEBUG( ":rot:" << shift->getX().rotation()(0,0)<<"," << shift->getX().rotation()(0,1) <<"," << shift->getX().rotation()(0,2) );
	    ATH_MSG_DEBUG(  "       " << shift->getX().rotation()(1,0)<<"," << shift->getX().rotation()(1,1) <<"," << shift->getX().rotation()(1,2) );
	    ATH_MSG_DEBUG(  "       " << shift->getX().rotation()(2,0)<<"," << shift->getX().rotation()(2,1) <<"," << shift->getX().rotation()(2,2) );
            //
        }

        if (clv->getShape()->type() == "Trd") {
            const GeoTrd* trd = dynamic_cast<const GeoTrd*>(clv->getShape());
            //
            ATH_MSG_DEBUG(  "trd dimensions:" << trd->getXHalfLength1() << "," << trd->getXHalfLength2() << "," << trd->getYHalfLength1() << ","
			    << trd->getYHalfLength2() << "," << trd->getZHalfLength() );
            //
        }
        if (clv->getShape()->type() == "Box") {
            const GeoBox* box = dynamic_cast<const GeoBox*>(clv->getShape());
            //
            ATH_MSG_DEBUG(  " box dimensions:" << box->getXHalfLength() << "," << box->getYHalfLength() << "," << box->getZHalfLength() );
            //
        }

        printChildren(cv, level+1);
	ic++;
    }
}

double Muon::MuonStationTypeBuilder::get_x_size(const GeoVPhysVol* pv) const {
    double xlow = 0;
    double xup = 0;
    // subcomponents
    GeoVolumeVec_t vols = geoGetVolumes(pv);
    if (vols.empty()) {
        const GeoLogVol* clv = pv->getLogVol();
        double xh = 0;
        std::string type = clv->getShape()->type();
        if (type == "Trd") {
            const GeoTrd* trd = dynamic_cast<const GeoTrd*>(clv->getShape());
            xh = fmax(trd->getXHalfLength1(), trd->getXHalfLength2());
        }
        if (type == "Box") {
            const GeoBox* box = dynamic_cast<const GeoBox*>(clv->getShape());
            xh = box->getXHalfLength();
        }
        if (type == "Tube") {
            const GeoTube* tube = dynamic_cast<const GeoTube*>(clv->getShape());
            xh = tube->getRMax();
        }
        if (type == "Subtraction") { xh = decodeX(clv->getShape()); }

        return xh;
    }

    for (const GeoVolumeVec_t::value_type& p : vols) {
        const Amg::Transform3D& transf = p.second;
        const GeoVPhysVol* cv = p.first;
        const GeoLogVol* clv = cv->getLogVol();
        double xh = 0;
        std::string type = clv->getShape()->type();
        if (type == "Trd") {
            const GeoTrd* trd = dynamic_cast<const GeoTrd*>(clv->getShape());
            xh = fmax(trd->getXHalfLength1(), trd->getXHalfLength2());
        }
        if (type == "Box") {
            const GeoBox* box = dynamic_cast<const GeoBox*>(clv->getShape());
            xh = box->getXHalfLength();
        }
        if (type == "Tube") {
            const GeoTube* tube = dynamic_cast<const GeoTube*>(clv->getShape());
            xh = tube->getRMax();
        }
        if (type == "Subtraction") { xh = decodeX(clv->getShape()); }

        xlow = fmin(xlow, (transf.translation())[0] - xh);
        xup = fmax(xup, (transf.translation())[0] + xh);
    }

    return fmax(-xlow, xup);
}

Trk::MaterialProperties Muon::MuonStationTypeBuilder::getAveragedLayerMaterial(const GeoVPhysVol* pv, double volume,
                                                                               double thickness) const {
    ATH_MSG_DEBUG(name() << "::getAveragedLayerMaterial:processing ");
    // loop through the whole hierarchy; collect material
    Trk::MaterialProperties sumMat;
    // protect nan
    if (thickness > 0.) m_volumeConverter.collectMaterial(pv, sumMat, volume / thickness);

    ATH_MSG_VERBOSE(name() << " combined material thickness: " << sumMat.thickness());
    ATH_MSG_VERBOSE(name() << " actual layer thickness: " << thickness);

    // scale material properties to the actual layer thickness
    if (sumMat.thickness() != thickness && sumMat.thickness() > 0.) {
        double sf = thickness / sumMat.thickness();
        sumMat.material().X0 /= sf;
        sumMat.material().L0 /= sf;
        sumMat.material().rho *= sf;
        ATH_MSG_VERBOSE("averaged material scale :" << sf << " sumMat.material().X0() " << sumMat.material().X0 << " sumMat.material().L0 "
                                                    << sumMat.material().L0 << " sumMat.material().rho " << sumMat.material().rho
                                                    << " sumMat.material().x0() " << sumMat.material().x0());
        ATH_MSG_VERBOSE("averaged material:d,x0,dInX0:" << sumMat.thickness() << "," << sumMat.material().x0());
        return sumMat;
    }
    return sumMat;
}

Trk::LayerArray* Muon::MuonStationTypeBuilder::processCSCTrdComponent(const GeoVPhysVol*& pv, Trk::TrapezoidVolumeBounds*& compBounds,
                                                                      Amg::Transform3D*& transf, Cache& cache) const {
    // tolerance
    std::string name = pv->getLogVol()->getName();
    // printChildren(pv);
    std::vector<Trk::PlaneLayer*> layers;
    std::vector<double> x_array;
    std::vector<Trk::MaterialProperties> x_mat;
    std::vector<double> x_thickness;
    std::vector<int> x_active;
    double currX = -100000;
    // while waiting for better suggestion, define a single material layer
    Trk::MaterialProperties matCSC(0., 10.e10, 10.e10, 13., 26., 0.);
    double thickness = 2 * compBounds->halflengthZ();
    double minX = compBounds->minHalflengthX();
    double maxX = compBounds->maxHalflengthX();
    double halfY = compBounds->halflengthY();
    double halfZ = compBounds->halflengthZ();
    if (name.compare(name.size() - 5, 5, "CSC01") == 0) {
        if (!cache.m_matCSC01) {
            double vol = (minX + maxX) * 2 * halfY * thickness;
            cache.m_matCSC01 = std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(pv, vol, thickness));
        }
        matCSC = Trk::MaterialProperties(*cache.m_matCSC01);
        // retrieve number of gas gaps and their position -> turn them into active
        // layers step 1 level below
        const GeoVPhysVol* cv1 = &(*(pv->getChildVol(0)));
        for (const GeoVolumeVec_t::value_type& p : geoGetVolumes(cv1)) {
            const GeoVPhysVol* cv = p.first;
            const Amg::Transform3D& transfc = p.second;
            const GeoLogVol* clv = cv->getLogVol();
            if (clv->getName() == "CscArCO2") {
                double xl = transfc.translation()[0];
                if (x_array.empty() || xl >= x_array.back()) {
                    x_array.push_back(xl);
                } else {
                    unsigned int ix = 0;
                    while (ix < x_array.size() && x_array[ix] < xl) { ix++; }
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
            double xthick = 2 * fmin(x_array[0] + halfZ, halfZ - x_array[0]);
            double scale = xthick / thickness;
            Trk::MaterialProperties xmatCSC(xthick, scale * matCSC.x0(), scale * matCSC.l0(), matCSC.averageA(), matCSC.averageZ(),
                                            matCSC.averageRho() / scale);
            x_mat.push_back(xmatCSC);
            x_thickness.push_back(xthick);
            x_active.push_back(1);
        } else {
            double currX = -halfZ;
            for (unsigned int il = 0; il < x_array.size(); il++) {
                double xthick;
                if (il < x_array.size() - 1) {
                    xthick = 2 * fmin(x_array[il] - currX, 0.5 * (x_array[il + 1] - x_array[il]));
                } else {
                    xthick = 2 * fmin(x_array[il] - currX, halfZ - x_array[il]);
                }
                x_thickness.push_back(xthick);
                const Trk::MaterialProperties& xmatCSC(matCSC);
                x_mat.push_back(xmatCSC);
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
    Trk::PlaneLayer* layer = nullptr;
    std::unique_ptr<Trk::OverlapDescriptor> od = nullptr;
    Trk::TrapezoidBounds* tbounds = new Trk::TrapezoidBounds(minX, maxX, halfY);
    Trk::SharedObject<const Trk::SurfaceBounds> bounds(tbounds);
    for (unsigned int iloop = 0; iloop < x_array.size(); iloop++) {
        Amg::Transform3D cTr((*transf) * Amg::Translation3D(0., 0., x_array[iloop]));  // this won't work for multiple layers !!! //
        Trk::HomogeneousLayerMaterial cscMaterial(x_mat[iloop], 0.);
        layer = new Trk::PlaneLayer(cTr, bounds, cscMaterial, x_thickness[iloop], std::move(od));
        // make preliminary identification of active layers
        layer->setLayerType(x_active[iloop]);
        layers.push_back(layer);
    }

    // create the BinnedArray
    std::vector<Trk::SharedObject<Trk::Layer>> layerOrder;
    std::vector<float> binSteps;
    double xShift = transf->translation()[0];
    float lowX = -compBounds->halflengthZ() + xShift;
    binSteps.push_back(lowX);

    if (!layers.empty()) {
        currX = lowX - xShift;
        for (unsigned int i = 0; i < layers.size() - 1; i++) {
            const Amg::Transform3D ltransf(Amg::Translation3D(x_array[i], 0., 0.));
            layerOrder.push_back(Trk::SharedObject<Trk::Layer>(layers[i]));
            currX = ltransf.translation()[0] + 0.5 * layers[i]->thickness();
            binSteps.push_back(currX + xShift);
        }
        const Amg::Transform3D ltransf(Amg::Translation3D(x_array.back(), 0., 0.));
        layerOrder.push_back(Trk::SharedObject<Trk::Layer>(layers.back()));
        binSteps.push_back(compBounds->halflengthZ() + xShift);
    }
    // Trk::BinUtility* binUtility = new Trk::BinUtility1DX( lowX, new
    // std::vector<double>(binSteps));
    Trk::BinUtility* binUtility = new Trk::BinUtility(binSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
    Trk::LayerArray* cscLayerArray = nullptr;
    cscLayerArray = new Trk::NavBinnedArray1D<Trk::Layer>(layerOrder, binUtility, new Amg::Transform3D(Trk::s_idTransform));

    return cscLayerArray;
}

Trk::LayerArray* Muon::MuonStationTypeBuilder::processCSCDiamondComponent(const GeoVPhysVol*& pv,
                                                                          Trk::DoubleTrapezoidVolumeBounds*& compBounds,
                                                                          Amg::Transform3D*& transf, Cache& cache) const {
    // tolerance
    std::string name = pv->getLogVol()->getName();
    std::vector<Trk::PlaneLayer*> layers;
    std::vector<double> x_array;
    std::vector<Trk::MaterialProperties> x_mat;
    std::vector<double> x_thickness;
    std::vector<int> x_active;
    double currX = -100000;
    // while waiting for better suggestion, define a single material layer
    Trk::MaterialProperties matCSC(0., 10e8, 10e8, 13., 26., 0.);
    double thickness = 2 * compBounds->halflengthZ();
    double minX = compBounds->minHalflengthX();
    double medX = compBounds->medHalflengthX();
    double maxX = compBounds->maxHalflengthX();
    double halfY1 = compBounds->halflengthY1();
    double halfY2 = compBounds->halflengthY2();
    double halfZ = compBounds->halflengthZ();
    if (name.compare(name.size() - 5, 5, "CSC02") == 0) {
        if (!cache.m_matCSC02) {
            double vol = ((minX + medX) * 2 * halfY1 + (medX + maxX) * 2 * halfY2) * thickness;
            cache.m_matCSC02 = std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(pv, vol, thickness));
        }
        matCSC = Trk::MaterialProperties(*cache.m_matCSC02);
        // retrieve number of gas gaps and their position -> turn them into active
        // layers step 1 level below
        const GeoVPhysVol* cv1 = &(*(pv->getChildVol(0)));
        for (const GeoVolumeVec_t::value_type& p : geoGetVolumes(cv1)) {
            const GeoVPhysVol* cv = p.first;
            const Amg::Transform3D& transfc = p.second;
            const GeoLogVol* clv = cv->getLogVol();
            if (clv->getName() == "CscArCO2") {
                double xl = transfc.translation()[0];
                if (x_array.empty() || xl >= x_array.back()) {
                    x_array.push_back(xl);
                } else {
                    unsigned int ix = 0;
                    while (ix < x_array.size() && x_array[ix] < xl) { ix++; }
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
            x_thickness.push_back(2 * fmin(x_array[0] + halfZ, halfZ - x_array[0]));
            x_active.push_back(1);
        } else {
            double currX = -halfZ;
            for (unsigned int il = 0; il < x_array.size(); il++) {
                double xthick = 0.;
                if (il < x_array.size() - 1) {
                    xthick = 2 * fmin(x_array[il] - currX, 0.5 * (x_array[il + 1] - x_array[il]));
                    x_thickness.push_back(xthick);
                } else {
                    xthick = 2 * fmin(x_array[il] - currX, halfZ - x_array[il]);
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
            double vol = ((minX + medX) * 2 * halfY1 + (medX + maxX) * 2 * halfY2) * thickness;
            cache.m_matCSCspacer2 = std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(pv, vol, thickness));
        }
        matCSC = Trk::MaterialProperties(*cache.m_matCSCspacer2);
        x_array.push_back(0.);
        x_mat.push_back(matCSC);
        x_thickness.push_back(thickness);
        x_active.push_back(0);
    }
    // create layers
    Trk::PlaneLayer* layer = nullptr;
    std::unique_ptr<Trk::OverlapDescriptor> od = nullptr;
    Trk::DiamondBounds* dbounds = new Trk::DiamondBounds(minX, medX, maxX, halfY1, halfY2);
    ;
    Trk::SharedObject<const Trk::SurfaceBounds> bounds(dbounds);
    for (unsigned int iloop = 0; iloop < x_array.size(); iloop++) {
        Amg::Transform3D cTr((*transf) * Amg::Translation3D(0., 0., x_array[iloop]));  // this won't work for multiple layers !!! //
        Trk::HomogeneousLayerMaterial cscMaterial(x_mat[iloop], 0.);
        layer = new Trk::PlaneLayer(cTr, bounds, cscMaterial, x_thickness[iloop], std::move(od));
        layers.push_back(layer);
        // make preliminary identification of active layers
        layer->setLayerType(x_active[iloop]);
    }

    // create the BinnedArray
    std::vector<Trk::SharedObject<Trk::Layer>> layerOrder;
    std::vector<float> binSteps;
    double xShift = transf->translation()[0];
    double lowX = -compBounds->halflengthZ() + xShift;
    binSteps.push_back(lowX);

    if (!layers.empty()) {
        currX = lowX;
        for (unsigned int i = 0; i < layers.size() - 1; i++) {
            const Amg::Transform3D ltransf(Amg::Translation3D(x_array[i], 0., 0.));
            layerOrder.push_back(Trk::SharedObject<Trk::Layer>(layers[i]));
            currX = ltransf.translation()[0] + 0.5 * layers[i]->thickness() + xShift;
            binSteps.push_back(currX);
        }
        const Amg::Transform3D ltransf(Amg::Translation3D(x_array.back(), 0., 0.));
        layerOrder.push_back(Trk::SharedObject<Trk::Layer>(layers.back()));
        binSteps.push_back(compBounds->halflengthZ() + xShift);
    }
    // Trk::BinUtility* binUtility = new Trk::BinUtility1DX( lowX, new
    // std::vector<double>(binSteps));
    Trk::BinUtility* binUtility = new Trk::BinUtility(binSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
    Trk::LayerArray* cscLayerArray = nullptr;
    cscLayerArray = new Trk::NavBinnedArray1D<Trk::Layer>(layerOrder, binUtility, new Amg::Transform3D(Trk::s_idTransform));

    return cscLayerArray;
}

Trk::LayerArray* Muon::MuonStationTypeBuilder::processTGCComponent(const GeoVPhysVol*& pv, Trk::TrapezoidVolumeBounds*& tgcBounds,
                                                                   Amg::Transform3D*& transf, Cache& cache) const {
    // tolerance
    double tol = 0.001;
    std::string name = pv->getLogVol()->getName();
    std::vector<Trk::PlaneLayer*> layers;
    std::vector<double> x_array;
    std::vector<Trk::MaterialProperties> x_mat;
    std::vector<double> x_thickness;
    double currX = -100000;
    // while waiting for better suggestion, define a single material layer
    Trk::MaterialProperties matTGC(0., 10e8, 10e8, 13., 26., 0.);
    double minX = tgcBounds->minHalflengthX();
    double maxX = tgcBounds->maxHalflengthX();
    double halfY = tgcBounds->halflengthY();
    double halfZ = tgcBounds->halflengthZ();
    double thickness = 2 * halfZ;
    if (std::abs(tgcBounds->halflengthZ() - 35.00) < tol) {
        if (!cache.m_matTGC01) {
            double vol = (minX + maxX) * 2 * halfY * thickness;
            cache.m_matTGC01 = std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(pv, vol, thickness));
        }
        matTGC = Trk::MaterialProperties(*cache.m_matTGC01);
    } else if (std::abs(tgcBounds->halflengthZ() - 21.85) < tol) {
        if (!cache.m_matTGC06) {
            double vol = (minX + maxX) * 2 * halfY * thickness;
            cache.m_matTGC06 = std::make_unique<Trk::MaterialProperties>(getAveragedLayerMaterial(pv, vol, thickness));
        }
        matTGC = Trk::MaterialProperties(*cache.m_matTGC06);
    } else {
      ATH_MSG_DEBUG( "unknown TGC material:" << tgcBounds->halflengthZ() );
    }

    for (const GeoVolumeVec_t::value_type& p : geoGetVolumes(pv)) {
        const GeoVPhysVol* cv = p.first;
        const Amg::Transform3D& transfc = p.second;
        const GeoLogVol* clv = cv->getLogVol();
        if (clv->getName() == "muo::TGCGas") {
            double xl = transfc.translation()[0];
            if (x_array.empty() || xl >= x_array.back()) {
                x_array.push_back(xl);
            } else {
                unsigned int ix = 0;
                while (ix < x_array.size() && x_array[ix] < xl) { ix++; }
                x_array.insert(x_array.begin() + ix, xl);
            }
        }
    }
    double activeThick = 0.;
    if (x_array.empty()) {
        x_array.push_back(0.);
        x_thickness.push_back(thickness);
        activeThick = thickness;
    } else if (x_array.size() == 1) {
        x_thickness.push_back(2 * fmin(x_array[0] + halfZ, halfZ - x_array[0]));
        activeThick += x_thickness.back();
    } else {
        double currX = -halfZ;
        for (unsigned int il = 0; il < x_array.size(); il++) {
            if (il < x_array.size() - 1) {
                x_thickness.push_back(2 * fmin(x_array[il] - currX, 0.5 * (x_array[il + 1] - x_array[il])));
            } else {
                x_thickness.push_back(2 * fmin(x_array[il] - currX, halfZ - x_array[il]));
            }
            currX = x_array[il] + 0.5 * x_thickness.back();
            activeThick += x_thickness.back();
        }
    }
    // rescale material to match the combined thickness of active layers
    double scale = activeThick / thickness;
    matTGC = Trk::MaterialProperties(activeThick, scale * matTGC.x0(), scale * matTGC.l0(), matTGC.averageA(), matTGC.averageZ(),
                                     matTGC.averageRho() / scale);
    // create layers
    Trk::PlaneLayer* layer = nullptr;
    std::unique_ptr<Trk::OverlapDescriptor> od = nullptr;
    Trk::TrapezoidBounds* tbounds = new Trk::TrapezoidBounds(minX, maxX, halfY);
    ;
    Trk::SharedObject<const Trk::SurfaceBounds> bounds(tbounds);
    for (unsigned int iloop = 0; iloop < x_array.size(); iloop++) {
        Amg::Transform3D cTr(Amg::Translation3D(x_array[iloop], 0., 0.) * (*transf));  // this won't work for multiple layers !!! //
        Trk::HomogeneousLayerMaterial tgcMaterial(matTGC, 0.);
        layer = new Trk::PlaneLayer(cTr, bounds, tgcMaterial, x_thickness[iloop], std::move(od));
        // make preliminary identification of active layers
        layer->setLayerType(1);
        layers.push_back(layer);
    }
    // create the BinnedArray
    std::vector<Trk::SharedObject<Trk::Layer>> layerOrder;
    std::vector<float> binSteps;
    //
    float xShift = transf->translation()[0];
    float lowX = -halfZ + xShift;
    binSteps.push_back(lowX);
    if (!layers.empty()) {
        currX = lowX;
        for (unsigned int i = 0; i < layers.size() - 1; i++) {
            const Amg::Transform3D ltransf(Amg::Translation3D(x_array[i], 0., 0.));
            layerOrder.push_back(Trk::SharedObject<Trk::Layer>(layers[i]));
            currX = ltransf.translation()[0] + 0.5 * layers[i]->thickness() + xShift;
            binSteps.push_back(currX);
        }
        const Amg::Transform3D ltransf(Amg::Translation3D(x_array.back(), 0., 0.));
        layerOrder.push_back(Trk::SharedObject<Trk::Layer>(layers.back()));
        binSteps.push_back(halfZ + xShift);
    }
    Trk::BinUtility* binUtility = new Trk::BinUtility(binSteps, Trk::BinningOption::open, Trk::BinningValue::binX);
    Trk::LayerArray* tgcLayerArray = nullptr;
    tgcLayerArray = new Trk::NavBinnedArray1D<Trk::Layer>(layerOrder, binUtility, new Amg::Transform3D(Trk::s_idTransform));

    return tgcLayerArray;
}

double Muon::MuonStationTypeBuilder::decodeX(const GeoShape* sh) const {
    double xHalf = 0;

    const GeoTrd* trd = dynamic_cast<const GeoTrd*>(sh);
    const GeoBox* box = dynamic_cast<const GeoBox*>(sh);
    const GeoTube* tub = dynamic_cast<const GeoTube*>(sh);
    const GeoTubs* tubs = dynamic_cast<const GeoTubs*>(sh);
    const GeoShapeShift* shift = dynamic_cast<const GeoShapeShift*>(sh);
    const GeoShapeUnion* uni = dynamic_cast<const GeoShapeUnion*>(sh);
    const GeoShapeSubtraction* sub = dynamic_cast<const GeoShapeSubtraction*>(sh);
    const GeoSimplePolygonBrep* spb = dynamic_cast<const GeoSimplePolygonBrep*>(sh);

    if (!trd && !box && !tub && !tubs && !shift && !uni && !sub && !spb) {
        ATH_MSG_WARNING("decodeX(GeoShape=" << sh->type() << "): shape type " << sh->type() << " is unknown, returning xHalf=0");
        return xHalf;
    }

    if (spb) {
        for (unsigned int i = 0; i < spb->getNVertices(); i++) {
            ATH_MSG_DEBUG(" XVertex " << spb->getXVertex(i) << " YVertex " << spb->getYVertex(i));
            if (spb->getXVertex(i) > xHalf) xHalf = spb->getXVertex(i);
        }
        ATH_MSG_DEBUG(" GeoSimplePolygonBrep xHalf " << xHalf);
    }

    if (trd) xHalf = fmax(trd->getXHalfLength1(), trd->getXHalfLength2());
    if (box) xHalf = box->getXHalfLength();
    if (tub) xHalf = tub->getRMax();

    if (sub) {
        // be careful to handle properly GeoModel habit of subtracting large volumes
        // from smaller ones
        double xA = decodeX(sub->getOpA());
        xHalf = xA;
    }
    if (uni) {
        double xA = decodeX(uni->getOpA());
        double xB = decodeX(uni->getOpB());
        xHalf = fmax(xA, xB);
    }
    if (shift) {
        double xA = decodeX(shift->getOp());
        double xB = shift->getX().translation()[0];
        xHalf = xA + std::abs(xB);
    }

    return xHalf;
}

std::pair<Trk::Layer*, const std::vector<Trk::Layer*>*> Muon::MuonStationTypeBuilder::createLayerRepresentation(
    Trk::TrackingVolume* trVol) const {
    Trk::Layer* layRepr = nullptr;
    if (!trVol) return std::pair<Trk::Layer*, const std::vector<Trk::Layer*>*>(layRepr, 0);

    std::vector<Trk::Layer*>* multi = new std::vector<Trk::Layer*>;

    // retrieve volume envelope

    Trk::CuboidVolumeBounds* cubBounds = dynamic_cast<Trk::CuboidVolumeBounds*>(&(trVol->volumeBounds()));
    Trk::TrapezoidVolumeBounds* trdBounds = dynamic_cast<Trk::TrapezoidVolumeBounds*>(&(trVol->volumeBounds()));
    Trk::DoubleTrapezoidVolumeBounds* dtrdBounds = dynamic_cast<Trk::DoubleTrapezoidVolumeBounds*>(&(trVol->volumeBounds()));

    Amg::Transform3D subt = Trk::s_idTransform;

    Trk::SubtractedVolumeBounds* subBounds = dynamic_cast<Trk::SubtractedVolumeBounds*>(&(trVol->volumeBounds()));
    if (subBounds) {
        subt *= Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 1., 0.)) * Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 0., 1.));
        while (subBounds) {
            cubBounds = dynamic_cast<Trk::CuboidVolumeBounds*>(&(subBounds->outer()->volumeBounds()));
            trdBounds = dynamic_cast<Trk::TrapezoidVolumeBounds*>(&(subBounds->outer()->volumeBounds()));
            dtrdBounds = dynamic_cast<Trk::DoubleTrapezoidVolumeBounds*>(&(subBounds->outer()->volumeBounds()));
            subBounds = dynamic_cast<Trk::SubtractedVolumeBounds*>(&(subBounds->outer()->volumeBounds()));
        }
    }

    Trk::PlaneLayer* layer = nullptr;

    if (cubBounds) {
        double thickness = 2 * cubBounds->halflengthX();
        double sf = 4 * cubBounds->halflengthZ() * cubBounds->halflengthY();
        auto bounds = std::make_shared<Trk::RectangleBounds>(cubBounds->halflengthY(), cubBounds->halflengthZ());
        std::unique_ptr<Trk::OverlapDescriptor> od = nullptr;
        Trk::MaterialProperties matProp = collectStationMaterial(trVol, sf);
        ATH_MSG_VERBOSE(" collectStationMaterial cub " << matProp);
        if (matProp.thickness() > thickness) {
            ATH_MSG_DEBUG(" thickness of combined station material exceeds station size:" << trVol->volumeName());
        } else if (matProp.thickness() < thickness && matProp.thickness() > 0.) {
            // if (matProp.thickness()> 0.)  matProp *= thickness/matProp.thickness();
            double sf = thickness / matProp.thickness();
            // matProp.scale(sf);
            matProp = Trk::MaterialProperties(thickness, sf * matProp.x0(), sf * matProp.l0(), matProp.averageA(), matProp.averageZ(),
                                              matProp.averageRho() / sf);
        }
        Trk::HomogeneousLayerMaterial mat(matProp, 0.);
        layer = new Trk::PlaneLayer(Amg::Transform3D(trVol->transform() * Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 1., 0.)) *
                                                     Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 0., 1.))),
                                    bounds, mat, thickness, std::move(od), 1);
        // multilayers
        if (m_multilayerRepresentation && trVol->confinedVolumes()) {
            Trk::BinnedArraySpan<Trk::TrackingVolume * const> vols = trVol->confinedVolumes()->arrayObjects();
            if (vols.size() > 1) {
                for (auto *vol : vols) {
                    Trk::MaterialProperties matMulti = collectStationMaterial(vol, sf);
                    ATH_MSG_VERBOSE(" collectStationMaterial cub matMulti " << matMulti);
                    multi->push_back(new Trk::PlaneLayer(
                        Amg::Transform3D(vol->transform() * Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 1., 0.)) *
                                         Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 0., 1.))),
                        bounds, Trk::HomogeneousLayerMaterial(matMulti, 0.), matMulti.thickness(), std::move(od), 1));
                }
            }
        }
    } else if (trdBounds) {
        double thickness = 2 * trdBounds->halflengthZ();
        double sf = 2 * (trdBounds->minHalflengthX() + trdBounds->maxHalflengthX()) * trdBounds->halflengthY();
        const std::vector<const Trk::Surface*>* surfs = 
          trdBounds->decomposeToSurfaces(Trk::s_idTransform);
        const Trk::TrapezoidBounds* tbounds = dynamic_cast<const Trk::TrapezoidBounds*>(&(*(surfs))[0]->bounds());
        Trk::SharedObject<const Trk::SurfaceBounds> bounds(new Trk::TrapezoidBounds(*tbounds));
        std::unique_ptr<Trk::OverlapDescriptor> od = nullptr;
        Trk::MaterialProperties matProp = collectStationMaterial(trVol, sf);
        ATH_MSG_VERBOSE(" collectStationMaterial trd " << matProp << trVol->volumeName());
        if (matProp.thickness() > thickness) {
            ATH_MSG_DEBUG(" thickness of combined station material exceeds station size:" << trVol->volumeName());
        } else if (matProp.thickness() < thickness && matProp.thickness() > 0.) {
            float sf = thickness / matProp.thickness();
            matProp = Trk::MaterialProperties(thickness, sf * matProp.x0(), sf * matProp.l0(), matProp.averageA(), matProp.averageZ(),
                                              matProp.averageRho() / sf);
        }
        Trk::HomogeneousLayerMaterial mat(matProp, 0.);
        layer = new Trk::PlaneLayer(Amg::Transform3D(subt * trVol->transform()), bounds, mat, thickness, std::move(od), 1);
        for (const auto *surf : *surfs) delete surf;
        delete surfs;
        // multilayers
        if (m_multilayerRepresentation && trVol->confinedVolumes()) {
          Trk::BinnedArraySpan<Trk::TrackingVolume * const> vols = trVol->confinedVolumes()->arrayObjects();
            if (vols.size() > 1) {
                for (auto *vol : vols) {
                    Trk::MaterialProperties matMulti = collectStationMaterial(vol, sf);
                    ATH_MSG_VERBOSE(" collectStationMaterial trd matMulti  " << matMulti);
                    multi->push_back(new Trk::PlaneLayer(Amg::Transform3D(vol->transform()), bounds,
                                                         Trk::HomogeneousLayerMaterial(matMulti, 0.), matMulti.thickness(), std::move(od), 1));
                }
            }
        }
    } else if (dtrdBounds) {
        double thickness = 2 * dtrdBounds->halflengthZ();
        double sf = 2 * (dtrdBounds->minHalflengthX() + dtrdBounds->medHalflengthX()) * dtrdBounds->halflengthY1() +
                    2 * (dtrdBounds->medHalflengthX() + dtrdBounds->maxHalflengthX()) * dtrdBounds->halflengthY2();
        const std::vector<const Trk::Surface*>* surfs = 
          dtrdBounds->decomposeToSurfaces(Trk::s_idTransform);
        const Trk::DiamondBounds* dbounds = dynamic_cast<const Trk::DiamondBounds*>(&(*(surfs))[0]->bounds());
        Trk::SharedObject<const Trk::SurfaceBounds> bounds(new Trk::DiamondBounds(*dbounds));
        std::unique_ptr<Trk::OverlapDescriptor> od = nullptr;
        Trk::MaterialProperties matProp = collectStationMaterial(trVol, sf);
        ATH_MSG_VERBOSE(" collectStationMaterial dtrd  " << matProp);
        if (matProp.thickness() > thickness) {
            ATH_MSG_DEBUG(" thickness of combined station material exceeds station size:" << trVol->volumeName());
        } else if (matProp.thickness() < thickness && matProp.thickness() > 0.) {
            float sf = thickness / matProp.thickness();
            matProp = Trk::MaterialProperties(thickness, sf * matProp.x0(), sf * matProp.l0(), matProp.averageA(), matProp.averageZ(),
                                              matProp.averageRho() / sf);
        }
        Trk::HomogeneousLayerMaterial mat(matProp, 0.);
        layer = new Trk::PlaneLayer(Amg::Transform3D(trVol->transform()), bounds, mat, thickness, std::move(od), 1);
        for (const auto *surf : *surfs) delete surf;
        delete surfs;
        // multilayers
        if (m_multilayerRepresentation && trVol->confinedVolumes()) {
          Trk::BinnedArraySpan<Trk::TrackingVolume * const> vols = trVol->confinedVolumes()->arrayObjects();
            if (vols.size() > 1) {
                for (auto *vol : vols) {
                    Trk::MaterialProperties matMulti = collectStationMaterial(vol, sf);
                    ATH_MSG_VERBOSE(" collectStationMaterial dtrd matMulti  " << matMulti);
                    multi->push_back(new Trk::PlaneLayer(Amg::Transform3D(vol->transform()), bounds,
                                                         Trk::HomogeneousLayerMaterial(matMulti, 0.), matMulti.thickness(), std::move(od), 1));
                }
            }
        }
    }

    layRepr = layer;

    if (multi->empty()) {
        delete multi;
        multi = nullptr;
    }
    return std::pair<Trk::Layer*, const std::vector<Trk::Layer*>*>(layRepr, multi);
}

Identifier Muon::MuonStationTypeBuilder::identifyNSW(const MuonGM::MuonDetectorManager* muonDetMgr, const std::string& vName,
                                                     const Amg::Transform3D& transf) {
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
        if (transf.translation().z() < 0.) iEta *= -1;
        // station Phi
        unsigned int iPhi = 1;
        // if (trVol->center().z()>0.) iPhi += 8;
        // station multilayer
        std::istringstream istm(&vName[3]);
        int iMult;
        istm >> iMult;
        if (vName[0] == 'Q' && vName[3] == 'P') iMult = (vName[1] == 'L') ? 1 : 2;
	if (vName[0] == 'Q' && vName[3] == 'C') iMult = (vName[1] == 'L') ? 2 : 1;
        // layer
        std::string stl(&vName[vName.size() - 1]);
        std::istringstream istl(stl);
        int iLay;
        istl >> iLay;
        iLay += 1;
        if (vName[0] == 'Q') {
            std::string stName = (vName[1] == 'L') ? "STL" : "STS";
            // int stId = (vName[2]=='L') ? 0 : 1;
            id = muonDetMgr->stgcIdHelper()->channelID(stName, iEta, iPhi, iMult, iLay, 2, 1);   // wire position 
        } else {
            std::string stName = (vName[2] == 'L') ? "MML" : "MMS";
            // int stId = (vName[2]=='L') ? 0 : 1;
            id = muonDetMgr->mmIdHelper()->channelID(stName, iEta, iPhi, iMult, iLay, 1);
        }
    }

    return id;
}

Trk::Layer* Muon::MuonStationTypeBuilder::createLayer(const MuonGM::MuonDetectorManager* muonDetMgr, Trk::TrackingVolume* trVol,
                                                      Trk::MaterialProperties* matEx, Amg::Transform3D& transf) const {
    // identification first

    std::string vName = trVol->volumeName().substr(trVol->volumeName().find('-') + 1);

    const Trk::RotatedTrapezoidBounds* rtrd = nullptr;
    const Trk::TrapezoidBounds* trd = nullptr;
    Amg::Vector3D mrg_pos = transf.translation();

    unsigned int layType = 0;

    if ((vName[0] == 'Q') || (vName[0] == 'M')) {  // NSW stations
        // station eta
        std::istringstream istr(&vName[1]);
        int iEta;
        if (vName[0] == 'Q') {
            std::istringstream istr2(&vName[2]);
            istr2 >> iEta;
        } else
            istr >> iEta;
        if (transf.translation().z() < 0.) iEta *= -1;
        // station Phi
        unsigned int iPhi = 1;
        // station multilayer
        std::istringstream istm(&vName[3]);
        unsigned int iMult;
        istm >> iMult;
        if (vName[0] == 'Q' && vName[3] == 'P') iMult = 1;
        if (vName[0] == 'Q' && vName[3] == 'C') iMult = 2;
        // layer
        std::string stl(&vName[vName.size() - 1]);
        std::istringstream istl(stl);
        unsigned int iLay;
        istl >> iLay;
        iLay += 1;
        if (vName[0] == 'Q') {  // vName looks like QL3P...
            // Alexandre Laurier: for stName,  used to be vName[2] which would give
            // 1,2,3 so stName would always be STS for stId, vName[2] was used, always
            // giving a 0 value
            std::string stName = (vName[1] == 'L') ? "STL" : "STS";
            Identifier id = muonDetMgr->stgcIdHelper()->channelID(stName, iEta, iPhi, iMult, iLay, 2, 1);    // wire position
            const MuonGM::sTgcReadoutElement* stgc = muonDetMgr->getsTgcReadoutElement(id);
            layType = id.get_identifier32().get_compact();
            if (stgc) {
                rtrd = dynamic_cast<const Trk::RotatedTrapezoidBounds*>(&stgc->bounds(id));
                trd = dynamic_cast<const Trk::TrapezoidBounds*>(&stgc->bounds(id));
                mrg_pos = stgc->center(id);
            }
        } else {
            std::string stName = (vName[2] == 'L') ? "MML" : "MMS";
            Identifier id = muonDetMgr->mmIdHelper()->channelID(stName, iEta, iPhi, iMult, iLay, 1);
            const MuonGM::MMReadoutElement* mm = muonDetMgr->getMMReadoutElement(id);
            layType = id.get_identifier32().get_compact();
            if (mm) {
                rtrd = dynamic_cast<const Trk::RotatedTrapezoidBounds*>(&mm->bounds(id));
                mrg_pos = mm->center(id);
            }
        }
    }

    Trk::Layer* layRepr = nullptr;

    // retrieve volume envelope

    Trk::CuboidVolumeBounds* cubBounds = dynamic_cast<Trk::CuboidVolumeBounds*>(&(trVol->volumeBounds()));
    Trk::TrapezoidVolumeBounds* trdBounds = dynamic_cast<Trk::TrapezoidVolumeBounds*>(&(trVol->volumeBounds()));
    Trk::DoubleTrapezoidVolumeBounds* dtrdBounds = dynamic_cast<Trk::DoubleTrapezoidVolumeBounds*>(&(trVol->volumeBounds()));
    Trk::SimplePolygonBrepVolumeBounds* pbBounds = dynamic_cast<Trk::SimplePolygonBrepVolumeBounds*>(&(trVol->volumeBounds()));

    if (cubBounds)
        ATH_MSG_VERBOSE("before loop -- cubBounds ");
    else if (trdBounds)
        ATH_MSG_VERBOSE("before loop -- trdBounds ");
    else if (dtrdBounds)
        ATH_MSG_VERBOSE("before loop -- dtrdBounds ");
    else if (pbBounds)
        ATH_MSG_VERBOSE("before loop -- pbBounds ");
    else
        ATH_MSG_VERBOSE("before loop -- no Bounds ");

    Amg::Transform3D subt(Trk::s_idTransform);

    Trk::SubtractedVolumeBounds* subBounds = dynamic_cast<Trk::SubtractedVolumeBounds*>(&(trVol->volumeBounds()));
    if (subBounds) {
        subt *= Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 1., 0.)) * Amg::AngleAxis3D(0.5 * M_PI, Amg::Vector3D(0., 0., 1.));
        while (subBounds) {
            ATH_MSG_VERBOSE("looping over subtracted volume bounds:outer,inner position:" << subBounds->outer()->center() << ","
                                                                                          << subBounds->inner()->center());
            Trk::CuboidVolumeBounds* ocubBounds = dynamic_cast<Trk::CuboidVolumeBounds*>(&(subBounds->outer()->volumeBounds()));
            Trk::TrapezoidVolumeBounds* otrdBounds =
                dynamic_cast<Trk::TrapezoidVolumeBounds*>(&(subBounds->outer()->volumeBounds()));
            Trk::DoubleTrapezoidVolumeBounds* odtrdBounds =
                dynamic_cast<Trk::DoubleTrapezoidVolumeBounds*>(&(subBounds->outer()->volumeBounds()));
            Trk::SimplePolygonBrepVolumeBounds* opbBounds =
                dynamic_cast<Trk::SimplePolygonBrepVolumeBounds*>(&(subBounds->outer()->volumeBounds()));
            ATH_MSG_VERBOSE("outer volume:box,trd,dtrd,spb,subtr:" << ocubBounds << "," << otrdBounds << "," << odtrdBounds << ","
                                                                   << opbBounds << "," << subBounds);
            if (ocubBounds) cubBounds = ocubBounds;
            if (otrdBounds) trdBounds = otrdBounds;
            if (odtrdBounds) dtrdBounds = odtrdBounds;
            if (opbBounds) pbBounds = opbBounds;
            subBounds = dynamic_cast<Trk::SubtractedVolumeBounds*>(&(subBounds->outer()->volumeBounds()));
        }
    }

    if (cubBounds)
        ATH_MSG_VERBOSE("after loop -- cubBounds ");
    else if (trdBounds)
        ATH_MSG_VERBOSE("after loop -- trdBounds ");
    else if (dtrdBounds)
        ATH_MSG_VERBOSE("after loop -- dtrdBounds ");
    else if (pbBounds)
        ATH_MSG_VERBOSE("after loop -- pbBounds ");
    else {
        ATH_MSG_VERBOSE("after loop -- no Bounds ");
        return layRepr;
    }

    Trk::PlaneLayer* layer = nullptr;

    if (cubBounds) {
        double thickness = 2 * cubBounds->halflengthX();
        std::unique_ptr<Trk::OverlapDescriptor> od = nullptr;
        double scale = matEx->thickness() / thickness;
        Trk::MaterialProperties matProp(thickness, matEx->x0() / scale, matEx->l0() / scale, matEx->averageA(), matEx->averageZ(),
                                        scale * matEx->averageRho());
        Trk::HomogeneousLayerMaterial mat(matProp, 0.);

        auto bounds = std::make_shared<const Trk::RectangleBounds>(cubBounds->halflengthY(), cubBounds->halflengthZ());
        layer = new Trk::PlaneLayer(Amg::Transform3D(trVol->transform() * Amg::AngleAxis3D(M_PI_2, Amg::Vector3D(0., 1., 0.)) *
                                                     Amg::AngleAxis3D(M_PI_2, Amg::Vector3D(0., 0., 1.))),
                                    bounds, mat, thickness, std::move(od), 1);
    } else if (trdBounds) {
        double thickness = 2 * trdBounds->halflengthZ();
        std::unique_ptr<Trk::OverlapDescriptor> od = nullptr;
        double scale = matEx->thickness() / thickness;
        Trk::MaterialProperties matProp(thickness, matEx->x0() / scale, matEx->l0() / scale, matEx->averageA(), matEx->averageZ(),
                                        scale * matEx->averageRho());
        Trk::HomogeneousLayerMaterial mat(matProp, 0.);
        // double sf        =
        // 2*(trdBounds->minHalflengthX()+trdBounds->maxHalflengthX())*trdBounds->halflengthY();
        if (rtrd) {
            Trk::TrapezoidBounds* tbounds = new Trk::TrapezoidBounds(rtrd->halflengthX(), rtrd->minHalflengthY(), rtrd->maxHalflengthY());
            Trk::SharedObject<const Trk::SurfaceBounds> bounds(tbounds);
            layer = new Trk::PlaneLayer(Amg::Transform3D(subt * trVol->transform()), bounds, mat, thickness, std::move(od), 1);
            Amg::Vector3D mtg_pos = (transf * subt * trVol->transform()).translation();
            transf *= Amg::Translation3D(mrg_pos - mtg_pos);
        } else if (trd) {
            Trk::TrapezoidBounds* tbounds = new Trk::TrapezoidBounds(trd->minHalflengthX(), trd->maxHalflengthX(), trd->halflengthY());
            Trk::SharedObject<const Trk::SurfaceBounds> bounds(tbounds);
            layer = new Trk::PlaneLayer(Amg::Transform3D(subt * trVol->transform()), bounds, mat, thickness, std::move(od), 1);
            Amg::Vector3D mtg_pos = (transf * subt * trVol->transform()).translation();
            transf *= Amg::Translation3D(mrg_pos - mtg_pos);
        } else {
            const std::vector<const Trk::Surface*>* surfs = 
              trdBounds->decomposeToSurfaces(Amg::Transform3D(Trk::s_idTransform));
            const Trk::TrapezoidBounds* tbounds = dynamic_cast<const Trk::TrapezoidBounds*>(&(*(surfs))[0]->bounds());
            Trk::SharedObject<const Trk::SurfaceBounds> bounds(new Trk::TrapezoidBounds(*tbounds));
            layer = new Trk::PlaneLayer(Amg::Transform3D(subt * trVol->transform()), bounds, mat, thickness, std::move(od), 1);
        }
    } else if (dtrdBounds) {
        double thickness = 2 * dtrdBounds->halflengthZ();
        const std::vector<const Trk::Surface*>* surfs = 
          dtrdBounds->decomposeToSurfaces(Amg::Transform3D(Trk::s_idTransform));
        const Trk::DiamondBounds* dbounds = dynamic_cast<const Trk::DiamondBounds*>(&(*(surfs))[0]->bounds());
        Trk::SharedObject<const Trk::SurfaceBounds> bounds(new Trk::DiamondBounds(*dbounds));
        std::unique_ptr<Trk::OverlapDescriptor> od = nullptr;
        double scale = matEx->thickness() / thickness;
        Trk::MaterialProperties matProp(thickness, matEx->x0() / scale, matEx->l0() / scale, matEx->averageA(), matEx->averageZ(),
                                        scale * matEx->averageRho());
        Trk::HomogeneousLayerMaterial mat(matProp, 0.);
        layer = new Trk::PlaneLayer(Amg::Transform3D(trVol->transform()), bounds, mat, thickness, std::move(od), 1);
        delete surfs;
    } else if (pbBounds) {
        ATH_MSG_WARNING(" no implementatiom for SimplePolygonBrepBounds ");
        return layRepr;
    }

    layRepr = layer;
    layRepr->setLayerType(layType);

    return layRepr;
}

Trk::MaterialProperties Muon::MuonStationTypeBuilder::collectStationMaterial(const Trk::TrackingVolume* vol, double sf) const {
    Trk::MaterialProperties layMat(0., 10.e10, 10.e10, 13., 26., 0.);

    // sf is surface of the new layer used to calculate the average 'thickness' of
    // components layers
    if (vol->confinedLayers()) {
      Trk::BinnedArraySpan<Trk::Layer const * const> lays = vol->confinedLayers()->arrayObjects();
        for (const auto *lay : lays) {
            const Trk::MaterialProperties* mLay =
                lay->layerMaterialProperties()->fullMaterial(lay->surfaceRepresentation().center());
            // protect nan
            if (mLay && lay->thickness() > 0 && mLay->material().x0() > 0.) {
                layMat.addMaterial(mLay->material(), lay->thickness() / mLay->material().x0());
                ATH_MSG_VERBOSE(" collectStationMaterial after add confined lay " << layMat);
            }
        }
    }
    if (!vol->confinedArbitraryLayers().empty()) {
        Trk::ArraySpan<const Trk::Layer* const> lays = vol->confinedArbitraryLayers();
        for (const auto *lay : lays) {
            const Trk::MaterialProperties* mLay =
                lay->layerMaterialProperties()->fullMaterial(lay->surfaceRepresentation().center());
            // scaling factor
            const Trk::RectangleBounds* rect = dynamic_cast<const Trk::RectangleBounds*>(&(lay->surfaceRepresentation().bounds()));
            const Trk::TrapezoidBounds* trap = dynamic_cast<const Trk::TrapezoidBounds*>(&(lay->surfaceRepresentation().bounds()));
            if ((rect || trap) && mLay) {
                double scale = rect ? 4 * rect->halflengthX() * rect->halflengthY() / sf
                                    : 2 * (trap->minHalflengthX() + trap->maxHalflengthX()) * trap->halflengthY() / sf;
                // protect nan
                if (lay->thickness() > 0 && mLay->material().x0() > 0.) {
                    layMat.addMaterial(mLay->material(), scale * lay->thickness() / mLay->material().x0());
                    ATH_MSG_VERBOSE(" collectStationMaterial after add confined sub lay " << layMat);
                }
            }
        }
    }
    // subvolumes
    if (vol->confinedVolumes()) {
      Trk::BinnedArraySpan<Trk::TrackingVolume const * const> subVols = vol->confinedVolumes()->arrayObjects();
        for (const auto *subVol : subVols) {
            if (subVol->confinedLayers()) {
              Trk::BinnedArraySpan<Trk::Layer const * const> lays = subVol->confinedLayers()->arrayObjects();
                for (const auto *lay : lays) {
                    const Trk::MaterialProperties* mLay =
                        lay->layerMaterialProperties()->fullMaterial(lay->surfaceRepresentation().center());
                    // protect nan
                    if (mLay && lay->thickness() > 0 && mLay->material().x0() > 0.) {
                        layMat.addMaterial(mLay->material(), lay->thickness() / mLay->material().x0());
                        ATH_MSG_VERBOSE(" collectStationMaterial after add confined vol " << layMat);
                    }
                }
            }
            if (!subVol->confinedArbitraryLayers().empty()) {
                Trk::ArraySpan<const Trk::Layer* const> lays = (subVol->confinedArbitraryLayers());
                for (const auto *lay : lays) {
                    const Trk::MaterialProperties* mLay =
                        lay->layerMaterialProperties()->fullMaterial(lay->surfaceRepresentation().center());
                    // scaling factor
                    const Trk::RectangleBounds* rect =
                        dynamic_cast<const Trk::RectangleBounds*>(&(lay->surfaceRepresentation().bounds()));
                    const Trk::TrapezoidBounds* trap =
                        dynamic_cast<const Trk::TrapezoidBounds*>(&(lay->surfaceRepresentation().bounds()));
                    if ((rect || trap) && mLay) {
                        double scale = rect ? 4 * rect->halflengthX() * rect->halflengthY() / sf
                                            : 2 * (trap->minHalflengthX() + trap->maxHalflengthX()) * trap->halflengthY() / sf;
                        // protect nan
                        if (lay->thickness() > 0 && mLay->material().x0() > 0.) {
                            layMat.addMaterial(mLay->material(), scale * lay->thickness() / mLay->material().x0());
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

void Muon::MuonStationTypeBuilder::printTransform(std::string comment, Amg::Transform3D transf) const {

  ATH_MSG_DEBUG( comment );
  ATH_MSG_DEBUG( "translation:" << transf.translation().x() <<"," <<  transf.translation().y() <<"," <<  transf.translation().z() );
  ATH_MSG_DEBUG( "rotation:" << transf.rotation()(0,0) <<"," <<  transf.rotation()(0,1) <<"," <<  transf.rotation()(0,2) );
  ATH_MSG_DEBUG( "               " << transf.rotation()(1,0) <<"," <<  transf.rotation()(1,1) <<"," <<  transf.rotation()(1,2) );
  ATH_MSG_DEBUG( "               " << transf.rotation()(2,0) <<"," <<  transf.rotation()(2,1) <<"," <<  transf.rotation()(2,2) );
  
}

void Muon::MuonStationTypeBuilder::printVolumeBounds(std::string comment, const Trk::VolumeBounds& volBounds) const {
  
  ATH_MSG_DEBUG( comment );
  
  const Trk::CuboidVolumeBounds* box = dynamic_cast< const Trk::CuboidVolumeBounds*> (&volBounds);
  if (box) {
    ATH_MSG_DEBUG("cuboid:" << box->halflengthX() <<"," << box->halflengthY() <<"," << box->halflengthZ() );
    return;
  }
  const Trk::TrapezoidVolumeBounds* trd = dynamic_cast< const Trk::TrapezoidVolumeBounds*> (&volBounds);
  if (trd) {
    ATH_MSG_DEBUG("trapezoid:" << trd->minHalflengthX() <<"," << trd->maxHalflengthX() <<"," <<
		  trd->halflengthY() <<"," << trd->halflengthZ() );
    return;
  }
  const Trk::DoubleTrapezoidVolumeBounds* dtrd = dynamic_cast< const Trk::DoubleTrapezoidVolumeBounds*> (&volBounds);
  if (dtrd) {
    ATH_MSG_DEBUG("double trapezoid:" << dtrd->minHalflengthX() <<"," << dtrd->medHalflengthX() <<"," << dtrd->maxHalflengthX() <<"," <<
		  dtrd->halflengthY1() <<"," <<dtrd->halflengthY2() <<"," << dtrd->halflengthZ() );
    return;
  }

  const Trk::SimplePolygonBrepVolumeBounds* spb = dynamic_cast< const Trk::SimplePolygonBrepVolumeBounds*> (&volBounds);
    ATH_MSG_DEBUG( "SimplePolygonBrep bounds: number of vertices:" << spb->xyVertices().size() );
										     
}

double Muon::MuonStationTypeBuilder::envelopeThickness( const Trk::VolumeBounds& volBounds) const {
  
  const Trk::CuboidVolumeBounds* box = dynamic_cast< const Trk::CuboidVolumeBounds*> (&volBounds);
  if (box)   return box->halflengthZ() ;
 
  const Trk::TrapezoidVolumeBounds* trd = dynamic_cast< const Trk::TrapezoidVolumeBounds*> (&volBounds);
  if (trd)  return trd->halflengthZ() ;

  const Trk::DoubleTrapezoidVolumeBounds* dtrd = dynamic_cast< const Trk::DoubleTrapezoidVolumeBounds*> (&volBounds);
  if (dtrd)  return dtrd->halflengthZ() ;

  const Trk::SimplePolygonBrepVolumeBounds* spb = dynamic_cast< const Trk::SimplePolygonBrepVolumeBounds*> (&volBounds);
  if (spb) return spb->halflengthZ() ;

  return 0.;
}

Trk::SurfaceBounds*  Muon::MuonStationTypeBuilder::getLayerBoundsFromEnvelope( const Trk::Volume* envelope) const {
    
  const Trk::CuboidVolumeBounds* box = dynamic_cast< const Trk::CuboidVolumeBounds*> (&(envelope->volumeBounds()));
  if (box)   return new Trk::RectangleBounds(box->halflengthX(), box->halflengthY() );
 
  const Trk::TrapezoidVolumeBounds* trd = dynamic_cast< const Trk::TrapezoidVolumeBounds*> (&(envelope->volumeBounds()));
	     
  if (trd)  return new Trk::TrapezoidBounds( trd->minHalflengthX(), trd->maxHalflengthX(), trd->halflengthY() );

  const Trk::DoubleTrapezoidVolumeBounds* dtrd = dynamic_cast< const Trk::DoubleTrapezoidVolumeBounds*> (&(envelope->volumeBounds()));

  if (dtrd) return new Trk::DiamondBounds( dtrd->minHalflengthX(), dtrd->medHalflengthX(), dtrd->maxHalflengthX(), dtrd->halflengthY1(), dtrd->halflengthY2() );

  return nullptr;
										     
}

double  Muon::MuonStationTypeBuilder::area( const Trk::SurfaceBounds*  sb) const {
    
  const Trk::RectangleBounds* box = dynamic_cast< const Trk::RectangleBounds*> (sb);
  if (box)   return 4*box->halflengthX()*box->halflengthY();
 
  const Trk::TrapezoidBounds* trd = dynamic_cast< const Trk::TrapezoidBounds*> (sb);
  if (trd)  return 2*(trd->minHalflengthX()+trd->maxHalflengthX()) * trd->halflengthY() ;

  const Trk::DiamondBounds* dtrd = dynamic_cast< const Trk::DiamondBounds*> (sb);
  if (dtrd)  return 2*(dtrd->minHalflengthX()+dtrd->medHalflengthX()) *dtrd->halflengthY1()
	       + 2*(dtrd->medHalflengthX()+dtrd->maxHalflengthX()) * dtrd->halflengthY2() ;

  return 0.;
										     
}
