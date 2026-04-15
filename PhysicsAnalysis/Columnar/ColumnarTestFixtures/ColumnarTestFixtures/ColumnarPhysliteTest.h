/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TEST_FIXTURES__COLUMNAR_PHYS_LITE_TEST_H
#define COLUMNAR_TEST_FIXTURES__COLUMNAR_PHYS_LITE_TEST_H

#include <AsgTools/AsgTool.h>
#include <ColumnarInterfaces/ColumnInfo.h>
#include <ColumnarInterfaces/IColumnarTool.h>
#include <ColumnarTestFixtures/Configuration.h>
#include <ColumnarTestFixtures/IXAODToolCaller.h>
#include <ROOT/RNTupleDescriptor.hxx>
#include <ROOT/RNTupleInspector.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RPageStorageFile.hxx>
#include <gtest/gtest.h>

#include <functional>
#include <span>
#include <string>
#include <vector>

class TFile;
class TTree;

namespace columnar
{
  class ColumnVectorHeader;
  class ToolColumnVectorMap;

  namespace TestUtils
  {
    class IColumnData;
    struct RNTupleBackend;
  }


  struct ColumnarPhysLiteTest : testing::Test
  {
    std::unique_ptr<TFile> file;
    TTree *tree = nullptr;
    std::unique_ptr<ROOT::RNTupleReader> rntreader;
    std::unique_ptr<ROOT::Experimental::RNTupleInspector> inspector;
    columnar::TestUtils::RNTupleBackend* rntbackend = nullptr;
    std::vector<std::shared_ptr<TestUtils::IColumnData>> knownColumns;
    std::vector<std::shared_ptr<TestUtils::IColumnData>> usedColumns;
    std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*> offsetColumns;

    ColumnarPhysLiteTest ();
    ~ColumnarPhysLiteTest ();

    /// \brief make a unique tool name to be used in unit tests
    std::string makeUniqueName ();

    /// @brief check whether we have the right mode
    static bool checkMode ();

    void setupKnownColumns (std::span<const TestUtils::TestDefinition> testDefinitions);

    void setupColumns (const ColumnVectorHeader& columnHeader);

    void doCall (const TestUtils::TestDefinition& testDefinition);

    void doCallMulti (const std::vector<TestUtils::TestDefinition>& testDefinitions);
  };
}

#endif
