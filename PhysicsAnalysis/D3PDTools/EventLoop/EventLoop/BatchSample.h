/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef EVENT_LOOP_BATCH_SAMPLE_HH
#define EVENT_LOOP_BATCH_SAMPLE_HH

/// This module defines the arguments passed from the BATCH driver to
/// the BATCH worker.



#include <EventLoop/Global.h>

#include <TObject.h>
#include <SampleHandler/MetaObject.h>

namespace EL
{
  struct BatchSample : public TObject
  {
    //
    // public interface
    //

    /// effects: standard default constructor
    /// guarantee: no-fail
  public:
    BatchSample ();


    /// effects: standard destructor
    /// guarantee: no-fail
  public:
    ~BatchSample ();


    /// description: the names of the sample
  public:
    std::string name;


    /// description: the sample meta-information
  public:
    SH::MetaObject meta;


    /// description: the list of files we are reading
  public:
    std::vector<std::string> files;


    /// description: the beginning and end of the segments for this
    ///   sample
  public:
    UInt_t begin_segments, end_segments;



    //
    // private interface
    //

    ClassDef(BatchSample, 1);
  };
}

#endif
