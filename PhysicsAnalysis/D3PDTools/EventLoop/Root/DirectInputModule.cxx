/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <EventLoop/DirectInputModule.h>

#include <EventLoop/IInputModuleActions.h>
#include <EventLoop/EventRange.h>
#include <RootCoreUtils/Assert.h>

//
// method implementations
//

namespace EL
{
  namespace Detail
  {
    StatusCode DirectInputModule ::
    processInputs (ModuleData& /*data*/, IInputModuleActions& actions)
    {
      Long64_t toSkip = this->skipEvents.value ();
      std::optional<Long64_t> toProcess;
      if (this->maxEvents.value() != -1)
        toProcess = this->maxEvents.value();
      for (const std::string& fileName : fileList.value())
      {
        // open the input file to inspect it
        ANA_CHECK (actions.openInputFile (fileName));
        ANA_MSG_DEBUG ("Opened input file: " << fileName);

        EventRange eventRange;
        eventRange.m_url = fileName;
        eventRange.m_endEvent = actions.inputFileNumEntries();

        if (toSkip > 0)
        {
          if (toSkip >= eventRange.m_endEvent)
          {
            toSkip -= eventRange.m_endEvent;
            ANA_MSG_INFO ("File " << fileName << " has only " << eventRange.m_endEvent << " events, skipping it.");
            continue;
          }
          eventRange.m_beginEvent = toSkip;
          toSkip = 0u;
        }

        if (toProcess.has_value())
        {
          if (eventRange.m_endEvent >= eventRange.m_beginEvent + toProcess.value())
          {
            eventRange.m_endEvent = eventRange.m_beginEvent + toProcess.value();
            toProcess = 0u;
          } else
          {
            toProcess.value() -= eventRange.m_endEvent - eventRange.m_beginEvent;
          }
        }
        ANA_CHECK (actions.processEvents (eventRange));
        if (toProcess.has_value() && toProcess.value() == 0u)
        {
          ANA_MSG_INFO ("Reached maximum number of events, stopping.");
          break;
        }
      }
      return StatusCode::SUCCESS;
    }
  }
}
