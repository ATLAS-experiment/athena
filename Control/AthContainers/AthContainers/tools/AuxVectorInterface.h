// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/tools/AuxVectorInterface.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2024
 * @brief Make an AuxVectorData object from either a raw vector or an aux store.
 */


#ifndef ATHCONTAINERS_AUXVECTORINTERFACE_H
#define ATHCONTAINERS_AUXVECTORINTERFACE_H


#include "AthContainers/AuxVectorData.h"


namespace SG {


/**
 * @brief Make an AuxVectorData object from either a raw array or an aux store.
 *
 * The swap/copy/clear operations in AuxTypeRegistry take AuxVectorData
 * instances as input.  But sometimes we have only the auxiliary store,
 * or even just a pointer to a raw array.  In that case, we can create
 * a temporary instance of this class to wrap the data.
 */
class AuxVectorInterface
  : public AuxVectorData
{
public:
  /**
   * @brief Build from a (non-const) auxiliary store.
   * @param store The auxiliary store to wrap.
   */
  AuxVectorInterface (IAuxStore& store);


  /**
   * @brief Build from a (const) auxiliary store.
   * @param store The auxiliary store to wrap.
   */
  AuxVectorInterface (const IConstAuxStore& store);


  /**
   * @brief Build from a (non-const) raw array.
   * @param auxid ID of the represented variable.
   * @param size Length of the array.
   * @param ptr Pointer to the array.
   */
  AuxVectorInterface (SG::auxid_t auxid, size_t size, void* ptr);


  /**
   * @brief Build from a (const) raw array.
   * @param auxid ID of the represented variable.
   * @param size Length of the array.
   * @param ptr Pointer to the array.
   */
  AuxVectorInterface (SG::auxid_t auxid, size_t size, const void* ptr);


  /**
   * @brief Return the size of the container.
   */
  virtual size_t size_v() const override final;


  /**
   * @brief Return the capacity of the container.
   */
  virtual size_t capacity_v() const override final;


private:
  /// Size of the container.
  size_t m_size;
};


} // namespace SG


#include "AthContainers/tools/AuxVectorInterface.icc"


#endif // not ATHCONTAINERS_AUXVECTORINTERFACE_H
