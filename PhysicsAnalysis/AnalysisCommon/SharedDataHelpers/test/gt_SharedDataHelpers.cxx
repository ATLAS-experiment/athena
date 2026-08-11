/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <SharedDataHelpers/MessageCheck.h>
#include <SharedDataHelpers/SharedDataHelpers.h>
#include <SharedDataHelpers/TFileHelpers.h>

#include <AsgTesting/UnitTest.h>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>

#include <atomic>
#include <thread>
#include <vector>

//
// method implementations
//

namespace asg
{
  using namespace msgSharedDataHelpers;

  TEST (SharedDataHelpersTest, basic)
  {
    std::shared_ptr<const std::string> data1, data2;
    std::shared_ptr<const unsigned> data3;
    ASSERT_SUCCESS (getMakeSharedData ("data", data1, [] (std::shared_ptr<const std::string>& ptr) {ptr = std::make_shared<std::string> ("hello"); return StatusCode::SUCCESS;}));
    ASSERT_EQ (*data1, "hello");
    ASSERT_SUCCESS (getMakeSharedData ("data", data2, [] (std::shared_ptr<const std::string>& ptr) {ptr = std::make_shared<std::string> ("hello"); return StatusCode::SUCCESS;}));
    ASSERT_EQ (data1, data2);
    ASSERT_SUCCESS (getMakeSharedData ("data2", data2, [] (std::shared_ptr<const std::string>& ptr) {ptr = std::make_shared<std::string> ("world"); return StatusCode::SUCCESS;}));
    ASSERT_NE (data1, data2);
    ASSERT_EQ (*data2, "world");

    ASSERT_FAILURE (getMakeSharedData ("data3", data2, [] (std::shared_ptr<const std::string>& ptr) {ptr = std::make_shared<std::string> ("world"); return StatusCode::FAILURE;}));
    ASSERT_FAILURE (getMakeSharedData ("data4", data2, [] (std::shared_ptr<const std::string>& /*ptr*/) {return StatusCode::SUCCESS;}));
    ASSERT_FAILURE (getMakeSharedData ("data", data3, [] (std::shared_ptr<const unsigned>& ptr) {ptr = std::make_shared<unsigned> (42); return StatusCode::SUCCESS;}));

    std::weak_ptr<const std::string> weak_data1 = data1;
    ASSERT_FALSE (weak_data1.expired ());
    data1.reset ();
    ASSERT_TRUE (weak_data1.expired ());
    ASSERT_SUCCESS (getMakeSharedData ("data", data1, [] (std::shared_ptr<const std::string>& ptr) {ptr = std::make_shared<std::string> ("hello"); return StatusCode::SUCCESS;}));
    ASSERT_EQ (*data1, "hello");
  }

  TEST (SharedDataHelpersTest, TFile)
  {
    {
      std::unique_ptr<TFile> testFile {TFile::Open ("test.root", "RECREATE")};
      {
        TH2F hist2d ("hist2d", "hist2d", 10, 0, 10, 10, 0, 10);
        hist2d.Write();
      }
      // write the next inside a subdirectory
      TDirectory* subdir = testFile->mkdir("subdir");
      subdir->cd();
      {
        TH2F hist2d ("hist2dNested", "hist2dNested", 10, 0, 10, 10, 0, 10);
        hist2d.Write();
      }
      // and another nested further
      TDirectory* subdir2 = subdir->mkdir("subdir2");
      subdir2->cd();
      {
        TH2F hist2d ("hist2dNested2", "hist2dNested2", 10, 0, 10, 10, 0, 10);
        hist2d.Write();
      }
    }

    std::shared_ptr<TFile> file;

    std::shared_ptr<const TH2> hist2;
    ASSERT_SUCCESS (readFromTFile (file, "test.root", "hist2d", hist2));
    ASSERT_NE (hist2, nullptr);

    std::shared_ptr<const TH1> hist1;
    ASSERT_SUCCESS (readFromTFile (file, "test.root", "hist2d", hist1));
    ASSERT_NE (hist1, nullptr);

    std::shared_ptr<const TH3> hist3;
    ASSERT_FAILURE (readFromTFile (file, "test.root", "hist2d", hist3));

    std::shared_ptr<TFile> file2;
    ASSERT_FAILURE (readFromTFile (file2, "testMISS.root", "hist2d", hist1));
    ASSERT_FAILURE (readFromTFile (file, "test.root", "hist2dMISS", hist1));

    std::shared_ptr<const TH2> hist2Nested;
    ASSERT_SUCCESS (readFromTFile (file, "test.root", "subdir/hist2dNested", hist2Nested));
    ASSERT_NE (hist2Nested, nullptr);

    std::shared_ptr<const TH2> hist2Nested2;
    ASSERT_SUCCESS (readFromTFile (file, "test.root", "subdir/subdir2/hist2dNested2", hist2Nested2));
    ASSERT_NE (hist2Nested2, nullptr);
  }
}

ATLAS_GOOGLE_TEST_MAIN
