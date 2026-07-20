/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AnaAlgorithm/AlgorithmWorkerData.h>
#include <AnaAlgorithm/IAlgorithmWrapper.h>
#include <AsgTesting/UnitTest.h>
#include <EventLoop/AlgorithmData.h>
#include <EventLoop/AlgorithmStateModule.h>
#include <EventLoop/ModuleData.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

//
// unit test
//

namespace EL
{
  namespace Detail
  {
    /// @brief minimal IAlgorithmWrapper for driving AlgorithmStateModule
    ///
    /// Records each callback in a counter so tests can assert which
    /// callbacks fired and how often.  When @ref requestSkip is true,
    /// @ref execute simulates @c setFilterPassed(false) by toggling
    /// @c ModuleData::m_skipEvent on the owning data block.
    class MockAlgorithm final : public IAlgorithmWrapper
    {
    public:
      MockAlgorithm (std::string name, ModuleData* owner)
        : m_name (std::move (name)), m_owner (owner) {}

      // callback counters
      unsigned initializeCount {0};
      unsigned executeCount {0};
      unsigned postExecuteCount {0};
      unsigned finalizeCount {0};
      unsigned fileExecuteCount {0};
      unsigned beginInputFileCount {0};
      unsigned endInputFileCount {0};

      // behavior knobs
      bool requestSkip {false};
      ::StatusCode executeStatus {StatusCode::SUCCESS};

      std::string_view getName () const override { return m_name; }

      bool hasName (const std::string& name) const override
      { return name == m_name; }

      std::unique_ptr<IAlgorithmWrapper> makeClone () const override
      { throw std::logic_error ("MockAlgorithm::makeClone not implemented"); }

      ::StatusCode initialize (const AlgorithmWorkerData& /*workerData*/) override
      { ++initializeCount; return StatusCode::SUCCESS; }

      ::StatusCode execute (const EventContext& /*ctx*/) override
      {
        ++executeCount;
        if (requestSkip)
          m_owner->m_skipEvent = true;
        return executeStatus;
      }

      ::StatusCode postExecute () override
      { ++postExecuteCount; return StatusCode::SUCCESS; }

      ::StatusCode finalize () override
      { ++finalizeCount; return StatusCode::SUCCESS; }

      ::StatusCode fileExecute () override
      { ++fileExecuteCount; return StatusCode::SUCCESS; }

      ::StatusCode beginInputFile () override
      { ++beginInputFileCount; return StatusCode::SUCCESS; }

      ::StatusCode endInputFile () override
      { ++endInputFileCount; return StatusCode::SUCCESS; }

    private:
      std::string m_name;
      ModuleData* m_owner {nullptr};
    };



    class AlgorithmStateModuleTest : public ::testing::Test
    {
    public:
      ModuleData data;
      AlgorithmStateModule module {"AlgorithmStateModule"};

      /// @brief append a MockAlgorithm to @c data.m_algs and return a raw
      /// pointer for the test to inspect
      MockAlgorithm* addAlg (const std::string& name, bool sequenceStart = false)
      {
        auto mock = std::make_unique<MockAlgorithm> (name, &data);
        MockAlgorithm* raw = mock.get();
        AlgorithmData entry (std::move (mock));
        entry.m_sequenceStart = sequenceStart;
        data.m_algs.emplace_back (std::move (entry));
        return raw;
      }
    };



    // initialize and finalize should fan out to every algorithm exactly
    // once.  A second onInitialize call must fail (guarded by m_initialized).
    TEST_F (AlgorithmStateModuleTest, initializeAndFinalize)
    {
      auto* a = addAlg ("a");
      auto* b = addAlg ("b");

      ASSERT_SUCCESS (module.onInitialize (data));
      EXPECT_EQ (1u, a->initializeCount);
      EXPECT_EQ (1u, b->initializeCount);

      // second initialize must fail
      EXPECT_EQ (StatusCode::FAILURE, module.onInitialize (data));

      ASSERT_SUCCESS (module.onFinalize (data));
      EXPECT_EQ (1u, a->finalizeCount);
      EXPECT_EQ (1u, b->finalizeCount);
    }



    // onFinalize without prior onInitialize is a no-op
    TEST_F (AlgorithmStateModuleTest, finalizeWithoutInitialize)
    {
      auto* a = addAlg ("a");
      ASSERT_SUCCESS (module.onFinalize (data));
      EXPECT_EQ (0u, a->finalizeCount);
    }



    // file-level callbacks fan out to every algorithm
    TEST_F (AlgorithmStateModuleTest, fileLevelCallbacks)
    {
      auto* a = addAlg ("a");
      auto* b = addAlg ("b");

      ASSERT_SUCCESS (module.onInitialize (data));

      data.m_hasInputEvents = true;
      ASSERT_SUCCESS (module.onNewInputFile (data));
      EXPECT_EQ (1u, a->beginInputFileCount);
      EXPECT_EQ (1u, b->beginInputFileCount);

      ASSERT_SUCCESS (module.onFileExecute (data));
      EXPECT_EQ (1u, a->fileExecuteCount);
      EXPECT_EQ (1u, b->fileExecuteCount);

      ASSERT_SUCCESS (module.onCloseInputFile (data));
      EXPECT_EQ (1u, a->endInputFileCount);
      EXPECT_EQ (1u, b->endInputFileCount);
    }



    // onNewInputFile must skip beginInputFile when the file has no events
    TEST_F (AlgorithmStateModuleTest, newInputFileEmptyFile)
    {
      auto* a = addAlg ("a");
      ASSERT_SUCCESS (module.onInitialize (data));

      data.m_hasInputEvents = false;
      ASSERT_SUCCESS (module.onNewInputFile (data));
      EXPECT_EQ (0u, a->beginInputFileCount);
    }



    // onNewInputFile without prior onInitialize must fail
    TEST_F (AlgorithmStateModuleTest, newInputFileWithoutInitialize)
    {
      addAlg ("a");
      data.m_hasInputEvents = true;
      EXPECT_EQ (StatusCode::FAILURE, module.onNewInputFile (data));
    }



    // single sequence, no skip: every algorithm gets execute + postExecute
    TEST_F (AlgorithmStateModuleTest, singleSequenceNoSkip)
    {
      auto* a = addAlg ("a");
      auto* b = addAlg ("b");
      auto* c = addAlg ("c");

      ASSERT_SUCCESS (module.onInitialize (data));
      ASSERT_SUCCESS (module.onExecute (data));

      for (auto* alg : {a, b, c})
      {
        EXPECT_EQ (1u, alg->executeCount);
        EXPECT_EQ (1u, alg->postExecuteCount);
      }
      for (auto& entry : data.m_algs)
      {
        EXPECT_EQ (1u, entry.m_executeCount);
        EXPECT_EQ (0u, entry.m_skipCount);
      }
    }



    // single sequence, first algorithm skips: with no further sequence
    // start the whole event is skipped, so no postExecute anywhere.
    TEST_F (AlgorithmStateModuleTest, singleSequenceSkipFirst)
    {
      auto* a = addAlg ("a");
      auto* b = addAlg ("b");
      auto* c = addAlg ("c");

      a->requestSkip = true;

      ASSERT_SUCCESS (module.onInitialize (data));
      ASSERT_SUCCESS (module.onExecute (data));

      EXPECT_EQ (1u, a->executeCount);
      EXPECT_EQ (0u, a->postExecuteCount);
      EXPECT_EQ (0u, b->executeCount);
      EXPECT_EQ (0u, b->postExecuteCount);
      EXPECT_EQ (0u, c->executeCount);
      EXPECT_EQ (0u, c->postExecuteCount);

      EXPECT_EQ (1u, data.m_algs[0].m_skipCount);
      EXPECT_EQ (0u, data.m_algs[1].m_skipCount);
      EXPECT_EQ (0u, data.m_algs[2].m_skipCount);
    }



    // two sequences, no skip: full execute + postExecute across both
    TEST_F (AlgorithmStateModuleTest, twoSequencesNoSkip)
    {
      auto* a = addAlg ("a");
      auto* b = addAlg ("b");
      auto* c = addAlg ("c", /*sequenceStart=*/true);
      auto* d = addAlg ("d");

      ASSERT_SUCCESS (module.onInitialize (data));
      ASSERT_SUCCESS (module.onExecute (data));

      for (auto* alg : {a, b, c, d})
      {
        EXPECT_EQ (1u, alg->executeCount);
        EXPECT_EQ (1u, alg->postExecuteCount);
      }
    }



    // two sequences, skip in the first: the first sequence's tail is
    // dropped (no postExecute), execution resumes at the next sequence
    // start, and the second sequence runs to completion.
    TEST_F (AlgorithmStateModuleTest, twoSequencesSkipFirst)
    {
      auto* a = addAlg ("a");
      auto* b = addAlg ("b");
      auto* c = addAlg ("c", /*sequenceStart=*/true);
      auto* d = addAlg ("d");

      a->requestSkip = true;

      ASSERT_SUCCESS (module.onInitialize (data));
      ASSERT_SUCCESS (module.onExecute (data));

      EXPECT_EQ (1u, a->executeCount);
      EXPECT_EQ (0u, a->postExecuteCount);
      EXPECT_EQ (0u, b->executeCount);
      EXPECT_EQ (0u, b->postExecuteCount);
      EXPECT_EQ (1u, c->executeCount);
      EXPECT_EQ (1u, c->postExecuteCount);
      EXPECT_EQ (1u, d->executeCount);
      EXPECT_EQ (1u, d->postExecuteCount);

      EXPECT_EQ (1u, data.m_algs[0].m_skipCount);
    }



    // two sequences, skip in the second: the first sequence runs to
    // completion (and gets postExecute), but the second sequence's tail
    // is dropped.
    TEST_F (AlgorithmStateModuleTest, twoSequencesSkipSecond)
    {
      auto* a = addAlg ("a");
      auto* b = addAlg ("b");
      auto* c = addAlg ("c", /*sequenceStart=*/true);
      auto* d = addAlg ("d");

      c->requestSkip = true;

      ASSERT_SUCCESS (module.onInitialize (data));
      ASSERT_SUCCESS (module.onExecute (data));

      EXPECT_EQ (1u, a->executeCount);
      EXPECT_EQ (1u, a->postExecuteCount);
      EXPECT_EQ (1u, b->executeCount);
      EXPECT_EQ (1u, b->postExecuteCount);
      EXPECT_EQ (1u, c->executeCount);
      EXPECT_EQ (0u, c->postExecuteCount);
      EXPECT_EQ (0u, d->executeCount);
      EXPECT_EQ (0u, d->postExecuteCount);

      EXPECT_EQ (1u, data.m_algs[2].m_skipCount);
    }



    // three sequences, skip in the middle: the first sequence completes
    // (postExecute), the middle sequence is dropped at the skip point,
    // and execution resumes at the third sequence start which runs to
    // completion.
    TEST_F (AlgorithmStateModuleTest, threeSequencesSkipMiddle)
    {
      auto* a = addAlg ("a");
      auto* b = addAlg ("b");
      auto* c = addAlg ("c", /*sequenceStart=*/true);
      auto* d = addAlg ("d");
      auto* e = addAlg ("e", /*sequenceStart=*/true);
      auto* f = addAlg ("f");

      c->requestSkip = true;

      ASSERT_SUCCESS (module.onInitialize (data));
      ASSERT_SUCCESS (module.onExecute (data));

      EXPECT_EQ (1u, a->executeCount);
      EXPECT_EQ (1u, a->postExecuteCount);
      EXPECT_EQ (1u, b->executeCount);
      EXPECT_EQ (1u, b->postExecuteCount);
      EXPECT_EQ (1u, c->executeCount);
      EXPECT_EQ (0u, c->postExecuteCount);
      EXPECT_EQ (0u, d->executeCount);
      EXPECT_EQ (0u, d->postExecuteCount);
      EXPECT_EQ (1u, e->executeCount);
      EXPECT_EQ (1u, e->postExecuteCount);
      EXPECT_EQ (1u, f->executeCount);
      EXPECT_EQ (1u, f->postExecuteCount);

      EXPECT_EQ (1u, data.m_algs[2].m_skipCount);
    }



    // m_skipEvent is reset each time onExecute is entered, so a skip on
    // one event must not bleed into the next.
    TEST_F (AlgorithmStateModuleTest, skipFlagResetBetweenEvents)
    {
      auto* a = addAlg ("a");
      auto* b = addAlg ("b");

      ASSERT_SUCCESS (module.onInitialize (data));

      a->requestSkip = true;
      ASSERT_SUCCESS (module.onExecute (data));
      EXPECT_EQ (1u, a->executeCount);
      EXPECT_EQ (0u, b->executeCount);

      a->requestSkip = false;
      ASSERT_SUCCESS (module.onExecute (data));
      EXPECT_EQ (2u, a->executeCount);
      EXPECT_EQ (1u, a->postExecuteCount);
      EXPECT_EQ (1u, b->executeCount);
      EXPECT_EQ (1u, b->postExecuteCount);
    }



    // a FAILURE return from execute() must propagate out and abort
    // processing of the remaining algorithms.
    TEST_F (AlgorithmStateModuleTest, executeFailurePropagates)
    {
      auto* a = addAlg ("a");
      auto* b = addAlg ("b");

      a->executeStatus = StatusCode::FAILURE;

      ASSERT_SUCCESS (module.onInitialize (data));
      EXPECT_EQ (StatusCode::FAILURE, module.onExecute (data));

      EXPECT_EQ (1u, a->executeCount);
      EXPECT_EQ (0u, a->postExecuteCount);
      EXPECT_EQ (0u, b->executeCount);
      EXPECT_EQ (0u, b->postExecuteCount);
    }
  }
}

ATLAS_GOOGLE_TEST_MAIN
