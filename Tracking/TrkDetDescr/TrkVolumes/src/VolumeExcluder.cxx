/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// VolumeExcluder.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkVolumes/VolumeExcluder.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"


// constructor with volume
Trk::VolumeExcluder::VolumeExcluder(std::unique_ptr<Trk::Volume> vol)
  : m_vol(std::move(vol))
{}

// copy constructor
Trk::VolumeExcluder::VolumeExcluder(const VolumeExcluder& ex)
  : Trk::AreaExcluder(ex)
  , m_vol{ex.m_vol->clone()}
{}

/** Assignment operator */
Trk::VolumeExcluder&
Trk::VolumeExcluder::operator=(const VolumeExcluder& vol)
{
  if (&vol != this) {
    AreaExcluder::operator=(vol);
    m_vol.reset(vol.m_vol->clone());
  }
  return *this;
}

Trk::VolumeExcluder*
Trk::VolumeExcluder::clone() const
{
  return new Trk::VolumeExcluder(*this);
}

