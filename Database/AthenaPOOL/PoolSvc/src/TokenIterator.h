/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_TOKENITERATOR_H
#define POOLSVC_TOKENITERATOR_H

#include <string>

namespace pool {
  class FileDescriptor;
  class DbContainer;
}

class Token;

/** @class TokenIterator
 */

class TokenIterator final {
public:
  /// Constructor taking as argument the file descriptor, the container name
  TokenIterator(pool::FileDescriptor& fileDescriptor, const std::string& containerName);

  ~TokenIterator();

  TokenIterator( const TokenIterator& ) = delete;
  TokenIterator& operator=( const TokenIterator& ) = delete;

  /** 
  * @brief Advances tne iterator and returns a pointer to next token.
  * @returns Shared Token ptr (refCount+1) if not at the end, nullptr otherwise.
  */
  Token* next();

  /**
  * @brief Return the size of the collection.
  */
  std::size_t size();

  /**
  * @brief Seek to a given position in the collection
  * @param position  The position to which to seek.
  * @returns True if successful, false otherwise.
  */
  bool seek(std::size_t position);

private:
  pool::DbContainer* m_container;
  Token* m_refToken;
};

#endif
