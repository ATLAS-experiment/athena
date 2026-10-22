/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarTestFixtures/ColumnarPhysliteTest.h>

#include <ColumnarCore/ColumnarTool.h>
#include <ColumnarTestFixtures/Benchmark.h>
#include <ColumnarTestFixtures/ColumnReader.h>
#include <ColumnarTestFixtures/PerformanceData.h>
#include <ColumnarTestFixtures/PhysliteTest.h>
#include <ColumnarTestFixtures/ToolWrapper.h>
#include <ColumnarToolWrapper/ColumnVectorWrapper.h>
#include <ColumnarToolWrapper/ColumnarToolHelpers.h>

#include <PATInterfaces/ISystematicsTool.h>
#include <xAODRootAccess/Init.h>

#ifndef XAOD_STANDALONE
#include <POOLRootAccess/TEvent.h>
#endif

#include <TTree.h>

//
// method implementations
//

namespace columnar 
{
  // I'm moving code to this namespace but some of the code in this file
  // is still just in the columnar namespace. As I evolve the code I'll
  // move more of it to TestUtils.
  using namespace TestUtils;

  namespace TestUtils 
  {
    namespace
    {
      /// @brief wrapper around a tool and its benchmarks for mode 2
      struct ToolData
      {
        std::string name;
        ColumnarTool<ColumnarModeArray>* tool = nullptr;
        std::unique_ptr<ToolColumnVectorMap> toolWrapper;
        bool noRepeatCall = false;
        bool runToolTwice = false;

        Benchmark benchmarkCall;
        Benchmark benchmarkCall2;

        ToolData (const UserConfiguration& config, const TestDefinition& td,
                  ColumnVectorHeader& columnHeader)
          : name (td.name)
          , noRepeatCall (td.noRepeatCall)
          , runToolTwice (config.runToolTwice)
          , benchmarkCall ("", config.batchSize)
          , benchmarkCall2 ("", config.batchSize)
        {
          tool = dynamic_cast<ColumnarTool<ColumnarModeArray>*>(td.tool);
          if (!tool)
            throw std::runtime_error ("tool is not a ColumnarTool<ColumnarModeArray>: " + td.name);
          if (!td.containerRenames.empty())
            renameContainers (*tool, td.containerRenames);
          toolWrapper = std::make_unique<ToolColumnVectorMap> (columnHeader, *tool);
        }

        /// @brief call the tool with timing, optionally twice
        void call (ColumnVectorData& columnData)
        {
          benchmarkCall.startTimer ();
          columnData.callNoCheck (*tool);
          benchmarkCall.stopTimer ();
          if (runToolTwice && !noRepeatCall)
          {
            benchmarkCall2.startTimer ();
            columnData.callNoCheck (*tool);
            benchmarkCall2.stopTimer ();
          }
        }
      };
    }
  }




  ColumnarPhysLiteTest ::
  ColumnarPhysLiteTest (const std::type_info& val_columnar_mode_type)
    : columnar_mode_type (val_columnar_mode_type)
  {
    static std::once_flag flag;
    std::call_once (flag, [] ()
    {
#ifdef XAOD_STANDALONE
      xAOD::Init().ignore();
#else
      POOL::Init();
#endif
    });

    auto userConfiguration = TestUtils::UserConfiguration::fromEnvironment();
    const auto fileName = userConfiguration.getInputFileName();
    file.reset(TFile::Open(fileName.c_str(), "READ"));
    if (!file)
      throw std::runtime_error("failed to open file: " + fileName);
    if (userConfiguration.isrntuple) 
    {
      // the xAOD modes read the rntuple through the xAOD event, only
      // the columnar mode needs its own reader
      if (columnar_mode_type == typeid(columnar::ColumnarModeArray))
      {
        rntreader = ROOT::RNTupleReader::Open("EventData", fileName);
        inspector = ROOT::Experimental::RNTupleInspector::Create("EventData", fileName);
        if (!rntreader or !inspector)
          throw std::runtime_error("failed to open rntuple in file: " + fileName);
        rntbackend = new TestUtils::RNTupleBackend{rntreader.get(), inspector.get()};
      }
    } else 
    {
      tree = dynamic_cast<TTree*>(file->Get("CollectionTree"));
      if (!tree)
        throw std::runtime_error("failed to find CollectionTree in file: " + fileName);
    }
  }

  ColumnarPhysLiteTest ::~ColumnarPhysLiteTest() 
  {
    if (rntbackend)
      delete rntbackend;
  }

  std::string ColumnarPhysLiteTest :: makeUniqueName ()
  {
    static std::atomic<unsigned> index = 0;
    return "UniquePhysliteTestTool" + std::to_string(++index);
  }

  bool ColumnarPhysLiteTest ::
  checkMode ()
  {
    return true;
  }

  void ColumnarPhysLiteTest :: setupKnownColumns (std::span<const TestUtils::TestDefinition> testDefinitions, const ColumnVectorHeader& columnHeader)
  {
    using namespace TestUtils;

    makeCommonColumnReaders (knownColumns);

    if (tree) 
    {
      makeTTreeColumnReaders (knownColumns, tree);

    } else if (rntbackend) 
    {
      makeRNTupleColumnReaders (knownColumns, rntbackend);
    }

    // For METMaker we need to preplace all of the MET terms that we
    // expect to be used, that's what this line does.
    std::vector<std::string> allMetTermNames;
    for (const auto& td : testDefinitions)
    {
      for (const auto& name : td.metTermNames)
      {
        if (std::find (allMetTermNames.begin(), allMetTermNames.end(), name) == allMetTermNames.end())
          allMetTermNames.push_back (name);
      }
    }

    makeCommonLateColumnReaders (knownColumns, allMetTermNames);

    makeOutputColumnReaders (knownColumns, columnHeader);
  }

  void ColumnarPhysLiteTest :: setupColumns (const ColumnVectorHeader& columnHeader)
  {
    using namespace asg::msgUserCode;

    // Get all column info directly from the header (all tools have already
    // registered their columns via ToolColumnVectorMap)
    auto requestedColumns = columnHeader.getAllColumnInfo();

    // Print requested columns
    for (auto& [name, info] : requestedColumns)
      std::cout << "requested columns: " << name << std::endl;

    for (auto& column : knownColumns)
    {
      if (tree) 
      {
        if (column->connect (tree, offsetColumns, requestedColumns))
          usedColumns.push_back (column);
      } else if (rntbackend) 
      {
        if (column->connect(rntbackend, offsetColumns, requestedColumns))
          usedColumns.push_back(column);
      }
    }

    std::set<std::string> unclaimedColumns;
    for (auto& column : requestedColumns)
    {
      if (!column.second.isOptional)
        unclaimedColumns.insert (column.first);
      else
        std::cout << "optional column not claimed: " << column.first << std::endl;
    }
    if (!unclaimedColumns.empty()) 
    {
      std::string message = "columns not claimed:";
      for (auto& column : unclaimedColumns)
        message += " " + column;
      throw std::runtime_error(message);
    }
  }

  void ColumnarPhysLiteTest :: doCall (const TestDefinition& testDefinition)
  {
    doCallMulti ({testDefinition});
  }

  void ColumnarPhysLiteTest :: doCallMulti (const std::vector<TestDefinition>& testDefinitions) {
    using namespace asg::msgUserCode;
    auto userConfiguration = TestUtils::UserConfiguration::fromEnvironment();

    // apply systematics for all test definitions
    for (const auto& td : testDefinitions) {
      if (!td.sysName.empty()) {
        auto* sysTool = dynamic_cast<CP::ISystematicsTool*>(td.tool);
        if (!sysTool)
          throw std::runtime_error("tool does not support systematics");
        std::cout << "applying systematic variation: " << td.sysName << std::endl;
        if (sysTool->applySystematicVariation(CP::SystematicSet(td.sysName))
                .isFailure())
          throw std::runtime_error("failed to apply systematic variation: " +
                                  td.sysName);
      }
    }

    if (columnar_mode_type == typeid(columnar::ColumnarModeArray)) {
      // Create shared column header for all tools
      ColumnVectorHeader columnHeader;

      // Build vector of ToolData from all testDefinitions
      std::vector<TestUtils::ToolData> toolDataVec;
      for (const auto& td : testDefinitions)
        toolDataVec.emplace_back(userConfiguration, td, columnHeader);

      setupKnownColumns(testDefinitions, columnHeader);
      // Set up columns using the shared header (all tools have already
      // registered their columns via ToolColumnVectorMap, so we get all columns
      // from the header)
      setupColumns(columnHeader);

      // connect column indices from header to each column for direct setting
      for (auto& column : usedColumns)
        column->connectColumnIndices(columnHeader);

      Benchmark benchmarkEmpty("empty");
      Benchmark benchmarkCheck("", userConfiguration.batchSize);
      auto numberOfEvents = 0;
      if (tree) {
        numberOfEvents = tree->GetEntries();
      } else if (rntbackend) {
        numberOfEvents = rntreader->GetNEntries();
      }
      Long64_t entry = 0;
      const auto startTime = std::chrono::high_resolution_clock::now();
      bool endLoop = false;
      for (; !endLoop; ++entry) {
        // just sample how much overhead there is for starting and
        // stopping the timer
        benchmarkEmpty.startTimer();
        benchmarkEmpty.stopTimer();
        ColumnVectorData columnData(&columnHeader);
        for (auto& column : usedColumns)
          column->getEntry(entry % numberOfEvents);
        if ((entry + 1) % userConfiguration.batchSize == 0) {
          if (entry < numberOfEvents) {
            for (auto& column : usedColumns)
              column->collectColumnData();
          }
          for (auto& column : usedColumns)
            column->setData(columnData);

          // Check data once (shared column data)
          benchmarkCheck.startTimer();
          columnData.checkData();
          benchmarkCheck.stopTimer();
          // Call each tool
          for (auto& toolData : toolDataVec) {
            toolData.call(columnData);
          }
          for (auto& column : usedColumns)
            column->clearColumns();
          if ((std::chrono::high_resolution_clock::now() - startTime) >
              userConfiguration.targetTime)
            endLoop = true;
        } else if (entry + 1 == numberOfEvents) {
          for (auto& column : usedColumns)
            column->collectColumnData();
        }
      }
      std::cout << "Entries in file: " << numberOfEvents << std::endl;
      std::cout << "Total entries read: " << entry << std::endl;
      const float emptyTime = benchmarkEmpty.getEntryTime(0).value();
      std::cout << "Empty benchmark time: " << emptyTime << "ns (tick=" << Benchmark::getTickDuration() << "ns)" << std::endl;
      benchmarkEmpty.setSilence();
      const auto checkTime = benchmarkCheck.getEntryTime(emptyTime);
      if (checkTime)
        std::cout << "Check data time: " << checkTime.value() << "ns" << std::endl;
      benchmarkCheck.setSilence();
      {
        std::vector<TestUtils::BranchPerfData> branchPerfData;
        TestUtils::BranchPerfData summary;
        summary.name = "total";
        summary.timeRead = 0;
        summary.timeUnpack = 0;
        summary.timeShallowCopy = 0;
        summary.entrySize = 0;
        summary.uncompressedSize = 0;
        summary.numBaskets = 0;
        summary.entries = std::nullopt;
        summary.nullEntries = std::nullopt;
        for (auto& column : usedColumns)
        {
          branchPerfData.push_back (column->getPerfData (emptyTime));
          summary.timeRead.value() += branchPerfData.back().timeRead.value_or(0);
          summary.timeUnpack.value() += branchPerfData.back().timeUnpack.value_or(0);
          summary.entrySize.value() += branchPerfData.back().entrySize.value_or(0);
          summary.uncompressedSize.value() += branchPerfData.back().uncompressedSize.value_or(0);
          summary.numBaskets.value() += branchPerfData.back().numBaskets.value_or(0);
          summary.timeShallowCopy.value() += branchPerfData.back().timeShallowCopy.value_or(0);
        }
        std::sort (branchPerfData.begin(), branchPerfData.end(), [] (const auto& a, const auto& b) {return a.name < b.name;});
        branchPerfData.insert (branchPerfData.end(), summary);
        const std::size_t nameWidth = std::max_element (branchPerfData.begin(), branchPerfData.end(), [] (const auto& a, const auto& b) {return a.name.size() < b.name.size();})->name.size();
        std::string label = userConfiguration.isrntuple ? "field name" : "branch name";
        std::string header = std::format ("{:{}} | read(ns) | unpack(ns) | size(B) | rate(MB/s) | compression | baskets | entries | null", label, nameWidth);
        std::cout << "\n" << header << std::endl;
        std::cout << std::string (header.size(), '-') << std::endl;
        for (auto& data : branchPerfData)
        {
          if (data.name == "total")
            std::cout << std::string (header.size(), '-') << std::endl;
          std::cout << std::format ("{:{}} |", data.name, nameWidth);
          if (data.timeRead)
            std::cout << std::format ("{:>9.0f} |", data.timeRead.value());
          else
            std::cout << "          |";
          if (data.timeUnpack)
            std::cout << std::format ("{:>11.1f} |", data.timeUnpack.value());
          else
            std::cout << "            |";
          if (data.entrySize)
            std::cout << std::format ("{:>8.1f} |", data.entrySize.value());
          else
            std::cout << "         |";
          if (data.timeRead && data.entrySize)
            std::cout << std::format ("{:>11.1f} |", (data.entrySize.value() / (data.timeRead.value() * 1e-3 * 1.024 * 1.024)));
          else
            std::cout << "            |";
          if (data.entrySize && data.uncompressedSize)
            std::cout << std::format ("{:>12.2f} |", float (data.uncompressedSize.value()) / data.entrySize.value());
          else
            std::cout << "             |";
          if (data.numBaskets)
            std::cout << std::format ("{:>8} |", data.numBaskets.value());
          else
            std::cout << "         |";
          if (data.entries)
            std::cout << std::format ("{:>8.2f} |", static_cast<float>(data.entries.value())/numberOfEvents);
          else
            std::cout << "         |";
          if (data.nullEntries && data.entries)
            std::cout << std::format ("{:>4.0f}%", static_cast<float>(data.nullEntries.value()) / data.entries.value() * 100.0f);
          std::cout << std::endl;
        }
      }
      {
        std::vector<TestUtils::ToolPerfData> toolPerfData;
        for (auto& toolData : toolDataVec)
        {
          toolPerfData.emplace_back ();
          toolPerfData.back().name = toolData.name;
          toolPerfData.back().timeCall = toolData.benchmarkCall.getEntryTime (emptyTime);
          if (userConfiguration.runToolTwice)
            toolPerfData.back().timeCall2 = toolData.benchmarkCall2.getEntryTime (emptyTime);
        }
        const std::size_t nameWidth = std::max_element (toolPerfData.begin(), toolPerfData.end(), [] (const auto& a, const auto& b) {return a.name.size() < b.name.size();})->name.size();
        std::string header = std::format ("{:{}} | call(ns) | call2(ns)", "tool name", nameWidth);
        std::cout << "\n" << header << std::endl;
        std::cout << std::string (header.size(), '-') << std::endl;
        for (auto& data : toolPerfData)
        {
          std::cout << std::format ("{:{}} |", data.name, nameWidth);
          if (data.timeCall)
            std::cout << std::format ("{:>9.0f} |", data.timeCall.value());
          else
            std::cout << "          |";
          if (data.timeCall2)
            std::cout << std::format ("{:>10.0f}", data.timeCall2.value());
          else
            std::cout << "          ";
          std::cout << std::endl;
        }
        // Add totals line for multiple tools
        if (toolPerfData.size() > 1)
        {
          std::optional<float> totalCall, totalCall2;
          for (const auto& data : toolPerfData)
          {
            if (data.timeCall)
              totalCall = totalCall.value_or (0) + data.timeCall.value();
            if (data.timeCall2)
              totalCall2 = totalCall2.value_or (0) + data.timeCall2.value();
          }
          std::cout << std::string (header.size(), '-') << std::endl;
          std::cout << std::format ("{:{}} |", "total", nameWidth);
          if (totalCall)
            std::cout << std::format ("{:>9.0f} |", totalCall.value());
          else
            std::cout << "          |";
          if (totalCall2)
            std::cout << std::format ("{:>10.0f}", totalCall2.value());
          else
            std::cout << "          ";
          std::cout << std::endl;
        }
      }
    } else if (columnar_mode_type == typeid(columnar::ColumnarModeXAOD))
    {
      TestUtils::runXaodTest (userConfiguration, testDefinitions, file.get());
    } else if (columnar_mode_type == typeid(columnar::ColumnarModeXAODArray))
    {
      if (userConfiguration.isrntuple)
        throw std::runtime_error ("rntuple input is not supported in xAOD array mode");
      const auto& testDefinition = testDefinitions[0];
      TestUtils::runXaodArrayTest (userConfiguration, testDefinition, file.get());
    }
  }
}
