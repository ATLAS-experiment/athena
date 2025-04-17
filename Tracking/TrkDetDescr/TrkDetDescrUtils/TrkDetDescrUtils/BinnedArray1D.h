/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// BinnedArray1D.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRUTILS_BINNEDARRAY1D_H
#define TRKDETDESCRUTILS_BINNEDARRAY1D_H

#include "TrkDetDescrUtils/BinUtility.h"
#include "TrkDetDescrUtils/BinnedArray.h"
#include "TrkDetDescrUtils/SharedObject.h"

// STL
#include <vector>

#include "CxxUtils/CachedUniquePtr.h"


namespace Trk {

/** @class BinnedArray1D

    1-dimensional binned arry based on a sorting
    given by the BinUtitlity.

   @author Andreas.Salzburger@cern.ch
   @author Christos Anastopoulos (Athena MT modifications)
   */

template<class T>
class BinnedArray1D final : public BinnedArray<T>
{

public:
 //defaults, copy assignment can not be defaulted (CachedPtr)
 BinnedArray1D() = default;
 BinnedArray1D(BinnedArray1D&&) = default;
 BinnedArray1D& operator=(BinnedArray1D&&) = default;
 ~BinnedArray1D() = default;
 /** ctors with arguments*/
 BinnedArray1D(
     const std::vector<std::pair<SharedObject<T>, Amg::Vector3D>>& tclassvector,
     const BinUtility& bingen)
     : BinnedArray<T>(),
       m_array{},
       m_arrayObjects(nullptr),
       m_binUtility(bingen) {
   initialize(tclassvector);
  }
  BinnedArray1D(
    const std::vector<std::pair<SharedObject<T>, Amg::Vector3D>>& tclassvector,
    BinUtility&& bingen)
    : BinnedArray<T>()
    , m_array{}
    , m_arrayObjects(nullptr)
    , m_binUtility(std::move(bingen))
  {
    initialize(tclassvector);
  }
  /**Copy */
  BinnedArray1D(const BinnedArray1D& barr)
    : BinnedArray<T>()
    , m_array{barr.m_array}
    , m_arrayObjects(nullptr)
    , m_binUtility(barr.m_binUtility)
  {
  }
  /**Assignment */
  BinnedArray1D& operator=(const BinnedArray1D& barr)
  {
    if (this != &barr) {
      m_binUtility = barr.m_binUtility;
      m_array.clear();
      m_arrayObjects.release();
      m_array = barr.m_array;
    }
    return *this;
  }
  /** Implicit Constructor */
  BinnedArray1D* clone() const { return new BinnedArray1D(*this); }

  /** Returns the pointer to the templated class object from the BinnedArray,
      it returns nullptr if not defined;
   */
  T* object(const Amg::Vector2D& lp) const
  {
    if (m_binUtility.inside(lp)) {
      return (m_array[m_binUtility.bin(lp, 0)]).get();
    }
    return nullptr;
  }

  /** Returns the pointer to the templated class object from the BinnedArray
      it returns nullptr if not defined;
   */
  T* object(const Amg::Vector3D& gp) const
  {
    return (m_array[m_binUtility.bin(gp, 0)]).get();
  }

  /** Returns the pointer to the templated class object from the BinnedArray -
   * entry point*/
  T* entryObject(const Amg::Vector3D& gp) const
  {
    return (m_array[m_binUtility.entry(gp, 0)]).get();
  }

  /** Returns the pointer to the templated class object from the BinnedArray
   */
  T* nextObject(const Amg::Vector3D& gp,
                const Amg::Vector3D& mom,
                bool associatedResult = true) const
  {
    // the bins
    size_t bin = associatedResult ? m_binUtility.bin(gp, 0)
                                  : m_binUtility.next(gp, mom, 0);
    return (m_array[bin]).get();
  }

  /** Return all objects of the Array non-const T*/
  BinnedArraySpan<T* const> arrayObjects()
  {
    createArrayCache();
    return Trk::BinnedArraySpan<T* const>(&*(m_arrayObjects->begin()), &*(m_arrayObjects->end()));
  }

  /** Return all objects of the Array const T*/
  BinnedArraySpan<T const * const > arrayObjects() const
  {
    createArrayCache();
    return Trk::BinnedArraySpan<const T* const>(&*(m_arrayObjects->begin()), &*(m_arrayObjects->end()));
  }

  /** Number of Entries in the Array */
  unsigned int arrayObjectsNumber() const { return arrayObjects().size(); }

  /** Return the BinUtility*/
  const BinUtility* binUtility() const { return &m_binUtility; }

private:
  void createArrayCache() const
  {
    if (!m_arrayObjects) {
      std::unique_ptr<std::vector<T*>> arrayObjects = std::make_unique<std::vector<T*>>();
      auto bins = m_binUtility.bins(0);
      arrayObjects->reserve(bins);
      for (size_t ill = 0; ill < bins; ++ill) {
        arrayObjects->push_back((m_array[ill]).get());
      }
      m_arrayObjects.set(std::move(arrayObjects));
    }
  }

  void initialize(const std::vector<std::pair<SharedObject<T>, Amg::Vector3D>>& tclassvector) {
    // prepare the binned Array
    size_t vecsize = tclassvector.size();
    m_array = std::vector<SharedObject<T>>(vecsize);
    for (size_t ivec = 0; ivec < vecsize; ++ivec) {
      const Amg::Vector3D currentGlobal(((tclassvector[ivec]).second));
      if (m_binUtility.inside(currentGlobal)) {
        m_array[m_binUtility.bin(currentGlobal, 0)] =
            ((tclassvector)[ivec]).first;
      } else
        throw GaudiException("BinnedArray1D constructor",
                             "Object outside bounds", StatusCode::FAILURE);
    }
  }
  //!< vector of pointers to the class T
  std::vector<SharedObject<T>> m_array{};
  //!< 1D vector of cached not owning pointers to class T
  CxxUtils::CachedUniquePtr<std::vector<T*>> m_arrayObjects{nullptr};
  //!< binUtility for retrieving and filling the Array
  BinUtility m_binUtility{};
};

} // end of namespace Trk

#endif // TRKSURFACES_BINNEDARRAY1D_H
