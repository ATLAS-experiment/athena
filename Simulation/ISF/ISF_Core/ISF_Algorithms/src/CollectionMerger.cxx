/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CollectionMerger.h"

/** Constructor */
ISF::CollectionMerger::CollectionMerger( const std::string& name, ISvcLocator* pSvcLocator ) :
  ::AthReentrantAlgorithm( name, pSvcLocator )
{
}


/** Athena Algorithm initialize */
StatusCode ISF::CollectionMerger::initialize()
{
  ATH_MSG_VERBOSE("In initialize");
  ATH_CHECK( m_inputBCMHits.initialize() );
  ATH_CHECK( m_inputBLMHits.initialize() );
  ATH_CHECK( m_inputPixelHits.initialize() );
  ATH_CHECK( m_inputSCTHits.initialize() );
  ATH_CHECK( m_inputTRTUncompressedHits.initialize() );

  ATH_CHECK( m_inputITkPixelHits.initialize() );
  ATH_CHECK( m_inputITkStripHits.initialize() );
  ATH_CHECK( m_inputPLRHits.initialize() );
  ATH_CHECK( m_inputHGTDHits.initialize() );

  ATH_CHECK( m_inputLArEMBHits.initialize() );
  ATH_CHECK( m_inputLArEMECHits.initialize() );
  ATH_CHECK( m_inputLArFCALHits.initialize() );
  ATH_CHECK( m_inputLArHECHits.initialize() );

  ATH_CHECK( m_inputTileHits.initialize() );
  ATH_CHECK( m_inputMBTSHits.initialize() );

  ATH_CHECK( m_inputCSCHits.initialize() );
  ATH_CHECK( m_inputMDTHits.initialize() );
  ATH_CHECK( m_inputRPCHits.initialize() );
  ATH_CHECK( m_inputTGCHits.initialize() );
  ATH_CHECK( m_inputsTGCHits.initialize() );
  ATH_CHECK( m_inputMMHits.initialize() );


  ATH_CHECK( m_outputBCMHits.initialize(!m_inputBCMHits.empty()) );
  ATH_CHECK( m_outputBLMHits.initialize(!m_inputBLMHits.empty()) );
  ATH_CHECK( m_outputPixelHits.initialize(!m_inputPixelHits.empty()) );
  ATH_CHECK( m_outputSCTHits.initialize(!m_inputSCTHits.empty()) );
  ATH_CHECK( m_outputTRTUncompressedHits.initialize(!m_inputTRTUncompressedHits.empty()) );

  ATH_CHECK( m_outputITkPixelHits.initialize(!m_inputITkPixelHits.empty()) );
  ATH_CHECK( m_outputITkStripHits.initialize(!m_inputITkStripHits.empty()) );
  ATH_CHECK( m_outputPLRHits.initialize(!m_inputPLRHits.empty()) );
  ATH_CHECK( m_outputHGTDHits.initialize(!m_inputHGTDHits.empty()) );

  ATH_CHECK( m_outputLArEMBHits.initialize(!m_inputLArEMBHits.empty()) );
  ATH_CHECK( m_outputLArEMECHits.initialize(!m_inputLArEMECHits.empty()) );
  ATH_CHECK( m_outputLArFCALHits.initialize(!m_inputLArFCALHits.empty()) );
  ATH_CHECK( m_outputLArHECHits.initialize(!m_inputLArHECHits.empty()) );

  ATH_CHECK( m_outputTileHits.initialize(!m_inputTileHits.empty()) );
  ATH_CHECK( m_outputMBTSHits.initialize(!m_inputMBTSHits.empty()) );

  ATH_CHECK( m_outputCSCHits.initialize(!m_inputCSCHits.empty()) );
  ATH_CHECK( m_outputMDTHits.initialize(!m_inputMDTHits.empty()) );
  ATH_CHECK( m_outputRPCHits.initialize(!m_inputRPCHits.empty()) );
  ATH_CHECK( m_outputTGCHits.initialize(!m_inputTGCHits.empty()) );
  ATH_CHECK( m_outputsTGCHits.initialize(!m_inputsTGCHits.empty()) );
  ATH_CHECK( m_outputMMHits.initialize(!m_inputMMHits.empty()) );

  return StatusCode::SUCCESS;
}


/** Athena Algorithm execute */
StatusCode ISF::CollectionMerger::execute(const EventContext& ctx) const
{
  ATH_MSG_VERBOSE("in execute()");
  ATH_CHECK(mergeCollections( m_inputBCMHits,             m_outputBCMHits,             ctx ));
  ATH_CHECK(mergeCollections( m_inputBLMHits,             m_outputBLMHits,             ctx ));
  ATH_CHECK(mergeCollections( m_inputPixelHits,           m_outputPixelHits,           ctx ));
  ATH_CHECK(mergeCollections( m_inputSCTHits,             m_outputSCTHits,             ctx ));
  ATH_CHECK(mergeCollections( m_inputTRTUncompressedHits, m_outputTRTUncompressedHits, ctx ));

  ATH_CHECK(mergeCollections( m_inputITkPixelHits,        m_outputITkPixelHits,        ctx ));
  ATH_CHECK(mergeCollections( m_inputITkStripHits,        m_outputITkStripHits,        ctx ));
  ATH_CHECK(mergeCollections( m_inputPLRHits,             m_outputPLRHits,             ctx ));
  ATH_CHECK(mergeCollections( m_inputHGTDHits,            m_outputHGTDHits,            ctx ));

  ATH_CHECK(mergeCollections( m_inputLArEMBHits,          m_outputLArEMBHits,          ctx ));
  ATH_CHECK(mergeCollections( m_inputLArEMECHits,         m_outputLArEMECHits,         ctx ));
  ATH_CHECK(mergeCollections( m_inputLArFCALHits,         m_outputLArFCALHits,         ctx ));
  ATH_CHECK(mergeCollections( m_inputLArHECHits,          m_outputLArHECHits,          ctx ));

  ATH_CHECK(mergeCollections( m_inputTileHits,            m_outputTileHits,            ctx ));
  ATH_CHECK(mergeCollections( m_inputMBTSHits,            m_outputMBTSHits,            ctx ));

  ATH_CHECK(mergeCollections( m_inputCSCHits,             m_outputCSCHits,             ctx ));
  ATH_CHECK(mergeCollections( m_inputMDTHits,             m_outputMDTHits,             ctx ));
  ATH_CHECK(mergeCollections( m_inputRPCHits,             m_outputRPCHits,             ctx ));
  ATH_CHECK(mergeCollections( m_inputTGCHits,             m_outputTGCHits,             ctx ));
  ATH_CHECK(mergeCollections( m_inputsTGCHits,            m_outputsTGCHits,            ctx ));
  ATH_CHECK(mergeCollections( m_inputMMHits,              m_outputMMHits,              ctx ));

  return StatusCode::SUCCESS;
}
