/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <SampleHandler/SampleHist.h>

#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/RootUtils.h>
#include <SampleHandler/SampleLocal.h>
#include <TFile.h>
#include <memory>
#include <stdexcept>

//
// method implementations
//

ClassImp (SH::SampleHist)

namespace SH
{
  void SampleHist ::
  testInvariant () const
  {
    RCU_INVARIANT (!m_file.empty() || name() == "unnamed");    
  }



  SampleHist ::
  SampleHist ()
    : Sample ("unnamed")
  {
    RCU_NEW_INVARIANT (this);
  }



  SampleHist ::
  SampleHist (const std::string& name, const std::string& file)
    : Sample (name), m_file (file)
  {
    RCU_NEW_INVARIANT (this);
  }



  std::size_t SampleHist ::
  getNumFiles () const
  {
    RCU_READ_INVARIANT (this);
    return 1;
  }



  std::string SampleHist ::
  getFileName (const std::size_t
#ifndef NDEBUG
	       index
#endif
	       ) const
  {
    RCU_READ_INVARIANT (this);
    RCU_REQUIRE (index == static_cast<std::size_t>(0));
    return m_file;
  }



  std::unique_ptr<SampleLocal> SampleHist ::
  doMakeLocal () const
  {
    RCU_READ_INVARIANT (this);
    throw std::runtime_error ("Sample::makeLocal not supported for SampleHist");
  }



  std::vector<std::string> SampleHist ::
  doMakeFileList () const
  {
    RCU_READ_INVARIANT (this);

    std::vector<std::string> result;
    result.push_back (m_file);
    return result;
  }



  void SampleHist ::
  doUpdateLocation (const std::string& from, const std::string& to)
  {
    RCU_CHANGE_INVARIANT (this);
    // rationale: only replace the prefix on a path boundary (so that
    //   "/a/b" does not also match "/a/bc") and join without inserting
    //   a duplicate '/'.
    if (m_file.starts_with (from) &&
	(m_file.size() == from.size() || m_file[from.size()] == '/'))
    {
      std::string rest = m_file.substr (from.size());
      if (!rest.empty() && rest.front() == '/')
	rest.erase (0, 1);
      m_file = rest.empty() ? to : to + "/" + rest;
    }
  }



  TObject *SampleHist ::
  doReadHist (const std::string& name) const
  {
    RCU_READ_INVARIANT (this);

    std::unique_ptr<TFile> file (TFile::Open (m_file.c_str(), "READ"));
    if (file.get() == nullptr)
      throw std::runtime_error ("could not open file " + m_file);
    TObject *object = file->Get (name.c_str());
    if (object != nullptr)
      RCU::SetDirectory (object, nullptr);
    return object;
  }
}
