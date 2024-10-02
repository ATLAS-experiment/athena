// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  IDataShare.h
 * @author peter van gemmeren <gemmeren@anl.gov>
 * @date March, 2015
 */

#ifndef ATHENAKERNEL_IDATASHARE_H
#define ATHENAKERNEL_IDATASHARE_H 1

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/INamedInterface.h"

/**
 * @class IDataShare
 * @brief Abstract interface for sharing data
 */
class IDataShare : virtual public INamedInterface
{
public:
  DeclareInterfaceID(IDataShare, 1, 0);

  /**
   * @brief Destructor.
   */
  virtual ~IDataShare ();

  /**
   * @brief Make this a server.
   * @param num  The number for the server.
   */
   virtual StatusCode makeServer(int num) = 0;

  /**
   * @brief Make this a client.
   * @param num  The number for the client.
   */
   virtual StatusCode makeClient(int num) = 0;

  /**
   * @brief Read the data
   */
   virtual StatusCode readData() = 0;

  /**
   * @brief Commit Catalog
   */
   virtual StatusCode commitCatalog() = 0;
};


#endif // not ATHENAKERNEL_IDATASHARE_H
