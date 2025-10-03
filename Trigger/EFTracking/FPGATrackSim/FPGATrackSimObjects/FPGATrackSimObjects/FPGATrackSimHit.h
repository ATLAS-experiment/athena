// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef TRIGFPGATrackSimOBJECTS_FPGATrackSimHIT_H
#define TRIGFPGATrackSimOBJECTS_FPGATrackSimHIT_H

/**
 * @file: FPGATrackSimHit.h
 * @author: Lots of peeps
 * @date: Lightyears ago
 * @brief: FPGATrackSim-specific class to represent an hit in the detector.
 *
 * Declarations in this file:
 *      enum class HitType
 *      class FPGATrackSimHit
 *      vector<vector<const FPGATrackSimHit*>> sortByLayer(vector<const FPGATrackSimHit*> const & hits);
 *      ostream & operator<<(ostream & os, HitType t);
 *
 *
 * Pay close attention to the enum HitType below. Several members of the class
 * will be invalid until the hit is put through some step of the processing,
 * like mapping or clustering. Calling a get function on the wrong type will
 * throw std::domain_error.
 */

#include <vector>
#include <ostream>
#include <memory>

#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimMultiTruth.h"

#include <Rtypes.h>
#include "TMath.h"

#include "GeneratorObjects/HepMcParticleLink.h"

 // Some types are inclusive of others. For example, clustered implies mapped.
enum class HitType { unmapped, mapped, clustered, wildcard, guessed, undefined, extrapolated, spacepoint };

class FPGATrackSimHit
{
public:

    ///////////////////////////////////////////////////////////////////////
    // Constructors

    FPGATrackSimHit() = default;
    virtual ~FPGATrackSimHit() = default;

    ///////////////////////////////////////////////////////////////////////
    // Getters/Setters

    // --- Hit Type ---
    void setHitType(HitType type) { m_hitType = type; }
    void setDetType(SiliconTech detType) { m_detType = detType; }
    void setDetectorZone(DetectorZone detZone) { m_detectorZone = detZone; }
    HitType      getHitType()      const { return m_hitType; }
    SiliconTech  getDetType()      const { return m_detType; }
    DetectorZone getDetectorZone() const { return m_detectorZone; }

    bool isMapped() const;
    bool isClustered() const;
    bool isReal() const; // ie not wildcard or guessed
    bool isPixel() const { return m_detType == SiliconTech::pixel; }
    bool isStrip() const { return m_detType == SiliconTech::strip; }
    bool isBarrel() const { return m_detectorZone == DetectorZone::barrel; }
    unsigned getDim() const { return isPixel() ? 2 : 1; } // TODO all calls of this should be replaced with a call to plane map
    int getRoadID() const { return m_roadID; }

    // --- Unmapped Location ---
    void setIdentifier(unsigned int v) { m_identifier = v; } // 32 bit (short) module identifier
    void setIdentifierHash(unsigned v) { m_identifierHash = v; }
    void setLayerDisk(unsigned v) { m_layer_disk = v; } // ITk layer number
    void setSide(unsigned v) { m_side = v; }
    void setPhysLayer(unsigned v); // Sets using the FPGATrackSim-defined physical layer index (see FPGATrackSimPlaneMap.h)
    void setEtaModule(int v) { m_etaModule_old = m_etaModule; m_etaModule = v; };
    void setPhiModule(unsigned v) { m_phiModule = v; }
    void setEtaWidth(unsigned v) { m_etaWidth = v; }
    void setPhiWidth(unsigned v) { m_phiWidth = v; }
    unsigned int getIdentifier() const { return m_identifier; } // 32 bit (short) module identifier
    unsigned getIdentifierHash() const { return m_identifierHash; } // TODO note this might break things in the same way as getSide() a few lines below. If so, recomment.
    unsigned getLayerDisk(bool old=false) const { if (old && isRemapped()) return m_layer_disk_old; else return m_layer_disk;} // ITk layer number
    unsigned getSide() const { return m_side; } // strip side TODO note this has been uncommented on 4/20/21. If wrappers suddenly break, recomment this. Same for getIdentifierHash above.
    unsigned getPhysLayer(bool old=false) const; // Returns the FPGATrackSim-defined physical layer index (see FPGATrackSimPlaneMap.h)
    unsigned getEtaWidth() const { return m_etaWidth; }
    unsigned getPhiWidth() const { return m_phiWidth; }
    int getEtaModule(bool old=false) const { if (old && isRemapped()) return m_etaModule_old; else return m_etaModule; }
    unsigned getPhiModule() const { return m_phiModule; }

    // --- Mapped Location ---
    // NB: isMapped() should return true to access these members
    void setLayer(unsigned v) { m_layer = v; } // This is the logical layer
    void setSection(unsigned v) { m_section = v; }
    int getLayer() const;
    unsigned getSection() const;
    void setRoadID(int roadID) { m_roadID = roadID; }

    // --- Local Coordinates ---
    // The local coordinate is stored as an unsigned int, as in the hardware.
    // To get the actual local coordinate, use the float getCoord functions instead.
    void setPhiIndex(unsigned v) { m_phiIndex = v; }
    void setEtaIndex(unsigned v) { m_etaIndex = v; }
    void setCentroidPhiIndex(float v) { m_centroidPhiIndex = v; }
    void setCentroidEtaIndex(float v) { m_centroidEtaIndex = v; }
    void setPhiCoord(float v) { m_phiCoord = v; }
    void setEtaCoord(float v) { m_etaCoord = v; }
    unsigned getPhiIndex() const { return m_phiIndex; }
    unsigned getEtaIndex() const { return m_etaIndex; }
    float getCentroidPhiIndex() const { return m_centroidPhiIndex; }
    float getCentroidEtaIndex() const { return m_centroidEtaIndex; }
    float getPhiCoord() const { return m_phiCoord; }
    float getEtaCoord() const { return m_etaCoord; }

    void setMaxPhiIndex(int v) { m_maxPhiIndex = v; }
    void setMinPhiIndex(int v) { m_minPhiIndex = v; }
    void setMaxEtaIndex(int v) { m_maxEtaIndex = v; }
    void setMinEtaIndex(int v) { m_minEtaIndex = v; }
    int getMaxPhiIndex() const { return m_maxPhiIndex; }
    int getMinPhiIndex() const { return m_minPhiIndex; }
    int getMaxEtaIndex() const { return m_maxEtaIndex; }
    int getMinEtaIndex() const { return m_minEtaIndex; }

    float getPhiWindow() const { return m_phiWindow; }

    // Converts the hit into a spacepoint. Keeps copies of the local information.
    void makeSpacepoint(float x, float y, float z, float window, FPGATrackSimHit& other, FPGATrackSimMultiTruth& new_truth);

    // Retrieves a (copy of) the original hit with the original local coordinates used to make a SP.
    // If not a SP, returns a copy of this hit.
    const FPGATrackSimHit getOriginalHit() const;

    // These return coordinates for the inner hit in the pair, if a SP
    int getPairedEtaModule() const { return m_pairedEtaModule; }
    unsigned getPairedPhiModule() const { return m_pairedPhiModule; }
    unsigned getPairedSection() const { return m_pairedSection; }
    unsigned getPairedLayer() const { return m_pairedLayer; }
    unsigned getPairedPhysLayer() const { return m_pairedPhysLayer; }
    DetectorZone getPairedDetZone() const { return m_pairedDetZone; }
    SiliconTech getPairedDetType() const { return m_pairedDetType; }

    // These setters are used by FPGATrackSimPlaneMap to map hits that are SPs.
    void setPairedLayer(unsigned v) { m_pairedLayer = v; }
    void setPairedSection(unsigned v) { m_pairedSection = v; }

    // --- Global Coordinates ---
    void setX(float v) { m_originalX = m_x; m_x = v; }
    void setY(float v) { m_originalY = m_y; m_y = v; }
    void setZ(float v) { m_originalZ = m_z; m_z = v; }
    float getX() const { return m_x; }
    float getY() const { return m_y; }
    float getZ() const { return m_z; }
    float getR() const { return TMath::Sqrt(m_x * m_x + m_y * m_y); } // This is cylindrical radius
    float getGPhi() const { return TMath::ATan2(m_y, m_x); }
    float getGCotTheta() const { return m_z / getR(); } // cot(theta) = z/r

    // --- Truth and Other ---
    void setToT(unsigned v) { m_ToT = v; }
    void setBarcode(const HepMcParticleLink::barcode_type& v) { m_barcode = v; }
    void setUniqueID(const HepMcParticleLink::barcode_type& v) { m_uniqueID = v; }
    void setBarcodePt(float v) { m_barcode_pt = v; }
    void setEventIndex(long v) { m_eventindex = v; }
    void setParentageMask(unsigned long v) { m_parentage_mask = v; }
    void setTruth(const FPGATrackSimMultiTruth& v) { m_truth = v; }
    unsigned getToT() const { return m_ToT ;}
    HepMcParticleLink::barcode_type getBarcode() const { return m_barcode; }
    HepMcParticleLink::barcode_type getUniqueID() const { return m_uniqueID; }
    long getEventIndex() const { return m_eventindex; }
    float getBarcodePt() const { return m_barcode_pt; }
    unsigned long getParentageMask() const { return m_parentage_mask; }
    const FPGATrackSimMultiTruth& getTruth() const { return m_truth; }


    ///////////////////////////////////////////////////////////////////////
    // Add information for creating the bytestream TV for ITk pixel and strip
    // The current athena EDM is using ID bytestream encoders and these happens 
    // to have issues and we need to work around them to make TV
    // Hence these special function - Please talk to Haider [sabidi@cern.ch]
    // If these need to functions are being used anywhere outside bytestream encoders

    bool isValidForITkHit() const {return m_isValidForITK;}
    int getStripRowIDForITk() const {return m_stripRowinITKEDM;}
    int getStripChipIDForITk() const {return m_stripChipinITKEDM;}
    int getStripHitMapForITk() const {return m_stripHitMapinITKEDM;}

    void setisValidForITkHit(bool v){ m_isValidForITK = v;}
    void setStripRowIDForITk(int v){ m_stripRowinITKEDM = v;}
    void setStripChipIDForITk(int v){ m_stripChipinITKEDM = v;}
    void setStripHitMapForITk(int v){ m_stripHitMapinITKEDM = v;}

    int getCluster1ID() const {return m_clusterIndex1ForFPGA;}
    void setCluster1ID(int v) {m_clusterIndex1ForFPGA = v;}

    int getCluster2ID() const {return m_clusterIndex2ForFPGA;}
    void setCluster2ID(int v) {m_clusterIndex2ForFPGA = v;}

    // methods for hit cluster equiv
    const std::vector<int>& getPhiIndexVec() const { return m_PhiIndexVec;}
    const std::vector<int>& getEtaIndexVec() const { return m_EtaIndexVec;}  
    const std::vector<unsigned>& getIDHashVec() const { return m_IDhashVec;}
  
    void addPhiIndexToVec(int phi) {m_PhiIndexVec.push_back(phi);}
    void addEtaIndexToVec(int eta) {m_EtaIndexVec.push_back(eta);}
    void addIDHashToVec(unsigned id) {m_IDhashVec.push_back(id);}
    ///////////////////////////////////////////////////////////////////////
    // Other Interface

    friend std::ostream& operator<<(std::ostream&, const FPGATrackSimHit&);

    //remap interface
    bool isRemapped() const;
    void setRemap();

protected:

    // --- Hit Type ---
    HitType m_hitType = HitType::undefined;
    DetectorZone m_detectorZone = DetectorZone::undefined; // barrel / posEC / negEC (0,1,2)
    SiliconTech m_detType = SiliconTech::undefined; // strip / pixel

    // --- Unmapped Location ---
    unsigned int m_identifier = 0; // Global module ID, from offline (32 bit variant)
    unsigned m_identifierHash = 0; // Global module ID hash, from ITk
    unsigned m_layer_disk = 0;     // ITk layer number
    unsigned m_layer_disk_old = 0;     // ITk layer number
    unsigned m_side = 0;           // Side of the strip module
    unsigned m_side_old = 0;           // Side of the strip module
    int m_etaModule = 0; // eta index of the module that the hit is located on
    int m_etaModule_old = 0; // eta index of the module that the hit is located on
    unsigned m_phiModule = 0; // phi index of the module that the hit is located on
    unsigned m_etaWidth = 0;  // clustering width along eta
    unsigned m_phiWidth = 0;  // clustering width in phi direction

    // --- Mapped Location ---
    // isMapped() should return true to access these members
    int m_layer = -1;   // Logical layer this hit is on
    int m_section = -1; // Index of detector element in the logical layer (see FPGATrackSimPlaneMap.h)
    
    // --- Map Tag
    bool m_isRemapped = false;
    // --- Local Coordinates ---
    int m_phiIndex = -1; // phi index for pixel, strip for strip
    int m_etaIndex = -1; // eta index for pixel, row for strip
    float m_centroidPhiIndex = -1; // centroid's phi index for pixel, strip for strip
    float m_centroidEtaIndex = -1; // centroid's eta index for pixel, row for strip
    float m_phiCoord = -999; // local position along phi direction
    float m_etaCoord = -999; // local position along eta direction

    // --- Min/Max Indices used for cluster equivalent to estimate widths ---
    int m_maxPhiIndex = std::numeric_limits<int>::min();
    int m_minPhiIndex = std::numeric_limits<int>::max();
    int m_maxEtaIndex = std::numeric_limits<int>::min();
    int m_minEtaIndex = std::numeric_limits<int>::max();

    // --- Global Coordinates ---
    float m_x = 0;  // Hit position in global coordinates
    float m_y = 0;
    float m_z = 0;

    // Original X/Y/Z coordinates, if this is a spacepoint
    float m_originalX = 0;
    float m_originalY = 0;
    float m_originalZ = 0;

    // Phi window used in spacepoint calculation
    float m_phiWindow = 0;

    // These are the coordinates of the inner layer in a SP
    unsigned m_pairedPhiModule = -1;
    int m_pairedEtaModule = -1;

    // These are the unmapped / physical detector coordinates of the SP inner layer.
    DetectorZone m_pairedDetZone = DetectorZone::undefined; // barrel / posEC / negEC (0,1,2)
    SiliconTech m_pairedDetType = SiliconTech::undefined;        // strip / pixel
    unsigned m_pairedPhysLayer = 0;

    // These are the mapped versions of the inner layer in a SP.
    unsigned m_pairedSection = 0;
    unsigned m_pairedLayer = 0;

    // --- Truth and Other ---
    unsigned m_hw_word = 0; // store Strip and Pixel cluster positions as integers using the same FPGATrackSim_IM HW definition
    unsigned m_ToT = 0; // time over Threshold
    long m_eventindex = 0; // athena event index assigned to this channel
    HepMcParticleLink::barcode_type m_barcode = 0; // geant particle barcode assigned to this channel
    HepMcParticleLink::barcode_type m_uniqueID = 0;
    float m_barcode_pt = 0; // maximum 'pt' for any 'good' geant particle contributing to the channel.
                            // corresponds to the particle with m_barcode
    unsigned long m_parentage_mask = 0; // ancestor information of this channel

    FPGATrackSimMultiTruth m_truth;
    // geant truth data (clusters only). filled during clustering using the m_barcode and
    // m_barcode_frac data for each raw hit in the cluster.


    // For ITk EDM encoding
    int m_clusterIndex1ForFPGA = -1; // Index1 to keep a track of all the clusters used in track in FPGA
    int m_clusterIndex2ForFPGA = -1; // Index2 to keep a track of all the clusters used in track in FPGA

    bool m_isValidForITK = false; // Should this hit be used for ITk EDM testing
    int m_stripRowinITKEDM = -1; // Strip hit row ID in ITk EDM format
    int m_stripChipinITKEDM = -1; // Strip chip ID in ITk EDM format
    int m_stripHitMapinITKEDM = -1; // Strip hit map in ITk EDM format


    int m_roadID = 0;

    std::vector<unsigned> m_IDhashVec; // for a cluster, when we get the equivalent hit info we want to store all the ID hash containing it
    std::vector<int> m_PhiIndexVec; // for a cluster, all the phi indices in it
    std::vector<int> m_EtaIndexVec; // for a cluster, all the eta indices in it
    ClassDefNV(FPGATrackSimHit, 14);
};

// Container of <FPGATrackSimHit const *>
template <class Container>
std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> sortByLayer(Container const& hits)
{
    std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> out;
    for (auto const &hit : hits)
    {
        size_t layer = hit->getLayer();
        if (layer >= out.size()) out.resize(layer + 1);
        out[layer].push_back(hit);
    }
    return out;
}


std::ostream& operator<<(std::ostream& os, HitType t);
std::string to_string(HitType t);

#endif // FPGATrackSimHIT_H

