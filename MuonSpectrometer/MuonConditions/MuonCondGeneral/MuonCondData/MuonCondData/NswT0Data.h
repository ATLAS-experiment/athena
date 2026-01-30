/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDDATA_NSWT0DATA_H
#define MUONCONDDATA_NSWT0DATA_H



// Athena includes
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h" 
// STL includes
#include <vector>
#include <optional>

class Identifier;

namespace Muon{
  class IMuonIdHelperSvc;
}


/** @brief Conditions data object to calibrate the timeoff set of 
 *         each individual channel in the NSW */
class NswT0Data {


public:
    NswT0Data(const Muon::IMuonIdHelperSvc* idHelperSvc);
     ~NswT0Data() = default;

    /** @brief Set the t0 calibration constant for a given nsw channel
     *  @param channelId: Identifier of the readout channel to calibrate
     *  @param channelT0: Calibration constantto be applied */
    void setData(const Identifier& channelId, const float channelT0);
    /** @brief Retrieve the t0 calibration constant for a given NSW channel
     *  @param channelId: Identifier of the readout channel to calibrate
     */
    std::optional<float> getT0 (const Identifier& channelId) const;
 
private:
    /** @brief The calibration data is internally stored as
     *        two jagged vectors, one for MM the other for sTGC.
     *        The outer vector is sorted by the module index which is
     *        a composition of the detElementHash && the gasgap.
     *        Transform the channel identifier to the module Index
     * @param channelId: Identifier of the channel to transform */
    std::size_t identToModuleIdx(const Identifier& channelId) const;

    // ID helpers
    const Muon::IMuonIdHelperSvc* m_idHelperSvc{};
    // containers
    using ChannelArray = std::vector<std::vector<std::optional<float>>>;
    ChannelArray m_data_mmg{};
    ChannelArray m_data_stg{};


};

CLASS_DEF( NswT0Data ,148122593  , 1 )
CLASS_DEF( CondCont<NswT0Data> , 69140433 , 1 )

#endif
