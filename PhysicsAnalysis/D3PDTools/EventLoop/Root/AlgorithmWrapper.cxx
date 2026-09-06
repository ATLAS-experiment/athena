/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EventLoop/AlgorithmWrapper.h>

#include <AnaAlgorithm/MessageCheck.h>
#include <AnaAlgorithm/AlgorithmWorkerData.h>
#include <EventLoop/Algorithm.h>
#include <EventLoop/IWorker.h>
#include <RootCoreUtils/Assert.h>

#include <format>
#include <stdexcept>

//
// method implementations
//

namespace EL
{
  void AlgorithmWrapper ::
  testInvariant () const
  {
    RCU_INVARIANT (m_algorithm != nullptr);
  }



  AlgorithmWrapper ::
  AlgorithmWrapper (std::unique_ptr<Algorithm>&& val_algorithm)
    : m_algorithm (std::move (val_algorithm))
  {
    RCU_NEW_INVARIANT (this);
  }



  std::string_view AlgorithmWrapper ::
  getName () const
  {
    RCU_READ_INVARIANT (this);
    return m_algorithm->GetName();
  }



  bool AlgorithmWrapper ::
  hasName (const std::string& name) const
  {
    RCU_READ_INVARIANT (this);
    return m_algorithm->hasName (name);
  }



  std::unique_ptr<IAlgorithmWrapper> AlgorithmWrapper ::
  makeClone() const
  {
    using namespace msgAlgorithmConfig;
    RCU_READ_INVARIANT (this);

    std::unique_ptr<Algorithm> myalgorithm
      (dynamic_cast<Algorithm*>(m_algorithm->Clone ()));
    if (myalgorithm == nullptr)
      throw std::runtime_error (std::format ("failed to clone algorithm {}", m_algorithm->GetName()));
    return std::make_unique<AlgorithmWrapper> (std::move (myalgorithm));
  }



  Algorithm *AlgorithmWrapper ::
  getLegacyAlg ()
  {
    RCU_READ_INVARIANT (this);
    return m_algorithm.get();
  }



  StatusCode AlgorithmWrapper ::
  initialize (const AlgorithmWorkerData& workerData)
  {
    using namespace msgAlgorithmConfig;
    RCU_CHANGE_INVARIANT (this);

    m_algorithm->m_wk = workerData.m_wk;
    m_algorithm->m_evtStorePtr = workerData.m_evtStore;
    if (m_algorithm->histInitialize().isFailure())
    {
      ANA_MSG_ERROR ("failed to call histInitialize() on algorithm: " << m_algorithm->name());
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }



  StatusCode AlgorithmWrapper ::
  execute (const EventContext& /*ctx*/)
  {
    using namespace msgAlgorithmConfig;
    RCU_CHANGE_INVARIANT (this);

    if (m_algorithm->execute().isFailure())
    {
      ANA_MSG_ERROR ("failed to call execute() on algorithm: " << m_algorithm->name());
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }



  StatusCode AlgorithmWrapper ::
  postExecute ()
  {
    using namespace msgAlgorithmConfig;
    RCU_CHANGE_INVARIANT (this);

    if (m_algorithm->postExecute().isFailure())
    {
      ANA_MSG_ERROR ("failed to call postExecute() on algorithm: " << m_algorithm->name());
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }



  StatusCode AlgorithmWrapper ::
  finalize ()
  {
    using namespace msgAlgorithmConfig;
    RCU_CHANGE_INVARIANT (this);

    if (m_isInitialized == true)
    {
      if (m_algorithm->finalize().isFailure())
      {
        ANA_MSG_ERROR ("failed to call finalize() on algorithm: " << m_algorithm->name());
        return StatusCode::FAILURE;
      }
    }
    if (m_algorithm->histFinalize().isFailure())
    {
      ANA_MSG_ERROR ("failed to call histFinalize() on algorithm: " << m_algorithm->name());
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }



  ::StatusCode AlgorithmWrapper ::
  fileExecute ()
  {
    using namespace msgAlgorithmConfig;
    RCU_CHANGE_INVARIANT (this);

    if (m_algorithm->fileExecute().isFailure())
    {
      ANA_MSG_ERROR ("failed to call fileExecute() on algorithm: " << m_algorithm->name());
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }



  ::StatusCode AlgorithmWrapper ::
  beginInputFile ()
  {
    using namespace msgAlgorithmConfig;
    RCU_CHANGE_INVARIANT (this);

    if (m_algorithm->changeInput(m_firstFile).isFailure())
    {
      ANA_MSG_ERROR ("failed to call changeInput() on algorithm: " << m_algorithm->name());
      return StatusCode::FAILURE;
    }
    m_firstFile = false;
    if (m_isInitialized == false && m_algorithm->m_wk->hasInputEvents())
    {
      if (m_algorithm->initialize().isFailure())
      {
        ANA_MSG_ERROR ("failed to call initialize() on algorithm: " << m_algorithm->name());
        return StatusCode::FAILURE;
      }
      m_isInitialized = true;
    }
    return StatusCode::SUCCESS;
  }



  ::StatusCode AlgorithmWrapper ::
  endInputFile ()
  {
    using namespace msgAlgorithmConfig;
    RCU_CHANGE_INVARIANT (this);

    if (m_algorithm->endOfFile().isFailure())
    {
      ANA_MSG_ERROR ("failed to call endOfFile() on algorithm: " << m_algorithm->name());
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }
}
