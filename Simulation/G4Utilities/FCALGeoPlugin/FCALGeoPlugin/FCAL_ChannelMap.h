// Copyright (c) 2024 CERN for the benefit of the FastCaloSim project

#ifndef FCAL_CHANNELMAP_H
#define FCAL_CHANNELMAP_H

#include "CxxUtils/checker_macros.h"

#include <map>
#include <string>
#include <vector>

// Avoid colliding with the class of the same name in LArReadoutGeometry.
namespace FCALGeo {

/** Tube and tile maps for the FCAL. */
class FCAL_ChannelMap
{
public:
  typedef unsigned int tileName_t;
  typedef unsigned int tubeID_t;

  class TubePosition
  {
  public:
    TubePosition();
    TubePosition(tileName_t name,
                 float x,
                 float y,
                 const std::string& hvFT);
    tileName_t get_tileName() const;
    float x() const;
    float y() const;
    const std::string& getHVft() const;

  private:
    tileName_t m_tileName;
    float m_x;
    float m_y;
    std::string m_hvFT;
  };

  explicit FCAL_ChannelMap(int flag);

  typedef std::map<tubeID_t, TubePosition> tubeMap_t;
  typedef tubeMap_t::size_type tubemap_sizetype;
  typedef tubeMap_t::value_type tubemap_valuetype;
  typedef tubeMap_t::const_iterator tubemap_const_iterator;

  tubemap_const_iterator tubemap_begin(int isam) const;
  tubemap_const_iterator tubemap_end(int isam) const;
  tubemap_sizetype tubemap_size(int isam) const;

  tubemap_const_iterator getTubeByCopyNumber(int isam, int copyNo) const;

  bool getTileID(int isam, float x, float y, int& eta, int& phi) const;

  float x(int isam, int eta, int phi) const;
  float y(int isam, int eta, int phi) const;

private:
  /// Return a tile position before coordinate inversion.
  float x_raw(int isam, int eta, int phi) const;
  float y_raw(int isam, int eta, int phi) const;

public:

  void tileSize(int sam, int eta, int phi, float& dx, float& dy) const;

  void tileSize(int isam, int ntubes, float& dx, float& dy) const;

  void print_tubemap(int isam) const;

  bool invert_x() const;
  bool invert_xy() const;
  void set_invert_x(bool);
  void set_invert_xy(bool);

  void add_tube(const std::string& tileName,
                int mod,
                int id,
                int i,
                int j,
                double xCm,
                double yCm);
  void add_tube(const std::string& tileName,
                int mod,
                int id,
                int i,
                int j,
                double xCm,
                double yCm,
                const std::string& hvFT);

  /// Build tile maps after all tubes have been added.
  void finish();

  class TilePosition
  {
  public:
    TilePosition();
    TilePosition(float x, float y, int ntubes);
    float x() const;
    float y() const;
    unsigned int ntubes() const;

  private:
    float m_x;
    float m_y;
    unsigned int m_ntubes;
  };

  typedef std::map<tileName_t, TilePosition> tileMap_t;
  typedef tileMap_t::size_type tileMap_sizetype;
  typedef tileMap_t::value_type tileMap_valuetype;
  typedef tileMap_t::const_iterator tileMap_const_iterator;

  tileMap_const_iterator begin(int isam) const
  {
    return m_tileMap[isam - 1].begin();
  }
  tileMap_const_iterator end(int isam) const
  {
    return m_tileMap[isam - 1].end();
  };

private:
  static const double m_tubeSpacing[];
  double m_tubeDx[3];
  double m_tubeDy[3];
  double m_tileDx[3];
  double m_tileDy[3];
  bool m_invert_x;
  bool m_invert_xy;

  tileMap_t m_tileMap[3];
  void create_tileMap(int isam);

  tubeMap_t m_tubeMap[3];
  std::vector<tubemap_const_iterator> m_tubeIndex[3];
};

inline FCAL_ChannelMap::TubePosition::TubePosition()
    : m_tileName(0)
    , m_x(0)
    , m_y(0)
    , m_hvFT("")
{
}

inline FCAL_ChannelMap::TubePosition::TubePosition(tileName_t name,
                                                   float x,
                                                   float y,
                                                   const std::string& hvFT)
    : m_tileName(name)
    , m_x(x)
    , m_y(y)
    , m_hvFT(hvFT)
{
}

inline FCAL_ChannelMap::tileName_t FCAL_ChannelMap::TubePosition::get_tileName()
    const
{
  return m_tileName;
}

inline float FCAL_ChannelMap::TubePosition::x() const
{
  return m_x;
}

inline float FCAL_ChannelMap::TubePosition::y() const
{
  return m_y;
}

inline const std::string& FCAL_ChannelMap::TubePosition::getHVft() const
{
  return m_hvFT;
}

inline FCAL_ChannelMap::TilePosition::TilePosition()
    : m_x(0)
    , m_y(0)
    , m_ntubes(0)
{
}

inline FCAL_ChannelMap::TilePosition::TilePosition(float x, float y, int ntubes)
    : m_x(x)
    , m_y(y)
    , m_ntubes(ntubes)
{
}

inline float FCAL_ChannelMap::TilePosition::x() const
{
  return m_x;
}

inline float FCAL_ChannelMap::TilePosition::y() const
{
  return m_y;
}

inline unsigned int FCAL_ChannelMap::TilePosition::ntubes() const
{
  return m_ntubes;
}

inline bool FCAL_ChannelMap::invert_x() const
{
  return m_invert_x;
}

inline bool FCAL_ChannelMap::invert_xy() const
{
  return m_invert_xy;
}

inline void FCAL_ChannelMap::set_invert_x(bool flag)
{
  m_invert_x = flag;
  return;
}

inline void FCAL_ChannelMap::set_invert_xy(bool flag)
{
  m_invert_xy = flag;
  return;
}

}  // namespace FCALGeo

#endif  // FCAL_CHANNELMAP_H
