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
    StatusCode SETTER(ElementPtr<ELE_TYPE> element);

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
class MuonChamber;

class MuonDetectorManager : public GeoVDetectorManager, public AthMessaging {

   public:
    MuonDetectorManager();
    ~MuonDetectorManager() = default;

    template <class MuonDetectorType>
    using ElementPtr = std::unique_ptr<MuonDetectorType>;
    template <class MuonDetectorType>
    using ElementStorage = std::vector<ElementPtr<MuonDetectorType>>;
    
    /// Access specifically the individual readout element technologies
    DECLARE_ELEMENT(MdtReadoutElement)
    DECLARE_ELEMENT(TgcReadoutElement)    
    DECLARE_ELEMENT(RpcReadoutElement)
    DECLARE_ELEMENT(sTgcReadoutElement)
    DECLARE_ELEMENT(MmReadoutElement)

    /// Returns the number of primary nodes in the GeoModel tree
    /// that are building the full MuonSystem (MuonBarrel, MuonEndCap, NSW etc)
    unsigned int getNumTreeTops() const override final;
    /// Returns the i-th top node of the MuonSystem trees 
    PVConstLink getTreeTop(unsigned int i) const override final;
    /// Adds a new GeoModelTree node indicating the entrance to a muon system description
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
    const MuonChamber* getChamber(const Identifier& channelId) const;
    /// Helper struct to ensur that the sorting of the MuonChambers
    /// remains constants across the jobs
    struct ChamberSorter{
        bool operator()(const MuonChamber* a, const MuonChamber* b) const;
    };
    using MuonChamberSet = std::set<const MuonChamber*, ChamberSorter>;
    /// @brief: Returns all MuonChambers associated with the readout geometry
    MuonChamberSet getAllChambers() const;
#endif

    /// Returns a list of all detector types
    std::vector<ActsTrk::DetectorType> getDetectorTypes() const;
   private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        "Muon::MuonIdHelperSvc/MuonIdHelperSvc", "MuonDetectorManager"};

    ElementStorage<MdtReadoutElement> m_mdtEles{};
    ElementStorage<TgcReadoutElement> m_tgcEles{};    
    ElementStorage<RpcReadoutElement> m_rpcEles{};
    ElementStorage<sTgcReadoutElement> m_sTgcEles{};
    ElementStorage<MmReadoutElement> m_mmEles{};

    std::vector<PVConstLink> m_treeTopVector{};
};

}  // namespace MuonGMR4

CLASS_DEF(MuonGMR4::MuonDetectorManager, 248531088, 1)
/// Delete the macro again
#undef DECLARE_GETTERSETTER
#undef DECLARE_ELEMENT
#include <MuonReadoutGeometryR4/MuonDetectorManager.icc>
#endif
