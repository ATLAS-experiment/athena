/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKINGGEOMETRY_STAGEDTRACKINGGEOMETRYBUILDERIMPL_H
#define INDETTRACKINGGEOMETRY_STAGEDTRACKINGGEOMETRYBUILDERIMPL_H

//Trk
#include "TrkDetDescrInterfaces/IGeometryBuilderCond.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeCreator.h"
#include "TrkDetDescrInterfaces/ILayerArrayCreator.h"
#include "TrkDetDescrUtils/BinningType.h"
#include "TrkGeometry/TrackingVolumeManipulator.h"
#include "TrkGeometry/TrackingGeometry.h"
// EnvelopeDefinitionService
#include "SubDetectorEnvelopes/IEnvelopeDefSvc.h"
// Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/CachedUniquePtr.h"
#include "CxxUtils/checker_macros.h"
// Gaudi
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
// STL
#include <vector>
#include <string>

#include "CxxUtils/checker_macros.h"

#ifndef TRKDETDESCR_TAKESMALLERBIGGER
#define TRKDETDESCR_TAKESMALLERBIGGER
#define takeSmaller(current,test) current = current < test ? current : test
#define takeBigger(current,test)  current = current > test ? current : test
#define takeSmallerBigger(cSmallest, cBiggest, test) takeSmaller(cSmallest, test); takeBigger(cBiggest, test)
#endif


namespace Trk {
 class Layer;
 class Material;
}

namespace InDet {
struct LayerSetup
{

  // the layer cache
  std::vector<Trk::Layer*> negativeLayers;
  std::vector<Trk::Layer*> centralLayers;
  std::vector<Trk::Layer*> positiveLayers;

  // center information
  double minRadiusCenter;
  double maxRadiusCenter;
  double zExtendCenter;
  int binningCenter;

  // endcap information
  bool buildEndcap;
  double minRadiusEndcap;
  double maxRadiusEndcap;
  double minZextendEndcap;
  double maxZextendEndcap;
  int binningEndcap;

  // full setup information
  double zSector;
  double rMin;
  double rMax;
  double zMax;

  std::string identification;
  int colorCode;

  LayerSetup(const std::string& idName, int cCode,
             const std::vector<Trk::Layer*>& negLayers,
             const std::vector<Trk::Layer*>& cenLayers,
             const std::vector<Trk::Layer*>& posLayers, double minRc,
             double maxRc, double zC, int binC, bool bec = false,
             double minRe = 0., double maxRe = 0., double zMinE = 0.,
             double zMaxE = 0., int binE = 0)
      : negativeLayers(negLayers),
        centralLayers(cenLayers),
        positiveLayers(posLayers),
        minRadiusCenter(minRc),
        maxRadiusCenter(maxRc),
        zExtendCenter(zC),
        binningCenter(binC),
        buildEndcap(bec),
        minRadiusEndcap(minRe),
        maxRadiusEndcap(maxRe),
        minZextendEndcap(zMinE),
        maxZextendEndcap(zMaxE),
        binningEndcap(binE),
        identification(idName),
        colorCode(cCode) {
    rMin =
      minRadiusCenter < minRadiusEndcap ? minRadiusCenter : minRadiusEndcap;
    rMax =
      maxRadiusCenter > maxRadiusEndcap ? maxRadiusCenter : maxRadiusEndcap;
    zMax = zExtendCenter > maxZextendEndcap ? zExtendCenter : maxZextendEndcap;
    zSector =
      buildEndcap ? 0.5 * (zExtendCenter + minZextendEndcap) : zExtendCenter;
  }
};


  /** @class StagedTrackingGeometryBuilderImpl

      New Geometry builder that adapts to different layer setups

      Only a few parameters are not automated:
       - m_outwardsFraction: this defines how much you orient yourself on the next bigger layer
                             if you wrap an outer volume around an inner 0.5 would lead to a boundary fully in bewteen
                            1. at the outer boundary, 0. at the inner boundary

      @author Andreas.Salzburger@cern.ch

    */

  class ATLAS_NOT_THREAD_SAFE StagedTrackingGeometryBuilderImpl : //const_cast
    public AthAlgTool,
    public Trk::TrackingVolumeManipulator {


    public:
      /** Constructor */
      StagedTrackingGeometryBuilderImpl(const std::string&,const std::string&,const IInterface*);

      /** Destructor */
      virtual ~StagedTrackingGeometryBuilderImpl() = default;

      /** AlgTool initialize method.*/
      virtual StatusCode initialize() override;
      /** AlgTool finalize method */
      virtual StatusCode finalize() override;

    protected:

      std::unique_ptr<Trk::TrackingGeometry> trackingGeometryImpl(
        std::vector<InDet::LayerSetup>& layerSetups,
        double maximumLayerExtendZ,
        double maximumLayerRadius,
        double envelopeVolumeHalfZ,
        double envelopeVolumeRadius) const;
 
      /** Private helper method to estimate the layer dimensions */
      void estimateLayerDimensions(const std::vector<Trk::Layer*>& layers,
                                   double& rMin,
                                   double& rMax,
                                   double& zMin,
                                   double& zMax) const;

      /** Private helper method, creates a TrackingVolume - and checks if
         configured - for Ring Layout
            - in case a ring layout is given, it creates the corresponding
         sub-volumes and updates the radius
            */
      Trk::TrackingVolume* createTrackingVolume(
        const std::vector<Trk::Layer*>& layers,
        double innerRadius,
        double& outerRadius,
        double zMin,
        double zMax,
        const std::string& volumeName,
        Trk::BinningType btype,
        bool doAdjustOuterRadius = true) const;

      /** Private helper method, creates and packs a triple containing of NegEndcap-Barrel-PosEndcap volumes */
      Trk::TrackingVolume* packVolumeTriple(
        const std::vector<Trk::TrackingVolume*>& negVolumes,
        const std::vector<Trk::TrackingVolume*>& centralVolumes,
        const std::vector<Trk::TrackingVolume*>& posVolumes,
        const std::string& baseName = "UndefinedVolume") const;
      /** Private helper method, estimates the overal dimensions */
      LayerSetup estimateLayerSetup(
        const std::string& idName,
        size_t ils,
        const std::vector<Trk::Layer*>& negLayers,
        const std::vector<Trk::Layer*>& centralLayers,
        const std::vector<Trk::Layer*>& posLayers,
        double maxR,
        double maxZ) const;

      /** Private helper method to check if a sector is compatible with the cache */
      bool setupFitsCache(
        LayerSetup& layerSetup,
        std::vector<InDet::LayerSetup>& layerSetupCache) const;

      /** Private helper method to flush the cache into the id volumes - return
       * volume is the one to be provided */
      Trk::TrackingVolume* createFlushVolume
      (std::vector<InDet::LayerSetup>& layerSetupCache,
       double innerRadius,
       double& outerRadius,
       double extendZ) const;

      /** Private helper method, creates and packs a triple containing of NegEndcap-Barrel-PosEndcap layers
          - in case of a ring layout the subvolumes are created and the rMax is adapted
         */
      Trk::TrackingVolume* packVolumeTriple
      (LayerSetup& layerSetup,
       double rMin,
       double& rMax,
       double zMin,
       double zPosCentral) const;

      /** Private helper method for detection of Ring layout */
      bool ringLayout(const std::vector<Trk::Layer*>& layers, std::vector<double>& rmins, std::vector<double>& rmaxs) const;

      /** helper method needed for the Ring layout */
      void checkForInsert(std::vector<double>& radii, double radius) const;
      void checkForInsert(double rmin, double rmax, std::vector<std::pair<double, double>>& radii) const;

      /** Private helper method for merging of rings with z-overlap */
      std::vector<Trk::Layer*> checkZoverlap(std::vector<Trk::Layer*>& lays) const;
      Trk::Layer* mergeDiscLayers(std::vector<Trk::Layer*>& dlays) const;

      // material configuration
      CxxUtils::CachedUniquePtrT<Trk::Material> m_materialProperties;       //!< overal material properties of the ID

      // Configurable Properties
      //!< Helper Tool to create TrackingVolumes
      PublicToolHandle<Trk::ITrackingVolumeCreator> m_trackingVolumeCreator{
          this, "TrackingVolumeCreator",
          "Trk::CylinderVolumeCreator/CylinderVolumeCreator"};
      PublicToolHandle<Trk::ILayerArrayCreator> m_layerArrayCreator{
          this, "LayerArrayCreator",
          "Trk::LayerArrayCreator/LayerArrayCreator"};  //!< Helper Tool to
                                                        //!< create BinnedArrays
      // configurations for the layer builders
      IntegerArrayProperty m_layerBinningTypeCenter{
          this,
          "LayerBinningTypeCenter",
          {}};  //!< binning type for the provided layers
      IntegerArrayProperty m_layerBinningTypeEndcap{
          this,
          "LayerBinningTypeEndcap",
          {}};  //!< binning type for the provided layers
      IntegerArrayProperty m_colorCodesConfig{
          this, "ColorCodes", {}};  //!< Color codes

      // enclosing endcap/cylinder layer
      ServiceHandle<IEnvelopeDefSvc> m_enclosingEnvelopeSvc{
          this, "EnvelopeDefinitionSvc",
          "AtlasEnvelopeDefSvc"};  //!< the service to provide the ID envelope  size
      DoubleArrayProperty m_enclosingCylinderRadius{
          this,
          "VolumeEnclosureCylinderRadii",
          {}};  //!< the cylinder layer inside the enclosing volume
      DoubleArrayProperty m_enclosingDiscPositionZ{
          this,
          "VolumeEnclosureDiscPositions",
          {}};  //!< the disc position inside the enclosing volume

      DoubleProperty m_layerEnvelopeCover{
          this, "EnvelopeCover",
          2 * Gaudi::Units::mm};  //!< innermost - outermost
      BooleanProperty m_buildBoundaryLayers{this, "BuildBoundaryLayers",
                                            true};  //!< create boundary layers
      BooleanProperty m_replaceJointBoundaries{
          this, "ReplaceAllJointBoundaries",
          true};  //!< run with replacement of all joint boundaries

      // robust layer indexing
      BooleanProperty m_indexStaticLayers{
          this, "IndexStaticLayers",
          true};  //!< forces robust indexing for layers

      // check for endcap ring layout
      BooleanProperty m_checkForRingLayout{
          this, "CheckForRingLayout",
          false};  //!< this is to check for the endcap ring layout
      DoubleProperty m_ringTolerance{
          this, "MinimalRadialGapForVolumeSplit",
          10 * Gaudi::Units::mm};  //!< the ring tolerance

      // naming schema
      StringProperty m_namespace{this, "VolumeNamespace",
                                 "InDet::"};  //!< identificaton namespace
      // ID container
      StringProperty m_exitVolume{
          this, "ExitVolumeName",
          "InDet::Containers::InnerDetector"};  //!< the final ID container

      // Make room for HGTD (3420 mm < |z| < 3545 mm) within the ID tracking geometry volume
      // This will be filled by the dedicated HGTD Tracking Geometry Builder
      // and volumes will be glued when the combined tracking geometry is built
      BooleanProperty m_removeHGTD{this, "RemoveHGTD", false};
      FloatProperty m_zMinHGTD{this, "ZminHGTD", 3420.f};
  };

  inline void StagedTrackingGeometryBuilderImpl::checkForInsert(std::vector<double>& radii, double radius) const {
      bool exists = false;
      // loop and check
      for (auto& checkr : radii) {
          if ( (checkr-radius)*(checkr-radius) < m_ringTolerance*m_ringTolerance ){
              exists = true; break;
          }
      }
      // insert
      if (!exists) radii.push_back(radius);
      // re-sort
      std::sort(radii.begin(),radii.end());
  }

  inline void StagedTrackingGeometryBuilderImpl::checkForInsert(double rmin, double rmax, std::vector<std::pair<double, double>>& radii) const {

    // range into non-overlapping layers

    if (radii.empty()) radii.push_back(std::pair<double,double>(rmin,rmax));
    //now radii has at least one element
    unsigned int ir=0;
    while ( ir != radii.size() && rmin > radii[ir].second ) ir++;

    if (ir==radii.size()) radii.push_back(std::pair<double,double>(rmin,rmax));
    // insert ?
    else if (rmax<radii[ir].first) radii.insert(radii.begin()+ir,std::pair<double,double>(rmin,rmax));
    // overlaps
    else {
      // resolve low edge
      if (rmin<radii[ir].first) radii[ir].first=rmin;
      // resolve upper edge
      unsigned int imerge = ir;
      //coverity[INTEGER_OVERFLOW:FALSE]
      while (imerge<radii.size()-1 && rmax>radii[imerge+1].first) imerge++;
      radii[ir].second = rmax > radii[imerge].second ? rmax : radii[imerge].second;
      if (imerge>ir) radii.erase(radii.begin()+ir+1,radii.begin()+imerge);
    }
  }


} // end of namespace

#endif //INDETTRACKINGGEOMETRY_STAGEDTRACKINGGEOMETRYBUILDERIMPL_H
