/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TileVolumeBuilder.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TILETRACKINGGEOMETRY_TILEVOLUMEBUILDER_H
#define TILETRACKINGGEOMETRY_TILEVOLUMEBUILDER_H

// Gaudi
#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ToolHandle.h"
// GeoModel
#include "GeoPrimitives/GeoPrimitives.h"
//
#include "GeoModelKernel/GeoVPhysVol.h"
// Trk
#include "CaloTrackingGeometry/ICaloSurfaceBuilder.h"
#include "TrkDetDescrInterfaces/ICaloTrackingVolumeBuilder.h"
#include "TrkGeometry/Material.h"
// STL
#include <memory>
#include <mutex>
#include <vector>

class TileDetDescrManager;
class CaloDetDescrManager;
class CaloDepthTool;
class StoreGateSvc;

namespace Trk {
class ILayerArrayCreator;
class ITrackingVolumeCreator;
class TrackingVolume;
class Volume;
class Layer;
}

namespace Tile {

/** @class TileVolumeBuilder

    The TileVolumeBuilder builds the TrackingVolumes for
       - Tile Barrel
       - Tile Extended Barrel

    @author Andreas.Salzburger@cern.ch

    dense volume description & layer/cell navigation
    @author sarka.todorova@cern.ch

  */
class TileVolumeBuilder final
  : public AthAlgTool
  , virtual public Trk::ICaloTrackingVolumeBuilder
{

public:
  /** AlgTool style constructor */
  TileVolumeBuilder(const std::string&, const std::string&, const IInterface*);
  /** Destructor */
  virtual ~TileVolumeBuilder();

  /** AlgTool initialize method */
  virtual StatusCode initialize() override final;

  /** TrackingVolumeBuilder interface method - returns vector of ptrs
   * to volumes. The caller assumes ownership of the pointers*/
  virtual std::vector<Trk::TrackingVolume*> trackingVolumes(
      const CaloDetDescrManager& caloDDM,
      const GeoAlignmentStore* geoAlign) const override final;

 private:
  static void printCheckResult(MsgStream& log, const Trk::TrackingVolume* vol) ;

  void printInfo(const GeoPVConstLink& pv) const;
  void printChildren(const GeoPVConstLink& pv, int igen, const Amg::Transform3D& trIn) const;

  void throwIntoGarbage(std::unique_ptr<Trk::Material> mat) const;

  const TileDetDescrManager* m_tileMgr; //!< Calo DetDescrMgr
  std::string m_tileMgrLocation;        //!< Location of the CaloDetDescrMgr

  ToolHandle<Trk::ITrackingVolumeCreator> m_trackingVolumeCreator; //!< Second helper for volume creation

  double m_tileBarrelEnvelope; //!< envelope Cover of the Barrel

  bool m_useCaloSurfBuilder; //!< if true use DetDescr based layering, if false use biequidistant layering
  unsigned int m_tileBarrelLayersPerSampling; //!< if m_useCaloSurfBuilder == true, number of layers per dead material
                                              //!< region or sampling
  ToolHandle<ICaloSurfaceBuilder> m_surfBuilder; //!< tool required for definition of active volumes

  bool m_forceSymmetry; //!< forces volume symmetry between negative/positive part

};

} // end of namespace

#endif // TILETRACKINGGEOMETRY_TILEVOLUMEBUILDER_H

