/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef EVENT_LOOP__ALGORITHM_MEMORY_WRAPPER_H
#define EVENT_LOOP__ALGORITHM_MEMORY_WRAPPER_H

#include <EventLoop/Global.h>

#include <AnaAlgorithm/IAlgorithmWrapper.h>
#include <Rtypes.h>
#include <chrono>

namespace EL
{
  /// \brief an \ref IAlgorithmWrapper that adds a memory monitor to an
  /// algorithm
  ///
  /// This is a little weird in how its done:  We can't just count the total
  /// memory allocated as all algorithms allocate memory which gets released at
  /// the end of the event.  We also can't just count what gets allocated in
  /// initialize (which would seem natural), as some tools and algorithms do not
  /// allocate all their memory until we hit execute.  And it may not even be
  /// the first execute either, it depends on what's in the event.  So I'm
  /// summing up the memory increase from the top ten calls to functions on the
  /// algorithm.  This is a bit of a hack, but the whole mechanism is meant to
  /// be quick and dirty.  If you want a proper estimate, you should use
  /// valgrind.
  ///
  /// @note This module is specifically intended to debug the issue with
  /// analysis jobs running out of memory (Feb 24).  Once that issue is
  /// resolved for good, or if there are fundamental issues that break this
  /// module it can be removed.
  ///
  /// @note There is a dedicated test in AnalysisAlgorithms config that runs a
  /// test job with this module enabled to ensure it runs and doesn't break
  /// the output.

  class AlgorithmMemoryWrapper final : public IAlgorithmWrapper
  {
    /// Public Members
    /// ==============

  public:

    /// \brief the clock we use for our Memory
    using clock_type = std::chrono::high_resolution_clock;

    /// \brief test the invariant of this object
    void testInvariant () const;

    /// \brief standard default constructor for serialization
    AlgorithmMemoryWrapper () {};

    /// \brief standard constructor
    AlgorithmMemoryWrapper (std::unique_ptr<IAlgorithmWrapper>&& val_algorithm);



    /// Inherited Members
    /// =================

  public:

    virtual std::string_view getName () const override;

    virtual bool hasName (const std::string& name) const override;

    virtual std::unique_ptr<IAlgorithmWrapper> makeClone() const override;

    virtual Algorithm *getLegacyAlg () override;

    virtual StatusCode initialize (const AlgorithmWorkerData& workerData) override;

    virtual StatusCode execute () override;

    virtual StatusCode postExecute () override;

    virtual StatusCode finalize () override;

    virtual StatusCode fileExecute () override;

    virtual StatusCode beginInputFile () override;

    virtual StatusCode endInputFile () override;



    /// Private Members
    /// ===============

  private:

    /// \brief the actual algorithm
    std::unique_ptr<IAlgorithmWrapper> m_algorithm;

    /// \brief the Memory Consumption for different calls
    std::vector<Long_t> m_mem_resident;
    std::vector<Long_t> m_mem_virtual;

    Long_t m_preMem_resident = 0;
    Long_t m_preMem_virtual = 0;

    StatusCode recordPreMemory();
    StatusCode recordPostMemory();
  };
}

#endif
