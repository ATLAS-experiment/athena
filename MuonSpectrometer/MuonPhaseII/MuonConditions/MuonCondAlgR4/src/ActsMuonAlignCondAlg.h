/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCONDALGR4_ACTSMUONALIGNCONDALG_H
#define MUONCONDALGR4_ACTSMUONALIGNCONDALG_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <StoreGate/CondHandleKeyArray.h>
#include <StoreGate/WriteCondHandle.h>

#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <ActsGeometryInterfaces/RawGeomAlignStore.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonAlignmentData/CorrContainer.h>

#include <MuonAlignmentDataR4/MdtAlignmentStore.h>
#include <MuonAlignmentDataR4/MmAlignmentStore.h>
#include <MuonAlignmentDataR4/sTgcAlignmentStore.h>

#include <map>
#include <set>
#include <unordered_map>
/* The ActsMuonAlignCondAlg takes the ALineContainer and translates this into the  
 *  a GeoAlignmentStore. The store is filled with the AlignableTransforms of the ReadoutGeometry
 *  which are connected with the A-line transformations of the ALineContainer.
 **/
class ActsMuonAlignCondAlg: public AthReentrantAlgorithm {
public:
      ActsMuonAlignCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
      virtual ~ActsMuonAlignCondAlg() = default;
      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext& ctx) const override;
      virtual bool isReEntrant() const override { return false; }

private:
    /// Returns the Identifier serving as key to find the alignment parameters connected with
    /// the readout element 
    Identifier alignmentId(const MuonGMR4::MuonReadoutElement* reElement) const;

    /// Association map of the GeoAlignableTransforms with the rigid alignment transformations
    using deltaMap = std::unordered_map<const GeoAlignableTransform*, std::shared_ptr<const Amg::Transform3D>>;
    /// Association map of the GeoAlignable transforms with the detector technologies.
    using alignTechMap = std::map<ActsTrk::DetectorType, std::set<const GeoAlignableTransform*>>;
    /// Loads the ALineContainer from the conditions store and fills the deltaMap with the
    /// A-Line delta transformations and the technology map to connect mutually moving detectors
    StatusCode loadDeltas(const EventContext& ctx,
                          deltaMap& alignDeltas,
                          alignTechMap& techTransforms) const;
    /// Loads the BLine container and the Mdt-as built parameters from the Conditions store
    /// and stores them into the tracking alignment object of the RawGeomAlignmentStore
    StatusCode loadMdtDeformPars(const EventContext& ctx,
                                 ActsTrk::RawGeomAlignStore& store) const;
    
    StatusCode loadMmDeformPars(const EventContext& ctx,
                                ActsTrk::RawGeomAlignStore& store) const;

    StatusCode loadStgcDeformPars(const EventContext& ctx,
                                  ActsTrk::RawGeomAlignStore& store) const;
                                
    /// Loads the corresponding ReadCondHandles from the Conditions store
    /// and adds their IOVs to the dependency of the writeHandle
    StatusCode declareDependencies(const EventContext& ctx,
                                   ActsTrk::DetectorType detType,
                                   SG::WriteCondHandle<ActsTrk::RawGeomAlignStore>& writeHandle) const;

    
    std::vector<ActsTrk::DetectorType> m_techs{};
    
    SG::ReadCondHandleKey<ALineContainer> m_readKeyALines{this, "ReadKeyALines", "ALineContainer",
                                                    "Key of the ALine container created from the DB"};

    SG::ReadCondHandleKey<BLineContainer> m_readKeyBLines{this, "ReadKeyBLines", "BLineContainer",
                                                    "Key of the BLine container created from the DB"};

    SG::ReadCondHandleKey<MdtAsBuiltContainer> m_readMdtAsBuiltKey{this, "ReadMdtAsBuiltKey", "MdtAsBuiltContainer", 
                                                    "Key of output muon alignment MDT/AsBuilt condition data"};
    SG::ReadCondHandleKey<NswAsBuiltDbData> m_readNswAsBuiltKey{this, "ReadNswAsBuiltKey", "NswAsBuiltDbData", 
                                                    "Key of NswAsBuiltDbData object containing conditions data for NSW as-built params!"};
    SG::ReadCondHandleKey<NswPassivationDbData> m_readNswPassivKey {this, "dMmPassivationKey", "NswPassivationDbData", 
                                                    "Key of NswPassivationDbData object containing passivation data for MMs"};

    SG::WriteCondHandleKeyArray<ActsTrk::RawGeomAlignStore> m_writeKeys{this, "WriteKeys", {},
                                                                        "Keys of the alignment technologies"};
    Gaudi::Property<std::string> m_keyToken{this, "CondKeyToken","ActsAlignContainer",
                                            "Common name token of all written alignment objects (e.g.) MdtActsAlignContainer"};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

    Gaudi::Property<bool> m_applyMmPassivation{this, "applyMmPassivation", false};
    
    Gaudi::Property<bool> m_applyNswAsBuilt{this, "applyNswAsBuilt", false, 
                                            "Toggles the application of the Nsw as-built parameters"};

    Gaudi::Property<bool> m_applyMdtAsBuilt{this, "applyMdtAsBuilt", false, 
                                            "Toggles the application of the Mdt as-built parameters"};
    /// Apply translations and rotations to align the Muon stations
    Gaudi::Property<bool> m_applyALines{this, "applyALines", true};
    /// Apply the chamber deformation model (Mdts + Nsw)
    Gaudi::Property<bool> m_applyBLines{this, "applyBLines", false};


};


#endif
