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
#include <ranges>

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
      for (auto&& part : std::views::split(preloader.value(), ',')) preloaderList.emplace_back(part.begin(), part.end());

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
