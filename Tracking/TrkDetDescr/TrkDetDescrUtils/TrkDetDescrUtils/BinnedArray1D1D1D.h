/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// BinnedArray1D1D1D.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRUTILS_BINNEDARRAY1D1D1D_H
#define TRKDETDESCRUTILS_BINNEDARRAY1D1D1D_H
//
#include "TrkDetDescrUtils/BinUtility.h"
#include "TrkDetDescrUtils/BinnedArray.h"
#include "CxxUtils/CachedUniquePtr.h"
//
// STL
#include <cassert>
#include <vector>
#include <utility>
#include <memory>
//
class MsgStream;
//
namespace Trk {

/** @class BinnedArray1D1D1D
    Avoiding a map search, the templated BinnedArray class can help
    ordering geometrical objects by providing a dedicated BinUtility.
    dedicated for 3-dim (non-eq) binning ; allows variable binning in 1 coordinate

    @author Sarka.Todorova@cern.ch
    @author Christos Anastopoulos (Athena MT modifications)
*/

template<class T>
class BinnedArray1D1D1D final : public BinnedArray<T>
{
public:
 //defaults, copy assignment can not be defaulted (CachedPtr)
 BinnedArray1D1D1D() = default;
 BinnedArray1D1D1D(BinnedArray1D1D1D&&) = default;
 BinnedArray1D1D1D& operator=(BinnedArray1D1D1D&&) = default;
 ~BinnedArray1D1D1D() = default;

 /**ctors */
 BinnedArray1D1D1D(
     const std::vector<std::pair<std::shared_ptr<T>, Amg::Vector3D>>& tclassvector,
     const BinUtility& binUtil1, const BinUtility& binUtil2,
     const std::vector<std::vector<BinUtility>>& binUtilVec)
     : BinnedArray<T>(),
       m_array{},
       m_arrayObjects(nullptr),
       m_binUtil1(binUtil1),
       m_binUtil2(binUtil2),
       m_binUtilArray(binUtilVec) {
   initialize(tclassvector);
 }
 BinnedArray1D1D1D(
     const std::vector<std::pair<std::shared_ptr<T>, Amg::Vector3D>>& tclassvector,
     BinUtility&& binUtil1,
     BinUtility&& binUtil2,
     std::vector<std::vector<BinUtility>>&& binUtilVec)
     : BinnedArray<T>(),
       m_array{},
       m_arrayObjects(nullptr),
       m_binUtil1(std::move(binUtil1)),
       m_binUtil2(std::move(binUtil2)),
       m_binUtilArray(std::move(binUtilVec)) {
   initialize(tclassvector);
 }

 /** Copy */
 BinnedArray1D1D1D(const BinnedArray1D1D1D& barr)
     : BinnedArray<T>(),
       m_array{barr.m_array},
       m_arrayObjects(nullptr),
       m_binUtil1(barr.m_binUtil1),
       m_binUtil2(barr.m_binUtil2),
       m_binUtilArray(barr.m_binUtilArray) {}

 /** Assignment operator */
 BinnedArray1D1D1D& operator=(const BinnedArray1D1D1D& barr) {
   if (this != &barr) {
     // bin utilities
     m_binUtil1 = barr.m_binUtil1;
     m_binUtil2 = barr.m_binUtil2;
     m_binUtilArray = barr.m_binUtilArray;
     m_array = barr.m_array;
     m_arrayObjects.release();
    }
    return *this;
  }
  /** Implicit Constructor */
  BinnedArray1D1D1D* clone() const { return new BinnedArray1D1D1D(*this); }

  /** Returns the pointer to the templated class object from the BinnedArray,
      it returns nullptr if not defined
   */
  T* object(const Amg::Vector2D& lp) const
  {
    if (!m_binUtil1.inside(lp) || !m_binUtil2.inside(lp)){
      return nullptr;
    }
    int bin1 = m_binUtil1.bin(lp);
    int bin2 = m_binUtil2.bin(lp);
    if (!std::as_const(m_binUtilArray)[bin1][bin2].inside(lp)){
      return nullptr;
    }
    int bin3 = std::as_const(m_binUtilArray)[bin1][bin2].bin(lp);

    return (m_array[bin1][bin2][bin3]).get();
  }

  /** Returns the pointer to the templated class object from the BinnedArray,
      it returns nullptr if not defined
   */
  T* object(const Amg::Vector3D& gp) const
  {
    if (!m_binUtil1.inside(gp) || !m_binUtil2.inside(gp)){
      return nullptr;
    }
    int bin1 = m_binUtil1.bin(gp);
    int bin2 = m_binUtil2.bin(gp);
    if (!std::as_const(m_binUtilArray)[bin1][bin2].inside(gp)){
      return nullptr;
    }
    unsigned int bin3 = std::as_const(m_binUtilArray)[bin1][bin2].bin(gp);
    if (bin3 >= std::as_const(m_binUtilArray)[bin1][bin2].bins()){
      return nullptr;
    }
    return (m_array[bin1][bin2][bin3]).get();
  }

  /** Returns the pointer to the templated class object from the BinnedArray -
   * entry point */
  T* entryObject(const Amg::Vector3D& gp) const
  {
    int bin1 = m_binUtil1.entry(gp, 0);
    int bin2 = m_binUtil2.entry(gp, 0);
    int bin3 = std::as_const(m_binUtilArray)[bin1][bin2].entry(gp, 0);
    return (m_array[bin1][bin2][bin3]).get();
  }

  /** Returns the pointer to the templated class object from the BinnedArray
      dummy for multidimensional arrays      */
  T* nextObject(const Amg::Vector3D&, const Amg::Vector3D&, bool) const
  {
    return nullptr;
  }

  /** Return all objects of the Array const T*/
  BinnedArraySpan<T* const> arrayObjects()
  {
    createArrayCache();
    return BinnedArraySpan<T* const>(&*(m_arrayObjects->begin()), &*(m_arrayObjects->end()));
  }

  /** Return all objects of the Array const T*/
  BinnedArraySpan<T const * const> arrayObjects() const
  {
    createArrayCache();
    return BinnedArraySpan<T const * const>(&*(m_arrayObjects->begin()), &*(m_arrayObjects->end()));
  }

  /** Number of Entries in the Array */
  unsigned int arrayObjectsNumber() const { return arrayObjects().size(); }

  /** Return the BinUtility - returns the first binUtility in this case*/
  const BinUtility* binUtility() const { return &m_binUtil1; }

private:
 void createArrayCache() const {
   if (!m_arrayObjects) {
     std::unique_ptr<std::vector<T*>> arrayObjects = std::make_unique<std::vector<T*>>();
     for (size_t ibin1 = 0; ibin1 < m_binUtil1.bins(); ++ibin1) {
       for (size_t ibin2 = 0; ibin2 < m_binUtil2.bins(); ++ibin2) {
         for (size_t ibin3 = 0; ibin3 < std::as_const(m_binUtilArray)[ibin1][ibin2].bins(); ++ibin3) {
           arrayObjects->push_back((m_array[ibin1][ibin2][ibin3]).get());
         }
       }
     }
     m_arrayObjects.set(std::move(arrayObjects));
   }
 }

  void initialize(const std::vector<std::pair<std::shared_ptr<T>, Amg::Vector3D>>& tclassvector) {
    int v1Size = m_binUtil1.bins();
    int v2Size = m_binUtil2.bins();
    m_array = std::vector<std::vector<std::vector<std::shared_ptr<T>>>>(v1Size);
    for (int i = 0; i < v1Size; ++i) {
      m_array[i] = std::vector<std::vector<std::shared_ptr<T>>>(v2Size);
      for (int j = 0; j < v2Size; ++j) {
        m_array[i][j] = std::vector<std::shared_ptr<T>>(m_binUtilArray[i][j].bins());
      }
    }
    // fill the Volume vector into the array
    int vecsize(tclassvector.size());
    for (int ivec = 0; ivec < vecsize; ++ivec) {
      Amg::Vector3D currentGlobal((tclassvector[ivec]).second);
      if (m_binUtil1.inside(currentGlobal) && m_binUtil2.inside(currentGlobal)) {
        int bin1 = m_binUtil1.bin(currentGlobal);
        int bin2 = m_binUtil2.bin(currentGlobal);
        int bin3 = m_binUtilArray[bin1][bin2].bin(currentGlobal);
        std::vector<std::vector<std::shared_ptr<T>>>& currArr = m_array[bin1];
        std::vector<std::shared_ptr<T>>& curVec = currArr[bin2];
        curVec[bin3] = (tclassvector[ivec]).first;
      } else
        throw GaudiException("BinnedArray1D1D1D", "Object outside bounds",
                             StatusCode::FAILURE);
    }
  }

  //!< vector of pointers to the class T
  std::vector<std::vector<std::vector<std::shared_ptr<T>>>> m_array{};
  //!<  1D cache of non owning pointers to class T
  CxxUtils::CachedUniquePtr<std::vector<T*>> m_arrayObjects{nullptr};
  BinUtility m_binUtil1{}; //!< binUtility for retrieving and filling the Array
  BinUtility m_binUtil2{}; //!< binUtility for retrieving and filling the Array
  //!< binUtility for retrieving and filling the Array
  std::vector<std::vector<BinUtility>> m_binUtilArray{};
};
} // end of namespace Trk
#endif // TRKSURFACES_BINNEDARRAY1D1D1D_H
