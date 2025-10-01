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

      for (Long64_t file = beginFile; file != endFile; ++ file)
      {
        RCU_ASSERT (std::size_t(file) < sample->files.size());
        EventRange eventRange;
        eventRange.m_url = sample->files[file];
        eventRange.m_beginEvent = (file == beginFile ? beginEvent : 0);
        eventRange.m_endEvent = (file == lastFile ? endEvent : EventRange::eof);
        if (maxEvents != -1)
        {
          if (eventRange.m_endEvent == EventRange::eof)
          {
            ANA_CHECK (actions.openInputFile (eventRange.m_url));
            eventRange.m_endEvent = actions.inputFileNumEntries();
          }
          eventRange.m_endEvent = std::min<std::uint64_t> (eventRange.m_endEvent, eventRange.m_beginEvent + maxEvents.value());
        }
          
        ANA_CHECK (actions.processEvents (eventRange));
      }

      return StatusCode::SUCCESS;
    }
  }
}
