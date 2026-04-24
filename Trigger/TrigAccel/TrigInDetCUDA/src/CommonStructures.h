/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGINDETCUDA_COMMON_H
#define TRIGINDETCUDA_COMMON_H

#include <tbb/tick_count.h>
#include <memory>

class WorkTimeStampQueueImpl;

typedef struct gpuParameters {
  int m_nSMX{};
  int m_nNUM_SMX_CORES{};
  int m_nNUM_TRIPLET_BLOCKS{};
} GPU_PARAMETERS;


class WorkTimeStamp {
public:
  WorkTimeStamp(unsigned int id, int ev, const tbb::tick_count& t) :
    m_workId(id), m_eventType(ev), m_time(t) {};
  WorkTimeStamp(const WorkTimeStamp& w) : m_workId(w.m_workId), m_eventType(w.m_eventType), m_time(w.m_time) {};
  unsigned int m_workId;
  int m_eventType;
  tbb::tick_count m_time;
};


class WorkTimeStampQueue{
public:
  WorkTimeStampQueue();
  ~WorkTimeStampQueue();
  void clear();
  size_t size() const;
  WorkTimeStamp& operator[]( size_t ndx );
  void push_back( const WorkTimeStamp& ts );
  

private:
  std::unique_ptr<WorkTimeStampQueueImpl> m_impl;
};

#endif
