// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file AthenaKernel/DataBucketTrait.h
 * @author scott snyder
 * @date Mar, 2008
 * @brief Forward declaration of DataBucketTrait template.
 */


#ifndef ATHENAKERNEL_DATABUCKETTRAITFWD_H
#define ATHENAKERNEL_DATABUCKETTRAITFWD_H


namespace SG {


  /**
   * @brief Metafunction to find the proper @c DataBucket class for @c T.
   *
   * See in StorableConversions.h for full documentation.
   */
  template <class T, class U = T>
  struct DataBucketTrait;


} // namespace SG


#endif // not ATHENAKERNEL_DATABUCKETTRAITFWD_H
