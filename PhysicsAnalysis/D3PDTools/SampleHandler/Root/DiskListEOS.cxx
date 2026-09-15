/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <SampleHandler/DiskListEOS.h>

#include <sstream>
#include <stdexcept>
#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/ShellExec.h>

//
// method implementations
//

namespace SH
{
  void DiskListEOS :: 
  testInvariant () const
  {
  }



  DiskListEOS :: 
  DiskListEOS (const std::string& val_dir)
    : m_dir (val_dir), m_prefix ("root://eosatlas.cern.ch/" + val_dir), m_isRead (false)
  {
    RCU_NEW_INVARIANT (this);
  }



  DiskListEOS ::
  DiskListEOS (const std::string& val_dir, const std::string& val_prefix)
    : m_dir (val_dir), m_prefix (val_prefix), m_isRead (false)
  {
    RCU_NEW_INVARIANT (this);
  }



  bool DiskListEOS ::
  getNext ()
  {
    RCU_CHANGE_INVARIANT (this);

    if (!m_isRead)
    {
      m_list = RCU::Shell::exec_read ("eos ls -l " + RCU::Shell::quote (m_dir));
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

      // rationale: this should handle it correctly if there is a
      //   formatting escape sequence at the beginning of the line.
      std::string::size_type split2 = line.find ('r');
      if (split2 == std::string::npos || split2 == 0)
	throw std::runtime_error ("failed to parse EOS listing line: " + line);
      m_isDir = line[split2-1] == 'd';
      line = line.substr (split2);

      // rationale: the listing has eight fixed whitespace-separated
      //   columns (permissions, links, user, group, size and three
      //   date/time fields), followed by the file name.  we skip the
      //   columns individually and then take the remainder of the line
      //   as the name, so that names containing spaces are preserved.
      std::istringstream str (line);
      std::string field;
      for (unsigned iter = 0, end = 8; iter != end; ++ iter)
      {
	if (!(str >> field))
	  throw std::runtime_error ("failed to parse EOS listing line: " + line);
      }
      std::string name;
      if (!(str >> std::ws) || !std::getline (str, name) || name.empty())
	throw std::runtime_error ("failed to parse EOS listing line: " + line);
      m_file = name;
      return true;
    }
    return false;
  }



  std::string DiskListEOS ::
  getPath () const
  {
    RCU_READ_INVARIANT (this);
    return m_prefix + "/" + m_file;
  }



  DiskList *DiskListEOS ::
  doOpenDir () const
  {
    RCU_READ_INVARIANT (this);

    if (m_file.empty() || !m_isDir)
      return nullptr;

    return new DiskListEOS (m_dir + "/" + m_file, m_prefix + "/" + m_file);
  }



  std::string DiskListEOS ::
  getDirname () const
  {
    RCU_READ_INVARIANT (this);
    return m_dir;
  }
}
