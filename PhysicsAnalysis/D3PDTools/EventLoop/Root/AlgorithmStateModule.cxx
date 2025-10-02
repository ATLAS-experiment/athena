/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EventLoop/AlgorithmStateModule.h>

#include <AnaAlgorithm/AlgorithmWorkerData.h>
#include <AnaAlgorithm/IAlgorithmWrapper.h>
#include <AsgTools/SgTEvent.h>
#include <EventLoop/MessageCheck.h>
#include <EventLoop/ModuleData.h>
#include <EventLoop/Worker.h>
#include <RootCoreUtils/Assert.h>
#include <TTree.h>
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

      if (data.m_inputTree == nullptr ||
          data.m_inputTree->GetEntries() == 0)
        return StatusCode::SUCCESS;

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
      auto iter = data.m_algs.begin();
      try
      {
        for (auto end = data.m_algs.end();
            iter != end; ++ iter)
        {
          iter->m_executeCount += 1;
          if (iter->m_algorithm->execute() == StatusCode::FAILURE)
          {
            ANA_MSG_ERROR ("while calling execute() on algorithm " << iter->m_algorithm->getName());
            return StatusCode::FAILURE;
          }

          if (data.m_skipEvent)
          {
            iter->m_skipCount += 1;
            return StatusCode::SUCCESS;
          }
        }
      } catch (...)
      {
        Detail::report_exception (std::current_exception());
        ANA_MSG_ERROR ("while calling execute() on algorithm " << iter->m_algorithm->getName());
        return StatusCode::FAILURE;
      }

      /// rationale: this will make sure that the post-processing runs
      ///   for all algorithms for which the regular processing was run
      try
      {
        for (auto jter = data.m_algs.begin(), end = iter;
            jter != end && !data.m_skipEvent; ++ jter)
        {
          if (jter->m_algorithm->postExecute() == StatusCode::FAILURE)
          {
            ANA_MSG_ERROR ("while calling postExecute() on algorithm " << iter->m_algorithm->getName());
            return StatusCode::FAILURE;
          }
        }
      } catch (...)
      {
        Detail::report_exception (std::current_exception());
        ANA_MSG_ERROR ("while calling postExecute() on algorithm " << iter->m_algorithm->getName());
        return StatusCode::FAILURE;
      }

      return StatusCode::SUCCESS;
    }
  }
}
