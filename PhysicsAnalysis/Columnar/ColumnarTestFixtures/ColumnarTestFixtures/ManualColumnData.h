/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TEST_FIXTURES_MANUAL_COLUMN_DATA_H
#define COLUMNAR_TEST_FIXTURES_MANUAL_COLUMN_DATA_H

#include <boost/core/demangle.hpp>

#include <any>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace columnar
{
  namespace TestUtils
  {
    /// @brief a class that holds manually specified column data
    ///
    /// This is for use in unit test code to allow users specifying a
    /// column as a simple list of values, e.g.
    /// ```
    ///   columnMap.addColumn ({1.0f, 2, 3u});
    /// ```
    ///
    /// Internally this represents the user data as a vector of
    /// `std::any`, and then converts it to the appropriate type when
    /// asked.  This is not particularly efficient, but it is only
    /// meant for use in unit tests, where ease of use is more
    /// important than efficiency.

    class ManualColumnData final
    {
      /// Public Members
      /// ==============
    public:

      /// @brief standard constructor
      ManualColumnData (std::vector<std::any>&& data);


      /// @brief configure for the given type
      void configureType (const std::string& columnName, const std::type_info& type);

      /// @brief get the type we are configured for
      [[nodiscard]] const std::type_info* type() const noexcept {
        return m_type; }

      /// @brief get the configured column as an `std::span`
      template<typename T>
      [[nodiscard]] std::span<const T> getSpan (const std::string& name) const
      {
        if (m_type == nullptr)
          throw std::runtime_error ("column " + name + " not configured");
        if (*m_type != typeid(T))
          throw std::runtime_error ("column " + name + " has wrong type: " + boost::core::demangle(m_type->name()) + " != " + boost::core::demangle(typeid(T).name()));
        return std::span<const T> (static_cast<const T*> (m_column.get()), m_data.size());
      }

      /// @brief get the size of the column
      [[nodiscard]] std::size_t columnSize () const noexcept {
        return m_data.size(); }

      /// @brief get the data pointer for the column
      [[nodiscard]] void* columnVoidData() noexcept {
        return m_column.get(); }



      /// Private Members
      /// ===============
    private:

      /// @brief a vector of untyped data provided by the user
      std::vector<std::any> m_data;

      /// @brief the actual type for the column
      const std::type_info *m_type = nullptr;

      /// @brief the column created from @ref m_data
      std::shared_ptr<void> m_column;
    };
  }
}

#endif