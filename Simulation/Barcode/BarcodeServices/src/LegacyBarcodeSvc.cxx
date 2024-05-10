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
  m_currentVertex(-1),
  m_firstSecondary(HepMC::SIM_BARCODE_THRESHOLD+1),
  m_secondaryIncrement(1),
  m_currentSecondary(1)
{
  // python properties
  declareProperty("VertexIncrement"            ,  m_vertexIncrement);
  declareProperty("SecondaryIncrement"         ,  m_secondaryIncrement);
}


Barcode::LegacyBarcodeSvc::~LegacyBarcodeSvc()
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

StatusCode Barcode::LegacyBarcodeSvc::initializeBarcodes() {
    static std::mutex barcodeMutex;
    std::lock_guard<std::mutex> barcodeLock(barcodeMutex);
    ATH_MSG_DEBUG( name() << "::initializeBarcodes()" );

  // look for pair containing barcode info using the thread ID
  // if it doesn't exist, construct one and insert it.
  const auto tid = std::this_thread::get_id();
  auto bcPair = m_bcThreadMap.find(tid);
  if ( bcPair == m_bcThreadMap.end() ) {
      auto result = m_bcThreadMap.insert( std::make_pair( tid, BarcodeInfo(m_currentVertex, m_currentSecondary)) );
      if (result.second) {
          ATH_MSG_DEBUG( "initializeBarcodes: initialized new barcodes for thread ID " << tid );
          ATH_CHECK( this->resetBarcodes() );
          ATH_MSG_DEBUG( "initializeBarcodes: reset new barcodes for thread ID " << tid );
      } else {
          ATH_MSG_ERROR( "initializeBarcodes: failed to initialize new barcode for thread ID " << tid );
      }
  } else {
      ATH_MSG_DEBUG( "initializeBarcodes: barcodes for this thread ID found, did not construct new" );
      ATH_CHECK( this->resetBarcodes() );
      ATH_MSG_DEBUG( "initializeBarcodes: reset existing barcodes for thread ID " << tid );
  }
  return StatusCode::SUCCESS;
}

//FIXME this should return an optional type, since returning the value of the end iterator is undefined behaviour (I think it causes a segfault).
Barcode::LegacyBarcodeSvc::BarcodeInfo& Barcode::LegacyBarcodeSvc::getBarcodeInfo() {
    const auto tid = std::this_thread::get_id();
    auto bcPair = m_bcThreadMap.find(tid);
    if ( bcPair == m_bcThreadMap.end() ) {
        ATH_MSG_ERROR( "getBarcodeInfo: failed to get BarcodeInfo for thread ID " << tid );
        return bcPair->second;
    } else {
        return bcPair->second;
    }
}

/** Generate a new unique vertex barcode, based on the parent particle barcode and
    the physics process code causing the truth vertex*/
int Barcode::LegacyBarcodeSvc::newVertex( int /* parent */,
                                                             int /* process */)
{
  BarcodeInfo& bc = getBarcodeInfo();
  bc.currentVertex += m_vertexIncrement;
  // a naive underflog checking based on the fact that vertex
  // barcodes should never be positive
  if ( bc.currentVertex > 0)
    {
      ATH_MSG_ERROR("LegacyBarcodeSvc::newVertex(...)"
                    << " will return a vertex barcode greater than 0: "
                    << bc.currentVertex << ". Possibly Integer Underflow?");
    }

  return bc.currentVertex;
}


/** Generate a new unique barcode for a secondary particle, based on the parent
    particle barcode and the process code of the physics process that created
    the secondary  */
int Barcode::LegacyBarcodeSvc::newSecondary( int /* parentBC */,
                                                                  int /* process */)
{
  BarcodeInfo& bc = getBarcodeInfo();
  bc.currentSecondary += m_secondaryIncrement;
  // a naive overflow checking based on the fact that particle
  // barcodes should never be negative
  if (bc.currentSecondary < 0)
    {
      ATH_MSG_DEBUG("LegacyBarcodeSvc::newSecondary(...)"
                    << " will return a particle barcode of less than 0: "
                    << bc.currentSecondary << ". Reset to zero.");

      bc.currentSecondary = HepMC::UNDEFINED_ID;
    }

  return bc.currentSecondary;
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


void Barcode::LegacyBarcodeSvc::registerLargestGenEvtParticleBC( int /* bc */) {
}


void Barcode::LegacyBarcodeSvc::registerLargestGenEvtVtxBC( int /* bc */) {
}


/** Return the secondary particle offset */
int Barcode::LegacyBarcodeSvc::secondaryParticleBcOffset() const {
  return m_firstSecondary;
}


/** Return the secondary vertex offset */
int Barcode::LegacyBarcodeSvc::secondaryVertexBcOffset() const {
  return m_firstVertex;
}


StatusCode Barcode::LegacyBarcodeSvc::resetBarcodes()
{
    ATH_MSG_DEBUG( "resetBarcodes: resetting barcodes" );
    BarcodeInfo& bc = getBarcodeInfo();
    bc.currentVertex    = m_firstVertex    - m_vertexIncrement;
    bc.currentSecondary = m_firstSecondary - m_secondaryIncrement;
    return StatusCode::SUCCESS;
}


/** framework methods */
StatusCode Barcode::LegacyBarcodeSvc::finalize()
{
  ATH_MSG_VERBOSE ("finalize() ...");
  ATH_MSG_VERBOSE ("finalize() successful");
  return StatusCode::SUCCESS;
}
