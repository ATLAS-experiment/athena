/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// IVertexFitter.h, (c) ATLAS Detector software 2005
///////////////////////////////////////////////////////////////////

#ifndef TRKVERTEXFITTERINTERFACE_IVERTEXFITTER_H
#define TRKVERTEXFITTERINTERFACE_IVERTEXFITTER_H

#include "GaudiKernel/IAlgTool.h"
#include "TrkNeutralParameters/NeutralParameters.h"
#include "TrkParameters/TrackParameters.h"
#include <vector>

// xAOD includes
#include "memory"
#include "xAODTracking/NeutralParticle.h"
#include "xAODTracking/TrackParticleFwd.h"
#include "xAODTracking/Vertex.h"

class EventContext;

/**
 * @class Trk::IVertexFitter
 *
 * Interface class for vertex fitting AlgTools,
 * for more detailed information, take a look at the header file of the actual
 * implementation file. In general, the vertex fitters is an AlgTool,
 * taking a list of tracks or their parameters as an argument and returning a
 * vertex candidate.
 *
 * <br>
 * Modification for the use of VertexSeedFinders (algorithms
 * providing initial point of linearization); February 2006
 *
 * @author Kirill.Prokofiev@cern.ch, Andreas.Wildauer@cern.ch,
 * Giacinto.Piacquadio@physik.uni-freiburg.de
 *
 * -------------------------------------------------------------------------------------------------------
 * Changes:
 *
 * David Shope <david.richard.shope@cern.ch> (2016-04-19)
 *
 * EDM Migration to xAOD - from Trk::VxCandidate to xAOD::Vertex,
 *                         from Trk::RecVertex   to xAOD::Vertex,
 *                         from Trk::Vertex      to Amg::Vector3D
 *
 * Changes for Athena MT RD Schaffer, C Anastopoulos
 */

namespace Trk {
class IVertexFitter : virtual public IAlgTool
{

public:
  DeclareInterfaceID(IVertexFitter, 1, 0);

  /**
   * Virtual destructor
   */
  virtual ~IVertexFitter() = default;


  /**
   * Interface for xAOD::TrackParticle with starting point
   * Event Context aware interface
   */
  virtual std::unique_ptr<xAOD::Vertex> fit(
    const EventContext& ctx,
    const std::vector<const xAOD::TrackParticle*>& vectorTrk,
    const Amg::Vector3D& startingPoint) const = 0;


  /**
   *Interface for xAOD::TrackParticle and xAOD::NeutralParticle with starting
   *point. Event Context aware method
   */
  virtual std::unique_ptr<xAOD::Vertex> fit(
    const EventContext& ctx,
    const std::vector<const xAOD::TrackParticle*>& vectorTrk,
    const std::vector<const xAOD::NeutralParticle*>& vectorNeu,
    const Amg::Vector3D& startingPoint) const = 0;


  /**
   * Interface for xAOD::TrackParticle and xAOD::NeutralParticle with vertex
   * constraint the position of the constraint is ALWAYS the starting point
   * Event Context aware method
   */
  virtual std::unique_ptr<xAOD::Vertex> fit(
    const EventContext& ctx,
    const std::vector<const xAOD::TrackParticle*>& vectorTrk,
    const std::vector<const xAOD::NeutralParticle*>& vectorNeu,
    const xAOD::Vertex& constraint) const = 0;


  /**
   * Interface for xAOD::TrackParticle with vertex constraint
   * the position of the constraint is ALWAYS the starting point
   * Event Context aware method
   */
  virtual std::unique_ptr<xAOD::Vertex> fit(
    const EventContext& ctx,
    const std::vector<const xAOD::TrackParticle*>& vectorTrk,
    const xAOD::Vertex& constraint) const = 0;


  /**
   * Interface for TrackParameters and NeutralParameters with starting point
   * Event Context aware method
   */
  virtual std::unique_ptr<xAOD::Vertex> fit(
    const EventContext& ctx,
    const std::vector<const Trk::TrackParameters*>& perigeeList,
    const std::vector<const Trk::NeutralParameters*>& neutralPerigeeList,
    const Amg::Vector3D& startingPoint) const = 0;


  /**
   * Interface for TrackParameters with starting point
   * Event Context aware method
   */
  virtual std::unique_ptr<xAOD::Vertex> fit(
    const EventContext& ctx,
    const std::vector<const Trk::TrackParameters*>& perigeeList,
    const Amg::Vector3D& startingPoint) const = 0;


  /**
   * Interface for TrackParameters and NeutralParameters with vertex constraint
   * the position of the constraint is ALWAYS the starting point
   * EventContext aware method.
   */
  virtual std::unique_ptr<xAOD::Vertex> fit(
    const EventContext& ctx,
    const std::vector<const Trk::TrackParameters*>& perigeeList,
    const std::vector<const Trk::NeutralParameters*>& neutralPerigeeList,
    const xAOD::Vertex& constraint) const = 0;


  /**
   * Interface for TrackParameters with vertex constraint
   * the position of the constraint is ALWAYS the starting point
   * EventContext aware method.
   */
  virtual std::unique_ptr<xAOD::Vertex> fit(
    const EventContext& ctx,
    const std::vector<const Trk::TrackParameters*>& perigeeList,
    const xAOD::Vertex& constraint) const = 0;


  /**
   * Fit method using the VertexSeedFinder to estimate initial
   * position of the vertex and taking it as a first linearization point
   * (in iterative fitters). EventContext aware method.
   */
  virtual std::unique_ptr<xAOD::Vertex> fit(
    const EventContext& ctx,
    const std::vector<const Trk::TrackParameters*>& perigeeList,
    const std::vector<const Trk::NeutralParameters*>& neutralPerigeeList) const = 0;


  /**
   * Fit method using the VertexSeedFinder to estimate initial
   * position of the vertex and taking it as a first linearization point
   * (in iterative fitters). EventContext aware method.
   */
  virtual std::unique_ptr<xAOD::Vertex> fit(
    const EventContext& ctx,
    const std::vector<const Trk::TrackParameters*>& perigeeList) const = 0;

};
}
#endif

