/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonCablingData/HedgehogBoard.h>
#include <cassert>
#include <iostream>
#include <iomanip>


namespace Muon {
    std::ostream& operator<<(std::ostream& ostr, const Muon::HedgehogBoard& board) {
        ostr<<"boardID: "<<static_cast<int>(board.boardId())<<", ";
        ostr<<"nLayers: "<<static_cast<int>(board.numTubeLayers())<<", ";
        ostr<<"nTubesPerLay: "<<static_cast<int>(board.numTubesPerLayer())<<", ";
        ostr<<"mapping: "<<std::endl;
        for (uint8_t lay = board.numTubeLayers(); lay > 0; --lay) {
            if (!(lay % 2))
                ostr << "   ";
            for (uint8_t tube = 1; tube <= board.numTubesPerLayer(); ++tube) {
                ostr << std::setw(5) << static_cast<int>(board.pairPin(lay, tube));
            }
            ostr << std::endl;
        }
        return ostr;
    }

    using TubeLayer = HedgehogBoard::TubeLayer;
    HedgehogBoard::HedgehogBoard(const Mapping& map, 
                                 const uint8_t nTubeLay,
                                 const uint16_t boardId):
        m_hedgePins{map},
        m_nTubeLay{nTubeLay},
        m_id{boardId} {
        assert(nTubeLay == 3 || nTubeLay == 4);
        m_nTubePerLay = m_hedgePins.size() / nTubeLay;    
    }
    bool HedgehogBoard::operator<(const HedgehogBoard& other) const {
        return boardId() < other.boardId();
    }
    uint16_t HedgehogBoard::boardId() const { return m_id; }
    uint8_t HedgehogBoard::numTubeLayers() const { return m_nTubeLay; }
    uint8_t HedgehogBoard::numTubesPerLayer() const { return m_nTubePerLay; }
    const HedgehogBoard::Mapping& HedgehogBoard::data() const { return m_hedgePins; }
    uint8_t HedgehogBoard::pinNumber(const TubeLayer& tubeLay) const {
        return pinNumber(tubeLay.layer, tubeLay.tube);
    }
    uint8_t HedgehogBoard::pinNumber(uint8_t layer, uint8_t tube) const {
        uint8_t pin = (layer-1) * m_nTubePerLay + (tube -1) % m_nTubePerLay;
        assert(pin < m_hedgePins.size()); 
        return pin;
    }
    uint8_t HedgehogBoard::pairPin(const TubeLayer& tubeLay) const {
        return m_hedgePins[pinNumber(tubeLay)];
    }
    uint8_t HedgehogBoard::pairPin(uint8_t layer, uint8_t tube) const {
        return m_hedgePins[pinNumber(layer, tube)];
    }

    TubeLayer HedgehogBoard::twinPair(const TubeLayer& tubeLay) const {
        return twinPair(tubeLay.layer, tubeLay.tube);
    }
    TubeLayer HedgehogBoard::twinPair(uint8_t layer, uint8_t tube) const {
        const uint8_t pin = pinNumber(layer, tube);
        const uint8_t twin = m_hedgePins[pin];
        TubeLayer result{};
        if (pin != twin) {
            result.layer = (twin - twin%m_nTubePerLay) / m_nTubePerLay + 1;
            result.tube = (twin % m_nTubePerLay) + 1;
        } else {
            result.tube = tube;
            result.layer = layer;
        }
        return result;
    }
    void HedgehogBoard::setHVDelayTime(const double hvTime) {
        m_hvDelay = std::make_optional<double>(hvTime);
    }
    bool HedgehogBoard::hasHVDelayTime() const {
        return m_hvDelay.has_value();
    }           
    double HedgehogBoard::hvDelayTime() const {
        return m_hvDelay.value_or(0.);
    }
}
