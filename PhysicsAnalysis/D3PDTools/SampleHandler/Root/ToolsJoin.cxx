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

    std::regex mypattern (pattern.c_str());
    for (auto& sample : sh.samples())
    {
      if (RCU::match_expr (mypattern, sample->name()))
      {
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
    mysh.add (mysample);
    swap (mysh, sh);
  }
}
