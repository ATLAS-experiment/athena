/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <EventLoop/BatchInputModule.h>

#include <EventLoop/BatchJob.h>
#include <EventLoop/BatchSample.h>
#include <EventLoop/BatchSegment.h>
#include <EventLoop/EventRange.h>
#include <EventLoop/IInputModuleActions.h>
#include <EventLoop/ModuleData.h>
#include <RootCoreUtils/Assert.h>

//
// method implementations
//

namespace EL
{
  namespace Detail
  {
    StatusCode BatchInputModule ::
    processInputs (ModuleData& data, IInputModuleActions& actions)
    {
      if (data.m_batchJob == nullptr)
      {
        ANA_MSG_ERROR ("no batch job configured for the input module");
        return StatusCode::FAILURE;
      }
      if (jobId.value() < 0)
      {
        ANA_MSG_ERROR ("no valid job id configured: " << jobId.value());
        return StatusCode::FAILURE;
      }
      if (std::size_t (jobId.value()) >= data.m_batchJob->segments.size())
      {
        ANA_MSG_ERROR ("job id " << jobId.value() << " out of range, only "
                       << data.m_batchJob->segments.size() << " segments");
        return StatusCode::FAILURE;
      }

      BatchSegment *segment = &data.m_batchJob->segments.at(jobId.value());
      if (int (segment->job_id) != jobId.value())
      {
        ANA_MSG_ERROR ("inconsistenty in job id " << jobId.value() << " != " << segment->job_id);
        return StatusCode::FAILURE;
      }
      BatchSample *sample = &data.m_batchJob->samples.at(segment->sample);

      Long64_t beginFile = segment->begin_file;
      Long64_t endFile   = segment->end_file;
      Long64_t lastFile  = segment->end_file;
      RCU_ASSERT (beginFile <= endFile);
      Long64_t beginEvent = segment->begin_event;
      Long64_t endEvent   = segment->end_event;
      if (endEvent > 0) endFile += 1;

      // the number of events still to be processed across the whole
      // segment, so that maxEvents is honoured for the segment as a whole
      // rather than being reapplied to every file
      std::optional<std::int64_t> toProcess;
      if (maxEvents != -1)
        toProcess = maxEvents.value();

      for (Long64_t file = beginFile; file != endFile; ++ file)
      {
        RCU_ASSERT (std::size_t(file) < sample->files.size());
        EventRange eventRange;
        eventRange.m_url = sample->files[file];
        eventRange.m_beginEvent = (file == beginFile ? beginEvent : 0);
        eventRange.m_endEvent = (file == lastFile ? endEvent : EventRange::eof);
        if (toProcess.has_value())
        {
          if (eventRange.m_endEvent == EventRange::eof)
          {
            ANA_CHECK (actions.openInputFile (eventRange.m_url));
            eventRange.m_endEvent = actions.inputFileNumEntries();
          }
          if (eventRange.m_endEvent > eventRange.m_beginEvent + toProcess.value())
            eventRange.m_endEvent = eventRange.m_beginEvent + toProcess.value();
          toProcess.value() -= eventRange.m_endEvent - eventRange.m_beginEvent;
        }

        ANA_CHECK (actions.processEvents (eventRange));

        if (toProcess.has_value() && toProcess.value() == 0)
        {
          ANA_MSG_INFO ("Reached maximum number of events, stopping.");
          break;
        }
      }

      return StatusCode::SUCCESS;
    }
  }
}
