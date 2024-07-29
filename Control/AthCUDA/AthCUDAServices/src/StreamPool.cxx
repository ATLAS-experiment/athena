//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//


#include "StreamPool.h"

// TBB include(s).
#include <tbb/concurrent_queue.h>


namespace AthCUDA {


class StreamPoolImpl
{
public:
  tbb::concurrent_bounded_queue< StreamPool::ptr > m_streams;
};



StreamPool::StreamPool()
{
}



StreamPool::~StreamPool()
{
}


bool StreamPool::empty() const
{
  return m_impl->m_streams.empty();
}


StreamPool::ptr StreamPool::pop()
{
  ptr s = nullptr;
  m_impl->m_streams.pop( s );
  return s;
}


void StreamPool::push( const ptr& s )
{
  m_impl->m_streams.push( s );
}


} // namespace AthCUDA
