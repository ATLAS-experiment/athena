/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TEST_FIXTURES_BENCHMARK_H
#define COLUMNAR_TEST_FIXTURES_BENCHMARK_H

#include <optional>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>

namespace columnar
{
  namespace TestUtils
  {
    /// @brief this is a simple benchmarking helper class wrapping
    /// timers from std::chrono
    ///
    /// Essentially this just takes care of all the bookkeeping around
    /// starting and stopping the timer, accumulating the total time,
    /// and computing the average time per entry. The compiler should
    /// (hopefully) inline the performance sensitive parts to avoid the
    /// overhead. This allows to perform fine-grained and precise
    /// measurements of various parts of the code and is extensively
    /// used in the PHYSLITE tests.
    ///
    /// By default this will just print the average time per entry in
    /// the destructor, however this is not scalable to large number of
    /// separate benchmarks. So in general you should set it to silent
    /// and use @ref getEntryTime to retrieve the time instead. At some
    /// point I may also remove the automatic printing altogether and
    /// make that the default behavior.
    ///
    /// This also allows to specify a batch size, which is useful when
    /// we process multiple events at once and want to compare to time
    /// per event with other batch sizes or single-event processing.
    ///
    /// @warning There is a non-negligible overhead to starting and
    /// stopping the timer. So for one you should not use this in
    /// production code. For another you should measure the overhead
    /// with a separate benchmark instance that starts and stops
    /// immediately and subtract that from the measured time
    /// (getEntryTime provides that option).

    class Benchmark final
    {
      /// Public Members
      /// ==============
    public:

      Benchmark (const std::string& val_name = "", unsigned val_batchSize = 1)
        : m_name (val_name), m_batchSize (val_batchSize)
      {
        if (m_name.empty())
          m_silence = true;
      }

      ~Benchmark ()
      {
        if (m_count > 0 && !m_silence)
          std::cout << m_name << ": " << std::chrono::duration<std::uint64_t,std::nano> (m_ticks) / (m_count * m_batchSize) << std::endl;
      }

      void setSilence ()
      {
        m_silence = true;
      }

      std::optional<float> getEntryTime (float emptyTime) const
      {
        if (m_count == 0)
          return std::nullopt;
        return static_cast<float>((std::chrono::duration<float,std::nano> (m_ticks) / (m_count * m_batchSize)) / std::chrono::duration<float,std::nano> (1))-emptyTime/m_batchSize;
      }

      auto getTotalTime () const
      {
        return m_ticks;
      }

      void startTimer ()
      {
        m_start = std::chrono::high_resolution_clock::now();
      }

      void stopTimer ()
      {
        m_ticks += std::chrono::high_resolution_clock::now() - m_start;
        m_count += 1;
      }



      /// Private Members
      /// ===============
    private:

      std::string m_name;

      std::chrono::time_point<std::chrono::high_resolution_clock> m_start;

      /// accumulated time m_ticks
      std::chrono::high_resolution_clock::duration m_ticks {};

      /// the number of times the timer has been started
      std::uint64_t m_count = 0;

      /// the number of calls per batch
      unsigned m_batchSize = 1;

      /// whether to suppress output
      bool m_silence = false;
    };
  }
}

#endif