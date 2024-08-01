/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "CommonStructures.h"
#include "tbb/concurrent_vector.h"


class WorkTimeStampQueueImpl
{
public:
  tbb::concurrent_vector<WorkTimeStamp> m_timeLine;
};


WorkTimeStampQueue::WorkTimeStampQueue()
  : m_impl (std::make_unique<WorkTimeStampQueueImpl>())
{
}


WorkTimeStampQueue::~WorkTimeStampQueue()
{
}


void WorkTimeStampQueue::clear()
{
  m_impl->m_timeLine.clear();
}


size_t WorkTimeStampQueue::size() const
{
  return m_impl->m_timeLine.size();
}


WorkTimeStamp& WorkTimeStampQueue::operator[]( size_t ndx )
{
  return m_impl->m_timeLine[ndx];
}


void WorkTimeStampQueue::push_back( const WorkTimeStamp& ts )
{
  m_impl->m_timeLine.push_back( ts );
}


