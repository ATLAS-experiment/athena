/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <SampleHandler/ToolsSplit.h>

#include <RootCoreUtils/Assert.h>
#include <SampleHandler/MessageCheck.h>
#include <SampleHandler/MetaFields.h>
#include <SampleHandler/MetaObject.h>
#include <SampleHandler/MetaVector.h>
#include <SampleHandler/SampleHandler.h>
#include <SampleHandler/SampleLocal.h>
#include <TFile.h>
#include <TTree.h>
#include <memory>
#include <sstream>
#include <stdexcept>

//
// method implementations
//

namespace SH
{
  void scanNEvents (SampleHandler& sh)
  {
    for (auto *sample : sh)
      scanNEvents (*sample);
  }



  void scanNEvents (Sample& sample)
  {
    SampleLocal *const mysample = dynamic_cast<SampleLocal*>(&sample);
    if (!mysample)
      throw std::runtime_error ("sample not of type SampleLocal");

    const std::string tree_name = sample.meta()->castString (MetaFields::treeName, MetaFields::treeName_default);
    if (tree_name.empty())
      throw std::runtime_error ("scanNEvents: sample doesn't contain a tree name");

    Long64_t tot_entries = 0;
    std::vector<Long64_t> entries;
    const auto fileList = sample.makeFileList ();
    for (const std::string& file_name : fileList)
    {
      std::unique_ptr<TFile> file (TFile::Open (file_name.c_str(), "READ"));
      if (!file.get())
        throw std::runtime_error ("failed to open file " + file_name);
      Long64_t treeEntries = 0;
      TTree *const tree = dynamic_cast<TTree*>(file->Get (tree_name.c_str()));
      if (tree != 0)
	treeEntries = tree->GetEntries ();
      entries.push_back (treeEntries);
      tot_entries += treeEntries;
    }
    RCU_ASSERT (entries.size() == fileList.size());
    sample.meta()->addReplace (new MetaVector<Long64_t>(MetaFields::numEventsPerFile, entries));
    sample.meta()->setDouble (MetaFields::numEvents, tot_entries);
  }



  SampleHandler splitSample (Sample& sample, const Long64_t nevt)
  {
    using namespace msgSplit;

    if (!dynamic_cast<SampleLocal*>(&sample))
      throw std::runtime_error ("sample not of type SampleLocal");

    TObject *meta = sample.meta()->get (MetaFields::numEventsPerFile);
    if (!meta)
    {
      ANA_MSG_WARNING ("sample " << sample.name() << " lacks nc_nevtfile, running scanNEvents, please save sample");
      scanNEvents (sample);
      meta = sample.meta()->get (MetaFields::numEventsPerFile);
    }
    RCU_ASSERT (meta != 0);
    MetaVector<Long64_t> *const nentries
      = dynamic_cast<MetaVector<Long64_t> *>(meta);
    if (nentries == 0)
      throw std::runtime_error ("nc_nevtfile is of the wrong type");
    if (nentries->value.size() != sample.numFiles())
      throw std::runtime_error ("nc_nevtfile has the wrong number of entries");

    SampleHandler result;
    std::unique_ptr<SampleLocal> res;
    Long64_t num = 0;
    const std::string meta_tree = sample.meta()->castString (MetaFields::treeName, MetaFields::treeName_default);
    const double meta_xs = sample.meta()->castDouble (MetaFields::crossSection, 0);
    for (unsigned file = 0, end = nentries->value.size(); file != end; ++ file)
    {
      if (num > 0 && num + nentries->value[file] > nevt)
      {
	result.add (std::move (res));
	res.reset();
	num = 0;
      }
      if (res.get() == 0)
      {
	std::ostringstream name;
	name << sample.name() << "_" << result.size();
	res = std::make_unique<SampleLocal> (name.str());
	res->tags (sample.tags());
	res->meta()->fetch (*sample.meta());
	if (!meta_tree.empty())
	  res->meta()->setString (MetaFields::treeName, meta_tree);
	if (meta_xs != 0)
	  res->meta()->setDouble (MetaFields::crossSection, meta_xs);
      }
      res->add (sample.fileName (file));
      num += nentries->value[file];
    }
    if (num > 0)
      result.add (std::move (res));
    return result;
  }
}
