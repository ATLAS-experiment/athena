/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CNVTOOLS_TGC_HID2RESRCID
#define MUONTGC_CNVTOOLS_TGC_HID2RESRCID

#include <inttypes.h>

#include <vector>

class TgcRdo;
class Identifier;

namespace Muon {
class TgcCablingMap;

/** @class TGC_Hid2RESrcID
 *  This class provides conversion between TGC RDO Id and RESrcID.
 *  RESrcID is used for identifying each ROD.
 *
 *  @author Susumu Oda <Susumu.Oda@cern.ch>
 *
 *  This class was developed by Tadashi Maeno based on
 *  MDT_Hid2RESrcID written by Naples.
 */

class TGC_Hid2RESrcID {
   public:
    /** Constrcutor */
    TGC_Hid2RESrcID() = default;

    /** Destrcutor */
    ~TGC_Hid2RESrcID() = default;

    /** Make a ROD Source ID for TGC RDO. */
    static uint32_t getRodID(const TgcRdo* rdo);
    /** Make a ROD Source ID for SubDetector ID and ROD ID. */
    static uint32_t getRodID(uint16_t subDetectorId, uint16_t rodId);
    /** Make a ROD Source ID for TgcDigitCollection. */
    static uint32_t getRodID(const Identifier& offlineId,
                             const TgcCablingMap* cabling);
    /** Make a ROB Source ID from a ROD source ID. */
    static uint32_t getRobID(uint32_t rod_id);
    /** Make a ROS Source ID from a ROB source ID. */
    static uint32_t getRosID(uint32_t rob_id);
    /** Make a SubDetector ID from ROS source ID. */
    static uint32_t getDetID(uint32_t ros_id);

    /** Return all the ROB IDs. */
    const std::vector<uint32_t>& allRobIds() const { return m_robIDs; }

    /** Fill all the ROB IDs. */
    void fillAllRobIds();

   private:
    std::vector<uint32_t> m_robIDs;
};

}  // namespace Muon

#endif  // MUONTGC_CNVTOOLS_TGC_HID2RESRCID
