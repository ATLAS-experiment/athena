/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// NavBinnedArray1D.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRUTILS_NAVBINNEDARRAY1D_H
#define TRKDETDESCRUTILS_NAVBINNEDARRAY1D_H

#include "TrkDetDescrUtils/BinUtility.h"

// STL
#include <vector>
#include <memory>

class MsgStream;

namespace Trk {

/** @class NavBinnedArray1D

Avoiding a map search, the templated BinnedArray class can help
ordereing geometrical objects by providing a dedicated BinUtility.

For use within navigation objects, global coordinates/transform refer to
the position within mother navigation object

@author Andreas.Salzburger@cern.ch, Sarka.Todorova@cern.ch
@author Christos Anastopoulos (Athena MT modifications)
*/

template<class T>
class NavBinnedArray1D final : public BinnedArray<T>
{

public:
 // defaults, copy assignment can not be defaulted (CachedPtr)
 NavBinnedArray1D() = default;
 NavBinnedArray1D(NavBinnedArray1D&&) = default;
 NavBinnedArray1D& operator=(NavBinnedArray1D&&) = default;
 ~NavBinnedArray1D() = default;

 /**Constructor with std::vector and a  BinUtility - reference counted, will
 delete objects at the end, if this deletion should be turned off, the boolean
 deletion should be switched to false the global position is given by pointer
 and then deleted! */
 NavBinnedArray1D(const std::vector<std::shared_ptr<T>>& tclassvector,
                  const BinUtility& bingen, const Amg::Transform3D& transform)
     : BinnedArray<T>(),
       m_array{tclassvector},
       m_arrayObjects(nullptr),
       m_binUtility(bingen),
       m_transf(transform) {}

  /**Copy Constructor with shift */
  NavBinnedArray1D(const NavBinnedArray1D& barr,
                   std::vector<std::shared_ptr<T>>&& vec,
                   const Amg::Transform3D& shift)
    : BinnedArray<T>()
    , m_array(std::move(vec))
    , m_arrayObjects{}
    , m_binUtility(barr.m_binUtility)
    , m_transf(Amg::Transform3D(shift * (barr.m_transf)))
  {}

  /**Copy Constructor */
  NavBinnedArray1D(const NavBinnedArray1D& barr)
    : BinnedArray<T>()
    , m_array{barr.m_array}
    , m_arrayObjects(nullptr)
    , m_binUtility(barr.m_binUtility)
    , m_transf(barr.m_transf)
  {
  }

  /**Assignment operator*/
  NavBinnedArray1D& operator=(const NavBinnedArray1D& barr)
  {
    if (this != &barr) {
      m_arrayObjects.release();
      m_binUtility = barr.m_binUtility;
      // --------------------------------------------------------------------------
      m_array = (barr.m_array);
      m_transf = barr.m_transf;
    }
    return *this;
  }

  /** Implicit Constructor */
  NavBinnedArray1D* clone() const { return new NavBinnedArray1D(*this); }

  /** Returns the pointer to the templated class object from the BinnedArray,
  it returns nullptr if not defined;
  */
  T* object(const Amg::Vector2D& lp) const
  {
    if (m_binUtility.inside(lp)) {
      return (m_array[m_binUtility.bin(lp)]).get();
    }
    return nullptr;
  }
  /** Returns the pointer to the templated class object from the BinnedArray
  it returns nullptr if not defined;
  */
  T* object(const Amg::Vector3D& gp) const
  {
    // transform into navig.coordinates
    const Amg::Vector3D navGP((m_transf.inverse()) * gp);
    if (m_binUtility.inside(navGP)) {
      return (m_array[m_binUtility.bin(navGP)]).get();
    }
    return nullptr;
  }

  /** Returns the pointer to the templated class object from the BinnedArray -
   * entry point*/
  T* entryObject(const Amg::Vector3D&) const { return (m_array[0]).get(); }

  /** Returns the pointer to the templated class object from the BinnedArray
   */
  T* nextObject(const Amg::Vector3D& gp,
                const Amg::Vector3D& mom,
                bool associatedResult = true) const
  {
    // transform into navig.coordinates
    const Amg::Vector3D navGP((m_transf.inverse()) * gp);
    const Amg::Vector3D navMom((m_transf.inverse()).linear() * mom);
    // the bins
    size_t firstBin = m_binUtility.next(navGP, navMom, 0);
    // use the information of the associated result
    if (associatedResult) {
      if (firstBin <= m_binUtility.max(0)) {
        return (m_array[firstBin]).get();
      } else {
        return nullptr;
      }
    }
    // the associated result was 0 -> set to boundary
    firstBin = (firstBin < m_binUtility.bins(0))
                 ? firstBin
                 : m_binUtility.max(0);
    return (m_array[m_binUtility.bin(navGP)]).get();
  }

  /** Return all objects of the Array non-const T*/
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

  /** Return the BinUtility*/
  const BinUtility* binUtility() const { return &m_binUtility; }

  /** Return the transform*/
  const Amg::Transform3D* transform() const { return &m_transf; }

  /** Reposition */
  void updateTransform(Amg::Transform3D& transform)
  {
    m_transf = Amg::Transform3D(transform * m_transf);
  }

private:
  void createArrayCache() const
  {
    if (!m_arrayObjects) {
      auto arrayObjects = std::make_unique<std::vector<T*>>();
      for (unsigned int ill = 0; ill < m_array.size(); ++ill) {
        arrayObjects->push_back((m_array[ill]).get());
      }
      m_arrayObjects.set(std::move(arrayObjects));
    }
  }

  //!< vector of pointers to the class T
  std::vector<std::shared_ptr<T>> m_array;
  //!< 1D vector of cached not owning pointers to class T
  CxxUtils::CachedUniquePtr<std::vector<T*>> m_arrayObjects;
  //!< binUtility for retrieving and filling the Array
  BinUtility m_binUtility;
  // !< transform into local navigation coordinates
  Amg::Transform3D m_transf;
};

} // end of namespace Trk

#endif // TRKDETDESCRUTILS_NAVBINNEDARRAY1D_H
