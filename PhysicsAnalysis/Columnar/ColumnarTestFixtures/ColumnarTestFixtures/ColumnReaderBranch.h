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
#include <ColumnarInterfaces/KnownSgKeys.h>
#include <ColumnarTestFixtures/Benchmark.h>
#include <ColumnarTestFixtures/PerformanceData.h>
#include <SGCore/sgkey_t.h>
#include <xAODCaloEvent/CaloClusterContainer.h>

/// @file templated ColumnReader implementations for reading branches
/// from input files
///
/// These are shared between the TTree and RNTuple backends, taking a
/// template parameter for the actual BranchReader implementation.

namespace columnar
{
  namespace TestUtils
  {
    class LinkColumnVector final
    {
      /// Public Members
      /// ==============
    public:

      using CM = ColumnarModeArray;

      ~LinkColumnVector () noexcept
      {
        if (!m_unknownKeysAllowedTargets.empty())
        {
          std::cout << "found unknown keys for " << m_columnName << ":";
          for (auto& [key, allowedSet] : m_unknownKeysAllowedTargets)
          {
            std::cout << "  " << std::hex << key << std::dec << " (allowed targets:";
            for (auto index : allowedSet)
              std::cout << " " << m_targetNames.at(index);
            std::cout << ")";
          }
        }
      }

      [[nodiscard]] std::vector<std::string> connect (const ColumnInfo& columnInfo, const std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& offsetColumns, const std::unordered_map<std::string,ColumnInfo>& requestedColumns)
      {
        m_columnName = columnInfo.name;
        std::vector<std::string> keyColumnNames;
        if (!columnInfo.soleLinkTargetName.empty())
        {
          addTarget (columnInfo.soleLinkTargetName, offsetColumns, columnInfo.soleLinkTargetClid);
        } else
        {
          for (auto& [requestedName, requestedInfo] : requestedColumns)
          {
            if (requestedInfo.keyColumnForVariantLink == m_columnName)
            {
              keyColumnNames.push_back (requestedName);
              m_keysColumns.emplace_back();
              for (const auto& targetName : requestedInfo.variantLinkTargetNames)
                addTarget (targetName, offsetColumns);
            }
          }
          if (m_keysColumns.empty())
            throw std::runtime_error ("no key column found for variant link: " + m_columnName);
        }
        return keyColumnNames;
      }

      void clear ()
      {
        m_columnData.clear();
      }

      void checkOffsets (unsigned eventIndex)
      {
        for (std::size_t i = 0; i < m_targetNames.size(); ++ i)
        {
          auto& targetOffsetColumn = *m_targetOffsetColumns.at(i);
          if (eventIndex + 1 >= targetOffsetColumn.size())
            throw std::runtime_error ("target offset column not yet filled for: " + m_targetNames.at(i) + " when checking link column " + m_columnName);
        }
      }

      template<typename T>
      void addLink (const ElementLink<T>& element, unsigned eventIndex)
      {
        if (element.isDefault())
        {
          addEmptyLink();
          return;
        }

        addSplitLink (element.index(), element.key(), eventIndex);
      }

      void addEmptyLink ()
      {
        m_columnData.push_back (invalidObjectIndex);
      }

      void addSplitLink (std::size_t linkIndex, SG::sgkey_t linkKey, unsigned eventIndex)
      {
        if (linkIndex == 0 && linkKey == 0)
        {
          addEmptyLink();
          return;
        }

        unsigned targetIndex = 0u;
        while (targetIndex < m_targetKeys.size() && m_targetKeys.at(targetIndex) != linkKey)
          ++ targetIndex;

        // We didn't find the key, so we try to figure out which of the
        // targets it could be. The idea is that you wouldn't rely on
        // this for real tests, but that you then go and fill in those
        // keys in the central lookup table. It will always record and
        // report, that means if there is a variant link with extra
        // targets you didn't declare you will get a diagnostic. This
        // may be overly cautious, but it gives an extra diagnostic if
        // maybe you missed a target.
        if (targetIndex == m_targetKeys.size())
        {
          if (!m_unknownKeysAllowedTargets.contains (linkKey))
          {
            auto& allowedSet = m_unknownKeysAllowedTargets[linkKey];
            for (std::size_t i = 0; i < m_targetKeys.size(); ++ i)
            {
              if (m_targetKeys.at(i) == 0)
                allowedSet.insert(i);
            }
          }
          auto& allowedSet = m_unknownKeysAllowedTargets[linkKey];
          for (auto iter = allowedSet.begin(); iter != allowedSet.end();)
          {
            auto index = *iter;
            auto& targetOffsetColumn = *m_targetOffsetColumns.at(index);
            if (eventIndex + 1 >= targetOffsetColumn.size())
              throw std::runtime_error ("target offset column not yet filled for: " + m_targetNames.at(index));
            if (targetOffsetColumn.at(eventIndex) + linkIndex >= targetOffsetColumn.at(eventIndex + 1))
              iter = allowedSet.erase(iter);
            else
              ++ iter;
          }
          // Not quite sure whether it is safer to use or not use one of
          // the targets from the allowed set in this case. In general
          // tools are expected to handle invalid links gracefully,
          // worst case they throw an exception when trying to access
          // it. So what I came up with is that for variant links we
          // assume it invalid, but for non-variant links the tool
          // expects exactly one target and we either found it or throw
          // an exception.
          if (m_keysColumns.empty())
          {
            if (allowedSet.size() == 1 && m_targetKeys.at(*allowedSet.begin()) == 0 && m_unknownKeysAllowedTargets.size() == 1)
              targetIndex = *allowedSet.begin();
            else
            {
              std::ostringstream error;
              error << "target key mismatch: read sgkey " << std::hex << linkKey << std::dec;
              error << " for column " << m_columnName << " with element index " << linkIndex << " targeting " << m_targetNames.at(0);
              if (m_targetKeys.at(0) != 0u)
              {
                error << ", expected sgkey " << std::hex << m_targetKeys.at(0) << std::dec;
              } else if (m_unknownKeysAllowedTargets.size() > 1)
              {
                error << ", alternate key found for non-variant link:";
                for (auto& [key, allowedSet] : m_unknownKeysAllowedTargets)
                {
                  if (key != linkKey)
                    error << " " << std::hex << key << std::dec;
                }
              } else
              {
                error << ", no expected sgkey configured but the maximum allowed index for the target is " << m_targetOffsetColumns.at(0)->at(eventIndex + 1) - m_targetOffsetColumns.at(0)->at(eventIndex) - 1;
              }
              throw std::runtime_error (std::move (error).str());
            }
          }
        }

        if (targetIndex == m_targetKeys.size())
        {
          // this creates a link with an unknown key, which the user
          // will ignore
          m_columnData.push_back (CM::mergeLinkKeyIndex (0xff, linkIndex));
          return;
        }

        auto& targetOffsetColumn = *m_targetOffsetColumns.at(targetIndex);
        if (eventIndex + 1 >= targetOffsetColumn.size())
          throw std::runtime_error ("target offset column not yet filled for: " + m_targetNames.at(targetIndex));
        auto myLinkIndex = linkIndex + targetOffsetColumn.at(eventIndex);
        if (myLinkIndex >= targetOffsetColumn.at(eventIndex + 1))
          throw std::runtime_error ("index out of range for link: " + m_columnName + " with element index " + std::to_string(linkIndex) + " targeting " + m_targetNames.at(targetIndex) + " with offset " + std::to_string(targetOffsetColumn.at(eventIndex)) + " and next offset " + std::to_string(targetOffsetColumn.at(eventIndex + 1)));

        m_columnData.push_back (CM::mergeLinkKeyIndex (targetIndex, myLinkIndex));
      }

      [[nodiscard]] std::size_t size () const noexcept
      {
        return m_columnData.size();
      }

      [[nodiscard]] const typename CM::LinkIndexType* data () const noexcept
      {
        return m_columnData.data();
      }

      [[nodiscard]] auto begin () const noexcept { return m_columnData.begin(); }
      [[nodiscard]] auto end () const noexcept { return m_columnData.end(); }

      [[nodiscard]] const std::vector<typename CM::LinkKeyType>& keysColumn (std::size_t index) const
      {
        return m_keysColumns.at(index);
      }



      /// Private Members
      /// ===============
    private:

      std::vector<typename CM::LinkIndexType> m_columnData;

      std::string m_columnName;

      std::vector<std::string> m_targetNames;
      std::vector<SG::sgkey_t> m_targetKeys;
      std::vector<const std::vector<ColumnarOffsetType>*> m_targetOffsetColumns;

      // there can be multiple keys-columns, hence this is a vector of
      // vectors. if this is empty, then it is a single-target link
      std::vector<std::vector<typename CM::LinkKeyType>> m_keysColumns;

      std::unordered_map<SG::sgkey_t,std::unordered_set<std::size_t>> m_unknownKeysAllowedTargets;



      void addTarget (const std::string& name, const std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& offsetColumns, std::uint32_t clid = 0)
      {
        unsigned targetIndex = 0;
        while (targetIndex < m_targetNames.size() && m_targetNames.at(targetIndex) != name)
          ++ targetIndex;
        if (targetIndex == m_targetNames.size())
        {
          m_targetNames.push_back(name);
          if (auto offsetIter = offsetColumns.find (name); offsetIter != offsetColumns.end())
            m_targetOffsetColumns.push_back (offsetIter->second);
          else
            throw std::runtime_error ("missing offset column: " + name);
          if (clid != 0)
            m_targetKeys.push_back (computeSgKey (name, clid));
          else if (auto keyIter = knownSgKeys.find (name); keyIter != knownSgKeys.end())
            m_targetKeys.push_back (keyIter->second);
          else
            m_targetKeys.push_back (0);
        }
        if (!m_keysColumns.empty())
          m_keysColumns.back().push_back (targetIndex);
      }
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
      using CM = ColumnarModeArray;
      Reader<std::vector<std::vector<ElementLink<T>>>> branchReader;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<ColumnarOffsetType> eventOffsets = {0};
      LinkColumnVector columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;
      unsigned nullEntries = 0;

      explicit ColumnReaderVectorVectorLink (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName() + ".data"});
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
        eventOffsets.clear();
        eventOffsets.push_back (0);
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        columnData.checkOffsets (eventOffsets.size() - 1);
        for (auto& data : branchData)
        {
          for (auto& element : data)
          {
            columnData.addLink (element, eventOffsets.size()-1);
          }
          offsets.push_back (columnData.size());
        }
        eventOffsets.push_back (offsets.size());
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
            colData.setColumn (outputColumns.at(i).columnIndex, columnData.keysColumn(i-2).size(), columnData.keysColumn(i-2).data());
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
        for (const auto& index : columnData)
        {
          if (index == invalidObjectIndex)
            nullEntries += 1;
        }
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
      using CM = ColumnarModeArray;
      Reader<std::vector<ElementLink<T>>> branchReader;
      const std::vector<ColumnarOffsetType>* offsetColumn = nullptr;
      std::vector<ColumnarOffsetType> offsets = {0};
      LinkColumnVector columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;
      unsigned nullEntries = 0;

      ColumnReaderVectorLink (const std::string& val_branchName)
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
        columnData.checkOffsets (offsets.size() - 1);
        for (auto& element : branchData)
          columnData.addLink (element, offsets.size()-1);
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
            colData.setColumn (outputColumns.at(i).columnIndex, columnData.keysColumn(i-2).size(), columnData.keysColumn(i-2).data());
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
        for (const auto& index : columnData)
        {
          if (index == invalidObjectIndex)
            nullEntries += 1;
        }
      }
    };

    template <typename T, template <typename> class Reader>
    struct ColumnReaderVectorRLink final : public TestUtils::ColumnReader 
    {
      using CM = ColumnarModeArray;
      Reader<std::vector<ElementLink<T>>> branchReader;
      const std::vector<ColumnarOffsetType>* offsetColumn = nullptr;
      std::vector<ColumnarOffsetType> offsets = {0};
      LinkColumnVector columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;
      unsigned nullEntries = 0;

      ColumnReaderVectorRLink(const std::string& val_branchName)
      : branchReader(val_branchName), benchmarkUnpack(branchReader.columnName() + "(unpack)"), benchmark(branchReader.columnName()) 
      {
        outputColumns.push_back({.name = branchReader.columnName()});
        outputColumns.push_back({.name = branchReader.containerName(), .isOffset = true, .primary = false});
      }

      virtual bool connect(Backend source, std::unordered_map<std::string, const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
        auto iter = requestedColumns.find(outputColumns.at(0).name);
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

      virtual void clearColumns() override 
      {
        columnData.clear();
        offsets.clear();
        offsets.push_back(0);
      }

      virtual void getEntry(Long64_t entry) override 
      {
        benchmark.startTimer();
        const auto& branchData = branchReader.getEntry(entry);
        benchmark.stopTimer();
        benchmarkUnpack.startTimer();

       columnData.checkOffsets (offsets.size() - 1);
       for (const auto& element : branchData) 
       {
         if (element.isDefault() || element.index() == static_cast<unsigned int>(-1)) 
           columnData.addEmptyLink ();
          else
            columnData.addSplitLink (element.index(), element.key(), offsets.size()-1);
        }

        offsets.push_back(columnData.size());

        if (offsetColumn) {
          if (offsetColumn->size() != offsets.size()) 
          {
            throw std::runtime_error("offset column not filled yet: " + outputColumns.at(1).name);
          }
          if (offsetColumn->back() != offsets.back()) 
          {
            throw std::runtime_error("offset column does not match: " + outputColumns.at(1).name);
        }
        }

        benchmarkUnpack.stopTimer();
      }

      virtual void setData(ColumnVectorData& colData) override 
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn(outputColumns.at(0).columnIndex, columnData.size(), columnData.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex) 
          colData.setColumn(outputColumns.at(1).columnIndex, offsets.size(), offsets.data());
        for (std::size_t i = 2; i < outputColumns.size(); ++ i)
        {
          if (outputColumns.at(i).columnIndex != ColumnVectorHeader::nullIndex)
            colData.setColumn (outputColumns.at(i).columnIndex, columnData.keysColumn(i-2).size(), columnData.keysColumn(i-2).data());
        }
      }

      [[nodiscard]] virtual BranchPerfData getPerfData(float emptyTime) override 
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

      virtual void collectColumnData() override 
      {
        entries += columnData.size();
        for (const auto& index : columnData) 
        {
          if (index == invalidObjectIndex)
            nullEntries += 1;
          }
      }
    };

    template <typename T, template <typename> class Reader>
    struct ColumnReaderVectorVectorVariantLink final : public TestUtils::ColumnReader 
    {
      using CM = ColumnarModeArray;
      Reader<std::vector<std::vector<ElementLink<T>>>> branchReader;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<ColumnarOffsetType> eventOffsets = {0};
      LinkColumnVector columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;
      unsigned nullEntries = 0;

      explicit ColumnReaderVectorVectorVariantLink (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName() + ".data"});
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

        iter = requestedColumns.find (outputColumns.at(1).name);
        if (iter != requestedColumns.end())
        {
          outputColumns.at(1).enabled = true;
          requestedColumns.erase (iter);
        }

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
        eventOffsets.clear();
        eventOffsets.push_back (0);
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        columnData.checkOffsets (eventOffsets.size() - 1);
        for (auto& data : branchData)
        {
          for (auto& element : data)
          {
            columnData.addLink (element, eventOffsets.size()-1);
          }
          offsets.push_back (columnData.size());
        }
        eventOffsets.push_back (offsets.size());
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
            colData.setColumn (outputColumns.at(i).columnIndex, columnData.keysColumn(i-2).size(), columnData.keysColumn(i-2).data());
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
        for (const auto& index : columnData)
        {
          if (index == invalidObjectIndex)
            nullEntries += 1;
        }
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
  }
}

#endif // COLUMNAR_TEST_FIXTURES_COLUMN_READER_BRANCH_H