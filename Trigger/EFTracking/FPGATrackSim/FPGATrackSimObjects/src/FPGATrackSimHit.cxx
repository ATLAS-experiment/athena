/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "FPGATrackSimObjects/FPGATrackSimHit.h"

#include <stdexcept>




bool FPGATrackSimHit::isMapped() const
{
    if(m_layer>=0){
        return true;
    }
    return false;
}
bool FPGATrackSimHit::isRemapped() const
{
    return m_isRemapped;
}
void FPGATrackSimHit::setRemap() 
{
    m_isRemapped=true;
}
bool FPGATrackSimHit::isClustered() const
{
    switch (m_hitType)
    {
    case HitType::clustered: // TODO do wildcard and guessed hits have cluster info too?
    case HitType::spacepoint:
        return true;
    default:
        return false;
    }
}

bool FPGATrackSimHit::isReal() const
{
    switch (m_hitType)
    {
    case HitType::unmapped:
    case HitType::mapped:
    case HitType::clustered:
    case HitType::spacepoint:
        return true;
    default:
        return false;
    }
}

// Sets using the physical layer index as defined by FPGATrackSimPlaneMap
void FPGATrackSimHit::setPhysLayer(unsigned v)
{
    if (m_detType == SiliconTech::strip)
    {
        m_layer_disk = v / 2;
        m_side = v % 2;
    }
    else
    {
        m_layer_disk = v;
    }
}

// Returns the physical layer index as defined by FPGATrackSimPlaneMap
unsigned FPGATrackSimHit::getPhysLayer() const
{
    if (m_detType == SiliconTech::strip)
        return 2 * m_layer_disk + m_side;
    return m_layer_disk;
}


unsigned FPGATrackSimHit::getLayer() const
{
    if (isMapped() || (m_hitType == HitType::guessed)) return m_layer;
    throw std::domain_error("FPGATrackSimHit::getLayer() called on a hit with invalid type: " + to_string(m_hitType));
}

unsigned FPGATrackSimHit::getSection() const
{
    if (isMapped()) return m_section;
    throw std::domain_error("FPGATrackSimHit::getSection() called on a hit with invalid type");
}

void FPGATrackSimHit::makeSpacepoint(float x, float y, float z, float window, FPGATrackSimHit& other, FPGATrackSimMultiTruth& new_truth) {
    // Update coordinates. This keeps a copy of the old ones.
    setX(x);
    setY(y);
    setZ(z);

    // Store the phi window.
    m_phiWindow = window;

    // Update the truth, so we can do truth matching.
    setTruth(new_truth);

    // Store the local coordinates of the inner hit.
    // the need for a subclass in the futer should be considered.
    const FPGATrackSimHit* inner = ((getPhysLayer() % 2) == 0) ? this : &other;
    m_pairedEtaModule = inner->getEtaModule();
    m_pairedPhiModule = inner->getPhiModule();

    // If the hit is unmapped we need to store the physical
    // layer coordinates, and when the hit *becomes* mapped, update the paired logical layer too.
    if (isMapped()) {
        m_pairedSection = inner->getSection();
        m_pairedLayer = inner->getLayer();
    } 
    
    m_pairedDetZone = inner->getDetectorZone();
    m_pairedDetType = inner->getDetType();
    m_pairedPhysLayer = inner->getPhysLayer();    

    // Update the type.
    setHitType(HitType::spacepoint);
}

const FPGATrackSimHit FPGATrackSimHit::getOriginalHit() const {
  // Only works for spacepoints. TODO: subclass...
  FPGATrackSimHit original = *(this);
  if (getHitType() != HitType::spacepoint) {
    return *this;
  }

  // Restore the x/y/z coordinates. That should be all we have to do!
  original.setX(m_originalX);
  original.setY(m_originalY);
  original.setZ(m_originalZ);

  // Change the type... is "clustered" the right type?
  original.setHitType(HitType::clustered);

  // Technically, we could also restore the original truth.

  return original;
}

std::ostream& operator<<(std::ostream& out, const FPGATrackSimHit& hit)
{
    out << "type=" << hit.m_hitType
        << " tech=" << hit.m_detType
        << " zone=" << hit.m_detectorZone
        << " etamod=" << hit.getEtaModule()
        << " phimod=" << hit.getPhiModule()
        << " physLayer=" << hit.getPhysLayer()
        << " eta=" << hit.getEtaCoord()
        << " phi=" << hit.getPhiCoord();

    return out;
}


std::string to_string(HitType t)
{
    switch (t)
    {
    case HitType::unmapped:   return "unmapped";
    case HitType::mapped:     return "mapped";
    case HitType::clustered:  return "clustered";
    case HitType::wildcard:   return "wildcard";
    case HitType::guessed:    return "guessed";
    case HitType::extrapolated:  return "extrapolated";
    case HitType::spacepoint: return "spacepoint";
    default:                  return "undefined";
    }
}


std::ostream& operator<<(std::ostream& os, HitType t)
{
    return (os << to_string(t));
}


