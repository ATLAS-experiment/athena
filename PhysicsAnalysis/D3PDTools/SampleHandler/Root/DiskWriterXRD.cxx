/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <SampleHandler/DiskWriterXRD.h>

#include <CxxUtils/hash_utils.h>
#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/ShellExec.h>
#include <TFile.h>
#include <TSystem.h>
#include <format>
#include <chrono>
#include <iostream>
#include <optional>
#include <random>
#include <sstream>
#include <stdexcept>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

//
// method implementations
//

namespace SH
{
  void DiskWriterXRD :: 
  testInvariant () const
  {
  }



  DiskWriterXRD :: 
  DiskWriterXRD (const std::string& val_path)
    : m_path (val_path)
  {
    RCU_REQUIRE (val_path.find ("root://") == 0);

    const char *tmpdir = getenv ("TMPDIR");
    std::size_t hash {0};
    CxxUtils::hash_combine (hash, std::hash<pid_t>() (getpid()));
    std::size_t tries = 0;
    while (m_file == nullptr || !m_file->IsOpen())
    {
      if (++ tries == 10)
        throw std::runtime_error ("infinite loop trying to create tempory file for DiskWriterXRD");

      auto time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
      CxxUtils::hash_combine (hash, std::hash<decltype(time)>() (time));
      std::size_t hash16 {hash};
      while (hash16 > 0xffff)
        hash16 = (hash16&0xffff) ^ (hash16 >> 16);

      std::ostringstream str;
      if (tmpdir)
	str << tmpdir;
      else
	str << "/tmp";
      str << "/SH-XRD-" << m_path.substr (m_path.rfind ("/")+1)
          << "-" << std::format("{:04x}", hash16);
      m_tmp = str.str();
      if (gSystem->AccessPathName (m_tmp.c_str()) != 0)
	m_file.reset (TFile::Open (m_tmp.c_str(), "CREATE"));
    }

    RCU_NEW_INVARIANT (this);
  }



  DiskWriterXRD :: 
  ~DiskWriterXRD ()
  {
    RCU_DESTROY_INVARIANT (this);

    if (m_file != nullptr)
    {
      try
      {
	close ();
      } catch (std::exception& e)
      {
	std::cerr << "exception closing file " << m_path << ": "
		  << e.what() << std::endl;
	
      } catch (...)
      {
	std::cerr << "unknown exception closing file " << m_path << std::endl;
      }
    }
  }



  std::string DiskWriterXRD ::
  getPath () const
  {
    RCU_READ_INVARIANT (this);
    return m_path;
  }



  TFile *DiskWriterXRD ::
  getFile ()
  {
    RCU_CHANGE_INVARIANT (this);
    RCU_REQUIRE2_SOFT (m_file != nullptr, "file already closed");
    return m_file.get();
  }



  void DiskWriterXRD ::
  doClose ()
  {
    RCU_CHANGE_INVARIANT (this);
    RCU_REQUIRE2_SOFT (m_file != nullptr, "file already closed");

    if (m_file->IsOpen())
    {
      m_file->Write ();
      if (m_file->TestBit (TFile::kWriteError))
        throw std::runtime_error ("failed to write to file: " + m_path);
      m_file->Close ();
    }

    // rationale: the RNG is only needed on the retry path, and
    //   constructing std::random_device typically opens /dev/urandom,
    //   so we construct it lazily on first use.
    std::optional<std::mt19937> gen;
    bool success = false;
    unsigned tries = 0u;
    while (!success)
    {
      try
      {
        // using the -f flag, because if this copy failed previously
        // we need to force an overwrite.  note that there would be no
        // point in leaving this out on the first try (even though
        // there should be no file there), because it would just fail
        // and retry with the flag set.
        RCU::Shell::exec ("xrdcp -f " + RCU::Shell::quote (m_tmp) + " " + RCU::Shell::quote (m_path));
        success = true;
      } catch (...)
      {
        std::cerr << "encountered error copying files to XRD path: \"" << m_path << "\"" << std::endl;
        if (tries < 10u)
        {
          tries += 1;
          if (!gen)
          {
            std::random_device rd;
            gen.emplace (rd());
          }
          // sleeping for a random period of time, to reduce the
          // chance that the problem is that multiple jobs finishing
          // at the same time keep overloading the server by
          // repeatedly hitting it at the same time.
          unsigned seconds = std::uniform_int_distribution<>(30,60) (*gen);
          std::cerr << "sleeping for " << seconds << " seconds before retrying" << std::endl;
          std::this_thread::sleep_for (std::chrono::seconds(seconds));
        } else
        {
          std::cerr << "giving up, leaving file at " << m_tmp << std::endl;
          throw std::runtime_error ("failed to copy file to XRD");
        }
      }
    }
    // rationale: the upload has succeeded, so release the file handle
    //   now.  that way a failure to remove the temporary file does not
    //   leave the writer looking un-closed and trigger a second upload
    //   from the destructor; a failed cleanup is only a warning.
    m_file.reset ();
    try
    {
      RCU::Shell::exec ("rm " + RCU::Shell::quote (m_tmp));
    } catch (std::exception& e)
    {
      std::cerr << "failed to remove temporary file " << m_tmp << ": "
                << e.what() << std::endl;
    }
  }
}
