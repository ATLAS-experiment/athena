/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// LArVolumeBuilder.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef LARTRACKINGGEOMETRY_LARVOLUMEBUILDER_H
#define LARTRACKINGGEOMETRY_LARVOLUMEBUILDER_H

// Athena/Gaudi
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

// STL
#include <vector>


namespace Trk {
class TrackingVolume;
class Material;
class Volume;
class Layer;
}

namespace LAr {

/** @class LArVolumeBuilder

    The LArVolumeBuilder builds the TrackingVolumes for
       - LAr Barrel
       - LAr Inner Endcap
       - LAr Outer Endcap

    The HEC and Forward Calorimeter have to be added later when knowing the
   dimensions of the Tile Calorimter.

    @author Andreas.Salzburger@cern.ch
  */
class LArVolumeBuilder final
  : public AthAlgTool
  , virtual public Trk::ICaloTrackingVolumeBuilder
{

public:
  /** AlgTool style constructor */
  LArVolumeBuilder(const std::string&, const std::string&, const IInterface*);
  /** Destructor */
  virtual ~LArVolumeBuilder();

  /** AlgTool initialize method */
  virtual StatusCode initialize() override final;
  /** AlgTool finalize method */
  virtual StatusCode finalize() override final;

  /** TrackingVolumeBuilder interface method - returns vector of ptrs to tracking Volumes */
  virtual std::vector<Trk::TrackingVolume*> trackingVolumes(
      const CaloDetDescrManager& caloDDM,
      const GeoAlignmentStore* geoAlign) const override final;

 private:
  // ------------- private methods -----------------------------------------
  static void printCheckResult(MsgStream& log, const Trk::TrackingVolume* vol);

  void printInfo(const GeoPVConstLink& pv, const GeoAlignmentStore* gas, int gen = -1) const;
  void printChildren(const GeoPVConstLink& pv,
                     int gen,
                     int igen,
                     const Amg::Transform3D& tr) const;
  GeoPVConstLink getChild(const GeoPVConstLink& mother,
                          const std::string& name,
                          Amg::Transform3D& trIn) const;

  // ------------- private members -----------------------------------------
  StringProperty m_lArMgrLocation{
    this, "LArDetManagerLocation", "LArMgr", "Store Gate key for LAr Detector Manager"};

  //!< helper for volume creation
  ToolHandle<Trk::ITrackingVolumeCreator> m_trackingVolumeCreator{
    this, "TrackingVolumeCreator", "Trk::CylinderVolumeCreator/TrackingVolumeCreator"};

  //!< envelope Cover of the Barrel
  DoubleProperty m_lArBarrelEnvelope{this, "BarrelEnvelopeCover", 25.*Gaudi::Units::mm};
  //!< envelope Cover of the Endcap
  DoubleProperty m_lArEndcapEnvelope{this, "EndcapEnvelopeCover", 25.*Gaudi::Units::mm};

  //!< if true use DetDescr based layering, if false use biequidistant layering
  BooleanProperty m_useCaloSurfBuilder{this, "UseCaloSurfBuilder", true};

  //!< if m_useCaloSurfBuilder == true, number of layers
  //!< per dead material region or sampling
  UnsignedIntegerProperty m_lArLayersPerRegion{this, "LayersPerRegion", 1};

  //!< if true use DetDescr based layering,
  //!< if false use biequidistant layering
  BooleanProperty m_useCaloTrackingGeometryBounds{this, "UseCaloTrackingGeometryBounds", true};

  //!< tool required for DetDescr-based layering
  ToolHandle<ICaloSurfaceBuilder> m_calosurf{this, "CaloSurfaceBuilder", "CaloSurfaceBuilder"};

  // material scaling ( temporary ? )
  FloatProperty m_scale_HECmaterial{this, "ScaleFactor_HECmaterial", 1.1};
};

} // end of namespace


#endif // CALOTRACKINGGEOMETRY_LARVOLUMEBUILDER_H

