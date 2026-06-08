/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef ROOT_CORE_UTILS__SHELL_EXEC_H
#define ROOT_CORE_UTILS__SHELL_EXEC_H

#include <RootCoreUtils/Global.h>

#include <string>

namespace RCU
{
  namespace Shell
  {
    /// effects: execute the given command
    /// guarantee: strong
    /// failures: out of memory II
    /// failures: system failure
    /// failures: command failure
    void exec (const std::string& cmd);


    /// effects: execute the given command and return the output
    /// returns: the output of the command
    /// guarantee: strong
    /// failures: out of memory III
    /// failures: system failure
    /// failures: command failure
    std::string exec_read (const std::string& cmd);


    /// effects: execute the given command and return the output
    /// returns: the output of the command
    /// guarantee: strong
    /// failures: out of memory III
    /// failures: system failure
    /// failures: command failure
    std::string exec_read (const std::string& cmd, int& rc);


    /// effects: quote the given name to protect it from the shell
    /// returns: the quoted name
    /// guarantee: strong
    /// failures: out of memory II
    std::string quote (const std::string& name);
  }
}

#endif
