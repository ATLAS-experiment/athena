/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONMDT_CABLING_HEDGEHOGBOARD_H
#define MUONMDT_CABLING_HEDGEHOGBOARD_H

#include <array>
#include <string>
#include <cinttypes>
#include <optional>
#include <memory>
#include <iosfwd>
#include <CxxUtils/ArrayHelper.h>

namespace Muon{
    /** @brief  Helper struct to represent the High-voltage pins and a possible connection between them. Likewise the MdtMezzanine Card,
      *         the HedgehogBoard covers exactly 24 tubes and the tube numbering layout may differ depending on whether the Mdt chamber
      *         has 3 or 4 multilayers
      *
      *                             (16) (17) (18) (19) (20) (21) (22) (23)
      *                               (08) (09) (10) (11) (12) (13) (14) (15)
      *                            (00) (01) (02) (03) (04) (05) (06) (07)
      *
      *                               (18) (19) (20) (21) (22) (23)
      *                             (12) (13) (14) (15) (16) (17)
      *                               (06) (07) (08) (09) (10) (11)
      *                            (00) (01) (02) (03) (04) (05)
      *
      *         If the Hedgehogboard is moounted onto a twin chamber, i.e. two HV pins are connected with a time impedance between,
      *         the n-th entry of the array indicates to which k-th tube, the n-th tube is actually connected and the k-th tube contains
      *         an n in order to close the connection. */
    class HedgehogBoard{
        public:
            using HedgehogBoardPtr = std::shared_ptr<const HedgehogBoard>;

            static constexpr unsigned nChPerBoard = 24;
            using Mapping = std::array<uint8_t, nChPerBoard>;

            /** @brief Default constructor */
            HedgehogBoard() = default;
            /** @brief Standard constructor
             *  @param map: Encoding of the hedgehog pins
             *  @param nTubeLay: Number of tube layers 3 or 4
             *  @param boardId: Identifier number of the Hedgehog board  */
            HedgehogBoard(const Mapping& map, 
                          const uint8_t nTubeLay,
                          const uint16_t boardId);
            /** @brief Ordering of hedgehog boards, performed by Identifier */
            bool operator<(const HedgehogBoard& other) const;
            /** @brief Returns the identifier of the hedgehog board */
            uint16_t boardId() const;
            /** @brief Returns the number of tube layers 3 or 4 */
            uint8_t numTubeLayers() const;
            /** @brief Returns the number of tubes per layer */
            uint8_t numTubesPerLayer() const;
            /** @brief Helper struct to return a tube & tube layer pair */
            struct TubeLayer {
                /** @brief Tube number [1- 24/numTubeLayers()] */
                uint8_t tube{0};
                /** @brief Layer number [1- numTubeLayers()] */
                uint8_t layer{0};
                /** @brief Equality operator with another TubeLayer object */
                bool operator==(const TubeLayer& other) const {
                    return tube == other.tube && layer == other.layer;
                }
                /** @brief Inequality operator with another TubeLayer object */
                bool operator!=(const TubeLayer& other) const {
                    return tube != other.tube || layer != other.layer;
                }
                friend std::ostream& operator<<(std::ostream& ostr, const TubeLayer& tubeLay){
                    ostr<<"layer: "<<static_cast<int>(tubeLay.layer)
                        <<", tube: "<<static_cast<int>(tubeLay.tube);
                    return ostr;
                }
            };
            /** @brief Returns the pinNumber of the tubeLayer
             *  @param tubeLay: Helper struct encoding the tube & layer number */
            uint8_t pinNumber(const TubeLayer& tubeLay) const;
            /** @brief  Returns the pin number of the layer - tube pair
             *  @param layer: Tube layer number [1-numTubeLayers()]
             *  @param tube:  Tube number [1-nTubes()] */
            uint8_t pinNumber(uint8_t layer, uint8_t  tube) const;
            /** @brief Returns the number of the pin that's twin to the tubeLayer
             *         If the pin is not short circuited, then the actual tube, layer pair
             *         is returned.
             *  @param tubeLay: Helper struct encoding the tube & layer number */
            uint8_t pairPin(const TubeLayer& tubeLay) const;
            /** @brief Returns the number of the pin that's twin to the tubeLayer
             *         If the pin is not short circuited, then the actual tube, layer pair
             *         is returned.
             *  @param layer: Tube layer number [1-numTubeLayers()]
             *  @param tube:  Tube number [1-nTubes()] */
            uint8_t pairPin(uint8_t layer, uint8_t tube) const;            
            /** @brief Returns the tube & layer short circuited with the given tubeLayer
              *  @param tubeLay: Helper struct encoding the tube & layer number */
            TubeLayer twinPair(const TubeLayer& tubeLay) const;
            /** @brief Returns the tube & layer short circuited with the given tubeLayer
             *  @param layer: Tube layer number [1-numTubeLayers()]
             *  @param tube:  Tube number [1-nTubes()] */
            TubeLayer twinPair(uint8_t layer, uint8_t tube) const;
            /** @brief Define a HV delay in the time of arrival between the primary & twin
             *         electronics signal
             * @param hvTime: Delay time in ns */
            void setHVDelayTime(const double hvTime);
            /** @brief Returns whether the HedgehogBoard has a specifc delay time */
            bool hasHVDelayTime() const;
            /** @brief Returns the HV delay time. If not set 0 is returned  */
            double hvDelayTime() const;
            /** @brief Returns the underlying map */
            const Mapping& data() const;
        private:
            Mapping m_hedgePins{make_array<uint8_t, nChPerBoard>(-1)};
            uint8_t m_nTubeLay{0};
            uint8_t m_nTubePerLay{0};
            uint16_t m_id{0};
            std::optional<double> m_hvDelay{std::nullopt};
    };
    std::ostream& operator<<(std::ostream& ostr, const Muon::HedgehogBoard& board);
}


#endif