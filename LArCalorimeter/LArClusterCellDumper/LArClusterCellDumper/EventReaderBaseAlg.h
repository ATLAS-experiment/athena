// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef LARCLUSTERCELLDUMPER_EVENTREADERBASEALG_H
#define LARCLUSTERCELLDUMPER_EVENTREADERBASEALG_H

#include "AthenaBaseComps/AthAlgorithm.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODEgamma/Electron.h"
#include "xAODTracking/VertexContainer.h"
#include "CxxUtils/checker_macros.h"
class TTree;
class CaloCell;





class ATLAS_NOT_THREAD_SAFE EventReaderBaseAlg: public ::AthAlgorithm
{
    public:
     
    
        EventReaderBaseAlg( const std::string& name, ISvcLocator* pSvcLocator );
        
        virtual ~EventReaderBaseAlg() override;

        virtual StatusCode        initialize()  override;
        virtual StatusCode        execute()     override;
        virtual StatusCode        finalize()    override;
    
    private:
        
    protected:
        bool                      isEtaOutsideLArCrack(float absEta);
        bool                      isTagElectron(const xAOD::Electron* electron);
        bool                      isGoodProbeElectron(const xAOD::Electron* el);
        bool                      trackSelectionElectrons(const xAOD::Electron *electron, SG::ReadHandle<xAOD::VertexContainer> &primVertexCnt, SG::ReadHandle<xAOD::EventInfo> &ei); // for Xtalk studies
        bool                      eOverPElectron(const xAOD::Electron* electron);
        int                       getCaloRegionIndex(const CaloCell* cell); //customized representative indexes of calo region
        double                    fixPhi(double phi);
        double                    deltaPhi(double phi1 , double phi2);
        double                    deltaR( double eta, double phi);

        void                      clear();
        void                      clearLBData();
        void                      bookBranches(TTree *tree);
        void                      bookDatabaseBranches(TTree *tree);


        Gaudi::Property<float> m_elecEtaCut {this, "electronEtaCut", 1.4, "Electron |eta| cut value."};
        Gaudi::Property<std::string> m_offTagTightness {this, "offTagTightness", "LHMedium"}; /*! Define the PID for tag electron */
        Gaudi::Property<std::string> m_offProbeTightness {this, "offProbeTightness", "Loose"}; /*! define the Pid of Probe from the user */
        Gaudi::Property<float> m_etMinProbe {this, "etMinProbe", 15 ,"Min electron Pt value for Zee probe selection loose (GeV)."}; // Et or pT ?
        Gaudi::Property<float> m_etMinTag {this, "etMinTag", 15 ,"Min Et value for the electrons in Zee tag selection (GeV)."}; // Et?
        Gaudi::Property<float> m_d0TagSig {this, "d0TagSig", 5, "d_0 transverse impact parameter significance."};
        Gaudi::Property<float> m_z0Tag {this, "z0Tag", 0.5, "z0 longitudinal impact parameter (mm)"};// https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/TrackingCPEOYE2015
        Gaudi::Property<bool> m_doElecSelectByTrackOnly {this, "doElecSelectByTrackOnly", false, "Perform electron selection by track only, witout tag and probe."};
        Gaudi::Property<bool> m_doTagAndProbe {this, "doTagAndProbe", true, "First, the electrons are selected by track criteria, then, perform Tag and Probe selection for Zee."};
        Gaudi::Property<bool> m_doPhotonDump {this, "doPhotonDump", false, "Perform a photon particle dump based on offline Photons Container."};
        Gaudi::Property<bool> m_doTruthEventDump {this, "doTruthEventDump", false, "Dump the Truth Event variables."};
        Gaudi::Property<bool> m_doTruthPartDump {this, "doTruthPartDump", false, "Perform a truth particle dump."};
        Gaudi::Property<bool> m_doLArEMBHitsDump {this, "doLArEMBHitsDump", false, "Dump the EMB HITs container energy and time."};
        Gaudi::Property<bool> m_isMC {this, "isMC", false, "Switch the dumper to MC sample mode."};
        Gaudi::Property<bool> m_getLArCalibConstants {this, "getLArCalibConstants", false, "Get the LAr calorimeter calibration constants, related to cells energy and time (online and offline)."};
        Gaudi::Property<bool> m_noBadCells {this, "noBadCells", true, "If True, skip the cells tagged as badCells/channels."};
        Gaudi::Property<bool> m_doAssocTopoCluster711Dump {this, "doAssocTopoCluster711Dump", false, "Dump the 7x11 EMB2 window cells."};

        //#############################
        // Global Variables in general
        //#############################

        TTree *m_Tree{};
        TTree *m_secondTree{};

        //##################
        // Dumped Variables
        //##################

        // ## Lumiblock variables
        std::vector < std::vector < float > >   *m_lb_bcidLuminosity = nullptr;
        std::vector < int >                     *m_lb_lumiblock = nullptr;

        // ## EventInfo ##
        unsigned int                            m_e_runNumber         = 9999; //< Run number
        unsigned int                            m_e_bcid              = 9999; //< BCID number
        unsigned long long                      m_e_eventNumber       = 9999; //< Event number
        int                                     m_e_lumiBlock         = 999; // lumiblock number
        float                                   m_e_inTimePileup      = -999; //< avg_mu
        float                                   m_e_outOfTimePileUp   = -999; // OOT pileup

        // ## Cluster (TOPOCLUSTER) ##
        int m_c_clusterIndexCounter = 0; // cluster index counter, for the entire event
        std::vector < int > *m_c_clusterIndex = nullptr; // cluster index, for each cluster in the same event. (they have the same number of inputs)
        std::vector < int > *m_c_electronIndex_clusterLvl = nullptr; // electron index, for each cluster in the same event.
        std::vector < double > *m_c_clusterEnergy = nullptr; //energy of the cluster
        std::vector < double > *m_c_clusterTime = nullptr; //timing of the cluster
        std::vector < double > *m_c_clusterEta = nullptr; // clus. baricenter eta
        std::vector < double > *m_c_clusterPhi = nullptr; // clus. baricenter phi
        std::vector < double > *m_c_clusterPt = nullptr; // clus. baricenter eta
        // Cell
        int                                   m_c_cellIndexCounter = 0; // cell index counter inside a cluster, for the entire event
        std::vector < int >                   *m_c_clusterIndex_cellLvl = nullptr; // cluster index, for each cell index. (they have the same number of inputs)
        std::vector < int >                   *m_c_clusterCellIndex = nullptr; // cell index inside a cluster
        std::vector < int >                   *m_c_cellGain = nullptr; // gain of cell signal, from 'CaloGain' standard.
        std::vector < int >                   *m_c_cellLayer = nullptr; // layer index of cell signal, from caloDDE.
        std::vector < int >                   *m_c_cellRegion = nullptr; // region of calorimeter system (custom index from 'getCaloRegionIndex')
        std::vector < double >                *m_c_cellEnergy = nullptr; // cell inside cluster energy
        std::vector < double >                *m_c_cellTime = nullptr; // cell inside cluster energy
        std::vector < double >                *m_c_cellEta = nullptr; // cell inside cluster baricenter eta
        std::vector < double >                *m_c_cellPhi = nullptr; // cell inside cluster baricenter phi
        std::vector < double >                *m_c_cellDEta = nullptr; // cell inside cluster delta_eta (granularity)
        std::vector < double >                *m_c_cellDPhi = nullptr; // cell inside cluster delta_phi (granularity)
        std::vector < double >                *m_c_cellToClusterDPhi = nullptr; // cell inside cluster delta_phi distance to cluster baricenter.
        std::vector < double >                *m_c_cellToClusterDEta = nullptr; // cell inside cluster delta_eta distance to cluster baricenter.
        //**** Channel
        std::vector < int >                     *m_c_clusterIndex_chLvl = nullptr; // cluster index, for each channel index. (they have the same number of inputs)
        std::vector < int >                     *m_c_clusterChannelIndex = nullptr; // index for a sequence of channels. It is a float for identify PMTs: +0.0 LAr, +0.1 Tile PMT1, +0.2 Tile PMT2
        std::vector < std::vector < int > >     *m_c_channelChInfo = nullptr; // from both LAr (barrel_ec, pos_neg, feedthrough, slot, channel) and Tile (ros, drawer, channel, adc)
        std::vector < std::vector < float > >   *m_c_channelDigits = nullptr;  // samples from LAr and Tile cells/channels
        std::vector < double >                  *m_c_channelEnergy = nullptr; // energy of cell or readout channel inside cluster (MeV)
        std::vector < double >                  *m_c_channelTime = nullptr; // time of channel inside cluster
        std::vector < int >                     *m_c_channelLayer = nullptr; // layer index of channel signal, from caloDDE.
        std::vector < bool >                    *m_c_channelBad = nullptr; // channel linked to a cluster, whitch is tagged as bad. (1 - bad, 0 - not bad)
        std::vector <  unsigned int  >          *m_c_channelHashMap = nullptr; //cell map of ALL cells inside a cluster. id 0x2d214a140000000. is a 64-bit number that represent the cell.
        std::vector <  unsigned int  >          *m_c_channelChannelIdMap = nullptr;
        std::vector < float >                   *m_c_channelEffectiveSigma  = nullptr;
        std::vector < float >                   *m_c_channelNoise  = nullptr;
        std::vector < float >                   *m_c_channelDSPThreshold  = nullptr; // get from Athena POOL Utilities
        std::vector < std::vector < double > >  *m_c_channelOFCa = nullptr;
        std::vector < std::vector < double > >  *m_c_channelOFCb = nullptr;
        std::vector < std::vector < double > >  *m_c_channelShape = nullptr; // shape
        std::vector < std::vector < double > >  *m_c_channelShapeDer = nullptr; // shape derivative
        std::vector < double >                  *m_c_channelOFCTimeOffset = nullptr; 
        std::vector < float >                   *m_c_channelADC2MEV0 = nullptr;  //ramp0
        std::vector < float >                   *m_c_channelADC2MEV1 = nullptr; //ramp1
        std::vector < float >                   *m_c_channelPed = nullptr; //pedestal from DB (LAr / maskedTile)
        std::vector < float >                   *m_c_channelMinBiasAvg = nullptr; //cell minBias average
        std::vector < float >                   *m_c_channelOfflHVScale = nullptr;
        std::vector < float >                   *m_c_channelOfflEneRescaler = nullptr;
        
        // Raw channel
        std::vector < std::vector < int > > *m_c_rawChannelChInfo = nullptr; // from both LAr (barrel_ec, pos_neg, feedthrough, slot, channel, provenance) and Tile (ros, drawer, channel, adc, pedestal)
        std::vector <  unsigned int  >      *m_c_rawChannelIdMap = nullptr;
        std::vector < float >               *m_c_rawChannelAmplitude = nullptr; // raw channel energy (adc)
        std::vector < float >               *m_c_rawChannelTime = nullptr; // ps
        std::vector < int >                 *m_c_rawChannelLayer = nullptr; // layer index of cell signal, from caloDDE.
        std::vector < float >               *m_c_rawChannelPed = nullptr; // raw channel estimated pedestal (Tile) or LAr provenance
        std::vector < float >               *m_c_rawChannelProv = nullptr; // raw channel estimated pedestal (Tile) or LAr provenance
        std::vector < float >               *m_c_rawChannelQuality = nullptr; // raw channel quality
        std::vector < float >               *m_c_clusterRawChannelIndex = nullptr; // raw channel index
        std::vector < int >                 *m_c_clusterIndex_rawChLvl = nullptr; // cluster index at raw channel level
        std::vector < float >               *m_c_rawChannelDSPThreshold  = nullptr; // get from Athena POOL Utilities

        // LAr HITs
        size_t                              m_ncell   = 0;
        const double                        m_minEnergy = 1e-9;
        std::vector   < int >               *m_hits_sampling = nullptr;
        std::vector   < int >               *m_hits_clusterIndex_chLvl = nullptr;
        std::vector   < int >               *m_hits_clusterChannelIndex = nullptr;
        std::vector   < unsigned int >      *m_hits_hash = nullptr;
        std::vector   < double >            *m_hits_energy  = nullptr;
        std::vector   < double >            *m_hits_time    = nullptr;
        std::vector   < float >             *m_hits_sampFrac = nullptr; // sampling fraction from MC/data E_calibration
        std::vector   < double >            *m_hits_energyConv  = nullptr;
        std::vector   < double >            *m_hits_cellEta  = nullptr;
        std::vector   < double >            *m_hits_cellPhi  = nullptr;


        // ## Cluster (EMB2 EGAMMA CALO CLUSTER 7_11) ##
        int                     *m_c711_clusterIndexCounter = 0; // cluster index counter, for the entire event
        std::vector < int >     *m_c711_clusterIndex = nullptr; // cluster index, for each cluster in the same event. (they have the same number of inputs)
        std::vector < int >     *m_c711_electronIndex_clusterLvl = nullptr; // electron index, for each cluster in the same event.
        std::vector < double >  *m_c711_clusterEnergy = nullptr; //energy of the cluster
        std::vector < double >  *m_c711_clusterTime = nullptr; //timing of the cluster
        std::vector < double >  *m_c711_clusterEta = nullptr; // clus. baricenter eta
        std::vector < double >  *m_c711_clusterPhi = nullptr; // clus. baricenter phi
        std::vector < double >  *m_c711_clusterPt = nullptr; // clus. baricenter eta

        // Cell
        int                     *m_c711_cellIndexCounter = nullptr; // cell index counter inside a cluster, for the entire event
        std::vector < int >     *m_c711_clusterIndex_cellLvl = nullptr; // cluster index, for each cell index. (they have the same number of inputs)
        std::vector < int >     *m_c711_clusterCellIndex = nullptr; // cell index inside a cluster
        std::vector < int >     *m_c711_cellGain = nullptr; // gain of cell signal, from 'CaloGain' standard.
        std::vector < int >     *m_c711_cellLayer = nullptr; // layer index of cell signal, from caloDDE.
        std::vector < int >     *m_c711_cellRegion = nullptr; // region of calorimeter system (custom index from 'getCaloRegionIndex')
        std::vector < double >  *m_c711_cellEnergy = nullptr; // cell inside cluster energy
        std::vector < double >  *m_c711_cellTime = nullptr; // cell inside cluster energy
        std::vector < double >  *m_c711_cellEta = nullptr; // cell inside cluster baricenter eta
        std::vector < double >  *m_c711_cellPhi = nullptr; // cell inside cluster baricenter phi
        std::vector < double >  *m_c711_cellDEta = nullptr; // cell inside cluster delta_eta (granularity)
        std::vector < double >  *m_c711_cellDPhi = nullptr; // cell inside cluster delta_phi (granularity)
        std::vector < double >  *m_c711_cellToClusterDPhi = nullptr; // cell inside cluster delta_phi distance to cluster baricenter.
        std::vector < double >  *m_c711_cellToClusterDEta = nullptr; // cell inside cluster delta_eta distance to cluster baricenter.
        // Channel
        std::vector < int >                     *m_c711_clusterIndex_chLvl = nullptr; // cluster index, for each channel index. (they have the same number of inputs)
        std::vector < int >                     *m_c711_clusterChannelIndex = nullptr; // index for a sequence of channels. It is a float for identify PMTs: +0.0 LAr, +0.1 Tile PMT1, +0.2 Tile PMT2
        std::vector < std::vector < int > >     *m_c711_channelChInfo = nullptr; // from both LAr (barrel_ec, pos_neg, feedthrough, slot, channel) and Tile (ros, drawer, channel, adc)
        std::vector < std::vector < float > >   *m_c711_channelDigits = nullptr;  // samples from LAr and Tile cells/channels
        std::vector < double >                  *m_c711_channelEnergy = nullptr; // energy of cell or readout channel inside cluster (MeV)
        std::vector < double >                  *m_c711_channelTime = nullptr; // time of channel inside cluster
        std::vector < int >                     *m_c711_channelLayer = nullptr; // layer index of cell signal, from caloDDE.
        std::vector < bool >                    *m_c711_channelBad = nullptr; // channel linked to a cluster, whitch is tagged as bad. (1 - bad, 0 - not bad)
        std::vector <  unsigned int  >          *m_c711_channelHashMap = nullptr; //cell map of ALL cells inside a cluster. id 0x2d214a140000000. is a 64-bit number that represent the cell.
        std::vector <  unsigned int  >          *m_c711_channelChannelIdMap = nullptr;
        std::vector < float >                   *m_c711_channelEffectiveSigma  = nullptr;
        std::vector < float >                   *m_c711_channelNoise  = nullptr;
        std::vector < float >                   *m_c711_channelDSPThreshold  = nullptr; // get from Athena POOL Utilities
        std::vector < std::vector < double > >  *m_c711_channelOFCa = nullptr; 
        std::vector < std::vector < double > >  *m_c711_channelOFCb = nullptr; 
        std::vector < std::vector < double > >  *m_c711_channelShape = nullptr; // shape
        std::vector < std::vector < double > >  *m_c711_channelShapeDer = nullptr; // shape derivative
        std::vector < double >                  *m_c711_channelOFCTimeOffset = nullptr; 
        std::vector < float >                   *m_c711_channelADC2MEV0 = nullptr;
        std::vector < float >                   *m_c711_channelADC2MEV1 = nullptr;
        std::vector < float >                   *m_c711_channelPed = nullptr; //pedestal from DB (LAr / maskedTile)
        std::vector < float >                   *m_c711_channelMinBiasAvg = nullptr;
        std::vector < float >                   *m_c711_channelOfflHVScale = nullptr;
        std::vector < float >                   *m_c711_channelOfflEneRescaler = nullptr;
        // Raw channel
        std::vector < std::vector < int > > *m_c711_rawChannelChInfo = nullptr; // from both LAr (barrel_ec, pos_neg, feedthrough, slot, channel, provenance) and Tile (ros, drawer, channel, adc, pedestal)
        std::vector <  unsigned int  >      *m_c711_rawChannelIdMap = nullptr;
        std::vector < float >               *m_c711_rawChannelAmplitude = nullptr; // raw channel energy (adc)
        std::vector < float >               *m_c711_rawChannelTime = nullptr; // raw channel time
        std::vector < int >                 *m_c711_rawChannelLayer = nullptr; // layer index of cell signal, from caloDDE.
        std::vector < float >               *m_c711_rawChannelPed = nullptr; // raw channel estimated pedestal 
        std::vector < float >               *m_c711_rawChannelProv = nullptr; // raw channel LAr provenance (tile masked)
        std::vector < float >               *m_c711_rawChannelQuality = nullptr; // raw channel quality
        std::vector < float >               *m_c711_clusterRawChannelIndex = nullptr; // raw channel index
        std::vector < int >                 *m_c711_clusterIndex_rawChLvl = nullptr; // cluster index at raw channel level
        std::vector < float >               *m_c711_rawChannelDSPThreshold  = nullptr; // get from  Athena POOL utilities
        
        // ## Particle Truth ## (MC)
        std::vector < float > *m_mc_part_energy   = nullptr;
        std::vector < float > *m_mc_part_pt       = nullptr;
        std::vector < float > *m_mc_part_m        = nullptr;
        std::vector < float > *m_mc_part_eta      = nullptr;
        std::vector < float > *m_mc_part_phi      = nullptr;    
        std::vector < int >   *m_mc_part_pdgId    = nullptr;
        std::vector < int >   *m_mc_part_status   = nullptr;
        std::vector < int >   *m_mc_part_barcode  = nullptr;

        // ## Vertex Truth ## (MC)
        std::vector < float > *m_mc_vert_x     = nullptr;
        std::vector < float > *m_mc_vert_y     = nullptr;
        std::vector < float > *m_mc_vert_z     = nullptr;
        std::vector < float > *m_mc_vert_time  = nullptr;
        std::vector < float > *m_mc_vert_perp  = nullptr;
        std::vector < float > *m_mc_vert_eta   = nullptr;
        std::vector < float > *m_mc_vert_phi   = nullptr;
        std::vector < int > *m_mc_vert_barcode = nullptr;
        std::vector < int > *m_mc_vert_status      = nullptr;

        // ## Primary Vertex ##
        std::vector < float >  *m_vtx_x            = nullptr;
        std::vector < float >  *m_vtx_y            = nullptr;
        std::vector < float >  *m_vtx_z            = nullptr;
        std::vector < float >  *m_vtx_deltaZ0      = nullptr;
        std::vector < float >  *m_vtx_delta_z0_sin = nullptr;
        std::vector < double > *m_vtx_d0sig        = nullptr;
    
        // ## Photons Reco ##
        std::vector < float > *m_ph_eta     = nullptr;
        std::vector < float > *m_ph_phi     = nullptr;
        std::vector < float > *m_ph_pt      = nullptr;
        std::vector < float > *m_ph_energy  = nullptr;
        std::vector < float > *m_ph_m       = nullptr;

        // ## Electrons ##
        std::vector < int >   *m_el_index   = nullptr;
        std::vector < float > *m_el_Pt      = nullptr;
        std::vector < float > *m_el_et      = nullptr;
        std::vector < float > *m_el_Eta     = nullptr;
        std::vector < float > *m_el_Phi     = nullptr;
        std::vector < float > *m_el_m       = nullptr;
        std::vector < float > *m_el_eoverp  = nullptr;
        // offline shower shapes
        std::vector < float > *m_el_f1  = nullptr;
        std::vector < float > *m_el_f3  = nullptr;
        std::vector < float > *m_el_eratio = nullptr;
        std::vector < float > *m_el_weta1 = nullptr;  
        std::vector < float > *m_el_weta2 = nullptr;  
        std::vector < float > *m_el_fracs1 = nullptr; 
        std::vector < float > *m_el_wtots1 = nullptr; 
        std::vector < float > *m_el_e277 = nullptr; 
        std::vector < float > *m_el_reta = nullptr; 
        std::vector < float > *m_el_rphi = nullptr; 
        std::vector < float > *m_el_deltae = nullptr; 
        std::vector < float > *m_el_rhad = nullptr;  
        std::vector < float > *m_el_rhad1 = nullptr; 

        // ## Tag and probe ##
        std::vector < float > *m_tp_electronPt = nullptr;
        std::vector < float > *m_tp_electronEt = nullptr;
        std::vector < float > *m_tp_electronEta = nullptr;
        std::vector < float > *m_tp_electronPhi = nullptr;
        // indexes
        std::vector < int >   *m_tp_probeIndex = nullptr; // contains the electron indexes associated to each electron (tags). The vector position represent the Tags, the number on it, the associated Probe.
        std::vector < int >   *m_tp_tagIndex = nullptr; // index for each electron in the container
        std::vector < bool >  *m_tp_isTag = nullptr;
        std::vector < bool >  *m_tp_isProbe = nullptr;
        // Zee
        std::vector < double >  *m_zee_M = nullptr;
        std::vector < double >  *m_zee_E = nullptr;
        std::vector < double >  *m_zee_pt = nullptr;
        std::vector < double >  *m_zee_px = nullptr;
        std::vector < double >  *m_zee_py = nullptr;
        std::vector < double >  *m_zee_pz = nullptr;
        std::vector < double >  *m_zee_T = nullptr;
        std::vector < double >  *m_zee_deltaR = nullptr;
};

#endif
