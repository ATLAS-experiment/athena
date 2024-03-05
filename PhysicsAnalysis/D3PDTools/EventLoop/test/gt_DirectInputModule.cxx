/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgTesting/UnitTest.h>
#include <EventLoop/IInputModuleActions.h>
#include <EventLoop/ModuleData.h>
#include <EventLoop/DirectInputModule.h>
#include <EventLoop/EventRange.h>
#include <EventLoop/MessageCheck.h>
#include <TH1F.h>
#include <TKey.h>
#include <TSystem.h>
#include <TTree.h>
#include <filesystem>

//
// unit test
//

namespace EL
{
  namespace Detail
  {
    struct TestActions final : public IInputModuleActions
    {
      std::unordered_map<std::string,unsigned> inputFiles;
      std::optional<Long64_t> inputFileNumEntries_result;
      std::vector<EventRange> toProcessFiles;
      std::size_t processIndex = 0u;

      virtual ::StatusCode processEvents (EventRange& eventRange) override
      {
        using namespace msgEventLoop;

        // do the standard behavior
        ANA_CHECK (openInputFile (eventRange.m_url));
        if (eventRange.m_endEvent == EventRange::eof)
          eventRange.m_endEvent = inputFileNumEntries();

        if (processIndex >= toProcessFiles.size())
          return StatusCode::FAILURE;
        EXPECT_EQ (eventRange.m_url, toProcessFiles[processIndex].m_url);
        EXPECT_EQ (eventRange.m_beginEvent, toProcessFiles[processIndex].m_beginEvent);
        EXPECT_EQ (eventRange.m_endEvent, toProcessFiles[processIndex].m_endEvent);
        processIndex += 1u;
        return StatusCode::SUCCESS;
      }

      virtual ::StatusCode openInputFile (const std::string& inputFileUrl) override
      {
        using namespace msgEventLoop;

        if (inputFileUrl.empty())
        {
          inputFileNumEntries_result.reset();
          return StatusCode::SUCCESS;
        }
        auto iter = inputFiles.find (inputFileUrl);
        if (iter == inputFiles.end())
        {
          ANA_MSG_ERROR ("unknown input file: " << inputFileUrl);
          return StatusCode::FAILURE;
        }
        inputFileNumEntries_result = iter->second;
        return StatusCode::SUCCESS;
      }

      [[nodiscard]] virtual Long64_t inputFileNumEntries () const override
      {
        if (!inputFileNumEntries_result.has_value())
          throw std::logic_error ("no input file open");
        return inputFileNumEntries_result.value();
      }
    };



    class DirectInputModuleTest : public ::testing::Test  {
    public:
      DirectInputModuleTest()
      {
        actions.inputFiles["empty.root"] = 0u;
        actions.inputFiles["test1.root"] = 10u;
        actions.inputFiles["test2.root"] = 20u;
      }

      ModuleData data;
      TestActions actions;
    };



    TEST_F (DirectInputModuleTest, simpleTest)
    {
      auto module = std::make_unique<DirectInputModule> ("DirectInputModule");
      module->fileList = {"test1.root", "test2.root"};

      actions.toProcessFiles =
      {
        {"test1.root", 0, 10},
        {"test2.root", 0, 20},
      };

      ASSERT_SUCCESS (module->processInputs (data, actions));
    }



    TEST_F (DirectInputModuleTest, skipLimitTest)
    {
      auto module = std::make_unique<DirectInputModule> ("DirectInputModule");
      module->fileList = {"test1.root", "test2.root"};
      module->skipEvents = 5u;
      module->maxEvents = 10u;

      actions.toProcessFiles =
      {
        {"test1.root", 5, 10},
        {"test2.root", 0, 5},
      };

      ASSERT_SUCCESS (module->processInputs (data, actions));
    }



    TEST_F (DirectInputModuleTest, skipFileTest)
    {
      auto module = std::make_unique<DirectInputModule> ("DirectInputModule");
      module->fileList = {"test1.root", "test2.root"};
      module->skipEvents = 15u;

      actions.toProcessFiles =
      {
        {"test2.root", 5, 20},
      };

      ASSERT_SUCCESS (module->processInputs (data, actions));
    }



    TEST_F (DirectInputModuleTest, limitFileTest)
    {
      auto module = std::make_unique<DirectInputModule> ("DirectInputModule");
      module->fileList = {"test1.root", "test2.root"};
      module->maxEvents = 5u;

      actions.toProcessFiles =
      {
        {"test1.root", 0, 5},
      };

      ASSERT_SUCCESS (module->processInputs (data, actions));
    }



    TEST_F (DirectInputModuleTest, emptyFileTest)
    {
      auto module = std::make_unique<DirectInputModule> ("DirectInputModule");
      module->fileList = {"empty.root", "test1.root", "test2.root"};

      actions.toProcessFiles =
      {
        {"empty.root", 0, 0},
        {"test1.root", 0, 10},
        {"test2.root", 0, 20},
      };
      ASSERT_SUCCESS (module->processInputs (data, actions));
    }



    TEST_F (DirectInputModuleTest, skipToEmptyTest)
    {
      auto module = std::make_unique<DirectInputModule> ("DirectInputModule");
      module->fileList = {"test1.root", "empty.root", "test2.root"};
      module->skipEvents = 10u;

      actions.toProcessFiles =
      {
        {"empty.root", 0, 0},
        {"test2.root", 0, 20},
      };

      ASSERT_SUCCESS (module->processInputs (data, actions));
    }
  }
}

ATLAS_GOOGLE_TEST_MAIN
