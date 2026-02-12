/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef EXPRESSIONPARSING_POINTERCACHE_H
#define EXPRESSIONPARSING_POINTERCACHE_H
#include <atomic>
#include <memory>

namespace ExpressionParsing {
   /// @brief Helper class to cache a non owning or owning pointer in an atomic way.
   template <class T, std::memory_order MEMORY_ORDER>
   class PointerCache
   {
   public:
      PointerCache() = default;
      ~PointerCache() {
         if (m_owner) {
            delete m_ptr.load();
         }
      }

      /// @brief set a pointer without handing over ownership
      void setIfUnset(const T &obj) {
         setPointer(&obj, false);
      }
      /// @brief set a pointer andhand over ownership
      void setIfUnset(std::unique_ptr<T> &&ptr) {
         setPointer(ptr.release(), true);
      }
      /// @brief set whether a valid pointer is set
      operator bool() const {
         return m_ptr != nullptr;
      }
      /// @brief dereference the pointer
      /// @note result is undefined if the pointer was not set and points to a valid object of type T
      const T *operator->() const {
         assert (m_ptr);
         return m_ptr.load(MEMORY_ORDER);
      }

   private:
      void setPointer(const T *ptr, bool owner) {
         while (m_ptr == nullptr) {
            const T *expected=nullptr;
            if (m_ptr.compare_exchange_weak(expected, ptr, MEMORY_ORDER)) break;
         }
         if (m_ptr == ptr) {
            m_owner = owner;
         }
         else {
            if (owner) {
               delete ptr;
            }
         }
      }
      std::atomic<const T *> m_ptr;
      std::atomic<bool> m_owner;
   };
}
#endif
