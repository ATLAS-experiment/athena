/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CylinderVolumeCreator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRTOOLS_CYLINDERVOLUMECREATOR_H
#define TRKDETDESCRTOOLS_CYLINDERVOLUMECREATOR_H


// Trk
#include "TrkDetDescrInterfaces/ITrackingVolumeCreator.h"
#include "TrkVolumes/BoundarySurfaceFace.h"
// Amg
#include "GeoPrimitives/GeoPrimitives.h"
// Gaudi & Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
// STL
#include <string>
#include <vector>

#include "CxxUtils/checker_macros.h"
namespace Trk {

class ILayerArrayCreator;
class ITrackingVolumeArrayCreator;
class ITrackingVolumeHelper;
class Layer;
class CylinderLayer;
class DiscLayer;
class TrackingVolume;
class VolumeBounds;
class CylinderVolumeBounds;
class Material;

/** @class CylinderVolumeCreator

  The concrete implementation for cylindrical TrackingVolume
  objects of the ITrackingVolumeCreator interface

  @author Andreas.Salzburger@cern.ch
 */

class CylinderVolumeCreator
  : public AthAlgTool
  , virtual public ITrackingVolumeCreator
{

public:
  /** Constructor */
  CylinderVolumeCreator(const std::string&,
                        const std::string&,
                        const IInterface*);
  /** Destructor */
  ~CylinderVolumeCreator();

  /** AlgTool initialize method */
  virtual StatusCode initialize() override;

  /** @copydoc ITrackingVolumeCreator::createTrackingVolume(const
   * std::vector<const Layer*>&, Material&, VolumeBounds*,
   * Amg::Transform3D*,bool, const std::string&) const;
   */
  virtual TrackingVolume* createTrackingVolume(
    const std::vector<Layer*>& layers,
    Material& matprop,
    VolumeBounds* volBounds = 0,
    Amg::Transform3D* transform = 0,
    const std::string& volumeName = "UndefinedVolume",
    BinningType btype = arbitrary) const override final;

  /** @copydoc ITrackingVolumeCreator::createTrackingVolume(const
   * std::vector<const Layer*>& , Material&,
   * ,double,double,double,double,bool,const std::string&) const;
   */
  virtual TrackingVolume* createTrackingVolume(
    const std::vector<Layer*>& layers,
    Material& matprop,
    double loc1Min,
    double loc1Max,
    double loc2Min,
    double loc2Max,
    const std::string& volumeName = "UndefinedVolume",
    BinningType btype = arbitrary) const override final;

  /** @copydoc
   * ITrackingVolumeCreator::createGapTrackingVolume(Material&,,double,double,double,double,int,bool,const
   * std::string&) const;
   */
 virtual TrackingVolume* createGapTrackingVolume ( 
    Material& matprop,
    double rMin,
    double rMax,
    double zMin,
    double zMax,
    unsigned int materialLayers,
    bool cylinder = true,
    const std::string& volumeName = "UndefinedVolume") const override final;

  /** @copydoc
   * ITrackingVolumeCreator::createGaoTrackingVolume(Material&,,std::vector<double>&,int,bool,const
   * std::string&) const;
   */
  virtual TrackingVolume* createGapTrackingVolume (
    Material& matprop,
    double rMin,
    double rMax,
    double zMin,
    double zMax,
    const std::vector<double>& layerPositions,
    bool cylinder = true,
    const std::string& volumeName = "UndefinedVolume",
    BinningType btype = arbitrary) const override final;

  /** @copydoc ITrackingVolumeCreator::createContainerTrackingVolume(
          const std::vector<const TrackingVolume*>, const Material&, ,
          const std::string&) const;
   */
  virtual TrackingVolume* createContainerTrackingVolume
  (const std::vector<TrackingVolume*>& volumes,
                        const Material& matprop,
                        const std::string& volumeName = "UndefinedVolume",
                        bool buildBoundaryLayers = false,
                        bool replaceBoundaryFace = false) const override final;

private:
  /** Private method - it estimates the CylinderBounds and Translation of
     layers,
              if given, these are checked against the layer
     positions/dimensions. */

  StatusCode estimateAndCheckDimension(
    const std::vector<Layer*>& layers,
    Trk::CylinderVolumeBounds*& cylBounds,
    Amg::Transform3D*& translation,
    std::vector<CylinderLayer*>& cylLayers,
    std::vector<DiscLayer*>& discLayers,
    double& rMinClean,
    double& rMaxClean,
    double& zMinClean,
    double& zMaxClean,
    BinningType bType = arbitrary) const;

  /** Private method - interglue all volumes contained by a TrackingVolume
              and set the outside glue volumes in the descriptor */
  StatusCode interGlueTrackingVolume
  (TrackingVolume& tVolume,
                        bool rBinned,
                        bool buildBoundaryLayers,
                        bool replaceBoundaryFace = false) const;

  /** Private method - helper method not to duplicate code */
  void addFaceVolumes(TrackingVolume& tvol,
                      Trk::BoundarySurfaceFace bsf,
                      std::vector<Trk::TrackingVolume*>& vols) const;

  /** Private method - glue volume to the other -- use trackingVolume helper */
  void glueTrackingVolumes(TrackingVolume& volumeOne,
                           BoundarySurfaceFace faceOne,
                           TrackingVolume& volumeTwo,
                           BoundarySurfaceFace faceTwo,
                           bool buildBoundaryLayers,
                           bool replaceBoundaryFace = false) const;

  /** Private method - helper method to save some code */
  CylinderLayer* createCylinderLayer(double z,
                                     double r,
                                     double halflength,
                                     double thickness,
                                     int binsPhi,
                                     int binsZ) const;

  /** Private method - helper method to save some code */
  DiscLayer* createDiscLayer(double z,
                             double rMin,
                             double rMax,
                             double thickness,
                             int binsPhi,
                             int binsR) const;

  // helper tools
  //!< A Tool for coherent LayerArray creation
  ToolHandle<ILayerArrayCreator> m_layerArrayCreator;
  //!< Helper Tool to create TrackingVolume Arrays
  ToolHandle<ITrackingVolumeArrayCreator> m_trackingVolumeArrayCreator;
  //!< TrackingVolume helper
  ToolHandle<ITrackingVolumeHelper> m_trackingVolumeHelper;

  double m_passiveLayerThickness; //!< thickness of passive layers
  int m_passiveLayerPhiBins;      //!< bins in phi for the passive layer
  int m_passiveLayerRzBins;       //!< bins in r/z for the passive layer
};
}

#endif

