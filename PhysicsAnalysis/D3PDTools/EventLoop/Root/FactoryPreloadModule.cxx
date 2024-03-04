/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <EventLoop/FactoryPreloadModule.h>

#include <AsgTools/MessageCheckAsgTools.h>
#include <TInterpreter.h>
#include <TSystem.h>
#include <boost/algorithm/string.hpp>

//
// method implementations
//

namespace EL
{
  namespace Detail
  {
    StatusCode FactoryPreloadModule::onInitialize (ModuleData& /*data*/)
    {
      std::vector<std::string> preloaderList;
      boost::split (preloaderList, preloader, boost::is_any_of (","));

      if (preloaderList.size() % 2 != 0)
      {
        ANA_MSG_ERROR ("Invalid preloader list");
        return StatusCode::FAILURE;
      }

      for (std::size_t iter = 0; iter < preloaderList.size(); iter += 2)
      {
        const std::string& libName = preloaderList[iter];
        const std::string& funcName = preloaderList[iter + 1];

        if (gSystem->Load(libName.c_str()) != 0)
        {
          ANA_MSG_ERROR ("Failed to load library " << libName);
          return StatusCode::FAILURE;
        }
        if (gInterpreter->Calc((funcName + "()").c_str()) != 1)
        {
          ANA_MSG_ERROR ("Failed to call function " << funcName << " from library " << libName);
          return StatusCode::FAILURE;
        }
      }

      return StatusCode::SUCCESS;
    }
  }
}
