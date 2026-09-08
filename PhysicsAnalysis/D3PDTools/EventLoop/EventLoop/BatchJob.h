/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef EVENT_LOOP_BATCH_JOB_HH
#define EVENT_LOOP_BATCH_JOB_HH

/// This module defines the arguments passed from the BATCH driver to
/// the BATCH worker.



#include <EventLoop/Global.h>

#include <vector>
#include <TObject.h>
#include <EventLoop/Job.h>
#include <SampleHandler/MetaObject.h>

namespace EL
{
  struct BatchJob : public TObject
  {
    //
    // public interface
    //

    /// effects: standard default constructor
    /// guarantee: no-fail
  public:
    BatchJob ();


    /// effects: standard destructor
    /// guarantee: no-fail
  public:
    ~BatchJob ();


    /// description: the job we are using
  public:
    Job job;


    /// description: the location of the submission directory, if it
    ///   is shared
    /// rationale: this allows to place the output files directly
    ///   where they belong, instead of copying them around
  public:
    std::string location;


    /// description: the list of samples
  public:
    std::vector<BatchSample> samples;

    /// description: the list of segments
  public:
    std::vector<BatchSegment> segments;



    //
    // private interface
    //

    ClassDef(BatchJob, 1);
  };
}

#endif
