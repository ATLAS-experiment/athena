/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PERSISTENCYSVC_TOKENITERATOR_H
#define PERSISTENCYSVC_TOKENITERATOR_H

#include "PersistencySvc/ITokenIterator.h"
#include "GaudiKernel/implements.h"

#include <string>

namespace pool {

  // forward declarations
  class FileDescriptor;
  class DbContainer;

  namespace PersistencySvc {

    /** @class TokenIterator
     *
     *  TokenIterator is an implementation of the ITokenIterator interface
     *
     */

    class TokenIterator : virtual public ITokenIterator
      {
      public:
	/** Constructor taking as argument the file descriptor, the container name
	 */
	TokenIterator( FileDescriptor& fileDescriptor,
		       const std::string& containerName );
	/// Destructor
	~TokenIterator();

        TokenIterator( const TokenIterator& ) = delete;
        TokenIterator& operator=( const TokenIterator& ) = delete;

	/** Returns the pointer to next token.
	 *  Token ownership is passed to the user.
	 *  if no other token is available in the iteration
	 *  sequence, 0 is returned.
	 */
	Token* next();

        /**
         * @brief Return the size of the collection.
         */
        virtual std::size_t size();

        /**
         * @brief Seek to a given position in the collection
         * @param position  The position to which to seek.
         * @returns True if successful, false otherwise.
         */
        virtual bool seek(std::size_t position);

      private:
	DbContainer* m_container;
        Token* m_refToken;
      };
  }
}

#endif
