/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ComponentFactoryPreloader/ComponentFactoryPreloader.h>

#include <AsgTools/AsgComponentFactories.h>
#include <AsgTools/MessageCheckAsgTools.h>
#include <mutex>

//
// method implementations
//

namespace CP
{
  // this function gets called once by the function below.  calling it
  // once avoids any errors if setup happens multiple times, e.g. in
  // test fixtures
  static bool doPreloadComponentFactories ()
  {
    using namespace asg::msgComponentConfig;
    ANA_CHECK_SET_TYPE (bool);

    // uncomment this if you want to see detailed messages about
    // component factories and configuration
    // asg::msgComponentConfig::setMsgLevel (MSG::DEBUG);

    ANA_MSG_INFO ("preloading component factories");

    return true;
  }

  bool preloadComponentFactories ()
  {
    static bool result = false;
    static std::once_flag flag;
    std::call_once (flag, [&] () { result = doPreloadComponentFactories (); });
    return result;
  }
}
