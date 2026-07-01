/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <ColumnarTestFixtures/PhysliteTest.h>

#include <AsgTesting/UnitTest.h>
#include <ColumnarInterfaces/KnownSgKeys.h>
#include <ColumnarTestFixtures/Benchmark.h>
#include <ColumnarTestFixtures/Configuration.h>
#include <ColumnarTestFixtures/IXAODToolCaller.h>
#include <ColumnarTestFixtures/PerformanceData.h>

#include <xAODRootAccess/TEvent.h>
#include <xAODRootAccess/TStore.h>
#ifndef XAOD_STANDALONE
#include <POOLRootAccess/TEvent.h>
#endif

#include <Rtypes.h>

#include <algorithm>
#include <array>
#include <format>
#include <iostream>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

//
// method implementations
//

namespace columnar
{
  namespace TestUtils
  {
    namespace
    {
      using namespace columnar::TestUtils;

      /// @brief interface for event reading abstraction
      struct IEventReader
      {
        virtual ~IEventReader() = default;
        virtual StatusCode readFrom(TFile* file) = 0;
        virtual Long64_t getEntries() = 0;
        virtual StatusCode getEntry(std::uint64_t entry) = 0;
        virtual void clearStore() = 0;
        virtual bool hasClearStore() const = 0;
        virtual void printBenchmarkTable(float emptyTime) = 0;
      };


      /// @brief event reader using xAODRootAccess (TEvent + TStore)
      ///
      /// This is also compiled (though not used) in Athena. That is
      /// just so that the code still gets checked during compilation in
      /// Athena projects.
      struct XAODEventReader final : IEventReader
      {
        xAOD::TEvent m_event;
        xAOD::TStore m_store;
        Benchmark m_benchmarkGetEntry;
        Benchmark m_benchmarkClear;
        Benchmark m_benchmarkEmptyClear;

        StatusCode readFrom(TFile* file) override
        {
          using namespace asg::msgUserCode;
          ANA_CHECK(m_event.readFrom(file));
          // Print known keys diagnostic
          std::cout << "known container keys:" << std::endl;
          for (auto& [container, key] : knownSgKeys)
          {
            std::cout << std::format("  {} -> 0x{:x}, 0x{:x} -> {}",
              container, m_event.getHash(container), key, m_event.getName(key)) << std::endl;
          }
          return StatusCode::SUCCESS;
        }

        Long64_t getEntries() override
        {
          return m_event.getEntries();
        }

        StatusCode getEntry(std::uint64_t entry) override
        {
          m_benchmarkGetEntry.startTimer();
          auto result = m_event.getEntry(entry);
          m_benchmarkGetEntry.stopTimer();
          return result >= 0 ? StatusCode::SUCCESS : StatusCode::FAILURE;
        }

        void clearStore() override
        {
          m_benchmarkClear.startTimer();
          m_store.clear();
          m_benchmarkClear.stopTimer();
          // Measure empty-clear overhead
          m_benchmarkEmptyClear.startTimer();
          m_store.clear();
          m_benchmarkEmptyClear.stopTimer();
        }

        bool hasClearStore() const override { return true; }

        void printBenchmarkTable(float emptyTime) override
        {
          std::string header = "         | getEntry(ns) | clear(ns) | empty clear(ns)";
          std::cout << "\n" << header << std::endl;
          std::cout << std::string(header.size(), '-') << std::endl;
          std::cout << "xAOD     |";
          auto getEntryTime = m_benchmarkGetEntry.getEntryTime(emptyTime);
          auto clearTime = m_benchmarkClear.getEntryTime(emptyTime);
          auto emptyClearTime = m_benchmarkEmptyClear.getEntryTime(emptyTime);
          if (getEntryTime)
            std::cout << std::format("{:>13.0f} |", getEntryTime.value());
          else
            std::cout << "              |";
          if (clearTime)
            std::cout << std::format("{:>10.0f} |", clearTime.value());
          else
            std::cout << "           |";
          if (emptyClearTime)
            std::cout << std::format("{:>16.0f}", emptyClearTime.value());
          std::cout << std::endl;
          m_benchmarkGetEntry.setSilence();
          m_benchmarkClear.setSilence();
          m_benchmarkEmptyClear.setSilence();
        }
      };


#ifndef XAOD_STANDALONE
      /// @brief event reader using POOLRootAccess (TEvent only, no TStore)
      struct POOLEventReader final : IEventReader
      {
        POOL::TEvent m_event;
        Benchmark m_benchmarkGetEntry;

        StatusCode readFrom(TFile* file) override
        {
          return m_event.readFrom(file);
        }

        Long64_t getEntries() override
        {
          return m_event.getEntries();
        }

        StatusCode getEntry(std::uint64_t entry) override
        {
          m_benchmarkGetEntry.startTimer();
          auto result = m_event.getEntry(entry);
          m_benchmarkGetEntry.stopTimer();
          return result >= 0 ? StatusCode::SUCCESS : StatusCode::FAILURE;
        }

        void clearStore() override
        {
          // POOL::TEvent has no separate store to clear
        }

        bool hasClearStore() const override { return false; }

        void printBenchmarkTable(float emptyTime) override
        {
          std::string header = "         | getEntry(ns)";
          std::cout << "\n" << header << std::endl;
          std::cout << std::string(header.size(), '-') << std::endl;
          std::cout << "POOL     |";
          auto getEntryTime = m_benchmarkGetEntry.getEntryTime(emptyTime);
          if (getEntryTime)
            std::cout << std::format("{:>13.0f}", getEntryTime.value());
          std::cout << std::endl;
          m_benchmarkGetEntry.setSilence();
        }
      };
#endif


      std::unique_ptr<IEventReader> makeEventReader()
      {
        // while xAOD::TEvent/TStore exists in Athena builds, they don't
        // allow objects to be retrieved via the event store, so we use
        // POOLRootAccess in Athena builds.
#ifdef XAOD_STANDALONE
        return std::make_unique<XAODEventReader>();
#else
        return std::make_unique<POOLEventReader>();
#endif
      }


      /// @brief wrapper around a tool and its benchmarks
      ///
      /// This encapsulates the IXAODToolCaller and wraps each call with
      /// benchmarking. The index parameter indicates which call phase:
      /// 0=prep, 1=call, 2=repeat.
      struct ToolData
      {
        std::string name;
        IXAODToolCaller* xAODToolCaller = nullptr;
        bool noRepeatCall = false;

        // Arrays of benchmarks indexed by call index
        // retrieve and copyRecord only used for prep and call (not repeat)
        std::array<Benchmark, 2> benchmarkRetrieve;
        std::array<Benchmark, 2> benchmarkCopyRecord;
        std::array<Benchmark, 3> benchmarkCall;
        std::array<Benchmark, 2> benchmarkClear;

        StatusCode retrieve(std::size_t index, IXAODToolCaller::EventStoreType& evtStore)
        {
          if (index >= benchmarkRetrieve.size())
            throw std::out_of_range("ToolData::retrieve index out of range");
          benchmarkRetrieve[index].startTimer();
          auto result = xAODToolCaller->retrieve(evtStore);
          benchmarkRetrieve[index].stopTimer();
          return result;
        }

        StatusCode copyRecord(std::size_t index, IXAODToolCaller::EventStoreType& evtStore,
                              const std::string& postfix)
        {
          if (index >= benchmarkCopyRecord.size())
            throw std::out_of_range("ToolData::copyRecord index out of range");
          benchmarkCopyRecord[index].startTimer();
          auto result = xAODToolCaller->copyRecord(evtStore, postfix);
          benchmarkCopyRecord[index].stopTimer();
          return result;
        }

        StatusCode call(std::size_t index)
        {
          if (index >= benchmarkCall.size())
            throw std::out_of_range("ToolData::call index out of range");
          benchmarkCall[index].startTimer();
          auto result = xAODToolCaller->call();
          benchmarkCall[index].stopTimer();
          return result;
        }

        void clear(std::size_t index)
        {
          if (index >= benchmarkClear.size())
            throw std::out_of_range("ToolData::clear index out of range");
          benchmarkClear[index].startTimer();
          xAODToolCaller->clear();
          benchmarkClear[index].stopTimer();
        }
      };
    }



    void runXaodTest (const UserConfiguration& userConfiguration, std::span<const TestDefinition> testDefinitions, TFile *file)
    {
      using namespace asg::msgUserCode;

      auto eventReader = makeEventReader();
      ANA_CHECK_THROW(eventReader->readFrom(file));

      std::vector<ToolData> toolDataVec;
      for (const auto& testDefinition : testDefinitions)
      {
        if (testDefinition.xAODToolCaller != nullptr)
        {
          ToolData toolData;
          toolData.name = testDefinition.name;
          toolData.xAODToolCaller = testDefinition.xAODToolCaller;
          toolData.noRepeatCall = testDefinition.noRepeatCall;
          toolDataVec.push_back(std::move(toolData));
        }
      }

      IXAODToolCaller::EventStoreType* evtStore = nullptr;
      if (!testDefinitions.empty() && testDefinitions[0].tool)
        evtStore = &*testDefinitions[0].tool->evtStore();

      Benchmark benchmarkEmpty;

      const std::uint64_t numberOfEvents = eventReader->getEntries();
      if (numberOfEvents == 0){
        throw std::runtime_error ("ColumnarPhysLiteTest: numberOfEvents == 0");
      }
      std::uint64_t entry = 0;

      // Instead of running for a fixed number of events, we run for a
      // fixed amount of time.  That is because individual tools can
      // vary wildly in how long they take to run, and we mostly want to
      // make sure that we ran the tool enough to get a precise
      // performance estimate.
      const auto startTime = std::chrono::high_resolution_clock::now();
      for (; (std::chrono::high_resolution_clock::now() - startTime) < userConfiguration.targetTime; ++entry)
      {
        benchmarkEmpty.startTimer();
        benchmarkEmpty.stopTimer();
        ANA_CHECK_THROW(eventReader->getEntry(entry % numberOfEvents));
        if (eventReader->hasClearStore())
        {
          static const std::string prepPostfix = "Prep";
          for (auto& toolData : toolDataVec)
            ASSERT_SUCCESS (toolData.retrieve (0, *evtStore));
          for (auto& toolData : toolDataVec)
            ASSERT_SUCCESS (toolData.copyRecord (0, *evtStore, prepPostfix));
          for (auto& toolData : toolDataVec)
            ASSERT_SUCCESS (toolData.call (0));
          for (auto& toolData : toolDataVec)
            toolData.clear (0);
          eventReader->clearStore();
        }
        static const std::string callPostfix = "Call";
        for (auto& toolData : toolDataVec)
          ASSERT_SUCCESS (toolData.retrieve (1, *evtStore));
        for (auto& toolData : toolDataVec)
          ASSERT_SUCCESS (toolData.copyRecord (1, *evtStore, callPostfix));
        for (auto& toolData : toolDataVec)
        {
          ASSERT_SUCCESS (toolData.call (1));
          // a second call, immediately after, to have the tool in
          // instruction cache and simulate it being run on multi-event
          // batches. this is a bit too good, as it will run on exactly
          // the same event, instead of different events as would happen
          // in a real multi-event batch.
          if (userConfiguration.runToolTwice && !toolData.noRepeatCall)
          {
            ASSERT_SUCCESS (toolData.call (2));
          }
        }
        for (auto& toolData : toolDataVec)
          toolData.clear (1);
        if (eventReader->hasClearStore())
          eventReader->clearStore();
      }
      std::cout << "Total entries read: " << entry << std::endl;
      const float emptyTime = benchmarkEmpty.getEntryTime(0).value();
      std::cout << "Empty benchmark time: " << emptyTime << "ns (tick=" << Benchmark::getTickDuration() << "ns)" << std::endl;
      eventReader->printBenchmarkTable(emptyTime);

      // Fill vector of ToolPerfData pairs from benchmarks
      std::vector<std::pair<ToolPerfData, ToolPerfData>> toolPerfData;
      for (auto& toolData : toolDataVec)
      {
        ToolPerfData prepPerfData;
        prepPerfData.name = toolData.name;
        if (eventReader->hasClearStore()) {
          prepPerfData.timeRetrieve = toolData.benchmarkRetrieve[0].getEntryTime(emptyTime);
          prepPerfData.timeCopyRecord = toolData.benchmarkCopyRecord[0].getEntryTime(emptyTime);
          prepPerfData.timeCall = toolData.benchmarkCall[0].getEntryTime(emptyTime);
          prepPerfData.timeClear = toolData.benchmarkClear[0].getEntryTime(emptyTime);
          toolData.benchmarkRetrieve[0].setSilence();
          toolData.benchmarkCopyRecord[0].setSilence();
          toolData.benchmarkCall[0].setSilence();
          toolData.benchmarkClear[0].setSilence();
        }

        ToolPerfData callPerfData;
        callPerfData.name = toolData.name;
        callPerfData.timeRetrieve = toolData.benchmarkRetrieve[1].getEntryTime(emptyTime);
        callPerfData.timeCopyRecord = toolData.benchmarkCopyRecord[1].getEntryTime(emptyTime);
        callPerfData.timeCall = toolData.benchmarkCall[1].getEntryTime(emptyTime);
        callPerfData.timeClear = toolData.benchmarkClear[1].getEntryTime(emptyTime);
        if (userConfiguration.runToolTwice) {
          callPerfData.timeCall2 = toolData.benchmarkCall[2].getEntryTime(emptyTime);
          toolData.benchmarkCall[2].setSilence();
        }
        toolData.benchmarkRetrieve[1].setSilence();
        toolData.benchmarkCopyRecord[1].setSilence();
        toolData.benchmarkCall[1].setSilence();
        toolData.benchmarkClear[1].setSilence();

        toolPerfData.emplace_back(std::move(prepPerfData), std::move(callPerfData));
      }

      // Helper lambda for printing optional time values
      auto printOptionalTime = [](std::optional<float> time, int width, bool trailingBar = true) {
        if (time)
          std::cout << std::format("{:>{}.0f}", time.value(), width);
        else
          std::cout << std::format("{:>{}}", "", width);
        if (trailingBar)
          std::cout << " |";
      };

      // Calculate name width from all entries
      std::size_t nameWidth = std::string_view("tool name").size();
      for (const auto& [prep, call] : toolPerfData)
        nameWidth = std::max(nameWidth, prep.name.size());

      // Print tool performance table with grouped columns
      std::string groupHeader = std::format("{:{}} |{:^42}|{:^52}",
        "", nameWidth, "prep", "call");
      std::string colHeader = std::format("{:{}} | retr(ns) | copy(ns) | call(ns) | clr(ns) | retr(ns) | copy(ns) | call(ns) | 2nd(ns) | clr(ns)",
        "tool name", nameWidth);

      std::cout << "\ntool benchmarks:" << std::endl;
      std::cout << groupHeader << std::endl;
      std::cout << colHeader << std::endl;
      std::cout << std::string(colHeader.size(), '-') << std::endl;

      // Print data rows
      for (const auto& [prep, call] : toolPerfData)
      {
        std::cout << std::format("{:{}} |", prep.name, nameWidth);
        printOptionalTime(prep.timeRetrieve, 9);
        printOptionalTime(prep.timeCopyRecord, 9);
        printOptionalTime(prep.timeCall, 9);
        printOptionalTime(prep.timeClear, 8);
        printOptionalTime(call.timeRetrieve, 9);
        printOptionalTime(call.timeCopyRecord, 9);
        printOptionalTime(call.timeCall, 9);
        printOptionalTime(call.timeCall2, 8);
        printOptionalTime(call.timeClear, 8, false);
        std::cout << std::endl;
      }

      // Print total line only when there are multiple tools
      if (toolPerfData.size() > 1)
      {
        std::optional<float> totalPrepRetrieve, totalPrepCopyRecord, totalPrepCall, totalPrepClear;
        std::optional<float> totalCallRetrieve, totalCallCopyRecord, totalCallCall, totalCallClear, totalRepeat;
        for (const auto& [prep, call] : toolPerfData)
        {
          if (prep.timeRetrieve)
            totalPrepRetrieve = totalPrepRetrieve.value_or(0) + prep.timeRetrieve.value();
          if (prep.timeCopyRecord)
            totalPrepCopyRecord = totalPrepCopyRecord.value_or(0) + prep.timeCopyRecord.value();
          if (prep.timeCall)
            totalPrepCall = totalPrepCall.value_or(0) + prep.timeCall.value();
          if (prep.timeClear)
            totalPrepClear = totalPrepClear.value_or(0) + prep.timeClear.value();
          if (call.timeRetrieve)
            totalCallRetrieve = totalCallRetrieve.value_or(0) + call.timeRetrieve.value();
          if (call.timeCopyRecord)
            totalCallCopyRecord = totalCallCopyRecord.value_or(0) + call.timeCopyRecord.value();
          if (call.timeCall)
            totalCallCall = totalCallCall.value_or(0) + call.timeCall.value();
          if (call.timeClear)
            totalCallClear = totalCallClear.value_or(0) + call.timeClear.value();
          if (call.timeCall2)
            totalRepeat = totalRepeat.value_or(0) + call.timeCall2.value();
        }

        std::cout << std::string(colHeader.size(), '-') << std::endl;
        std::cout << std::format("{:{}} |", "total", nameWidth);
        printOptionalTime(totalPrepRetrieve, 9);
        printOptionalTime(totalPrepCopyRecord, 9);
        printOptionalTime(totalPrepCall, 9);
        printOptionalTime(totalPrepClear, 8);
        printOptionalTime(totalCallRetrieve, 9);
        printOptionalTime(totalCallCopyRecord, 9);
        printOptionalTime(totalCallCall, 9);
        printOptionalTime(totalRepeat, 8);
        printOptionalTime(totalCallClear, 8, false);
        std::cout << std::endl;
      }
    }
  }
}
