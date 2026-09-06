/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <SampleHandler/ToolsDuplicates.h>

#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <TChain.h>
#include <TFile.h>
#include <TLeaf.h>
#include <TTree.h>
#include <RootCoreUtils/Assert.h>
#include <SampleHandler/MessageCheck.h>
#include <SampleHandler/Sample.h>
#include <SampleHandler/SampleHandler.h>
#include <CxxUtils/checker_macros.h>

//
// method implementations
//

namespace SH
{
  namespace
  {
    /// description: the type for run-event number pairs
    ///
    /// rationale: the event number is 64-bit (ULong64_t) in the xAOD
    ///   data model, so both members are stored as ULong64_t to avoid
    ///   truncation.
    using RunEvent = std::pair<ULong64_t,ULong64_t>;

    /// description: the type for lists of run-event numbers
    using RunEventList = std::set<RunEvent>;



    /// \brief a helper to read an integer run/event-number branch of
    ///   unknown width (32- or 64-bit) into a ULong64_t via the
    ///   type-checked SetBranchAddress overload
    ///
    /// rationale: the previous code bound every branch to a 4-byte
    ///   UInt_t through a type-erasing void* cast, which silently
    ///   corrupted the stack when reading a 64-bit event-number branch.
    struct NumberBranch
    {
      ULong64_t m_value64 = 0;
      UInt_t m_value32 = 0;
      bool m_is64 = false;

      void connect (TTree& tree, const std::string& name)
      {
        TLeaf *leaf = tree.GetLeaf (name.c_str());
        if (leaf == nullptr)
          throw std::runtime_error ("failed to find leaf: " + name);
        const std::string type = leaf->GetTypeName();
        m_is64 = (type == "ULong64_t" || type == "Long64_t");
        tree.SetBranchStatus (name.c_str(), 1);
        const Int_t rc = m_is64
          ? tree.SetBranchAddress (name.c_str(), &m_value64)
          : tree.SetBranchAddress (name.c_str(), &m_value32);
        if (rc < 0)
          throw std::runtime_error ("failed to set branch address for: " + name);
      }

      ULong64_t value () const
      {
        return m_is64 ? m_value64 : ULong64_t (m_value32);
      }
    };



    /// description: the different names for run numbers
    /// guarantee: strong
    /// failures: out of memory II
    const std::set<std::string>& runNames ()
    {
      static const std::set<std::string> result =
        { "RunNumber", "runNumber" };
      return result;
    }



    /// description: the different names for event numbers
    /// guarantee: strong
    /// failures: out of memory II
    const std::set<std::string>& eventNames ()
    {
      static const std::set<std::string> result =
        { "EventNumber", "eventNumber" };
      return result;
    }



    /// effects: find which of the names is present in the n-tuple
    /// guarantee: strong
    /// failures: out of memory II
    /// failures: names not found
    std::string
    findBranch (const TTree& tree, const std::set<std::string>& names)
    {
      TTree& tree_nc ATLAS_THREAD_SAFE = const_cast<TTree&>(tree);
      TObjArray *branches = tree_nc.GetListOfBranches();

      for (const auto& name : names)
      {
	if (branches->FindObject (name.c_str()) != 0)
	  return name;
      }
      throw std::runtime_error ("failed to find branch of valid name");
    }



    /// effects: check the given tree for duplicate events and then
    ///   print them out, as well as accumulating them
    /// guarantee: basic, may print partially
    /// failures: out of memory III
    /// failures: i/o errors
    void printDuplicateEvents (TTree& tree, RunEventList& list)
    {
      using namespace msgDuplicates;

      const Long64_t nentries = tree.GetEntries();
      if (nentries < 0)
        throw std::runtime_error ("failed to read number of events from n-tuple");
      if (nentries == 0)
	return;

      const std::string runName = findBranch (tree, runNames());
      const std::string eventName = findBranch (tree, eventNames());

      tree.SetBranchStatus ("*", 0);
      NumberBranch run;
      run.connect (tree, runName);
      NumberBranch event;
      event.connect (tree, eventName);

      tree.SetCacheSize (10 * 1024 * 1024);
      for (Long64_t entry = 0;
	   entry < nentries; ++ entry)
      {
	if (tree.GetEntry (entry) < 0)
	  throw std::runtime_error ("failed to read event");
	RunEvent runEvent (run.value(), event.value());
	if (list.find (runEvent) == list.end())
	{
	  list.insert (runEvent);
	} else
	{
	  ANA_MSG_WARNING ("duplicate event run=" << run.value() << " event=" << event.value() << " file=" << tree.GetCurrentFile()->GetName());
	}
      }
    }
  }



  void printDuplicateEvents (const Sample& sample)
  {
    RunEventList list;
    std::unique_ptr<TChain> chain (sample.makeTChain ());
    printDuplicateEvents (*chain, list);
  }



  void printDuplicateEventsSplit (const SampleHandler& sh)
  {
    for (auto *sample : sh)
    {
      printDuplicateEvents (*sample);
    }
  }



  void printDuplicateEventsJoint (const SampleHandler& sh)
  {
    RunEventList list;
    for (auto *sample : sh)
    {
      std::unique_ptr<TChain> chain (sample->makeTChain ());
      printDuplicateEvents (*chain, list);
    }
  }
}
