/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <SampleHandler/DiskWriterLocal.h>

#include <exception>
#include <iostream>
#include <stdexcept>
#include <TFile.h>
#include <TSystem.h>
#include <RootCoreUtils/Assert.h>

//
// method implementations
//

namespace SH
{
  void DiskWriterLocal :: 
  testInvariant () const
  {
  }



  DiskWriterLocal :: 
  DiskWriterLocal (const std::string& val_path)
    : m_path (val_path)
  {
    RCU_REQUIRE (!val_path.empty());

    std::string dir = gSystem->DirName (val_path.c_str());
    if (gSystem->mkdir (dir.c_str(), true) != 0 &&
        gSystem->AccessPathName (dir.c_str()) != 0)
      throw std::runtime_error ("failed to create directory: " + dir);
    m_file = std::make_unique<TFile> (val_path.c_str(), "RECREATE");
    if (m_file->IsZombie() || !m_file->IsOpen())
    {
      m_file.reset();
      throw std::runtime_error ("failed to create file: " + val_path);
    }

    RCU_NEW_INVARIANT (this);
  }



  DiskWriterLocal :: 
  ~DiskWriterLocal ()
  {
    RCU_DESTROY_INVARIANT (this);

    if (m_file)
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



  std::string DiskWriterLocal ::
  getPath () const
  {
    RCU_READ_INVARIANT (this);
    return m_path;
  }



  TFile *DiskWriterLocal ::
  getFile ()
  {
    RCU_CHANGE_INVARIANT (this);
    RCU_REQUIRE2_SOFT (m_file != nullptr, "file already closed");
    return m_file.get();
  }



  void DiskWriterLocal ::
  doClose ()
  {
    RCU_CHANGE_INVARIANT (this);
    RCU_REQUIRE2_SOFT (m_file != nullptr, "file already closed");

    m_file->Write ();
    if (m_file->TestBit (TFile::kWriteError))
      throw std::runtime_error ("failed to write to file: " + m_path);
    m_file->Close ();
    m_file.reset();
  }
}
