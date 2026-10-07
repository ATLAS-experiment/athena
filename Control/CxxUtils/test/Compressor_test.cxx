/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//
// Unit test for the Compressor class.
//

#include "CxxUtils/Compressor.h"
#include <cassert>
#include <print>
#include <vector>


/// Helper: print results of a reduceToUS / expandFromUStoFloat round-trip
void testReduceToUS( const std::vector<float>& input )
{
  Compressor c;
  std::vector<unsigned short> compressed;
  c.reduceToUS( input, compressed );

  std::vector<float> output;
  c.expandFromUStoFloat( compressed, output );

  assert( output.size() == input.size() );

  std::println ("reduceToUS round-trip ({} values):",
                input.size());
  for ( std::size_t i = 0; i < input.size(); ++i ) {
    std::println ("  in={}  out={}", input[i], output[i]);
  }
}


/// Helper: print results of a reduce / expandToFloat round-trip
void testReduce( const std::vector<float>& input, int nBits, bool ignoreSign = false )
{
  Compressor c;
  c.setNrBits( nBits );
  if ( ignoreSign ) c.setIgnoreSign();

  std::vector<unsigned int> packed;
  c.reduce( input, packed );

  std::vector<float> output;
  c.expandToFloat( packed, output );

  assert( output.size() == input.size() );

  std::println ("reduce/expandToFloat round-trip (bits={}{}, {} values):",
                nBits, ( ignoreSign ? ", ignoreSign" : "" ), input.size());
  for ( std::size_t i = 0; i < input.size(); ++i ) {
    std::println ("  in={}  out={}", input[i], output[i]);
  }
}


int main()
{
  // --- reduceToUS: standard 32->16 bit compression
  const std::vector<float> basic = { 1.0f, -1.0f, 3.14159f, -2.71828f,
                                     0.5f, 1234.5f, -0.001f, 1e5f };
  testReduceToUS( basic );

  // --- reduce/expandToFloat at default 16 bits
  testReduce( basic, 16 );

  // --- 18-bit mode (as used by CaloCalibrationHitContainer converters)
  const std::vector<float> energies = { 1.0f, 2.5f, -3.75f, 1000.0f,
                                        0.123f, -456.789f };
  testReduce( energies, 18 );

  // --- ignoreSign: positive-only values
  const std::vector<float> positive = { 1.0f, 2.5f, 100.0f, 0.001f, 1e4f };
  testReduce( positive, 16, /*ignoreSign=*/true );

  // --- empty vector: both paths should produce no output
  const std::vector<float> empty;
  testReduceToUS( empty );
  testReduce( empty, 16 );

  // --- setNrBits clamping: request too few bits (clamped to 12)
  testReduce( basic, 4 );

  // --- setNrBits clamping: request too many bits (clamped to 31)
  testReduce( basic, 32 );

  return 0;
}
