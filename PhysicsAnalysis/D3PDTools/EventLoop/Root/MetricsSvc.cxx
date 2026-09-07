/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <EventLoop/MetricsSvc.h>
#include <RootCoreUtils/Assert.h>
#include <TBenchmark.h>
#include <TFile.h>
#include <TTree.h>
#include <TTreeCache.h>

ClassImp(EL::MetricsSvc)


namespace EL
{
  const std::string MetricsSvc::name = "Metrics";
}

void EL::MetricsSvc :: testInvariant () const
{}

const char *EL::MetricsSvc :: GetName() const
{
  RCU_READ_INVARIANT (this);
  return name.c_str();
}


EL::MetricsSvc :: MetricsSvc ()
  : m_fileMetrics (0), m_jobMetrics (0), m_filesRead (0)
  , m_eventsRead (0), m_benchmark (0)
{
  RCU_NEW_INVARIANT (this);
}



EL::MetricsSvc :: ~MetricsSvc ()
{
  RCU_DESTROY_INVARIANT (this);
  if (m_benchmark) { delete m_benchmark; }
}



EL::StatusCode EL::MetricsSvc :: histInitialize ()
{ 
  RCU_CHANGE_INVARIANT (this);
  m_benchmark = new TBenchmark;
  m_benchmark->Start("loopmetrics");
  wk()->addOutput (m_fileMetrics = new TTree ("EventLoop_Metrics/cacheStats",
					      "TTree cache stats per file"));
  wk()->addOutput (m_jobMetrics = new TTree ("EventLoop_Metrics/jobs",
					     "event throughput per job"));

  // Create the per-file branches once, pointing at the member buffers, so
  // that the branch addresses stay valid across every per-file Fill().
  m_fileMetrics->Branch ("nBranches", &m_nBranches, "nBranches/I");
  m_fileMetrics->Branch ("learnEntries", &m_nLearn, "learnEntries/I");
  m_fileMetrics->Branch ("cacheEfficiency", &m_cacheEfficiency, "cacheEfficiency/D");
  m_fileMetrics->Branch ("cacheEfficiencyRel", &m_cacheEfficiencyRel, "cacheEfficiencyRel/D");
  m_fileMetrics->Branch ("bytesRead", &m_bytesRead, "bytesRead/L");
  m_fileMetrics->Branch ("readCalls", &m_readCalls, "readCalls/I");
  m_fileMetrics->Branch ("noCacheBytesRead", &m_noCacheBytesRead, "noCacheBytesRead/L");
  m_fileMetrics->Branch ("noCacheReadCalls", &m_noCacheReadCalls, "noCacheReadCalls/I");
  m_fileMetrics->Branch ("readaheadSize", &m_readaheadSize, "readaheadSize/I");
  m_fileMetrics->Branch ("bytesReadExtra", &m_bytesReadExtra, "bytesReadExtra/L");
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EL::MetricsSvc :: fileExecute ()
{
  RCU_CHANGE_INVARIANT (this);
  m_filesRead ++;
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EL::MetricsSvc :: endOfFile ()
{
  RCU_CHANGE_INVARIANT (this);
  RCU_ASSERT(m_fileMetrics); 
  RCU_ASSERT(wk()->inputFile()); 
  RCU_ASSERT(wk()->tree()); 
  TTreeCache *tc = (TTreeCache*)wk()->inputFile()->GetCacheRead(wk()->tree());
  if (tc) {
    // update the member buffers wired to the branches in histInitialize
    m_nBranches = tc->GetCachedBranches()->GetEntries();
    m_nLearn = tc->GetLearnEntries();
    m_cacheEfficiency = tc->GetEfficiency ();
    m_cacheEfficiencyRel = tc->GetEfficiencyRel ();
    m_bytesRead = tc->GetBytesRead ();
    m_readCalls = tc->GetReadCalls ();
    m_noCacheBytesRead = tc->GetNoCacheBytesRead ();
    m_noCacheReadCalls = tc->GetNoCacheReadCalls ();
    m_readaheadSize = TFile::GetReadaheadSize ();
    m_bytesReadExtra = tc->GetBytesReadExtra ();
    m_fileMetrics->Fill ();
  }
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EL::MetricsSvc :: execute ()
{
  RCU_CHANGE_INVARIANT (this);
  m_eventsRead ++;
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EL::MetricsSvc :: histFinalize ()
{
  RCU_CHANGE_INVARIANT (this);
  m_benchmark->Stop("loopmetrics");
  m_jobMetrics->Branch("eventsRead", &m_eventsRead, "eventsRead/I");
  m_jobMetrics->Branch("filesRead", &m_filesRead, "filesRead/I");
  Float_t realTime = m_benchmark->GetRealTime("loopmetrics");
  m_jobMetrics->Branch("loopWallTime", &realTime, "loopWallTime/F");
  Float_t cpuTime = m_benchmark->GetCpuTime("loopmetrics");
  m_jobMetrics->Branch("loopCpuTime", &cpuTime, "loopCpuTime/F");
  Float_t evtsWallSec = m_eventsRead / (realTime != 0 ? realTime : 1.);
  m_jobMetrics->Branch("eventsPerWallSec", &evtsWallSec, "eventsPerWallSec/F");
  Float_t evtsCpuSec = m_eventsRead / (cpuTime != 0 ? cpuTime : 1.);  
  m_jobMetrics->Branch("eventsPerCpuSec", &evtsCpuSec, "eventsPerCpuSec/F");
  m_jobMetrics->Fill();
  return EL::StatusCode::SUCCESS;
}
