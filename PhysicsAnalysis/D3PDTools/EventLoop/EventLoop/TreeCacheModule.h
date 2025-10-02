/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef EVENT_LOOP__TREE_CACHE_MODULE_H
#define EVENT_LOOP__TREE_CACHE_MODULE_H

#include <EventLoop/Global.h>

#include <AsgTools/PropertyWrapper.h>
#include <EventLoop/Module.h>

namespace EL
{
  namespace Detail
  {
    /// \brief a \ref Module for setting the TTreeCache parameters on
    /// the input

    class TreeCacheModule : public Module
    {
      //
      // public interface
      //

    public:

      using Module::Module;

      virtual StatusCode onNewInputFile (ModuleData& data) override;
      virtual StatusCode onCloseInputFile (ModuleData& data) override;

      Gaudi::Property<std::int64_t> cacheSize {this, "cacheSize", 0};
      Gaudi::Property<std::int64_t> cacheLearnEntries {this, "cacheLearnEntries", 0};
      Gaudi::Property<bool> printPerFileStats {this, "printPerFileStats", false};
    };
  }
}

#endif
