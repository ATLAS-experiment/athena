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
#include <TROOT.h>

// Gaudi/EventLoop include(s):
#ifdef XAOD_STANDALONE
#include "EventLoop/Worker.h"
#else
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/AttribStringParser.h"
#include "Gaudi/Property.h"

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
         // AthAna has no direct way to get Tfile pointer. Output filename is
         // instead used as the handle. retrieve pointer to THistSvc, it
         // contains entry like:"ANALYSIS(stream name) DATAFILE='output.root' OPT='RECREATE'"
         SmartIF<ITHistSvc> tHistSvc{service("THistSvc")};
         ATH_CHECK(tHistSvc.isValid());
         Gaudi::Property<std::vector<std::string>> outputProp("Output", {});
         ATH_CHECK(SmartIF<IProperty>(tHistSvc.get())->getProperty(&outputProp));
         std::string fileName;
         const std::string& targetStream = m_outputStreamName.value(); //select ANALYSIS stream not ANALYSIS_HIST stream
         for (const auto& entry : outputProp.value()) {
           if (!entry.starts_with(targetStream + " ")) continue;
           for (const auto& attrib : Gaudi::Utils::AttribStringParser(entry)) {
             if (attrib.tag == "DATAFILE") {
               fileName = attrib.value;
               break;
             }
           }
           break;
         }
         if (fileName.empty()) {
           ATH_MSG_ERROR("Empty output file name for stream: " << targetStream);
           return StatusCode::FAILURE;
         }
         ATH_MSG_INFO( "RNTuple Output file: " << fileName );
         // naive implementation for AthAnalysis, I don't see any Ath Svc offer
         // getting the output stream easily
         outputFile = dynamic_cast<TFile*>(gROOT->GetListOfFiles()->FindObject(fileName.c_str()));
         ATH_MSG_INFO( "RNTuple found output file: " << (fileName.empty() ? "nullptr" : outputFile->GetName()) );
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
