/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_ITOKENITERATOR_H
#define POOLSVC_ITOKENITERATOR_H

#include <cstddef>

// forward declarations
class Token;

namespace pool {

  /** @class ITokenIterator ITokenIterator.h PoolSvc/ITokenIterator.h
   *
   *  ITokenIterator is the interface class for iterators of tokens
   *
   */

  class ITokenIterator {
  public:
    /// Empty destructor
    virtual ~ITokenIterator() = default;

    /** @brief Returns the size of the collection.
     */
    virtual std::size_t size () = 0;

    /**
     * @brief Seek to a given position in the collection
     * @param position  The position to which to seek.
     * @returns True if successful, false otherwise.
     */

    virtual bool seek (std::size_t position) = 0;

    /**
     * @brief Advances tne iterator and returns a pointer to next token.
     * @returns Shared Token ptr (refCount+1) if not at the end, nullptr otherwise.
     */
    virtual Token* next() = 0;
  };

}

#endif
