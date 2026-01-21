/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <ColumnarTestFixtures/PhysliteTest.h>

#include <ColumnarCore/ColumnarTool.h>
#include <ColumnarTestFixtures/PerformanceData.h>
#include <ColumnarInterfaces/ColumnInfo.h>
#include <ColumnarTestFixtures/Benchmark.h>
#include <ColumnarTestFixtures/Configuration.h>
#include <ColumnarToolWrapper/ColumnarToolHelpers.h>
#include <ColumnarToolWrapper/ToolColumnVectorMap.h>

#include <xAODCaloEvent/CaloClusterContainer.h>
#include <xAODCore/ShallowCopy.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODMuon/MuonContainer.h>

#include <AsgTesting/UnitTest.h>
#include <xAODRootAccess/TEvent.h>
#include <xAODRootAccess/TStore.h>

#include <boost/core/demangle.hpp>
#include <format>
#include <gtest/gtest.h>

//
// method implementations
//

namespace columnar
{
  namespace TestUtils
  {
    namespace
    {
      struct IColumnReaderXA;

      struct ColumnDataXA
      {
        ColumnInfo info;
        std::shared_ptr<TestUtils::IColumnReaderXA> reader;
      };

      struct IColumnReaderXA
      {
        virtual ~IColumnReaderXA () = default;

        virtual void connect (ColumnDataXA& /*columnData*/, std::unordered_map<std::string,ColumnDataXA>& /*requestedColumns*/) {}

        virtual void requestShallowCopy () {
          throw std::runtime_error ("shallow copy not supported for column reader");
        }

        virtual StatusCode retrieveContainer (xAOD::TEvent& /*event*/, xAOD::TStore& /*store*/, ColumnVectorData& /*columnData*/) { return StatusCode::SUCCESS; };

        virtual std::pair<const SG::AuxElement*,std::size_t> getObject () const {
          throw std::runtime_error ("getObject not implemented for this XAOD column reader"); }

        virtual void retrieveAuxData (ColumnVectorData& /*columnData*/) {}

        [[nodiscard]] virtual BranchPerfData getPerfData (float /*emptyTime*/) = 0;
      };

      struct ColumnDataXAEventInfo final : public IColumnReaderXA, asg::AsgMessaging
      {
        std::string m_name = "EventInfo";
        unsigned index = 0;
        std::array<ColumnarOffsetType, 2> data = {0, 1};
        const xAOD::EventInfo* eventInfo = nullptr;
        Benchmark benchmarkFirstRetrieve;
        Benchmark benchmarkSecondRetrieve;

        ColumnDataXAEventInfo (const ColumnInfo& info)
          : AsgMessaging ("ColumnDataXAEventInfo"), index (info.index)
        {}

        virtual StatusCode retrieveContainer (xAOD::TEvent& event, xAOD::TStore& /*store*/, ColumnVectorData& columnData) override
        {
          benchmarkFirstRetrieve.startTimer ();
          ATH_CHECK (event.retrieve (eventInfo, "EventInfo"));
          benchmarkFirstRetrieve.stopTimer ();
          benchmarkSecondRetrieve.startTimer ();
          ATH_CHECK (event.retrieve (eventInfo, "EventInfo"));
          benchmarkSecondRetrieve.stopTimer ();
          columnData.setColumn (index, data.size(), data.data());
          return StatusCode::SUCCESS;
        }

        virtual std::pair<const SG::AuxElement*,std::size_t> getObject () const override
        {
          return {eventInfo, 1};
        }

        [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
        {
          BranchPerfData result;
          result.name = "EventInfo";
          result.timeRead = benchmarkFirstRetrieve.getEntryTime(emptyTime);
          result.timeReadAgain = benchmarkSecondRetrieve.getEntryTime(emptyTime);
          benchmarkFirstRetrieve.setSilence();
          benchmarkSecondRetrieve.setSilence();
          return result;
        }
      };

      template<typename XAODObjectType>
      struct ColumnDataXARetrieve final : public IColumnReaderXA, asg::AsgMessaging
      {
        std::string containerName;
        unsigned index = 0;
        bool shallowCopy = false;
        bool skipShallowCopies = false;
        std::array<ColumnarOffsetType, 2> data = {0, 1};
        const XAODObjectType* object = nullptr;
        Benchmark benchmarkFirstRetrieve;
        Benchmark benchmarkSecondRetrieve;
        Benchmark benchmarkShallowCopy;
        Benchmark benchmarkShallowRegister;

        ColumnDataXARetrieve (const ColumnInfo& info, const UserConfiguration& userConfiguration)
          : AsgMessaging ("ColumnDataXARetrieve_" + info.name), containerName (info.name), index (info.index), skipShallowCopies (userConfiguration.skipShallowCopies)
        {}

        virtual void requestShallowCopy() override {
          if (!skipShallowCopies)
            shallowCopy = true;
        }

        virtual StatusCode retrieveContainer (xAOD::TEvent& event, xAOD::TStore& store, ColumnVectorData& columnData) override
        {
          benchmarkFirstRetrieve.startTimer ();
          ATH_CHECK (event.retrieve (object, containerName));
          benchmarkFirstRetrieve.stopTimer ();
          benchmarkSecondRetrieve.startTimer ();
          ATH_CHECK (event.retrieve (object, containerName));
          benchmarkSecondRetrieve.stopTimer ();
          if (!object)
            throw std::logic_error ("no object retrieved for XAOD container (in retrieveContainer): " + containerName);
          data[1] = object->size();
          columnData.setColumn (index, data.size(), data.data());
          if (shallowCopy)
          {
            std::string shallowName = containerName + "_shallowCopy";
            std::string shallowAuxName = shallowName + "Aux.";
            benchmarkShallowCopy.startTimer ();
            auto shallowCopy = xAOD::shallowCopyContainer (*object);
            benchmarkShallowCopy.stopTimer ();
            benchmarkShallowRegister.startTimer ();
            ATH_CHECK (store.record (shallowCopy.first, shallowName));
            ATH_CHECK (store.record (shallowCopy.second, shallowAuxName));
            benchmarkShallowRegister.stopTimer ();
            object = shallowCopy.first;
          }
          return StatusCode::SUCCESS;
        }

        virtual std::pair<const SG::AuxElement*,std::size_t> getObject () const override
        {
          if (!object)
            throw std::logic_error ("no object retrieved for XAOD container: " + containerName);
          if (object->size() == 0)
            return {nullptr, 0};
          return {(*object)[0], object->size()};
        }

        [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
        {
          BranchPerfData result;
          result.name = containerName;
          result.timeRead = benchmarkFirstRetrieve.getEntryTime(emptyTime);
          result.timeReadAgain = benchmarkSecondRetrieve.getEntryTime(emptyTime);
          if (shallowCopy)
          {
            result.timeShallowCopy = benchmarkShallowCopy.getEntryTime(emptyTime);
            result.timeShallowRegister = benchmarkShallowRegister.getEntryTime(emptyTime);
          }
          benchmarkFirstRetrieve.setSilence();
          benchmarkSecondRetrieve.setSilence();
          benchmarkShallowCopy.setSilence();
          benchmarkShallowRegister.setSilence();
          return result;
        }
      };

      template<typename T>
      struct ColumnDataXAAccessor final : public IColumnReaderXA
      {
        std::string m_name;
        unsigned index = 0;
        std::optional<SG::AuxElement::Accessor<T>> accessor;
        std::optional<SG::AuxElement::Decorator<T>> decorator;
        bool isOptional = false;
        bool measureNonAccessForEmpty = false;
        const IColumnReaderXA *objectReader = nullptr;
        Benchmark benchmarkFirstRetrieve;
        Benchmark benchmarkSecondRetrieve;

        ColumnDataXAAccessor (const ColumnInfo& info, const UserConfiguration& userConfiguration)
          : m_name (info.name), index (info.index), measureNonAccessForEmpty (userConfiguration.measureNonAccessForEmpty)
        {
          auto name = info.name;
          if (!info.replacesColumn.empty())
            name = info.replacesColumn;
          name = name.substr (name.find_last_of('.') + 1);
          if (info.accessMode == ColumnAccessMode::input)
          {
            accessor.emplace (name);
            isOptional = info.isOptional;
          } else if (info.accessMode == ColumnAccessMode::output)
          {
            decorator.emplace (name);
          } else
          {
            throw std::runtime_error ("unsupported access mode for XAOD accessor: " + info.name);
          }
        }

        virtual void connect (ColumnDataXA& columnData, std::unordered_map<std::string,ColumnDataXA>& requestedColumns) override
        {
          auto iter = requestedColumns.find (columnData.info.offsetName);
          if (iter == requestedColumns.end())
            throw std::runtime_error ("missing offset column for XAOD accessor: " + columnData.info.offsetName);
          objectReader = iter->second.reader.get();
          if (decorator.has_value())
            iter->second.reader->requestShallowCopy();
        }

        virtual void retrieveAuxData (ColumnVectorData& columnData) override
        {
          if (!objectReader)
            throw std::logic_error ("no object reader for XAOD accessor");
          auto [object, size] = objectReader->getObject();
          if (object == nullptr)
          {
            if (measureNonAccessForEmpty)
            {
              benchmarkFirstRetrieve.startTimer ();
              benchmarkFirstRetrieve.stopTimer ();
              benchmarkSecondRetrieve.startTimer ();
              benchmarkSecondRetrieve.stopTimer ();
            }
            if (accessor.has_value())
              columnData.setColumn (index, 0, static_cast<const T*>(nullptr));
            else if (decorator.has_value())
              columnData.setColumn (index, 0, static_cast<T*>(nullptr));
            else
              throw std::logic_error ("no accessor or decorator for XAOD accessor");
          } else if (accessor.has_value())
          {
            if (!isOptional)
            {
              const T *value = nullptr;
              benchmarkFirstRetrieve.startTimer ();
              (*accessor) (*object);
              benchmarkFirstRetrieve.stopTimer ();
              benchmarkSecondRetrieve.startTimer ();
              value = &(*accessor) (*object);
              benchmarkSecondRetrieve.stopTimer ();
              columnData.setColumn (index, size, value);
            } else
            {
              const T *value = nullptr;
              benchmarkFirstRetrieve.startTimer ();
              if (accessor->isAvailable (*object))
                (*accessor) (*object);
              benchmarkFirstRetrieve.stopTimer ();
              benchmarkSecondRetrieve.startTimer ();
              if (accessor->isAvailable (*object))
                value = &(*accessor) (*object);
              benchmarkSecondRetrieve.stopTimer ();
              if (accessor->isAvailable (*object))
                columnData.setColumn (index, size, value);
            }
          } else if (decorator.has_value())
          {
            T *value = nullptr;
            benchmarkFirstRetrieve.startTimer ();
            (*decorator) (*object);
            benchmarkFirstRetrieve.stopTimer ();
            benchmarkSecondRetrieve.startTimer ();
            value = &(*decorator) (*object);
            benchmarkSecondRetrieve.stopTimer ();
            columnData.setColumn (index, size, value);
          } else
          {
            throw std::logic_error ("no accessor or decorator for XAOD accessor");
          }
        }

        [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
        {
          BranchPerfData result;
          result.name = m_name;
          result.timeRead = benchmarkFirstRetrieve.getEntryTime(emptyTime);
          result.timeReadAgain = benchmarkSecondRetrieve.getEntryTime(emptyTime);
          benchmarkFirstRetrieve.setSilence();
          benchmarkSecondRetrieve.setSilence();
          return result;
        }
      };
    }



    void runXaodArrayTest (const UserConfiguration& userConfiguration, const TestDefinition& testDefinition)
    {
      using namespace asg::msgUserCode;

      xAOD::TEvent event;
      xAOD::TStore store;
      ANA_CHECK_THROW (event.readFrom (testDefinition.file));

      auto *myTool = dynamic_cast<ColumnarTool<ColumnarModeXAODArray>*>(testDefinition.tool);
      if (!myTool)
        throw std::runtime_error ("tool is not a ColumnarTool<ColumnarModeXAODArray>");
      if (!testDefinition.containerRenames.empty())
        renameContainers (*myTool, testDefinition.containerRenames);
      ColumnVectorHeader columnHeader;
      ToolColumnVectorMap toolWrapper (columnHeader, *myTool);

      std::unordered_map<std::string,TestUtils::ColumnDataXA> requestedColumns;
      for (auto& column : myTool->getColumnInfo())
        requestedColumns[column.name].info = std::move (column);
      for (auto& [name, data] : requestedColumns)
      {
        if (data.info.isOffset)
        {
          if (*data.info.type != typeid(ColumnarOffsetType))
            throw std::runtime_error ("unexpected type for offset column: " + name + " " + data.info.type->name());
          if (name == numberOfEventsName)
          {
            data.reader = std::make_shared<TestUtils::ColumnDataXAEventInfo> (data.info);
          } else if (data.info.offsetName == numberOfEventsName)
          {
            if (name == "AnalysisMuons")
              data.reader = std::make_shared<TestUtils::ColumnDataXARetrieve<xAOD::MuonContainer>> (data.info, userConfiguration);
            else if (name == "AnalysisElectrons")
              data.reader = std::make_shared<TestUtils::ColumnDataXARetrieve<xAOD::ElectronContainer>> (data.info, userConfiguration);
            else if (name == "AnalysisPhotons")
              data.reader = std::make_shared<TestUtils::ColumnDataXARetrieve<xAOD::PhotonContainer>> (data.info, userConfiguration);
            else if (name == "egammaClusters")
              data.reader = std::make_shared<TestUtils::ColumnDataXARetrieve<xAOD::CaloClusterContainer>> (data.info, userConfiguration);
            else if (name == "GSFTrackParticles")
              data.reader = std::make_shared<TestUtils::ColumnDataXARetrieve<xAOD::TrackParticleContainer>> (data.info, userConfiguration);
            else if (name == "GSFConversionVertices")
              data.reader = std::make_shared<TestUtils::ColumnDataXARetrieve<xAOD::VertexContainer>> (data.info, userConfiguration);
          }
        } else
        {
          if (*data.info.type == typeid(float))
            data.reader = std::make_shared<TestUtils::ColumnDataXAAccessor<float>> (data.info, userConfiguration);
          else if (*data.info.type == typeid(double))
            data.reader = std::make_shared<TestUtils::ColumnDataXAAccessor<double>> (data.info, userConfiguration);
          else if (*data.info.type == typeid(char))
            data.reader = std::make_shared<TestUtils::ColumnDataXAAccessor<char>> (data.info, userConfiguration);
          else if (*data.info.type == typeid(std::uint8_t))
            data.reader = std::make_shared<TestUtils::ColumnDataXAAccessor<std::uint8_t>> (data.info, userConfiguration);
          else if (*data.info.type == typeid(std::uint16_t))
            data.reader = std::make_shared<TestUtils::ColumnDataXAAccessor<std::uint16_t>> (data.info, userConfiguration);
          else if (*data.info.type == typeid(std::uint32_t))
            data.reader = std::make_shared<TestUtils::ColumnDataXAAccessor<std::uint32_t>> (data.info, userConfiguration);
          else if (*data.info.type == typeid(std::uint64_t))
            data.reader = std::make_shared<TestUtils::ColumnDataXAAccessor<std::uint64_t>> (data.info, userConfiguration);
          else if (*data.info.type == typeid(std::vector<float>))
            data.reader = std::make_shared<TestUtils::ColumnDataXAAccessor<std::vector<float>>> (data.info, userConfiguration);
          else if (*data.info.type == typeid(std::vector<ElementLink<xAOD::CaloClusterContainer>>))
            data.reader = std::make_shared<TestUtils::ColumnDataXAAccessor<std::vector<ElementLink<xAOD::CaloClusterContainer>>>> (data.info, userConfiguration);
          else if (*data.info.type == typeid(std::vector<ElementLink<xAOD::TrackParticleContainer>>))
            data.reader = std::make_shared<TestUtils::ColumnDataXAAccessor<std::vector<ElementLink<xAOD::TrackParticleContainer>>>> (data.info, userConfiguration);
          else if (*data.info.type == typeid(std::vector<ElementLink<xAOD::VertexContainer>>))
            data.reader = std::make_shared<TestUtils::ColumnDataXAAccessor<std::vector<ElementLink<xAOD::VertexContainer>>>> (data.info, userConfiguration);
        }
      }

      bool allColumnsHandled = true;
      for (auto& [name, data] : requestedColumns)
      {
        if (!data.reader)
        {
          allColumnsHandled = false;
          std::cout << "WARNING: no handling for requested column: name=" << name << " offset=" << data.info.offsetName << " type=" << boost::core::demangle(data.info.type->name()) << std::endl;
        }
      }
      if (!allColumnsHandled)
      {
        ADD_FAILURE() << "not all requested columns could be handled";
        return;
      }

      for (auto& [name, data]: requestedColumns)
      {
        data.reader->connect (data, requestedColumns);
      }

      Benchmark benchmarkEmptyClear (testDefinition.name + " empty clear");
      Benchmark benchmarkCallClear (testDefinition.name + " call clear");
      Benchmark benchmarkGetEntry (testDefinition.name + " getEntry");
      Benchmark benchmarkCheck (testDefinition.name + " check");
      Benchmark benchmarkCall2 (testDefinition.name + " call2");
      Benchmark benchmarkCall (testDefinition.name + " call");
      Benchmark benchmarkEmpty ("empty");

      const auto numberOfEvents = event.getEntries();
      if (numberOfEvents == 0){
        throw std::runtime_error ("ColumnarPhysLiteTest: numberOfEvents == 0");
      }
      Long64_t entry = 0;

      // Instead of running for a fixed number of events, we run for a
      // fixed amount of time.  That is because individual tools can
      // vary wildly in how long they take to run, and we mostly want to
      // make sure that we ran the tool enough to get a precise
      // performance estimate.
      const auto startTime = std::chrono::high_resolution_clock::now();
      for (; (std::chrono::high_resolution_clock::now() - startTime) < userConfiguration.targetTime; ++entry)
      {
        benchmarkEmpty.startTimer ();
        benchmarkEmpty.stopTimer ();

        benchmarkGetEntry.startTimer ();
        event.getEntry (entry % numberOfEvents);
        benchmarkGetEntry.stopTimer ();

        ColumnVectorData columnData (&columnHeader);
        for (auto& [name, data] : requestedColumns)
          ASSERT_SUCCESS (data.reader->retrieveContainer (event, store, columnData));
        for (auto& [name, data] : requestedColumns)
          data.reader->retrieveAuxData (columnData);

        benchmarkCheck.startTimer ();
        columnData.checkData ();
        benchmarkCheck.stopTimer ();
        benchmarkCall.startTimer ();
        columnData.callNoCheck (*myTool);
        benchmarkCall.stopTimer ();
        if (userConfiguration.runToolTwice)
        {
          benchmarkCall2.startTimer ();
          columnData.callNoCheck (*myTool);
          benchmarkCall2.stopTimer ();
        }

        benchmarkCallClear.startTimer ();
        store.clear ();
        benchmarkCallClear.stopTimer ();
        benchmarkEmptyClear.startTimer ();
        store.clear ();
        benchmarkEmptyClear.stopTimer ();
      }
      std::cout << "Total entries read: " << entry << std::endl;
      const float emptyTime = benchmarkEmpty.getEntryTime(0).value();
      std::cout << "Empty benchmark time: " << emptyTime << "ns" << std::endl;
      benchmarkEmpty.setSilence();
      std::cout << "Average getEntry time: " << benchmarkGetEntry.getEntryTime(emptyTime).value() << "ns" << std::endl;
      benchmarkGetEntry.setSilence();
      std::cout << "Average clear time: " << benchmarkCallClear.getEntryTime(emptyTime).value() << "ns" << " (empty=" << benchmarkEmptyClear.getEntryTime(emptyTime).value() << "ns)" << std::endl;
      benchmarkCallClear.setSilence();
      benchmarkEmptyClear.setSilence();

      // Column performance table
      {
        std::vector<BranchPerfData> columnPerfData;
        BranchPerfData summary;
        summary.name = "total";
        summary.timeRead = 0;
        summary.timeReadAgain = 0;
        summary.timeShallowCopy = 0;
        summary.timeShallowRegister = 0;
        for (auto& [name, data] : requestedColumns)
        {
          auto perfData = data.reader->getPerfData(emptyTime);
          if (perfData.timeRead.has_value() || perfData.timeReadAgain.has_value())
          {
            columnPerfData.push_back(perfData);
            summary.timeRead.value() += perfData.timeRead.value_or(0);
            summary.timeReadAgain.value() += perfData.timeReadAgain.value_or(0);
            summary.timeShallowCopy.value() += perfData.timeShallowCopy.value_or(0);
            summary.timeShallowRegister.value() += perfData.timeShallowRegister.value_or(0);
          }
        }
        std::sort(columnPerfData.begin(), columnPerfData.end(), [](const auto& a, const auto& b) { return a.name < b.name; });
        columnPerfData.push_back(summary);

        const std::size_t nameWidth = std::max_element(columnPerfData.begin(), columnPerfData.end(), [](const auto& a, const auto& b) { return a.name.size() < b.name.size(); })->name.size();
        std::string header = std::format("{:{}} | 1st(ns) | 2nd(ns) | shallow copy(ns)", "column name", nameWidth);
        std::cout << "\n" << header << std::endl;
        std::cout << std::string(header.size(), '-') << std::endl;
        for (auto& data : columnPerfData)
        {
          if (data.name == "total")
            std::cout << std::string(header.size(), '-') << std::endl;
          std::cout << std::format("{:{}} |", data.name, nameWidth);
          if (data.timeRead)
            std::cout << std::format("{:>8.0f} |", data.timeRead.value());
          else
            std::cout << "         |";
          if (data.timeReadAgain)
            std::cout << std::format("{:>8.1f} |", data.timeReadAgain.value());
          else
            std::cout << "         |";
          if (data.timeShallowCopy || data.timeShallowRegister)
            std::cout << std::format("{:>10.0f} +{:>5.0f}", data.timeShallowCopy.value_or(-1), data.timeShallowRegister  .value_or(-1));
          std::cout << std::endl;
        }
      }

      // Tool performance table
      {
        std::vector<ToolPerfData> toolPerfData;
        toolPerfData.emplace_back();
        toolPerfData.back().name = testDefinition.name;
        toolPerfData.back().timeCall = benchmarkCall.getEntryTime(emptyTime);
        if (userConfiguration.runToolTwice)
          toolPerfData.back().timeCall2 = benchmarkCall2.getEntryTime(emptyTime);
        toolPerfData.back().timeCheck = benchmarkCheck.getEntryTime(emptyTime);
        benchmarkCall.setSilence();
        benchmarkCall2.setSilence();
        benchmarkCheck.setSilence();

        const std::size_t nameWidth = std::max_element(toolPerfData.begin(), toolPerfData.end(), [](const auto& a, const auto& b) { return a.name.size() < b.name.size(); })->name.size();
        std::string header = std::format("{:{}} | call(ns) | call2(ns) | check(ns)", "tool name", nameWidth);
        std::cout << "\n" << header << std::endl;
        std::cout << std::string(header.size(), '-') << std::endl;
        for (auto& data : toolPerfData)
        {
          std::cout << std::format("{:{}} |", data.name, nameWidth);
          if (data.timeCall)
            std::cout << std::format("{:>9.0f} |", data.timeCall.value());
          else
            std::cout << "          |";
          if (data.timeCall2)
            std::cout << std::format("{:>10.0f} |", data.timeCall2.value());
          else
            std::cout << "           |";
          if (data.timeCheck)
            std::cout << std::format("{:>10.1f}", data.timeCheck.value());
          std::cout << std::endl;
        }
      }
    }
  }
}
