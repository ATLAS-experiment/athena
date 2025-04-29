/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_VECTOR_CONVERT_VIEW_H
#define COLUMNAR_CORE_VECTOR_CONVERT_VIEW_H

#include <ColumnarCore/ColumnAccessor.h>

namespace columnar
{
  namespace detail
  {
  /// @file VectorConvertView.h
  ///
  /// `std::vector` views and iterators that incorporate a type conversion
  /// ====================================================================
  ///
  /// If the underlying column is an `std::vector`, but the access
  /// requires a type conversion (e.g. `ElementLink` or an enum), then I
  /// can't just use a `std::span` to access the data.  Instead I rely
  /// on this view and iterator to handle the conversion on access.
  /// Presumably with C++23 this could be replaced with a range adaptor,
  /// but for now I need to do it manually.


    /// an iterator that does converts members using a passed in function
    template<typename FunctionType,typename IteratorType>
    class VectorConvertIterator final
    {
      /// Public Members
      /// ==============
    public:

      VectorConvertIterator (const FunctionType& val_function, IteratorType&& val_iterator)
        : m_function (val_function), m_iterator (std::move (val_iterator))
      {}

      [[nodiscard]] bool operator == (const VectorConvertIterator& that) const noexcept {
        return m_iterator == that.m_iterator;}
      [[nodiscard]] bool operator != (const VectorConvertIterator& that) const noexcept {
        return m_iterator != that.m_iterator;}
      
      VectorConvertIterator& operator ++ () noexcept {
        ++ m_iterator; return *this;}

      [[nodiscard]] decltype(auto) operator * () const noexcept {
        return m_function (*m_iterator);}

      /// Private Members
      /// ===============
    private:

      [[no_unique_address]] FunctionType m_function;
      IteratorType m_iterator;
    };
    template<typename FunctionType,typename IteratorType> VectorConvertIterator (FunctionType&&,IteratorType&&) -> VectorConvertIterator<std::remove_cv_t<FunctionType>,std::remove_cv_t<IteratorType>>;



    /// a range view that does converts members using a passed in function
    template<typename FunctionType,typename ViewType>
    class VectorConvertView final
    {
      /// Common Public Members
      /// =====================
    public:

      explicit VectorConvertView (const FunctionType& val_function, const ViewType& val_view) noexcept
        : m_function (val_function), m_view (val_view)
      {}

      [[nodiscard]] std::size_t size () const noexcept
      {
        return m_view.size();
      }

      [[nodiscard]] decltype(auto) operator[] (std::size_t index) const
      {
        if (index >= m_view.size()) [[unlikely]]
          throw std::out_of_range ("VectorConvertView::operator[]: index out of range");
        return m_function (m_view[index]);
      }

      [[nodiscard]] auto begin () const noexcept
      {
        return VectorConvertIterator (m_function, m_view.begin());
      }

      [[nodiscard]] auto end () const noexcept
      {
        return VectorConvertIterator (m_function, m_view.end());
      }

      /// Private Members
      /// ===============
    private:

      [[no_unique_address]] FunctionType m_function;
      ViewType m_view;
    };
    template<typename FunctionType,typename ViewType> VectorConvertView (FunctionType&&,ViewType&&) -> VectorConvertView<std::remove_cv_t<FunctionType>,std::remove_cv_t<ViewType>>;
  }
}

#endif
