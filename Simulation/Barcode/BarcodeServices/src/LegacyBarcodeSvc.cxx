/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BarcodeServices/LegacyBarcodeSvc.h"
// framework include
#include "TruthUtils/MagicNumbers.h"


/** Constructor **/
Barcode::LegacyBarcodeSvc::LegacyBarcodeSvc(const std::string& name,ISvcLocator* svc) :
  base_class(name,svc),
  m_firstVertex(-HepMC::SIM_BARCODE_THRESHOLD-1),
  m_vertexIncrement(-1),
  m_firstSecondary(HepMC::SIM_BARCODE_THRESHOLD+1),
  m_particleIncrement(1)
{
}


/** framework methods */
StatusCode Barcode::LegacyBarcodeSvc::initialize()
{
  ATH_MSG_VERBOSE ("initialize() ...");
  ATH_MSG_DEBUG( "LegacyBarcodeSvc start of initialize in thread ID: " << std::this_thread::get_id() );

  ATH_CHECK( this->initializeBarcodes() );

  ATH_MSG_VERBOSE ("initialize() successful");
  return StatusCode::SUCCESS;
}


StatusCode Barcode::LegacyBarcodeSvc::initializeBarcodes(int largestGeneratedParticleBC, int largestGeneratedVertexBC) {
    static std::mutex barcodeMutex;
    std::lock_guard<std::mutex> barcodeLock(barcodeMutex);
    ATH_MSG_DEBUG( name() << "::initializeBarcodes()" );

  // look for pair containing barcode info using the thread ID
  // if it doesn't exist, construct one and insert it.
  const auto tid = std::this_thread::get_id();
  auto bcPair = m_bcThreadMap.find(tid);
  if ( bcPair == m_bcThreadMap.end() ) {
    auto result = m_bcThreadMap.insert( std::make_pair( tid, BarcodeInfo(-HepMC::SIM_BARCODE_THRESHOLD, HepMC::SIM_BARCODE_THRESHOLD, largestGeneratedVertexBC, largestGeneratedParticleBC) ) );
      if (result.second) {
          ATH_MSG_DEBUG( "initializeBarcodes: initialized new barcodes for thread ID " << tid );
          ATH_CHECK( this->resetBarcodes(largestGeneratedParticleBC, largestGeneratedVertexBC) );
          ATH_MSG_DEBUG( "initializeBarcodes: reset new barcodes for thread ID " << tid );
      } else {
          ATH_MSG_ERROR( "initializeBarcodes: failed to initialize new barcode for thread ID " << tid );
      }
  } else {
      ATH_MSG_DEBUG( "initializeBarcodes: barcodes for this thread ID found, did not construct new" );
      ATH_CHECK( this->resetBarcodes(largestGeneratedParticleBC, largestGeneratedVertexBC) );
      ATH_MSG_DEBUG( "initializeBarcodes: reset existing barcodes for thread ID " << tid );
  }
  return StatusCode::SUCCESS;
}


Barcode::LegacyBarcodeSvc::BarcodeInfo& Barcode::LegacyBarcodeSvc::getBarcodeInfo() {
    const auto tid = std::this_thread::get_id();
    auto bcPair = m_bcThreadMap.find(tid);
    if ( bcPair == m_bcThreadMap.end() ) {
      ATH_MSG_WARNING( "getBarcodeInfo: failed to find BarcodeInfo for thread ID " << tid << ", created a new one." );
      Barcode::LegacyBarcodeSvc::BarcodeInfo barcodeInfo = BarcodeInfo(-HepMC::SIM_BARCODE_THRESHOLD, HepMC::SIM_BARCODE_THRESHOLD, 0, 0);
      auto result = m_bcThreadMap.insert( std::make_pair( tid, barcodeInfo ) );
      if (result.second) {
        auto bcPair = m_bcThreadMap.find(tid);
        return bcPair->second;
      }
      ATH_MSG_ERROR ( "getBarcodeInfo: could not add a the new BarcodeInfo object to the map!" );
      return m_bcThreadMap.begin()->second;
    }
    return bcPair->second;
}


/** Generate a new unique vertex simulated barcode */
int Barcode::LegacyBarcodeSvc::newSimulationVertex()
{
  BarcodeInfo& bc = getBarcodeInfo();
  bc.currentSimulationVertex += m_vertexIncrement;
  // a naive underflog checking based on the fact that vertex
  // barcodes should never be positive
  if ( bc.currentSimulationVertex > -HepMC::SIM_BARCODE_THRESHOLD)
    {
      ATH_MSG_WARNING("LegacyBarcodeSvc::newSimulationVertex()"
                    << " will return a vertex barcode greater than "
                    << -HepMC::SIM_BARCODE_THRESHOLD << ": "
                    << bc.currentSimulationVertex << ". Reset to "
                    << HepMC::UNDEFINED_ID);
      bc.currentSimulationVertex = HepMC::UNDEFINED_ID;
    }

  return bc.currentSimulationVertex;
}


/** Generate a new unique barcode for a secondary particle */
int Barcode::LegacyBarcodeSvc::newSecondaryParticle(int)
{
  BarcodeInfo& bc = getBarcodeInfo();
  bc.currentSecondaryParticle += m_particleIncrement;
  // a naive overflow checking based on the fact that particle
  // barcodes should never be negative
  if (bc.currentSecondaryParticle < HepMC::SIM_BARCODE_THRESHOLD)
    {
      ATH_MSG_WARNING("LegacyBarcodeSvc::newSecondaryParticle()"
                    << " will return a particle barcode of less than "
                    << HepMC::SIM_BARCODE_THRESHOLD << ": "
                    << bc.currentSecondaryParticle << ". Reset to "
                    << HepMC::UNDEFINED_ID);

      bc.currentSecondaryParticle = HepMC::UNDEFINED_ID;
    }

  return bc.currentSecondaryParticle;
}


/** Generate a new unique vertex barcode for pre-defined decay vertices */
int Barcode::LegacyBarcodeSvc::newGeneratedVertex()
{
  BarcodeInfo& bc = getBarcodeInfo();
  bc.currentGeneratedVertex += m_vertexIncrement;
  // a naive underflog checking based on the fact that vertex
  // barcodes should never be positive
  if ( bc.currentGeneratedVertex > 0) {
      ATH_MSG_WARNING("LegacyBarcodeSvc::newGeneratedVertex()"
                    << " will return a vertex barcode greater than 0: "
                    << bc.currentGeneratedVertex << ". Reset to "
                    << HepMC::UNDEFINED_ID);

      bc.currentGeneratedVertex = HepMC::UNDEFINED_ID;
    }
  if ( bc.currentGeneratedVertex < -HepMC::SIM_BARCODE_THRESHOLD) {
      ATH_MSG_ERROR("LegacyBarcodeSvc::newGeneratedVertex()"
                    << " will return a vertex barcode below "
                    << -HepMC::SIM_BARCODE_THRESHOLD << ": "
                    << bc.currentGeneratedVertex << ". Expect clashes with simulation vertices.");
    }

  return bc.currentGeneratedVertex;
}


/** Generate a new unique barcode for a particle produced in a pre-defined decay  */
int Barcode::LegacyBarcodeSvc::newGeneratedParticle(int)
{
  BarcodeInfo& bc = getBarcodeInfo();
  bc.currentGeneratedParticle += m_particleIncrement;
  // a naive overflow checking based on the fact that particle
  // barcodes should never be negative
  if (bc.currentGeneratedParticle < 0)
    {
      ATH_MSG_WARNING("LegacyBarcodeSvc::newGeneratedParticle()"
                    << " will return a particle barcode of less than 0: "
                    << bc.currentGeneratedParticle << ". Reset to "
                    << HepMC::UNDEFINED_ID);

      bc.currentGeneratedParticle = HepMC::UNDEFINED_ID;
    }
  if ( bc.currentGeneratedParticle > HepMC::SIM_BARCODE_THRESHOLD) {
      ATH_MSG_ERROR("LegacyBarcodeSvc::newGeneratedParticle()"
                    << " will return a particle barcode below "
                    << -HepMC::SIM_BARCODE_THRESHOLD << ": "
                    << bc.currentGeneratedParticle << ". Expect clashes with simulation particles.");
    }

  return bc.currentGeneratedParticle;
}


/** Generate a common barcode which will be shared by all children
    of the given parent barcode (used for child particles which are
    not stored in the mc truth event) */
int Barcode::LegacyBarcodeSvc::sharedChildBarcode( int /* parentBC */,
                                                                        int /* process */)
{
  // concept of shared barcodes not present in MC12 yet
  return HepMC::UNDEFINED_ID;
}


void Barcode::LegacyBarcodeSvc::registerLargestGeneratedParticleBC( int bc ) {
    ATH_MSG_DEBUG( "registering largest generated particle barcode" );
    BarcodeInfo& barcodeInfo = getBarcodeInfo();
    barcodeInfo.currentGeneratedParticle = bc;
}


void Barcode::LegacyBarcodeSvc::registerLargestGeneratedVtxBC( int bc ) {
    ATH_MSG_DEBUG( "registering largest generated particle barcode" );
    BarcodeInfo& barcodeInfo = getBarcodeInfo();
    barcodeInfo.currentGeneratedVertex = bc;
}


void Barcode::LegacyBarcodeSvc::registerLargestSecondaryParticleBC( int /* bc */) {
}


void Barcode::LegacyBarcodeSvc::registerLargestSimulationVtxBC( int /* bc */) {
}


/** Return the secondary particle offset */
int Barcode::LegacyBarcodeSvc::secondaryParticleBcOffset() const {
  return m_firstSecondary;
}


/** Return the secondary vertex offset */
int Barcode::LegacyBarcodeSvc::secondaryVertexBcOffset() const {
  return m_firstVertex;
}


StatusCode Barcode::LegacyBarcodeSvc::resetBarcodes(int largestGeneratedParticleBC, int largestGeneratedVertexBC)
{
    ATH_MSG_DEBUG( "resetBarcodes: resetting barcodes" );
    BarcodeInfo& bc = getBarcodeInfo();
    bc.currentSimulationVertex = m_firstVertex - m_vertexIncrement;
    bc.currentSecondaryParticle = m_firstSecondary - m_particleIncrement;
    bc.currentGeneratedVertex = largestGeneratedVertexBC;
    bc.currentGeneratedParticle = largestGeneratedParticleBC;

    return StatusCode::SUCCESS;
}
