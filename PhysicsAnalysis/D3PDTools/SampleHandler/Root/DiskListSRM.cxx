/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <SampleHandler/DiskListSRM.h>

#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/ShellExec.h>

//
// method implementations
//

namespace SH
{
  void DiskListSRM :: 
  testInvariant () const
  {
  }



  DiskListSRM :: 
  DiskListSRM (const std::string& val_dir)
    : m_dir (val_dir), m_prefix (val_dir)
  {
    RCU_NEW_INVARIANT (this);
  }



  DiskListSRM ::
  DiskListSRM (const std::string& val_dir, const std::string& val_prefix)
    : m_dir (val_dir), m_prefix (val_prefix)
  {
    RCU_NEW_INVARIANT (this);
  }



  bool DiskListSRM ::
  getNext ()
  {
    RCU_CHANGE_INVARIANT (this);

    if (!m_isRead)
    {
      m_list = RCU::Shell::exec_read ("srmls " + RCU::Shell::quote (m_dir));
      m_isRead = true;
      m_pos = 0;
      // rationale: skip the header line of the srmls output
      std::string::size_type split1 = m_list.find ('\n');
      if (split1 == std::string::npos)
	return false;
      m_pos = split1 + 1;
    }

    // rationale: we advance a running index through m_list rather than
    //   repeatedly copying its tail, so that listing N entries costs
    //   O(N) rather than O(N^2).
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
      if (line.size() > 2)
      {
	std::string::size_type split2 = line.rfind ('/', line.size()-2);
	if (split2 != std::string::npos)
	{
	  m_file = line.substr (split2 + 1);
	  if (m_file[m_file.size()-1] == '/')
	  {
	    m_isDir = true;
            m_file.resize (m_file.size()-1);
	  } else
	    m_isDir = false;
	  return true;
	}
      }
    }
    return false;
  }



  std::string DiskListSRM ::
  getPath () const
  {
    RCU_READ_INVARIANT (this);
    return m_prefix + "/" + m_file;
  }



  DiskList *DiskListSRM ::
  doOpenDir () const
  {
    RCU_READ_INVARIANT (this);

    if (m_file.empty() || !m_isDir)
      return nullptr;

    return new DiskListSRM (m_dir + "/" + m_file, m_prefix + "/" + m_file);
  }



  std::string DiskListSRM ::
  getDirname () const
  {
    RCU_READ_INVARIANT (this);
    return m_dir;
  }
}
