// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATRACKSIMLORENTZANGLETOOL_H
#define FPGATRACKSIMLORENTZANGLETOOL_H

// Athena headers
#include "AthenaBaseComps/AthAlgTool.h"

// ID helpers
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "InDetReadoutGeometry/SiDetectorManager.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"

// FPGATracSkim headers
#include "FPGATrackSimObjects/FPGATrackSimHit.h"

class PixelID;
class SCT_ID;
class PixelDetectorManager;
class SCT_DetectorManager;
class ISiLorentzAngleTool;

class FPGATrackSimHit;

namespace FPGATrackSim {
  class LorentzAngleTool : public AthAlgTool {
  public:
    LorentzAngleTool(const std::string&, const std::string&, const IInterface*);
    virtual ~LorentzAngleTool() = default;
    virtual StatusCode initialize() override;
    
    float getLorentzAngleShift(const FPGATrackSimHit & hit, int correctionType) const;
    StatusCode updateHitPosition(FPGATrackSimHit & hit, int correctionType) const;

private:
    Gaudi::Property<bool> m_useAthenaLorentzAngleTools{this, "UseAthenaLorentzAngleTools", false, "Use Athena Lorentz Angle tools to get the Lorentz angle shift"};
    Gaudi::Property<bool> m_shiftGlobalPosition{this, "shiftGlobalPosition", true, "Shift the global position of the hit along with the local position"};

    ToolHandle<ISiLorentzAngleTool> m_lorentzAngleToolPixel {this, "LorentzAngleToolPixel", "SiLorentzAngleTool/PixelLorentzAngleTool", "Tool to retrieve Lorentz angle of Pixel"};
    ToolHandle<ISiLorentzAngleTool> m_lorentzAngleToolStrip {this, "LorentzAngleToolStrip", "SiLorentzAngleTool/SCTLorentzAngleTool", "Tool to retrieve Lorentz angle of SCT"};

    const InDetDD::PixelDetectorManager* m_pixelManager = nullptr;
    const InDetDD::SiDetectorManager* m_SCTManager = nullptr;
    const PixelID* m_pixelId = nullptr;
    const SCT_ID* m_SCTId = nullptr;

    float getPixelBarrelShift_v0(unsigned layerDisk, int etaModule) const;
    float getPixelEndcapShift_v0(unsigned layerDisk, unsigned phiModule, int etaModule) const;
    float getStripBarrelShift_v0(bool isStereo, unsigned layerDisk, int etaModule) const;
    float getStripEndcapShift_v0(unsigned layerDisk, int etaModule, float z) const;

    float getPixelBarrelShift_v1(unsigned layerDisk, int etaModule) const;
    float getPixelEndcapShift_v1(unsigned layerDisk, unsigned phiModule, int etaModule) const;
    float getStripBarrelShift_v1(bool isStereo, int etaModule) const;
    float getStripEndcapShift_v1(unsigned layerDisk, int etaModule, float z) const;

    float getPixelBarrelShift_v2(unsigned layerDisk) const;
    float getPixelEndcapShift_v2(unsigned layerDisk, unsigned phiModule, int etaModule) const;
    float getStripBarrelShift_v2(bool isStereo) const;
    float getStripEndcapShift_v2(unsigned layerDisk, int etaModule, float z) const;
    
  };
} // namespace FPGATrackSim

#endif // FPGATRACKSIMLORENTZANGLETOOL_H
