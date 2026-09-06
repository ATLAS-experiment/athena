/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//
// includes
//

#include <SampleHandler/fetch.h>

#include <sstream>
#include <mutex>
#include <stdexcept>
#include <RVersion.h>
#include <TPython.h>
#include <TString.h>
#include <TSystem.h>
#include <SampleHandler/MetaDataQuery.h>
#include <SampleHandler/MetaDataSample.h>
#include <SampleHandler/MetaObject.h>
#include <RootCoreUtils/Assert.h>
#include <SampleHandler/MessageCheck.h>
#include <SampleHandler/MetaFields.h>
#include <SampleHandler/Sample.h>
#include <SampleHandler/SampleHandler.h>

//
// method implementations
//

namespace
{
  /// \brief escape a string for embedding inside single quotes in
  ///   generated Python
  std::string pyQuote (const std::string& name)
  {
    std::string result = "'";
    for (char c : name)
    {
      if (c == '\\' || c == '\'')
        result += '\\';
      result += c;
    }
    result += "'";
    return result;
  }
}

namespace SH
{
  void fetchMetaData (MetaDataQuery& query)
  {
    static std::once_flag loaded;
    auto do_load = []() {
      // rationale: the helper module is installed via
      //   atlas_install_python_modules and is importable through
      //   PYTHONPATH, so import it by name rather than loading it from
      //   the dead RootCore $ROOTCOREBIN path.
      if (!TPython::Exec ("import SampleHandler.SampleHandler_QueryAMI"))
        throw std::runtime_error ("failed to import python module SampleHandler.SampleHandler_QueryAMI");
    };
    std::call_once (loaded, do_load);

    std::ostringstream command;
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,33,01)
    command << "_anyresult = ";
#endif
    command << "SampleHandler.SampleHandler_QueryAMI.SampleHandler_QueryAmi([";
    for (std::size_t iter = 0, end = query.samples.size(); iter != end; ++ iter)
    {
      if (iter != 0)
	command << ", ";
      command << pyQuote (query.samples[iter].name);
    }
    command << "])";
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,33,01)
    std::any result;
    if (!TPython::Exec (command.str().c_str(), &result))
      throw std::runtime_error ("failed to execute AMI metadata query");
    query = std::any_cast<MetaDataQuery>(result);
#else
    MetaDataQuery* myquery = static_cast<MetaDataQuery*>
      ((void*) TPython::Eval (command.str().c_str()));
    if (myquery == nullptr)
      throw std::runtime_error ("failed to execute AMI metadata query");
    query = *myquery;
#endif
  }


  void fetchMetaData (SH::SampleHandler& sh, bool override)
  {
    using namespace msgFetch;

    std::vector<SH::Sample*> samples;
    // typedef std::vector<SH::Sample*> SamplesIter;
    MetaDataQuery query;
    for (auto *sample : sh)
    {
      std::string name = sample->meta()->castString (SH::MetaFields::gridName, sample->name());
      query.samples.push_back (MetaDataSample (name));
      samples.push_back (sample);
    }
    fetchMetaData (query);

    if (!query.messages.empty())
      ANA_MSG_INFO (query.messages);
    for (std::size_t iter = 0, end = query.samples.size(); iter != end; ++ iter)
    {
      if (query.samples[iter].unknown)
      {
        ANA_MSG_WARNING ("failed to find sample " << query.samples[iter].name);
      } else
      {
	if (iter >= samples.size())
	  throw std::runtime_error ("AMI query returned more samples than were requested");
	SH::Sample *sample = samples[iter];

	if (!override)
	{
	  query.samples[iter].isData = sample->meta()->castDouble (SH::MetaFields::isData, query.samples[iter].isData);
	  query.samples[iter].luminosity = sample->meta()->castDouble (SH::MetaFields::lumi, query.samples[iter].luminosity);
	  query.samples[iter].crossSection = sample->meta()->castDouble (SH::MetaFields::crossSection, query.samples[iter].crossSection);
	  query.samples[iter].nevents = sample->meta()->castDouble (SH::MetaFields::numEvents, query.samples[iter].nevents);
	  query.samples[iter].kfactor = sample->meta()->castDouble (SH::MetaFields::kfactor, query.samples[iter].kfactor);
	  query.samples[iter].filterEfficiency = sample->meta()->castDouble (SH::MetaFields::filterEfficiency, query.samples[iter].filterEfficiency);
	}
	if (query.samples[iter].isData != -1)
	  sample->meta()->setDouble (SH::MetaFields::isData, query.samples[iter].isData);
	if (query.samples[iter].luminosity != -1)
	  sample->meta()->setDouble (SH::MetaFields::lumi, query.samples[iter].luminosity);
	if (query.samples[iter].crossSection != -1)
	  sample->meta()->setDouble (SH::MetaFields::crossSection, query.samples[iter].crossSection);
	if (query.samples[iter].nevents != -1)
	  sample->meta()->setDouble (SH::MetaFields::numEvents, query.samples[iter].nevents);
	if (query.samples[iter].kfactor != -1)
	  sample->meta()->setDouble (SH::MetaFields::kfactor, query.samples[iter].kfactor);
	if (query.samples[iter].filterEfficiency != -1)
	  sample->meta()->setDouble (SH::MetaFields::filterEfficiency, query.samples[iter].filterEfficiency);
      }
    }
  }
}
