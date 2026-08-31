/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <RootCoreUtils/UnitTestDir.h>

#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/MessageCheck.h>

//
// method implementations
//

namespace RCU
{
  UnitTestDir ::
  UnitTestDir (const std::string& package, const std::string& name)
    : m_cleanup (getenv ("ROOTCORE_AUTO_UT") != nullptr)
  {
    std::string tmpdir = "/tmp";
    if (const char *env = getenv ("TMPDIR"))
      tmpdir = env;

    // use mkdtemp to create a unique, collision-proof directory
    const std::string templ = tmpdir + "/ut-" + package + "-" + name + "-XXXXXX";
    std::vector<char> buffer (templ.begin(), templ.end());
    buffer.push_back ('\0');
    if (mkdtemp (buffer.data()) == nullptr)
      throw std::runtime_error ("could not create output directory from template " + templ);
    m_path = buffer.data();
  }



  UnitTestDir ::
  ~UnitTestDir ()
  {
    using namespace msgRootCoreUtils;

    if (m_cleanup)
    {
      std::error_code ec;
      std::filesystem::remove_all (m_path, ec);
    }
    else
      ANA_MSG_INFO ("unit test data located at " << m_path);
  }



  const std::string& UnitTestDir ::
  path () const
  {
    return m_path;
  }



  bool UnitTestDir ::
  cleanup () const
  {
    return m_cleanup;
  }



  void UnitTestDir ::
  cleanup (bool val_cleanup)
  {
    m_cleanup = val_cleanup;
  }
}
