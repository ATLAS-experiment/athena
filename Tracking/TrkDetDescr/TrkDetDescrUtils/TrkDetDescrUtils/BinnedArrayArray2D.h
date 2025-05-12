/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// BinnedArrayArray2D.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRUTILS_BINNEDARRAYARRAY2D_H
#define TRKDETDESCRUTILS_BINNEDARRAYARRAY2D_H

#include "TrkDetDescrUtils/BinUtility.h"
#include "TrkDetDescrUtils/BinnedArray2D.h"
// GaudiKernel
#include "GaudiKernel/GaudiException.h"
// Eigen
#include "GeoPrimitives/GeoPrimitives.h"
// STD
#include <vector>
#include <memory>

namespace Trk {
/** @class BinnedArrayArray2D2D
   A Binned Array of 2D Binned Arrays used for TRT.
   @author Andreas.Salzburger@cern.ch
   @autho Christos Anastopoulos AthenaMT modifications
   */

template<class T>
class BinnedArrayArray2D final : public BinnedArray<T>
{

public:

  BinnedArrayArray2D(std::vector< std::pair<BinnedArray2D<T>,  Amg::Vector3D > >&& tbas,
                   const BinUtility& bUtility)
      : m_binUtility(bUtility),
        m_binnedArrays(bUtility.bins(0)),
        m_arrayObjects() {
    // looping over the contained binned arrays so as to flatten
    for (auto& barray : tbas) {
      // flat ordered binned array
      auto index = m_binUtility.bin(barray.second, 0);
      m_binnedArrays[index] = std::move(barray.first);
    }
    buildCache();
  }

  /** Implicit constructor */
  virtual BinnedArrayArray2D* clone() const override
  {
    return new BinnedArrayArray2D(*this);
  }


 /** Returns the pointer to the templated class object from the
    BinnedArrayArray2D. It returns nullptr if not defined, takes local position */
 virtual T* object(const Amg::Vector2D& lp) const override {
   if (m_binUtility.inside(lp)) {
     const BinnedArray2D<T>& ba = m_binnedArrays[m_binUtility.bin(lp, 0)];
       return ba.object(lp);
   }
   return nullptr;
 }

  /** Returns the pointer to the templated class object from the
    BinnedArrayArray2D it returns nullptr if not defined, takes global position */
  virtual T* object(const Amg::Vector3D& gp) const override
  {
    if (m_binUtility.inside(gp)) {
      const BinnedArray2D<T>& ba = m_binnedArrays[m_binUtility.bin(gp, 0)];
      return ba.object(gp);
    }
    return nullptr;
  }

  /** Returns the pointer to the templated class object from the
   * BinnedArrayArray2D - entry point*/
  virtual T* entryObject(const Amg::Vector3D& gp) const override
  {
    return object(gp);
  }

  /** Returns the pointer to the templated class object from the
   * BinnedArrayArray2D, takes 3D position & direction */
  virtual T* nextObject(const Amg::Vector3D& gp,
                        const Amg::Vector3D&,
                        bool) const override
  {
    return object(gp);
  }

  /** Return all objects of the Array non-const T*/
  virtual BinnedArraySpan<T* const > arrayObjects() override final
  {
    return BinnedArraySpan<T* const >(m_arrayObjects.begin(), m_arrayObjects.end());
  }

  /** Return all objects of the Array const T*/
  virtual BinnedArraySpan<T const * const> arrayObjects() const override final
  {
    return BinnedArraySpan<T const* const>(m_arrayObjects.begin(), m_arrayObjects.end());
  }

  /** Number of Entries in the Array */
  virtual unsigned int arrayObjectsNumber() const override final
  {
    return m_arrayObjects.size();
  };

  /** Return the BinUtility*/
  virtual const BinUtility* binUtility() const override { return &m_binUtility; }


private:
 void buildCache() {
   // flattened array object
   size_t numArrrays = m_binnedArrays.size();
   //reserve num of Bin Arrays times num of Bins
   m_arrayObjects.reserve(numArrrays * m_binUtility.bins(0));
   for (size_t index = 0; index <numArrrays; ++index) {
     BinnedArraySpan<T* const> aObjects = m_binnedArrays[index].arrayObjects();
     for (auto& o : aObjects) {
       m_arrayObjects.push_back(o);
     }
   }
 }

 BinUtility m_binUtility {};
 std::vector<BinnedArray2D<T>> m_binnedArrays{};
 //cached view of the elements.  The element lifetimes
 //are managed by the 2D BinnedArrays
 std::vector<T*> m_arrayObjects {};
};
} // end of namespace Trk

#endif // TRKDETDESCRUTILS_BINNEDARRAYARRAY_H
