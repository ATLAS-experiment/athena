/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TEST_FIXTURES_COLUMN_READER_H
#define COLUMNAR_TEST_FIXTURES_COLUMN_READER_H

#include <ColumnarInterfaces/ColumnarDef.h>
#include <ColumnarToolWrapper/ColumnVectorWrapper.h>
#include <ColumnarToolWrapper/ColumnarToolHelpers.h>

#include <Rtypes.h>

#include <memory>
#include <unordered_map>
#include <variant>

class TTree;
namespace ROOT
{
  class RNTupleReader;
  namespace Experimental
  {
    class RNTupleInspector;
  }
}

namespace columnar
{
  class ColumnVectorData;

  namespace TestUtils
  {
    struct BranchPerfData;
    struct TestDefinition;

    struct RNTupleBackend {
      ROOT::RNTupleReader* reader = nullptr;
      ROOT::Experimental::RNTupleInspector* inspector = nullptr;
    };

    using Backend = std::variant<TTree*, RNTupleBackend*>;

    class ColumnReader
    {
    public:

      struct OutputColumnInfo
      {
        std::string name;
        bool isOffset = false;
        bool primary = false;
        bool enabled = false;
        std::size_t columnIndex = ColumnVectorHeader::nullIndex;
      };
      std::vector<OutputColumnInfo> outputColumns;

      virtual ~ColumnReader () noexcept = default;

      virtual bool connect(Backend source, std::unordered_map<std::string, const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string, ColumnInfo>& requestedColumns) = 0;

      /// @brief lookup and store column indices from the header for all enabled output columns
      void connectColumnIndices (const ColumnVectorHeader& header)
      {
        for (auto& col : outputColumns)
        {
          if (col.enabled)
            col.columnIndex = header.getColumnIndex (col.name);
        }
      }

      virtual void clearColumns () = 0;

      virtual void getEntry (Long64_t entry) = 0;

      virtual void setData (ColumnVectorData& columnData) = 0;

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) = 0;

      virtual void collectColumnData () = 0;
    };

    void makeCommonColumnReaders (std::vector<std::shared_ptr<TestUtils::ColumnReader>>& knownColumns);
    void makeTTreeColumnReaders (std::vector<std::shared_ptr<TestUtils::ColumnReader>>& knownColumns, TTree *tree);
    void makeRNTupleColumnReaders (std::vector<std::shared_ptr<TestUtils::ColumnReader>>& knownColumns, RNTupleBackend* rntbackend);
    void makeCommonLateColumnReaders (std::vector<std::shared_ptr<TestUtils::ColumnReader>>& knownColumns, const std::vector<std::string>& allMetTermNames);
    void makeOutputColumnReaders (std::vector<std::shared_ptr<TestUtils::ColumnReader>>& knownColumns, const ColumnVectorHeader& columnHeader);
  }
}

#endif