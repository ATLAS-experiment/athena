/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <SampleHandler/SampleComposite.h>

#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/ThrowMsg.h>
#include <SampleHandler/SampleLocal.h>

//
// method implementations
//

ClassImp (SH::SampleComposite)

namespace SH
{
  void SampleComposite ::
  testInvariant () const
  {
    for (SamplesIter sample = m_samples.begin(),
	   end = m_samples.end(); sample != end; ++ sample)
    {
      RCU_INVARIANT (*sample != nullptr);
    };
  }



  SampleComposite ::
  SampleComposite ()
    : Sample ("unnamed")
  {
    RCU_NEW_INVARIANT (this);
  }



  SampleComposite ::
  SampleComposite (const std::string& name)
    : Sample (name)
  {
    RCU_NEW_INVARIANT (this);
  }



  void SampleComposite ::
  add (std::shared_ptr<Sample> sample)
  {
    RCU_CHANGE_INVARIANT (this);
    RCU_REQUIRE_SOFT (sample != nullptr);
    if (contains (sample->name()))
      RCU_THROW_MSG ("trying to add sample " + sample->name() + " to sample " + name() + ", which already contains a sample " + sample->name());
    m_samples.push_back (std::move (sample));
  }



  std::size_t SampleComposite ::
  getNumFiles () const
  {
    RCU_READ_INVARIANT (this);
    RCU_THROW_MSG ("Sample::numFiles not supported for SampleComposite");
    return 0; // compiler dummy
  }



  std::string SampleComposite ::
  getFileName (const std::size_t /*index*/) const
  {
    RCU_READ_INVARIANT (this);
    RCU_THROW_MSG ("Sample::fileName not supported for SampleComposite");
    return ""; // compiler dummy
  }



  std::vector<std::string> SampleComposite ::
  doMakeFileList () const
  {
    RCU_READ_INVARIANT (this);

    std::vector<std::string> result;
    for (SamplesIter sample = m_samples.begin(),
	   end = m_samples.end(); sample != end; ++ sample)
    {
      std::vector<std::string> subresult = (*sample)->makeFileList();
      result.insert (result.end(), subresult.begin(), subresult.end());
    }
    return result;
  }



  std::unique_ptr<SampleLocal> SampleComposite ::
  doMakeLocal () const
  {
    RCU_READ_INVARIANT (this);
    RCU_THROW_MSG ("Sample::makeLocal not supported for SampleComposite");
    return {}; // compiler dummy
  }



  void SampleComposite ::
  doUpdateLocation (const std::string& from, const std::string& to)
  {
    RCU_READ_INVARIANT (this);
    for (auto sample = m_samples.begin(),
	   end = m_samples.end(); sample != end; ++ sample)
    {
      (*sample)->updateLocation (from, to);
    }
  }



  bool SampleComposite ::
  getContains (const std::string& name) const
  {
    RCU_READ_INVARIANT (this);
    for (SamplesIter sample = m_samples.begin(),
	   end = m_samples.end(); sample != end; ++ sample)
    {
      if ((*sample)->contains (name))
	return true;
    };
    return false;
  }



  void SampleComposite ::
  doAddSamples (SampleHandler& result, const std::shared_ptr<Sample>& /*self*/)
  {
    RCU_READ_INVARIANT (this);
    for (auto& sample : m_samples)
    {
      sample->addSamples (result, sample);
    };
  }
}
