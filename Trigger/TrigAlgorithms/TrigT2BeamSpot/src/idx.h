/*
Copyright (C) 2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT2BEAMSPOT_IDX_H
#define TRIGT2BEAMSPOT_IDX_H

#include <cstdint> //for int8_t
namespace PESA{

// Ordering of the monomials in Bx, By, tx, ty powers,
// array indices correspond to power of those variables
constexpr std::int8_t g_order[3][3][3][3] = {

{
 {{  0,  1,  2 },     // Bx**0, By**0, tx**0, ty**0..ty**2
  {  3,  4, -1 },     // Bx**0, By**0, tx**1, ty**0..ty**2
  {  5, -1, -1 }},    // Bx**0, By**0, tx**2, ty**0..ty**2

 {{  6,  7, -1 },     // Bx**0, By**1, tx**0, ty**0..ty**2
  {  8, -1, -1 },     // Bx**0, By**1, tx**1, ty**0..ty**2
  { -1, -1, -1 }},    // Bx**0, By**1, tx**2, ty**0..ty**2

 {{  9, -1, -1 },     // Bx**0, By**2, tx**0, ty**0..ty**2
  { -1, -1, -1 },     // Bx**0, By**2, tx**1, ty**0..ty**2
  { -1, -1, -1 }},    // Bx**0, By**2, tx**2, ty**0..ty**2
},

{
 {{ 10, 11, -1 },     // Bx**1, By**0, tx**0, ty**0..ty**2
  { 12, -1, -1 },     // Bx**1, By**0, tx**1, ty**0..ty**2
  { -1, -1, -1 }},    // Bx**1, By**0, tx**2, ty**0..ty**2

 {{ 13, -1, -1 },     // Bx**1, By**1, tx**0, ty**0..ty**2
  { -1, -1, -1 },     // Bx**1, By**1, tx**1, ty**0..ty**2
  { -1, -1, -1 }},    // Bx**1, By**1, tx**2, ty**0..ty**2

 {{ -1, -1, -1 },     // Bx**1, By**2, tx**0, ty**0..ty**2
  { -1, -1, -1 },     // Bx**1, By**2, tx**1, ty**0..ty**2
  { -1, -1, -1 }},    // Bx**1, By**2, tx**2, ty**0..ty**2
},

{
 {{ 14, -1, -1 },     // Bx**2, By**0, tx**0, ty**0..ty**2
  { -1, -1, -1 },     // Bx**2, By**0, tx**1, ty**0..ty**2
  { -1, -1, -1 }},    // Bx**2, By**0, tx**2, ty**0..ty**2

 {{ -1, -1, -1 },     // Bx**2, By**1, tx**0, ty**0..ty**2
  { -1, -1, -1 },     // Bx**2, By**1, tx**1, ty**0..ty**2
  { -1, -1, -1 }},    // Bx**2, By**1, tx**2, ty**0..ty**2

 {{ -1, -1, -1 },     // Bx**2, By**2, tx**0, ty**0..ty**2
  { -1, -1, -1 },     // Bx**2, By**2, tx**1, ty**0..ty**2
  { -1, -1, -1 }},    // Bx**2, By**2, tx**2, ty**0..ty**2
}
};

// number of possible combinations of all powers,
// 15 is for squares, 1 for log
constexpr unsigned g_size = 15 + 1;

// Ordering of the monomials in omegax, omegay powers,
// array indices correspond to power of those variables
constexpr std::int8_t g_order2[3][3] = {
  {  0,  1,  2 },     // Bx**0, By**0, tx**0, ty**0..ty**2
  {  3,  4, -1 },     // Bx**0, By**0, tx**1, ty**0..ty**2
  {  5, -1, -1 },     // Bx**0, By**0, tx**2, ty**0..ty**2
};

// number of possible combinations of omega powers
constexpr unsigned g_size2 = 6;

constexpr unsigned nbins = 
        // two extra bins to count number of tracks and beam_size*n_tracks
        g_size*g_size2  // all polynomial coefficients
        + 1             // Sum(z0)
        + 1             // Sum(z0**2)
        + 1             // Sum(1)
        + 1             // Sum(beam_size)
        ;



template<unsigned Bx, unsigned By, unsigned tx, unsigned ty, unsigned ox, unsigned oy>
consteval int idx()
{   
  constexpr int i1 = g_order[Bx][By][tx][ty];
  constexpr int i2 = g_order2[ox][oy];
  static_assert(i1 >= 0 && i2 >= 0, "idx(): invalid power combination");
  return i1 * g_size2 + i2;
}


} // namespace


#endif
