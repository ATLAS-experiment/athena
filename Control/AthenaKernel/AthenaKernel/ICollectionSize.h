// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  ICollectionSize.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2005
 * @brief Abstract interface for finding the size of an event collection.
 */

#ifndef ATHENAKERNEL_ICOLLECTIONSIZE_H
#define ATHENAKERNEL_ICOLLECTIONSIZE_H


#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/IInterface.h"

/**
 * @class ICollectionSize
 * @brief Abstract interface for finding the size of an event collection.
 */
class ICollectionSize : virtual public IInterface
{
public:
  DeclareInterfaceID( ICollectionSize, 1, 0 );

  /**
   * @brief Destructor.
   */
  virtual ~ICollectionSize () {};

  /**
   * @brief Return the size of the collection.
   */
  virtual int size () = 0;
};


#endif // not ATHENAKERNEL_ICOLLECTIONSIZE_H
