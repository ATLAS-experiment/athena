/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackingVolumeHelper.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk include
#include "TrkDetDescrTools/TrackingVolumeHelper.h"
#include "TrkDetDescrInterfaces/ILayerArrayCreator.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeArrayCreator.h"
#include "TrkDetDescrUtils/BinnedArray.h"
#include "TrkDetDescrUtils/BinUtility.h"
#include "TrkSurfaces/Surface.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/CylinderBounds.h"
#include "TrkSurfaces/DiscBounds.h"
#include "TrkVolumes/VolumeBounds.h"
#include "TrkVolumes/CylinderVolumeBounds.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkGeometry/CylinderLayer.h"
#include "TrkGeometry/DiscLayer.h"
#include "TrkGeometry/MaterialLayer.h"
#include "TrkGeometry/LayerMaterialProperties.h"
#include "TrkGeometry/HomogeneousLayerMaterial.h"
#include "TrkGeometry/BinnedLayerMaterial.h"
#include "TrkGeometry/GlueVolumesDescriptor.h"
// Amg
#include "GeoPrimitives/GeoPrimitivesHelpers.h"

#include <memory>
#include <stdexcept>

namespace {
    template <class Obj>
        std::vector<Obj*> toRawVec(const std::vector<std::shared_ptr<Obj>>& in) {

        std::vector<Obj*> out{};
        out.reserve(in.size());
        for (const std::shared_ptr<Obj>& obj : in) {
            out.emplace_back(obj.get());
        }
        return out;
    }
}

namespace Trk {
// constructor
TrackingVolumeHelper::TrackingVolumeHelper(const std::string& t, const std::string& n, const IInterface* p)
: AthAlgTool(t,n,p),
  TrackingVolumeManipulator() {
    declareInterface<ITrackingVolumeHelper>(this);
}
// destructor
TrackingVolumeHelper::~TrackingVolumeHelper() = default;


// the interface methods
StatusCode TrackingVolumeHelper::initialize() {

    ATH_MSG_DEBUG( "initialize() " );
    ATH_CHECK(m_layerArrayCreator.retrieve());
    ATH_CHECK(m_trackingVolumeArrayCreator.retrieve());
    return StatusCode::SUCCESS;
}


/** Simply forward to base class method to enhance friendship relation */
void TrackingVolumeHelper::glueTrackingVolumes(TrackingVolume& firstVol,
                                               BoundarySurfaceFace firstFace,
                                               TrackingVolume& secondVol,
                                               BoundarySurfaceFace secondFace,
                                               bool buildBoundaryLayer) const
{
    TrackingVolumeManipulator::glueVolumes( firstVol, firstFace, secondVol, secondFace );

    // ----------------------------------------------------------------------------------------
    // create a MaterialLayer as a boundary
    if (buildBoundaryLayer){
        auto& bSurfacesFirst  =  firstVol.boundarySurfaces();
        auto& bSurfacesSecond =  secondVol.boundarySurfaces();
        // get the boundary surfaces
        Surface& firstFaceSurface  = bSurfacesFirst[firstFace]->surfaceRepresentation();
        Surface& secondFaceSurface  = bSurfacesSecond[secondFace]->surfaceRepresentation();
        // dynamic_cast to the right type
        std::unique_ptr<LayerMaterialProperties> lmps = layerMaterialProperties(firstFaceSurface);
        // LayerMaterialProperties will be cloned in MaterialLayer

        // set the layer to the two surfaces
        if (lmps){
            std::shared_ptr<Layer> mLayer = std::make_shared<MaterialLayer>(firstFaceSurface, *lmps);
            ATH_MSG_VERBOSE( "Set MaterialLayer to the BoundarySurface of first volume." );
            firstFaceSurface.setMaterialLayer(mLayer);
            ATH_MSG_VERBOSE("Set MaterialLayer to the BoundarySurface of second volume.");
            secondFaceSurface.setMaterialLayer(mLayer);
        }
    }
}

/** Simply forward to base class method to enhance friendship relation */
void TrackingVolumeHelper::glueTrackingVolumes(TrackingVolume& firstVol,
                                               BoundarySurfaceFace firstFace,
                                               const std::vector<TrackingVolume*>& secondVolumes,
                                               BoundarySurfaceFace secondFace,
                                               bool buildBoundaryLayer,
                                               bool boundaryFaceExchange) const
{

    if (msgLvl(MSG::VERBOSE)) {
        ATH_MSG_VERBOSE( "Glue Volume '" << firstVol.volumeName() << "' to " << secondVolumes.size() << " volume(s): " );
        for (const auto & volIter : secondVolumes)
            ATH_MSG_VERBOSE( "              -> " << (volIter)->volumeName() );
    }
    // prepare the material layer if needed
    std::shared_ptr<Layer> mLayer{};
    // ----------------------------------------------------------------------------------------
    // create a MaterialLayer as a boundary
    if (buildBoundaryLayer){
        // the first face surface
        Surface& firstFaceSurface = firstVol.boundarySurfaces()[firstFace]->surfaceRepresentation();
        std::unique_ptr<LayerMaterialProperties> lmps = layerMaterialProperties(firstFaceSurface);
        // LayerMaterialProperties are cloned by MaterialLayer

        // the material layer is ready - it can be assigned
        mLayer = std::make_unique<MaterialLayer>(firstFaceSurface, *lmps);
        ATH_MSG_VERBOSE( "Set MaterialLayer to the BoundarySurface of first volume (may be shared with second volume)." );
        firstFaceSurface.setMaterialLayer(mLayer);
    }
    // if only one volume was given in the vector call the standard one-to-one glueing
    // 1-to-1 case
    if (secondVolumes.size() == 1) {
        // self call for one-on-one
        glueTrackingVolumes(firstVol, firstFace, *(secondVolumes[0]), secondFace);
    } else {
        // create the navigation bin array
        BinnedArray<TrackingVolume>* navArray = nullptr;
        // create the Array - either r-binned or z-binned
        if (firstFace == negativeFaceXY || firstFace == positiveFaceXY )
            navArray = m_trackingVolumeArrayCreator->cylinderVolumesArrayInR(secondVolumes, true);
        else
            navArray = m_trackingVolumeArrayCreator->cylinderVolumesArrayInZ(secondVolumes, true);

        // set the volume array to the first boundary surface - this must always happen
        if (firstFace != tubeInnerCover)
            setOutsideTrackingVolumeArray( firstVol, firstFace, navArray );
        else
            setInsideTrackingVolumeArray( firstVol, firstFace, navArray );
        // the navigation arrays are completed now - check if the boundary face should be exchanged
        // [1] the boundary face exchange ----------------------------------------------------------------------------------------
        if (boundaryFaceExchange){
            // creating only one boundary surface
            ATH_MSG_VERBOSE("Creating a joint boundary surface for 1-to-n glueing case.");
            // get the dimension of boundary surface of the first volume
            std::shared_ptr<BoundarySurface<TrackingVolume> > bSurface = firstVol.boundarySurfaces()[firstFace];
            // replace the boundary surface
            for ( const auto & volIter: secondVolumes )
                setBoundarySurface(*volIter, bSurface, secondFace);
        } else {
         // [2] the traditional way, keeping two boundary surfaces
         // now set the face to the volume array -------------------------------------------------------------------------------
            for ( const auto & volIter: secondVolumes ) {
                // the secondGlueFace
                BoundarySurfaceFace secondGlueFace = secondFace;
                if (secondFace == tubeOuterCover) {
                    //check for cylinder case
                    const CylinderVolumeBounds* currentVolBounds = dynamic_cast<const CylinderVolumeBounds*>(&((volIter)->volumeBounds()));
                    // protection : there may be a cylinder within the tube vector
                    if (currentVolBounds && currentVolBounds->innerRadius() < 10e-3)
                        secondGlueFace = cylinderCover;
                    setOutsideTrackingVolume(*volIter, secondGlueFace, (&(firstVol)));
                } // for all surfaces except the tunbeInnerCover outside of the surface is identical to outside of the volume
                else if (secondGlueFace != tubeInnerCover)
                    setOutsideTrackingVolume(*volIter, secondGlueFace, (&(firstVol)));
                else
                    setInsideTrackingVolume(*volIter, secondGlueFace, (&(firstVol)));
                // if existing, set the material Layer
                // get the second face surface and set the new MaterialLayer
                Surface& secondFaceSurface = volIter->boundarySurfaces()[secondFace]->surfaceRepresentation();
                secondFaceSurface.setMaterialLayer(mLayer);
            }
        }
    } // 1-to-n case
}


/** Simply forward to base class method to enhance friendship relation */
void TrackingVolumeHelper::glueTrackingVolumes(const std::vector<TrackingVolume*>& firstVolumes,
                                                    BoundarySurfaceFace firstFace,
                                                    const std::vector<TrackingVolume*>& secondVolumes,
                                                    BoundarySurfaceFace secondFace,
                                                    bool buildBoundaryLayer,
                                                    bool boundaryFaceExchange) const
{


    BinnedArray<TrackingVolume>* navArrayOne = nullptr;
    BinnedArray<TrackingVolume>* navArrayTwo = nullptr;

    std::unique_ptr<Surface>     mLayerSurface;
    std::shared_ptr<Layer>       mLayer;

    ATH_MSG_VERBOSE("Glue configuration firstFace | secondFace = " << firstFace << " | " << secondFace );

    // create the Arrays - assuming cylindrical TrackingVolumes
    if (firstFace < 2 && secondFace < 2 ) {
        ATH_MSG_VERBOSE( "The glueing is done along z axis" );
        navArrayOne = m_trackingVolumeArrayCreator->cylinderVolumesArrayInR(firstVolumes, true);
        navArrayTwo = m_trackingVolumeArrayCreator->cylinderVolumesArrayInR(secondVolumes, true);
        // build a disc to separate the two
        if (buildBoundaryLayer || boundaryFaceExchange){
            double rmin = 10e10; double rmax = 0; double boundaryz = 0.; double centerzOne = 0.;
            for (const auto & volIter : firstVolumes ){
                const CylinderVolumeBounds* cb = dynamic_cast<const CylinderVolumeBounds*>(&(volIter->volumeBounds()));
                if (cb) {
                    takeSmaller(rmin,cb->innerRadius());
                    takeBigger(rmax,cb->outerRadius());
                    // get the z of the surface
                    boundaryz = volIter->boundarySurfaces()[firstFace]->surfaceRepresentation().center().z();
                }
                centerzOne = volIter->center().z();
            }
            if (buildBoundaryLayer){
              Amg::Transform3D mLayerTransform =
                Amg::Transform3D(Amg::Translation3D(0., 0., boundaryz));
              // layer surface
              mLayerSurface = std::make_unique<DiscSurface>(mLayerTransform, rmin, rmax);
              // create a MaterialLayer
              std::unique_ptr<LayerMaterialProperties> lmps = layerMaterialProperties(*mLayerSurface);
              // MaterialLayer clones the LayerMaterialPropteries.

              if (lmps) {
                mLayer = std::make_unique<MaterialLayer>(std::move(mLayerSurface), *lmps);
              }
            }
            if (boundaryFaceExchange){
                // creating only one boundary surface
                ATH_MSG_VERBOSE("Creating a joint boundary surface for n-to-n glueing case.");
                // check if the seconf volumes have a bigger z value or a smaller one
                double centerzTwo = secondVolumes[secondVolumes.size()-1]->center().z();
                // thi sboundary surface is having a z-axix along the global z-axis
                Amg::Transform3D boundaryTransform = Amg::getTranslateZ3D(boundaryz);
                // disc surfaces
                DiscSurface dSurface(boundaryTransform, rmin, rmax);
                // swap if needed
                if (centerzTwo < centerzOne){
                    std::swap(navArrayTwo, navArrayOne);
                }
                // create the new boudnary surface which spans over the entire volume border
                std::shared_ptr< BinnedArray<TrackingVolume> >  navArrayInside(navArrayOne);
                std::shared_ptr< BinnedArray<TrackingVolume> >  navArrayOutside(navArrayTwo);
                BoundaryDiscSurface<TrackingVolume>* boundarySurface = new BoundaryDiscSurface<TrackingVolume>(navArrayInside,navArrayOutside,dSurface);
                std::shared_ptr<BoundarySurface<TrackingVolume> > sharedBoundarySurface(boundarySurface);
                // attach the material layer to the shared boundary if existing
                if (mLayer) {
                    ATH_MSG_VERBOSE( "Set MaterialLayer to the BoundarySurface of volume from second array." );
                    boundarySurface->surfaceRepresentation().setMaterialLayer(mLayer);
                }
                // set the boundary surface to the volumes of both sides
                for (const auto & volIter : firstVolumes){
                    ATH_MSG_VERBOSE("  -> first array : setting a newly created boundary surface to  " << volIter->volumeName());
                    setBoundarySurface(*volIter,sharedBoundarySurface,firstFace);
                }
                for (const auto & volIter : secondVolumes){
                    ATH_MSG_VERBOSE("  -> second array : setting a newly created boundary surface to  " << volIter->volumeName());
                    setBoundarySurface(*volIter,sharedBoundarySurface,secondFace);
                }
                // we are done here
                return;
            }
        }
    } else {
        ATH_MSG_VERBOSE( "The glueing is done along the radius." );
        navArrayOne = m_trackingVolumeArrayCreator->cylinderVolumesArrayInZ(firstVolumes, true);
        navArrayTwo = m_trackingVolumeArrayCreator->cylinderVolumesArrayInZ(secondVolumes, true);
        // check if the boundary layer was configured to be built
        if (buildBoundaryLayer || boundaryFaceExchange){
            // build a cylinder to separate the two
            double zmin = 10e10; double zmax = -10e10; double boundaryr = 0.; double volumerOne = 0.; double volumerTwo = 10e10;
            for (const auto & volIter : firstVolumes ){
                const CylinderVolumeBounds* cb = dynamic_cast<const CylinderVolumeBounds*>(&(volIter->volumeBounds()));
                if (cb) {
                    takeSmaller(zmin,volIter->center().z()-cb->halflengthZ());
                    takeBigger(zmax,volIter->center().z()+cb->halflengthZ());
                    // get the z of the surface
                    boundaryr = volIter->boundarySurfaces()[firstFace]->surfaceRepresentation().bounds().r();
                    // get the volume radius
                    volumerOne = cb->outerRadius();
                }
            }
            // check if boundary layer should be built
            if (buildBoundaryLayer){
              std::unique_ptr<Amg::Transform3D> mLayerTransform =
                ((zmin + zmax) * (zmin + zmax) < 10e-4)
                  ? nullptr
                  : std::make_unique < Amg::Transform3D>();

              if (mLayerTransform) (*mLayerTransform) = Amg::Translation3D(0.,0.,0.5*(zmin+zmax));
                mLayerSurface.reset( mLayerTransform ? new CylinderSurface(*mLayerTransform,boundaryr,0.5*(zmax-zmin))  :
                                     new CylinderSurface(boundaryr,0.5*(zmax-zmin)) );
                // create a MaterialLayer
                std::unique_ptr<LayerMaterialProperties>  lmps = layerMaterialProperties(*mLayerSurface);
                // LayerMaterialProperties will be cloned in MaterialLayer
                if (lmps) mLayer = std::make_unique<MaterialLayer>(
                                                               std::shared_ptr<Surface>(std::move(mLayerSurface)),
                                                               *lmps );
            }
            // check if boundary face should be exchanged
            if (boundaryFaceExchange) {
                // creating only one boundary surface
                ATH_MSG_VERBOSE("Creating a joint boundary surface for n-to-n glueing case.");
                // the boundary transform can be 0 for cylinder surfaces
                std::unique_ptr<Amg::Transform3D> boundaryTransform =
                  ((zmin + zmax) * (zmin + zmax) < 10e-4)
                    ? nullptr
                    : std::make_unique<Amg::Transform3D>();

                if (boundaryTransform) (*boundaryTransform) = Amg::getTranslateZ3D(0.5*(zmin+zmax));
                // create the cylinder surface for the shared boundary
                CylinderSurface cSurface = boundaryTransform ? CylinderSurface(*boundaryTransform,boundaryr,0.5*(zmax-zmin)) :
                                                               CylinderSurface(boundaryr,0.5*(zmax-zmin));
                // get the volume outer radius of the sconf volumes
                const CylinderVolumeBounds* cbTwo = dynamic_cast<const CylinderVolumeBounds*>(&(secondVolumes[secondVolumes.size()-1]->volumeBounds()));
                if (cbTwo){
                    volumerTwo = cbTwo->outerRadius();
                }
                // swap if needed
                if (volumerTwo < volumerOne){
                    BinnedArray<TrackingVolume>* navArraySwap = navArrayOne;
                    navArrayTwo = navArrayOne;
                    navArrayOne = navArraySwap;
                }
                // create the new boudnary surface which spans over the entire volume border
                std::shared_ptr< BinnedArray<TrackingVolume> >  navArrayInside(navArrayOne);
                std::shared_ptr< BinnedArray<TrackingVolume> >  navArrayOutside(navArrayTwo);
                BoundaryCylinderSurface<TrackingVolume>* boundarySurface = new BoundaryCylinderSurface<TrackingVolume>(navArrayInside,navArrayOutside,cSurface);
                std::shared_ptr<BoundarySurface<TrackingVolume> > sharedBoundarySurface(boundarySurface);
                // attach the material layer to the shared boundary if existing
                if (mLayer) {
                  ATH_MSG_VERBOSE("Set MaterialLayer to the BoundarySurface of volume from second array.");
                  // assume that now the mlayer onwership goes over to the TrackingVolume
                  boundarySurface->surfaceRepresentation().setMaterialLayer(mLayer);
                }
                // set the boundary surface to the volumes of both sides
                for (const auto & volIter : firstVolumes){
                    ATH_MSG_VERBOSE("  -> first array : setting a newly created boundary surface to  " << volIter->volumeName());
                    setBoundarySurface(*volIter,sharedBoundarySurface,firstFace);
                }
                for (const auto & volIter : secondVolumes){
                    ATH_MSG_VERBOSE("  -> second array : setting a newly created boundary surface to  " << volIter->volumeName());
                    setBoundarySurface(*volIter,sharedBoundarySurface,secondFace);
                }
                // we are done here
                return;

            }
        } // build either boundary layer or exchange the face
    } // radial glueing


    // create the boundary faces - not creating a joint one
    ATH_MSG_VERBOSE("Leaving individual boundary surfaces for n-to-n glueing case.");

    // assign the navigation arrays
    std::shared_ptr< BinnedArray< TrackingVolume> > navArrayOneShared(navArrayOne);
    std::shared_ptr< BinnedArray< TrackingVolume> > navArrayTwoShared(navArrayTwo);

    // (a) to the first set of volumes
    for (const auto & tVolIter: firstVolumes) {
        // take care of the orientation of the normal vector
        if (firstFace != tubeInnerCover) {
            setOutsideTrackingVolumeArray(*tVolIter,firstFace,navArrayTwoShared);
            ATH_MSG_VERBOSE( "Set outsideTrackingVolumeArray at face " << firstFace << " to " << (*tVolIter).volumeName() );
        } else {
            setInsideTrackingVolumeArray(*tVolIter,firstFace,navArrayTwoShared);
            ATH_MSG_VERBOSE( "Set insideTrackingVolumeArray at face " << firstFace << " to " << (*tVolIter).volumeName() );
        }
        // set the boundary layer if it exists
        if (mLayer) {
            ATH_MSG_VERBOSE( "Set MaterialLayer to the BoundarySurface of volume from first array." );
            Surface& firstFaceSurface = tVolIter->boundarySurfaces()[firstFace]->surfaceRepresentation();
            // assume that now the mlayer onwership goes over to the TrackingVolume
            //cppcheck-suppress ignoredReturnValue
            firstFaceSurface.setMaterialLayer(mLayer);
        }

    }
    // (b) to the second set of volumes
    for (const auto & tVolIter : secondVolumes) {
        // take care of the orientation of the normal vector
        if (secondFace != tubeInnerCover) {
            ATH_MSG_VERBOSE( "Set outsideTrackingVolumeArray at face " << secondFace << " to " << (*tVolIter).volumeName() );
            setOutsideTrackingVolumeArray(*tVolIter,secondFace,navArrayOneShared);
        } else {
            ATH_MSG_VERBOSE( "Set insideTrackingVolumeArray at face " << secondFace << " to " << (*tVolIter).volumeName() );
            setInsideTrackingVolumeArray(*tVolIter,secondFace,navArrayOneShared);
        }
        if (mLayer) {
            ATH_MSG_VERBOSE( "Set MaterialLayer to the BoundarySurface of volume from second array." );
            Surface& secondFaceSurface = tVolIter->boundarySurfaces()[secondFace]->surfaceRepresentation();
            // assume that now the mlayer onwership goes over to the TrackingVolume
            //cppcheck-suppress ignoredReturnValue
            secondFaceSurface.setMaterialLayer(mLayer);
        }
    }
    // coverity will report a bug here for mLayer running out of scope, but the memory management is done later in the TrackingVolume
}

std::unique_ptr<TrackingVolume> TrackingVolumeHelper::glueTrackingVolumeArrays(std::shared_ptr<TrackingVolume> firstVol,
                                                                               BoundarySurfaceFace firstFace,
                                                                               std::shared_ptr<TrackingVolume> secondVol,
                                                                               BoundarySurfaceFace secondFace,
                                                                               const std::string& name) const {
    std::unique_ptr<TrackingVolume> enclosingVolume{};

    const auto *cyl1 = dynamic_cast<const CylinderVolumeBounds*>(&(firstVol->volumeBounds()));
    const auto *cyl2 = dynamic_cast<const CylinderVolumeBounds*>(&(secondVol->volumeBounds()));

    if (!cyl1 || !cyl2) {
        ATH_MSG_ERROR( "TrackingVolumeHelper::glueTrackingVolumeArrays: input volumes not cylinders, return 0" );
        return enclosingVolume;
    }
    if (cyl1->halfPhiSector()!= M_PI || cyl2->halfPhiSector()!= M_PI ) {
        ATH_MSG_ERROR( "TrackingVolumeHelper::glueTrackingVolumeArrays: not coded for cylinder Phi sectors yet, return 0" );
        return enclosingVolume;
    }

    // if the swap is required
    BoundarySurfaceFace firstFaceCorr = firstFace;
    BoundarySurfaceFace secondFaceCorr = secondFace;


    // build volume envelope
    std::vector<std::shared_ptr<TrackingVolume>> vols;
    std::unique_ptr<CylinderVolumeBounds> envBounds{};
    std::unique_ptr<Amg::Transform3D> envTransf{};
    std::unique_ptr<BinnedArray<TrackingVolume>>  subVols{};
    vols.push_back(firstVol);
    vols.push_back(secondVol);
    std::vector<std::shared_ptr<TrackingVolume>> envGlueNegXY{}, envGluePosXY{}, envGlueOuter, envGlueInner{};

    if (firstFace==positiveFaceXY) {
        envBounds =  std::make_unique<CylinderVolumeBounds>(cyl1->innerRadius(),
                                                                 cyl1->outerRadius(),
                                                                 cyl1->halflengthZ() + cyl2->halflengthZ());

        const Amg::Vector3D center{firstVol->center()};
        envTransf = std::make_unique<Amg::Transform3D>(Amg::getTranslate3D(center.x(),
                                                                           center.y(),
                                                                           center.z() + cyl2->halflengthZ()));

        subVols = m_trackingVolumeArrayCreator->cylinderVolumesArrayInZ(vols, false);
        envGlueNegXY.push_back(firstVol);
        envGluePosXY.push_back(secondVol);
        envGlueOuter = vols;
        envGlueInner = vols;
    } else if (firstFace==negativeFaceXY) {
        envBounds =  std::make_unique<CylinderVolumeBounds>(cyl1->innerRadius(),
                                                                 cyl1->outerRadius(),
                                                                 cyl1->halflengthZ()+cyl2->halflengthZ());
        const Amg::Vector3D center{firstVol->center()};
        envTransf = std::make_unique<Amg::Transform3D>(Amg::getTranslate3D(center.x(),
                                                                           center.y(),
                                                                           center.z() - cyl2->halflengthZ()));
        envGlueNegXY.push_back(secondVol);
        envGluePosXY.push_back(firstVol);
        // revert vols
        vols.clear();
        vols.push_back(secondVol);
        vols.push_back(firstVol);
        // --- account for the swapping
        firstFaceCorr = secondFace;
        secondFaceCorr = firstFace;
        //
        subVols = m_trackingVolumeArrayCreator->cylinderVolumesArrayInZ(vols,false);
        envGlueOuter = vols;
        envGlueInner = vols;
    } else if (firstFace==tubeInnerCover) {
        if (secondFace==tubeOuterCover){
            envBounds =  std::make_unique<CylinderVolumeBounds>(cyl2->innerRadius(),
                                                                     cyl1->outerRadius(),
                                                                     cyl1->halflengthZ());
        } else {
            envBounds =  std::make_unique<CylinderVolumeBounds>(cyl1->outerRadius(),
                                                                     cyl1->halflengthZ());
        }
        if (!firstVol->transform().isApprox(Amg::Transform3D::Identity())) {
            envTransf = std::make_unique<Amg::Transform3D>(Amg::getTranslate3D(firstVol->center()));
        }
        // revert vols
        vols.clear();
        vols.push_back(secondVol);
        vols.push_back(firstVol);
        // account for the swapping
        firstFaceCorr = secondFace;
        secondFaceCorr = firstFace;
        //
        subVols = m_trackingVolumeArrayCreator->cylinderVolumesArrayInR(vols,false);
        envGlueNegXY = vols;
        envGluePosXY = vols;
        envGlueOuter.push_back(firstVol);
        envGlueInner.push_back(secondVol);
    } else {
        envBounds =  std::make_unique<CylinderVolumeBounds>(cyl1->innerRadius(),
                                                                 cyl2->outerRadius(),
                                                                 cyl1->halflengthZ());
        if(!firstVol->transform().isApprox(Amg::Transform3D::Identity())){
           envTransf = std::make_unique<Amg::Transform3D>(Amg::getTranslate3D(firstVol->center()));
        }
        subVols = m_trackingVolumeArrayCreator->cylinderVolumesArrayInR(vols, false);
        envGlueNegXY = vols;
        envGluePosXY = vols;
        envGlueOuter.push_back(secondVol);
        envGlueInner.push_back(firstVol);
        // account for the swapping
        firstFaceCorr = secondFace;
        secondFaceCorr = firstFace;
    }

    // create the enveloping volume
    enclosingVolume  =  std::make_unique<TrackingVolume>(envTransf.release(),
                                                         envBounds.release(),
                                                         *firstVol,
                                                         nullptr, subVols.release(), name);

    // ENVELOPE GLUE DESCRIPTION -----------------------------------------------------------------
    // glue descriptors ---- they jump to the first one
    GlueVolumesDescriptor& glueDescr  = enclosingVolume->glueVolumesDescriptor();

    // for the outside volumes, could be done in a loop as well, but will only save 4 lines
    std::vector<TrackingVolume*> glueNegXY{}, gluePosXY{}, glueInner{},glueOuter{};
    fillGlueVolumes(vols, envGlueNegXY, negativeFaceXY,glueNegXY);
    fillGlueVolumes(vols, envGluePosXY, positiveFaceXY,gluePosXY);
    fillGlueVolumes(vols, envGlueInner, tubeInnerCover,glueInner);
    fillGlueVolumes(vols, envGlueOuter, tubeOuterCover,glueOuter);
    // set them to the envelopGlueDescriptor
    glueDescr.registerGlueVolumes(negativeFaceXY, glueNegXY);
    glueDescr.registerGlueVolumes(positiveFaceXY, gluePosXY);
    glueDescr.registerGlueVolumes(tubeInnerCover, glueInner);
    glueDescr.registerGlueVolumes(tubeOuterCover, glueOuter);
    glueDescr.registerGlueVolumes(cylinderCover,  glueOuter);

    // INTERNAL GLUEING ---------------------------------------------------------------------------
    glueTrackingVolumes(vols, firstFaceCorr, secondFaceCorr);

    return enclosingVolume;
}



void TrackingVolumeHelper::fillGlueVolumes(const std::vector<std::shared_ptr<TrackingVolume>>& topLevelVolumes,
                                           const std::vector<std::shared_ptr<TrackingVolume>>& envelopeFaceVolumes,
                                           BoundarySurfaceFace glueFace,
                                           std::vector<TrackingVolume*>& glueVols){
    std::vector<std::shared_ptr<TrackingVolume>> sharedTops{}, sharedFaces{};
    return fillGlueVolumes(::toRawVec(topLevelVolumes),
                           ::toRawVec(envelopeFaceVolumes),
                           glueFace, glueVols);

}
void TrackingVolumeHelper::fillGlueVolumes(const std::vector<TrackingVolume*>& topLevelVolumes,
                                           const std::vector<TrackingVolume*>& envelopeFaceVolumes,
                                           BoundarySurfaceFace glueFace,
                                           std::vector<TrackingVolume*>& glueVols) {
    // loop over the topLevel Volumes
    auto refVolIter = topLevelVolumes.begin();
    for ( ; refVolIter != topLevelVolumes.end(); ++refVolIter ) {
        // loop over the faceVolumes
        for (auto *envelopeFaceVolume : envelopeFaceVolumes){
            // check whether this volume was assigned to on this face
            if (envelopeFaceVolume==(*refVolIter)) {
                // get the GlueVolumesDescriptor
                GlueVolumesDescriptor& glueVolDescriptor = (*refVolIter)->glueVolumesDescriptor();
                // if the size of glue volumes is 0 -> the referenceVolume is at navigation level
                if ( (glueVolDescriptor.glueVolumes(glueFace)).empty()) {
                    glueVols.push_back(*refVolIter);
                } else {
                    // fill all the sub-volumes described by the glueVolumeDescriptor
                    for (auto *isubNavVol : glueVolDescriptor.glueVolumes(glueFace))
                        glueVols.push_back( isubNavVol  );
                }
            }
        }// loop over envelopeFaceVolumes
    } // loop over reference Volumes
}


/** Execute the glueing  - the input volumes are all on navigation level */
void TrackingVolumeHelper::glueTrackingVolumes(const std::vector<std::shared_ptr<TrackingVolume>>& glueVols,
                                               BoundarySurfaceFace firstFace,
                                               BoundarySurfaceFace secondFace) const {
    glueTrackingVolumes(::toRawVec(glueVols), firstFace, secondFace);

}
void TrackingVolumeHelper::glueTrackingVolumes(const std::vector<TrackingVolume*>& glueVols,
                                               BoundarySurfaceFace firstFace,
                                               BoundarySurfaceFace secondFace) const {

    if (glueVols.size()<2) {
        ATH_MSG_VERBOSE( "Nothing to do in glueVolumes() " );
        return;
    }


    ATH_MSG_VERBOSE( " glueTrackingVolumes() called with boundary faces " << static_cast<int>(firstFace)
                    << " and " << static_cast<int>(secondFace) << "." );

    // the iterators through the volumes
    std::vector<TrackingVolume*>::const_iterator firstVol  = glueVols.begin();
    std::vector<TrackingVolume*>::const_iterator secondVol = firstVol + 1;
    for ( ; secondVol != glueVols.end(); ++firstVol, ++secondVol) {

        if (msgLvl(MSG::VERBOSE))
            ATH_MSG_VERBOSE( "Processing '" << (*firstVol)->volumeName() << "' and '" << (*secondVol)->volumeName() << "'." );

        // get the glue volume descriptors to see that we have all subvolumes
        GlueVolumesDescriptor& glueDescr1 = (*firstVol)->glueVolumesDescriptor();
        GlueVolumesDescriptor& glueDescr2 = (*secondVol)->glueVolumesDescriptor();

        // glue volumes at navigation level
        std::vector<TrackingVolume*> glueVols1{}, glueVols2{};
        glueVols1 = glueDescr1.glueVolumes(firstFace);
        glueVols2 = glueDescr2.glueVolumes(secondFace);

        // trivial cases
        // (glue one to the other)
        if (glueVols1.empty() && glueVols2.empty()) {
            glueTrackingVolumes(**firstVol,firstFace,**secondVol,secondFace);
            continue;
            // (glue one to many)
        } else if (glueVols1.empty() && !glueVols2.empty()) {
            glueVols1.push_back(*firstVol);
            // (glue the other one to many)
        } else if (!glueVols1.empty() && glueVols2.empty()) {
            glueVols2.push_back(*secondVol);
        }

        // non-trivial case :: array against array
        // in Z : assume 2dim R/Phi
        if (firstFace==negativeFaceXY || firstFace==positiveFaceXY ) {
            // turn both vectors into R/Phi 2dim binnedArrays; assume equidistant binning in Phi
            std::shared_ptr<BinnedArray<TrackingVolume>> sgv1{m_trackingVolumeArrayCreator->cylinderVolumesArrayInPhiR(glueVols1,true)};
            std::shared_ptr<BinnedArray<TrackingVolume>> sgv2{m_trackingVolumeArrayCreator->cylinderVolumesArrayInPhiR(glueVols2,true)};

            // array vs. array in Z
            if (glueVols2.size()>1)
                for (auto & vol : glueVols1) setOutsideTrackingVolumeArray( *vol, firstFace, sgv2 );
            else
                for (auto & vol : glueVols1) setOutsideTrackingVolume( *vol, firstFace, glueVols2[0] );

            if (glueVols1.size()>1)
                for (auto & vol : glueVols2) setOutsideTrackingVolumeArray( *vol, secondFace, sgv1 );
            else
                for (auto & vol : glueVols2) setOutsideTrackingVolume( *vol, secondFace, glueVols1[0] );


        } else {
            // turn both vectors into Z/Phi 2dim binnedArrays; assume equidistant binning in Phi
            std::shared_ptr<BinnedArray<TrackingVolume>> sgv1{m_trackingVolumeArrayCreator->cylinderVolumesArrayInPhiZ(glueVols1,true)};
            std::shared_ptr<BinnedArray<TrackingVolume>> sgv2{m_trackingVolumeArrayCreator->cylinderVolumesArrayInPhiZ(glueVols2,true)};

            // the glue cases -----------------------------------------------------------------------------------
            // handle the tube with care !
            // first vol
            for (auto & vol : glueVols1) {
                // set the array as the outside array of the firstVol
                if (firstFace != tubeInnerCover) {
                    if (glueVols2.size()>1)
                        setOutsideTrackingVolumeArray( *vol, firstFace, sgv2 );
                    else
                        setOutsideTrackingVolume( *vol, firstFace, glueVols2[0] );
                } else {
                    if (glueVols2.size()>1){
                        setInsideTrackingVolumeArray( *vol, firstFace, sgv2 );
                        setOutsideTrackingVolume( *vol, firstFace, vol );
                    } else {
                        setInsideTrackingVolume( *vol, firstFace, glueVols2[0] );
                        setOutsideTrackingVolume( *vol, firstFace, vol );
                    }
                }
            }
            // second
            for (auto & vol : glueVols2) {
                // set the array as the outside array of the secondVol
                if (secondFace != tubeInnerCover)
                    setOutsideTrackingVolumeArray( *vol, secondFace, sgv1 );
                else {
                    setInsideTrackingVolumeArray( *vol, secondFace, sgv1 );
                    setOutsideTrackingVolume( *vol, secondFace, vol );
                }
            }
        }
    }
}

std::unique_ptr<LayerMaterialProperties>
    TrackingVolumeHelper::layerMaterialProperties(const Surface& boundarySurface) const {

  std::unique_ptr<LayerMaterialProperties> layerMaterial{};

  if (boundarySurface.type() == SurfaceType::Cylinder){
        const CylinderBounds* cb = dynamic_cast<const CylinderBounds*>(&boundarySurface.bounds());
        if (!cb) throw std::logic_error("Not CylinderBounds");
        // --------------- material estimation ----------------------------------------------------------------
        // -- material with 1D binning
        double hz = cb->halflengthZ();
        double r  = cb->r();
        BinUtility layerBinUtilityZ(m_barrelLayerBinsZ, -hz, hz, open, binZ);
        if (m_barrelLayerBinsPhi==1){
            layerMaterial = std::make_unique<BinnedLayerMaterial>(layerBinUtilityZ);
        } else  { // -- material with 2D binning
            BinUtility layerBinUtilityRPhiZ(m_barrelLayerBinsPhi, -r*M_PI, r*M_PI, closed,binRPhi);
            layerBinUtilityRPhiZ += layerBinUtilityZ;
            layerMaterial = std::make_unique<BinnedLayerMaterial>(layerBinUtilityRPhiZ);
        }
        // --------------- material estimation ----------------------------------------------------------------
  }
  if (boundarySurface.type() == SurfaceType::Disc){
      // --------------- material estimation ----------------------------------------------------------------
      const DiscBounds* db = dynamic_cast<const DiscBounds*>(&boundarySurface.bounds());
      if (!db) throw std::logic_error("Not DiscBounds");
      double rMin = db->rMin();
      double rMax = db->rMax();
      BinUtility layerBinUtilityR(m_endcapLayerBinsR,rMin,rMax,open, binR);
      // -- material with 1D binning
      if (m_endcapLayerBinsPhi==1){
          layerMaterial =  std::make_unique<BinnedLayerMaterial>(layerBinUtilityR);
      } else { // -- material with 2D binning
          BinUtility layerBinUtilityPhi(m_endcapLayerBinsPhi,-M_PI,M_PI,closed,binPhi);
          layerBinUtilityR += layerBinUtilityPhi;
          layerMaterial = std::make_unique<BinnedLayerMaterial>(layerBinUtilityR);
      }
      // --------------- material estimation ----------------------------------------------------------------
  }
  // return what you have
  return layerMaterial;
}



/** Simply forward to base class method to enhance friendship relation */
void TrackingVolumeHelper::setInsideTrackingVolume(TrackingVolume& tvol,
                                                   BoundarySurfaceFace face,
                                                   TrackingVolume* insidevol) const {
    TrackingVolumeManipulator::setInsideVolume( tvol, face, insidevol );
}


/** Simply forward to base class method to enhance friendship relation */
void TrackingVolumeHelper::setInsideTrackingVolumeArray(TrackingVolume& tvol,
                                                        BoundarySurfaceFace face,
                                                        BinnedArray<TrackingVolume>* insidevolarray) const {
    TrackingVolumeManipulator::setInsideVolumeArray(tvol,face,insidevolarray);
}


/** Simply forward to base class method to enhance friendship relation */
void TrackingVolumeHelper::setInsideTrackingVolumeArray(TrackingVolume& tvol,
                                                        BoundarySurfaceFace face,
                                                        std::shared_ptr<BinnedArray<TrackingVolume> > insidevolarray) const {
    TrackingVolumeManipulator::setInsideVolumeArray(tvol,face,insidevolarray);
}


/** Simply forward to base class method to enhance friendship relation */
void TrackingVolumeHelper::setOutsideTrackingVolume(TrackingVolume& tvol,
                                                    BoundarySurfaceFace face,
                                                    TrackingVolume* outsidevol) const {
    ATH_MSG_VERBOSE( "     -> Glue '" << outsidevol->volumeName() << "' at face " << face << " to '" << tvol.volumeName() << "'.");
    TrackingVolumeManipulator::setOutsideVolume( tvol, face, outsidevol );
}


/** Simply forward to base class method to enhance friendship relation */
void TrackingVolumeHelper::setOutsideTrackingVolumeArray(TrackingVolume& tvol,
                                                         BoundarySurfaceFace face,
                                                         BinnedArray<TrackingVolume>* outsidevolarray) const {
    unsigned int numVols = outsidevolarray->arrayObjects().size() ;
    ATH_MSG_VERBOSE( "     -> Glue " << numVols << " volumes at face " << face << " to '" << tvol.volumeName() );
    TrackingVolumeManipulator::setOutsideVolumeArray( tvol, face, outsidevolarray );
}


/** Simply forward to base class method to enhance friendship relation */
void TrackingVolumeHelper::setOutsideTrackingVolumeArray(TrackingVolume& tvol,
                                                         BoundarySurfaceFace face,
                                                         std::shared_ptr<BinnedArray<TrackingVolume> > outsidevolarray) const {
    unsigned int numVols = outsidevolarray.get()->arrayObjects().size() ;
    ATH_MSG_VERBOSE( "     -> Glue " << numVols << " volumes at face " << face << " to '" << tvol.volumeName() );
    TrackingVolumeManipulator::setOutsideVolumeArray( tvol, face, outsidevolarray );
}
}
