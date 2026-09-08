/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef SAMPLE_HANDLER_META_VECTOR_HH
#define SAMPLE_HANDLER_META_VECTOR_HH

#include <SampleHandler/Global.h>

#include <vector>
#include <SampleHandler/Meta.h>

namespace SH
{
  /// \brief This class defines a templatized version of the meta-data
  /// in vector form.
  template<class T> class MetaVector : public Meta
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
    MetaVector ();


    /// \brief standard constructor
    ///
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  public:
    MetaVector (const std::string& name, const std::vector<T>& set_value);


    /// \brief the value contained
  public:
    std::vector<T> value;



    //
    // private interface
    //

    ClassDefOverride(MetaVector, 1);
  };
}

#include <RootCoreUtils/Assert.h>

namespace SH
{
  template<class T> void MetaVector<T> ::
  testInvariant () const
  {
  }



  template<class T> MetaVector<T> ::
  MetaVector ()
    : Meta ("")
  {
    RCU_NEW_INVARIANT (this);
  }



  template<class T> MetaVector<T> ::
  MetaVector (const std::string& name, const std::vector<T>& set_value)
    : Meta (name), value (set_value)
  {
    RCU_NEW_INVARIANT (this);
  }
}

#endif
