/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// BinnedArray1D1D.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRUTILS_BINNEDARRAY1D1D_H
#define TRKDETDESCRUTILS_BINNEDARRAY1D1D_H

#include "TrkDetDescrUtils/BinUtility.h"
#include "TrkDetDescrUtils/BinnedArray.h"
#include "TrkDetDescrUtils/SharedObject.h"

#include "CxxUtils/CachedUniquePtr.h"
#include <vector>
#include <utility>

namespace Trk {

/** @class BinnedArray1D1D

    2D dimensional binned array, where the binning grid is
    not symmetric.
    One steering bin utility finds the associated array of the other.

    @author Andreas.Salzburger@cern.ch
    @author Christos Anastopoulos (Athena MT modifications)
 */

template<class T>
class BinnedArray1D1D final : public BinnedArray<T>
{

public:
 // defaults, copy assignment can not be defaulted (CachedPtr)
 BinnedArray1D1D() = default;
 BinnedArray1D1D(BinnedArray1D1D&&) = default;
 BinnedArray1D1D& operator=(BinnedArray1D1D&&) = default;
 ~BinnedArray1D1D() = default;
 /** ctors with arguments*/
 /** Constructor with std::vector and a  BinUtility */
 BinnedArray1D1D(
     const std::vector<std::pair<SharedObject<T>, Amg::Vector3D>>& tclassvector,
     const BinUtility& steeringBinGen1D,
     const std::vector<BinUtility>& singleBinGen)
     : BinnedArray<T>(),
       m_array{},
       m_arrayObjects(nullptr),
       m_steeringBinUtility(steeringBinGen1D),
       m_singleBinUtilities(singleBinGen) {
   initialize(tclassvector);
 }
 BinnedArray1D1D(
     const std::vector<std::pair<SharedObject<T>, Amg::Vector3D>>& tclassvector,
     BinUtility&& steeringBinGen1D,
     std::vector<BinUtility>&& singleBinGen)
     : BinnedArray<T>(),
       m_array{},
       m_arrayObjects(nullptr),
       m_steeringBinUtility(std::move(steeringBinGen1D)),
       m_singleBinUtilities(std::move(singleBinGen)) {
   initialize(tclassvector);
 }

 /** Copy Constructor */
 BinnedArray1D1D(const BinnedArray1D1D& barr)
     : BinnedArray<T>(),
       m_array{barr.m_array},
       m_arrayObjects(nullptr),
       m_steeringBinUtility(barr.m_steeringBinUtility),
       m_singleBinUtilities(barr.m_singleBinUtilities) {}

 /** Assignment operator */
 BinnedArray1D1D& operator=(const BinnedArray1D1D& barr) {
   if (this != &barr) {
     m_arrayObjects.release();
     m_steeringBinUtility = barr.m_steeringBinUtility;
     m_singleBinUtilities = barr.m_singleBinUtilities;
     m_array = barr.m_array;
   }
   return *this;
 }

  /** Implicit Constructor */
  BinnedArray1D1D* clone() const { return new BinnedArray1D1D(*this); }

  /** Returns the pointer to the templated class object from the BinnedArray,
      it returns 0 if not defined
   */
  T* object(const Amg::Vector2D& lp) const
  {
    int steerBin = m_steeringBinUtility.bin(lp, 0);
    int singleBin = std::as_const(m_singleBinUtilities)[steerBin].bin(lp, 0);
    return (m_array[steerBin][singleBin]).get();
  }

  /** Returns the pointer to the templated class object from the BinnedArray,
      it returns 0 if not defined
   */
  T* object(const Amg::Vector3D& gp) const
  {
    int steerBin = m_steeringBinUtility.bin(gp, 0);
    int singleBin = std::as_const(m_singleBinUtilities)[steerBin].bin(gp, 0);
    return (m_array[steerBin][singleBin]).get();
  }

  /** Returns the pointer to the templated class object from the BinnedArray -
   * entry point */
  T* entryObject(const Amg::Vector3D& gp) const
  {
    int steerBin = m_steeringBinUtility.entry(gp, 0);
    int singleBin = std::as_const(m_singleBinUtilities)[steerBin].entry(gp, 0);
    return (m_array[steerBin][singleBin]).get();
  }

  /** Returns the pointer to the templated class object from the BinnedArray */
  T* nextObject(const Amg::Vector3D&, const Amg::Vector3D&, bool) const
  {
    return nullptr;
  }

  /** Return all objects of the Array non-const T*/
  BinnedArraySpan<T* const> arrayObjects()
  {
    createArrayCache();
    return Trk::BinnedArraySpan<T* const>(&*(m_arrayObjects->begin()), &*(m_arrayObjects->end()));
  }

  /** Return all objects of the Array const T*/
  BinnedArraySpan<T const * const> arrayObjects() const
  {
    createArrayCache();
    return BinnedArraySpan<const T* const>(&*(m_arrayObjects->begin()), &*(m_arrayObjects->end()));
  }

  /** Number of Entries in the Array */
  unsigned int arrayObjectsNumber() const { return arrayObjects().size(); }

  /** Return the BinUtility - returns the steering binUtility in this case*/
  const BinUtility* binUtility() const { return &m_steeringBinUtility; }

private:
  void createArrayCache() const
  {
    if (!m_arrayObjects) {
      std::unique_ptr<std::vector<T*>> arrayObjects = std::make_unique<std::vector<T*>>();
      for (size_t isteer = 0; isteer < m_steeringBinUtility.bins(); ++isteer) {
        for (size_t isingle = 0;
             isingle < std::as_const(m_singleBinUtilities)[isteer].bins(); ++isingle) {
          arrayObjects->push_back((m_array[isteer][isingle]).get());
        }
      }
      m_arrayObjects.set(std::move(arrayObjects));
    }
  }

  void initialize(const std::vector<std::pair<SharedObject<T>, Amg::Vector3D>>& tclassvector) {
    m_array = std::vector<std::vector<SharedObject<T>>>(m_steeringBinUtility.bins());
    for (size_t i = 0; i < m_steeringBinUtility.bins(); ++i) {
      size_t sizeOfSubBin = ((m_singleBinUtilities)[i]).bins();
      m_array[i] = std::vector<SharedObject<T>>(sizeOfSubBin);
    }
    // fill the Volume vector into the array
    int vecsize(tclassvector.size());
    for (int ivec = 0; ivec < vecsize; ++ivec) {
      const Amg::Vector3D currentGlobal((tclassvector[ivec]).second);
      if (m_steeringBinUtility.inside(currentGlobal)) {
        int steeringBin = m_steeringBinUtility.bin(currentGlobal, 0);
        int singleBin = (m_singleBinUtilities[steeringBin]).bin(currentGlobal, 0);
        std::vector<SharedObject<T>>& curVec = m_array[steeringBin];
        curVec[singleBin] = ((tclassvector)[ivec]).first;
      } else {
        throw GaudiException("BinnedArray1D1D", "Object outside bounds",
                             StatusCode::FAILURE);
      }
    }
  }

  //!< vector of pointers to the class T
  std::vector<std::vector<SharedObject<T>>> m_array{};
  //!< forced 1D vector of pointers to class T
  CxxUtils::CachedUniquePtr<std::vector<T*>> m_arrayObjects{nullptr};
  //!< binUtility for retrieving and filling the Array
  BinUtility m_steeringBinUtility{};
  std::vector<BinUtility> m_singleBinUtilities{}; //!< single bin utilities
};
} // end of namespace Trk

#endif // TRKSURFACES_BINNEDARRAY1D1D_H
