/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <EventLoop/Worker.h>

#include <AnaAlgorithm/IAlgorithmWrapper.h>
#include <EventLoop/AlgorithmMemoryModule.h>
#include <EventLoop/AlgorithmStateModule.h>
#include <EventLoop/AlgorithmTimerModule.h>
#include <EventLoop/BatchInputModule.h>
#include <EventLoop/BatchJob.h>
#include <EventLoop/BatchSample.h>
#include <EventLoop/BatchSegment.h>
#include <EventLoop/DirectInputModule.h>
#include <EventLoop/Driver.h>
#include <EventLoop/EventCountModule.h>
#include <EventLoop/EventRange.h>
#include <EventLoop/FactoryPreloadModule.h>
#include <EventLoop/FileExecutedModule.h>
#include <EventLoop/GridReportingModule.h>
#include <EventLoop/Job.h>
#include <EventLoop/LeakCheckModule.h>
#include <EventLoop/MemoryMonitorModule.h>
#include <EventLoop/MessageCheck.h>
#include <EventLoop/OutputStream.h>
#include <EventLoop/OutputStreamData.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/StopwatchModule.h>
#include <EventLoop/PostClosedOutputsModule.h>
#include <EventLoop/TEventModule.h>
#include <EventLoop/WorkerConfigModule.h>
#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/RootUtils.h>
#include <RootCoreUtils/ThrowMsg.h>
#include <RootUtils/WithRootErrorHandler.h>
#include <SampleHandler/DiskOutput.h>
#include <SampleHandler/DiskWriter.h>
#include <SampleHandler/MetaFields.h>
#include <SampleHandler/MetaObject.h>
#include <SampleHandler/Sample.h>
#include <SampleHandler/SamplePtr.h>
#include <SampleHandler/ToolsOther.h>
#include <TFile.h>
#include <TH1.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TTree.h>
#include <TObjString.h>
#include <fstream>
#include <memory>
#include <exception>

//
// method implementations
//

namespace EL
{
  void Worker ::
  testInvariant () const
  {
    RCU_INVARIANT (this != nullptr);
    for (std::size_t iter = 0, end = m_algs.size(); iter != end; ++ iter)
    {
      RCU_INVARIANT (m_algs[iter].m_algorithm != nullptr);
    }
  }



  Worker ::
  ~Worker ()
  {
    RCU_DESTROY_INVARIANT (this);
  }



  void Worker ::
  addOutput (TObject *output_swallow)
  {
    std::unique_ptr<TObject> output (output_swallow);

    RCU_CHANGE_INVARIANT (this);
    RCU_REQUIRE_SOFT (output_swallow != 0);

    RCU::SetDirectory (output_swallow, 0);
    ModuleData::addOutput (std::move (output));
  }



  void Worker ::
  addOutputList (const std::string& name, TObject *output_swallow)
  {
    std::unique_ptr<TObject> output (output_swallow);

    RCU_CHANGE_INVARIANT (this);
    RCU_REQUIRE_SOFT (output_swallow != 0);

    RCU::SetDirectory (output_swallow, 0);
    std::unique_ptr<TList> list (new TList);
    list->SetName (name.c_str());
    list->Add (output.release());
    addOutput (list.release());
  }



  TObject *Worker ::
  getOutputHist (const std::string& name) const
  {
    RCU_READ_INVARIANT (this);

    TObject *result = m_histOutput->getOutputHist (name);
    if (result == nullptr) RCU_THROW_MSG ("unknown output histogram: " + name);
    return result;
  }



  TFile *Worker ::
  getOutputFile (const std::string& label) const
  {
    RCU_READ_INVARIANT (this);
    TFile *result = getOutputFileNull (label);
    if (result == 0)
      RCU_THROW_MSG ("no output dataset defined with label: " + label);
    return result;
  }



  TFile *Worker ::
  getOutputFileNull (const std::string& label) const
  {
    RCU_READ_INVARIANT (this);
    auto iter = m_outputs.find (label);
    if (iter == m_outputs.end())
      return 0;
    return iter->second->file();
  }



  ::StatusCode Worker::
  addTree( const TTree& tree, const std::string& stream )
  {
    using namespace msgEventLoop;
    RCU_READ_INVARIANT( this );

    auto outputIter = m_outputs.find (stream);
    if (outputIter == m_outputs.end())
    {
      ANA_MSG_ERROR ( "No output file with stream name \"" + stream +
                      "\" found" );
      return ::StatusCode::FAILURE;
    }

    outputIter->second->addClone (tree);

    // Return gracefully:
    return ::StatusCode::SUCCESS;
  }



  TTree *Worker::
  getOutputTree( const std::string& name, const std::string& stream ) const
  {
    using namespace msgEventLoop;
    RCU_READ_INVARIANT( this );

    auto outputIter = m_outputs.find (stream);
    if (outputIter == m_outputs.end())
    {
      RCU_THROW_MSG ( "No output file with stream name \"" + stream
                      + "\" found" );
    }

    TTree *result = outputIter->second->getOutputTree( name );
    if( result == nullptr ) {
      RCU_THROW_MSG ( "No tree with name \"" + name + "\" in stream \"" +
                      stream + "\"" );
    }
    return result;
  }



  const SH::MetaObject *Worker ::
  metaData () const
  {
    RCU_READ_INVARIANT (this);
    return m_metaData;
  }



  TTree *Worker ::
  tree () const
  {
    RCU_READ_INVARIANT (this);
    return m_inputTree;
  }



  Long64_t Worker ::
  treeEntry () const
  {
    RCU_READ_INVARIANT (this);
    return m_inputTreeEntry;
  }



  TFile *Worker ::
  inputFile () const
  {
    RCU_READ_INVARIANT (this);
    return m_inputFile.get();
  }



  std::string Worker ::
  inputFileName () const
  {
    // no invariant used
    std::string path = inputFile()->GetName();
    auto split = path.rfind ('/');
    if (split != std::string::npos)
      return path.substr (split + 1);
    else
      return path;
  }



  TTree *Worker ::
  triggerConfig () const
  {
    RCU_READ_INVARIANT (this);
    return dynamic_cast<TTree*>(inputFile()->Get("physicsMeta/TrigConfTree"));
  }



  xAOD::TEvent *Worker ::
  xaodEvent () const
  {
    RCU_READ_INVARIANT (this);

    if (m_tevent == nullptr)
      RCU_THROW_MSG ("Job not configured for xAOD support");
    return m_tevent;
  }



  xAOD::TStore *Worker ::
  xaodStore () const
  {
    RCU_READ_INVARIANT (this);

    if (m_tstore == nullptr)
      RCU_THROW_MSG ("Job not configured for xAOD support");
    return m_tstore;
  }



  Algorithm *Worker ::
  getAlg (const std::string& name) const
  {
    RCU_READ_INVARIANT (this);
    for (auto& alg : m_algs)
    {
      if (alg->hasName (name))
	return alg.m_algorithm->getLegacyAlg();
    }
    return 0;
  }



  void Worker ::
  skipEvent ()
  {
    RCU_CHANGE_INVARIANT (this);
    m_skipEvent = true;
  }



  bool Worker ::
  filterPassed () const noexcept
  {
    RCU_READ_INVARIANT (this);
    return !m_skipEvent;
  }



  void Worker ::
  setFilterPassed (bool val_filterPassed) noexcept
  {
    RCU_CHANGE_INVARIANT (this);
    m_skipEvent = !val_filterPassed;
  }



  Worker ::
  Worker ()
  {
    m_worker = this;

    RCU_NEW_INVARIANT (this);
  }



  void Worker ::
  setMetaData (const SH::MetaObject *val_metaData)
  {
    RCU_CHANGE_INVARIANT (this);
    RCU_REQUIRE (val_metaData != 0);

    m_metaData = val_metaData;
  }



  void Worker ::
  setOutputHist (const std::string& val_outputTarget)
  {
    RCU_CHANGE_INVARIANT (this);

    m_outputTarget = val_outputTarget;
  }



  void Worker ::
  setSegmentName (const std::string& val_segmentName)
  {
    RCU_CHANGE_INVARIANT (this);

    m_segmentName = val_segmentName;
  }



  void Worker ::
  setJobConfig (JobConfig&& jobConfig)
  {
    RCU_CHANGE_INVARIANT (this);
    for (std::unique_ptr<IAlgorithmWrapper>& alg : jobConfig.extractAlgorithms())
    {
      m_algs.push_back (std::move (alg));
    }
  }



  ::StatusCode Worker ::
  initialize ()
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    const bool xAODInput = m_metaData->castBool (Job::optXAODInput, false);

    ANA_MSG_INFO ("xAODInput = " << xAODInput);
    if (metaData()->castBool (Job::optAlgorithmMemoryMonitor, false))
      m_modules.push_back (std::make_unique<Detail::MemoryMonitorModule> ("EarlyMemoryMonitorModule"));
    if (xAODInput)
      m_modules.push_back (std::make_unique<Detail::TEventModule> ("TEventModule"));
    auto factoryPreload = metaData()->castString (Job::optFactoryPreload, "");
    if (!factoryPreload.empty())
    {
      auto module = std::make_unique<Detail::FactoryPreloadModule> ("FactoryPreloadModule");
      module->preloader = factoryPreload;
      m_modules.push_back (std::move (module));
    }
    m_modules.push_back (std::make_unique<Detail::LeakCheckModule> ("LeakCheckModule"));
    m_modules.push_back (std::make_unique<Detail::StopwatchModule> ("StopwatchModule"));
    if (metaData()->castBool (Job::optGridReporting, false))
      m_modules.push_back (std::make_unique<Detail::GridReportingModule>("GridReportingModule"));
    if (metaData()->castBool (Job::optAlgorithmTimer, false))
      m_modules.push_back (std::make_unique<Detail::AlgorithmTimerModule> ("AlgorithmTimerModule"));
    if (metaData()->castBool (Job::optAlgorithmMemoryMonitor, false))
      m_modules.push_back (std::make_unique<Detail::AlgorithmMemoryModule> ("AlgorithmMemoryModule"));
    m_modules.push_back (std::make_unique<Detail::FileExecutedModule> ("FileExecutedModule"));
    m_modules.push_back (std::make_unique<Detail::EventCountModule> ("EventCountModule"));
    m_modules.push_back (std::make_unique<Detail::WorkerConfigModule> ("WorkerConfigModule"));
    m_modules.push_back (std::make_unique<Detail::AlgorithmStateModule> ("AlgorithmStateModule"));
    m_modules.push_back (std::make_unique<Detail::PostClosedOutputsModule> ("PostClosedOutputsModule"));
    if (metaData()->castBool (Job::optAlgorithmMemoryMonitor, false))
      m_modules.push_back (std::make_unique<Detail::MemoryMonitorModule> ("LateMemoryMonitorModule"));

    if (m_outputs.find (Job::histogramStreamName) == m_outputs.end())
    {
      Detail::OutputStreamData data {
        m_outputTarget + "/hist-" + m_segmentName + ".root", "RECREATE"};
      ANA_CHECK (addOutputStream (Job::histogramStreamName, std::move (data)));
    }
    RCU_ASSERT (m_outputs.find (Job::histogramStreamName) != m_outputs.end());
    m_histOutput = m_outputs.at(Job::histogramStreamName).get();
    if (auto aliases = metaData()->castString (Job::optStreamAliases, ""); !aliases.empty())
    {
      // the format of aliases is "alias1=realname1,alias2=realname2"
      std::istringstream iss (aliases);
      std::string alias;
      while (std::getline (iss, alias, ','))
      {
        auto pos = alias.find ('=');
        if (pos == std::string::npos)
        {
          ANA_MSG_ERROR ("Invalid alias format: " << alias);
          return ::StatusCode::FAILURE;
        }
        auto aliasName = alias.substr (0, pos);
        auto realName = alias.substr (pos + 1);
        auto realOutput = m_outputs.find (realName);
        if (realOutput == m_outputs.end())
        {
          ANA_MSG_ERROR ("output stream " << realName << " not found for alias " << aliasName);
          return ::StatusCode::FAILURE;
        }
        auto [aliasOutput, success] = m_outputs.emplace (aliasName, realOutput->second);
        if (!success)
        {
          ANA_MSG_ERROR ("output stream " << aliasName << " already exists, can't make alias");
          return ::StatusCode::FAILURE;
        }
      }
    }

    m_jobStats = std::make_unique<TTree>
      ("EventLoop_JobStats", "EventLoop job statistics");
    m_jobStats->SetDirectory (nullptr);

    ANA_MSG_INFO ("calling firstInitialize on all modules");
    for (auto& module : m_modules)
      ANA_CHECK (module->firstInitialize (*this));
    ANA_MSG_INFO ("calling preFileInitialize on all modules");
    for (auto& module : m_modules)
      ANA_CHECK (module->preFileInitialize (*this));
    
    return ::StatusCode::SUCCESS;
  }



  ::StatusCode Worker ::
  processInputs ()
  {
    using namespace msgEventLoop;

    RCU_CHANGE_INVARIANT (this);

    for (auto& module : m_modules)
      ANA_CHECK (module->processInputs (*this, *this));
    return ::StatusCode::SUCCESS;
  }



  ::StatusCode Worker ::
  finalize ()
  {
    using namespace msgEventLoop;

    RCU_CHANGE_INVARIANT (this);

    if (m_algorithmsInitialized == false)
    {
      ANA_MSG_ERROR ("algorithms never got initialized");
      return StatusCode::FAILURE;
    }

    ANA_CHECK (openInputFile (""));
    for (auto& module : m_modules)
      ANA_CHECK (module->onFinalize (*this));
    for (auto& output : m_outputs)
    {
      if (output.first != Job::histogramStreamName && output.second->mainStreamName() == output.first)
      {
        output.second->saveOutput ();
        output.second->close ();
        std::string path = output.second->finalFileName ();
        if (!path.empty())
          addOutputList ("EventLoop_OutputStream_" + output.first, new TObjString (path.c_str()));
      }
    }
    for (auto& module : m_modules)
      ANA_CHECK (module->postFinalize (*this));
    if (m_jobStats->GetListOfBranches()->GetEntries() > 0)
    {
      if (m_jobStats->Fill() <= 0)
      {
        ANA_MSG_ERROR ("failed to fill the job statistics tree");
        return ::StatusCode::FAILURE;
      }
      ModuleData::addOutput (std::move (m_jobStats));
    }
    m_histOutput->saveOutput ();
    for (auto& module : m_modules)
      ANA_CHECK (module->onWorkerEnd (*this));
    m_histOutput->saveOutput ();
    m_histOutput->close ();

    for (auto& module : m_modules){
      ANA_CHECK (module->postFileClose(*this));
    }
    ANA_MSG_INFO ("worker finished successfully");
    return ::StatusCode::SUCCESS;
  }



  ::StatusCode Worker ::
  processEvents (EventRange& eventRange)
  {
    using namespace msgEventLoop;

    RCU_CHANGE_INVARIANT (this);
    RCU_REQUIRE (!eventRange.m_url.empty());
    RCU_REQUIRE (eventRange.m_beginEvent >= 0);
    RCU_REQUIRE (eventRange.m_endEvent == EventRange::eof || eventRange.m_endEvent >= eventRange.m_beginEvent);

    ANA_CHECK (openInputFile (eventRange.m_url));

    if (eventRange.m_beginEvent > inputFileNumEntries())
    {
      ANA_MSG_ERROR ("first event (" << eventRange.m_beginEvent << ") points beyond last event in file (" << inputFileNumEntries() << ")");
      return ::StatusCode::FAILURE;
    }
    if (eventRange.m_endEvent == EventRange::eof)
    {
      eventRange.m_endEvent = inputFileNumEntries();
    } else if (eventRange.m_endEvent > inputFileNumEntries())
    {
      ANA_MSG_ERROR ("end event (" << eventRange.m_endEvent << ") points beyond last event in file (" << inputFileNumEntries() << ")");
      return ::StatusCode::FAILURE;
    }

    m_inputTreeEntry = eventRange.m_beginEvent;

    if (m_algorithmsInitialized == false)
    {
      for (auto& module : m_modules)
        ANA_CHECK (module->onInitialize (*this));
      m_algorithmsInitialized = true;
    }

    if (m_newInputFile)
    {
      m_newInputFile = false;
      for (auto& module : m_modules)
        ANA_CHECK (module->onNewInputFile (*this));
    }

    if (eventRange.m_beginEvent == 0)
    {
      for (auto& module : m_modules)
        ANA_CHECK (module->onFileExecute (*this));
    }

    ANA_MSG_INFO ("Processing events " << eventRange.m_beginEvent << "-" << eventRange.m_endEvent << " in file " << eventRange.m_url);

    for (uint64_t event = eventRange.m_beginEvent;
         event != uint64_t (eventRange.m_endEvent);
         ++ event)
    {
      m_inputTreeEntry = event;
      for (auto& module : m_modules)
      {
        if (module->onExecute (*this).isFailure())
        {
          ANA_MSG_ERROR ("processing event " << treeEntry() << " on file " << inputFileName());
          return ::StatusCode::FAILURE;
        }
      }
      if (m_firstEvent)
      {
        m_firstEvent = false;
        for (auto& module : m_modules)
          ANA_CHECK (module->postFirstEvent (*this));
      }
      m_eventsProcessed += 1;
      if (m_eventsProcessed % 10000 == 0)
        ANA_MSG_INFO ("Processed " << m_eventsProcessed << " events");
    }
    return ::StatusCode::SUCCESS;
  }



  bool Worker ::
  fileOpenErrorFilter(int level, bool /*b1*/, const char* s1, const char * s2)
  {
    // Don't fail on missing dictionary messages.
    if (strstr (s2, "no streamer or dictionary") != nullptr) {
      return true;
    }

    // For messages above warning level (SysError, Error, Fatal)
    if( level > kWarning ) {
      // We won't output further; ROOT should have already put something in the log file
      std::string msg = "ROOT error detected in Worker.cxx: ";
      msg += s1;
      msg += " ";
      msg += s2;
      throw std::runtime_error(msg);

      // No need for further error handling
      return false;
    }

    // Pass to the default error handlers
    return true;
  }

  ::StatusCode Worker ::
  openInputFile (const std::string& inputFileUrl)
  {
    using namespace msgEventLoop;

    // Enable custom error handling in a nice way
    RootUtils::WithRootErrorHandler my_handler( fileOpenErrorFilter );

    RCU_CHANGE_INVARIANT (this);

    if (m_inputFileUrl == inputFileUrl)
      return ::StatusCode::SUCCESS;

    if (!m_inputFileUrl.empty())
    {
      if (m_newInputFile == false)
      {
        for (auto& module : m_modules)
          ANA_CHECK (module->onCloseInputFile (*this));
        for (auto& module : m_modules)
          ANA_CHECK (module->postCloseInputFile (*this));
      }
      m_newInputFile = false;
      m_inputTree = nullptr;
      m_inputFile.reset ();
      m_inputFileUrl.clear ();
    }

    if (inputFileUrl.empty())
      return ::StatusCode::SUCCESS;

    ANA_MSG_INFO ("Opening file " << inputFileUrl);
    std::unique_ptr<TFile> inputFile;
    try
    {
      inputFile = SH::openFile (inputFileUrl, *metaData());
    } catch (...)
    {
      Detail::report_exception (std::current_exception());
    }
    if (inputFile.get() == 0)
    {
      ANA_MSG_ERROR ("failed to open file " << inputFileUrl);
      for (auto& module : m_modules)
        module->reportInputFailure (*this);
      return ::StatusCode::FAILURE;
    }
    if (inputFile->IsZombie())
    {
      ANA_MSG_ERROR ("input file is a zombie: " << inputFileUrl);
      for (auto& module : m_modules)
        module->reportInputFailure (*this);
      return ::StatusCode::FAILURE;
    }

    TTree *tree = 0;
    const std::string treeName
      = m_metaData->castString (SH::MetaFields::treeName, SH::MetaFields::treeName_default);
    tree = dynamic_cast<TTree*>(inputFile->Get (treeName.c_str()));
    if (tree == nullptr)
    {
      ANA_MSG_INFO ("tree " << treeName << " not found in input file: " << inputFileUrl);
      ANA_MSG_INFO ("treating this like a tree with no events");
    }

    m_newInputFile = true;
    m_inputTree = tree;
    m_inputTreeEntry = 0;
    m_inputFile = std::move (inputFile);
    m_inputFileUrl = std::move (inputFileUrl);

    return ::StatusCode::SUCCESS;
  }



  ::StatusCode Worker ::
  addOutputStream (const std::string& label,
                   Detail::OutputStreamData data)
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    if (m_outputs.find (label) != m_outputs.end())
    {
      ANA_MSG_ERROR ("output file already defined for label: " + label);
      return ::StatusCode::FAILURE;
    }
    if (data.file() == nullptr)
    {
      ANA_MSG_ERROR ("output stream does not have a file attached");
      return ::StatusCode::FAILURE;
    }
    if (data.mainStreamName().empty())
      data.setMainStreamName (label);
    m_outputs.insert (std::make_pair (label, std::make_shared<Detail::OutputStreamData>(std::move (data))));
    return ::StatusCode::SUCCESS;
  }



  Long64_t Worker ::
  inputFileNumEntries () const
  {
    RCU_READ_INVARIANT (this);
    RCU_REQUIRE (inputFile() != 0);

    if (m_inputTree != 0)
      return m_inputTree->GetEntries();
    else
      return 0;
  }



  uint64_t Worker ::
  eventsProcessed () const noexcept
  {
    RCU_READ_INVARIANT (this);
    return m_eventsProcessed;
  }



  void Worker ::
  addModule (std::unique_ptr<Detail::Module> module)
  {
    RCU_CHANGE_INVARIANT (this);
    RCU_REQUIRE (module != nullptr);
    m_modules.push_back (std::move (module));
  }



  ::StatusCode Worker ::
  directExecute (const SH::SamplePtr& sample, const Job& job,
                 const std::string& location, const SH::MetaObject& options)
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    SH::MetaObject meta (*sample->meta());
    meta.fetchDefaults (options);

    setMetaData (&meta);
    setOutputHist (location);
    setSegmentName (sample->name());

    ANA_MSG_INFO ("Running sample: " << sample->name());

    setJobConfig (JobConfig (job.jobConfig()));

    for (Job::outputIter out = job.outputBegin(),
           end = job.outputEnd(); out != end; ++ out)
    {
      Detail::OutputStreamData data {
        out->output()->makeWriter (sample->name(), "", ".root")};
      ANA_CHECK (addOutputStream (out->label(), std::move (data)));
    }

    {
      auto module = std::make_unique<Detail::DirectInputModule> ("DirectInputModule");
      module->fileList = sample->makeFileList();
      Long64_t maxEvents = metaData()->castDouble (Job::optMaxEvents, -1);
      if (maxEvents != -1)
        module->maxEvents = maxEvents;
      Long64_t skipEvents = metaData()->castDouble (Job::optSkipEvents, 0);
      if (skipEvents != 0)
        module->skipEvents = skipEvents;
      addModule (std::move (module));
    }

    ANA_CHECK (initialize ());
    ANA_CHECK (processInputs ());
    ANA_CHECK (finalize ());
    return ::StatusCode::SUCCESS;
  }



  ::StatusCode Worker ::
  batchExecute (unsigned job_id, const char *confFile)
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    try
    {
      std::unique_ptr<TFile> file (TFile::Open (confFile, "READ"));
      if (file.get() == nullptr || file->IsZombie())
      {
        ANA_MSG_ERROR ("failed to open file: " << confFile);
        return ::StatusCode::FAILURE;
      }

      std::unique_ptr<BatchJob> job (dynamic_cast<BatchJob*>(file->Get ("job")));
      if (job.get() == nullptr)
      {
        ANA_MSG_ERROR ("failed to retrieve BatchJob object");
        return ::StatusCode::FAILURE;
      }

      if (job_id >= job->segments.size())
      {
        ANA_MSG_ERROR ("invalid job-id " << job_id << ", max is " << job->segments.size());
        return ::StatusCode::FAILURE;
      }
      BatchSegment *segment = &job->segments[job_id];
      RCU_ASSERT (segment->job_id == job_id);
      RCU_ASSERT (segment->sample < job->samples.size());
      BatchSample *sample = &job->samples[segment->sample];

      gSystem->Exec ("pwd");
      gSystem->MakeDirectory ("output");

      setMetaData (&sample->meta);
      setOutputHist (job->location + "/fetch");
      setSegmentName (segment->fullName);

      setJobConfig (JobConfig (job->job.jobConfig()));

      for (Job::outputIter out = job->job.outputBegin(),
             end = job->job.outputEnd(); out != end; ++ out)
      {
        Detail::OutputStreamData data {
          out->output()->makeWriter (segment->sampleName, segment->segmentName, ".root")};
        ANA_CHECK (addOutputStream (out->label(), std::move (data)));
      }

      {
        auto module = std::make_unique<Detail::BatchInputModule> ("BatchInputModule");
        Long64_t maxEvents = metaData()->castDouble (Job::optMaxEvents, -1);
        if (maxEvents != -1)
          module->maxEvents = maxEvents;
        module->sample = sample;
        module->segment = segment;
        addModule (std::move (module));
      }

      ANA_CHECK (initialize ());
      ANA_CHECK (processInputs ());
      ANA_CHECK (finalize ());

      std::ostringstream job_name;
      job_name << job_id;
      std::ofstream completed ((job->location + "/status/completed-" + job_name.str()).c_str());
      return ::StatusCode::SUCCESS;
    } catch (...)
    {
      Detail::report_exception (std::current_exception());
      return ::StatusCode::FAILURE;
    }
  }



  ::StatusCode Worker ::
  gridExecute (const std::string& sampleName, Long64_t SkipEvents, Long64_t nEventsPerJob)
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    ANA_MSG_INFO ("Running with ROOT version " << gROOT->GetVersion()
                  << " (" << gROOT->GetVersionDate() << ")");

    ANA_MSG_INFO ("Loading EventLoop grid job");


    TList bigOutputs;
    std::unique_ptr<JobConfig> jobConfig;
    SH::MetaObject *mo = 0;

    std::unique_ptr<TFile> f (TFile::Open("jobdef.root"));
    if (f == nullptr || f->IsZombie()) {
      ANA_MSG_ERROR ("Could not read jobdef");
      return ::StatusCode::FAILURE;
    }

    mo = dynamic_cast<SH::MetaObject*>(f->Get(sampleName.c_str()));
    if (!mo)
      mo = dynamic_cast<SH::MetaObject*>(f->Get("defaultMetaObject"));
    if (!mo) {
      ANA_MSG_ERROR ("Could not read in sample meta object");
      return ::StatusCode::FAILURE;
    }

    jobConfig.reset (dynamic_cast<JobConfig*>(f->Get("jobConfig")));
    if (jobConfig == nullptr)
    {
      ANA_MSG_ERROR ("failed to read jobConfig object");
      return ::StatusCode::FAILURE;
    }

    {
      std::unique_ptr<TList> outs ((TList*)f->Get("outputs"));
      if (outs == nullptr)
      {
        ANA_MSG_ERROR ("Could not read list of outputs");
        return ::StatusCode::FAILURE;
      }

      TIter itr(outs.get());
      TObject *obj = 0;
      while ((obj = itr())) {
        EL::OutputStream * out = dynamic_cast<EL::OutputStream*>(obj);
        if (out) {
          bigOutputs.Add(out);
        }
        else {
          ANA_MSG_ERROR ("Encountered unexpected entry in list of outputs");
          return ::StatusCode::FAILURE;
        }
      }
    }

    f->Close();
    f.reset ();

    const std::string location = ".";

    mo->setBool (Job::optGridReporting, true);
    setMetaData (mo);
    setOutputHist (location);
    setSegmentName ("output");

    ANA_MSG_INFO ("Starting EventLoop Grid worker");

    {//Create and register the "big" output files with base class
      TIter itr(&bigOutputs);
      TObject *obj = 0;
      while ((obj = itr())) {
        EL::OutputStream *os = dynamic_cast<EL::OutputStream*>(obj);
        if (os == nullptr)
        {
          ANA_MSG_ERROR ("Bad input");
          return ::StatusCode::FAILURE;
        }
        {
          Detail::OutputStreamData data {
            location + "/" + os->label() + ".root", "RECREATE"};
          ANA_CHECK (addOutputStream (os->label(), std::move (data)));
        }
      }
    }

    setJobConfig (std::move (*jobConfig));

    {
      auto module = std::make_unique<Detail::DirectInputModule> ("DirectInputModule");
      std::ifstream infile("input.txt");
      while (infile) {
        std::string sLine;
        if (!getline(infile, sLine)) break;
        std::istringstream ssLine(sLine);
        while (ssLine) {
          std::string sFile;
          if (!getline(ssLine, sFile, ',')) break;
          module->fileList.push_back(sFile);
        }
      }
      if (module->fileList.size() == 0) {
        ANA_MSG_ERROR ("no input files provided");
        //User was expecting input after all.
        gSystem->Exit(EC_BADINPUT);
      }

      if (nEventsPerJob != -1)
        module->maxEvents = nEventsPerJob;
      if (SkipEvents != 0)
        module->skipEvents = SkipEvents;
      addModule (std::move (module));
    }

    ANA_CHECK (initialize());
    ANA_CHECK (processInputs ());
    ANA_CHECK (finalize ());

    int nEvents = eventsProcessed();
    ANA_MSG_INFO ("Loop finished.");
    ANA_MSG_INFO ("Read/processed " << nEvents << " events.");

    ANA_MSG_INFO ("EventLoop Grid worker finished");
    ANA_MSG_INFO ("Saving output");
    return ::StatusCode::SUCCESS;
  }
}
