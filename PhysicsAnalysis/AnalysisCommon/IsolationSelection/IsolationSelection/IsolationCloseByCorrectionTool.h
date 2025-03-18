/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef IsolationSelection_IsolationCloseByCorrectionTool_H
#define IsolationSelection_IsolationCloseByCorrectionTool_H

#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgTools/AsgTool.h>
#include <AsgTools/PropertyWrapper.h>
#include <AsgTools/ToolHandle.h>
#include <CxxUtils/checker_macros.h>
#ifndef XAOD_ANALYSIS
#include <StoreGate/ReadDecorHandleKeyArray.h>
#include <StoreGate/WriteDecorHandleKeyArray.h>
#include "RecoToolInterfaces/IParticleCaloExtensionTool.h"
#endif

#include <InDetTrackSelectionTool/IInDetTrackSelectionTool.h>
#include <IsolationSelection/Defs.h>
#include <IsolationSelection/IIsolationCloseByCorrectionTool.h>
#include <IsolationSelection/IsoVariableHelper.h>
#include <IsolationSelection/IsolationWP.h>


#include "IsolationSelection/IIsolationSelectionTool.h"
#include "TrackVertexAssociationTool/ITrackVertexAssociationTool.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODPFlow/FlowElementContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"

namespace CP {
    class IsolationCloseByCorrectionTool : public asg::AsgTool, public virtual IIsolationCloseByCorrectionTool {
    public:
        enum TopoConeCorrectionModel {
            SubtractObjectsDirectly = -1,
            UseAveragedDecorators = 0,
            
        };

        using caloDecorNames = std::array<std::string, 4>;
        /// Returns an array with the calo cluster decoration ames [0]-> eta, [1]->phi, [2]->energy. [3]->isDecorated
        static caloDecorNames caloDecors();
        static caloDecorNames pflowDecors();

        using IsoHelperMap = std::map<IsoType, std::unique_ptr<IsoVariableHelper>>;

        IsolationCloseByCorrectionTool(const std::string& name);
        // Proper constructor for athena
        ASG_TOOL_CLASS(IsolationCloseByCorrectionTool, IIsolationCloseByCorrectionTool)

        virtual StatusCode initialize() override;

        virtual CorrectionCode getCloseByCorrection(std::vector<float>& corrections, const xAOD::IParticle& par,
                                                    const std::vector<xAOD::Iso::IsolationType>& types,
                                                    const xAOD::IParticleContainer& closePar) const override;

        virtual asg::AcceptData acceptCorrected(const xAOD::IParticle& x, const xAOD::IParticleContainer& closePar) const override;

        virtual CorrectionCode getCloseByIsoCorrection (
                                                        const EventContext& ctx,
                                                        const xAOD::ElectronContainer* Electrons, 
                                                        const xAOD::MuonContainer* Muons,
                                                        const xAOD::PhotonContainer* Photons) const override;

        virtual float getOriginalIsolation(const xAOD::IParticle& P, IsoType type) const override;
        virtual float getOriginalIsolation(const xAOD::IParticle* particle, IsoType type) const override;

        /// Load all TrackParticles associated with the particles in the Container. The particles have to pass the selection decoration flag
        TrackSet getTrackCandidates(const EventContext& ctx, const xAOD::IParticle* particle) const override;

        const xAOD::IParticle* isoRefParticle(const xAOD::IParticle* particle) const override;
      
        void associateCluster(const xAOD::IParticle* particle, float& eta, float& phi, float& energy) const override;

        void associateFlowElement(const EventContext& ctx, const xAOD::IParticle* particle, float& eta, float& phi,
                                  float& energy) const override;

        /// Helper struct to collect all relevant objects for the procedure
        using PrimaryCollection = std::set<const xAOD::IParticle*>;
        struct ObjectCache {
            ObjectCache() = default;
            const xAOD::Vertex* prim_vtx{nullptr};
            PrimaryCollection prim_parts{};
            PrimaryCollection not_sel_parts{};
            TrackSet tracks{};
            ClusterSet clusters{};
            PflowSet flows{};
            UnorderedClusterSet eg_associated_clusters{};
        };

    private:
        /// Helper function to load all Isolation types from the iso working points
        void isoTypesFromWP(const std::vector<std::unique_ptr<IsolationWP>>& WP, IsoVector& types);
        /// Retrieve all Inner detector tracks associated with the primary particle
        TrackSet getAssociatedTracks(const xAOD::IParticle* P) const;
        /// Retrieve the subset of tracks passing the isolation selection
        TrackSet getAssociatedTracks(const xAOD::IParticle* P, const xAOD::Vertex* vtx) const;
        /// Retrieve all Flow elements associated with the particles in the cache
        void getAssocFlowElements(const EventContext& ctx, ObjectCache& cache) const;

        // Function to pipe each container given by the interfaces through. It loops over all
        // particles and removes the isolation overlap between the objects
        CorrectionCode performCloseByCorrection (const EventContext& ctx, ObjectCache& cache) const;

        // Lock decorations that we produced.
        void lockDecorations (const xAOD::IParticleContainer* parts) const;

        // Helper function to obtain the isolation cones to use for a given particle
        const IsoVector& getIsolationTypes(const xAOD::IParticle* particle) const;

        // Functions to  perfrom  the isolation correction  directly
        CorrectionCode subtractCloseByContribution(const EventContext& ctx, const xAOD::IParticle* P, ObjectCache& cache) const;
        // Remove close-by tracks from the track isolation variables
        CorrectionCode getCloseByCorrectionTrackIso(const xAOD::IParticle* primary, const IsoType type, const ObjectCache& cache,
                                                    float& isoValue) const;
        // Remove close-by calo clusters from the topo et isolation variables
        CorrectionCode getCloseByCorrectionTopoIso(const EventContext& ctx, const xAOD::IParticle* primary, const IsoType type,
                                                   ObjectCache& cache, float& isoValue) const;
        // Remove close-by flow elements from the neflow isolation variables
        CorrectionCode getCloseByCorrectionPflowIso(const EventContext& ctx, const xAOD::IParticle* primary, const IsoType type,
                                                    const ObjectCache& cache, float& isoValue) const;
        // For primary particles not selected, copy isolation value to output decorator 
        CorrectionCode copyIsoValuesForPartsNotSelected(const xAOD::IParticle* part) const;


         /// Loads the topo clusters associated with the primary IParticle
        ClusterSet getAssociatedClusters(const EventContext& ctx, const xAOD::IParticle* particle,
                                         ObjectCache& cache) const;
        /// Loads the pflow elements associated with the primary IParticle
        PflowSet getAssocFlowElements(const EventContext& ctx, const xAOD::IParticle* particle) const;

#ifndef XAOD_ANALYSIS
        /// helper to get eta,phi of muon extrap
        bool getExtrapEtaPhi(const EventContext& ctx, const xAOD::TrackParticle* tp, float& eta, float& phi) const;
#endif

        // Returns the Size of the Isolation cone
        float coneSize(const xAOD::IParticle* particle, IsoType Cone) const;
        // Retrieves the uncalibrated pt from the particle
        float unCalibPt(const xAOD::IParticle* particle) const;

        // Clusters and tracks of particles surviving the selection quality
        // are considered for corrections
        bool passSelectionQuality(const xAOD::IParticle* particle) const;

        /// The Track particle has to pass the Track selection tool and the TTVA selection.
        /// The latter only applies if there's no WP floating around using the legacy ptcone variables
        bool passFirstStage(const xAOD::TrackParticle* trk, const xAOD::Vertex* vtx) const;

        void printIsolationCones(const IsoVector& types, xAOD::Type::ObjectType T) const;

    public:
        // Extrapolated phi eta needed for proper dR of the muons
        void getExtrapEtaPhi(const xAOD::IParticle* particlear, float& eta, float& phi) const;
        /// Filter all electrons/muons/photons from the collection which pass the selection decoration
        void loadPrimaryParticles(const xAOD::IParticleContainer* container, ObjectCache& cache) const;
        /// Load all associated tracks / clusters / flow elements into the cache
        void loadAssociatedObjects(const EventContext& ctx, ObjectCache& cache) const;
 
        // Some helper functions for Overlap and DeltaR
        bool isSame(const xAOD::IParticle* particle, const xAOD::IParticle* particle1) const;
        bool overlap(const xAOD::IParticle* particle, const xAOD::IParticle* particle1, float dR) const;
        float deltaR2(const xAOD::IParticle* particle, const xAOD::IParticle* particle1, bool AvgCalo = false) const;

        // Fixed cone size isolation variables
        static bool isFixedTrackIso(xAOD::Iso::IsolationType type);
        // Any trackisolation variable with variable con size
        static bool isVarTrackIso(xAOD::Iso::IsolationType type);

        static bool isFixedTrackIsoTTVA(xAOD::Iso::IsolationType type);
        // PtVarcones of the pile-up robust isolation variables
        static bool isVarTrackIsoTTVA(xAOD::Iso::IsolationType Iso);
        //  Any track isolation variable
        static bool isTrackIso(xAOD::Iso::IsolationType type);
        // Pileup robust track isolation variables
        static bool isTrackIsoTTVA(xAOD::Iso::IsolationType type);
        // The pile-up robust isolation cones only accept
        // tracks with a minimum pt requirement
        static float trackPtCut(xAOD::Iso::IsolationType type);

        static bool isTopoEtIso(xAOD::Iso::IsolationType type);

        static bool isPFlowIso(xAOD::Iso::IsolationType type);

        static bool isEgamma(const xAOD::IParticle* particle);

        const xAOD::Vertex* retrieveIDBestPrimaryVertex(const EventContext& ctx) const;

       

        static float clusterEtMinusTile(const xAOD::CaloCluster* C);

        static std::string particleName(const xAOD::IParticle* C);
        static std::string particleName(xAOD::Type::ObjectType T);

    private:
        // IMPORTANT USER PROPERTIES
        ToolHandle<InDet::IInDetTrackSelectionTool> m_trkselTool{
            this, "TrackSelectionTool", "", "TrackSelectionTool to select tracks which made it actually into the isolation"};
        ToolHandle<CP::ITrackVertexAssociationTool> m_ttvaTool{this, "TTVASelectionTool", "",
                                                               "TTVASelectionTool to correct for the pile-up robust WPs"};
        ToolHandle<CP::IIsolationSelectionTool> m_selectorTool{this, "IsolationSelectionTool", "",
                                                               "Please give me your configured IsolationSelectionTool!"};

        // OPTIONAL PROPERTIES
        // Name of the isolation selection and input quality decorators
        Gaudi::Property<std::string> m_quality_name{
            this, "SelectionDecorator", "",
            "Name of the char auxdata defining whether the particle shall be considered for iso correction"};
        Gaudi::Property<std::string> m_passOR_name{this, "PassoverlapDecorator", "",
                                                   "Does the particle also need to pass the overlap removal?"};
        Gaudi::Property<std::string> m_isoSelection_name{this, "IsolationSelectionDecorator", "", "Name of the final isolation decorator."};

        Gaudi::Property<std::string> m_backup_prefix{
            this, "BackupPrefix", "", "Prefix in front of the isolation variables, if the original cone values need  to  be backuped"};

        Gaudi::Property<std::string> m_isoDecSuffix{
            this, "IsoDecSuffix", "", "Suffix added to output isolation variable nanes for close by corrections"};

        /// EXPERT PROPERTIES
        Gaudi::Property<int> m_caloModel{this, "CaloCorrectionModel", TopoConeCorrectionModel::SubtractObjectsDirectly};
        // The core of the topoEt variables. Clusters within the core shall not be
        // added to the isolation of the object itself. They are defined to be associated with it.
        Gaudi::Property<float> m_coreConeEl{this, "CoreConeElectrons", 0.1,
                                            "This is the size of the core cone for the topoetcone variables."};
        
        //  Muons have half of the cone-size compared to electrons
        // (c.f. https://gitlab.cern.ch/atlas/athena/blob/21.2/Reconstruction/RecoTools/IsolationTool/Root/CaloIsolationTool.cxx#L82)
        Gaudi::Property<float> m_coreConeMu{this, "CoreConeMuons", 0.05, "This is the size of the core cone for the topoetcone variables."};

        // Reference value to calculate the size of the mini-iso variables
        //  dR = min (fixed , m_ptvarcone / particle->pt())
        Gaudi::Property<float> m_ptvarconeRadius{this, "PtvarconeRadius", 1.e4, "This is the kT parameter for the ptvarcone variables."};

        // Upper limit on the energy fraction of the close-by cluster
        // to the isolation variable such that it is still subtracted from the cone.
        // Only considered if the particle to correct is not an Egamma object. Since the reference
        // position is extrapolated from the ID-track into the calorimeter.
        // Aim of the game -> Find out whether it might contributed
        Gaudi::Property<float> m_maxTopoPolution{
            this, "MaxClusterFrac", 1.1,
            "Maximum energy fraction a single cluster can make up to be considered as contributed to the isolation"};

        Gaudi::Property<float> m_ConeSizeVariation{
            this, "ExtrapolationConeSize", 1.2,
            "Constant factor to be multiplied on top of the topo-etcone size if the reference particle is not a calorimeter particle in "
            "order to account for extrapolation effects"};  // Extend - shrink the cone size to account for extrapolation effects

        Gaudi::Property<bool> m_declareCaloDecors{this, "declareCaloDecors", false, "If set to true, the data dependency on the calo/pflow decors will be declared"};
        /// Declare the data dependencies of the Input containers. 
        /// The isolation variables used in the working point calculation are declared to the avalanche scheduler. 
        /// This is not needed for the AthAnalysis nor AnalysisBase releases.
#ifndef XAOD_ANALYSIS
        Gaudi::Property<std::vector<std::string>> m_elecKeys{
            this, "EleContainers", {}, "Pipe the list of electron containers given later to the tool"};
        Gaudi::Property<std::vector<std::string>> m_muonKeys{
            this, "MuoContainers", {}, "Pipe the list of muon containers given later to the tool"};
        Gaudi::Property<std::vector<std::string>> m_photKeys{
            this, "PhoContainers", {}, "Pipe the list of photon containers given later to the tool"};
        SG::ReadDecorHandleKeyArray<xAOD::IParticleContainer> m_isoVarKeys{
            this, "IsoVarKeys", {}, "The list is filled during the initialization"};
        SG::WriteDecorHandleKeyArray<xAOD::IParticleContainer> m_isoWriteDecVarKeys{
            this, "IsoWriteDecVarKeys", {}, "The list is filled during the initialization"};

        /// calo extension tool for muon track particle extrapolation to calo
        ToolHandle<Trk::IParticleCaloExtensionTool> m_caloExtTool {this, "ParticleCaloExtensionTool", "Trk::ParticleCaloExtensionTool/ParticleCaloExtensionTool"};

        /// Helper function to declare the data dependencies
        void declareDependency(const std::vector<std::string>& containers, const IsoVector& types);
#endif

        SG::ReadHandleKey<xAOD::VertexContainer> m_VtxKey{this, "VertexContainer", "PrimaryVertices",
                                                          "Name of the primary vertex container"};
        SG::ReadHandleKey<xAOD::CaloClusterContainer> m_CaloClusterKey{this, "CaloClusterContainer", "CaloCalTopoClusters",
                                                                       "Name of the primary calo cluster container"};
        SG::ReadHandleKey<xAOD::FlowElementContainer> m_PflowKey{this, "PflowContainer", "CHSNeutralParticleFlowObjects",
                                                                 "Name of the neutral pflow elements"};
        /// Isolation variables used by the muon working point
        IsoVector m_muon_isoTypes{};
        /// Isolation variables used by the electron working point
        IsoVector m_electron_isoTypes{};
        /// Isolation variables used by the photon working point
        IsoVector m_photon_isoTypes{};
        /// Switch whether a pile-up non robust TTVA working point is defined
        bool m_has_nonTTVA{false};
        /// Switch whether a pflow isolation working point is defined
        bool m_hasPflowIso{false};
        /// Switch whether a topoetcone isolation working point is defined
        bool m_hasEtConeIso{false};
        

        bool m_isInitialised{false};

        SelectionAccessor m_acc_quality{nullptr};
        SelectionAccessor m_acc_passOR{nullptr};
        SelectionDecorator m_dec_isoselection{nullptr};

        // Functionallity to backup the original cone variables if needed
        mutable IsoHelperMap m_isohelpers ATLAS_THREAD_SAFE;
        /// Mutex to protect the map if the method with signature
        ///   getCloseByCorrection(std::vector<float>& corrections, const xAOD::IParticle& par, const std::vector<xAOD::Iso::IsolationType>&
        ///   types, const xAOD::IParticleContainer& closePar)
        /// is called
        mutable std::mutex m_isoHelpersMutex ATLAS_THREAD_SAFE;
    };

}  // namespace CP
#endif
