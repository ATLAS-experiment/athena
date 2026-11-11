/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
#include "GaudiKernel/SystemOfUnits.h"
// GeoModel
#include "GeoPrimitives/GeoPrimitives.h"
//
#include "GeoModelKernel/GeoVPhysVol.h"
// Trk
#include "CaloTrackingGeometry/ICaloSurfaceBuilder.h"
#include "TrkDetDescrInterfaces/ICaloTrackingVolumeBuilder.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeCreator.h"
#include "TrkGeometry/Material.h"
// STL
#include <memory>
#include <mutex>
#include <vector>
#include "Gaudi/Property.h"

class TileDetDescrManager;
class CaloDetDescrManager;
class CaloDepthTool;
class StoreGateSvc;

namespace Trk {
class ILayerArrayCreator;
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

  const TileDetDescrManager* m_tileMgr{nullptr}; //!< Calo DetDescrMgr
  Gaudi::Property<std::string> m_tileMgrLocation{this, "TileDetManagerLocation", "Tile"};        //!< Location of the CaloDetDescrMgr

  PublicToolHandle<Trk::ITrackingVolumeCreator> m_trackingVolumeCreator{this, "TrackingVolumeCreator", "Trk::CylinderVolumeCreator/TrackingVolumeCreator"}; //!< Second helper for volume creation

  Gaudi::Property<double> m_tileBarrelEnvelope{this, "BarrelEnvelopeCover", 25.*Gaudi::Units::mm}; //!< envelope Cover of the Barrel

  Gaudi::Property<bool> m_useCaloSurfBuilder{this, "UseCaloSurfBuilder", true}; //!< if true use DetDescr based layering, if false use biequidistant layering
  Gaudi::Property<unsigned int> m_tileBarrelLayersPerSampling{this, "BarrelLayersPerSampling", 1}; //!< if m_useCaloSurfBuilder == true, number of layers per dead material
                                              //!< region or sampling
  PublicToolHandle<ICaloSurfaceBuilder> m_surfBuilder{this, "CaloSurfaceBuilder", "CaloSurfaceBuilder"}; //!< tool required for definition of active volumes

  Gaudi::Property<bool> m_forceSymmetry{this, "ForceVolumeSymmetry", true}; //!< forces volume symmetry between negative/positive part

};

} // end of namespace

#endif // TILETRACKINGGEOMETRY_TILEVOLUMEBUILDER_H

