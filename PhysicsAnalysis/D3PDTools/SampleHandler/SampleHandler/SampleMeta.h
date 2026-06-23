/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef SAMPLE_HANDLER__SAMPLE_META_H
#define SAMPLE_HANDLER__SAMPLE_META_H

#include <SampleHandler/Global.h>

#include <SampleHandler/Sample.h>

namespace SH
{
  /// \brief A Sample that consists only of Meta-Information.
  ///
  /// The purpose of this sample is that you can fill an entire
  /// SampleHandler with just Meta-Information and then pull the data
  /// for the samples you are actually using into your local sample
  /// via SampleHandler::fetchDefaults().
  class SampleMeta final : public Sample
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


    /// \brief standard constructor
    ///
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory I
    /// \warning only to be used by serialization code
  public:
    SampleMeta ();


    /// \brief standard constructor
    ///
    /// \param name the name of the sample
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  public:
    SampleMeta (const std::string& name);



    //
    // inherited interface
    //

    /// \copydoc Sample::getNumFiles
  private:
    virtual std::size_t getNumFiles () const override;

    /// \copydoc Sample::getFileName
  private:
    virtual std::string getFileName (std::size_t index) const override;

    /// \copydoc Sample::doMakeLocal
  private:
    virtual std::unique_ptr<SampleLocal> doMakeLocal () const override;

    /// \copydoc Sample::doMakeFileList
  protected:
    virtual std::vector<std::string> doMakeFileList () const override;



    //
    // private interface
    //

    ClassDefOverride (SampleMeta, 1);
  };
}

#endif
