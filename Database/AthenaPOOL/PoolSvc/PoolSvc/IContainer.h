/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_ICONTAINER_H
#define POOLSVC_ICONTAINER_H

// includes
#include <string>

namespace pool {

  // forward declarations
  class ITokenIterator;
  class ITechnologySpecificAttributes;

  /** @class IContainer IContainer.h PoolSvc/IContainer.h
   *
   *  IContainer is the base class for container objects
   *
   */

  class IContainer {
  public:
    /// Returns the name of this container
    virtual const std::string& name() const = 0;

    /// Returns the technology identifier for this container
    virtual long technology() const = 0;

    /** Starts an iteration over the tokens in the container.
     *  Returns a token iterator whose ownership is passed to the user.
     */
    virtual ITokenIterator* tokens() = 0;

    /// Returns the object holding the technology specific attributes for a given technology domain
    virtual ITechnologySpecificAttributes& technologySpecificAttributes() = 0;

    /// Virtual destructor for the interface
    virtual ~IContainer() = default;
  };
}

#endif
