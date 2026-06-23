/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EventLoop/AlgorithmStateModule.h>

#include <AnaAlgorithm/AlgorithmWorkerData.h>
#include <AnaAlgorithm/IAlgorithmWrapper.h>
#include <AsgTools/SgEvent.h>
#include <EventLoop/MessageCheck.h>
#include <EventLoop/ModuleData.h>
#include <EventLoop/Worker.h>
#include <RootCoreUtils/Assert.h>
#include <TTree.h>
#include <algorithm>
#include <exception>

//
// method implementations
//

namespace EL
{
  namespace Detail
  {
    namespace
    {
      template<typename F> StatusCode
      forAllAlgorithms (MsgStream& msg, ModuleData& data, const char *funcName, F&& func)
      {
        for (AlgorithmData& alg : data.m_algs)
        {
          try
          {
            typedef typename std::decay<decltype(func(alg))>::type scType__;
            if (!::asg::CheckHelper<scType__>::isSuccess (func (alg)))
            {
              msg << MSG::ERROR << "executing " << funcName << " on algorithm " << alg->getName() << endmsg;
              return StatusCode::FAILURE;
            }
          } catch (...)
          {
            report_exception (std::current_exception());
            msg << MSG::ERROR << "executing " << funcName << " on algorithm " << alg->getName() << endmsg;
            return StatusCode::FAILURE;
          }
        }
        return StatusCode::SUCCESS;
      }
    }



    StatusCode AlgorithmStateModule ::
    onInitialize (ModuleData& data)
    {
      if (m_initialized)
      {
        ANA_MSG_ERROR ("getting second initialize call");
        return StatusCode::FAILURE;
      }
      m_initialized = true;
      AlgorithmWorkerData workerData;
      workerData.m_histogramWorker = data.m_worker;
      workerData.m_treeWorker = data.m_worker;
      workerData.m_filterWorker = data.m_worker;
      workerData.m_wk = data.m_worker;
      workerData.m_evtStore = data.m_evtStore;
      return forAllAlgorithms (msg(), data, "initialize", [&] (AlgorithmData& alg) {
        return alg->initialize (workerData);});
    }



    StatusCode AlgorithmStateModule ::
    onFinalize (ModuleData& data)
    {
      if (!m_initialized)
        return StatusCode::SUCCESS;
      if (forAllAlgorithms (msg(), data, "finalize", [&] (AlgorithmData& alg) {
            return alg->finalize ();}).isFailure())
        return StatusCode::FAILURE;
      return StatusCode::SUCCESS;
    }



    StatusCode AlgorithmStateModule ::
    onCloseInputFile (ModuleData& data)
    {
      return forAllAlgorithms (msg(), data, "endInputFile", [&] (AlgorithmData& alg) {
          return alg->endInputFile ();});
    }



    StatusCode AlgorithmStateModule ::
    onNewInputFile (ModuleData& data)
    {
      if (!m_initialized)
      {
        ANA_MSG_ERROR ("algorithms have not been initialized yet");
        return StatusCode::FAILURE;
      }

      // Check that there are events on input
      if (!data.m_hasInputEvents) return StatusCode::SUCCESS;

      if (forAllAlgorithms (msg(), data, "changeInput", [&] (AlgorithmData& alg) {
            return alg->beginInputFile ();}).isFailure())
        return StatusCode::FAILURE;
      return StatusCode::SUCCESS;
    }



    StatusCode AlgorithmStateModule ::
    onFileExecute (ModuleData& data)
    {
      return forAllAlgorithms (msg(), data, "fileExecute", [&] (AlgorithmData& alg) {
          return alg->fileExecute ();});
    }



    StatusCode AlgorithmStateModule ::
    onExecute (ModuleData& data)
    {
      data.m_skipEvent = false;
      bool sequenceSkip = false;
      for (auto& algData : data.m_algs)
      {
        try
        {
          if (algData.m_sequenceStart)
            sequenceSkip = false;
          else if (sequenceSkip)
          {
            algData.m_wasSkipped = true;
            continue;
          }

          algData.m_executeCount += 1;
          if (algData.m_algorithm->execute() == StatusCode::FAILURE)
          {
            ANA_MSG_ERROR ("while calling execute() on algorithm " << algData.m_algorithm->getName());
            return StatusCode::FAILURE;
          }

          if (data.m_skipEvent)
          {
            algData.m_skipCount += 1;
            algData.m_wasSkipped = true;
            sequenceSkip = true;
            data.m_skipEvent = false;
          }
        } catch (...)
        {
          Detail::report_exception (std::current_exception());
          ANA_MSG_ERROR ("while calling execute() on algorithm " << algData.m_algorithm->getName());
          return StatusCode::FAILURE;
        }
      }

      for (auto& algData : data.m_algs)
      {
        try
        {
          // This will skip `postExecute` for all algorithms that called
          // `setFilterPassed(false)` or that were skipped because of a
          // prior algorithm calling `setFilterPassed(false)`.
          if (algData.m_wasSkipped)
          {
            algData.m_wasSkipped = false;
            continue;
          }
          if (algData.m_algorithm->postExecute() == StatusCode::FAILURE)
          {
            ANA_MSG_ERROR ("while calling postExecute() on algorithm " << algData.m_algorithm->getName());
            return StatusCode::FAILURE;
          }
        } catch (...)
        {
          Detail::report_exception (std::current_exception());
          ANA_MSG_ERROR ("while calling postExecute() on algorithm " << algData.m_algorithm->getName());
          return StatusCode::FAILURE;
        }
      }

      return StatusCode::SUCCESS;
    }
  }
}
