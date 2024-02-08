/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EventLoop/AlgorithmMemoryWrapper.h>

#include <EventLoop/MessageCheck.h>
#include <RootCoreUtils/Assert.h>
#include <TSystem.h>
#include <iomanip>
#include <ios>
#include <numeric>

//
// method implementations
//

namespace EL
{
  void AlgorithmMemoryWrapper ::
  testInvariant () const
  {
  }



  AlgorithmMemoryWrapper ::
  AlgorithmMemoryWrapper (std::unique_ptr<IAlgorithmWrapper>&& val_algorithm)
    : m_algorithm (std::move (val_algorithm))
  {
    RCU_NEW_INVARIANT (this);
  }



  std::string_view AlgorithmMemoryWrapper ::
  getName () const
  {
    RCU_READ_INVARIANT (this);
    return m_algorithm->getName();
  }



  bool AlgorithmMemoryWrapper ::
  hasName (const std::string& name) const
  {
    RCU_READ_INVARIANT (this);
    return m_algorithm->hasName (name);
  }



  std::unique_ptr<IAlgorithmWrapper> AlgorithmMemoryWrapper ::
  makeClone() const
  {
    using namespace msgEventLoop;
    RCU_READ_INVARIANT (this);

    return std::make_unique<AlgorithmMemoryWrapper> (m_algorithm->makeClone());
  }



  Algorithm *AlgorithmMemoryWrapper ::
  getLegacyAlg ()
  {
    RCU_READ_INVARIANT (this);
    return m_algorithm->getLegacyAlg();
  }



  StatusCode AlgorithmMemoryWrapper ::
  initialize (const AlgorithmWorkerData& workerData)
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    ANA_CHECK (recordPreMemory());
    auto result = m_algorithm->initialize (workerData);
    ANA_CHECK (recordPostMemory());
    return result;
  }



  StatusCode AlgorithmMemoryWrapper ::
  execute ()
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    ANA_CHECK (recordPreMemory());
    auto result = m_algorithm->execute ();
    ANA_CHECK (recordPostMemory());
    return result;
  }



  StatusCode AlgorithmMemoryWrapper ::
  postExecute ()
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    ANA_CHECK (recordPreMemory());
    auto result = m_algorithm->postExecute ();
    ANA_CHECK (recordPostMemory());
    return result;
  }



  StatusCode AlgorithmMemoryWrapper ::
  finalize ()
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    ANA_CHECK (recordPreMemory());
    auto result = m_algorithm->finalize ();
    ANA_CHECK (recordPostMemory());
    std::ostringstream str;
    str << "algorithm memory: " << m_algorithm->getName() << " rss=";
    for (auto& rss : m_mem_resident)
      str << rss << ",";
    str << " vss=";
    for (auto& vss : m_mem_virtual)
      str << vss << ",";
    ANA_MSG_INFO (str.str());
    return result;
  }



  StatusCode AlgorithmMemoryWrapper ::
  fileExecute ()
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    ANA_CHECK (recordPreMemory());
    auto result = m_algorithm->fileExecute ();
    ANA_CHECK (recordPostMemory());
    return result;
  }



  StatusCode AlgorithmMemoryWrapper ::
  beginInputFile ()
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    ANA_CHECK (recordPreMemory());
    auto result = m_algorithm->beginInputFile ();
    ANA_CHECK (recordPostMemory());
    return result;
  }



  StatusCode AlgorithmMemoryWrapper ::
  endInputFile ()
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    ANA_CHECK (recordPreMemory());
    auto result = m_algorithm->endInputFile ();
    ANA_CHECK (recordPostMemory());
    return result;
  }



  StatusCode AlgorithmMemoryWrapper ::
  recordPreMemory()
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);
    ::ProcInfo_t pinfo;
    if (gSystem->GetProcInfo (&pinfo) != 0) {
      ANA_MSG_ERROR ("Could not get memory usage information");
      return StatusCode::FAILURE;
    }
    m_preMem_resident = pinfo.fMemResident;
    m_preMem_virtual = pinfo.fMemVirtual;
    return StatusCode::SUCCESS;    
  }



  StatusCode AlgorithmMemoryWrapper ::
  recordPostMemory()
  {
    constexpr std::size_t max_entries = 3;

    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);
    ::ProcInfo_t pinfo;
    if (gSystem->GetProcInfo (&pinfo) != 0) {
      ANA_MSG_ERROR ("Could not get memory usage information");
      return StatusCode::FAILURE;
    }
    m_mem_resident.push_back (pinfo.fMemResident - m_preMem_resident);
    std::sort (m_mem_resident.begin()+1, m_mem_resident.end(), std::greater<Long_t>());
    if (m_mem_resident.size() > max_entries)
      m_mem_resident.resize (max_entries);
    m_mem_virtual.push_back (pinfo.fMemVirtual - m_preMem_virtual);
    std::sort (m_mem_virtual.begin()+1, m_mem_virtual.end(), std::greater<Long_t>());
    if (m_mem_virtual.size() > max_entries)
      m_mem_virtual.resize (max_entries);
    return StatusCode::SUCCESS;    
  }
}
