/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackingGeometry.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKGEOMETRY_TRACKINGGEOMETRY_H
#define TRKGEOMETRY_TRACKINGGEOMETRY_H

// Amg
#include "GeoPrimitives/GeoPrimitives.h"
// Trk
#include "TrkDetDescrUtils/GeometrySignature.h"
#include "TrkGeometry/TrackingVolume.h"
// CLASS DEF
#include "AthenaKernel/CLASS_DEF.h"
// STL
#include <map>
// ATH_MSG macros
#include "AthenaBaseComps/AthMsgStreamMacros.h"
//boost
#include <boost/iterator/transform_iterator.hpp>
#include <boost/range.hpp>

#include "CxxUtils/checker_macros.h"

class MsgStream;

// must be known to declare friend class
namespace Muon {
class MuonTrackingGeometryBuilderCond;
}

namespace Trk {

/** @enum NavigationLevel
    destinguishes an association TrackingGeometry with
    one for global search */

enum NavigationLevel
{
  noNavigation = 0,
  globalSearch = 1,
  association = 2
};

class TrackingVolume;
class Layer;
class MaterialProperties;

/**
  @class TrackingGeometry

  The TrackingGeometry class is the owner of the constructed TrackingVolumes.
  It enables both, a global search for an asociatedVolume
  (respectively, if existing, a global search of an associated Layer or the next
  associated Layer), such as a continous navigation by BoundarySurfaces between
  the confined TrackingVolumes.

  @author Andreas.Salzburger@cern.ch
  @author Christos Anastopoulos (Athena MT modifications)

  */
class TrackingGeometry
{

  /** Give the GeometryBuilder friend rights */
  friend class GeometryBuilder;

  friend class GeometryBuilderCond;


  class constTransformLayerMap
  {
  public:
    std::pair<const Trk::Layer*, int> operator()(
      const std::pair<Trk::Layer*, int>& in) const
    {
      return std::pair<const Trk::Layer*, int>{ in.first, in.second };
    }
  };

  using iterator_convert_const_t =
    boost::transform_iterator<constTransformLayerMap,
                              std::map<Trk::Layer*, int>::const_iterator>;

public:

  using constMapRange_t = boost::iterator_range<iterator_convert_const_t>;
  /** Constructor */
  TrackingGeometry(TrackingVolume* highestVolume,
                   NavigationLevel navlevel = globalSearch);
  /** Destructor */
  virtual ~TrackingGeometry();

  /** return the world */
  const TrackingVolume* highestTrackingVolume() const;
  TrackingVolume* highestTrackingVolume();


  /** return the lowest tracking Volume */
  const TrackingVolume* lowestTrackingVolume(const Amg::Vector3D& gp) const;

  /** return the vector of lowest detached tracking Volume(->overlaps) */
  std::vector<const Trk::DetachedTrackingVolume*>
  lowestDetachedTrackingVolumes(const Amg::Vector3D& gp) const;

  /** return the lowest static tracking Volume */
  const TrackingVolume* lowestStaticTrackingVolume(
    const Amg::Vector3D& gp) const;

  /** return the tracking Volume by name, 0 if it doesn't exist */
  const TrackingVolume* trackingVolume(const std::string& name) const;

  /** Forward the associated Layer information */
  const Layer* associatedLayer(const Amg::Vector3D& gp) const;

  /** Forward the next Layer information */
  const Layer* nextLayer(const Amg::Vector3D& gp,
                         const Amg::Vector3D& mom,
                         bool skipNavLayer = false) const;

  /** Closest Material Layer - used for the mapping option */
  template<class T>
  LayerIntersection<Amg::Vector3D> closestMaterialLayer(
    const T& pars,
    PropDirection pDir = Trk::alongMomentum,
    const BoundaryCheck& bchk = true) const;
  /** check position at volume boundary */
  static bool atVolumeBoundary(const Amg::Vector3D& gp,
                               const TrackingVolume* vol,
                               double tol);

  /** check position at volume boundary + navigation link */
  static bool atVolumeBoundary(const Amg::Vector3D& gp,
                               const Amg::Vector3D& mom,
                               const TrackingVolume* vol,
                               const TrackingVolume*& nextVol,
                               Trk::PropDirection dir,
                               double tol);

  /** Return the unique BoundarySurfaces with MaterialInformation */
  const std::map<Layer*, int>& boundaryLayers() ;
  constMapRange_t  boundaryLayers() const;

  size_t  numBoundaryLayers() const;
  /** Return the Navigation Level - only one TrackingGeometry can have full
     association to GeoModel */
  NavigationLevel navigationLevel() const;

  /** Print the summary of volume Hierarchy of the TrackingGeometry */
  void printVolumeHierarchy(MsgStream& msgstream) const;

  /** indexLayers : method to re-set the index of the layers, depending on
   * geometrySignature */
  void indexStaticLayers(GeometrySignature geosit, int offset = 0);

  void dump(MsgStream& out, const std::string& head) const;

  // get ownership of Muon Tracking Geometry Elements
  void addToGarbage(std::vector<std::unique_ptr<Trk::DetachedTrackingVolume>>&& garbageVec);
  void addToGarbage(std::vector<std::unique_ptr<Trk::TrackingVolume>>&& garbageVec);


 private:
  /** Geometry Builder busineess:
      the geometry builder has to sign*/
  void sign(GeometrySignature geosit, GeometryType geotype = Static);

  /** Geometry Builder busineess:
      set all contained surfaces TG owned - this should save memory and avoid
     surface copying
      - prints compactification statistics */
  void compactify(MsgStream& msgstream, TrackingVolume* vol = nullptr);

  /**  Geometry Builder business:
       synchronize all layers to enclosed volume dimensions */
  void synchronizeLayers(MsgStream& msgstream, TrackingVolume* vol = nullptr);

  /** private method the Navigation Level */
  void registerNavigationLevel(NavigationLevel navlevel);

  /** private method to register recursively the tracking volumes */
  void registerTrackingVolumes(TrackingVolume& tvol,
                               TrackingVolume* mvol = nullptr,
                               int lvl = 0);

  /**  private method to be called from GeometryBuilder: return the world with
   * ownership */
  TrackingVolume* checkoutHighestTrackingVolume();

  /** print VolumeInformation with Level */
  void printVolumeInformation(MsgStream& msgstream,
                              const TrackingVolume& tvol,
                              int lvl) const;

  static void dumpLayer(MsgStream& out,
                        const std::string& head,
                        const Layer* layer);

  /** The known world - and the beam */
  TrackingVolume* m_world{};

  /** The unique boundary Layers */
  std::map<Layer*, int> m_boundaryLayers;

  /** The Volumes in a map for later finding */
  std::map<const std::string, const TrackingVolume*> m_trackingVolumes;

  /** The Navigation level for identification */
  NavigationLevel m_navigationLevel;

  /** keep ownership of MuonTrackingGeometry elements in here */
  // muon chambers
  std::vector<std::shared_ptr<DetachedTrackingVolume>> m_detachedVolGarbage{};  
  std::vector<std::shared_ptr<TrackingVolume>> m_trkVolumeGarbage{};
 
};

} // end of namespace
#include "TrkGeometry/TrackingGeometry.icc"
CLASS_DEF(Trk::TrackingGeometry, 167645219, 1)
#include "AthenaKernel/CondCont.h"
CONDCONT_MIXED_DEF(Trk::TrackingGeometry, 119021535);

#endif // TRKGEOMETRY_TRACKINGGEOMETRY_H
