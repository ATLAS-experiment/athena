/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <RootCoreUtils/ShellExec.h>

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <vector>
#include <TSystem.h>

//
// method implementations
//

namespace RCU
{
  namespace Shell
  {
    void exec (const std::string& cmd)
    {
      if (gSystem->Exec (cmd.c_str()) != 0)
        throw std::runtime_error ("command failed: " + cmd);
    }



    std::string exec_read (const std::string& cmd)
    {
      int rc = 0;
      std::string result = exec_read (cmd, rc);
      if (rc != 0)
        throw std::runtime_error ("command failed: " + cmd + "\nwith output:\n" + result);
      return result;
    }



    std::string exec_read (const std::string& cmd, int& rc)
    {
      std::unique_ptr<FILE, int (*)(FILE*)> pipe (popen (cmd.c_str(), "r"), pclose);
      if (pipe == nullptr)
        throw std::runtime_error ("failed to run command: " + cmd);

      std::string result;
      std::vector<char> buffer (1024);
      size_t read;
      while ((read = fread (&buffer[0], 1, buffer.size(), pipe.get())) > 0)
      {
	result.append (&buffer[0], read);
      }
      rc = pclose (pipe.release());
      return result;
    }



    std::string quote (const std::string& name)
    {
      // wrap the argument in single quotes, which protects every
      // character from the shell (including newlines); an embedded
      // single quote is emitted as the standard '\'' sequence
      std::string result = "'";
      for (char c : name)
      {
	if (c == '\'')
	  result += "'\\''";
	else
	  result += c;
      }
      result += "'";
      return result;
    }
  }
}
