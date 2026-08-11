/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <AsgMessaging/MessageCheck.h>
#include <AsgServices/AsgServiceConfig.h>
#include <AsgTesting/UnitTest.h>
// #include <AsgTools/AsgTool.h>
// #include <AsgTools/CurrentContext.h>
// #include <xAODRootAccess/Init.h>
// #include <xAODRootAccess/TEvent.h>
// #include <xAODRootAccess/TStore.h>
// #include <xAODJet/JetContainer.h>
// #include <xAODJet/JetAuxContainer.h>
// #include <SystematicsHandles/ISystematicsSvc.h>
// #include <SystematicsHandles/SysListHandle.h>
// #include <SystematicsHandles/SysCopyHandle.h>
// #include <SystematicsHandles/SysReadHandle.h>
#include <SharedDataSvc/ISharedDataSvc.h>
#include <AsgServices/ServiceHandle.h>
// #include <memory>

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
    ASSERT_SUCCESS (svc->get_make_shared ("data", data1, [] (std::shared_ptr<const std::string>& ptr) {ptr = std::make_shared<std::string> ("hello"); return StatusCode::SUCCESS;}));
    ASSERT_EQ (*data1, "hello");
    ASSERT_SUCCESS (svc->get_make_shared ("data", data2, [] (std::shared_ptr<const std::string>& ptr) {ptr = std::make_shared<std::string> ("hello"); return StatusCode::SUCCESS;}));
    ASSERT_EQ (data1, data2);
    ASSERT_SUCCESS (svc->get_make_shared ("data2", data2, [] (std::shared_ptr<const std::string>& ptr) {ptr = std::make_shared<std::string> ("world"); return StatusCode::SUCCESS;}));
    ASSERT_NE (data1, data2);
    ASSERT_EQ (*data2, "world");

    ASSERT_FAILURE (svc->get_make_shared ("data3", data2, [] (std::shared_ptr<const std::string>& ptr) {ptr = std::make_shared<std::string> ("world"); return StatusCode::FAILURE;}));
    ASSERT_FAILURE (svc->get_make_shared ("data4", data2, [] (std::shared_ptr<const std::string>& /*ptr*/) {return StatusCode::SUCCESS;}));
    ASSERT_FAILURE (svc->get_make_shared ("data", data3, [] (std::shared_ptr<const unsigned>& ptr) {ptr = std::make_shared<unsigned> (42); return StatusCode::SUCCESS;}));

    std::weak_ptr<const std::string> weak_data1 = data1;
    ASSERT_FALSE (weak_data1.expired ());
    data1.reset ();
    ASSERT_TRUE (weak_data1.expired ());
    ASSERT_SUCCESS (svc->get_make_shared ("data", data1, [] (std::shared_ptr<const std::string>& ptr) {ptr = std::make_shared<std::string> ("hello"); return StatusCode::SUCCESS;}));
    ASSERT_EQ (*data1, "hello");
  }
}

ATLAS_GOOGLE_TEST_MAIN
