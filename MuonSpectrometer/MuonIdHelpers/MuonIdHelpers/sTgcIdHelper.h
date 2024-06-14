/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// ******************************************************************************
// ATLAS Muon Identifier Helpers Package
// -----------------------------------------
// ******************************************************************************

#ifndef MUONIDHELPERS_STGCIDHELPER_H
#define MUONIDHELPERS_STGCIDHELPER_H

// Includes
#include "MuonIdHelpers/MuonIdHelper.h"

// ******************************************************************************
// class sTgcIdHelper
// ******************************************************************************//
// Description
// -----------
// This factory class constructs sTGC identifiers and ranges and provides access
// to the levels.  ATLAS note ATL-MUON-2001-014 provides a complete description
// of the hierarchical identifier scheme.  sTgcIdHelper provides an interface to the
// following fields of the identifier.
//
// Field           Range               Notes
// ==============================================================================
// diehl: todo: update this section for sTGC
// StationName     unsigned integer    maps to T1F,T3E,etc.
// StationEta      [-4,-1]             backward endcap (-1 at lowest R)
//                 [1,4]               forward endcap (1 at lowest R)
// StationPhi      [1,8]               increases with phi
// Technology      [4]                 maps to sTGC
// GasGap          [1,2]               doublet: increases with |Z|
//                 [1,3]               triplet: increases with |Z|
// ChannelType     [0,2]               0 if pad, 1 if strip, 2 if wire
// Channel         [1,n]               increases with R         for channelType=1 (strip)
//                                     increases with phi       for channelType=2 (wire)
//                                     increases with phi and R for channelType=0 (pad)
//
// ==============================================================================
//
// Inheritance
// -----------
// Inherits from MuonIdHelpers/MuonIdHelper.
//
// Authors
// ------
// Edward Diehl <diehl@umich.edu> based on TgcIdHelper.h
// Nektarios Chr. Benekos <nectarios.benekos@cern.ch>
// Philipp Fleischmann <philipp.fleischmann@cern.ch>
//
// ******************************************************************************

class sTgcIdHelper : public MuonIdHelper {
public:
    // Constructor
    sTgcIdHelper();

    // Destructor
    virtual ~sTgcIdHelper() = default;

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

    Identifier channelID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channelType, int channel) const;
    Identifier channelID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channelType, int channel,
                         bool& isValid) const;

    Identifier channelID(const std::string& stationNameStr, int stationEta, int stationPhi, int multilayer, int gasGap, int channelType,
                         int channel) const;
    Identifier channelID(const std::string& stationNameStr, int stationEta, int stationPhi, int multilayer, int gasGap, int channelType,
                         int channel, bool& isValid) const;

    Identifier channelID(const Identifier& id, int multilayer, int gasGap, int channelType, int channel) const;
    Identifier channelID(const Identifier& id, int multilayer, int gasGap, int channelType, int channel, bool& isValid) const;

    Identifier padID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channelType, int padEta,
                     int padPhi) const;
    Identifier padID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channelType, int padEta, int padPhi,
                     bool& isValid) const;

    Identifier padID(const std::string& stationNameStr, int stationEta, int stationPhi, int multilayer, int gasGap, int channelType,
                     int padEta, int padPhi) const;
    Identifier padID(const std::string& stationNameStr, int stationEta, int stationPhi, int multilayer, int gasGap, int channelType,
                     int padEta, int padPhi, bool& isValid) const;

    Identifier padID(const Identifier& id, int multilayer, int gasGap, int channelType, int padEta, int padPhi) const;
    Identifier padID(const Identifier& id, int multilayer, int gasGap, int channelType, int padEta, int padPhi, bool& isValid) const;

    Identifier parentID(const Identifier& id) const;

    Identifier multilayerID(const Identifier& channeldID) const;
    Identifier multilayerID(const Identifier& moduleID, int multilayer) const;
    Identifier multilayerID(const Identifier& moduleID, int multilayer, bool& isValid) const;

    
    /*
    For the sTGCs each layer of a quadruplet is read out by two front end boards, one covering the strips (sFEB) and one covering the pads and wires (pFEBs).
    This helper function creates a dummy identifier which can be associated with a front end board, which is primarily needed to deal with the correlation between DCS data and the hits in athena
    sFEBs are always mapped to channel type strip and to channel one of the layer that is readout by this board. pFEBs are mapped to pad channel type and channel one of a layer.
    The last fuction which just takes the identifier as an input is meant to convert hit identifiers to front end identifiers. It retruns the same fields as the input identifier except for channel which is set to 1 and for wire identifiers the channel type is set to pad.
    */
    
    Identifier febID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channelType) const;
    Identifier febID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channelType, bool& isValid) const;
    Identifier febID(const std::string& stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channelType) const;
    Identifier febID(const std::string& stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channelType, bool& isValid) const;
    Identifier febID(const Identifier& channelID, int channelType) const;
    Identifier febID(const Identifier& channelID, int channelType, bool& isValid) const;
    Identifier febID(const Identifier& channelID) const;


    /*
    One layer of sTGCs is segmented into 4 high voltage sections. Eta station 1 is divided into two hv sections while eta stations 2 and 3 consist of one HV section each. The HV sections are numbered from 1-4. The following functions introduce an identifier that is mapping to these HV sections. For eta stations 2 and 3 it returns the gas gap id, e.g. channel 1 on a give gas gap. For station eta 1, the channel is set sto one for the inner HV section while it is set to 100 for the outer HV section. This is an arbitary mapping, channel 100 does not correspond to the transition between the two HV section (the location is different depending on the module type and the gas gap). The hv identifier is arbitrily set to be a strip type channel.
    */
    Identifier hvID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, bool isInnerQ1) const;
    Identifier hvID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, bool isInnerQ1, bool& isValid) const;
    Identifier hvID(const std::string& stationName, int stationEta, int stationPhi, int multilayer, int gasGap, bool isInnerQ1) const;
    Identifier hvID(const std::string& stationName, int stationEta, int stationPhi, int multilayer, int gasGap, bool isInnerQ1, bool& isValid) const;
    Identifier hvID(const Identifier& channelId, bool isInnerQ1)const;
    Identifier hvID(const Identifier& channelId, bool isInnerQ1, bool& isValid) const;

    // for an Identifier id, get the list of the daughter readout channel ids
    void idChannels(const Identifier& id, std::vector<Identifier>& vect) const;

    // Access to levels: missing field returns 0
    int gasGap(const Identifier& id) const override;
    int multilayer(const Identifier& id) const;
    int channelType(const Identifier& id) const;
    bool measuresPhi(const Identifier& id) const override;
    int channel(const Identifier& id) const override;
    int numberOfMultilayers(const Identifier& id) const;
    int padEta(const Identifier& id) const;
    int padPhi(const Identifier& id) const;

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
    static int channelTypeMin() ;
    static int channelTypeMax() ;
    static int channelMin() ;
    static int channelMax() ;
    static int padEtaMin() ;
    static int padEtaMax() ;
    static int padPhiMin() ;
    static int padPhiMax() ;

    // Access to min and max of level ranges
    int stationEtaMin(const Identifier& id) const;
    int stationEtaMax(const Identifier& id) const;
    int stationPhiMin(const Identifier& id) const;
    int stationPhiMax(const Identifier& id) const;
    int multilayerMin(const Identifier& id) const;
    int multilayerMax(const Identifier& id) const;
    int gasGapMin(const Identifier& id) const;
    int gasGapMax(const Identifier& id) const;
    int channelTypeMin(const Identifier& id) const;
    int channelTypeMax(const Identifier& id) const;
    int channelMin(const Identifier& id) const;
    int channelMax(const Identifier& id) const;

    // Public validation of levels
    bool valid(const Identifier& id) const;
    bool validElement(const Identifier& id) const;

    // Type indices
    enum sTgcChannelTypes { Pad = 0, Strip = 1, Wire = 2 };

private:
    bool isStNameInTech(const std::string& stationName) const override;

    int init_id_to_hashes();
    
    /// Small and big wedges
    static constexpr unsigned int s_stDim = 2;
    /// -3, -2, -1, 1, 2, 3
    static constexpr unsigned int s_etaDim = 6;
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
    size_type m_CHANNELTYPE_INDEX{7};

    IdDictFieldImplementation m_gap_impl;
    IdDictFieldImplementation m_typ_impl;
    IdDictFieldImplementation m_cha_impl;
    IdDictFieldImplementation m_mplet_impl;

    // Check level values
    bool validElement(const Identifier& id, int stationName, int stationEta, int stationPhi) const;
    bool validChannel(const Identifier& id, int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channelType,
                      int channel) const;
    bool validChannel(const Identifier& id, int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channelType,
                      int padEta, int padPhi) const;

    // Utility methods
    int stgcTechnology() const;
    bool LargeSector(int stationName) const;
    bool SmallSector(int stationName) const;

    // Level indices
    enum sTgcIndices { MultilayerIndex = 5, GasGapIndex = 6, ChannelTypeIndex = 7, ChannelIndex = 8 };

    // Level ranges
    enum sTgcRanges {
        StationEtaMin = -4,
        StationEtaMax = 4,
        StationPhiMin = 1,
        StationPhiMax = 8,
        MultilayerMin = 1,
        MultilayerMax = 2,
        GasGapMin = 1,
        GasGapMax = 4,
        ChannelTypeMin = 0,
        ChannelTypeMax = 2,
        ChannelMin = 1,
        ChannelMax = 315,
        PadPhiMin = 1,
        PadPhiMax = 8,
        PadEtaMin = 1,
        PadEtaMax = 18  // arbitrary values. these values should be taken from an XML
    };
};  // end class sTgcIdHelper
/*******************************************************************************/
// For backwards compatibility
typedef sTgcIdHelper sTGC_ID;

CLASS_DEF(sTgcIdHelper, 4174, 1)

#endif  // MUONIDHELPERS_STGCIDHELPER_H
