// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//


#ifndef ATHCUDASERVICES_STREAMPOOL_H
#define ATHCUDASERVICES_STREAMPOOL_H


#include "AthCUDACore/Macros.cuh"
#include <memory>


namespace AthCUDA {


class StreamPoolImpl;
class StreamPool
{
public:
  using ptr = void*;
  StreamPool();
  ~StreamPool();
  bool empty() const;
  ptr pop();
  void push( const ptr& s );


private:
  std::unique_ptr<StreamPoolImpl> m_impl;
};


} // namespace AthCUDA


#endif // not ATHCUDASERVICES_STREAMPOOL_H
