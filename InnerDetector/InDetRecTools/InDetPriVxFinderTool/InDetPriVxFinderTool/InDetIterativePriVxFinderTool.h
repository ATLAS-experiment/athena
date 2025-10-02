/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 *
 * @class InDet::InDetIterativePriVxFinderTool
 *
 * @author Giacinto Piacquadio (Freiburg University)
 *
 *
 * This class provides an implementation for a primary
 * vertex finding tool, which uses the Adaptive Vertex
 * Fitter to reject outliers not belonging to the primary
 * vertex interaction.
 *
 * ------------------------------------------------------
 * Changes:
 *
 * David Shope <david.richard.shope@cern.ch> (2016-04-26)
 *
 * EDM Migration to xAOD - from Trk::VxCandidate to xAOD::Vertex
 *
 *   findVertex will now always return an xAOD::VertexContainer,
 *   even when using a TrackCollection or a TrackParticleBaseCollection
 *   as input.
 *
 * Dave Casper <dcasper@uci.edu> (2018-03-28)
 *   Migrate beam spot service to conditions handle
 *
 ***************************************************************************/

// implemented using as template the InDetPriVxFinderTool class of A. Wildauer
// and F. Akesson

#ifndef INDETPRIVXFINDERTOOL_INDETITERATIVEPRIVXFINDERTOOL_H
#define INDETPRIVXFINDERTOOL_INDETITERATIVEPRIVXFINDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "GaudiKernel/ToolHandle.h"
#include "InDetRecToolInterfaces/IVertexFinder.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkTrack/TrackCollection.h" // type def ...
/**
 * Forward declarations
 */

#include "xAODTracking/TrackParticleContainerFwd.h"
#include "xAODTracking/TrackParticleFwd.h"
#include "xAODTracking/VertexContainerFwd.h"
#include "xAODTracking/VertexFwd.h"

#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "TrkVertexFitterInterfaces/IImpactPoint3dEstimator.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "TrkVertexFitterInterfaces/IVertexLinearizedTrackFactory.h"
#include "TrkVertexFitterInterfaces/IVertexSeedFinder.h"

namespace Trk {
class Track;
class ITrackLink;
}

namespace InDet {

class InDetIterativePriVxFinderTool final
  : public AthAlgTool
  , virtual public IVertexFinder
{

public:
  /**
   * Constructor
   */

  InDetIterativePriVxFinderTool(const std::string& t,
                                const std::string& n,
                                const IInterface* p);

  /**
   * Destructor
   */

  virtual ~InDetIterativePriVxFinderTool();

  virtual StatusCode initialize() override;

  /**
   * Finding method for Trk::Track collection
   */
  virtual std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*>
  findVertex(const EventContext& ctx,
             const TrackCollection* trackTES) const override;
  /**
   * Finding method for TrackParticle collection
   */
  virtual std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*>
  findVertex(const EventContext& ctx,
             const xAOD::TrackParticleContainer* trackParticles) const override;

  virtual StatusCode finalize() override;

private:
  std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*> findVertex(
    const EventContext& ctx,
    const std::vector<Trk::ITrackLink*>& trackVector) const;

  void removeCompatibleTracks(
    xAOD::Vertex* myxAODVertex,
    std::vector<const Trk::TrackParameters*>& perigeesToFit,
    std::vector<Trk::ITrackLink*>& seedTracks) const;

  void removeAllFrom(std::vector<const Trk::TrackParameters*>& perigeesToFit,
                     std::vector<Trk::ITrackLink*>& seedTracks) const;

  double compatibility(const Trk::TrackParameters& measPerigee,
                       const xAOD::Vertex& vertex) const;

  static void countTracksAndNdf(xAOD::Vertex* myxAODVertex,
                         double& ndf,
                         int& ntracks) ;

  ToolHandle<Trk::IVertexFitter> m_iVertexFitter{ this,
                                                  "VertexFitterTool",
                                                  "Trk::AdaptiveVertexFitter" };
  ToolHandle<InDet::IInDetTrackSelectionTool> m_trkFilter{
    this,
    "TrackSelector",
    "InDet::InDetTrackSelection"
  };

  ToolHandle<Trk::IVertexSeedFinder> m_SeedFinder{ this,
                                                   "SeedFinder",
                                                   "Trk::ZScanSeedFinder" };
  ToolHandle<Trk::IImpactPoint3dEstimator> m_ImpactPoint3dEstimator{
    this,
    "ImpactPoint3dEstimator",
    "Trk::ImpactPoint3dEstimator"
  };

  ToolHandle<Trk::IVertexLinearizedTrackFactory> m_LinearizedTrackFactory{
    this,
    "LinearizedTrackFactory",
    "Trk::FullLinearizedTrackFactory"
  };

  SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey{
    this,
    "BeamSpotKey",
    "BeamSpotData",
    "SG key for beam spot"
  };

  BooleanProperty m_useBeamConstraint{this, "useBeamConstraint", false};
  DoubleProperty m_significanceCutSeeding
    {this, "significanceCutSeeding", 10.};
  DoubleProperty m_maximumChi2cutForSeeding
    {this, "maximumChi2cutForSeeding", 36.};
  IntegerProperty m_maxVertices{this, "maxVertices", 25};

  BooleanProperty m_createSplitVertices{this, "createSplitVertices", false};
  IntegerProperty m_splitVerticesTrkInvFraction
    {this, "splitVerticesTrkInvFraction", 2, "inverse fraction to split tracks (1:N)"};

  BooleanProperty m_reassignTracksAfterFirstFit
    {this, "reassignTracksAfterFirstFit", false};

  BooleanProperty m_doMaxTracksCut{this, "doMaxTracksCut", false};
  UnsignedIntegerProperty m_maxTracks{this, "MaxTracks", 5000};

  /**
   * Internal method to print the parameters setting
   */
  void printParameterSettings();

}; // end of class definitions
} // end of namespace definitions
#endif

