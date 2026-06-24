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
#include "GaudiKernel/ITHistSvc.h"
#include <TH1.h>
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
         // The detour design: Output file is handled by THistSvc, including the file name. But THistSvc does not provide a direct way to get the TFile pointer for a given stream. 
         // So we register a temporary histogram to get the TFile pointer for the output stream.
         SmartIF<ITHistSvc> tHistSvc{service("THistSvc")};
         ATH_CHECK(tHistSvc.isValid());
         const std::string& targetStream = m_outputStreamName.value();
         const std::string probeId = "/" + targetStream + "/__rntuple_file_probe__";
         {
             auto probe = std::make_unique<TH1F>("__rntuple_file_probe__", "", 1, 0., 1.);
             probe->SetDirectory(nullptr);
             ATH_CHECK(tHistSvc->regHist(probeId, std::move(probe)));
         }
         TH1* probeHist = nullptr;
         ATH_CHECK(tHistSvc->getHist(probeId, probeHist));
         if (probeHist && probeHist->GetDirectory()) {
             outputFile = probeHist->GetDirectory()->GetFile();
         }
         ATH_CHECK(tHistSvc->deReg(probeId));
         delete probeHist;
         ATH_MSG_INFO("RNTuple found output file: " << (outputFile ? outputFile->GetName() : "nullptr"));
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
