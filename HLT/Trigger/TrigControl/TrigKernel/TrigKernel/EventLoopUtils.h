/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGSERVICES_EVENTLOOPUTILS_H
#define TRIGSERVICES_EVENTLOOPUTILS_H
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

namespace HLT {
  /// Helper class to manage a long-running thread (duration of event loop)
  class LoopThread {
  private:
    /// Condition for which the thread waits most of its lifetime
    std::condition_variable m_cond;
    /// Mutex used to notify the condition
    std::mutex m_mutex;
    /// The thread's inner while-loop condition variable
    bool m_keepRunning{true};
    /// Flag whether the main loop of the thread has started and will listen to further notifications
    bool m_started{false};
    /// Flag whether the main loop of the thread has finished
    bool m_finished{false};
    /// The callback executed in each step of the thread's inner while-loop
    std::function<void()> m_callback;
    /// If positive, call the callback periodically with this interval regardless of the m_cond
    int m_callbackIntervalMilliseconds{-1};
    /// The thread object
    std::unique_ptr<std::thread> m_thread;

    /// Helper to wait for the condition
    inline void waitForCond(std::unique_lock<std::mutex>& lock) {
      if (m_callbackIntervalMilliseconds < 0) {
        m_cond.wait(lock);
      } else {
        m_cond.wait_for(lock, std::chrono::milliseconds(m_callbackIntervalMilliseconds));
      }
    }

    /// Main function executed by the thread
    void run() {
      if (!m_keepRunning) {return;}
      std::unique_lock<std::mutex> lock{m_mutex};

      // first call outside the loop to set the m_started flag
      waitForCond(lock);
      m_started = true;
      m_callback();

      // subsequent calls in a loop
      while(m_keepRunning) {
        waitForCond(lock);
        m_callback();
      }
      m_finished = true;
    }

  public:
    explicit LoopThread(std::function<void()>&& callback, int callbackInterval=-1)
    : m_callback(std::move(callback)),
      m_callbackIntervalMilliseconds(callbackInterval),
      m_thread(std::make_unique<std::thread>([this]{run();})) {}

    ~LoopThread() {
      wait();
    }

    // Copy and move not allowed
    LoopThread(const LoopThread&) = delete;
    LoopThread(LoopThread&&) = delete;
    LoopThread& operator=(const LoopThread&) = delete;
    LoopThread& operator=(LoopThread&&) = delete;

    /// Keep notifying the thread until the callback is called for the first time
    /// (returns just before calling the callback)
    void start() {
      while (!m_started) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        m_cond.notify_one();
      }
    }

    /// Flag the main loop to finish
    void stop() {
      m_keepRunning=false;
      m_cond.notify_all();
    }

    /// Wait until main loop finishes
    void wait() {
      // Nothing to do if thread already finished
      if (m_thread==nullptr || !m_thread->joinable()) {return;}
      // Keep notifying the condition until the loop finishes
      while (!m_finished) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        m_cond.notify_all();
      }
      // Wait for the thread to return
      m_thread->join();
    }

    std::condition_variable& cond() {return m_cond;}
    std::mutex& mutex() {return m_mutex;}
  };
} // namespace HLT

#endif // TRIGSERVICES_EVENTLOOPUTILS_H
