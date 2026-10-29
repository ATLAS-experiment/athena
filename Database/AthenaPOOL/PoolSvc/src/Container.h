/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_CONTAINER_H
#define POOLSVC_CONTAINER_H

#include <string>
#include <typeinfo>

namespace pool {
  class FileDescriptor;
}

class TokenIterator;

/** @class Container
 */

class Container final {
public:
  Container( pool::FileDescriptor& fileDescriptor, const std::string& name );
  
  /// destructor
  ~Container() = default;

  /// Returns the name of this container
  const std::string& name() const { return m_name; }

  /** Starts an iteration over the tokens in the container.
   *  Returns a token iterator whose ownership is passed to the user.
   */
  TokenIterator* tokens();

  /// The method returning the attribute data given a name
  bool attributeOfType( const std::string& attributeName,
                        void* data,
                        const std::type_info& typeInfo,
                        const std::string& option );

  /// The method setting the attribute data given a name
  bool setAttributeOfType( const std::string& attributeName,
                           const void* data,
                           const std::type_info& typeInfo,
                           const std::string& option );

private:
  /// The name of the container
  std::string m_name;

  /// Reference to file descriptor of the parent database
  pool::FileDescriptor& m_fileDescriptor;
};

#endif
