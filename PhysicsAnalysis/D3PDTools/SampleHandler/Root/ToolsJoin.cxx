/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <SampleHandler/ToolsJoin.h>

#include <memory>
#include <RootCoreUtils/StringUtil.h>
#include <SampleHandler/MetaObject.h>
#include <SampleHandler/SampleHandler.h>
#include <SampleHandler/SampleLocal.h>

//
// method implementations
//

namespace SH
{
  void mergeSamples (SampleHandler& sh, const std::string& sampleName,
		     const std::string& pattern)
  {
    SampleHandler mysh;
    auto mysample = std::make_shared<SampleLocal> (sampleName);
    bool matched = false;

    std::regex mypattern (pattern);
    for (auto& sample : sh.samples())
    {
      if (RCU::match_expr (mypattern, sample->name()))
      {
        if (!matched)
        {
          // rationale: inherit the metadata (tree name, cross section,
          //   ...) from the first matched sample instead of falling
          //   back to the defaults.
          *mysample->meta() = *sample->meta();
          matched = true;
        }
        for (unsigned file = 0, end = sample->numFiles();
            file != end; ++ file)
        {
          mysample->add (sample->fileName (file));
        }
      } else
      {
        mysh.add (sample);
      }
    }
    // rationale: only add the merged sample if at least one sample
    //   matched, so we do not create a spurious empty sample.
    if (matched)
      mysh.add (mysample);
    swap (mysh, sh);
  }
}
