/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgTesting/UnitTest.h>
#include <AsgMessaging/AsgMessaging.h>
#include <EventLoop/IInputModuleActions.h>
#include <EventLoop/ModuleData.h>
#include <EventLoop/EventModule.h>
#include <EventLoop/EventRange.h>
#include <EventLoop/MessageCheck.h>
#include <xAODRootAccess/Event.h>
#include <TH1F.h>
#include <TKey.h>
#include <TSystem.h>
#include <TFile.h>
#include <filesystem>

//
// unit test
//

namespace EL
{
  namespace Detail
  {
    ::StatusCode openInputFile (const std::string& inputFileUrl, std::unique_ptr< TFile >& inputFile) {

      using namespace msgEventLoop;

      if (inputFileUrl.empty())
      {
        // inputFileNumEntries_result.reset();
        return StatusCode::SUCCESS;
      }
      // auto iter = inputFiles.find (inputFileUrl);
      // if (iter == inputFiles.end())
      // {
      //   ANA_MSG_ERROR ("unknown input file: " << inputFileUrl);
      //   return StatusCode::FAILURE;
      // }
      // inputFileNumEntries_result = iter->second;
      std::unique_ptr< TFile > ifile( TFile::Open( inputFileUrl.c_str(), "READ" ) );
      inputFile = std::move( ifile );
      return StatusCode::SUCCESS;
    }


    TEST (EventModuleTest, readTest)
    {

      uint64_t maxNumEntries = 10;
      auto module = std::make_unique<EventModule> ("EventModule");
      ModuleData data;

      std::vector<std::string> inputFiles;
      inputFiles.push_back(gSystem->Getenv( "ASG_TEST_FILE_LITE_RUN3_DATA" ));
      inputFiles.push_back(gSystem->Getenv( "ASG_TEST_FILE_RUN2_LITE_RNTUPLE_DATA" ));

      for ( auto& file : inputFiles ) {

        // Open input file
        ASSERT_SUCCESS ( openInputFile(file, data.m_inputFile) );

        std::cout << "EventModuleTest: opened file " << file << std::endl;


        // initialize the module
        ASSERT_SUCCESS (module->onFirstInputFile (data));

        // Check if a few collections are in the event format
        std::cout << "event format: " << data.m_event->inputEventFormat() << std::endl;
        ASSERT_TRUE (data.m_event->inputEventFormat() != nullptr); 
        if (data.m_event->inputEventFormat()) {
          std::cout << "AnalysisElectrons exist: " << data.m_event->inputEventFormat()->exists("AnalysisElectrons") << std::endl;
          ASSERT_TRUE (data.m_event->inputEventFormat()->exists("AnalysisElectrons"));
          std::cout << "AnalysisMuons exist: " << data.m_event->inputEventFormat()->exists("AnalysisMuons") << std::endl;
          ASSERT_TRUE (data.m_event->inputEventFormat()->exists("AnalysisMuons"));
          std::cout << "EventInfo exist: " << data.m_event->inputEventFormat()->exists("EventInfo") << std::endl;
          ASSERT_TRUE (data.m_event->inputEventFormat()->exists("EventInfo"));
        }

        // Loop over events
        for (data.m_inputEntry = 0; data.m_inputEntry < maxNumEntries; ++data.m_inputEntry) {
          std::cout << "EventModuleTest: read entry " << data.m_inputEntry << std::endl;
          ASSERT_SUCCESS (module->onExecute (data));
        }      

        // open the 'next' file - only one file is available, so we use that one
        ASSERT_SUCCESS (module->onNewInputFile (data));
        std::cout << "EventModuleTest: new input file " << file << std::endl;


        // Loop over events
        for (data.m_inputEntry = 0; data.m_inputEntry < maxNumEntries; ++data.m_inputEntry) {
          std::cout << "EventModuleTest: read entry " << data.m_inputEntry << std::endl;
          ASSERT_SUCCESS (module->onExecute (data));
        }   
        
        // Cleanup
        ASSERT_SUCCESS (module->postFinalize (data));
      }

    }

  }
}

ATLAS_GOOGLE_TEST_MAIN
