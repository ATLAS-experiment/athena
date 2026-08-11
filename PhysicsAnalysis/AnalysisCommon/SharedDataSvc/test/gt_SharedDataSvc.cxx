/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <SharedDataSvc/ISharedDataSvc.h>
#include <SharedDataSvc/TFileHelpers.h>

#include <AsgMessaging/MessageCheck.h>
#include <AsgServices/AsgServiceConfig.h>
#include <AsgTesting/UnitTest.h>
#include <AsgServices/ServiceHandle.h>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>

//
// method implementations
//

using namespace asg::msgUserCode;

namespace asg
{
  TEST (SharedDataSvcTest, basic)
  {
    asg::AsgServiceConfig config ("asg::SharedDataSvc/SharedDataSvc");
    std::shared_ptr<asg::ISharedDataSvc> svc;
    ASSERT_SUCCESS (config.makeService (svc));

    std::shared_ptr<const std::string> data1, data2;
    std::shared_ptr<const unsigned> data3;
    ASSERT_SUCCESS (svc->getMakeShared ("data", data1, [] (std::shared_ptr<const std::string>& ptr) {ptr = std::make_shared<std::string> ("hello"); return StatusCode::SUCCESS;}));
    ASSERT_EQ (*data1, "hello");
    ASSERT_SUCCESS (svc->getMakeShared ("data", data2, [] (std::shared_ptr<const std::string>& ptr) {ptr = std::make_shared<std::string> ("hello"); return StatusCode::SUCCESS;}));
    ASSERT_EQ (data1, data2);
    ASSERT_SUCCESS (svc->getMakeShared ("data2", data2, [] (std::shared_ptr<const std::string>& ptr) {ptr = std::make_shared<std::string> ("world"); return StatusCode::SUCCESS;}));
    ASSERT_NE (data1, data2);
    ASSERT_EQ (*data2, "world");

    ASSERT_FAILURE (svc->getMakeShared ("data3", data2, [] (std::shared_ptr<const std::string>& ptr) {ptr = std::make_shared<std::string> ("world"); return StatusCode::FAILURE;}));
    ASSERT_FAILURE (svc->getMakeShared ("data4", data2, [] (std::shared_ptr<const std::string>& /*ptr*/) {return StatusCode::SUCCESS;}));
    ASSERT_FAILURE (svc->getMakeShared ("data", data3, [] (std::shared_ptr<const unsigned>& ptr) {ptr = std::make_shared<unsigned> (42); return StatusCode::SUCCESS;}));

    std::weak_ptr<const std::string> weak_data1 = data1;
    ASSERT_FALSE (weak_data1.expired ());
    data1.reset ();
    ASSERT_TRUE (weak_data1.expired ());
    ASSERT_SUCCESS (svc->getMakeShared ("data", data1, [] (std::shared_ptr<const std::string>& ptr) {ptr = std::make_shared<std::string> ("hello"); return StatusCode::SUCCESS;}));
    ASSERT_EQ (*data1, "hello");
  }

  TEST (SharedDataSvcTest, TFile)
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

    asg::AsgServiceConfig config ("asg::SharedDataSvc/SharedDataSvc");
    std::shared_ptr<asg::ISharedDataSvc> svc;
    ASSERT_SUCCESS (config.makeService (svc));
    std::shared_ptr<TFile> file;

    std::shared_ptr<const TH2> hist2;
    ASSERT_SUCCESS (readFromTFile (*svc, file, "test.root", "hist2d", hist2));
    ASSERT_NE (hist2, nullptr);

    std::shared_ptr<const TH1> hist1;
    ASSERT_SUCCESS (readFromTFile (*svc, file, "test.root", "hist2d", hist1));
    ASSERT_NE (hist1, nullptr);

    std::shared_ptr<const TH3> hist3;
    ASSERT_FAILURE (readFromTFile (*svc, file, "test.root", "hist2d", hist3));

    std::shared_ptr<TFile> file2;
    ASSERT_FAILURE (readFromTFile (*svc, file2, "testMISS.root", "hist2d", hist1));
    ASSERT_FAILURE (readFromTFile (*svc, file, "test.root", "hist2dMISS", hist1));

    std::shared_ptr<const TH2> hist2Nested;
    ASSERT_SUCCESS (readFromTFile (*svc, file, "test.root", "subdir/hist2dNested", hist2Nested));
    ASSERT_NE (hist2Nested, nullptr);

    std::shared_ptr<const TH2> hist2Nested2;
    ASSERT_SUCCESS (readFromTFile (*svc, file, "test.root", "subdir/subdir2/hist2dNested2", hist2Nested2));
    ASSERT_NE (hist2Nested2, nullptr);
  }
}

ATLAS_GOOGLE_TEST_MAIN
