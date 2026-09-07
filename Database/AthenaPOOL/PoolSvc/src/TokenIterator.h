/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_TOKENITERATOR_H
#define POOLSVC_TOKENITERATOR_H

#include "PoolSvc/ITokenIterator.h"

#include <string>

namespace pool {

  // forward declarations
  class FileDescriptor;
  class DbContainer;

  /** @class TokenIterator
   *
   *  TokenIterator is an implementation of the ITokenIterator interface
   *
   */

  class TokenIterator : virtual public ITokenIterator
  {
    public:
      /// Constructor taking as argument the file descriptor, the container name
      TokenIterator(FileDescriptor& fileDescriptor, const std::string& containerName);

      ~TokenIterator();

      TokenIterator( const TokenIterator& ) = delete;
      TokenIterator& operator=( const TokenIterator& ) = delete;

      /** 
      * @brief Advances tne iterator and returns a pointer to next token.
      * @returns Shared Token ptr (refCount+1) if not at the end, nullptr otherwise.
      */
      virtual Token* next() override final;

      /**
      * @brief Return the size of the collection.
      */
      virtual std::size_t size()  override final;

      /**
      * @brief Seek to a given position in the collection
      * @param position  The position to which to seek.
      * @returns True if successful, false otherwise.
      */
      virtual bool seek(std::size_t position) override final;

    private:
      DbContainer* m_container;
      Token* m_refToken;
    };
  }

#endif
