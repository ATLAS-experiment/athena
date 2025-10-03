/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//
// Templated class for the Hit collections in athena
// There is a bunch of ifdef __CINT__ to make this class
// intelligible to AthenaRoot and work out a persistency mechanism
//

#ifndef AthenaHitsVector_H
#define AthenaHitsVector_H
//
//
// vector class
#include <vector>

#include "AthContainers/tools/DVLInfo.h"
#include "boost/iterator/transform_iterator.hpp"

//
// Gaudi includes, not provided to rootcint
#ifndef __CINT__
#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#endif

namespace AthHitVec{
  enum OwnershipPolicy {
    OWN_ELEMENTS = 0,  ///< this data object owns its elements
    VIEW_ELEMENTS = 1  ///< this data object is a view, does not own its elmts
  };
}


struct HitsVectorBase {
  // This base class is used to store AthenaHitsVector
  // and AtlasHitsVector in the same container, avoiding std::any RTTI.
  // This should stay empty other than the virtual destructor which is required
  // when converting a std::unique_ptr<Derived> to std::unique_ptr<Base>.
  virtual ~HitsVectorBase() = default;
};

//
template <typename T>
class AthenaHitsVector : public HitsVectorBase {
 public:
  //
  // additional typedef
  using base_value_type = T;
  using CONT = std::vector<T*>;
  using value_type = typename CONT::value_type;
  using pointer = typename CONT::pointer;
  using reference = typename CONT::reference;
  using iterator = typename CONT::iterator;
  using size_type = typename CONT::size_type;
  using difference_type = typename CONT::difference_type;
  using const_pointer = const T* const*;
  using const_reference = const T* const&;

  struct make_const {
    const T* operator()(const T* x) const { return x; }
  };
  using const_iterator =
      boost::transform_iterator<make_const, typename CONT::const_iterator>;
#ifdef __CINT__
  // default constructor for rootcint
  AthenaHitsVector() {}
#else
  // methods not provided to rootcint
  AthenaHitsVector(const std::string& collectionName = "DefaultCollectionName",
                   AthHitVec::OwnershipPolicy ownPolicy = AthHitVec::OWN_ELEMENTS)
    : m_name (collectionName),
      m_ownPolicy (ownPolicy)
  {
    IMessageSvc* msgSvc(Athena::getMessageSvc());
    MsgStream log(msgSvc, "AthenaHitsVector");
    log << MSG::DEBUG << " initialized " << collectionName
        << " with ownership policy " << m_ownPolicy << endmsg;
  }
  ~AthenaHitsVector() override { Clear(); }

  void Clear() {
    // delete pointers if we own the elements
    if (m_ownPolicy == AthHitVec::OWN_ELEMENTS) {
      for (unsigned int i = 0; i < m_hitvector.size(); i++)
        delete m_hitvector[i];
    }
    m_hitvector.clear();
  }

  void Clear(AthHitVec::OwnershipPolicy ownPolicy) {
    // delete pointers if we own the elements
    if (m_ownPolicy == AthHitVec::OWN_ELEMENTS) {
      for (unsigned int i = 0; i < m_hitvector.size(); i++)
        delete m_hitvector[i];
    }
    m_hitvector.clear();
    m_ownPolicy = ownPolicy;
  }

  void Insert(T* h) { m_hitvector.push_back(h); }
  int Size() const { return size(); }
#endif  // __CINT__

  /// copy constructor makes deep copy of elements,
  /// as by default the container is AthHitVec::OWN_ELEMENTS
  explicit AthenaHitsVector(const AthenaHitsVector<T>& rhs) {
    m_hitvector.reserve(rhs.m_hitvector.size());
    const_iterator i(rhs.begin()), e(rhs.end());
    while (i != e) {
      m_hitvector.push_back((nullptr != *i) ? new T(**i) : nullptr);
      ++i;
    }
  }

  /// assignment deletes old elements and copies the new ones
  /// deep copy if AthHitVec::OWN_ELEMENTS shallow copy if VIEW_ELEMENTS
  // cppcheck-suppress operatorEqVarError; m_ownPolicy deliberately not copied
  AthenaHitsVector<T>& operator=(const AthenaHitsVector<T>& rhs) {
    if (this != &rhs) {
      this->Clear();
      if (this->m_ownPolicy == AthHitVec::OWN_ELEMENTS) {
        m_hitvector.reserve(rhs.m_hitvector.size());
        const_iterator i(rhs.begin()), e(rhs.end());
        while (i != e) {
          m_hitvector.push_back((nullptr != *i) ? new T(**i) : nullptr);
          ++i;
        }
      } else {
        this->m_hitvector = rhs.m_hitvector;
      }
    }
    return *this;
  }

  const std::string& Name() const { return m_name; }

  void setName(const std::string& name) { m_name = name; }
  //
  // vector methods.
  const std::vector<T*>& getVector() { return m_hitvector; }

  bool empty() const { return m_hitvector.empty(); }

  const_iterator begin() const {
    return const_iterator(m_hitvector.begin(), make_const());
  }

  const_iterator end() const {
    return const_iterator(m_hitvector.end(), make_const());
  }

  iterator begin() { return m_hitvector.begin(); }

  iterator end() { return m_hitvector.end(); }

  size_type size() const { return m_hitvector.size(); }

  void push_back(T* t) { m_hitvector.push_back(t); }
  void push_back(std::unique_ptr<T> t) { m_hitvector.push_back(t.release()); }

  const T* At(unsigned int pos) const { return m_hitvector.at(pos); }

  const T* operator[](size_type n) const { return m_hitvector[n]; }

  void resize(size_type sz) {
    if (sz < size()) {
      if (m_ownPolicy == AthHitVec::OWN_ELEMENTS) {
        iterator i(m_hitvector.begin() + sz), e(m_hitvector.end());
        while (i != e) {
          delete *i++;
        }
      }
      m_hitvector.resize(sz);
    } else {
      m_hitvector.insert(m_hitvector.end(), sz - m_hitvector.size(), nullptr);
    }
  }

  void clear() {
    if (m_ownPolicy == AthHitVec::OWN_ELEMENTS) {
      for (unsigned int i = 0; i < m_hitvector.size(); i++)
        delete m_hitvector[i];
    }
    m_hitvector.clear();
  }

  void reserve(size_type n) { m_hitvector.reserve(n); }

 protected:
  std::string m_name;
  std::vector<T*> m_hitvector;
  AthHitVec::OwnershipPolicy m_ownPolicy = AthHitVec::OWN_ELEMENTS;

 public:
  // Used to ensure that the DVLInfo gets registered
  // when the dictionary for this class is loaded.
  static const std::type_info* initHelper() {
    return DataModel_detail::DVLInfo<AthenaHitsVector<T> >::initHelper();
  };
  static const std::type_info* const s_info;
};

/**
 * @brief Construct a new container.
 * @param nreserve Number of elements for which to reserve space.
 *                 (Ignored if not appropriate.)
 *
 * This is broken out from the @c make method to allow specializing
 * just this method.
 */
template <class T>
void dvl_makecontainer(size_t nreserve, AthenaHitsVector<T>*& cont) {
  cont = new AthenaHitsVector<T>;
  cont->reserve(nreserve);
}

// Ensure that the DVLInfo gets registered
// when the dictionary for this class is loaded.
template <class T>
const std::type_info* const AthenaHitsVector<T>::s_info =
    AthenaHitsVector<T>::initHelper();

#endif
