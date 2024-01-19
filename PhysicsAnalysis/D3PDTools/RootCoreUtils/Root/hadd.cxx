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
#include <RootCoreUtils/PrintMsg.h>
#include <filesystem>

//
// method implementations
//

namespace RCU
{
  void hadd (const std::string& output_file,
	     const std::vector<std::string>& input_files,
	     unsigned max_files)
  {
    if (input_files.size() == 1)
    {
      // if there is only one input file I just create a hard link instead of
      // bothering with the merging below.  that oughgt to save both disk space
      // and time.
      std::filesystem::create_hard_link (input_files.front(), output_file);
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
      RCU_THROW_MSG ("error opening target file: " + output_file);
    }

    for (std::vector<std::string>::const_iterator input = input_files.begin(),
	   end = input_files.end(); input != end; ++ input)
    {
      if (!merger.AddFile (input->c_str()))
      {
	RCU_THROW_MSG ("error adding input file: " + *input);
      }         
    }
    merger.SetNotrees (false);

    bool status = merger.Merge();

    if (status)
    {
      std::ostringstream msg;
      msg << "merged " << merger.GetMergeList()->GetEntries()
	  << " input files into " << output_file;
      RCU_PRINT_MSG (msg.str());
    } else
    {
      RCU_THROW_MSG ("hadd failure during the merge");
    }
  }
}
