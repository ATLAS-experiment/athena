/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "AsgAnalysisAlgorithms/RNtupleTreeMakerAlg.h"

// EDM include(s):
#include "AthContainers/AuxElement.h"
#include "AthContainers/AuxVectorBase.h"

// ROOT include(s):
#include <TClass.h>
#include <TFile.h>

// Gaudi/EventLoop include(s):
#ifdef XAOD_STANDALONE
#include "EventLoop/Worker.h"
#else
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/AttribStringParser.h"

#endif

namespace CP {

   StatusCode RNtupleTreeMakerAlg::initialize() {
      if( m_branches.empty() ) {
         ATH_MSG_ERROR( "No branches set up for writing" );
         return StatusCode::FAILURE;
      }
      ATH_CHECK( m_systematicsService.retrieve() );
      m_isInitialized = false;
      return StatusCode::SUCCESS;
   }

   StatusCode RNtupleTreeMakerAlg::execute(const EventContext& /*ctx*/) {
      if( ! m_isInitialized ) {
         m_model = ROOT::RNTupleModel::Create();
         if ( !m_model ) {
             ATH_MSG_ERROR( "Failed to create RNTupleModel" );
             return StatusCode::FAILURE;
         }

         ATH_CHECK( setupTree() );

         TFile* outputFile = nullptr;
#ifdef XAOD_STANDALONE
         if( wk() ) {
            outputFile = wk()->getOutputFile( m_outputStreamName.value() );
         } else {
             ATH_MSG_ERROR( "Worker not available in standalone mode" );
             return StatusCode::FAILURE;
         }
#else
         // Ath no direct way to get Tfile pointer. Output filename is instead used as the handle.
         // retrieve pointer to THistSvc
         SmartIF<ITHistSvc> tHistSvc{service("THistSvc")};
         ATH_CHECK(tHistSvc.isValid());
         std::string outputRepr;
         // THistSvc Output property example: [ 'ANALYSIS DATAFILE=\'output.root\' OPT=\'RECREATE\'' .. ]
         ATH_CHECK(SmartIF<IProperty>(tHistSvc.get())->getProperty("Output", outputRepr));
         ATH_MSG_INFO( "THistSvc Output property: " << outputRepr );
         std::string fileName;
         for (auto& attrib : Gaudi::Utils::AttribStringParser(outputRepr)) {
           auto tag = attrib.tag;
           ATH_MSG_INFO( "THistSvc Output attrib: " << tag << " = " << attrib.value );
           if (tag == "ANALYSIS DATAFILE")
             fileName = attrib.value;
         }
         ATH_MSG_INFO( "RNTuple Output file: " << fileName );
         // naive implementation for AthAnalysis, I don't see any Ath Svc offer
         // getting the output stream easily
         outputFile = TFile::Open( m_outputStreamName.value().c_str(), "UPDATE" );
         ATH_MSG_INFO( "RNTuple Opened output file: " << m_outputStreamName.value() ); //INFO RNTuple Opened output file: ANALYSIS
#endif

         if( !outputFile ) {
             ATH_MSG_ERROR( "Could not retrieve file for stream: " << m_outputStreamName.value() );
             return StatusCode::FAILURE;
         }

         try {
             m_writer = ROOT::RNTupleWriter::Append( std::move(m_model), m_modelName.value(), *outputFile );
         } catch( const std::exception& e ) {
             ATH_MSG_ERROR( "Failed to create RNTupleWriter: " << e.what() );
             return StatusCode::FAILURE;
         }

         m_isInitialized = true;
      }

      ATH_CHECK( m_processorList.process( *(evtStore()) ) );

      if ( m_writer ) {
          m_writer->Fill();
      }

      return StatusCode::SUCCESS;
   }

   StatusCode RNtupleTreeMakerAlg::finalize() {
       m_writer.reset();
       return StatusCode::SUCCESS;
   }

   StatusCode RNtupleTreeMakerAlg::setupTree() {
       std::unordered_set<std::string> nonContainerSet( m_nonContainers.begin(), m_nonContainers.end() );
       ATH_CHECK( m_processorList.setupTree( m_branches, std::move(nonContainerSet), *m_systematicsService, *m_model ) );
       return StatusCode::SUCCESS;
   }



   // Removed old ContainerProcessor/ElementProcessor implementations as they are now in FieldHelpers
} // namespace CP
