/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ROOTDOMAIN_H
#define ROOTDOMAIN_H 1

#include "StorageSvc/ITechDomain.h"

class StatusCode;
class DbOption;

/** @class RootDomain RootDomain.h src/RootDomain.h
    *
    * Description:
    * ROOT specific implementation of Database Domain.
    * ROOT does not support Domains. This is then simply
    * a memory list.
    */
class RootDomain : virtual public ITechDomain  {
  protected:
    /// Default compression level
    int m_defCompression;
    /// Default compression algorithm
    int m_defCompressionAlg;
    /// Default split level
    int m_defSplitLevel;
    /// Default Autosave parameter for trees
    int m_defAutoSave;
    /// Default buffer size parameter for Branches
    int m_defBufferSize;
    /// Offset table length for branches
    int	m_branchOffsetTabLen;
    /// Default container DbType
    int m_defContainerType;
    
  public:
    /// Standard Constuctor
    RootDomain();

    /// Set domain specific options
    /** @param type     [IN]  Specific database type (Including minor type)
      * @param refOpt   [IN]  Reference to option object
      *
      * @return StatusCode code indicating success or failure.  
      */
    virtual StatusCode setOption(const DbOption& refOpt);

    /// Access domain specific options
    /** @param type     [IN]  Specific database type (Including minor type)
      * @param refOpt   [IN]  Reference to option object
      *
      * @return StatusCode code indicating success or failure.  
      */
    virtual StatusCode getOption(DbOption& refOpt) const;
};

#endif
