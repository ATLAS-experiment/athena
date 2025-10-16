/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// ******************************************************************************
// ATLAS Muon Identifier Helpers Package
// -----------------------------------------
// ******************************************************************************

#ifndef MUONIDHELPERS_MMIDHELPER_H
#define MUONIDHELPERS_MMIDHELPER_H

// Includes
class MsgStream;
#include "MuonIdHelpers/MuonIdHelper.h"

// ******************************************************************************
// class MmIdHelper
// ******************************************************************************//
// Description
// -----------
// This factory class constructs MicroMegas (MM) identifiers and ranges and provides access
// to the levels.  ATLAS note ATL-MUON-2001-014 provides a complete description
// of the hierarchical identifier scheme.
// MmIdHelper provides an interface to the following fields of the identifier.
//
// Field           Range               Notes
// ==============================================================================
// nectar: todo: update this section for MM
// StationName     unsigned integer    maps to T1F,T3E,etc.
// StationEta      [-5,-1]             backward endcap (-1 at lowest R)
//                 [1,5]               forward endcap (1 at lowest R)
// StationPhi      [1,48]              endcap: increases with phi
//                 [1,24]              forward: increases with phi
// Technology      [5]                 maps to MM --> <label name="MM" value="5" /> in IdDictMuonSpectrometer_R.01.xml
// Multilayer       [1,2]               barrel: increases with R
//                                     endcap: increases with |Z|
// GasGap          [1,2]               doublet: increases with |Z|
//                 [1,3]               triplet: increases with |Z|
// Channel         [1,n]               increases with R for IsStrip=0 (wire gang)
//                                     increases with phi for IsStrip=1 (strip)
// ==============================================================================
//
// Inheritance
// -----------
// Inherits from MuonIdHelpers/MuonIdHelpers
//
// Author
// ------
// Nektarios Chr. Benekos <nectarios.benekos@cern.ch>
// Jochen Meyer <Jochen.Meyer@cern.ch>
// ******************************************************************************

class MmIdHelper : public MuonIdHelper {
public:
    // Constructor
    MmIdHelper();

    // Destructor
    virtual ~MmIdHelper() = default;

    ///////////// compact identifier stuff begins //////////////////////////////////////

    /// Initialization from the identifier dictionary
    virtual int initialize_from_dictionary(const IdDictMgr& dict_mgr) override;
    virtual int get_module_hash(const Identifier& id, IdentifierHash& hash_id) const override;
    virtual int get_detectorElement_hash(const Identifier& id, IdentifierHash& hash_id) const override;

    ///////////// compact identifier stuff ends   //////////////////////////////////////

    // Identifier builders
    Identifier elementID(int stationName, int stationEta, int stationPhi) const;
    Identifier elementID(int stationName, int stationEta, int stationPhi, bool& isValid) const;

    Identifier elementID(const std::string& stationNameStr, int stationEta, int stationPhi) const;
    Identifier elementID(const std::string& stationNameStr, int stationEta, int stationPhi, bool& isValid) const;

    Identifier elementID(const Identifier& channelID) const;

    Identifier channelID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channel) const;
    Identifier channelID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channel, bool& isValid) const;

    Identifier channelID(const std::string& stationNameStr, int stationEta, int stationPhi, int multilayer, int gasGap, int channel) const;
    Identifier channelID(const std::string& stationNameStr, int stationEta, int stationPhi, int multilayer, int gasGap, int channel,
                         bool& isValid) const;

    Identifier channelID(const Identifier& id, int multilayer, int gasGap, int channel) const;
    Identifier channelID(const Identifier& id, int multilayer, int gasGap, int channel, bool& isValid) const;

    Identifier parentID(const Identifier& id) const;

    Identifier multilayerID(const Identifier& channeldID) const;
    Identifier multilayerID(const Identifier& moduleID, int multilayer) const;
    Identifier multilayerID(const Identifier& moduleID, int multilayer, bool& isValid) const;

    Identifier pcbID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int pcb) const;
    Identifier pcbID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int pcb, bool& isValid) const;
    Identifier pcbID(const std::string& stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int pcb) const;
    Identifier pcbID(const std::string& stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int pcb, bool& isValid) const;
    Identifier pcbID(const Identifier& channelID, int pcb) const;
    Identifier pcbID(const Identifier& channelID, int pcb, bool& isValid) const;
    Identifier pcbID(const Identifier& channelID) const;
    /*
    One readout board of the Micromegas cover 512 channels and they are named according to their radial position in a layer (0-15).
    This helper function creates a dummy identifier which can be associated with a front end board, which is primarily needed to deal with the correlation between DCS data and the hits in athena.
    The identifier always points to the innermost channel that is read out by  a given front end board, i.e. radius*512 + 1 (_1 since athena counts from one). For the outer quads it is (radius-10)*512+1 since athena restarts from channel 1 in the outer quads but the DAQ convention for radius counts for the full sector and only febs 0-9 are reading out the inner quad
    The las function is meant to translate an athena identifier to a feb ID. It returns the same fields except for the channel which is set to the coresponding feb channel by doing ((channel-1)/512)*512+1   (one also needs to take into account the quad, see the actual implementation for that)
    */
    Identifier febID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int radius) const;
    Identifier febID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int radius, bool& isValid) const;
    Identifier febID(const std::string& stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int radius) const;
    Identifier febID(const std::string& stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int radius, bool& isValid) const;
    Identifier febID(const Identifier& channelID, int radius) const;
    Identifier febID(const Identifier& channelID, int radius, bool& isValid) const;
    Identifier febID(const Identifier& channelID) const;


    // for an Identifier id, get the list of the daughter readout channel ids
    void idChannels(const Identifier& id, std::vector<Identifier>& vect) const;

    // Access to levels: missing field returns 0
    int gasGap(const Identifier& id) const override;
    int multilayer(const Identifier& id) const;
    int channel(const Identifier& id) const override;
    bool isStereo(const Identifier& id) const;
    bool measuresPhi(const Identifier& id) const override;  // Returns false

    int numberOfMultilayers(const Identifier& id) const;

    // Access to min and max of level ranges
    // to be remove when we moved to compact ids
    static int stationEtaMin() ;
    static int stationEtaMax() ;
    static int stationPhiMin() ;
    static int stationPhiMax() ;
    static int multilayerMin() ;
    static int multilayerMax() ;
    static int gasGapMin() ;
    static int gasGapMax() ;
    static int channelMin() ;
    static int channelMax() ;

    // Access to min and max of level ranges
    int stationEtaMin(const Identifier& id) const;
    int stationEtaMax(const Identifier& id) const;
    int stationPhiMin(const Identifier& id) const;
    int stationPhiMax(const Identifier& id) const;
    int multilayerMin(const Identifier& id) const;
    int multilayerMax(const Identifier& id) const;
    int gasGapMin(const Identifier& id) const;
    int gasGapMax(const Identifier& id) const;
    int channelMin(const Identifier& id) const;
    int channelMax(const Identifier& id) const;

    // Utility methods
    static int sectorType(const std::string& stationName, int stationEta) ;
    int sectorType(int stationName, int stationEta) const;

    // Public validation of levels
    bool valid(const Identifier& id) const;
    bool validElement(const Identifier& id) const;

private:
    static int getFirstPcbChnl(int stationEta, int pcb) ;
    static int getFirstRadiusChnl(int stationEta, int pcb) ;
    bool isStNameInTech(const std::string& stationName) const override;

    int stationEtaMin(const ExpandedIdentifier& expId) const;
    int stationEtaMax(const ExpandedIdentifier& expId) const;
    int stationPhiMin(const ExpandedIdentifier& expId) const;
    int stationPhiMax(const ExpandedIdentifier& expId) const;
    int multilayerMin(const ExpandedIdentifier& expId) const;
    int multilayerMax(const ExpandedIdentifier& expId) const;
    int gasGapMin(const ExpandedIdentifier& expId) const;
    int gasGapMax(const ExpandedIdentifier& expId) const;
    int channelMin(const ExpandedIdentifier& expId) const;
    int channelMax(const ExpandedIdentifier& expId) const;

    int init_id_to_hashes();
    /// Small and big wedges
    static constexpr unsigned int s_stDim = 2;
    /// -2, -1 , 1, 2
    static constexpr unsigned int s_etaDim = 4;
    /// 8 phi station
    static constexpr unsigned int s_phiDim = 8;
    /// 2 multilayer
    static constexpr unsigned int s_mlDim = 2;

    static constexpr unsigned int s_modHashDim = s_stDim * s_etaDim * s_phiDim;
    static constexpr unsigned int s_detHashDim = s_modHashDim * s_mlDim;
    
    std::array<unsigned int, s_modHashDim> m_module_hashes{};    
    std::array<unsigned int, s_detHashDim> m_detectorElement_hashes{};

    unsigned int moduleHashIdx(const Identifier& id) const;
    unsigned int detEleHashIdx(const Identifier& id) const;
    /// Minimal station index found
    unsigned int m_stationShift{std::numeric_limits<unsigned int>::max()};
 
    // compact id indices
    size_type m_GASGAP_INDEX{6};

    IdDictFieldImplementation m_mplet_impl;
    IdDictFieldImplementation m_gap_impl;
    IdDictFieldImplementation m_cha_impl;

    // Check level values
    bool validElement(const ExpandedIdentifier& expId, int stationName, int stationEta, int stationPhi) const;
    bool validChannel(const Identifier& id, int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channel) const;

    // Utility methods
    int mmTechnology() const;
    bool LargeSector(int stationName) const;
    bool SmallSector(int stationName) const;

    // Level indices
    enum MmIndices { MultilayerIndex = 5, GasGapIndex = 6, ChannelIndex = 7 };

    // Level ranges
    enum MmRanges {
        StationEtaMin = 0,
        StationEtaMax = 3,
        StationPhiMin = 1,  // ED: change for MM
        StationPhiMax = 8,  // ED: change for MM
        MultilayerMin = 1,
        MultilayerMax = 2,
        GasGapMin = 1,
        GasGapMax = 4,
        ChannelMin = 1,
        ChannelMax = 200
    };

};  // end class MmIdHelper
/*******************************************************************************/
// For backwards compatibility
typedef MmIdHelper MM_ID;

CLASS_DEF(MmIdHelper, 4175, 1)

#endif  // MUONIDHELPERS_MMIDHELPER_H
