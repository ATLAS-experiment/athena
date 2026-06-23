/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EventLoop/Module.h>

#include <RootCoreUtils/Assert.h>

//
// method implementations
//

namespace EL
{
  namespace Detail
  {
    ::StatusCode Module ::
    firstInitialize (ModuleData& /*data*/)
    {
      return ::StatusCode::SUCCESS;
    }

    ::StatusCode Module ::
    preFileInitialize (ModuleData& /*data*/)
    {
      return ::StatusCode::SUCCESS;
    }

    ::StatusCode Module ::
    onFirstInputFile (ModuleData& /*data*/)
    {
      return ::StatusCode::SUCCESS;
    }

    ::StatusCode Module ::
    onNextInputFile (ModuleData& /*data*/)
    {
      return ::StatusCode::SUCCESS;
    }

    ::StatusCode Module ::
    onNewInputFile (ModuleData& /*data*/)
    {
      return ::StatusCode::SUCCESS;
    }

    ::StatusCode Module ::
    onCloseInputFile (ModuleData& /*data*/)
    {
      return ::StatusCode::SUCCESS;
    }

    ::StatusCode Module ::
    postCloseInputFile (ModuleData& /*data*/)
    {
      return ::StatusCode::SUCCESS;
    }

    ::StatusCode Module ::
    postFirstEvent (ModuleData& /*data*/)
    {
      return ::StatusCode::SUCCESS;
    }

    void Module ::
    reportInputFailure (ModuleData& /*data*/)
    {}

    ::StatusCode Module ::
    onFileExecute (ModuleData& /*data*/)
    {
      return ::StatusCode::SUCCESS;
    }

    ::StatusCode Module ::
    onExecute (ModuleData& /*data*/)
    {
      return ::StatusCode::SUCCESS;
    }

    ::StatusCode Module ::
    onInitialize (ModuleData& /*data*/)
    {
      return ::StatusCode::SUCCESS;
    }

    ::StatusCode Module ::
    processInputs (ModuleData& /*data*/, IInputModuleActions& /*actions*/)
    {
      return ::StatusCode::SUCCESS;
    }

    ::StatusCode Module ::
    onFinalize (ModuleData& /*data*/)
    {
      return ::StatusCode::SUCCESS;
    }

    ::StatusCode Module ::
    postFinalize (ModuleData& /*data*/)
    {
      return ::StatusCode::SUCCESS;
    }

    ::StatusCode Module ::
    onWorkerEnd (ModuleData& /*data*/)
    {
      return ::StatusCode::SUCCESS;
    }

    ::StatusCode Module ::
    postFileClose (ModuleData& /*data*/)
    {
      return ::StatusCode::SUCCESS;
    }
  }
}
