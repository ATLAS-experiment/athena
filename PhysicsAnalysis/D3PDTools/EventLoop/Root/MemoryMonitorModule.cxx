/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EventLoop/MemoryMonitorModule.h>

#include <EventLoop/MessageCheck.h>
#include <EventLoop/ModuleData.h>
#include <TSystem.h>

//
// method implementations
//

namespace EL
{
  namespace Detail
  {
    MemoryMonitorModule ::
    MemoryMonitorModule (const std::string& name)
      : AsgMessaging (name)
    {}



    StatusCode MemoryMonitorModule ::
    firstInitialize (ModuleData& /*data*/)
    {
      ANA_CHECK (printMemoryUsage ("firstInitialize"));
      return StatusCode::SUCCESS;
    }



    StatusCode MemoryMonitorModule ::
    onInitialize (ModuleData& /*data*/)
    {
      ANA_CHECK (printMemoryUsage ("onInitialize"));
      return StatusCode::SUCCESS;
    }



    StatusCode MemoryMonitorModule ::
    onExecute (ModuleData& /*data*/)
    {
      if (++ m_numExecute == m_executeNext)
      {
        m_executeNext += m_executeStep;
        if (m_executeNext == m_executeTarget)
          m_executeStep *= 10, m_executeTarget *= 10;

        ANA_CHECK (printMemoryUsage ("onExecute(" + std::to_string (m_numExecute) + ")"));
      }
      return StatusCode::SUCCESS;
    }



    StatusCode MemoryMonitorModule ::
    postFirstEvent (ModuleData& /*data*/)
    {
      ANA_CHECK (printMemoryUsage ("postFirstEvent"));
      return StatusCode::SUCCESS;
    }



    StatusCode MemoryMonitorModule ::
    onFinalize (ModuleData& /*data*/)
    {
      ANA_CHECK (printMemoryUsage ("onFinalize"));
      return StatusCode::SUCCESS;
    }



    StatusCode MemoryMonitorModule ::
    onWorkerEnd (ModuleData& /*data*/)
    {
      ANA_CHECK (printMemoryUsage ("onWorkerEnd"));
      return StatusCode::SUCCESS;
    }



    StatusCode MemoryMonitorModule ::
    printMemoryUsage (const std::string& location)
    {
      ::ProcInfo_t pinfo;
      if (gSystem->GetProcInfo (&pinfo) != 0) {
        ANA_MSG_ERROR ("Could not get memory usage information");
        return StatusCode::FAILURE;
      }
      ANA_MSG_INFO ("Memory usage at " << location << ": " << pinfo.fMemResident << " kB resident, " << pinfo.fMemVirtual << " kB virtual");
      lastRSS = pinfo.fMemResident;
      return StatusCode::SUCCESS;
    }
  }
}
