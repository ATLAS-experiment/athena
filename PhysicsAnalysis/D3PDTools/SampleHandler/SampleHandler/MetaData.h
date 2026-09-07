/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef SAMPLE_HANDLER_META_DATA_HH
#define SAMPLE_HANDLER_META_DATA_HH

#include <SampleHandler/Global.h>

#include <SampleHandler/Meta.h>

namespace SH
{
  /// \brief A class implementing a templatized version of the
  /// meta-data.
  template<class T> class MetaData : public Meta
  {
    //
    // public interface
    //

    /// \brief test the invariant of this object
    ///
    /// \par Guarantee
    ///   no-fail
  public:
    void testInvariant () const;


    /// \brief standard default constructor
    ///
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory I
    /// \par Rationale
    ///   needed for root persistification
  public:
    MetaData ();


    /// \brief standard constructor
    ///
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  public:
    MetaData (const std::string& name, const T& set_value);


    /// \brief the value contained
  public:
    T value;



    //
    // private interface
    //

    ClassDefOverride(MetaData, 1);
  };
}

#include <RootCoreUtils/Assert.h>

namespace SH
{
  template<class T> void MetaData<T> ::
  testInvariant () const
  {
  }



  template<class T> MetaData<T> ::
  MetaData ()
    : Meta ("")
  {
    RCU_NEW_INVARIANT (this);
  }



  template<class T> MetaData<T> ::
  MetaData (const std::string& name, const T& set_value)
    : Meta (name), value (set_value)
  {
    RCU_NEW_INVARIANT (this);
  }
}

#endif
