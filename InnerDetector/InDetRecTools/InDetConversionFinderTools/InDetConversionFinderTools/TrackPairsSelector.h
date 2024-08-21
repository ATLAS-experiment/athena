/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETCONVERSIONFINDERTOOLS_TRACKPAIRSSELECTOR_H
#define INDETCONVERSIONFINDERTOOLS_TRACKPAIRSSELECTOR_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "InDetConversionFinderTools/ConversionFinderUtils.h"
#include "Particle/TrackParticle.h"
#include "TrkVertexSeedFinderUtils/ITrkDistanceFinder.h"
#include "xAODTracking/TrackParticleFwd.h"

namespace Trk {
class Track;
}

namespace InDet {
/**
   @class TrackPairsSelector
   This class selects track pairs for conversion finder
   @author Tatjana Lenz, Thomas Koffas
*/

class TrackPairsSelector : public AthAlgTool
{

public:
  struct Cache
  {
    /** Distance of closest approach between the tracks **/
    float m_distance = 9999.;
    /** Delta cot theta between the tracks **/
    float m_deltaCotTheta = 9999.;
    /** Distance difference between initial hits of tracks */
    float m_deltaInit = 9999.;
  };

  TrackPairsSelector(const std::string& type,
                     const std::string& name,
                     const IInterface* parent);

  virtual ~TrackPairsSelector() = default;

  static const InterfaceID& interfaceID();

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  /** Track pair selectors.Return true if the argument track fulfills the
   * selection */
  bool selectTrackParticlePair(const xAOD::TrackParticle* trkPpos,
                               const xAOD::TrackParticle* trkPneg,
                               Cache& cache) const;

  bool selectTrackPair(const Trk::Track* trkpos,
                       const Trk::Track* trkneg) const;

  /** Return a map with the values calculated for the last pair
   * to decorate the vertex once it is created **/
  static std::map<std::string, float> getLastValues(const Cache& cache) ;

private:
  /**Conversion helper tool.  */
  ToolHandle<InDet::ConversionFinderUtils> m_helpertool{
    this,
    "ConversionFinderHelperTool",
    "InDet::ConversionFinderUtils",
    "Conversion helper tool"
  };
  /** Distance of minimum approach tool   */
  ToolHandle<Trk::ITrkDistanceFinder> m_distanceTool{
    this,
    "DistanceTool",
    "Trk::SeedNewtonTrkDistanceFinder",
    "Distance of minimum approach tool"
  };
  /** Properties for track selection: all cuts are ANDed */

  DoubleProperty m_maxR
    {this, "MaxFirstHitRadius", 500.,
     "Maximum initial hit radius in order to apply the impact point cut"};
  DoubleProperty m_MinTrkAngle
    {this, "MinTrackAngle", 0.,
     "Minimum allowed angle between decay tracks. Used only in V0 reconstruction."};
  DoubleArrayProperty m_maxDist
    {this, "MaxDistBetweenTracks", {8., 80., 45.},
     "Maximum allowed distance of minimum approach"};
  DoubleArrayProperty m_etaCut
    {this, "MaxEta", {0.8, 1.2, 1.2},
     "Maximum eta difference between tracks in pair"};
  DoubleArrayProperty m_initCut
    {this, "MaxInitDistance", {10000., 10000., 10000.},
     "Maximum distance difference between initial  hits of tracks in pair"};
};
}
#endif // INDETCONVERSIONFINDERTOOLS_TRACKPAIRSSELECTOR_H

