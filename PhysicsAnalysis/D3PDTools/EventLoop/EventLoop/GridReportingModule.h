/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef EVENT_LOOP_GRID__GRID_REPORTING_MODULE_H
#define EVENT_LOOP_GRID__GRID_REPORTING_MODULE_H

#include <EventLoop/Module.h>
#include <vector>

namespace EL
{
  namespace Detail
  {
    /// \brief a \ref Module that handles the reporting from the job
    /// on the grid

    class GridReportingModule final : public Module
    {
      /// the panda error code for bad input files
      static constexpr int EC_BADINPUT = 223;

      /// the list of files we processed
      std::vector<std::string> m_files;

      /// the number of events we processed
      unsigned m_eventsProcessed = 0;

    public:

      using Module::Module;

      virtual StatusCode onNewInputFile (ModuleData& data) override;
      virtual StatusCode onExecute (ModuleData& data) override;
      virtual StatusCode postFileClose (ModuleData& data) override;
      virtual void reportInputFailure (ModuleData& data) override;
    };
  }
}

#endif
