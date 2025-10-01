/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCABLINGDATA_MDTMEZZANINECARD_H
#define MUONCABLINGDATA_MDTMEZZANINECARD_H

#include <GaudiKernel/MsgStream.h>
#include <CxxUtils/ArrayHelper.h>

#include <array>
#include <iostream>
#include <memory>

///     MdtMezzanineCard -
///     Helper struct to represent the structure of a mezzanine card in a
///     consistent way Each mezzanine card reads out in total 24 tubes of a
///     chamber covering all tube layers of a chamber. Given that a chamber has
///     either 3 or 4 layers, 8 or 6 tubes per layer are read out by the cards.
///     The assignment of the tdc channels to the tubes does not follow a
///     distinct pattern:
///
///                Layer 2:   (01) (03) (05) (07) (06) (04) (02) (00)
///                Layer 1: (09) (11) (13) (15) (14) (12) (10) (08)
///                Layer 0:   (17) (19) (21) (23) (22) (20) (18) (16)
///
///     To preserve a continuous memory layout and to ease the translation of
///     online -> offline numbering scheme, the tubes are numbered sequentially
///                Layer 2:    (16) (17) (18) (19) (20) (21) (22) (23)
///                Layer 1: (08) (09) (10) (11) (12) (13) (14) (15)
///                Layer 0:    (00) (01) (02) (03) (04) (05) (06) (07)
///     The mapping between the two schemes is represented by a 24 long array,
///     where the index is the channel number, tdc in the case of online ->
///     offline, and tube number in the case of online to online.
class MdtMezzanineCard {
   public:
    static constexpr uint8_t NOTSET = 250;

    using MezzCardPtr = std::shared_ptr<const MdtMezzanineCard>;
    using Mapping = std::array<uint8_t, 24>;
    /// @brief Standard constructor of the mezzanine card
    /// @param tdcToTubeMap: array mapping the tdc channels to the tube numbers
    /// covered by the card
    /// @param num_layers: number of tube layers (3 or 4)
    /// @param mezz_id: Global identifier number to map the card in the online
    /// <-> offline conversion
    MdtMezzanineCard(const Mapping& tdcToTubeMap, uint8_t num_layers,
                     uint8_t mezz_id);

    /// @brief returns the tdc channel number
    /// @param tubeLay: tube layer (1-4)
    /// @param tube: global number of the tube in a layer (1-120)
    uint8_t tdcChannel(uint8_t tubeLay, uint8_t tube, MsgStream& msg) const;
    /// @brief returns the tube number
    /// @param tubeLay: tube layer (1-4)
    /// @param tube: global number of the tube in a layer (1-120)
    uint8_t tubeNumber(uint8_t tubeLay, uint8_t tube) const;

    /// checks whether the tdc mapping is complete. I.e.
    ///  -- all channels are uniquely defined
    ///  -- number of layers is 4 or 3
    bool checkConsistency(MsgStream& msg) const;

    /// returns mezzanine database identifier
    uint8_t id() const { return m_mezzId; }
    /// returns the number of layers
    uint8_t numTubeLayers() const { return m_nlay; }
    /// returns the number of tubes per layer;
    uint8_t numTubesPerLayer() const { return m_nTubes; }

    /// @brief Helper struct to pipe the result from the
    /// tdc -> offline channel translation
    struct OfflineCh {
        uint8_t tube{0};
        uint8_t layer{0};
        bool isValid{false};
    };
    OfflineCh offlineTube(uint8_t tdc, MsgStream& msg) const;

    /// @brief Returns the underlying TDC -> Tube conversion map
    const Mapping& tdcToTubeMap() const { return m_tdcToTubes; }
    /// @brief Returns the underlying Tube -> Tdc conversion map
    const Mapping& tubeToTdcMap() const { return m_tubesToTdc; }

   private:
    /// Mapping of the tdc channels to the mezzanine tube number
    Mapping m_tdcToTubes{make_array<uint8_t, 24>(NOTSET)};
    /// Mapping of the mezzanine tube number to the tdc channel
    Mapping m_tubesToTdc{make_array<uint8_t, 24>(NOTSET)};
    /// Number of tube layers
    uint8_t m_nlay{0};
    /// Number of tubes per layer
    uint8_t m_nTubes{0};
    /// Mezzanine database identifier
    uint8_t m_mezzId{0};
};
std::ostream& operator<<(std::ostream& ostr, const MdtMezzanineCard& map);

#endif
