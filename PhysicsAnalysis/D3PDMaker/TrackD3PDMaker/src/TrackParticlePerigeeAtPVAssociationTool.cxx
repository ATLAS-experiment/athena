/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file TrackD3PDMaker/src/TrackParticlePerigeeAtPVAssociationTool.cxx
 * @author remi zaidan <remi.zaidan@cern.ch>
 * @date Feb, 2010
 * @brief Associate from a TrackParticle to its default Perigee.
 */

#include "TrackParticlePerigeeAtPVAssociationTool.h"

#include "TrkParameters/TrackParameters.h"
#include "xAODTracking/VertexContainer.h"
#include "AthenaKernel/errorcheck.h"
#include "VxVertex/PrimaryVertexSelector.h"

namespace D3PD {

/**
 * @brief Standard Gaudi tool constructor.
 * @param type The name of the tool type.
 * @param name The tool name.
 * @param parent The tool's Gaudi parent.
 */
TrackParticlePerigeeAtPVAssociationTool::TrackParticlePerigeeAtPVAssociationTool
  (const std::string& type,
   const std::string& name,
   const IInterface* parent)
    : Base (type, name, parent),
      m_resolver ("TrackParticlePerigeeAtPVAssociationTool",
                  evtStore(),
                  m_vxCandidate)
{
  declareProperty ("SGKey", m_vxCandidate = "PrimaryVertices,VxPrimaryCandidate");
}

StatusCode TrackParticlePerigeeAtPVAssociationTool::initialize(){

  CHECK( Base::initialize() );
  CHECK( m_trackToVertexTool.retrieve() );

  return StatusCode::SUCCESS;
}


/**
 * @brief Return the target object.
 * @param track The source object for the association.
 *
 * Return the target of the association, or 0.
 */
const Trk::TrackParameters*
TrackParticlePerigeeAtPVAssociationTool::get (const xAOD::TrackParticle& track)
{
  m_resolver.initialize<xAOD::VertexContainer>().ignore();
  const xAOD::VertexContainer* vxContainer = 0;
  StatusCode sc = evtStore()->retrieve(vxContainer, m_resolver.key());
  if (sc.isFailure() || !vxContainer) {
    REPORT_MESSAGE (MSG::WARNING) << "Could not retrieve primary vertex container: " << m_vxCandidate;
    return 0;
  }

  if(vxContainer->size()<1){
    REPORT_MESSAGE (MSG::WARNING) << "No primary vertices reconstructed";
    return 0;
  }

  for (const xAOD::Vertex* vx : *vxContainer) {
    if (vx->vertexType() == xAOD::VxType::PriVtx)
      return m_trackToVertexTool->perigeeAtVertex(Gaudi::Hive::currentContext(), track, vx->position()).release();
  }

  REPORT_MESSAGE (MSG::WARNING) << "No primary vertices reconstructed";
  return 0;
}


/**
 * @brief Release an object retrieved from the association.
 * @param p The object to release.
 *
 * Call this when you are done with the object returned by
 * @c get().  The default implementation is a no-op,
 * but since the association dynamically allocates the object which
 * it returned, this gives it a chance to free it.
 */
void TrackParticlePerigeeAtPVAssociationTool::releaseObject (const Trk::TrackParameters* p)
{
  if(p) delete p;
}

} // namespace D3PD
