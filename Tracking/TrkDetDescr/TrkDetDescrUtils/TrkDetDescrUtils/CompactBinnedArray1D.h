/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CompactBinnedArray1D.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRUTILS_COMPACTBINNEDARRAY1D_H
#define TRKDETDESCRUTILS_COMPACTBINNEDARRAY1D_H

#include "TrkDetDescrUtils/BinUtility.h"
#include "TrkDetDescrUtils/CompactBinnedArray.h"

// STL
#include <vector>

class MsgStream;

namespace Trk {

/** @class CompactBinnedArray1D

    1-dimensional binned arry based on a sorting
    given by the BinUtitlity.

   @author sarka.todorova@cern.ch
   @author Christos Anastopoulos (Athena MT modifications)
   */

template<class T>
class CompactBinnedArray1D final : public CompactBinnedArray<T>
{

public:
  CompactBinnedArray1D() = default;
  ~CompactBinnedArray1D() = default;

  /**Constructor with std::vector and a BinUtility */
  CompactBinnedArray1D(const std::vector<T*>& tclassvector,
                       const std::vector<size_t>& indexvector,
                       const BinUtility& bingen)
    : CompactBinnedArray<T>()
    , m_array(indexvector)
    , m_arrayObjects(tclassvector)
    , m_binUtility(bingen)
  {
    // maximal index must stay within the range of available objects
    unsigned int iMax = 0;
    for (unsigned int i = 0; i < indexvector.size(); i++){
      if (indexvector[i] > iMax){
        iMax = indexvector[i];
      }
    }
  }

  /**Copy Constructor - copies only pointers !*/
  CompactBinnedArray1D(const CompactBinnedArray1D& barr)
    : CompactBinnedArray<T>()
    , m_binUtility(barr.m_binUtility)
  {
    m_array = barr.m_array;
    m_arrayObjects = barr.m_arrayObjects;
  }
  /**Assignment operator*/
  CompactBinnedArray1D& operator=(const CompactBinnedArray1D& barr)
  {
    if (this != &barr) {
      // now refill
      m_binUtility = (barr.m_binUtility);
      m_array = barr.m_array;
      m_arrayObjects = barr.m_arrayObjects;
    }
    return *this;
  }
  /** Implicit Constructor */
  CompactBinnedArray1D* clone() const
  {
    return new CompactBinnedArray1D(m_arrayObjects, m_array, m_binUtility);
  }

  CompactBinnedArray1D* clone(const std::vector<T*>& ptrs) const
  {
    assert(ptrs.size() == m_arrayObjects.size());
    return new CompactBinnedArray1D(ptrs, m_array, m_binUtility);
  }

  /** Returns the pointer to the templated class object from the BinnedArray,
      it returns 0 if not defined;
   */
  T* object(const Amg::Vector2D& lp) const
  {
    if (m_binUtility.inside(lp)){
      return m_arrayObjects[m_array[m_binUtility.bin(lp, 0)]];
    }
    return nullptr;
  }

  /** Returns the pointer to the templated class object from the BinnedArray
      it returns 0 if not defined;
   */
  T* object(const Amg::Vector3D& gp) const
  {
    return m_arrayObjects[m_array[m_binUtility.bin(gp, 0)]];
  }

  /** Returns the pointer to the templated class object from the BinnedArray -
   * entry point*/
  T* entryObject(const Amg::Vector3D& gp) const
  {
    return (m_arrayObjects[m_array[m_binUtility.entry(gp, 0)]]);
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
    return m_arrayObjects[m_array[bin]];
  }

  /** Return all objects of the Array non const T*/
  BinnedArraySpan<T* const> arrayObjects()
  {
    return BinnedArraySpan<T* const>(m_arrayObjects.data(),
                                     m_arrayObjects.data() + m_arrayObjects.size());
  }

  /** Return all objects of the Array const T*/
  BinnedArraySpan<T const * const> arrayObjects() const
  {
    return BinnedArraySpan<T const * const>(m_arrayObjects.data(),
                                            m_arrayObjects.data() + m_arrayObjects.size());
  }

  /** Number of Entries in the Array */
  unsigned int arrayObjectsNumber() const { return m_arrayObjects.size(); }

  /** Return the BinUtility*/
  const BinUtility* binUtility() const { return &m_binUtility; }

  /** Return the BinUtility*/
  const BinUtility* layerBinUtility(const Amg::Vector3D&) const
  {
    return &m_binUtility;
  }

  /** Return the layer bin*/
  size_t layerBin(const Amg::Vector3D& pos) const
  {
    return m_binUtility.bin(pos);
  }

private:
  std::vector<size_t> m_array{};       //!< vector of indices to objects
  std::vector<T*> m_arrayObjects{};    //!< objects
  BinUtility m_binUtility{};          //!< binUtility
};

} // end of namespace Trk

#endif // TRKDETDESCRUTILS_COMPACTBINNEDARRAY1D_H
