/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TEST_FIXTURES_COLUMN_READER_BRANCH_H
#define COLUMNAR_TEST_FIXTURES_COLUMN_READER_BRANCH_H

#include <ColumnarTestFixtures/ColumnReader.h>

#include <AthLinks/ElementLink.h>
#include <ColumnarCore/ColumnarDef.h>
#include <ColumnarCore/ColumnInfoHelpers.h>
#include <ColumnarTestFixtures/Benchmark.h>
#include <ColumnarTestFixtures/PerformanceData.h>
#include <SGCore/sgkey_t.h>
#include <xAODBase/IParticleContainer.h>
#include <xAODCaloEvent/CaloClusterContainer.h>
#include <xAODJet/JetContainer.h>
#include <xAODTracking/TrackParticleContainer.h>
#include <xAODTracking/VertexContainer.h>

/// @file templated ColumnReader implementations for reading branches
/// from input files
///
/// These are shared between the TTree and RNTuple backends, taking a
/// template parameter for the actual BranchReader implementation.

namespace columnar
{
  namespace TestUtils
  {
    /// @brief the link columns for key+index links
    ///
    /// This just passes on the key and index columns as they are found
    /// in the file, with the link accessors handling everything else.
    class LinkColumnKeyIndex final
    {
      /// Public Members
      /// ==============
    public:

      using CM = ColumnarModeArray;

      [[nodiscard]] static std::string dataColumnName (const std::string& baseName)
      {
        return baseName + "." + std::string (CM::linkIndexColumnName);
      }

      [[nodiscard]] std::vector<std::string> connect (const ColumnInfo& columnInfo, const std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& /*offsetColumns*/, const std::unordered_map<std::string,ColumnInfo>& requestedColumns)
      {
        auto baseName = columnInfo.name.substr (0, columnInfo.name.size() - CM::linkIndexColumnName.size() - 1);
        auto keyName = baseName + "." + std::string (CM::linkKeyColumnName);
        if (!requestedColumns.contains (keyName))
          throw std::runtime_error ("link index column requested without key column: " + columnInfo.name);
        return {keyName};
      }

      void clear ()
      {
        m_indices.clear();
        m_keys.clear();
      }

      template<typename T>
      void addLink (const ElementLink<T>& element)
      {
        m_keys.push_back (element.key());
        m_indices.push_back (element.persIndex());
      }

      void addEmptyLink ()
      {
        m_keys.push_back (CM::nullLinkKey);
        m_indices.push_back (CM::nullLinkIndex);
      }

      void addSplitLink (std::size_t linkIndex, SG::sgkey_t linkKey)
      {
        m_keys.push_back (linkKey);
        m_indices.push_back (linkIndex);
      }

      [[nodiscard]] std::size_t size () const noexcept
      {
        return m_indices.size();
      }

      [[nodiscard]] const typename CM::LinkIndexColumnType* data () const noexcept
      {
        return m_indices.data();
      }

      [[nodiscard]] unsigned countNullLinks () const noexcept
      {
        unsigned result = 0u;
        for (std::size_t i = 0; i < m_indices.size(); ++ i)
        {
          if (m_keys[i] == CM::nullLinkKey || m_indices[i] == CM::nullLinkIndex)
            result += 1u;
        }
        return result;
      }

      [[nodiscard]] const std::vector<typename CM::LinkKeyColumnType>& keysColumn () const noexcept
      {
        return m_keys;
      }

      /// Private Members
      /// ===============
    private:

      std::vector<typename CM::LinkIndexColumnType> m_indices;
      std::vector<typename CM::LinkKeyColumnType> m_keys;
    };



    template <typename T, template <typename> class Reader>
    struct ColumnReaderScalar final : public TestUtils::ColumnReader
    {
      Reader<T> branchReader;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      std::vector<T> outData;
      unsigned entries = 0;

      explicit ColumnReaderScalar (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName()});
      }

      virtual bool connect( Backend source, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& /*offsetColumns*/,std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;
        requestedColumns.erase (iter);

        branchReader.connectTree (source);

        return true;
      }
      virtual void clearColumns () override
      {
        outData.clear ();
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        outData.push_back (branchData);
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnVectorData& columnData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          columnData.setColumn (outputColumns.at(0).columnIndex, outData.size(), outData.data());
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
      {
        BranchPerfData result;
        result.name = branchReader.columnName();
        result.timeRead = benchmark.getEntryTime(emptyTime);
        result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
        benchmark.setSilence();
        benchmarkUnpack.setSilence();
        result.entrySize = branchReader.entrySize();
        result.uncompressedSize = branchReader.uncompressedSize();
        result.numBaskets = branchReader.numBaskets();
        result.entries = entries;
        return result;
      }

      virtual void collectColumnData () override
      {
        entries += outData.size();
      }
    };

    template <typename T, template <typename> class Reader>
    struct ColumnReaderVector final : public TestUtils::ColumnReader 
    {
      Reader<std::vector<T>> branchReader;
      const std::vector<ColumnarOffsetType>* offsetColumn = nullptr;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<T> outData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;

      explicit ColumnReaderVector (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName()});
        outputColumns.push_back ({.name = branchReader.containerName(), .isOffset = true, .primary = false});
      }

      virtual bool connect(Backend source, std::unordered_map<std::string, const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree(source);

        if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);

        requestedColumns.erase (iter);

        if (auto offsetIter = offsetColumns.find (outputColumns.at(1).name); offsetIter != offsetColumns.end())
          offsetColumn = offsetIter->second;
        else
          offsetColumns.emplace (outputColumns.at(1).name, &offsets);

        iter = requestedColumns.find (outputColumns.at(1).name);
        if (iter != requestedColumns.end())
        {
          requestedColumns.erase (iter);
          outputColumns.at(1).enabled = true;
        }

        return true;
      }

      virtual void clearColumns () override
      {
        offsets.clear ();
        offsets.push_back (0);
        outData.clear ();
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        outData.insert (outData.end(), branchData.begin(), branchData.end());
        offsets.push_back (outData.size());
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnVectorData& columnData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          columnData.setColumn (outputColumns.at(0).columnIndex, outData.size(), outData.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex)
          columnData.setColumn (outputColumns.at(1).columnIndex, offsets.size(), offsets.data());
        if (offsetColumn)
        {
          if (offsetColumn->size() != offsets.size())
            throw std::runtime_error ("offset column not filled yet: " + outputColumns.at(1).name);
          if (offsetColumn->back() != offsets.back())
            throw std::runtime_error ("offset column does not match: " + outputColumns.at(1).name);
        }
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
      {
        BranchPerfData result;
        result.name = branchReader.columnName();
        result.timeRead = benchmark.getEntryTime(emptyTime);
        result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
        benchmark.setSilence();
        benchmarkUnpack.setSilence();
        result.entrySize = branchReader.entrySize();
        result.uncompressedSize = branchReader.uncompressedSize();
        result.numBaskets = branchReader.numBaskets();
        result.entries = entries;
        return result;
      }

      virtual void collectColumnData () override
      {
        entries += outData.size();
      }
    };

    template <typename T, template <typename> class Reader>
    struct ColumnReaderVectorVector final : public TestUtils::ColumnReader 
    {
      Reader<std::vector<std::vector<T>>> branchReader;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<T> columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;

      explicit ColumnReaderVectorVector (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName() + ".data"});
        outputColumns.push_back ({.name = branchReader.columnName() + ".offset", .isOffset = true});
      }

      virtual bool connect(Backend source, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree(source);

        if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);

        requestedColumns.erase (iter);

        // register the per-object offsets, so that other readers (e.g.
        // for the event-index column) can use them
        offsetColumns.emplace (outputColumns.at(1).name, &offsets);

        iter = requestedColumns.find (outputColumns.at(1).name);
        if (iter == requestedColumns.end())
          return true;
        requestedColumns.erase (iter);
        outputColumns.at(1).enabled = true;
        return true;
      }

      virtual void clearColumns () override
      {
        columnData.clear();
        offsets.clear();
        offsets.push_back (0);
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        for (auto& data : branchData)
        {
          columnData.insert (columnData.end(), data.begin(), data.end());
          offsets.push_back (columnData.size());
        }
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnVectorData& colData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(0).columnIndex, columnData.size(), columnData.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(1).columnIndex, offsets.size(), offsets.data());
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
      {
        BranchPerfData result;
        result.name = branchReader.columnName();
        result.timeRead = benchmark.getEntryTime(emptyTime);
        result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
        benchmark.setSilence();
        benchmarkUnpack.setSilence();
        result.entrySize = branchReader.entrySize();
        result.uncompressedSize = branchReader.uncompressedSize();
        result.numBaskets = branchReader.numBaskets();
        result.entries = entries;
        return result;
      }

      virtual void collectColumnData () override
      {
        entries += columnData.size();
      }
    };

    template <typename T, template <typename> class Reader>
    struct ColumnReaderVectorVectorLink final : public TestUtils::ColumnReader 
    {
      Reader<std::vector<std::vector<ElementLink<T>>>> branchReader;
      std::vector<ColumnarOffsetType> offsets = {0};
      LinkColumnKeyIndex columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;
      unsigned nullEntries = 0;

      explicit ColumnReaderVectorVectorLink (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = LinkColumnKeyIndex::dataColumnName (branchReader.columnName())});
        outputColumns.push_back ({.name = branchReader.columnName() + ".offset", .isOffset = true});
      }

     virtual bool connect(Backend source, std::unordered_map<std::string, const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
     {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree(source);

        if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);
        for (const auto & keyColumn : columnData.connect (iter->second, offsetColumns, requestedColumns))
        {
          outputColumns.push_back ({.name = keyColumn, .primary = false, .enabled = true});
          requestedColumns.erase (keyColumn);
        }

        requestedColumns.erase (iter);

        // register the per-object offsets, so that other readers (e.g.
        // for the event-index column) can use them
        offsetColumns.emplace (outputColumns.at(1).name, &offsets);

        iter = requestedColumns.find (outputColumns.at(1).name);
        if (iter == requestedColumns.end())
          return true;
        requestedColumns.erase (iter);
        outputColumns.at(1).enabled = true;
        return true;
      }

      virtual void clearColumns () override
      {
        columnData.clear();
        offsets.clear();
        offsets.push_back (0);
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        for (auto& data : branchData)
        {
          for (auto& element : data)
          {
            columnData.addLink (element);
          }
          offsets.push_back (columnData.size());
        }
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnVectorData& colData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(0).columnIndex, columnData.size(), columnData.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(1).columnIndex, offsets.size(), offsets.data());
        for (std::size_t i = 2; i < outputColumns.size(); ++ i)
        {
          if (outputColumns.at(i).columnIndex != ColumnVectorHeader::nullIndex)
            colData.setColumn (outputColumns.at(i).columnIndex, columnData.keysColumn().size(), columnData.keysColumn().data());
        }
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
      {
        BranchPerfData result;
        result.name = branchReader.columnName();
        result.timeRead = benchmark.getEntryTime(emptyTime);
        result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
        benchmark.setSilence();
        benchmarkUnpack.setSilence();
        result.entrySize = branchReader.entrySize();
        result.uncompressedSize = branchReader.uncompressedSize();
        result.numBaskets = branchReader.numBaskets();
        result.entries = entries;
        result.nullEntries = nullEntries;
        return result;
      }

      virtual void collectColumnData () override
      {
        entries += columnData.size();
        nullEntries += columnData.countNullLinks();
      }
    };

    template <typename T, template <typename> class Reader>
    struct ColumnReaderVectorVectorVector final : public TestUtils::ColumnReader 
    {
      std::string columnName;
      Reader<std::vector<std::vector<std::vector<T>>>> branchReader;
      std::vector<ColumnarOffsetType> outerOffsets = {0};
      std::vector<ColumnarOffsetType> innerOffsets = {0};
      std::vector<T> columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;

      explicit ColumnReaderVectorVectorVector (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName() + ".data"});
        outputColumns.push_back ({.name = branchReader.columnName() + ".innerOffset", .isOffset = true});
        outputColumns.push_back ({.name = branchReader.columnName() + ".outerOffset", .isOffset = true});
      }

      virtual bool connect(Backend source, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& /*offsetColumns*/, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree(source);

        if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);

        requestedColumns.erase (iter);

        iter = requestedColumns.find (outputColumns.at(1).name);
        if (iter == requestedColumns.end())
          return true;
        outputColumns.at(1).enabled = true;

        if (iter->second.offsetName != outputColumns.at(2).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(2).name);

        requestedColumns.erase (iter);

        iter = requestedColumns.find (outputColumns.at(2).name);
        if (iter == requestedColumns.end())
          return true;
        outputColumns.at(2).enabled = true;
        requestedColumns.erase (iter);
        return true;
      }

      virtual void clearColumns () override
      {
        columnData.clear();
        innerOffsets.clear();
        innerOffsets.push_back (0);
        outerOffsets.clear();
        outerOffsets.push_back (0);
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        for (auto& outerData : branchData)
        {
          for (auto& innerData : outerData)
          {
            columnData.insert (columnData.end(), innerData.begin(), innerData.end());
            innerOffsets.push_back (columnData.size());
          }
          outerOffsets.push_back (innerOffsets.size()-1);
        }
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnVectorData& colData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(0).columnIndex, columnData.size(), columnData.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(1).columnIndex, innerOffsets.size(), innerOffsets.data());
        if (outputColumns.at(2).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(2).columnIndex, outerOffsets.size(), outerOffsets.data());
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
      {
        BranchPerfData result;
        result.name = branchReader.columnName();
        result.timeRead = benchmark.getEntryTime(emptyTime);
        result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
        benchmark.setSilence();
        benchmarkUnpack.setSilence();
        result.entrySize = branchReader.entrySize();
        result.uncompressedSize = branchReader.uncompressedSize();
        result.numBaskets = branchReader.numBaskets();
        result.entries = entries;
        return result;
      }

      virtual void collectColumnData () override
      {
        entries += columnData.size();
      }
    };

    template <typename T, template <typename> class Reader>
    struct ColumnReaderVectorLink final : public TestUtils::ColumnReader 
    {
      Reader<std::vector<ElementLink<T>>> branchReader;
      const std::vector<ColumnarOffsetType>* offsetColumn = nullptr;
      std::vector<ColumnarOffsetType> offsets = {0};
      LinkColumnKeyIndex columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;
      unsigned nullEntries = 0;

      ColumnReaderVectorLink (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = LinkColumnKeyIndex::dataColumnName (branchReader.columnName())});
        outputColumns.push_back ({.name = branchReader.containerName(), .isOffset = true, .primary = false});
      }


      virtual bool connect(Backend source, std::unordered_map<std::string, const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree(source);

        if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);
        for (const auto & keyColumn : columnData.connect (iter->second, offsetColumns, requestedColumns))
        {
          outputColumns.push_back ({.name = keyColumn, .primary = false, .enabled = true});
          requestedColumns.erase (keyColumn);
        }

        requestedColumns.erase (iter);

        if (auto offsetIter = offsetColumns.find (outputColumns.at(1).name); offsetIter != offsetColumns.end())
          offsetColumn = offsetIter->second;
        else
          offsetColumns.emplace (outputColumns.at(1).name, &offsets);

        iter = requestedColumns.find (outputColumns.at(1).name);
        if (iter != requestedColumns.end())
        {
          outputColumns.at(1).enabled = true;
          requestedColumns.erase (iter);
        }

        return true;
      }

      virtual void clearColumns () override
      {
        columnData.clear();
        offsets.clear();
        offsets.push_back (0);
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        for (auto& element : branchData)
          columnData.addLink (element);
        offsets.push_back (columnData.size());
        if (offsetColumn)
        {
          if (offsetColumn->size() != offsets.size())
            throw std::runtime_error ("offset column not filled yet: " + outputColumns.at(1).name);
          if (offsetColumn->back() != offsets.back())
            throw std::runtime_error ("offset column does not match: " + outputColumns.at(1).name);
        }
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnVectorData& colData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(0).columnIndex, columnData.size(), columnData.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(1).columnIndex, offsets.size(), offsets.data());
        for (std::size_t i = 2; i < outputColumns.size(); ++ i)
        {
          if (outputColumns.at(i).columnIndex != ColumnVectorHeader::nullIndex)
            colData.setColumn (outputColumns.at(i).columnIndex, columnData.keysColumn().size(), columnData.keysColumn().data());
        }
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
      {
        BranchPerfData result;
        result.name = branchReader.columnName();
        result.timeRead = benchmark.getEntryTime(emptyTime);
        result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
        benchmark.setSilence();
        benchmarkUnpack.setSilence();
        result.entrySize = branchReader.entrySize();
        result.uncompressedSize = branchReader.uncompressedSize();
        result.numBaskets = branchReader.numBaskets();
        result.entries = entries;
        result.nullEntries = nullEntries;
        return result;
      }

      virtual void collectColumnData () override
      {
        entries += columnData.size();
        nullEntries += columnData.countNullLinks();
      }
    };

    template <template <typename> class Reader>
    struct ColumnReaderMetNames final : public TestUtils::ColumnReader 
    {
      Reader<std::vector<std::string>> branchReader;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<char> columnData;
      std::vector<std::size_t> columnHashData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;

      ColumnReaderMetNames (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName() + ".data"});
        outputColumns.push_back ({.name = branchReader.columnName() + ".offset", .isOffset = true});
        outputColumns.push_back ({.name = branchReader.columnName() + "Hash"});
      }

      virtual bool connect(Backend source, std::unordered_map<std::string, const std::vector<ColumnarOffsetType>*>& /*offsetColumns*/, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree(source);

        if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);

        requestedColumns.erase (iter);

        iter = requestedColumns.find (outputColumns.at(1).name);
        if (iter == requestedColumns.end())
        {
          return true;
        }
        outputColumns.at(1).enabled = true;
        requestedColumns.erase (iter);

        iter = requestedColumns.find (outputColumns.at(2).name);
        if (iter != requestedColumns.end())
        {
          outputColumns.at(2).enabled = true;
          requestedColumns.erase (iter);
        }
        return true;
      }

      virtual void clearColumns () override
      {
        columnData.clear();
        offsets.clear();
        offsets.push_back (0);
        columnHashData.clear();
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        for (auto& data : branchData)
        {
          columnData.insert (columnData.end(), data.begin(), data.end());
          offsets.push_back (columnData.size());
          columnHashData.push_back (std::hash<std::string> () (data));
        }
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnVectorData& colData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(0).columnIndex, columnData.size(), columnData.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(1).columnIndex, offsets.size(), offsets.data());
        if (outputColumns.at(2).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(2).columnIndex, columnHashData.size(), columnHashData.data());
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
      {
        BranchPerfData result;
        result.name = branchReader.columnName();
        result.timeRead = benchmark.getEntryTime(emptyTime);
        result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
        benchmark.setSilence();
        benchmarkUnpack.setSilence();
        result.entrySize = branchReader.entrySize();
        result.uncompressedSize = branchReader.uncompressedSize();
        result.numBaskets = branchReader.numBaskets();
        return result;
      }

      virtual void collectColumnData () override
      {}
    };

    template <template <typename> class Reader>
    struct ColumnReaderSamplingPattern final : public TestUtils::ColumnReader 
    {
      Reader<xAOD::CaloClusterContainer> branchReader;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<std::uint32_t> columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;

      ColumnReaderSamplingPattern (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+".samplingPattern(fallback)(unpack)"), benchmark (branchReader.columnName() + ".samplingPattern(fallback)")
      {
        outputColumns.push_back ({.name = branchReader.columnName() + ".samplingPattern"});
        outputColumns.push_back ({.name = branchReader.columnName(), .isOffset = true, .primary = false});
      }

      virtual bool connect(Backend source, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& /*offsetColumns*/, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
       auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree(source);
       if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);

        requestedColumns.erase (iter);

        iter = requestedColumns.find (outputColumns.at(1).name);
        if (iter == requestedColumns.end())
        {
          return true;
        }
        outputColumns.at(1).enabled = true;
        requestedColumns.erase (iter);
        return true;
      }

      virtual void clearColumns () override
      {
        columnData.clear();
        offsets.clear();
        offsets.push_back (0);
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        for (auto data : branchData)
        {
          columnData.push_back (data->samplingPattern());
        }
        offsets.push_back (columnData.size());
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnVectorData& colData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(0).columnIndex, columnData.size(), columnData.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(1).columnIndex, offsets.size(), offsets.data());
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
      {
        BranchPerfData result;
        result.name = branchReader.columnName() + "(fallback)";
        result.timeRead = benchmark.getEntryTime(emptyTime);
        result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
        benchmark.setSilence();
        benchmarkUnpack.setSilence();
        result.entrySize = branchReader.entrySize();
        result.uncompressedSize = branchReader.uncompressedSize();
        result.numBaskets = branchReader.numBaskets();
        result.entries = entries;
        return result;
      }

      virtual void collectColumnData () override
      {
        entries += columnData.size();
      }
    };


    /// @brief add the reader for a key+index link branch of the given
    /// type, if there is one
    ///
    /// The readers need to name the type of the element link to read
    /// the branch, but the type itself doesn't matter, as we only look
    /// at the key and index.  So any type with a dictionary available
    /// can be added here.
    template <template <typename> class Reader>
    bool addKeyIndexLinkReader (std::vector<std::shared_ptr<TestUtils::ColumnReader>>& knownColumns, const std::string& name, const std::string& typeName)
    {
      if (typeName.find ("ElementLink<") == std::string::npos)
        return false;
      const bool isVectorVector = typeName.find ("vector<vector<") != std::string::npos || typeName.find ("vector<std::vector<") != std::string::npos;
      const auto addReader = [&]<typename T> (const char* xaodType, std::type_identity<T>)
      {
        if (typeName.find (xaodType) == std::string::npos)
          return false;
        if (isVectorVector)
          knownColumns.push_back (std::make_shared<ColumnReaderVectorVectorLink<T,Reader>> (name));
        else
          knownColumns.push_back (std::make_shared<ColumnReaderVectorLink<T,Reader>> (name));
        return true;
      };
      return addReader ("xAOD::TrackParticle_v", std::type_identity<xAOD::TrackParticleContainer> ()) ||
        addReader ("xAOD::CaloCluster_v", std::type_identity<xAOD::CaloClusterContainer> ()) ||
        addReader ("xAOD::Vertex_v", std::type_identity<xAOD::VertexContainer> ()) ||
        addReader ("xAOD::Jet_v", std::type_identity<xAOD::JetContainer> ()) ||
        addReader ("xAOD::IParticle", std::type_identity<xAOD::IParticleContainer> ());
    }
  }
}

#endif // COLUMNAR_TEST_FIXTURES_COLUMN_READER_BRANCH_H