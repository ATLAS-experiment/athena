/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ITkPixelCablingData_h
#define ITkPixelCablingData_h
/**
  * @file ITkPixelCablingData/ITkPixelCablingData.h
  * @author Ondra Kovanda, Shaun Roe, Fabrice Balli
  * @date June 2024
  * @brief Data object containing the offline-online mapping for ITkPixels
  */

#include "ITkPixelCabling/ITkPixelOnlineId.h"

// Athena includes
#include "Identifier/Identifier.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"

//STL
#include <unordered_map>
#include <iosfwd>

namespace ITkPixelCabling {

    //types of modules we have
    enum ModuleType {
        SimpleQuad,
        MergedQuad,
        IBTriplet,
        IECTriplet,
        Undefined
    };

    //types of coordinate transformations
    //kept separate from the chip types
    //in case per-module specifics are
    //needed
    enum TransformType {
        NominalQuad,
        NominalIBTriplet,
        NominalIECTriplet,
        UndefinedTransform
    };

    //We need to know what module type
    //we're dealing with from the readout
    //PoV, so that we can translate accordingly.
    template <class ID>
    struct ModuleInfo {
        ID id;
        ModuleType type {};
        TransformType transform {};
    };

    //Update to a realistic ROB structure that has ROBs
    //identified with sourceID (32b) and holds data from
    //multiple 'elinks' (front-ends, to all practicality)
    //in the payload, each data block prepended by
    //'DetectorResourceID'.

    //For **encoding**
    //1. we read in an RDO that contains offline ID only
    //2. that RDO also gives us chipID based on where in
    //   the module the row, col are
    //3. from that we can build (offline ID) << 2 | chipID,
    //   which gives us (almost) the entire DetectorResourceID
    //4. the online part of the DetectorResourceID has to come from cabling
    //5. What we need for encoding is therefore Offline part of
    //   DetectorResourceID -> {online part, sourceID} mapping. That way we
    //   can fill in a map of <(sourceID << 32 | DetectorResourceID), HitMap>
    //   and encode and place it in the ROB fragments accordingly with no further
    //   need for cabling.
    //   
    //   We will use ITkPixelOnlineId to store sourceID | DetectorResourceId in
    //   64b.

    //define transforms, assuming
    // Y | 2 | 3 |
    //   | 0 | 1 |
    //           X
    //to be confirmed and revisited

    using TransformFn = void(*)(uint16_t&, uint16_t&);

    //LUT for quad translation functions indexed by chip ID
    //The RD53C/ITkPixV2 chip dimensions are 400 columns, 384 rows,
    //indexing starts at 0, hence the possible values of col in [0, 399]
    //and row in [0, 383]. A quad is 2x2 chips, hence the total dimensions
    //of 800 columns and 768 rows, again indexed from 0, i. e. col in [0, 799]
    //and row in [0, 767].
    static constexpr TransformFn nominalQuadTable[4] = {
        []([[maybe_unused]] uint16_t& col, uint16_t& row){row = 383 - row;},
        []([[maybe_unused]] uint16_t& col, uint16_t& row){row = 383 - row; col += 400;},
        []([[maybe_unused]] uint16_t& col, uint16_t& row){row = 767 - row;},
        []([[maybe_unused]] uint16_t& col, uint16_t& row){row = 767 - row; col += 400;}
    };

    static constexpr TransformFn ibTransformFn =
        [](uint16_t& col, uint16_t& row){
            row = 2 * (383 - row) + (col & 1);
            col >>= 1;
        };

    static constexpr TransformFn iecTransformFn = 
        [](uint16_t& col, uint16_t& row) {
            std::swap(col, row);
        };
    
    static constexpr TransformFn nominalQuadInverseTable[4] = {
        []([[maybe_unused]] uint16_t& col, uint16_t& row){row = 383 - row;},
        []([[maybe_unused]] uint16_t& col, uint16_t& row){row = 383 - row; col -= 400;},
        []([[maybe_unused]] uint16_t& col, uint16_t& row){row = 767 - row;},
        []([[maybe_unused]] uint16_t& col, uint16_t& row){row = 767 - row; col -= 400;}
    };

    static constexpr TransformFn ibInverseTransformFn =
        [](uint16_t& col, uint16_t& row) {
            const uint16_t parity = row & 1;
            row = 383 - (row >> 1);
            col = (col << 1) | parity;
        };

    static constexpr TransformFn iecInverseTransformFn =
        [](uint16_t& col, uint16_t& row) {
            std::swap(col, row);
        };


    static inline void chipToModuleTransform(const TransformType& transform, const uint8_t& chipID, uint16_t& col, uint16_t& row){
        switch (transform){
            case TransformType::NominalQuad:
                nominalQuadTable[chipID](col, row);
                break;
            case TransformType::NominalIBTriplet:
                ibTransformFn(col, row);
                break;
            case TransformType::NominalIECTriplet:
                iecTransformFn(col, row);
                break;
            case TransformType::UndefinedTransform:
                break;
        }
    }

    static inline void chipToModuleInverseTransform(const TransformType& transform, const uint8_t& chipID, uint16_t& col, uint16_t& row){
        switch (transform){
            case TransformType::NominalQuad:
                nominalQuadInverseTable[chipID](col, row);
                break;
            case TransformType::NominalIBTriplet:
                ibInverseTransformFn(col, row);
                break;
            case TransformType::NominalIECTriplet:
                iecInverseTransformFn(col, row);
                break;
            case TransformType::UndefinedTransform:
                break;
        }
    }

}

class ITkPixelCablingData{
public:
  ///stream extraction to read value from stream into ITkPixelCablingData
  friend std::istream& operator>>(std::istream & is, ITkPixelCablingData & cabling);
  ///stream insertion for debugging
  friend std::ostream& operator<<(std::ostream & os, const ITkPixelCablingData & cabling);
  bool empty() const;
  std::size_t size() const;
  ITkPixelOnlineId onlineId(const Identifier & id) const;
  Identifier offlineId(const ITkPixelOnlineId& id) const;

  ITkPixelCabling::ModuleInfo<ITkPixelOnlineId> onlineModuleInfo(const Identifier & id) const;
  ITkPixelCabling::ModuleInfo<Identifier> offlineModuleInfo(const ITkPixelOnlineId & id) const;

  //Updating to a realistic ROB structure
  ITkPixelOnlineId onlineId(const uint32_t& offlineDetectorResourceID) const;
  const std::unordered_map<uint32_t, ITkPixelOnlineId>& onlineIdMap() const {return m_offlineDetectorResourceID2OnlineIdMap;}
  const std::vector<uint32_t>& sourceIDs() const {return m_sourceIDs;}
  ITkPixelCabling::TransformType transformType(const uint32_t& moduleID) const;
  template<typename Func> void forEachOffOn(Func&& f) const {
    for (const auto& [key, val] : m_offlineDetectorResourceID2OnlineIdMap){
        f(key, val);
    }
  }

  // chip ID number from 0 to 3
  static uint8_t chipID(const ITkPixelCabling::TransformType& t, const uint16_t& col, const uint16_t& row) ;

  //Add entry to the offline->online map. This is only for producing test streams,
  //from MC, and needs to propagate the type of the module. We also can at most map
  //with 4-fold degeneracy due to non-merged quads, which have 4 online IDs mapped to
  //a single offline ID.
  void addEntryOffOn(const Identifier& idOff, const ITkPixelOnlineId& idOn);
  void addEntryOffOn(const Identifier& idOff, const ITkPixelCabling::ModuleInfo<ITkPixelOnlineId>& moduleInfo);
  
  //Updating to a realistic ROB structure, working with the detectorResourceIDs
  //a la Data Handler, mapping the 'offline part' of the detectorResourceID (module ID << 2 | chipID)
  //to the full detectorResourceId and sourceID (a. k. a. ROB ID)
  void addEntryOffOn(const uint32_t& offlineDetectorResourceID, const ITkPixelOnlineId& onlineId);

  void addSourceID(const uint32_t& sourceID);

  void addTransformType(const uint32_t& moduleID, const ITkPixelCabling::TransformType& transform);

  //Add entry to the online->offline map. This is for decoding. For quick access,
  //these maps can also cache the type of the module, so that we know how to translate
  //the hit coordinates from chip to module
  void addEntryOnOff(const ITkPixelOnlineId& idOn, const Identifier& idOff);
  void addEntryOnOff(const ITkPixelOnlineId& idOn, const ITkPixelCabling::ModuleInfo<Identifier>& moduleInfo);

  void print() const;

  //Add entries to the 

private:
  //offline->online base map
  std::unordered_map<Identifier, ITkPixelOnlineId> m_offline2OnlineMap;

  //offline->online base map with info on module type
  std::unordered_map<Identifier, ITkPixelCabling::ModuleInfo<ITkPixelOnlineId>> m_offline2ModuleInfoMap;

  //simple online->offline map
  std::unordered_map<ITkPixelOnlineId, Identifier> m_online2OfflineMap;

  //online->offline map with info on module type
  std::unordered_map<ITkPixelOnlineId, ITkPixelCabling::ModuleInfo<Identifier>> m_online2ModuleInfoMap;

  //online base -> module type
  std::unordered_map<ITkPixelOnlineId, ITkPixelCabling::ModuleType> m_online2ModuleType;

  //offline -> module type
  std::unordered_map<Identifier, ITkPixelCabling::ModuleType> m_offline2ModuleType;

  //offline part of DetectorResourceID -> {online part of DetectorResourceID, sourceID}
  std::unordered_map<uint32_t, ITkPixelOnlineId> m_offlineDetectorResourceID2OnlineIdMap{};

  //list of sourceIDs
  std::vector<uint32_t> m_sourceIDs{};

  //list of IDs in l0 barrel - needed for decoding
  std::unordered_map<uint32_t, ITkPixelCabling::TransformType> m_module2TransformTypeMap{};

};
// Magic "CLassID" for storage/retrieval in StoreGate
// These values produced using clid script.
// "clid ITkPixelCablingData"
// 140860927 ITkPixelCablingData
CLASS_DEF( ITkPixelCablingData , 140860927 , 1 );
//"clid -cs ITkPixelCablingData"
//143807283
CONDCONT_DEF( ITkPixelCablingData , 143807283 );
#endif
