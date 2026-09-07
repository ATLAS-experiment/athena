/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef EVENT_LOOP__FILE_EXECUTED_MODULE_H
#define EVENT_LOOP__FILE_EXECUTED_MODULE_H

#include <EventLoop/Global.h>

#include <EventLoop/Module.h>
#include <TString.h>
#include <TTree.h>
#include <memory>

namespace EL
{
  namespace Detail
  {
    /// \brief a \ref Module recording when FileExecuted was called

    class FileExecutedModule final : public Module
    {
      //
      // public interface
      //

    public:

      using Module::Module;

      virtual StatusCode onInitialize (ModuleData& data) override;
      virtual StatusCode onFileExecute (ModuleData& data) override;
      virtual StatusCode postFinalize (ModuleData& data) override;



      //
      // private interface
      //

      /// \brief the tree containing the list of files for which
      /// fileExecute has been called
    private:
      std::unique_ptr<TTree> m_fileExecutedTree; //!

      /// \brief the name of the file being executed, to be stored
      /// inside \ref m_fileExecutedTree
    private:
      TString m_fileExecutedName; //!

      /// \brief pointer to \ref m_fileExecutedName used as the branch
      /// address
      ///
      /// This is a member (rather than a local) so that the address handed
      /// to the branch stays valid for as long as the tree lives.
    private:
      TString *m_fileExecutedNamePtr {&m_fileExecutedName}; //!
    };
  }
}

#endif
