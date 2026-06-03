/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef EVENT_LOOP__ALGORITHM_DATA_H
#define EVENT_LOOP__ALGORITHM_DATA_H

#include <EventLoop/Global.h>

#include <AnaAlgorithm/Global.h>
#include <cstdint>
#include <memory>

namespace EL
{
  namespace Detail
  {
    /// \brief all the data a worker tracks for an individual algorithm
    ///
    /// This is a simple `struct` without accessors, as it is meant to
    /// be strictly internal to the worker and module implementation.

    struct AlgorithmData final
    {
      /// \brief the algorithm we use
      std::unique_ptr<IAlgorithmWrapper> m_algorithm;

      /// \brief the number of times this algorithm has been called
      uint64_t m_executeCount {0};

      /// \brief the number of times this algorithm has asked to skip
      /// this event
      uint64_t m_skipCount {0};

      /// \brief whether this algorithm starts a new logical sequence
      ///
      /// This allows grouping algorithms into sub-sequences within the
      /// overall sequence. This allows filter algorithms to skip the
      /// rest of the current sub-sequence, instead of all algorithms. 
      bool m_sequenceStart {false};

      /// \brief whether this algorithm was skipped during the execute
      /// pass
      ///
      /// The AlgorithmStateModule sets this flag when an algorithm is
      /// skipped, to indicate that `postExecute` should not be run for
      /// this algorithm.
      bool m_wasSkipped {false};


      AlgorithmData () = default;

      AlgorithmData (std::unique_ptr<IAlgorithmWrapper> val_algorithm);

      AlgorithmData (AlgorithmData&& that) = default;

      ~AlgorithmData () noexcept;

      inline IAlgorithmWrapper *operator -> () {return m_algorithm.get();};

      inline const IAlgorithmWrapper *operator -> () const {return m_algorithm.get();};
    };
  }
}

#endif
