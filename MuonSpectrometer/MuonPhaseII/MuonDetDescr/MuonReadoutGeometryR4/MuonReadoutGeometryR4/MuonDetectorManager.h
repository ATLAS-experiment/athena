/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRY_MUONDETECTORMANAGER_H
#define MUONREADOUTGEOMETRY_MUONDETECTORMANAGER_H

#include "MuonReadoutGeometryR4/MuonDetectorDefs.h"
#include "MuonReadoutGeometryR4/MuonReadoutElement.h"
///
#include "AthenaKernel/CLASS_DEF.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GeoModelKernel/GeoVDetectorManager.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include <map>
#include <memory>

/// The muon detector manager is the central class administrating the readout
/// elements of All muon subdetectors defined in the Geometry. The detector
/// elements are stored in a std::vector and their IdentifierHashes are used as
/// their corresponding position index. For each element type, e.g. CakeElement,
/// it provides one setter method and four getter methods.
///
///
///  Add the detector element to the manager. Fails if an element with the same
///  hash has already been added
///       StatusCode addCakeElement(std::unique_ptr<CakeElement> ele_ptr);
///
///  Return the (const) pointer to the detector element. The input Identifier is
///  the full ATLAS Identifier of the measurement
///       (const) CakeElement* getCakeElement(const Identifier& id) const;
///
///  Return the  (const) pointer to the detector element. The IdentifierHash has
///  to correspond to the hash of the readout element
///       (const) CakeElement* getCakeElement(const IdentifierHash& id) const;

/// Helper macros to declare the interface
#define DECLARE_GETTERSETTER(ELE_TYPE, GETTER, SETTER)        \
    ELE_TYPE* GETTER(const IdentifierHash& hash);             \
    ELE_TYPE* GETTER(const Identifier& hash);                 \
                                                              \
    const ELE_TYPE* GETTER(const IdentifierHash& hash) const; \
    const ELE_TYPE* GETTER(const Identifier& hash) const;     \
                                                              \
    StatusCode SETTER(ElementPtr_t<ELE_TYPE> element);

#define DECLARE_ELEMENT(ELE_TYPE) \
    DECLARE_GETTERSETTER(ELE_TYPE, get##ELE_TYPE, add##ELE_TYPE)   \
                                                                   \
    std::vector<const ELE_TYPE*> getAll##ELE_TYPE##s() const;      \
    std::vector<ELE_TYPE*>       getAll##ELE_TYPE##s();     
namespace MuonGMR4 {

class MdtReadoutElement;
class TgcReadoutElement;
class RpcReadoutElement;
class sTgcReadoutElement;
class MmReadoutElement;
class SpectrometerSector;
class Chamber;

class MuonDetectorManager : public GeoVDetectorManager, public AthMessaging {

   public:
    MuonDetectorManager();
    ~MuonDetectorManager();

    using ChIndex = Muon::MuonStationIndex::ChIndex;
    /** @brief: Abrivation of the smart pointer holding the readout element */
    template <class MuonDetectorType> using ElementPtr_t = std::unique_ptr<MuonDetectorType>;
    /** @brief: Abbrivation of the container holding all readout elements of a technology.
     *          The index of the vector entry corresponds to the IdentifierHash of the readout element. */
    template <class MuonDetectorType> using ElementStorage_t = std::vector<ElementPtr_t<MuonDetectorType>>;
    
    /** @brief Declaration of the readout element getters & setter function as 
     *         described above. */
    DECLARE_ELEMENT(MdtReadoutElement)
    DECLARE_ELEMENT(TgcReadoutElement)    
    DECLARE_ELEMENT(RpcReadoutElement)
    DECLARE_ELEMENT(sTgcReadoutElement)
    DECLARE_ELEMENT(MmReadoutElement)
    
    /** @brief Returns the number of tree top nodes describing the muon system */
    unsigned int getNumTreeTops() const override final;
    /** @brief Returns the i-the tree top GeoModel volume */ 
    PVConstLink getTreeTop(unsigned int i) const override final;
    /** @brief Adds a new GeoModelVolume with its children as a new top node of the muon system */
    void addTreeTop(PVConstLink pv);
    /// Returns a pointer to the central MuonIdHelperSvc
    const Muon::IMuonIdHelperSvc* idHelperSvc() const;
    
    /// Returns the list of all detector elements
    std::vector<const MuonReadoutElement*> getAllReadoutElements() const;
    std::vector<MuonReadoutElement*> getAllReadoutElements();
    /// Returns a generic Muon readout element
    const MuonReadoutElement* getReadoutElement(const Identifier& id) const;
    MuonReadoutElement* getReadoutElement(const Identifier& id);

#ifndef SIMULATIONBASE
    /** @brief Add a spectrometer enevelope object to the manager
     *  @param chSector: Unique_ptr to the sector */
    void addSpectrometerSector(ElementPtr_t<SpectrometerSector>&& chSector);
    /** @brief Retrieves the spectrometer envelope enclosing the channel's readout element
     *  @param channelId: Identifier of a muon channel of interest */
    const SpectrometerSector* getSectorEnvelope(const Identifier& channelId) const;
    /** @brief Retrieves the spectrometer envelope from a generic identifier as it's 
     *         used by e.g., the xAOD::MuonSegment.
     *  @param chIdx: Chamber index indicating where the envelope is residing (BIL, BIS, etc.)
     *  @param sector: Global sector of the envelope (1-16)
     *  @param side: Integer indicating whether, the envelope is in the positive or negative hemisphere */
    const SpectrometerSector* getSectorEnvelope(const Muon::MuonStationIndex::ChIndex chIdx,
                                                const unsigned sector,
                                                const int side) const;
    /** @brief Retrieves the chamber enclosing the channel's readout element
      *  @param channelId: Identifier of a muon channel of interest*/
    const Chamber* getChamber(const Identifier& channelId) const;
    
    /** Helper struct to ensure that the spectrometer sectors & chambers are sorted */
    struct MSEnvelopeSorter{
        bool operator()(const SpectrometerSector* a, const SpectrometerSector* b) const;
        bool operator()(const Chamber* a, const Chamber* b) const;
    };
    using MuonSectorSet = std::set<const SpectrometerSector*, MSEnvelopeSorter>;
    using MuonChamberSet = std::set<const Chamber*, MSEnvelopeSorter>;
    /// @brief: Returns all MuonChambers associated with the readout geometry
    MuonSectorSet getAllSectors() const;
    MuonChamberSet getAllChambers() const;
#endif

    /// Returns a list of all detector types
    std::vector<ActsTrk::DetectorType> getDetectorTypes() const;
   private:
    /** @brief Method that connects the same elements from the station with the parsed readout Element and
     *         vice versa. The way how they are inter-linked depends on the detector technology
     *         For the moment, only link Mdts from the same multilayer against each other.
     *  @param allStore: Storage of all detector element that are cached up to this point
     *  @param readOutEle: Particular readout element to link against the existing elements */
    template <class MuonDetectorType> void linkElements(ElementStorage_t<MuonDetectorType>& allStore,
                                                        MuonDetectorType* readOutEle);

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{"Muon::MuonIdHelperSvc/MuonIdHelperSvc", 
                                                        "MuonDetectorManager"};
#ifndef SIMULATIONBASE
    ElementStorage_t<SpectrometerSector> m_secEnvelopes{};
    /** @brief Abbrivation to find the sector envelopes sorted by the generic MS identifier */
    using EnvelopeMap_t = std::unordered_map<unsigned, const SpectrometerSector*>;
    EnvelopeMap_t m_envelopesById{};
#endif
    ElementStorage_t<MdtReadoutElement> m_mdtEles{};
    ElementStorage_t<TgcReadoutElement> m_tgcEles{};    
    ElementStorage_t<RpcReadoutElement> m_rpcEles{};
    ElementStorage_t<sTgcReadoutElement> m_sTgcEles{};
    ElementStorage_t<MmReadoutElement> m_mmEles{};

    std::vector<PVConstLink> m_treeTopVector{};


};

template <> void MuonDetectorManager::linkElements(ElementStorage_t<MdtReadoutElement>& detStore, MdtReadoutElement* refEle);

}  // namespace MuonGMR4

CLASS_DEF(MuonGMR4::MuonDetectorManager, 248531088, 1)
/// Delete the macro again
#undef DECLARE_GETTERSETTER
#undef DECLARE_ELEMENT
#include <MuonReadoutGeometryR4/MuonDetectorManager.icc>
#endif
