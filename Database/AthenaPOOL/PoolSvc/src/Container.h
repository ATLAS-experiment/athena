/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_CONTAINER_H
#define POOLSVC_CONTAINER_H

// includes
#include "PoolSvc/IContainer.h"

namespace pool {

  // forward declarations
  class FileDescriptor;

  /** @class Container
   *
   *  Container is an implementation of the IContainer interface
   *
   */
  
  class Container : virtual public IContainer {
  public:
    Container( FileDescriptor& fileDescriptor,
               long technology,
               const std::string& name );
    
    /// destructor
    virtual ~Container() = default;

    /// Returns the name of this container
    virtual const std::string& name() const override final { return m_name; }

    /// Returns the technology identifier for this container
    virtual long technology() const override final { return m_technology; }

    /** Starts an iteration over the tokens in the container.
     *  Returns a token iterator whose ownership is passed to the user.
     */
    virtual ITokenIterator* tokens() override;

    /// The method returning the attribute data given a name
    virtual
    bool attributeOfType( const std::string& attributeName,
                          void* data,
                          const std::type_info& typeInfo,
                          const std::string& option ) override;

    /// The method setting the attribute data given a name
    virtual
    bool setAttributeOfType( const std::string& attributeName,
                             const void* data,
                             const std::type_info& typeInfo,
                             const std::string& option ) override;

  private:
    /// The name of the container
    std::string m_name;

    /// Reference to file descriptor of the parent database
    FileDescriptor& m_fileDescriptor;

    /// The technology identifier
    long m_technology;
  };
}

#endif
