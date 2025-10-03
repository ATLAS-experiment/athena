/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EventLoop/TreeCacheModule.h>

#include <EventLoop/Job.h>
#include <EventLoop/ModuleData.h>
#include <TTree.h>

//
// method implementations
//

namespace EL
{
  namespace Detail
  {
    StatusCode TreeCacheModule ::
    onNewInputFile (ModuleData& data)
    {
      if (data.m_inputTree)
      {
        if (cacheSize.value() > 0)
          data.m_inputTree->SetCacheSize (cacheSize.value());
        if (cacheLearnEntries.value() > 0)
          data.m_inputTree->SetCacheLearnEntries (cacheLearnEntries.value());
      }
      return StatusCode::SUCCESS;
    }



    StatusCode TreeCacheModule ::
    onCloseInputFile (ModuleData& data)
    {
      if (printPerFileStats.value())
      {
        ANA_MSG_INFO ("file stats for: " << data.m_inputFileUrl);
        data.m_inputTree->PrintCacheStats ();
      }
      return StatusCode::SUCCESS;
    }
  }
}
