
/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
 */
#ifndef GEOMODELUTILITIES_TRANSIENTCONSTSHAREDPTR_H
#define GEOMODELUTILITIES_TRANSIENTCONSTSHAREDPTR_H

#include <memory>
namespace GeoModel {
    /// The TransientConstSharedPtr allows non-const access if the pointer itself
    /// is non-const but in the const case only a const pointer is returned
    /// The class takes the ownership of the object using a shared_ptr
    template <typename Obj> class TransientConstSharedPtr {
    public:
        template <typename ObjGrp> friend class GeoModel::TransientConstSharedPtr;
        /// Get (non-const) access to the underlying object
        Obj* get() { return m_ptr.get(); }
        Obj* operator->() { return get(); }
        /// Get const access to the underlying object
        const Obj* get() const { return m_ptr.get(); }
        const Obj* operator->() const { return get(); }
        /// Dereference operator
        const Obj& operator*() const { return *m_ptr; }
        Obj& operator*() { return *m_ptr; }

        /// Constructor from unique_ptr
        template <typename ObjGrp>
        TransientConstSharedPtr(std::unique_ptr<ObjGrp> ptr) : m_ptr{std::move(ptr)} {}
        /// Constructor from shared_ptr
        template <typename ObjGrp>
        TransientConstSharedPtr(std::shared_ptr<ObjGrp> ptr) : m_ptr{std::move(ptr)} {}
        /// Constructor from raw ptr
        TransientConstSharedPtr(Obj* ptr) : m_ptr{ptr} {}

        /// Standard constructor
        TransientConstSharedPtr() = default;
        /// Delete the copy constructor if the object is const
        template <typename ObjGrp>
        TransientConstSharedPtr(const TransientConstSharedPtr<ObjGrp>& other):
             m_ptr{other.m_ptr}{}
        /// Standard move constructor
        template <typename ObjGrp>
        TransientConstSharedPtr(TransientConstSharedPtr<ObjGrp>&& other):
            m_ptr{std::move(other.m_ptr)}{}

        /// Assignment operator
        template <typename ObjGrp>
        TransientConstSharedPtr& operator=(const TransientConstSharedPtr<ObjGrp>& other){
            if (&other != this) {
                m_ptr = other.m_ptr;
            }
            return *this;
        }
        template <typename ObjGrp>
        TransientConstSharedPtr& operator=(const std::shared_ptr<ObjGrp>& other) {
            m_ptr = other.m_ptr;
            return *this;
        }
        /// Move assignment operator
        template <typename ObjGrp>
        TransientConstSharedPtr& operator=(TransientConstSharedPtr<ObjGrp>&& other){
            m_ptr = std::move(other.m_ptr);
            return *this;
        }
        template <typename ObjGrp>
        TransientConstSharedPtr& operator=(std::unique_ptr<ObjGrp>&& other) {
            m_ptr = std::move(other);
            return *this;
        }
        template <typename ObjGrp>
        TransientConstSharedPtr& operator=(std::shared_ptr<ObjGrp>&& other) {
            m_ptr = std::move(other);
            return *this;
        }
        /// Overload the pointer
        template <typename ObjGrp>
        void reset(std::unique_ptr<ObjGrp> newObj) { m_ptr = std::move(newObj); }
        void reset() { m_ptr.reset(); }
        /// Release the memory
        std::shared_ptr<const Obj> release() { return std::move(m_ptr); }
        /// Is the pointer defined
        operator bool() const { return m_ptr.get() != nullptr; }
        bool operator!() const { return !m_ptr; }
        /// How many clients does the pointer have
        size_t use_count() const { return m_ptr.use_count(); }

        /// Smaller operator to insert the pointer into sets
        bool operator<(const TransientConstSharedPtr& other) const { return m_ptr.get() < other.m_ptr.get(); }

    private:
        std::shared_ptr<Obj> m_ptr{};
    };
}  // namespace GeoModel
#endif