/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <SampleHandler/SampleLocal.h>

#include <RootCoreUtils/Assert.h>
#include <TSystem.h>

//
// method implementations
//

ClassImp (SH::SampleLocal)

namespace SH
{
  void SampleLocal ::
  testInvariant () const
  {
    for (FilesIter file = m_files.begin(),
	   end = m_files.end(); file != end; ++ file)
    {
      RCU_INVARIANT (!file->empty());
      RCU_INVARIANT ((*file)[0]==0 || file->find (":/") != std::string::npos);
    }
  }



  SampleLocal ::
  SampleLocal ()
    : Sample ("unnamed")
  {
    RCU_NEW_INVARIANT (this);
  }



  SampleLocal ::
  SampleLocal (const std::string& name)
    : Sample (name)
  {
    RCU_NEW_INVARIANT (this);
  }



  void SampleLocal ::
  add (const std::string& file)
  {
    RCU_CHANGE_INVARIANT (this);
    RCU_REQUIRE_SOFT (!file.empty());

    std::string myfile = file;
    if (myfile.find (":/") == std::string::npos)
    {
      if (myfile[0] != '/')
	myfile = gSystem->WorkingDirectory() + ("/" + myfile);
      myfile = "file://" + myfile;
    }
    m_files.push_back (myfile);
  }



  std::size_t SampleLocal ::
  getNumFiles () const
  {
    RCU_READ_INVARIANT (this);
    return m_files.size();
  }



  std::string SampleLocal ::
  getFileName (const std::size_t index) const
  {
    RCU_READ_INVARIANT (this);

    RCU_INVARIANT (index < m_files.size());
    return m_files[index];
  }



  std::unique_ptr<SampleLocal> SampleLocal ::
  doMakeLocal () const
  {
    RCU_READ_INVARIANT (this);
    return std::make_unique<SampleLocal>(*this);
  }



  std::vector<std::string> SampleLocal ::
  doMakeFileList () const
  {
    RCU_READ_INVARIANT (this);

    std::vector<std::string> result;
    for (FilesIter file = m_files.begin(),
	   end = m_files.end(); file != end; ++ file)
      result.push_back (*file);
    return result;
  }



  void SampleLocal ::
  doUpdateLocation (const std::string& from, const std::string& to)
  {
    RCU_READ_INVARIANT (this);
    for (FilesMIter file = m_files.begin(),
	   end = m_files.end(); file != end; ++ file)
    {
      if (strncmp (file->c_str(), from.c_str(), from.size()) == 0)
	*file = to + "/" + file->substr (from.size());
    }
  }
}
