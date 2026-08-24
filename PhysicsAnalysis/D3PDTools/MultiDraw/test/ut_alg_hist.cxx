/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <MultiDraw/Global.h>

#include <exception>
#include <iostream>
#include <memory>
#include <sstream>
#include <TFile.h>
#include <TH1.h>
#include <TString.h>
#include <TSystem.h>
#include <EventLoop/DirectDriver.h>
#include <EventLoop/Job.h>
#include <MultiDraw/AlgHist.h>
#include <MultiDraw/FormulaSvc.h>
#include <RootCoreUtils/UnitTestDir.h>
#include <SampleHandler/Sample.h>
#include <SampleHandler/SampleLocal.h>
#include <memory>
#include <stdexcept>

//
// main program
//

using namespace MD;

int main ()
{
  RCU::UnitTestDir outputDir ("MultiDraw", "alg_hist");
  TString output = outputDir.path() + "submit";

  try
  {
    TString input = "$ROOTCOREBIN/data/EventLoop/test_ntuple1.root";
    gSystem->ExpandPathName (input);
    std::string tree ("physics");

    EL::DirectDriver driver;
    auto sample = std::make_shared<SH::SampleLocal> ("dataset");
    sample->add (input.Data());
    sample->meta()->setString ("nc_tree", tree);

    EL::Job job;
    {
      SH::SampleHandler sh;
      sh.add (sample);
      job.sampleHandler (sh);
    }
    job.algsAdd (new AlgHist (new TH1F ("el_n", "el_n", 10, 0, 10), "el_n"));

    gSystem->Exec (("rm -rf " + output).Data());
    driver.submit (job, output.Data());
    TFile hist_file ((output + "/hist-dataset.root").Data(), "READ");
    TH1 *hist = dynamic_cast<TH1*>(hist_file.Get ("el_n"));
    if (hist == 0)
      throw std::runtime_error ("didn't find histogram el_n");
    float content [6] = {0, 0, 1, 5, 4, 0};
    for (int bin = 0, end = 6; bin != end; ++ bin)
    {
      if (hist->GetBinContent (bin) != content[bin])
      {
	std::ostringstream str;
	str << "bin content missmatch in bin " << bin
	    << " found " << hist->GetBinContent (bin)
	    << " expected " << content[bin];
	throw std::runtime_error (str.str());
      }
    }
  } catch (std::exception& e)
  {
    std::cerr << "caught error: " << e.what() << std::endl;
    return 1;
  } catch (std::string& s)
  {
    std::cerr << "caught error: " << s << std::endl;
    return 1;
  } catch (...)
  {
    std::cerr << "caught unknown error" << std::endl;
    return 1;
  }
}
