/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarTestFixtures/ColumnReader.h>

#include <AsgMessaging/MessageCheck.h>
#include <ColumnarTestFixtures/PerformanceData.h>
#include <TruthUtils/ParticleConstants.h>
#include <xAODBase/ObjectType.h>
#include <xAODMissingET/versions/MissingETBase.h>

//
// method implementations
//

namespace columnar 
{
  namespace TestUtils 
  {
    struct ColumnReaderEventCount final : public TestUtils::ColumnReader
    {
      std::array<ColumnarOffsetType, 2> data = {0, 0};

      ColumnReaderEventCount ()
      {
        outputColumns.push_back ({.name = eventRangeColumnName, .isOffset = true});
      }

      virtual bool connect(Backend /*source*/, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& /*offsetColumns*/,std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
       if (requestedColumns.contains(outputColumns.at(0).name)) 
       {
         requestedColumns.erase(outputColumns.at(0).name);
         outputColumns.at(0).enabled = true;
         return true;
       }
       return false;
      }
      virtual void clearColumns () override
      {
        data[0] = 0;
        data[1] = 0;
      }

      virtual void getEntry (Long64_t /*entry*/) override
      {
        data[1] += 1;
      }

      virtual void setData (ColumnVectorData& columnData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          columnData.setColumn (outputColumns.at(0).columnIndex, data.size(), data.data());
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float /*emptyTime*/) override
      {
        BranchPerfData result;
        result.name = "EventCount(auto)";
        return result;
      }

      virtual void collectColumnData () override
      {}
    };

    template <typename T>
    struct ColumnReaderOutVector final : public TestUtils::ColumnReader 
    {
      T defaultValue;
      const std::vector<ColumnarOffsetType>* offsetColumn = nullptr;
      std::vector<T> outData;
      unsigned entries = 0;

      ColumnReaderOutVector (const std::string& val_columnName, const T& val_defaultValue)
        : defaultValue (val_defaultValue)
      {
        outputColumns.push_back ({.name = val_columnName});
      }

      virtual bool connect(Backend /*source*/, std::unordered_map<std::string, const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        const auto & offsetName = iter->second.offsetName;
        if (offsetName.empty())
          throw std::runtime_error ("missing offset column for: " + outputColumns.at(0).name);

        if (auto offsetIter = offsetColumns.find (offsetName); offsetIter != offsetColumns.end())
          offsetColumn = offsetIter->second;
        else
          throw std::runtime_error ("missing offset column for: " + outputColumns.at(0).name + " (offset column: " + offsetName + ")");

        // IMPORTANT: This has to be erased at the end of the function,
        // since erasing it earlier would invalidate the offsetName
        // reference.
        requestedColumns.erase (iter);
        return true;
      }

      virtual void clearColumns () override
      {
        outData.clear ();
      }

      virtual void getEntry (Long64_t /*entry*/) override
      {
        outData.resize (offsetColumn->back(), defaultValue);
      }

      virtual void setData (ColumnVectorData& columnData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          columnData.setColumn (outputColumns.at(0).columnIndex, outData.size(), outData.data());
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float /*emptyTime*/) override
      {
        BranchPerfData result;
        result.name = outputColumns.at(0).name + "(out)";
        result.entries = entries;
        return result;
      }

      virtual void collectColumnData () override
      {
        entries += outData.size();
      }
    };

    struct ColumnReaderOutputMet final : public TestUtils::ColumnReader 
    {
     std::vector<std::string> termNames;
      const std::vector<ColumnarOffsetType>* offsetColumns = nullptr;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<ColumnarOffsetType> namesOffsets = {0};
      std::vector<char> namesData;
      std::vector<std::size_t> namesHash;

      ColumnReaderOutputMet (const std::string& val_columnName, std::vector<std::string> val_termNames)
        : termNames (std::move (val_termNames))
      {
        outputColumns.push_back ({.name = val_columnName, .isOffset = true});
        outputColumns.push_back ({.name = val_columnName + ".name.data"});
        outputColumns.push_back ({.name = val_columnName + ".name.offset", .isOffset = true});
        outputColumns.push_back ({.name = val_columnName + ".nameHash"});
      }

      virtual bool connect([[maybe_unused]]Backend source, std::unordered_map<std::string, const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
       if (auto iter = requestedColumns.find (outputColumns.at(0).name);
            iter != requestedColumns.end())
          requestedColumns.erase (iter);
        else
          return false;
        outputColumns.at(0).enabled = true;

        if (auto iter = requestedColumns.find (outputColumns.at(1).name);
            iter != requestedColumns.end())
        {
          outputColumns.at(1).enabled = true;
          requestedColumns.erase (iter);
        }

        if (auto iter = requestedColumns.find (outputColumns.at(2).name);
            iter != requestedColumns.end())
        {
          outputColumns.at(2).enabled = true;
          requestedColumns.erase (iter);
        }

        if (auto iter = requestedColumns.find (outputColumns.at(3).name);
            iter != requestedColumns.end())
        {
          outputColumns.at(3).enabled = true;
          requestedColumns.erase (iter);
        }

        // For multi-tool support, skip if offset column already registered
        if (auto offsetIter = offsetColumns.find (outputColumns.at(0).name); offsetIter == offsetColumns.end())
          offsetColumns.emplace (outputColumns.at(0).name, &offsets);

        return true;
      }

      virtual void clearColumns () override
      {
        offsets.clear ();
        offsets.push_back (0);
        namesData.clear ();
        namesOffsets.clear ();
        namesOffsets.push_back (0);
        namesHash.clear ();
      }

      virtual void getEntry (Long64_t /*entry*/) override
      {
        for (const auto& termName : termNames)
        {
          namesData.insert (namesData.end(), termName.begin(), termName.end());
          namesOffsets.push_back (namesData.size());
          namesHash.push_back (std::hash<std::string> () (termName));
        }
        offsets.push_back (namesHash.size());
      }

      virtual void setData (ColumnVectorData& colData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(0).columnIndex, offsets.size(), offsets.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(1).columnIndex, namesData.size(), namesData.data());
        if (outputColumns.at(2).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(2).columnIndex, namesOffsets.size(), namesOffsets.data());
        if (outputColumns.at(3).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(3).columnIndex, namesHash.size(), namesHash.data());
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float /*emptyTime*/) override
      {
        BranchPerfData result;
        result.name = outputColumns.at(0).name + "(met-out)";
        return result;
      }

      virtual void collectColumnData () override
      {}
    };



    void makeCommonColumnReaders (std::vector<std::shared_ptr<TestUtils::ColumnReader>>& knownColumns)
    {
      knownColumns.push_back (std::make_shared<ColumnReaderEventCount> ());
    }

    void makeCommonLateColumnReaders (std::vector<std::shared_ptr<TestUtils::ColumnReader>>& knownColumns, const std::vector<std::string>& allMetTermNames)
    {
       if (!allMetTermNames.empty())
         knownColumns.push_back(std::make_shared<ColumnReaderOutputMet>("OutputMET",allMetTermNames));

      // For METMaker we need various extra columns to run. This may need
      // some work to avoid, but would likey be worth it.
      knownColumns.push_back(std::make_shared<ColumnReaderOutVector<std::uint16_t>>("AnalysisMuons.objectType", xAOD::Type::Muon));
      knownColumns.push_back(std::make_shared<ColumnReaderOutVector<float>>("AnalysisMuons.m", ParticleConstants::muonMassInMeV));
      knownColumns.push_back(std::make_shared<ColumnReaderOutVector<std::uint16_t>>("AnalysisJets.objectType", xAOD::Type::Jet));

      // These are columns that represent variables that are normally held
      // by METAssociationHelper, or alternatively are decorated on the
      // MET terms (even though they are per object).
      knownColumns.push_back(std::make_shared<ColumnReaderOutVector<float>>("AnalysisMuons.MetObjectWeight", 0));
      knownColumns.push_back(std::make_shared<ColumnReaderOutVector<float>>("AnalysisJets.MetObjectWeight", 0));
      knownColumns.push_back(std::make_shared<ColumnReaderOutVector<float>>("AnalysisJets.MetObjectWeightSoft", 0));
      knownColumns.push_back(std::make_shared<ColumnReaderOutVector<MissingETBase::Types::bitmask_t>>("METAssoc_AnalysisMET.useObjectFlags", 0));
    }

    void makeOutputColumnReaders (std::vector<std::shared_ptr<TestUtils::ColumnReader>>& knownColumns, const ColumnVectorHeader& columnHeader)
    {
      using namespace asg::msgUserCode;

      // Get all column info directly from the header (all tools have already
      // registered their columns via ToolColumnVectorMap)
      auto requestedColumns = columnHeader.getAllColumnInfo();

      for (auto& [name, info] : requestedColumns)
      {
        if (info.accessMode != ColumnAccessMode::output || !info.fixedDimensions.empty())
          continue;
        auto offsetIter = requestedColumns.find(info.offsetName);
        if (offsetIter == requestedColumns.end() || offsetIter->second.offsetName != eventRangeColumnName)
        {
          ANA_MSG_WARNING ("skipping column {} as it is not a scalar decoration on a global container", name);
          continue;
        }
        std::shared_ptr<TestUtils::ColumnReader> myColumn;
        if (*info.type == typeid(float))
          myColumn = std::make_shared<TestUtils::ColumnReaderOutVector<float>>(info.name, 0);
        else if (*info.type == typeid(char))
          myColumn = std::make_shared<TestUtils::ColumnReaderOutVector<char>>(info.name, 0);
        else if (*info.type == typeid(std::uint16_t))
          myColumn = std::make_shared<TestUtils::ColumnReaderOutVector<std::uint16_t>>(info.name, 0);
        else if (*info.type == typeid(std::uint64_t))
          myColumn = std::make_shared<TestUtils::ColumnReaderOutVector<std::uint64_t>>(info.name, 0);
        else 
        {
          ANA_MSG_WARNING("unhandled column type: " << info.name << " "<< info.type->name());
          continue;
        }
        knownColumns.push_back(myColumn);
      }
    }
  }
}
