/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <RootCoreUtils/hadd.h>

#include <sstream>
#include <TFileMerger.h>
#include <TList.h>
#include <TSystem.h>
#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/MessageCheck.h>
#include <filesystem>
#include <stdexcept>

//
// method implementations
//

namespace RCU
{
  void hadd (const std::string& output_file,
	     const std::vector<std::string>& input_files,
	     unsigned max_files)
  {
    using namespace msgRootCoreUtils;

    if (input_files.size() == 1)
    {
      // if there is only one input file, create a symlink instead of merging
      std::filesystem::create_symlink (input_files.front(), output_file);
      return;
    }

    TFileMerger merger (false, false);

    merger.SetMsgPrefix ("rcu_hadd");
    merger.SetPrintLevel (98);

    if (max_files > 0)
    {
      merger.SetMaxOpenedFiles (max_files);
    }

    if (!merger.OutputFile (output_file.c_str(), false, 1) )
    {
      throw std::runtime_error ("error opening target file: " + output_file);
    }

    for (const std::string& input : input_files)
    {
      if (!merger.AddFile (input.c_str()))
      {
        throw std::runtime_error ("error adding input file: " + input);
      }
    }
    merger.SetNotrees (false);

    bool status = merger.Merge();

    if (status)
    {
      ANA_MSG_INFO ("merged " << merger.GetMergeList()->GetEntries() << " input files into " << output_file);
    } else
    {
      throw std::runtime_error ("hadd failure during the merge");
    }
  }
}
