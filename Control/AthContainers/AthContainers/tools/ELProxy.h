// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/tools/ELProxy.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2024
 * @brief Helpers for proxying ElementLink for PackedLink accessors.
 */


#ifndef ATHCONTAINERS_ELPROXY_H
#define ATHCONTAINERS_ELPROXY_H


#include "AthContainers/AuxVectorData.h"
#include "AthContainers/tools/PackedLinkVectorFactory.h"
#include "AthContainers/tools/PackedLinkConversions.h"
#include "AthContainers/tools/AuxDataTraits.h"
#include "AthLinks/DataLink.h"
#include "AthLinks/ElementLink.h"
#include "CxxUtils/range_with_at.h"
#include "CxxUtils/checker_macros.h"
#include <functional>
#include <ranges>
#include <concepts>
#include <vector>


namespace SG { namespace detail {


/**
 * @brief Base class for @c ELProxyT, with the converter held by value.
 */
template <class CONT>
class ELProxyValBase
{
public:
  /// The Storable object referred to by links.
  using Cont_t = CONT;


  /**
   * @brief Constructor.
   * @param cnv Converter to copy.
   */
  ELProxyValBase (PackedLinkConverter<CONT>& cnv);


   /**
   * @brief Constructor.
   * @param container Container holding the variables.
   * @param auxid The ID of the PackedLink variable.
   * @param linked_auxid The ID of the linked DataLinks.
   */
  ELProxyValBase (AuxVectorData& container,
                  SG::auxid_t auxid,
                  SG::auxid_t linked_auxid);


protected:
  /// The contained converter.
  PackedLinkConverter<CONT> m_cnv;
};


/**
 * @brief Base class for @c ELProxyT, with the converter held by reference.
 */
template <class CONT>
class ELProxyRefBase
{
public:
  /// The Storable object referred to by links.
  using Cont_t = CONT;

  /**
   * @brief Constructor.
   * @param cnv Converter to reference.
   */
  ELProxyRefBase (PackedLinkConverter<CONT>& cnv);


protected:
  /// Converter reference.
  PackedLinkConverter<CONT>& m_cnv;
};


/**
 * @brief Proxy for ElementLink.
 *
 * We want to be able to do things like:
 *@code
 *  SG::Accessor<SG::PackedLink<CONT> > acc ("plink");
 *  ElementLink<CONT> el = acc(*e);
 *  acc(*e) = el2;
 @endcode
 *
 * That means that the @c Accessor cannot return an @c ElementLink directly.
 * Rather, it needs to return a proxy object.  This proxy should both
 * be convertible to an @c ElementLink and be able to be assigned from
 * an @c ElementLnk.  In the latter case, it should then update the underlying
 * @c PackedLink value.
 *
 * This object can be used as such a proxy.  It holds a reference to the
 * @c PackedLink and also (via the base class) an appropriate converter.
 * It handles conversions to @c ElementLink and assignment from @c ElementLink.
 * It also proxies all the const methods of @c ElementLink.
 *
 * Now, in the case where we have an proxy by itself, we want the
 * proxy to hold a converter by value.  But if the proxy is part of a span,
 * then we want it to have just a reference to the converter, with the
 * converter itself being stored in the span (that would be in the
 * @c ELSpanProxy object below).  That's not just to save space --- the
 * converter holds a span over the @c DataLinks, and if that changes,
 * we want it visible by all members of the proxy span.  So how we hold
 * the converter is factored out into the @c BASE class, which will
 * be one of the two classes above.
 */
template <class BASE>
class ELProxyT
  : public BASE
{
public:
  /// Get the Storable from the base class.
  using Cont_t = typename BASE::Cont_t;

  /// The @c ElementLink class we provide.
  using Link_t = ElementLink<Cont_t>;

  /// The @c PackedLink class we proxy.
  using PLink_t = SG::PackedLink<Cont_t>;

  /// Don't allow these proxy objects to be copied.
  ELProxyT (const ELProxyT&) = delete;
  ELProxyT& operator= (const ELProxyT&) = delete;


  /**
   * @brief Constructor.
   * @param pl @c PackedLink object to proxy.
   * @param cnv Converter to use.
   */
  ELProxyT (PLink_t& pl, PackedLinkConverter<Cont_t>& cnv);


  /**
   * @brief Constructor.
   * @param container Container holding the variables.
   * @param auxid The ID of the PackedLink variable.
   * @param linked_auxid The ID of the linked DataLinks.
   *
   * This constructor can only be used if the converter is being
   * held by value.
   */
  ELProxyT (PLink_t& pl, AuxVectorData& container,
            SG::auxid_t auxid,
            SG::auxid_t linked_auxid);


  /**
   * @brief Convert the held @c PackedLink to an @c ElementLink.
   */
  operator const Link_t() const;


  /**
   * @brief Update the held @c PackedLink from an @c ElementLink.
   * @param link The @c ElementLink from which to update.
   */
  const Link_t operator= (const Link_t& link);


  /**
   * @brief Equality comparison with ElementLink.
   * @param l The link with which to compare.
   *
   * (Default conversions don't suffice to make this work.)
   */
  bool operator== (const Link_t& l) const;


  /**
   * @brief Equality comparison with another proxy.
   * @param l The proxy with which to compare.
   *
   * (Default conversions don't suffice to make this work.)
   */
  bool operator== (const ELProxyT& p) const;


  // nb. We don't proxy the implicit converison of ElementLink
  // to a pointer --- that can end up matching in places where we don't
  // want it to.


  // Proxy const methods of @c ElementLink.
#define ELPROXY(M) auto M() const { return this->m_cnv (m_pl).M(); }
  ELPROXY(getDataPtr)
  ELPROXY(getDataNonConstPtr)
  ELPROXY(getDataLink)
  ELPROXY(getStorableObjectPointer)
  ELPROXY(getStorableObjectRef)
  ELPROXY(cptr)
  ELPROXY(operator*)
  ELPROXY(operator->)
  ELPROXY(operator!)
  ELPROXY(isvalid)
  ELPROXY(cachedElement)
#ifndef XAOD_STANDALONE
  ELPROXY(isDefaultIndex)
#endif
  ELPROXY(hasCachedElement)
  ELPROXY(isDefault)
  ELPROXY(index)
  ELPROXY(persIndex)
  ELPROXY(dataID)
  ELPROXY(key)
  ELPROXY(persKey)
#ifndef XAOD_STANDALONE
  ELPROXY(source)
  ELPROXY(proxy)
#endif
#undef ELPROXY


private:
  /// The proxied @c PackedLink.
  PLink_t& m_pl;
};


//***************************************************************************


/**
 * @brief Extend @c PackedLinkConverter with a (non-const) conversion
 *        from @c PackedLink to a proxy object.
 */
template <class PROXY>
class ELProxyConverter
  : public detail::PackedLinkConverter<typename PROXY::Cont_t>
{
public:
  using Base = detail::PackedLinkConverter<typename PROXY::Cont_t>;
  using Link_t = typename PROXY::Link_t;
  using PLink_t = typename PROXY::PLink_t;
  using value_type = typename PROXY::Link_t;


  /**
   * @brief Constructor.
   * @param cnv Converter.  We'll make a copy of this.
   */
  ELProxyConverter (const Base& cnv);


  /**
   * @brief Constructor.
   * @param container Container holding the variables.
   * @param auxid The ID of the PackedLink variable.
   * @param linked_auxid The ID of the linked DataLinks.
   */
  ELProxyConverter (AuxVectorData& container,
                    SG::auxid_t auxid,
                    SG::auxid_t linked_auxid);

  
  /// Take const conversion from the base class.
  using Base::operator();


  /**
   * @brief Produce a proxy object for a given @c PackedLink.
   * @param pl The @c PackedLink object to proxy.
   */
  PROXY operator() (PLink_t& pl);
};


/// Proxy holding the converter by reference --- meant to be used
/// when we have a proxy as a member of a span.
template <class CONT>
using ELProxyInSpan = detail::ELProxyT<detail::ELProxyRefBase<CONT> >;


/// Converter producing a proxy --- meant to be used
/// when we have a proxy as a member of a span.
template <class CONT>
using ELProxyInSpanConverter =
  detail::ELProxyConverter<detail::ELProxyInSpan<CONT> >;


//***************************************************************************


/// A span over @c PackedLink.
template <class CONT, class PLINK_ALLOC>
using PackedLink_span = typename AuxDataTraits<PackedLink<CONT>, PLINK_ALLOC>::span;


/// A range transforming a span over @c PackedLink to @c ElementLink proxies.
template <class CONT, class PLINK_ALLOC>
using ELSpanProxyBase =
  CxxUtils::transform_view_with_at<PackedLink_span<CONT, PLINK_ALLOC>,
                                   std::reference_wrapper<detail::ELProxyInSpanConverter<CONT> > >;


/**
 * @brief Proxy for a span of @c ElementLinks.
 *
 * This class proxies a vector of @c PackedLink.
 * It acts as a span which returns @c ElementLink proxies, allowing
 * the span elements to be read and written as @c ElementLink.
 *
 * We also allow assignment from a range of @c ElementLink
 * and conversion to a vector of @c ElementLink.
 *
 * The proxies we return will reference the needed @c PackedLinkConverter
 * by reference.  The actual instance is stored by value in this class.
 *
 * Here, @c CONT is the type of the storable container that we reference,
 * and @c PLINK_ALLOC is the allocator type for the vector over @c PackedLinks.
 */
template <class CONT, class PLINK_ALLOC>
class ELSpanProxy
  : public ELSpanProxyBase<CONT, PLINK_ALLOC>
{
public:
  /// Base class of this one, holding the actual span.
  using Base = ELSpanProxyBase<CONT, PLINK_ALLOC>;

  /// The @c PackedLink class we proxy.
  using PLink_t = SG::PackedLink<CONT>;

  /// The @c ElementLink class we resolve to.
  using Link_t = ElementLink<CONT>;

  /// A span over @c PackedLink.
  using PackedLink_span = detail::PackedLink_span<CONT, PLINK_ALLOC>;

  /// The vector of @PackedLink that we proxy.
  using VElt_t = std::vector<SG::PackedLink<CONT>, PLINK_ALLOC>;

  /// Iterator type for this range.
  using iterator = std::ranges::iterator_t<Base>;


  /**
   * @brief Constructor.
   * @param velt The vector of @c PackedLink that we proxy.
   * @proxy container Container holding the variables.
   * @param auxid The ID of the PackedLink variable.
   * @param linked_auxid The ID of the linked DataLinks.
   */
  ELSpanProxy (VElt_t& velt, AuxVectorData& container,
               SG::auxid_t auxid,
               SG::auxid_t linked_auxid);


  /**
   * @brief Assign from a range of @c ElementLink.
   * @param r The range from which to assign.
   */
  template <ElementLinkRange<CONT> RANGE>
  void operator= (const RANGE& r);


  /**
   * @brief Convert to a vector of @c ElementLink.
   */
  template <class VALLOC>
  operator std::vector<Link_t, VALLOC>() const;


  /**
   * @brief Convert to a vector of @c ElementLink.
   */
  std::vector<Link_t> asVector() const;


  /**
   * @brief Equality testing.
   */
  template <class VALLOC>
  bool operator== (const std::vector<Link_t, VALLOC>& v) const;


  /**
   * @brief Add a new link to this vector of links.
   * @param l The new link to add.
   */
  void push_back (const Link_t& l);


  /**
   * @brief Clear this vector of links.
   */
  void clear();


  /**
   * @brief Resize this vector of links.
   * @param n The desired new size.
   * @param l Value with which to fill any new elements.
   */
  void resize (size_t n, const Link_t& l = Link_t());


  /**
   * @brief Erase one element from this vector of links.
   * @param pos The element to erase.
   */
  void erase (iterator pos);


  /**
   * @brief Erase a range of elements from this vector of links.
   * @param first The first element to erase.
   * @param last One past the last element to erase.
   * @param pos The element to erase.
   */
  void erase (iterator first, iterator last);


  /**
   * @brief Insert a new link into this vector of links.
   * @param pos The position at which to insert the link.
   * @param l The link to insert.
   */
  void insert (iterator pos, const Link_t& l);


  /**
   * @brief Insert copies of a new link into this vector of links.
   * @param pos The position at which to insert the link.
   * @param n Number of copies to insert.
   * @param l The link(s) to insert.
   */
  void insert (iterator pos, size_t n, const Link_t& l);


  /**
   * @brief Insert a range of links into this vector of links.
   * @param pos The position at which to insert the links.
   * @param first The first element to insert.
   * @param last One past the last element to insert.
   */
  template <CxxUtils::detail::InputValIterator<ElementLink<CONT> > ITERATOR>
  void insert (iterator pos, ITERATOR first, ITERATOR last);


  /**
   * @brief Insert a range of links into this vector of links.
   * @param pos The position at which to insert the links.
   * @param range The range to insert.
   */
  template <ElementLinkRange<CONT> RANGE>
  void insert_range (iterator pos, const RANGE& range);


  /**
   * @brief Append a range of links to the end of this vector of links.
   * @param range The range to append.
   */
  template <ElementLinkRange<CONT> RANGE>
  void append_range (const RANGE& range);


  /**
   * @brief Remove the last element in this vector of links.
   */
  void pop_back();


  /**
   * @brief Set this vector of links to copies of a new link.
   * @param n Number of copies to insert.
   * @param l The link(s) to insert.
   */
  void assign (size_t n, const Link_t& l);


  /**
   * @brief Set this vector of links to a range of links.
   * @param first The first element to copy.
   * @param last One past the last element to copy.
   */
  template <CxxUtils::detail::InputValIterator<ElementLink<CONT> > ITERATOR>
  void assign (ITERATOR first, ITERATOR last);


  /**
   * @brief Set this vector of links to a range of links.
   * @param range The range of links to copy.
   * @param last One past the last element to copy.
   */
  template <ElementLinkRange<CONT> RANGE>
  void assign_range (const RANGE& range);


private:
  /// The vector of @c PackedLink that we proxy.
  VElt_t& m_velt;

  /// The converter to use for the span.
  ELProxyInSpanConverter<CONT> m_cnv;
};


//***************************************************************************


/**
 * @brief Converter from a vector of @c PackedLink to a
 *        range of @c ElementLink proxies.
 */
template <class CONT, class PLINK_ALLOC>
class ELSpanConverter
{
public:
  using value_type = ELSpanProxy<CONT, PLINK_ALLOC>;
  using VElt_t = typename value_type::VElt_t;

  /***
   * @brief Constructor.
   * @param container Container holding the variables.
   * @param auxid The ID of the PackedLink variable.
   * @param linked_auxid The ID of the linked DataLinks.
   */
  ELSpanConverter (AuxVectorData& container,
                   auxid_t auxid,
                   auxid_t linked_auxid);


  /**
   * @brief Convert from a @c PackedLink vector to a proxy for the span.
   * @param velt The vector of @c PackedLink that we proxy.
   */
  value_type operator() (VElt_t& velt) const;


private:
  /// Container holding the variables.
  AuxVectorData& m_container;

  /// The ID of the PackedLink variable.
  auxid_t m_auxid;

  /// The ID of the linked DataLinks.
  auxid_t m_linkedAuxid;
};


}} // namespace SG::detail


#include "AthContainers/tools/ELProxy.icc"


#endif // not ATHCONTAINERS_ELPROXY_H
