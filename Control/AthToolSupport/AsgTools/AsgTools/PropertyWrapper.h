/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef ASG_TOOLS__PROPERTY_WRAPPER_H
#define ASG_TOOLS__PROPERTY_WRAPPER_H


#ifdef XAOD_STANDALONE

#include <AsgTools/PropertyMgr.h>
#include <iosfwd>
#include <string>
#include <utility>

namespace Gaudi
{
  /// \brief this is the standalone version of the \ref
  /// Gaudi::Property wrapper
  ///
  /// This makes it simpler to declare properties by just wrapping the
  /// property in this template and then using the inline constructor
  /// for it in the class definition:
  /// ```
  /// Gaudi::Property<int> m_property {
  ///   this, "propertyName", startingValue, "property description"};
  /// ```

  template<typename T> class Property final
  {
  public:
    template<typename T2>
    Property (T2 *owner, const std::string& name, const T& value,
              const std::string& title = "");

  public:
    template<typename T2>
    Property (T2 *owner, const std::string& name, T&& value,
              const std::string& title = "");

  public:
    operator const T& () const noexcept;
    const T& value () const noexcept {return m_value;}

  public:
    template<typename T2>
    Property<T>& operator = (T2&& that);

  public:
    template<typename T2>
    decltype(auto) operator + (T2&& that) const;

  public:
    decltype(auto) empty () const {return m_value.empty();}
    decltype(auto) size () const {return m_value.size();};
    decltype(auto) begin () const {return m_value.begin();}
    decltype(auto) end () const {return m_value.end();}
    template<typename T2> decltype(auto) operator [] (const T2& index) const {
      return m_value[index];}

    // the property we created.  this is used for debugging output and
    // includes the name for that reason.  it is maybe a bit hacky, but
    // it allows us to give proper printout (with proper meaning it
    // matches Athena).
  public:
    auto *getProperty () const {return m_property;}

  private:
    T m_value;

  private:
    const PropertyMgr::PropMap_t::value_type *m_property = nullptr;
  };



  /// Template Methods
  /// ================

  template<typename T> template<typename T2>
  Property<T> ::
  Property (T2 *owner, const std::string& name, const T& value,
            const std::string& title)
    : m_value (value)
  {
    owner->declareProperty (name, m_value, title);
    if (auto iter = owner->getPropertyMgr()->getProperties().find(name);
        iter != owner->getPropertyMgr()->getProperties().end())
      m_property = &*iter;
    else
      throw std::logic_error ("PropertyWrapper::Property - property not found");
  }



  template<typename T> template<typename T2>
  Property<T> ::
  Property (T2 *owner, const std::string& name, T&& value,
            const std::string& title)
    : m_value (std::move (value))
  {
    owner->declareProperty (name, m_value, title);
    if (auto iter = owner->getPropertyMgr()->getProperties().find(name);
        iter != owner->getPropertyMgr()->getProperties().end())
      m_property = &*iter;
    else
      throw std::logic_error ("PropertyWrapper::Property - property not found");
  }



  template<typename T>
  Property<T> ::
  operator const T& () const noexcept
  {
    return m_value;
  }



  template<typename T> template<typename T2>
  Property<T>& Property<T> ::
  operator = (T2&& that)
  {
    m_value = std::forward<T2>(that);
    return *this;
  }



  template<typename T> template<typename T2>
  decltype(auto) Property<T> ::
  operator + (T2&& that) const
  {
    return m_value + std::forward<T2>(that);
  }



  template<typename T1,typename T2>
  decltype(auto) operator + (T1&& lh, const Property<T2>& rh)
  {
    return std::forward<T1>(lh) + rh.value();
  }



  template<typename T,typename T2>
  bool operator == (const Property<T>& lhs, const T2& rhs)
  {
    return lhs.value() == rhs;
  }



  template<typename T,typename T2>
  bool operator != (const Property<T>& lhs, const T2& rhs)
  {
    return lhs.value() != rhs;
  }



  template<typename T>
  std::ostream& operator << (std::ostream& str, const Property<T>& property)
  {
    std::string propertyString;
    if (property.getProperty()->second->getString (propertyString).isFailure())
      throw std::runtime_error ("PropertyWrapper::operator << - no string conversion available");
    return str << "'" << property.getProperty()->first << "': " << propertyString;
  }
}

#endif

#endif
