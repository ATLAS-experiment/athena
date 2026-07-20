/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EventLoop/AlgorithmTimerWrapper.h>

#include <EventLoop/MessageCheck.h>
#include <RootCoreUtils/Assert.h>
#include <iomanip>
#include <ios>

//
// method implementations
//

namespace EL
{
  void AlgorithmTimerWrapper ::
  testInvariant () const
  {
  }



  AlgorithmTimerWrapper ::
  AlgorithmTimerWrapper (std::unique_ptr<IAlgorithmWrapper>&& val_algorithm)
    : m_algorithm (std::move (val_algorithm))
  {
    RCU_NEW_INVARIANT (this);
  }



  std::string_view AlgorithmTimerWrapper ::
  getName () const
  {
    RCU_READ_INVARIANT (this);
    return m_algorithm->getName();
  }



  bool AlgorithmTimerWrapper ::
  hasName (const std::string& name) const
  {
    RCU_READ_INVARIANT (this);
    return m_algorithm->hasName (name);
  }



  std::unique_ptr<IAlgorithmWrapper> AlgorithmTimerWrapper ::
  makeClone() const
  {
    using namespace msgEventLoop;
    RCU_READ_INVARIANT (this);

    return std::make_unique<AlgorithmTimerWrapper> (m_algorithm->makeClone());
  }



  Algorithm *AlgorithmTimerWrapper ::
  getLegacyAlg ()
  {
    RCU_READ_INVARIANT (this);
    return m_algorithm->getLegacyAlg();
  }



  StatusCode AlgorithmTimerWrapper ::
  initialize (const AlgorithmWorkerData& workerData)
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    auto start = clock_type::now();
    auto result = m_algorithm->initialize (workerData);
    auto stop = clock_type::now();
    m_time_global += stop - start;
    return result;
  }



  StatusCode AlgorithmTimerWrapper ::
  execute (const EventContext& ctx)
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    auto start = clock_type::now();
    auto result = m_algorithm->execute (ctx);
    auto stop = clock_type::now();
    m_time_event += stop - start;
    return result;
  }



  StatusCode AlgorithmTimerWrapper ::
  postExecute ()
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    auto start = clock_type::now();
    auto result = m_algorithm->postExecute ();
    auto stop = clock_type::now();
    m_time_event += stop - start;
    return result;
  }



  StatusCode AlgorithmTimerWrapper ::
  finalize ()
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    auto start = clock_type::now();
    auto result = m_algorithm->finalize ();
    auto stop = clock_type::now();
    m_time_global += stop - start;
    auto seconds = std::chrono::duration<float>(1);
    std::ostringstream str;
    str << std::setprecision(2) << std::fixed << "algorithm timer: " << m_algorithm->getName() << " " << (m_time_event/seconds)  << " " << (m_time_file/seconds) << " " << (m_time_global/seconds);
    ANA_MSG_INFO (str.str());
    return result;
  }



  ::StatusCode AlgorithmTimerWrapper ::
  fileExecute ()
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    auto start = clock_type::now();
    auto result = m_algorithm->fileExecute ();
    auto stop = clock_type::now();
    m_time_file += stop - start;
    return result;
  }



  ::StatusCode AlgorithmTimerWrapper ::
  beginInputFile ()
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    auto start = clock_type::now();
    auto result = m_algorithm->beginInputFile ();
    auto stop = clock_type::now();
    m_time_file += stop - start;
    return result;
  }



  ::StatusCode AlgorithmTimerWrapper ::
  endInputFile ()
  {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    auto start = clock_type::now();
    auto result = m_algorithm->endInputFile ();
    auto stop = clock_type::now();
    m_time_file += stop - start;
    return result;
  }
}
