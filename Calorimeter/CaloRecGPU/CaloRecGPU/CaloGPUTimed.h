//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_CALOGPUTIMED_H
#define CALORECGPU_CALOGPUTIMED_H

#include <vector>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <fstream>
#include "CxxUtils/checker_macros.h"

#include <GaudiKernel/IProperty.h>

/**
 * @class CaloGPUTimed
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 01 June 2022
 * @brief Base class to provide some basic common infrastructure for timing measurements... */

class CaloGPUTimed
{

 protected:


  /** @brief Mutex that is locked when recording times.
   */
  mutable std::shared_mutex m_timeMutex;
  /** @brief Vector to hold execution times to be recorded if necessary.
   */
  mutable std::vector<size_t> m_times ATLAS_THREAD_SAFE;
  //Mutexes should ensure no problems with thread safety.

  /** @brief Vector to hold the event numbers to be recorded if necessary.
   */
  mutable std::vector<size_t> m_eventNumbers ATLAS_THREAD_SAFE;
  //Mutexes should ensure no problems with thread safety.

  /** @brief If @p true, times are recorded to the file given by @p m_timeFileName.
   *  Defaults to @p false.
   */
  Gaudi::Property<bool> m_measureTimes;

  /** @brief File to which times should be saved.
   */
  Gaudi::Property<std::string> m_timeFileName;

  //Use CaloGPUTimed(this) in the derived classes for everything to work.

  template <class T>
  CaloGPUTimed(T * ptr):
    m_measureTimes(ptr, "MeasureTimes", false, "Save time measurements"),
    m_timeFileName(ptr, "TimeFileOutput", "times.txt", "File to which time measurements should be saved")
  {
  }


 private:

  inline void record_times_helper(size_t) const
  {
    //Do nothing
  }


  inline void record_times_helper(size_t index, size_t t) const
  {
    // coverity[missing_lock]
    m_times[index] = t;
    
    //This is called within a function that holds the lock itself.
  }

  template <class ... Args>
  inline void record_times_helper(size_t index, size_t t, Args && ... args) const
  {
    record_times_helper(index, t);
    record_times_helper(index + 1, std::forward<Args>(args)...);
  }

 protected:

  inline void record_times(const size_t event_num, const std::vector<size_t> & times) const
  {
    size_t old_size;
    
    {
      std::unique_lock<std::shared_mutex> lock(m_timeMutex);
      old_size = m_times.size();
      m_times.resize(old_size + times.size());
      m_eventNumbers.push_back(event_num);
    }
    
    {
      std::shared_lock<std::shared_mutex> lock(m_timeMutex);
      for (size_t i = 0; i < times.size(); ++i)
        {
          m_times[old_size + i] = times[i];
        }
    }
  }

  template <class ... Args>
  inline void record_times(const size_t event_num, const size_t & value) const
  {
    const size_t time_size = 1;

    size_t old_size;

    {
      std::unique_lock<std::shared_mutex> lock(m_timeMutex);
      old_size = m_times.size();
      m_times.resize(old_size + time_size);
      m_eventNumbers.push_back(event_num);
    }

    {
      std::shared_lock<std::shared_mutex> lock(m_timeMutex);
      record_times_helper(old_size, value);
    }
  }

  template <class ... Args>
  inline void record_times(const size_t event_num, const size_t & value, Args && ... args) const
  {
    const size_t time_size = sizeof...(args) + 1;

    size_t old_size;

    {
      std::lock_guard<std::shared_mutex> lock_guard(m_timeMutex);
      old_size = m_times.size();
      m_times.resize(old_size + time_size);
      m_eventNumbers.push_back(event_num);
    }

    {
      std::shared_lock<std::shared_mutex> lock(m_timeMutex);
      record_times_helper(old_size, value, std::forward<Args>(args)...);
    }

  }

  inline void print_times(const std::string & header, const size_t time_size) const
  {
    std::unique_lock<std::shared_mutex> lock(m_timeMutex);
    //In principle, the lock wouldn't be needed
    //since this is meant to be called during the finalize.
    //Since we are not particularly concerned with performance here,
    //we will lock just to be sure.
    
      
    if (m_timeFileName.size() == 0)
      {
        return;
      }
    
    std::vector<size_t> indices(m_eventNumbers.size());    
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b)
    {
      return m_eventNumbers[a] < m_eventNumbers[b];
    }
             );
    std::ofstream out(m_timeFileName);

    out << "Event_Number Total " << header << "\n";

    for (const size_t idx : indices)
      {
        out << m_eventNumbers[idx] << " ";

        size_t total = 0;

        for (size_t i = 0; i < time_size; ++i)
          {
            total += m_times[idx * time_size + i];
          }

        out << total << " ";

        for (size_t i = 0; i < time_size; ++i)
          {
            out << m_times[idx * time_size + i] << " ";
          }
        out << "\n";
      }

    out << std::endl;

    out.close();
  }
};


#endif //CALORECGPU_CALOGPUTIMED_H
