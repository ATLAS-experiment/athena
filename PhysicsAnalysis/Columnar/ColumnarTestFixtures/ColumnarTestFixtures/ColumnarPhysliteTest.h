/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TEST_FIXTURES__COLUMNAR_PHYS_LITE_TEST_H
#define COLUMNAR_TEST_FIXTURES__COLUMNAR_PHYS_LITE_TEST_H

#include <AsgTools/AsgTool.h>

#include <gtest/gtest.h>

#include <functional>
#include <string>
#include <vector>

class TFile;
class TTree;

namespace columnar
{
  namespace PhysliteTestHelpers
  {
    class IColumnData;
  }

  class ColumnarToolWrapper;

  struct ColumnarPhysLiteTest : testing::Test
  {
    std::unique_ptr<TFile> file;
    TTree *tree = nullptr;

    std::vector<std::shared_ptr<PhysliteTestHelpers::IColumnData>> knownColumns;
    std::vector<std::shared_ptr<PhysliteTestHelpers::IColumnData>> usedColumns;
    std::unordered_map<std::string,const PhysliteTestHelpers::IColumnData*> sizeColumns;

    ColumnarPhysLiteTest ();
    ~ColumnarPhysLiteTest ();

    /// \brief make a unique tool name to be used in unit tests
    std::string makeUniqueName ();

    /// @brief check whether we have the right mode
    static bool checkMode ();

    void setupKnownColumns ();

    void setupColumns (ColumnarToolWrapper& toolWrapper);

    /// the arguments for the function calling in xAOD mode
    struct XAODArgs
    {
      std::string inputContainer;
      std::string outputContainer;
      bool isPrepCall = false;
    };

    void doCall (asg::AsgTool& tool, const std::string& name, const std::string& container, std::function<void(XAODArgs&)> callXAOD, const std::vector<std::pair<std::string,std::string>>& containerRenames, const std::string& sysName = "");
  };
}

#endif