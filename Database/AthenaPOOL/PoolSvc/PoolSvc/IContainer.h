/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_ICONTAINER_H
#define POOLSVC_ICONTAINER_H

// includes
#include <string>
#include <typeinfo>

namespace pool {

  // forward declarations
  class ITokenIterator;

  /** @class IContainer IContainer.h PoolSvc/IContainer.h
   *
   *  IContainer is the base class for container objects
   *
   */

  class IContainer {
  public:
    /// Returns the name of this container
    virtual const std::string& name() const = 0;

    /** Starts an iteration over the tokens in the container.
     *  Returns a token iterator whose ownership is passed to the user.
     */
    virtual ITokenIterator* tokens() = 0;

    /// The method returning the attribute data given a name
    virtual bool attributeOfType( const std::string& attributeName,
                                  void* data,
                                  const std::type_info& typeInfo,
                                  const std::string& option ) = 0;

    /// The method setting the attribute data given a name
    virtual bool setAttributeOfType( const std::string& attributeName,
                                     const void* data,
                                     const std::type_info& typeInfo,
                                     const std::string& option ) = 0;

    /// Virtual destructor for the interface
    virtual ~IContainer() = default;
  };
}

#endif
