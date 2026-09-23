/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TestTools_expect_h
#define TestTools_expect_h

#undef NDEBUG
#ifndef XAOD_STANDALONE
# include "GaudiKernel/StatusCode.h"
#endif
#include <cassert>
#include <iostream>
#include <print>
#include <cmath>
namespace Athena_test {

  /*
   * Helpers for float/double comarisons with the precision and every other type exactly
   */
  template <typename T>
  bool cmp_eq( T a, T b ) {  return a == b; }
  template<>
  bool cmp_eq<float>( float a, float b ) { return std::abs(a - b) < 1.e-4; }
  template<>
  bool cmp_eq<double>( double a, double b ) { return std::abs(a - b) < 1.e-6; }

  /// Helpers for error message formatting.
  /// std::format doesn't accept arbitrary pointers, so decay them to void*.
  template <class U>
  const U& val_form (const U& x) { return x; }
  template <class U>
  const void* val_form (const U* x) { return x; }
  template <class U>
  const void* val_form (U* x) { return x; }
#ifndef XAOD_STANDALONE
  std::string val_form (const StatusCode& x) { return x.message(); }
#endif

  /*
   * Helper class, offering method to compare for equality to the value captured during construction.
   * In case of a difference the message of what is the value captured and what was expected 
   * is printed. In addition the assertion macro is used to make the test failing in this case.
   *  
   * There is also a symmetric method for checking for inequality.
   */
  template <typename T>
  class TestedValue {
  public:

    TestedValue( const T & v, std::string&& f, int l)
      : m_value(v),
	m_file(std::move(f)),
	m_line(l) {}
    void EXPECTED( const T& e ) {
      if ( not cmp_eq(e, m_value) ) {
        std::println (std::cerr, "{}:{}: error: Test failed, " 
                      "expected: {} obtained: {}",
                      m_file, m_line, val_form(e), val_form(m_value));
	assert( cmp_eq(e, m_value) );
      }
    }
    void NOT_EXPECTED( const T& e ) {
      if ( cmp_eq(e, m_value) ) {
        std::println (std::cerr, "{}:{}: error: Test failed, " 
                      "NOT expected: {} obtained: {}",
                      m_file, m_line, val_form(e), val_form(m_value));
	assert( not cmp_eq(e, m_value) );
      }
    }
  private:
    T m_value;
    std::string m_file;
    int m_line;
  };
}

#define VALUE( TESTED ) Athena_test::TestedValue<decltype(TESTED)>(TESTED, __FILE__, __LINE__).

/*
 * @brief macros (& simple class) for human readable stating assertions in unit tests
 * The syntax will be:
 * VALUE ( x ) EXPECTED ( true );  // exact comparisons
 * VALUE ( y ) EXPECTED ( "something");
 * VALUE ( z ) EXPECTED ( 3.1415 ); // this would compare with precision 1e-4 for floats and 1e-6 for doubles
 * VALUE ( t ) NOT_EXPECTED ( 0 ); // the inverted check is also possible
 */

#endif // TestTools_expect_h
