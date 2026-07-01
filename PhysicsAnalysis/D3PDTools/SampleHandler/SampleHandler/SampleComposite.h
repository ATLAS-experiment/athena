/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef SAMPLE_HANDLER_SAMPLE_COMPOSITE_HH
#define SAMPLE_HANDLER_SAMPLE_COMPOSITE_HH

#include <SampleHandler/Global.h>

#include <memory>
#include <string>
#include <vector>
#include <SampleHandler/Sample.h>

namespace SH
{
  /// \brief This module defines an implementation of Sample that
  /// contains composite samples.
  /// \warning This class hasn't been maintained in a long time and
  /// probably has never been fully functional in the first place.
  class SampleComposite final : public Sample
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
    SampleComposite ();


    /// \brief standard constructor
    ///
    /// \param name the name of the sample
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   low level errors II
  public:
    SampleComposite (const std::string& name);


    /// \brief add a sample to the list
    ///
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   low level errors II\n
    ///   sample contains this sample
    /// \pre sample != nullptr
  public:
    void add (std::shared_ptr<Sample> sample);



    //
    // inherited interface
    //

    /// \copydoc Sample::getNumFiles
  protected:
    virtual std::size_t getNumFiles () const override;

    /// \copydoc Sample::getFileName
  protected:
    virtual std::string getFileName (std::size_t index) const override;

    /// \copydoc Sample::doMakeLocal
  protected:
    virtual std::unique_ptr<SampleLocal> doMakeLocal () const override;

    /// \copydoc Sample::doMakeFileList
  protected:
    virtual std::vector<std::string> doMakeFileList () const override;

    /// \copydoc Sample::doUpdateLocation
  protected:
    virtual void
    doUpdateLocation (const std::string& from, const std::string& to) override;

    /// \copydoc Sample::getContains
  protected:
    virtual bool getContains (const std::string& name) const override;

    /// \copydoc Sample::doAddSamples
  protected:
    virtual void doAddSamples (SampleHandler& result,
                               const std::shared_ptr<Sample>& self) override;



    //
    // private interface
    //

    /// \brief the list of samples we use
  private:
    std::vector<std::shared_ptr<Sample>> m_samples;

    /// \brief the iterator for \ref m_samples
  private:
    typedef std::vector<std::shared_ptr<Sample>>::const_iterator SamplesIter;

    ClassDefOverride (SampleComposite, 1);
  };
}

#endif
