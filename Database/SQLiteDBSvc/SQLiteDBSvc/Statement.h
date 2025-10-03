/* -*- C++ -*- */
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef SQLITEDBSVC_STATEMENT_H
#define SQLITEDBSVC_STATEMENT_H

#include <concepts>
#include <mutex>
#include <source_location>  // Improves error messages
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "sqlite3.h"

namespace SQLite {
/// Test if a type is a valid SQLite parameter type
template <typename T>
concept ValidParamType = requires {
  requires std::convertible_to<T, std::int64_t> ||
               std::convertible_to<T, double> ||
               std::convertible_to<T, std::string_view>;
};

/// Test if a type is a valid placeholder for some SQLite column type
template <typename T>
concept ValidColumnType = requires {
  requires std::same_as<std::remove_cvref_t<T>, T> &&
               (std::is_arithmetic_v<T> || std::same_as<T, std::string>);
};

/// Return type for result of an SQL statement that returns data
template <ValidColumnType... Args>
  requires(sizeof...(Args) >= 1)
using ResultType = std::vector<std::tuple<Args...>>;

/// Helper to return void if Args is empty or void
template <typename... Args>
struct ResultTypeWrapper {
  using type = ResultType<Args...>;
};
template <>
struct ResultTypeWrapper<> {
  using type = void;
};

/// SQLite prepared statement
class Statement {
 public:
  /// Create a prepared statement attached to an SQLiteDBSvc
  /// This class should be constructed using the createStatement member function
  /// of SQLiteDBSvc
  Statement(sqlite3* db, std::string_view sql, std::source_location call);
  ~Statement();
  Statement() = default;
  Statement(const Statement&) = delete;

  /// Move assignment operator allows Statement member variables to be
  /// filled from createStatement
  Statement& operator=(Statement&& rhs) noexcept;

  /// Run the statement
  template <ValidColumnType... ReturnArgs, ValidParamType... ParamArgs>
  ResultTypeWrapper<ReturnArgs...>::type run(ParamArgs... params);

 private:
  /// Reset prepared statement for re-execution and clear bindings
  void reset();
  /// Step through the prepared statement
  bool step();

  /// Bind an integer to a parameter
  void bind(int index, std::int64_t value);
  // template to force the correct overload to be called for ints
  template <typename I>
    requires std::integral<I>
  void bind(int index, I value);
  /// Bind a double to a parameter
  void bind(int index, double value);
  /// Bind a string to a parameter
  void bind(int index, std::string_view value);

  /// Retrieve a column
  template <ValidColumnType T, std::size_t I>
  T column();
  /// Retrieve columns
  template <ValidColumnType... Ts, std::size_t... Is>
  std::tuple<Ts...> columns(std::index_sequence<Is...>);

  std::recursive_mutex m_stmtMutex;
  sqlite3_stmt* m_stmt = nullptr;
  sqlite3* m_db = nullptr;
  std::source_location m_creationPoint;
};
}  // namespace SQLite

#include "Statement.icc"
#endif  // SQLITEDBSVC_STATEMENT_H
