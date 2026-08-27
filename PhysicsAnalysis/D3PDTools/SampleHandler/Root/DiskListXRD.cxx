/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <SampleHandler/DiskListXRD.h>

#include <sstream>
#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/ShellExec.h>
#include <SampleHandler/MessageCheck.h>

//
// method implementations
//

namespace SH
{
  void DiskListXRD :: 
  testInvariant () const
  {
  }



  DiskListXRD ::
  DiskListXRD (const std::string& val_server, const std::string& val_directory,
	       bool val_laxParser)
    : m_server (val_server), m_directory (val_directory),
      m_laxParser (val_laxParser), m_isRead (false)
  {
    RCU_NEW_INVARIANT (this);
  }



  bool DiskListXRD ::
  getNext ()
  {
    RCU_CHANGE_INVARIANT (this);
    using namespace msgScanDir;

    if (!m_isRead)
    {
      std::string command = "xrdfs " + RCU::Shell::quote (m_server) + " ls -l " + RCU::Shell::quote (m_directory);
      ANA_MSG_DEBUG ("trying XRD command: " << command);
      m_list = RCU::Shell::exec_read (command);
      ANA_MSG_DEBUG ("XRD command output:\n" << m_list);
      m_context = "command: " + command + "\n" + m_list;
      m_isRead = true;
      m_pos = 0;
    }

    // rationale: we advance a running index through m_list rather than
    //   repeatedly copying its tail, so that listing N entries costs
    //   O(N) rather than O(N^2).  a final line without a trailing
    //   newline is still processed.
    while (m_pos < m_list.size())
    {
      std::string::size_type split1 = m_list.find ('\n', m_pos);
      std::string line;
      if (split1 == std::string::npos)
      {
	line = m_list.substr (m_pos);
	m_pos = m_list.size();
      } else
      {
	line = m_list.substr (m_pos, split1 - m_pos);
	m_pos = split1 + 1;
      }
      ANA_MSG_DEBUG ("next XRD list line: " << line);

      std::string::size_type split2 = line.find ('/');
      if (split2 != std::string::npos &&
	  (line[0] == '-' || line[0] == 'd'))
      {
	m_isDir = line[0] == 'd';
	m_file = line.substr (split2);
        ANA_MSG_DEBUG ("next XRD file found: " << m_file);
        ANA_MSG_DEBUG ("XRD file isDir: " << m_isDir);
	return true;
      }

      if (!line.empty())
      {
	std::string message = "failed to parse line: \"" + line + "\"\n" + m_context;

        ANA_MSG_WARNING (message);
	if (!m_laxParser)
          throw std::runtime_error (message);
      }
    }
    return false;
  }



  std::string DiskListXRD ::
  getPath () const
  {
    RCU_READ_INVARIANT (this);
    return "root://" + m_server + "/" + m_file;
  }



  DiskList *DiskListXRD ::
  doOpenDir () const
  {
    RCU_READ_INVARIANT (this);

    if (m_file.empty() || !m_isDir)
      return nullptr;

    return new DiskListXRD (m_server, m_file, m_laxParser);
  }



  std::string DiskListXRD ::
  getDirname () const
  {
    RCU_READ_INVARIANT (this);
    return m_directory;
  }
}
