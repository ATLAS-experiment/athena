/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Alexander Madsen
/// @author Nils Krumnack



#include <EventLoopGrid/PrunDriver.h>
#include <EventLoop/Algorithm.h>
#include <EventLoop/ManagerData.h>
#include <EventLoop/ManagerStep.h>
#include <EventLoop/Job.h>
#include <EventLoop/MessageCheck.h>
#include <EventLoop/OutputStream.h>
#include <PathResolver/PathResolver.h>
#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/hadd.h>
#include <RootCoreUtils/ShellExec.h>
#include <SampleHandler/MetaObject.h>
#include <SampleHandler/Sample.h>
#include <SampleHandler/SampleGrid.h>
#include <SampleHandler/SampleHandler.h>
#include <SampleHandler/GridTools.h>


#include <TList.h>
#include <TPython.h>
#include <TROOT.h>
#include <TFile.h>
#include <TSystem.h>

#include <algorithm>
#include <any>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

#include <ranges>

#include "pool.h"
#include <mutex>

ClassImp(EL::PrunDriver)

namespace {
  namespace JobState {  
    static const unsigned int NSTATES = 6;
    enum Enum { INIT=0, RUN=1, DOWNLOAD=2, MERGE=3, FINISHED=4, FAILED=5 };
    static const char* name[NSTATES] = 
      { "INIT", "RUNNING", "DOWNLOAD", "MERGE", "FINISHED", "FAILED" };
    Enum parse(const std::string& what)
    {
      for (unsigned int i = 0; i != NSTATES; ++i) {
	if (what == name[i]) { return static_cast<Enum>(i); }
      }
      throw std::runtime_error("PrunDriver.cxx: Failed to parse job state string");
    }
  }

  // When changing the values in the enum make sure
  // corresponding values in `data/ELG_jediState.py` script
  // are changed accordingly
  namespace Status {
    enum Enum { DONE=0, PENDING=1, FAIL=2 };
  }

  struct TransitionRule {
    JobState::Enum fromState;
    Status::Enum status;
    JobState::Enum toState;
  };

  struct TmpCd {
    const std::string origDir;
    TmpCd(const std::string & dir)
      : origDir(gSystem->pwd())
    {
      gSystem->cd(dir.c_str());
    }
    ~TmpCd()
    {
      gSystem->cd(origDir.c_str());
    }
  };
}

static JobState::Enum sampleState(SH::Sample* sample)
{
  RCU_REQUIRE(sample);
  static const std::string defaultState = JobState::name[JobState::INIT];
  std::string label = sample->meta()->castString ("nc_ELG_state", defaultState, SH::MetaObject::CAST_NOCAST_DEFAULT);
  return JobState::parse(label);
}

static JobState::Enum nextState(JobState::Enum state, Status::Enum status)  
{
  RCU_REQUIRE(state != JobState::FINISHED);
  RCU_REQUIRE(state != JobState::FAILED);
  static constexpr std::array<TransitionRule, 12> TABLE =
    {{
      {JobState::INIT,     Status::DONE,    JobState::RUN},
      {JobState::INIT,     Status::PENDING, JobState::INIT},
      {JobState::INIT,     Status::FAIL,    JobState::FAILED},
      {JobState::RUN,      Status::DONE,    JobState::DOWNLOAD},
      {JobState::RUN,      Status::PENDING, JobState::RUN},
      {JobState::RUN,      Status::FAIL,    JobState::FAILED},
      {JobState::DOWNLOAD, Status::DONE,    JobState::MERGE},
      {JobState::DOWNLOAD, Status::PENDING, JobState::DOWNLOAD},
      {JobState::DOWNLOAD, Status::FAIL,    JobState::FAILED},
      {JobState::MERGE,    Status::DONE,    JobState::FINISHED},
      {JobState::MERGE,    Status::PENDING, JobState::MERGE},
      {JobState::MERGE,    Status::FAIL,    JobState::DOWNLOAD}
    }};
  for (const TransitionRule& rule : TABLE) {
    if (rule.fromState == state && rule.status == status) {
      return rule.toState;
    }
  }
  throw std::logic_error("PrunDriver.cxx: Missing state transition rule");
}

static SH::MetaObject defaultOpts()
{
  SH::MetaObject o;
  o.setString("nc_nGBPerJob", "MAX");
  o.setString("nc_mergeOutput", "true");
  o.setString("nc_cmtConfig", gSystem->ExpandPathName("$AnalysisBase_PLATFORM"));
  o.setString("nc_useAthenaPackages", "true");
  const std::string mergestr = "elg_merge jobdef.root %OUT %IN";
  o.setString("nc_mergeScript", mergestr);
  return o;
}

// Serialize message output from the worker threads.  In standalone builds the
// EL::msgEventLoop MsgStream is a single shared std::ostringstream with no
// internal locking (and the printer writes to std::cout unguarded), so
// concurrent ANA_MSG_* calls from the download/run threads would otherwise race
// on that shared buffer.
static std::mutex& logMutex()
{
  static std::mutex mutex;
  return mutex;
}

static bool downloadContainer(const std::string& name,
			      const std::string& location)
{
  using namespace EL::msgEventLoop;
  RCU_ASSERT(not name.empty());
  RCU_ASSERT(name[name.size()-1] == '/');
  RCU_ASSERT(not location.empty());

  try {
    std::error_code ec;
    std::filesystem::create_directories(location, ec);
    if (ec) {
      std::lock_guard<std::mutex> lock(logMutex());
      ANA_MSG_ERROR("Failed to create directory " << location << ": " << ec.message());
      return false;
    }

    std::vector<std::string> datasets;
    for (auto& entry : SH::rucioListDids (name))
    {
      if (entry.type == "CONTAINER" || entry.type == "DIDType.CONTAINER")
        datasets.push_back (entry.name);
    }

    auto downloadResult = SH::rucioDownloadList (location, datasets);
    for (const auto& result : downloadResult)
    {
      if (result.notDownloaded != 0)
        return false;
    }
  } catch (const std::exception& e) {
    std::lock_guard<std::mutex> lock(logMutex());
    ANA_MSG_ERROR("Failed to download " << name << ": " << e.what());
    return false;
  }
  return true;
}

// Call the Python function @c func (defined in the macro file @c macroFile,
// loaded once on first use) on @c sample and return its integer result.
//
// The whole macro-load / bind / exec / unbind sequence is serialized under a
// single mutex.  @c ELG_SAMPLE is one global Python name shared by every
// caller, and the underlying CPython interpreter must be entered with the GIL
// held, so without this lock concurrent callers (e.g. the RUN-state polling
// threads) would clobber each other's binding and query the wrong sample's
// status.  Serializing is cheap here since these calls only poll task state.
// The alternative would be to pass the needed metadata as function arguments
// rather than through a global binding.
static int callPythonOnSample(const char* macroFile, const char* func,
                              SH::Sample* sample)
{
  static std::mutex mutex;
  std::lock_guard<std::mutex> lock(mutex);

  static std::set<std::string> loadedMacros;
  if (loadedMacros.insert(macroFile).second) {
    TPython::LoadMacro(PathResolverFindCalibFile(macroFile).c_str());
  }

  TPython::Bind(sample, "ELG_SAMPLE");
  std::any result;
  const std::string code =
    std::string("_anyresult = ROOT.std.make_any['int'](") + func + "(ELG_SAMPLE))";
  TPython::Exec(code.c_str(), &result);
  TPython::Bind(nullptr, "ELG_SAMPLE");
  return std::any_cast<int>(result);
}

static Status::Enum submit(SH::Sample* const sample, const bool isFirstSample)
{
  RCU_REQUIRE(sample);
  using namespace EL::msgEventLoop;

  ANA_MSG_INFO( "Submitting " << sample->name() << "..." );

  int ret = callPythonOnSample("EventLoopGrid/ELG_prun.py", "ELG_prun", sample);

  // Tarball is created for the first sample to be submitted
  // then the tarball is simply reused for the other samples 
  // If the returned value is 1 it implies the tarball creation failed 
  // See EventLoopGrid/data/ELG_prun.py script
  // Abort any further processing as the tarball was not succesfully created
  if (isFirstSample && ret == 1){
    ANA_MSG_ERROR("Failed to create tarball");
    throw std::runtime_error("PrunDriver.cxx: aborting due to tarball creation issue"); 
  }

  if (ret < 100) {
    sample->meta()->setString("nc_ELG_state_details", 
                              "problem submitting"); 
    return Status::FAIL;
  }

  sample->meta()->setDouble("nc_jediTaskID", ret);

  // Let's also tell people about their task ID
  ANA_MSG_INFO( "Task submitted; jediTaskID=" << ret );

  return Status::DONE;
}

static Status::Enum checkPandaTask(SH::Sample* const sample)
{
  RCU_REQUIRE(sample);
  RCU_REQUIRE(static_cast<int>(sample->meta()->castDouble("nc_jediTaskID",0, SH::MetaObject::CAST_NOCAST_DEFAULT)) > 100);

  int ret = callPythonOnSample("EventLoopGrid/ELG_jediState.py", "ELG_jediState", sample);

  if (ret == Status::DONE) return Status::DONE;
  if (ret == Status::FAIL) return Status::FAIL;

  // Value 90 corresponds to `running` state of the job
  if (ret != 90) { sample->meta()->setString("nc_ELG_state_details", "task status other than done/finished/failed/running"); }
  // Value 99 is returned if there is error in the script (import, missing ID)
  if (ret == 99) {
    sample->meta()->setString("nc_ELG_state_details",
                              "problem checking jedi task status");
  }

  return Status::PENDING;
}

static Status::Enum download(SH::Sample* const sample)
{
  RCU_REQUIRE(sample);
  using namespace EL::msgEventLoop;

  // This can run on several download threads at once; the shared message stream
  // is not thread-safe, so serialize the logging (see logMutex()).
  {
    std::lock_guard<std::mutex> lock(logMutex());
    ANA_MSG_INFO("Downloading output from: " << sample->name() << "...");
  }

  std::string container = sample->meta()->castString("nc_outDS", "", SH::MetaObject::CAST_NOCAST_DEFAULT);
  RCU_ASSERT(not container.empty());
  if (container[container.size()-1] == '/') {
    container.resize(container.size() - 1);
  }
  container += "_hist/";

  bool downloadOk = downloadContainer(container, "elg/download/" + container);

  if (not downloadOk) {
    std::lock_guard<std::mutex> lock(logMutex());
    ANA_MSG_ERROR("Failed to download one or more files");
    sample->meta()->setString("nc_ELG_state_details",
                              "error, check log for details");
    return Status::PENDING;
  }

  return Status::DONE;
}

static Status::Enum merge(SH::Sample* const sample)
{
  RCU_REQUIRE(sample);
  // The MERGE state is always processed single-threaded, so the logging here
  // needs no serialization (unlike download(), see logMutex()).
  using namespace EL::msgEventLoop;

  std::string container = sample->meta()->castString("nc_outDS", "", SH::MetaObject::CAST_NOCAST_DEFAULT);
  RCU_ASSERT(not container.empty());
  if (container[container.size()-1] == '/') {
    container.resize(container.size() - 1);
  }
  container += "_hist/";
  const std::string dir = "elg/download/" + container;

  const std::string fileName = "hist-output.root";

  const std::string target = Form("hist-%s.root", sample->name().c_str());

  // Collect the per-job output files, i.e. those whose name contains
  // ".hist-output.root" (matching the old `find -name "*.hist-output.root*"`).
  namespace fs = std::filesystem;
  const std::string needle = "." + fileName;
  std::vector<std::string> files;
  std::error_code ec;
  for (fs::recursive_directory_iterator it(dir, ec), end; it != end; it.increment(ec)) {
    if (ec) { break; }
    if (it->is_regular_file() &&
        it->path().filename().string().find(needle) != std::string::npos) {
      files.push_back(it->path().string());
    }
  }

  std::sort(files.begin(), files.end());
  // Duplicates are not expected (the directory scan never lists a file twice),
  // but drop them with a real runtime check rather than a side-effecting assert.
  std::vector<std::string> duplicates;
  for (size_t i = 1; i < files.size(); ++i) {
    if (files[i] == files[i - 1]) { duplicates.push_back(files[i]); }
  }
  if (not duplicates.empty()) {
    std::ostringstream dup;
    for (const std::string& name : duplicates) { dup << ' ' << name; }
    ANA_MSG_WARNING("Ignoring duplicate input file(s) for merging:" << dup.str());
    files.erase(std::unique(files.begin(), files.end()), files.end());
  }

  if (not files.size()) {
    ANA_MSG_ERROR("Found no input files for merging! "
		  "Requeueing sample for download...");
    sample->meta()->setString("nc_ELG_state_details", "retry, files were lost");
    return Status::FAIL;
  }

  try {
    RCU::hadd(target.c_str(), files);
  } catch (...) {
    sample->meta()->setString("nc_ELG_state_details",
                              "error, check log for details");
    fs::remove(target, ec);
    return Status::PENDING;
  }

  // Remove the merged inputs and the (now empty) download tree.
  for (const std::string& file : files) {
    fs::remove(file, ec);
    if (ec) {
      ANA_MSG_WARNING("Failed to remove merged input " << file << ": " << ec.message());
    }
  }
  fs::remove_all(dir, ec);
  if (ec) {
    ANA_MSG_WARNING("Failed to remove download directory " << dir << ": " << ec.message());
  }

  return Status::DONE;
}

static void processTask(SH::Sample* const sample, const bool isFirstSample)
{
  RCU_REQUIRE(sample);

  JobState::Enum state = sampleState(sample);
  
  sample->meta()->setString("nc_ELG_state_details", "");

  Status::Enum status = Status::PENDING;
  switch (state) {
  case JobState::INIT: 
    status = submit(sample, isFirstSample);
    break;
  case JobState::RUN: 
    status = checkPandaTask(sample);
    break;
  case JobState::DOWNLOAD: 
    status = download(sample);
    break;
  case JobState::MERGE: 
    status = merge(sample);
    break;
  case JobState::FINISHED: 
  case JobState::FAILED:
    break;
  }

  state = nextState(state, status);
  sample->meta()->setString("nc_ELG_state", JobState::name[state]);
}

static void processAllInState(const SH::SampleHandler& sh, JobState::Enum state,
                              const size_t nThreads)
{
  RCU_REQUIRE(sh.size());

  WorkList workList;

  bool isFirstSample = true;
  for (SH::Sample* const sample : sh) {
    if (sampleState(sample) == state) {
      workList.push_back([sample, isFirstSample, state]()->void{
        if (state == JobState::INIT) {
          // INIT is always processed single-threaded, so let an exception
          // (e.g. the deliberate tarball-creation abort in submit()) propagate
          // and stop the submission.
          processTask(sample, isFirstSample);
        } else {
          // On a worker thread an escaping exception would call std::terminate.
          // Record it against the sample and mark it FAILED so the pool
          // finishes processing the remaining samples.
          try {
            processTask(sample, isFirstSample);
          } catch (const std::exception& e) {
            using namespace EL::msgEventLoop;
            {
              std::lock_guard<std::mutex> lock(logMutex());
              ANA_MSG_ERROR ("Exception while processing " << sample->name()
                             << ": " << e.what());
            }
            sample->meta()->setString ("nc_ELG_state_details",
                                       std::string ("exception: ") + e.what());
            sample->meta()->setString ("nc_ELG_state",
                                       JobState::name[JobState::FAILED]);
          }
        }
      });
      // Change boolean to false as already processed one sample
      isFirstSample = false;
    }
  }
  process(workList, nThreads);
}

// Look up the grid nickname via panda's PsubUtils, going through TPython (which
// is already used for submission) instead of spawning a python subprocess.
// Returns nullopt when the nickname cannot be determined (e.g. no valid proxy
// yet), so the caller can leave %nickname% in the output pattern for the
// python side to substitute later.  Only a successful lookup is cached, so a
// proxy created later in the same process is still picked up.
static std::optional<std::string> gridNickname()
{
  static std::optional<std::string> cached;
  if (cached.has_value()) { return cached; }

  std::any result;
  const char* code =
    "try:\n"
    "    from pandatools import PsubUtils\n"
    "    _nick = str(PsubUtils.getNickname())\n"
    "except Exception:\n"
    "    _nick = ''\n"
    "_anyresult = ROOT.std.make_any['std::string'](_nick)\n";
  TPython::Exec(code, &result);
  const std::string nickname = std::any_cast<std::string>(result);

  // An empty result means the lookup failed; a very long one is panda's
  // "no proxy" message rather than an actual nickname.
  if (nickname.empty() || nickname.length() > 20) { return std::nullopt; }
  cached = nickname;
  return cached;
}

static std::string formatOutputName(const SH::MetaObject& sampleMeta,
				    const std::string & pattern)
{
  const std::string sampleName = sampleMeta.castString("sample_name");
  RCU_REQUIRE(not pattern.empty());
  using namespace EL::msgEventLoop;

  TString out = pattern.c_str();

  // Handle case of no proxy; will create a proxy later in the submission
  const std::optional<std::string> nickname = gridNickname();
  if (not nickname.has_value()){
    ANA_MSG_WARNING( "No proxy available - cannot use nickname yet. Will try a late replacement.");
  } else {
    out.ReplaceAll("%nickname%", *nickname);
  }

  out.ReplaceAll("%in:name%", sampleName);

  std::stringstream ss(sampleName);
  std::string item;
  int field = 0;
  while(std::getline(ss, item, '.')) {
    std::stringstream sskey;
    sskey << "%in:name[" << ++field << "]%";
    out.ReplaceAll(sskey.str(), item);
  }
  while (out.Index("%in:") != -1) {
    int i1 = out.Index("%in:");
    int i2 = out.Index("%", i1+1);
    if (i2 == -1) {
      ANA_MSG_ERROR("malformed output name pattern, unterminated %in: token in \""
		    << out.Data() << "\"");
      break;
    }
    TString metaName = out(i1+4, i2-i1-4);
    out.ReplaceAll("%in:"+metaName+"%",
		   sampleMeta.castString(std::string(metaName.Data())));
  }
  out.ReplaceAll("/", "");
  return out.Data();
}

static std::string outputFileNames(const EL::Job& job)
{
  std::string out = "hist:hist-output.root";
  for (EL::Job::outputIter os = job.outputBegin(),
	 end = job.outputEnd(); os != end; ++os) {
    const std::string name = os->label() + ".root";
    const std::string ds =
      os->options()->castString(EL::OutputStream::optContainerSuffix);
    out += "," + (ds.empty() ? name : ds + ":" + name);
  }
  return out;
}

// Save algortihms and lists of inputs and outputs to a root file
static void saveJobDef(const std::string& fileName,
		       const EL::Job& job,
		       const SH::SampleHandler& sh)
{
  TFile file(fileName.c_str(), "RECREATE");
  TList outputs;
  outputs.SetOwner(true);
  for (EL::Job::outputIter o = job.outputBegin(); o !=job.outputEnd(); ++o)
    outputs.Add(o->Clone());
  file.WriteTObject(&job.jobConfig(), "jobConfig", "SingleKey");        
  file.WriteTObject(&outputs, "outputs", "SingleKey");        
  bool haveDefault = false;
  for (SH::Sample* const sample : sh) {
    const SH::MetaObject& meta = *(sample->meta());
    file.WriteObject(&meta, meta.castString("sample_name").c_str());
    if (!haveDefault)
    {
      file.WriteObject (&meta, "defaultMetaObject");
      haveDefault = true;
    }
  }
}  

// Create a sample handler with grid locations of outputs with given label
static SH::SampleHandler outputSH(const SH::SampleHandler& in,
				  const std::string& outputLabel)
{
  SH::SampleHandler out;
  const std::string outputFile = "*" + outputLabel + ".root*";
  const std::string outDSSuffix = '_' + outputLabel + ".root/"; 
  for (SH::Sample* const sample : in) {
    auto outSample = std::make_unique<SH::SampleGrid>(sample->name());
    const std::string outputDS = sample->meta()->castString("nc_outDS", "", SH::MetaObject::CAST_NOCAST_DEFAULT) + outDSSuffix;
    outSample->meta()->setString("nc_grid", outputDS);
    outSample->meta()->setString("nc_grid_filter", outputFile);
    out.add(std::move(outSample));
  }
  out.fetch(in);
  return out;
}

void EL::PrunDriver::testInvariant() const 
{}

EL::PrunDriver::PrunDriver() 
{
  RCU_NEW_INVARIANT(this);
}

::StatusCode EL::PrunDriver ::
doManagerStep (Detail::ManagerData& data) const
{
  using namespace msgEventLoop;
  ANA_CHECK (Driver::doManagerStep (data));
  switch (data.step)
  {
  case Detail::ManagerStep::submitJob:
    {
      const std::string jobELGDir = data.submitDir + "/elg";
      const std::string runShFile = jobELGDir + "/runjob.sh";
      const std::string mergeShFile = jobELGDir + "/elg_merge";
      const std::string runShOrig = PathResolverFindCalibFile("EventLoopGrid/runjob.sh");
      const std::string mergeShOrig = PathResolverFindCalibFile("EventLoopGrid/elg_merge");

      const std::string jobDefFile = jobELGDir + "/jobdef.root";

      namespace fs = std::filesystem;
      std::error_code ec;
      fs::create_directories(jobELGDir, ec);
      if (ec) {
        ANA_MSG_ERROR("could not create directory " << jobELGDir << ": " << ec.message());
        return StatusCode::FAILURE;
      }
      // Copy the grid scripts into the submission directory and make them
      // executable, aborting submission if either step fails.
      const auto copyExecutable =
        [&] (const std::string& from, const std::string& to) -> StatusCode {
          std::error_code ec2;
          fs::copy_file(from, to, fs::copy_options::overwrite_existing, ec2);
          if (ec2) {
            ANA_MSG_ERROR("could not copy " << from << " to " << to << ": " << ec2.message());
            return StatusCode::FAILURE;
          }
          fs::permissions(to, fs::perms::owner_exec | fs::perms::group_exec |
                          fs::perms::others_exec, fs::perm_options::add, ec2);
          if (ec2) {
            ANA_MSG_ERROR("could not make " << to << " executable: " << ec2.message());
            return StatusCode::FAILURE;
          }
          return StatusCode::SUCCESS;
        };
      ANA_CHECK(copyExecutable(runShOrig, runShFile));
      ANA_CHECK(copyExecutable(mergeShOrig, mergeShFile));

      // create symbolic links for additionnal files/directories if any to ship to the grid 
      std::string listToShipToGrid = data.options.castString(EL::Job::optGridPrunShipAdditionalFilesOrDirs, ""); 
      // parse the list of comma separated files and/or directories to ship to the grid 
      if (listToShipToGrid.size()){
        ANA_MSG_INFO (
          "Creating symbolic links for additional files or directories to be sent to grid.\n"
          "For root or heavy files you should also add their name (not the full path) to EL::Job::optUserFiles.\n"
          "Otherwise prun ignores those files."
        );

        std::vector<std::string> vect_filesOrDirToShip;
        for (auto&& part : std::views::split(listToShipToGrid, ',')) vect_filesOrDirToShip.emplace_back(part.begin(), part.end());
        // Create symbolic links of files or directories to the submission directory
        for (const std::string & fileOrDirToShip: vect_filesOrDirToShip){
          ANA_MSG_INFO (("Creating symbolic link for: " +fileOrDirToShip).c_str());
          const fs::path linkPath =
            fs::path(jobELGDir) / fs::path(fileOrDirToShip).filename();
          // emulate `ln -sf`: replace any pre-existing link/file
          fs::remove(linkPath, ec);
          fs::create_symlink(fileOrDirToShip, linkPath, ec);
          if (ec) {
            ANA_MSG_ERROR("could not create symbolic link " << linkPath.string()
                          << " -> " << fileOrDirToShip << ": " << ec.message());
            return StatusCode::FAILURE;
          }
        }
        ANA_MSG_INFO ("Finished creation of symbolic links");
      }

      const SH::SampleHandler& sh = data.job->sampleHandler();

      for (SH::Sample* const sample : sh) {
        SH::MetaObject& meta = *sample->meta();
        meta.fetchDefaults(data.options);
        meta.fetchDefaults(defaultOpts());
        meta.setString("nc_outputs", outputFileNames(*data.job));
        std::string outputSampleName = meta.castString("nc_outputSampleName");
        if (outputSampleName.empty()) {
          outputSampleName = "user.%nickname%.%in:name%";
        }
        meta.setString("nc_outDS", formatOutputName(meta, outputSampleName));
        meta.setString("nc_inDS", meta.castString("nc_grid", sample->name()));
        meta.setString("nc_writeInputToTxt", "IN:input.txt");
        meta.setString("nc_match", meta.castString("nc_grid_filter"));
        const std::string execstr = "runjob.sh " + sample->name();
        meta.setString("nc_exec", execstr);
        meta.setString("nc_framework", "EventLoopGrid");
      }

      saveJobDef(jobDefFile, *data.job, sh);
  
      for (EL::Job::outputIter out = data.job->outputBegin();
           out != data.job->outputEnd(); ++out) {
        SH::SampleHandler shOut = outputSH(sh, out->label());
        shOut.save(data.submitDir + "/output-" + out->label());
      }
      SH::SampleHandler shHist = outputSH(sh, "hist-output");
      shHist.save(data.submitDir + "/output-hist");
 
      TmpCd keepDir(jobELGDir);

      processAllInState(sh, JobState::INIT, 0); 

      sh.save(data.submitDir + "/input");
      data.submitted = true;
    }
    break;

  case Detail::ManagerStep::doRetrieve:
    {
      ANA_CHECK (doRetrieve (data));
    }
    break;

  default:
    (void) true; // safe to do nothing
  }
  return ::StatusCode::SUCCESS;
}

::StatusCode EL::PrunDriver::doRetrieve (Detail::ManagerData& data) const 
{
  RCU_READ_INVARIANT(this);
  RCU_REQUIRE(not data.submitDir.empty());  

  TmpCd tmpDir(data.submitDir);

  SH::SampleHandler sh;
  sh.load("input");
  RCU_ASSERT(sh.size());

  const size_t nRunThreads = options()->castDouble("nc_run_threads", 0); 
  const size_t nDlThreads = options()->castDouble("nc_download_threads", 0); 
  processAllInState(sh, JobState::INIT, 0); 
  processAllInState(sh, JobState::RUN, nRunThreads);
  processAllInState(sh, JobState::DOWNLOAD, nDlThreads); 
  processAllInState(sh, JobState::MERGE, 0); 

  sh.save("input");

  std::cout << std::endl;

  bool allDone = true;
  for (SH::Sample* const sample : sh) {
    JobState::Enum state = sampleState(sample);
    std::string details = sample->meta()->castString("nc_ELG_state_details", "", SH::MetaObject::CAST_NOCAST_DEFAULT);
    if (not details.empty()) { details = '(' + details + ')'; }

    std::cout << sample->name() << "\t";
    switch (state) {
    case JobState::INIT:
    case JobState::RUN:
    case JobState::DOWNLOAD:
    case JobState::MERGE:
      std::cout << JobState::name[state] << "\t";
      break;
    case JobState::FINISHED:
      std::cout << "\033[1;32m" << JobState::name[state] << "\033[0m\t";
      break;
    case JobState::FAILED:
      std::cout << "\033[1;31m" << JobState::name[state] << "\033[0m\t";
      break;
    }
    std::cout << details << std::endl;
    
    allDone &= (state == JobState::FINISHED || state == JobState::FAILED);
  }

  std::cout << std::endl;
  
  data.retrieved = true;
  data.completed = allDone;
  return ::StatusCode::SUCCESS;
}
 
void EL::PrunDriver::status(const std::string& location)
{
  RCU_REQUIRE(not location.empty());  
  TmpCd tmpDir(location);
  SH::SampleHandler sh;
  sh.load("input");
  RCU_ASSERT(sh.size());
  processAllInState(sh, JobState::RUN, 0); 
  sh.save("input");
  for (SH::Sample* const sample : sh) {
    JobState::Enum state = sampleState(sample);
    std::string details = sample->meta()->castString("nc_ELG_state_details", "", SH::MetaObject::CAST_NOCAST_DEFAULT);
    if (not details.empty()) { details = '(' + details + ')'; }
    std::cout << sample->name() << "\t" << JobState::name[state]
	      << "\t" << details << std::endl;
  }
}

void EL::PrunDriver::setState(const std::string& location,
			      const std::string& task,
			      const std::string& state)
{
  RCU_REQUIRE(not location.empty());  
  RCU_REQUIRE(not task.empty());  
  RCU_REQUIRE(not state.empty());  
  TmpCd tmpDir(location);
  SH::SampleHandler sh;
  sh.load("input");
  RCU_ASSERT(sh.size());
  if (not sh.get(task)) {
    std::cout << "Unknown task: " << task << std::endl;
    std::cout << "Choose one of: " << std::endl;
    sh.print();
    return;
  }
  JobState::parse(state);
  sh.get(task)->meta()->setString("nc_ELG_state", state);
  sh.save("input");
}
