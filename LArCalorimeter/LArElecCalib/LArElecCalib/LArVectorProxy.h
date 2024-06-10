// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file LArElecCalib/LArVectorProxy.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2011
 * @brief Proxy for accessing a range of float values like a vector.
 *
 */


#ifndef LARELECCALIB_LARVECTORPROXY_H
#define LARELECCALIB_LARVECTORPROXY_H


#include <vector>
#include <span>
#include <cstdlib>


/**
 * @brief Proxy for accessing a range of float values like a vector.
 *
 * For the shape and OFC conditions data, the data for many channels
 * is flattened into a single vector.  But we still want to provide
 * a vector-like interface for the data for an individual channel.
 * 
 * We can do that with this proxy object, based on std::span. 
 * On top of std::span's interface, we implement methods that are 
 * used by existing clients.
 */


class LArVectorProxy :
  public std::span<const float>  {
  
public:
 
  /**
   * @brief Default constructor. 
   *        Creates the proxy in an invalid state.
   */
  LArVectorProxy();


  /**
   * @brief Construct a proxy referencing an existing vector.
   * @param vec The existing vector to reference.
   */
  LArVectorProxy (const std::vector<value_type>& vec);


  /**
   * @brief Construct a proxy referencing a range of vectors in memory.
   * @param beg Pointer to the start of the range.
   * @param end Pointer to the (exclusive) end of the range.
   */
  LArVectorProxy (const value_type* beg, const value_type* end);


  /**
   * @brief Test to see if the proxy has been initialized.
   */
  bool valid() const;

  /**
   * @brief Vector indexing with bounds check.
   */
  value_type at (size_t i) const;


  /**
   * @brief Convert back to a vector.
   */
  std::vector<value_type> asVector() const;
  
};


#include "LArElecCalib/LArVectorProxy.icc"


#endif // not LARELECCALIB__LARVECTORPROXY_H
