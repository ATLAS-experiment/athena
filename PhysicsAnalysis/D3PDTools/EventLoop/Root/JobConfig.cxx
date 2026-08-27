/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EventLoop/JobConfig.h>

#include <AnaAlgorithm/IAlgorithmWrapper.h>
#include <EventLoop/AlgorithmData.h>
#include <EventLoop/MessageCheck.h>
#include <RootCoreUtils/Assert.h>

#include <stdexcept>

//
// method implementations
//

ClassImp (EL::JobConfig)

namespace EL
{
  void JobConfig :: 
  testInvariant () const
  {}



  JobConfig :: 
  JobConfig () noexcept
  {
    RCU_NEW_INVARIANT (this);
  }



  JobConfig ::
  JobConfig (const JobConfig& that)
    : TObject (that),
      m_algorithmCount (that.m_algorithmCount),
      m_algSequenceStartIndices (that.m_algSequenceStartIndices)
  {
    RCU_READ_INVARIANT (&that);

    for (const auto& algorithm : that.m_algorithms)
    {
      if (algorithm != nullptr)
      {
        m_algorithms.push_back (algorithm->makeClone ());
      } else
      {
        m_algorithms.emplace_back (nullptr);
      }
    }

    RCU_NEW_INVARIANT (this);
  }



  JobConfig ::
  JobConfig (JobConfig&& that) noexcept
    : JobConfig ()
  {
    that.swap (*this);

    // no invariant used
  }



  JobConfig ::
  ~JobConfig () noexcept
  {
    RCU_DESTROY_INVARIANT (this);

    // not actually doing anything here, but have to make the
    // destructor explicit to break include dependencies.
  }



  JobConfig& JobConfig ::
  operator = (const JobConfig& that)
  {
    // no invariant used
    JobConfig (that).swap (*this);
    return *this;
  }



  JobConfig& JobConfig ::
  operator = (JobConfig&& that) noexcept
  {
    // no invariant used
    that.swap (*this);
    return *this;
  }



  void JobConfig ::
  swap (JobConfig& that) noexcept
  {
    RCU_CHANGE_INVARIANT (this);
    RCU_CHANGE_INVARIANT (&that);
    std::swap (m_algorithmCount, that.m_algorithmCount);
    m_algorithms.swap (that.m_algorithms);
    m_algSequenceStartIndices.swap (that.m_algSequenceStartIndices);
  }



  ::StatusCode JobConfig ::
  addAlgorithm (std::unique_ptr<IAlgorithmWrapper>&& val_algorithm)
  {
    using namespace msgEventLoop;

    RCU_CHANGE_INVARIANT (this);
    RCU_REQUIRE (val_algorithm != nullptr);

    if (getAlgorithm (val_algorithm->getName()) != nullptr)
    {
      ANA_MSG_ERROR ("can't have two algorithms with the same name: " << val_algorithm->getName());
      return ::StatusCode::FAILURE;
    }

    m_algorithms.push_back (std::move (val_algorithm));
    ++ m_algorithmCount;
    return ::StatusCode::SUCCESS;
  }



  const IAlgorithmWrapper *JobConfig ::
  getAlgorithm (std::string_view name) const noexcept
  {
    RCU_READ_INVARIANT (this);
    for (const auto& algorithm : m_algorithms)
    {
      if (algorithm != nullptr && algorithm->getName() == name)
        return algorithm.get();
    }
    return nullptr;
  }



  std::vector<Detail::AlgorithmData> JobConfig ::
  extractAlgorithms ()
  {
    RCU_CHANGE_INVARIANT (this);
    if (m_algorithmCount != m_algorithms.size())
      throw std::runtime_error ("JobConfig::extractAlgorithms: algorithm count missmatch.  streaming error?");
    for (const auto& algorithm : m_algorithms)
    {
      if (algorithm == nullptr)
        throw std::runtime_error ("JobConfig::extractAlgorithms: algorithm null.  streaming error?");
    }
    m_algorithmCount = 0;
    std::vector<Detail::AlgorithmData> result;
    result.reserve (m_algorithms.size());
    for (auto& algorithm : m_algorithms)
      result.emplace_back (std::move (algorithm));
    m_algorithms.clear();
    for (auto sequenceStartIndex : m_algSequenceStartIndices)
    {
      // need to check here, since the user could have added a new
      // sequence start at the end of the list, without adding an
      // algorithm for it.
      if (sequenceStartIndex < result.size())
        result[sequenceStartIndex].m_sequenceStart = true;
    }
    return result;
  }



  std::size_t JobConfig ::
  numberOfAlgorithms () const noexcept
  {
    RCU_READ_INVARIANT (this);
    return m_algorithms.size();
  }



  void 
  JobConfig ::
  startNewAlgSequence ()
  {
    RCU_CHANGE_INVARIANT (this);
    m_algSequenceStartIndices.push_back (m_algorithms.size());
  }
}
