/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BarcodeServices/ValidationBarcodeSvc.h"
// framework include
#include "GaudiKernel/IIncidentSvc.h"
#include "TruthUtils/MagicNumbers.h"


/** Constructor **/
Barcode::ValidationBarcodeSvc::ValidationBarcodeSvc(const std::string& name,ISvcLocator* svc) :
  base_class(name,svc),
  m_incidentSvc("IncidentSvc", name),
  m_firstVertex(-HepMC::SIM_BARCODE_THRESHOLD-1),
  m_vertexIncrement(-1),
  m_currentVertex(-1),
  m_firstSecondary(HepMC::SIM_BARCODE_THRESHOLD+1),
  m_secondaryIncrement(1),
  m_currentSecondary(1),
  m_particleGenerationIncrement(HepMC::SIM_REGENERATION_INCREMENT),
  m_barcodeGenerationOffset(1e8)
{
  // python properties
  declareProperty("VertexIncrement"            ,  m_vertexIncrement);
  declareProperty("SecondaryIncrement"         ,  m_secondaryIncrement);
  declareProperty("BarcodeGenerationOffset"    ,  m_barcodeGenerationOffset);
}


Barcode::ValidationBarcodeSvc::~ValidationBarcodeSvc()
{}


/** framework methods */
StatusCode Barcode::ValidationBarcodeSvc::initialize()
{
  ATH_MSG_VERBOSE ("initialize() ...");

  CHECK( m_incidentSvc.retrieve() );

  // register to the incident service: BeginEvent needed for refresh of counter
  m_incidentSvc->addListener( this, IncidentType::BeginEvent);

  ATH_MSG_VERBOSE ("initialize() successful");
  return StatusCode::SUCCESS;
}


/** Generate a new unique vertex barcode*/
int Barcode::ValidationBarcodeSvc::newSimulationVertex()
{
  m_currentVertex += m_vertexIncrement;
  // a naive underflog checking based on the fact that vertex
  // barcodes should never be positive
  if ( m_currentVertex > 0)
    {
      ATH_MSG_ERROR("ValidationBarcodeSvc::newVertex(...)"
                    << " will return a vertex barcode greater than 0: "
                    << m_currentVertex << ". Possibly Integer Underflow?");
    }
  return m_currentVertex;
}


/** Generate a new unique barcode for a secondary particle, based on the parent
    particle barcode */
int Barcode::ValidationBarcodeSvc::newSecondaryParticle( int parentBC )
{
  m_currentSecondary += m_secondaryIncrement;
  // a naive overflow checking based on the fact that particle
  // barcodes should never be negative
  if ( m_currentSecondary < 0)
    {
      ATH_MSG_ERROR("ValidationBarcodeSvc::newSecondaryParticle(...)"
                    << " will return a particle barcode of less than 0: "
                    << m_currentSecondary << ". Possibly Integer Overflow?");
    }
  int primGen = int( (parentBC-m_particleGenerationIncrement) / m_barcodeGenerationOffset);
  int returnBC = m_currentSecondary + (primGen+1)*m_barcodeGenerationOffset;

  return returnBC;
}


/** Generate a new unique particle barcode below the simulation offset (for particles from pre-defined decays) */
int Barcode::ValidationBarcodeSvc::newGeneratedParticle( int parentBC )
{
  m_currentSecondary += m_secondaryIncrement;
  // a naive overflow checking based on the fact that particle
  // barcodes should never be negative
  if ( m_currentSecondary < 0)
    {
      ATH_MSG_ERROR("ValidationBarcodeSvc::newGeneratedParticle(...)"
                    << " will return a particle barcode of less than 0: "
                    << m_currentSecondary << ". Possibly Integer Overflow?");
    }
  int primGen = int( (parentBC-m_particleGenerationIncrement) / m_barcodeGenerationOffset);
  int returnBC = m_currentSecondary + (primGen+1)*m_barcodeGenerationOffset;

  return returnBC;
}


/** Generate a new unique vertex barcode*/
int Barcode::ValidationBarcodeSvc::newGeneratedVertex()
{
  m_currentVertex += m_vertexIncrement;
  // a naive underflog checking based on the fact that vertex
  // barcodes should never be positive
  if ( m_currentVertex > 0)
    {
      ATH_MSG_ERROR("ValidationBarcodeSvc::newGeneratedVertex(...)"
                    << " will return a vertex barcode greater than 0: "
                    << m_currentVertex << ". Possibly Integer Underflow?");
    }
  return m_currentVertex;
}


/** Return the secondary particle offset */
int Barcode::ValidationBarcodeSvc::secondaryParticleBcOffset() const
{
  return m_firstSecondary;
}


/** Return the secondary vertex offset */
int Barcode::ValidationBarcodeSvc::secondaryVertexBcOffset() const
{
  return m_firstVertex;
}


/** Handle incident */
void Barcode::ValidationBarcodeSvc::handle(const Incident& inc)
{
  if ( inc.type() == IncidentType::BeginEvent )
    {
      ATH_MSG_VERBOSE("'BeginEvent' incident caught. Resetting Vertex and Particle barcode counters.");
      m_currentVertex    = m_firstVertex    - m_vertexIncrement;
      m_currentSecondary = m_firstSecondary - m_secondaryIncrement;
    }
}


/** framework methods */
StatusCode Barcode::ValidationBarcodeSvc::finalize()
{
  ATH_MSG_VERBOSE ("finalize() ...");
  ATH_MSG_VERBOSE ("finalize() successful");
  return StatusCode::SUCCESS;
}
