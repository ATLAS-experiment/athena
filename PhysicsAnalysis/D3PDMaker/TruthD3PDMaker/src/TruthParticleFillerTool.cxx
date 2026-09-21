/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file TruthD3PDMaker/src/TruthParticleFillerTool.cxx
 * @author Ryan Reece <ryan.reece@cern.ch>
 * @date Dec, 2009
 * @brief Block filler tool for TruthParticle's.
 */

#include "TruthParticleFillerTool.h"
#include "AthenaKernel/errorcheck.h"
#include "TruthUtils/MagicNumbers.h"
#include "TruthUtils/HepMCHelpers.h"

namespace D3PD {


/**
 * @brief Standard Gaudi tool constructor.
 * @param type The name of the tool type.
 * @param name The tool name.
 * @param parent The tool's Gaudi parent.
 */
TruthParticleFillerTool::TruthParticleFillerTool (const std::string& type,
                                                  const std::string& name,
                                                  const IInterface* parent)
  : Base (type, name, parent)
{
  declareProperty ("PDGIDVariable",  m_PDGIDVariable = "pdgId");

  book().ignore(); // Avoid coverity warnings.
}


/**
 * @brief Standard Gaudi initialize method.
 */
StatusCode TruthParticleFillerTool::initialize()
{
  return StatusCode::SUCCESS;
}


/**
 * @brief Book variables for this block.
 */
StatusCode TruthParticleFillerTool::book()
{
  CHECK( addVariable("status",    m_status) );
  CHECK( addVariable("barcode",   m_uniqueID) ); // TODO Rename variable to be consistent?
  CHECK( addVariable(m_PDGIDVariable,     m_pdgId) );
  CHECK( addVariable("charge",    m_charge) );
  return StatusCode::SUCCESS;
}


/**
 * @brief Fill one block --- type-safe version.
 * @param p The input object.
 *
 * This is called once per object.  The caller
 * is responsible for arranging that all the pointers for booked variables
 * are set appropriately upon entry.
 */
StatusCode TruthParticleFillerTool::fill (const xAOD::TruthParticle& p)
{
  *m_status = HepMC::status(p);
  *m_uniqueID = HepMC::uniqueID(p);
  *m_pdgId = p.pdgId();
  *m_charge = static_cast<int>(MC::charge(p.pdgId()));
  return StatusCode::SUCCESS;
}


} // namespace D3PD
