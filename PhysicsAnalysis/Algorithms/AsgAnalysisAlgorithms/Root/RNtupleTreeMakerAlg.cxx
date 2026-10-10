/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "AsgAnalysisAlgorithms/RNtupleTreeMakerAlg.h"

// ROOT include(s):
#include <TFile.h>
#include <TROOT.h>

// Gaudi/EventLoop include(s):
#ifdef XAOD_STANDALONE
#include "EventLoop/Worker.h"
#else
#include "Gaudi/Parsers/CommonParsers.h"
#endif

#ifndef XAOD_STANDALONE
namespace
{

TFile *fileForStream(const std::vector<std::string> &outputs,
                     const std::string &stream)
{
    std::string fileName;
    for (const std::string &entry : outputs) {
        const auto pos = entry.find_first_of(" \t");
        if (entry.substr(0, pos) == stream) {
            static const std::regex re(R"((?:DATA)?FILE\s*=\s*['"]([^'"]+)['"])", std::regex::icase);
            std::smatch m;
            if (std::regex_search(entry, m, re)) {
                fileName = m[1].str();
                break;
            }
            return nullptr;
        }
    }

    if (fileName.empty()) {
        return nullptr;
    }

    for (TObject *file : *gROOT->GetListOfFiles()) {
        if (file == nullptr) {
            continue;
        }

        if (file->GetName() == fileName) {
            return dynamic_cast<TFile*>(file);
        }
    }

    return nullptr;
}

}
#endif

namespace CP {

   StatusCode RNtupleTreeMakerAlg::initialize() {
      if( m_branches.empty() ) {
         ATH_MSG_ERROR( "No branches set up for writing" );
         return StatusCode::FAILURE;
      }
      ATH_CHECK( m_systematicsService.retrieve() );
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
         // Get the output file from THistSvc until we can use it directly
         SmartIF<IProperty> prop{histSvc().get()};
         std::vector<std::string> outputs;
         ATH_CHECK(Gaudi::Parsers::parse(outputs, prop->getProperty("Output").toString()));
         outputFile = fileForStream(outputs, m_outputStreamName.value());
         if (outputFile == nullptr) {
             ATH_MSG_ERROR( "Could not find TFile for stream: " << m_outputStreamName.value() );
             return StatusCode::FAILURE;
         }
#endif

         if( !outputFile ) {
             ATH_MSG_ERROR( "Could not retrieve file for stream: " << m_outputStreamName.value() );
             return StatusCode::FAILURE;
         }

         try {
             // ROOT requires the zipped target to not exceed the unzipped maximum
             // at every step, so set the zipped target first
             ROOT::RNTupleWriteOptions options;
             if( m_approxZippedClusterSize.value() > 0 ) {
                 options.SetApproxZippedClusterSize( m_approxZippedClusterSize.value() );
             }
             if( m_maxUnzippedClusterSize.value() > 0 ) {
                 if ( options.GetApproxZippedClusterSize() > m_maxUnzippedClusterSize.value() ) {
                    options.SetApproxZippedClusterSize( m_maxUnzippedClusterSize.value() );
                 }
                 options.SetMaxUnzippedClusterSize( m_maxUnzippedClusterSize.value() );
             }
             m_writer = ROOT::RNTupleWriter::Append( std::move(m_model), m_modelName.value(), *outputFile, options );
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
