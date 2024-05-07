/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "BarcodeServices/GenericBarcodeSvc.h"
// framework include
#include "GaudiKernel/IIncidentSvc.h"
#include "TruthUtils/MagicNumbers.h"


/** Constructor **/
Barcode::GenericBarcodeSvc::GenericBarcodeSvc(const std::string& name,ISvcLocator* svc) :
  base_class(name,svc),
  m_incidentSvc("IncidentSvc", name),
  m_firstVertex(-HepMC::SIM_BARCODE_THRESHOLD-1),
  m_vertexIncrement(-1),
  m_currentVertex(-1),
  m_firstSecondary(HepMC::SIM_BARCODE_THRESHOLD+1),
  m_secondaryIncrement(1),
  m_currentSecondary(1),
  m_encodePhysicsProcess(false)
{
  // python properties
  declareProperty("VertexIncrement"               ,  m_vertexIncrement);
  declareProperty("SecondaryIncrement"            ,  m_secondaryIncrement);
  declareProperty("EncodePhysicsProcessInVertexBC",  m_encodePhysicsProcess);
}


Barcode::GenericBarcodeSvc::~GenericBarcodeSvc()
{}


/** framework methods */
StatusCode Barcode::GenericBarcodeSvc::initialize()
{
  ATH_MSG_VERBOSE ("initialize() ...");

  CHECK( m_incidentSvc.retrieve());

  // register to the incident service: BeginEvent needed for refresh of counter
  m_incidentSvc->addListener( this, IncidentType::BeginEvent);

  ATH_MSG_VERBOSE ("initialize() successful");
  return StatusCode::SUCCESS;
}


/** Generate a new unique vertex barcode, based on the parent particle barcode and
    the physics process code causing the truth vertex*/
Barcode::VertexBarcode Barcode::GenericBarcodeSvc::newVertex( Barcode::ParticleBarcode /* parent */,
                                                              Barcode::PhysicsProcessCode process )
{
  // update the internal vertex BC counter
  m_currentVertex += m_vertexIncrement;

  // the barcode that will be returned
  Barcode::VertexBarcode newBC = m_currentVertex;

  // if enabled, put the physics process code into the vertex barcode
  if (m_encodePhysicsProcess)
    {
      newBC = newBC - process;
      // an example vertex BC would be (8th vtx, process #1234):  -8201234
    }

  // a naive underflog checking based on the fact that vertex
  // barcodes should never be positive
  if (  newBC > 0)
    {
      ATH_MSG_ERROR("LegacyBarcodeSvc::newVertex(...)"
                    << " will return a vertex barcode greater than 0: '"
                    << m_currentVertex << "'. Possibly Integer Underflow?");
    }

  return newBC;
}


/** Generate a new unique barcode for a secondary particle, based on the parent
    particle barcode and the process code of the physics process that created
    the secondary  */
Barcode::ParticleBarcode Barcode::GenericBarcodeSvc::newSecondary( Barcode::ParticleBarcode /* parentBC */,
                                                                   Barcode::PhysicsProcessCode /* process */)
{
  m_currentSecondary += m_secondaryIncrement;

  // a naive overflow checking based on the fact that particle
  // barcodes should never be negative
  if ( m_currentSecondary < 0)
    {
      ATH_MSG_ERROR("LegacyBarcodeSvc::newSecondary(...)"
                    << " will return a particle barcode of less than 0: '"
                    << m_currentSecondary << "'. Reset to zero.");
      m_currentSecondary = Barcode::fUndefinedBarcode;
    }

  return m_currentSecondary;
}


/** Generate a common barcode which will be shared by all children
    of the given parent barcode (used for child particles which are
    not stored in the mc truth event) */
Barcode::ParticleBarcode Barcode::GenericBarcodeSvc::sharedChildBarcode( Barcode::ParticleBarcode /* parentBC */,
                                                                         Barcode::PhysicsProcessCode /* process */)
{
  // concept of shared barcodes not supported here yet
  return Barcode::fUndefinedBarcode;
}


void Barcode::GenericBarcodeSvc::registerLargestGenEvtParticleBC( Barcode::ParticleBarcode /* bc */)
{
}


void Barcode::GenericBarcodeSvc::registerLargestGenEvtVtxBC( Barcode::VertexBarcode /* bc */)
{
}


/** Return the secondary particle offset */
Barcode::ParticleBarcode Barcode::GenericBarcodeSvc::secondaryParticleBcOffset() const
{
  return m_firstSecondary;
}


/** Return the secondary vertex offset */
Barcode::VertexBarcode Barcode::GenericBarcodeSvc::secondaryVertexBcOffset() const
{
  return m_firstVertex;
}


/** Handle incident */
void Barcode::GenericBarcodeSvc::handle(const Incident& inc)
{
  if ( inc.type() == IncidentType::BeginEvent )
    {
      ATH_MSG_VERBOSE("'BeginEvent' incident caught. Resetting Vertex and Particle barcode counters.");
      m_currentVertex    = m_firstVertex    - m_vertexIncrement;
      m_currentSecondary = m_firstSecondary - m_secondaryIncrement;
    }
}


/** framework methods */
StatusCode Barcode::GenericBarcodeSvc::finalize()
{
  ATH_MSG_VERBOSE ("finalize() ...");
  ATH_MSG_VERBOSE ("finalize() successful");
  return StatusCode::SUCCESS;
}
