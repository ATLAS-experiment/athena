/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include <cstdio>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "FCALGeoPlugin/FCAL_ChannelMap.h"

namespace FCALGeo {

const double cm = 10.;
const double FCAL_ChannelMap::m_tubeSpacing[] = {
    0.75 * cm, 0.8179 * cm, 0.90 * cm};

FCAL_ChannelMap::FCAL_ChannelMap(int flag)
{
  static const double root3_2 = std::sqrt(3.) / 2;
  for (int i = 0; i < 3; i++) {
    m_tubeDx[i] = m_tubeSpacing[i] / 2.;
    m_tubeDy[i] = m_tubeSpacing[i] * root3_2;
  }

  // FCAL1 small cells are 2x2 tubes
  m_tileDx[0] = 2. * m_tubeSpacing[0];
  m_tileDy[0] = 2. * m_tubeSpacing[0] * root3_2;

  // FCAL2 small cells are 2x3 tubes
  m_tileDx[1] = 2. * m_tubeSpacing[1];
  m_tileDy[1] = 3. * m_tubeSpacing[1] * root3_2;

  // FCAL3 cells are 6x6 tubes
  m_tileDx[2] = 6. * m_tubeSpacing[2];
  m_tileDy[2] = 6. * m_tubeSpacing[2] * root3_2;

  m_invert_x = flag & 1;
  m_invert_xy = flag & 2;
}

void FCAL_ChannelMap::finish()
{
  create_tileMap(1);
  create_tileMap(2);
  create_tileMap(3);
}

void FCAL_ChannelMap::add_tube(const std::string& tileName,
                               int mod,
                               int /*id*/,
                               int i,
                               int j,
                               double x,
                               double y)
{
  // Decode the eta and phi fields embedded in the tile name.
  std::istringstream tileStream2(std::string(tileName, 3, 2));
  std::istringstream tileStream3(std::string(tileName, 6, 3));
  int a2 = 0, a3 = 0;
  if (tileStream2)
    tileStream2 >> a2;
  if (tileStream3)
    tileStream3 >> a3;

  tileName_t tilename = (a3 << 16) + a2;

  TubePosition tb(tilename, x * cm, y * cm, "");
  // Make the possibly negative tube indices suitable for packed IDs.
  i = i + 200;
  j = j + 200;
  unsigned int ThisId = (j << 16) + i;
  tubemap_const_iterator p = m_tubeMap[mod - 1].insert(
      m_tubeMap[mod - 1].end(), std::make_pair(ThisId, tb));
  m_tubeIndex[mod - 1].push_back(p);
}

void FCAL_ChannelMap::add_tube(const std::string& tileName,
                               int mod,
                               int /*id*/,
                               int i,
                               int j,
                               double x,
                               double y,
                               const std::string& hvFT)
{
  // Decode the eta and phi fields embedded in the tile name.
  std::istringstream tileStream2(std::string(tileName, 3, 2));
  std::istringstream tileStream3(std::string(tileName, 6, 3));
  int a2 = 0, a3 = 0;
  if (tileStream2)
    tileStream2 >> a2;
  if (tileStream3)
    tileStream3 >> a3;

  tileName_t tilename = (a3 << 16) + a2;

  TubePosition tb(tilename, x * cm, y * cm, hvFT);
  // Make the possibly negative tube indices suitable for packed IDs.
  i = i + 200;
  j = j + 200;
  unsigned int ThisId = (j << 16) + i;
  tubemap_const_iterator p = m_tubeMap[mod - 1].insert(
      m_tubeMap[mod - 1].end(), std::make_pair(ThisId, tb));
  m_tubeIndex[mod - 1].push_back(p);
}

FCAL_ChannelMap::tubemap_const_iterator FCAL_ChannelMap::getTubeByCopyNumber(
    int isam, int copyNo) const
{
  return m_tubeIndex[isam - 1][copyNo];
}

void FCAL_ChannelMap::create_tileMap(int isam)
{
  tileMap_const_iterator tile;
  tubemap_const_iterator first = m_tubeMap[isam - 1].begin();
  tubemap_const_iterator last = m_tubeMap[isam - 1].end();

  // Accumulate tube positions by tile.
  while (first != last) {
    tileName_t tileName = (first->second).get_tileName();
    tile = m_tileMap[isam - 1].find(tileName);

    if (tile == m_tileMap[isam - 1].end()) {  // New tile found
      float x = (first->second).x();
      float y = (first->second).y();
      unsigned int ntubes = 1;
      TilePosition tp(x, y, ntubes);
      m_tileMap[isam - 1][tileName] = tp;
    } else {  // Existing tile
      float x = (tile->second).x() + (first->second).x();
      float y = (tile->second).y() + (first->second).y();
      unsigned int ntubes = (tile->second).ntubes() + 1;
      TilePosition tp(x, y, ntubes);
      m_tileMap[isam - 1][tileName] = tp;
    }
    ++first;
  }

  // Replace each accumulated position with the tile centre.
  tileMap_const_iterator tilefirst = m_tileMap[isam - 1].begin();
  tileMap_const_iterator tilelast = m_tileMap[isam - 1].end();
  while (tilefirst != tilelast) {
    tileName_t tileName = tilefirst->first;
    unsigned int ntubes = (tilefirst->second).ntubes();
    float xtubes = (float)ntubes;
    float x = (tilefirst->second).x() / xtubes;
    float y = (tilefirst->second).y() / xtubes;
    TilePosition tp(x, y, ntubes);
    m_tileMap[isam - 1][tileName] = tp;
    ++tilefirst;
  }
}

bool FCAL_ChannelMap::getTileID(
    int isam, float x_orig, float y_orig, int& eta, int& phi) const
{
  float x = x_orig;
  float y = y_orig;

  if (m_invert_xy) {
    x = y_orig;
    y = x_orig;
  }

  if (m_invert_x)
    x = -x;

  // Find the closest tube in the staggered hexagonal grid.
  int ktx = (int)(x / m_tubeDx[isam - 1]);
  int kty = (int)(y / m_tubeDy[isam - 1]);
  if (x < 0.)
    ktx--;
  if (y < 0.)
    kty--;

  tubemap_const_iterator it = m_tubeMap[isam - 1].begin();
  unsigned int firstId = it->first;

  // Derive the packed-index offset from the first mapped tube.
  int ix = ktx
      + ((int)((firstId & 0xffff) - it->second.x() / m_tubeDx[isam - 1])) + 1;
  int iy =
      kty + ((int)((firstId >> 16) - it->second.y() / m_tubeDy[isam - 1])) + 1;

  int isOddEven = (((firstId >> 16) % 2) + (firstId % 2)) % 2;
  bool movex = false;

  if ((iy % 2) != ((ix + isOddEven) % 2)) {
    double yc = y / m_tubeDy[isam - 1] - kty - 0.5;
    if (fabs(yc) > 0.5 / sqrt(3)) {
      double xk = x / m_tubeDx[isam - 1] - ktx;
      if (xk > 0.5) {
        xk = 1 - xk;
      }
      double yn = 0.5 - xk / 3;
      if (fabs(yc) > fabs(yn)) {
        if (yc > 0)
          iy++;
        else
          iy--;
      } else
        movex = true;
    } else
      movex = true;
    if (movex) {
      if (x / m_tubeDx[isam - 1] - ktx > 0.5)
        ix++;
      else
        ix--;
    }
  }

  tubeID_t tubeID = (iy << 16) + ix;

  it = m_tubeMap[isam - 1].find(tubeID);
  if (it != m_tubeMap[isam - 1].end()) {
    tileName_t tilename = (it->second).get_tileName();
    phi = tilename & 0xffff;
    eta = tilename >> 16;
    return true;
  }
  return false;
}

float FCAL_ChannelMap::x_raw(int isam, int eta, int phi) const
{
  tileName_t tilename = (eta << 16) + phi;

  tileMap_const_iterator it = m_tileMap[isam - 1].find(tilename);
  if (it == m_tileMap[isam - 1].end()) {
    char l_str[200];
    std::snprintf(l_str, sizeof(l_str),
                  "FCAL_ChannelMap::x: unknown tile phi=%d, eta=%d", phi, eta);
    std::string errorMessage(l_str);
    throw std::range_error(errorMessage.c_str());
  }

  return (it->second).x();
}

float FCAL_ChannelMap::y_raw(int isam, int eta, int phi) const
{
  tileName_t tilename = (eta << 16) + phi;

  tileMap_const_iterator it = m_tileMap[isam - 1].find(tilename);
  if (it == m_tileMap[isam - 1].end()) {
    char l_str[200];
    std::snprintf(l_str, sizeof(l_str),
                  "FCAL_ChannelMap::y: unknown tile phi=%d, eta=%d", phi, eta);
    std::string errorMessage(l_str);
    throw std::range_error(errorMessage.c_str());
  }

  return (it->second).y();
}

float FCAL_ChannelMap::x(int isam, int eta, int phi) const
{
  if (m_invert_xy) {
    // Swapped coordinates use the raw y value for x.
    return y_raw(isam, eta, phi);
  }

  float x = x_raw(isam, eta, phi);

  if (m_invert_x) {
    return -x;
  }
  return x;
}

float FCAL_ChannelMap::y(int isam, int eta, int phi) const
{
  if (m_invert_xy) {
    // Swapped coordinates use the corrected raw x value for y.
    float x = x_raw(isam, eta, phi);
    if (m_invert_x) {
      return -x;
    }
    return x;
  }

  return y_raw(isam, eta, phi);
}

void FCAL_ChannelMap::tileSize(int sam, int ntubes, float& dx, float& dy) const
{
  dx = m_tubeDx[sam - 1];
  dy = m_tubeDy[sam - 1];
  if (sam == 1 || sam == 3) {
    float scale = std::sqrt(ntubes);
    dx = dx * scale;
    dy = dy * scale;
  } else {
    float scale = std::sqrt(ntubes / 1.5);
    dx = dx * scale;
    dy = dy * scale * 1.5;
  }

  // The staggered grid has twice as many x positions as y positions.
  dx = 2 * dx;

  if (m_invert_xy) {
    float temp = dx;
    dx = dy;
    dy = temp;
  }
}

void FCAL_ChannelMap::tileSize(
    int sam, int eta, int phi, float& dx, float& dy) const
{
  tileName_t tilename = (eta << 16) + phi;

  tileMap_const_iterator it = m_tileMap[sam - 1].find(tilename);
  if (it != m_tileMap[sam - 1].end()) {
    int ntubes = (it->second).ntubes();
    tileSize(sam, ntubes, dx, dy);
    return;
  }
  char l_str[200];
  std::snprintf(l_str, sizeof(l_str),
                "FCAL_ChannelMap::tileSize: unknown tile phi=%d, eta=%d",
                phi, eta);
  throw std::range_error(l_str);
}

void FCAL_ChannelMap::print_tubemap(int imap) const
{
  FCAL_ChannelMap::tubemap_const_iterator it = m_tubeMap[imap - 1].begin();
  const auto end = m_tubeMap[imap - 1].end();

  std::cout << "First 10 elements of the New FCAL tube map : " << imap
            << std::endl;
  std::cout.precision(5);
  for (int i = 0; i < 10 && it != end; ++i, ++it)
    std::cout << std::hex << it->first << "\t" << (it->second).get_tileName()
              << std::dec << "\t" << (it->second).x() << "\t"
              << (it->second).y() << std::endl;
}

FCAL_ChannelMap::tubemap_const_iterator FCAL_ChannelMap::tubemap_begin(
    int imap) const
{
  return m_tubeMap[imap - 1].begin();
}

FCAL_ChannelMap::tubemap_const_iterator FCAL_ChannelMap::tubemap_end(
    int imap) const
{
  return m_tubeMap[imap - 1].end();
}

FCAL_ChannelMap::tubemap_sizetype FCAL_ChannelMap::tubemap_size(int imap) const
{
  return m_tubeMap[imap - 1].size();
}

}  // namespace FCALGeo
