/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <SampleHandler/SampleHandler.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <TFile.h>
#include <TSystem.h>
#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/StringUtil.h>
#include <SampleHandler/DiskListLocal.h>
#include <SampleHandler/MetaFields.h>
#include <SampleHandler/MetaObject.h>
#include <SampleHandler/Sample.h>

//
// method implementations
//

ClassImp (SH::SampleHandler)

namespace SH
{
  namespace
  {
    /*
    bool isspace (const std::string& str)
    {
      for (std::string::const_iterator iter = str.begin(),
	     end = str.end(); iter != end; ++ iter)
      {
	if (!std::isspace (*iter))
	  return false;
      };
      return true;
    }
    */
  }



  std::string dbg (const SampleHandler& obj, unsigned verbosity)
  {
    std::ostringstream result;
    result << "SampleHandler with " << obj.size() << " samples";
    if (verbosity % 10 > 0)
    {
      result << "\n";
      for (auto *sample : obj)
      {
	result << dbg (*sample, verbosity / 10) << "\n";
      }
    }
    return result.str();
  }



  void swap (SampleHandler& a, SampleHandler& b)
  {
    swap (a.m_samples, b.m_samples);
    swap (a.m_named, b.m_named);
  }



  void SampleHandler ::
  testInvariant () const
  {
  }



  SampleHandler ::
  SampleHandler ()
  {
    RCU_NEW_INVARIANT (this);
  }



  SampleHandler ::
  SampleHandler (const SampleHandler& that)
    : TObject (that), m_samples (that.m_samples), m_named (that.m_named)
  {
    RCU_NEW_INVARIANT (this);
  }



  SampleHandler ::
  ~SampleHandler ()
  {
    RCU_DESTROY_INVARIANT (this);
  }



  SampleHandler& SampleHandler ::
  operator = (const SampleHandler& that)
  {
    // no invariant used
    SampleHandler tmp (that);
    swap (tmp, *this);
    return *this;
  }



  void SampleHandler ::
  add (const Sample& sample)
  {
    // no invariant used
    std::unique_ptr<Sample> copy (dynamic_cast<Sample*> (sample.Clone()));
    RCU_ASSERT (copy != nullptr);
    add (std::move (copy));
  }



  void SampleHandler ::
  add (std::shared_ptr<Sample> sample)
  {
    RCU_CHANGE_INVARIANT (this);
    RCU_REQUIRE_SOFT (sample != nullptr);
    RCU_REQUIRE_SOFT (!sample->name().empty());

    if (!sample->name().empty() && m_named.find (sample->name()) != m_named.end())
      throw std::runtime_error ("can't add sample of name " + sample->name() + "\na sample with that name already exists\nold sample:\n" + dbg (*m_named.find (sample->name())->second, 9999) + "\nnew sample:\n" + dbg (*sample, 9999));

    m_samples.push_back (sample);
    if (!sample->name().empty())
    {
      try
      {
        m_named[sample->name()] = sample;
      } catch (...)
      {
        m_samples.pop_back();
        throw;
      }
      // rationale: only lock the name once the sample has been
      //   successfully registered in both containers, so that a failed
      //   insert does not leave the sample permanently name-locked.
      sample->lockName ();
    }
  }



  void SampleHandler ::
  add (const SampleHandler& sh)
  {
    // invariant not used
    RCU_REQUIRE_SOFT (this != &sh);

    for (auto& sample : sh.m_samples)
    {
      add (sample);
    }
  }



  void SampleHandler ::
  addWithPrefix (const SampleHandler& sh, const std::string& prefix)
  {
    // invariant not used
    RCU_REQUIRE_SOFT (this != &sh);

    for (auto *source : sh)
    {
      std::unique_ptr<Sample> sample (dynamic_cast<Sample*>(source->Clone ()));
      RCU_ASSERT (sample != nullptr);
      sample->name (prefix + source->name());
      add (std::move (sample));
    }
  }



  void SampleHandler ::
  remove (const std::string& name)
  {
    // invariant not used
    const Sample *sample = get (name);
    if (sample == nullptr)
      throw std::runtime_error ("sample " + name + " not found in SampleHandler");
    remove (sample);
  }



  void SampleHandler ::
  remove (const Sample *sample)
  {
    RCU_CHANGE_INVARIANT (this);
    RCU_REQUIRE_SOFT (sample != nullptr);

    auto nameIter = m_named.find (sample->name());
    if (nameIter == m_named.end())
      throw std::runtime_error ("sample " + sample->name() + " not found in SampleHandler");
    if (nameIter->second.get() != sample)
      throw std::runtime_error ("different sample of name " + sample->name() + " found in SampleHandler");
    std::erase_if (m_samples, [sample] (const std::shared_ptr<Sample>& p) { return p.get() == sample; });
    m_named.erase (nameIter);
  }



  Sample *SampleHandler ::
  get (const std::string& name)
  {
    RCU_READ_INVARIANT (this);

    auto iter = m_named.find (name);
    if (iter != m_named.end())
      return iter->second.get();
    return nullptr;
  }


  const Sample *SampleHandler ::
  get (const std::string& name) const
  {
    RCU_READ_INVARIANT (this);

    auto iter = m_named.find (name);
    if (iter != m_named.end())
      return iter->second.get();
    return nullptr;
  }



  SampleHandler SampleHandler ::
  find (const std::string& tags) const
  {
    // no invariant used
    return find (TagList (tags, ','));
  }



  SampleHandler SampleHandler ::
  find (const TagList& tags) const
  {
    RCU_READ_INVARIANT (this);

    SampleHandler result;

    for (auto& sample : m_samples)
    {
      bool use = false;
      for (auto iter = tags.begin(),
          end = tags.end(); !use && iter != end; ++ iter)
        use = sample->tags().has (*iter);
      if (use)
        result.add (sample);
    }
    return result;
  }



  Sample *SampleHandler ::
  findBySource (const std::string& name) const
  {
    RCU_READ_INVARIANT (this);

    std::vector<Sample*> result;
    for (auto *sample : *this)
    {
      if (name == sample->meta()->castString (MetaFields::sourceSample, sample->name()))
	result.push_back (sample);
    }
    if (result.size() > 1)
    {
      std::ostringstream message;
      message << "multiple samples have " << name << " as a source:";
      for (auto *sample : result)
	message << " " << sample->name();
      throw std::runtime_error (message.str());
    }
    if (result.empty())
      return nullptr;
    return result.front();
  }



  SampleHandler SampleHandler ::
  findByName (const std::string& pattern) const
  {
    RCU_READ_INVARIANT (this);
    SampleHandler result;
    std::regex expr (pattern);
    for (auto& sample : m_samples)
    {
      if (RCU::match_expr (expr, sample->name()))
        result.add (sample);
    }
    return result;
  }



  void SampleHandler ::
  print () const
  {
    RCU_READ_INVARIANT (this);
    std::cout << dbg (*this, 9999) << std::endl;
  }



  void SampleHandler ::
  printContent () const
  {
    // not using invariant
    print ();
  }



  void SampleHandler ::
  save (const std::string& directory) const
  {
    RCU_READ_INVARIANT (this);

    // rationale: not checking the return status, since this is just a
    //   courtesy directory creation that is Ok to fail.
    gSystem->MakeDirectory (directory.c_str());
    for (auto *sample : *this)
    {
      TFile file ((directory + "/" + sample->name() + ".root").c_str(), "RECREATE");
      sample->Write ("sample");
    }
  }



  void SampleHandler ::
  load (const std::string& directory)
  {
    RCU_CHANGE_INVARIANT (this);

    DiskListLocal mydir (directory);
    while (mydir.next())
    {
      const std::string file = mydir.fileName();

      if (file.size() > 5 && file.rfind (".root") == file.size() - 5)
      {
        TFile myfile (mydir.path().c_str(), "READ");
        std::unique_ptr<Sample> sample {dynamic_cast<Sample*>(myfile.Get ("sample"))};
        if (sample != 0)
          add (std::move(sample));
      }
    }
  }



  void SampleHandler ::
  updateLocation (const std::string& from, const std::string& to)
  {
    // no invariant used
    RCU_REQUIRE_SOFT (!from.empty());
    RCU_REQUIRE_SOFT (!to.empty());
    for (auto *sample : *this)
      sample->updateLocation (from, to);
  }



  void SampleHandler ::
  fetch (const SampleHandler& source)
  {
    // invariant not used

    for (auto *sample : *this)
    {
      const std::string name
	= sample->meta()->castString (MetaFields::sourceSample, sample->name());
      const Sample *const mysource = source.get (name);
      if (mysource)
	sample->meta()->fetch (*mysource->meta());
    }
  }



  void SampleHandler ::
  fetchDefaults (const SampleHandler& source)
  {
    // invariant not used

    for (auto *sample : *this)
    {
      const std::string name
	= sample->meta()->castString (MetaFields::sourceSample, sample->name());
      const Sample *const mysource = source.get (name);
      if (mysource)
	sample->meta()->fetchDefaults (*mysource->meta());
    }
  }



  bool SampleHandler ::
  check_complete (const SampleHandler& source) const
  {
    // invariant not used

    std::set<std::string> names;
    for (auto *sample : *this)
    {
      names.insert (sample->meta()->castString (MetaFields::sourceSample, sample->name()));
    }

    for (auto *sample : source)
    {
      if (names.find (sample->name()) == names.end())
	return false;
    }
    return true;
  }



  void SampleHandler ::
  setMetaDouble (const std::string& name, double value)
  {
    // no invariant used

    for (auto *sample : *this)
    {
      sample->meta()->setDouble (name, value);
    }
  }



  void SampleHandler ::
  setMetaString (const std::string& name, const std::string& value)
  {
    // no invariant used

    for (auto *sample : *this)
    {
      sample->meta()->setString (name, value);
    }
  }



  void SampleHandler ::
  setMetaDouble (const std::string& pattern, const std::string& name,
		 double value)
  {
    // no invariant used

    std::regex mypattern (pattern);

    for (auto *sample : *this)
    {
      if (RCU::match_expr (mypattern, sample->name()))
	sample->meta()->setDouble (name, value);
    }
  }



  void SampleHandler ::
  setMetaString (const std::string& pattern, const std::string& name,
		 const std::string& value)
  {
    // no invariant used

    std::regex mypattern (pattern);

    for (auto *sample : *this)
    {
      if (RCU::match_expr (mypattern, sample->name()))
	sample->meta()->setString (name, value);
    }
  }



  Sample *SampleHandler::SamplePtrToRawSample ::
  operator () (const std::shared_ptr<Sample>& p) const
  {
    return p.get();
  }



  SampleHandler::iterator SampleHandler ::
  begin () const
  {
    RCU_READ_INVARIANT (this);
    return boost::make_transform_iterator
      (m_samples.begin(), SamplePtrToRawSample{});
  }



  SampleHandler::iterator SampleHandler ::
  end () const
  {
    RCU_READ_INVARIANT (this);
    return boost::make_transform_iterator
      (m_samples.end(), SamplePtrToRawSample{});
  }



  std::size_t SampleHandler ::
  size () const
  {
    RCU_READ_INVARIANT (this);
    return m_samples.size();
  }



  Sample *SampleHandler ::
  operator [] (std::size_t index) const
  {
    // no invariant used
    return at (index);
  }



  Sample *SampleHandler ::
  at (std::size_t index) const
  {
    RCU_READ_INVARIANT (this);
    RCU_REQUIRE_SOFT (index < size());
    return m_samples[index].get();
  }



  std::span<const std::shared_ptr<Sample>> SampleHandler ::
  samples () const
  {
    RCU_READ_INVARIANT (this);
    return std::span<const std::shared_ptr<Sample>> (m_samples.data(), m_samples.size());
  }



  void SampleHandler ::
  Streamer (TBuffer& b)
  {
    if (b.IsReading())
    {
      RCU_CHANGE_INVARIANT (this);
      SampleHandler sh;
      ULong_t count = 0;
      b.ReadULong (count);
      for (ULong_t iter = 0; iter != count; ++ iter)
      {
	Sample *sample = nullptr;
	b >> sample;
	sh.add (std::shared_ptr<Sample>(sample));
      }
      swap (*this, sh);
    } else
    {
      RCU_READ_INVARIANT (this);
      ULong_t count = m_samples.size();
      b.WriteULong (count);
      for (const auto& sample_ptr : m_samples)
      {
	Sample *sample = sample_ptr.get();
	b << sample;
      }
    }
  }
}
