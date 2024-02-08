/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef EVENT_LOOP__ALGORITHM_TIMER_WRAPPER_H
#define EVENT_LOOP__ALGORITHM_TIMER_WRAPPER_H

#include <EventLoop/Global.h>

#include <AnaAlgorithm/IAlgorithmWrapper.h>
#include <chrono>

namespace EL
{
  /// \brief an \ref IAlgorithmWrapper that adds a timer to an
  /// algorithm
  ///
  /// @note There is a dedicated test in AnalysisAlgorithms config that runs a
  /// test job with this module enabled to ensure it runs and doesn't break
  /// the output.

  class AlgorithmTimerWrapper final : public IAlgorithmWrapper
  {
    /// Public Members
    /// ==============

  public:

    /// \brief the clock we use for our timer
    using clock_type = std::chrono::high_resolution_clock;

    /// \brief test the invariant of this object
    void testInvariant () const;

    /// \brief standard default constructor for serialization
    AlgorithmTimerWrapper () {};

    /// \brief standard constructor
    AlgorithmTimerWrapper (std::unique_ptr<IAlgorithmWrapper>&& val_algorithm);



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

    virtual ::StatusCode fileExecute () override;

    virtual ::StatusCode beginInputFile () override;

    virtual ::StatusCode endInputFile () override;



    /// Private Members
    /// ===============

  private:

    /// \brief the actual algorithm
    std::unique_ptr<IAlgorithmWrapper> m_algorithm;

    /// \brief the timers for different calls
    clock_type::duration m_time_global {};
    clock_type::duration m_time_file {};
    clock_type::duration m_time_event {};
  };
}

#endif
