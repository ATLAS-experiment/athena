/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATRACKSIM_MAPMAKERALG_H
#define FPGATRACKSIM_MAPMAKERALG_H

/*
 * Produce Map files from wrappers.
 */

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/MsgStream.h"
#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimEventInputHeader.h"
#include "FPGATrackSimMaps/FPGATrackSimModuleRelabel.h"
#include "FPGATrackSimInput/IFPGATrackSimEventInputHeaderTool.h"
#include "TFile.h"
#include <fstream>

class IFPGATrackSimEventInputHeaderTool;
class FPGATrackSimEventInputHeader;
class FPGATrackSimHit;

typedef std::tuple<SiliconTech, DetectorZone, int, int, int> FPGATrackSimModuleId;

class FPGATrackSimMapMakerAlg : public AthAlgorithm
{
    public:
        FPGATrackSimMapMakerAlg(const std::string& name, ISvcLocator* pSvcLocator);
        virtual ~FPGATrackSimMapMakerAlg() = default;

        StatusCode initialize() override;
        StatusCode execute() override;
        StatusCode finalize() override;

    private:
        // Handles
        ToolHandle<IFPGATrackSimEventInputHeaderTool>    m_hitInputTool { this, "InputTool", "FPGATrackSimSGToRawHitsTool/FPGATrackSimInputTool", "HitInput Tool" };

        FPGATrackSimEventInputHeader         m_eventHeader;

        struct Module {
            SiliconTech det;
            DetectorZone bec;
            int lyr;
            int eta;
            int phi;
            std::vector<int> numTracks; // used for trimming, indexed by slice
            int plane;
            Module() : det(SiliconTech::strip), bec(DetectorZone::barrel), lyr(0), eta(0), phi(0), numTracks(std::vector<int>()), plane(-1) {}
            Module(SiliconTech det, DetectorZone bec, int lyr, int eta, int phi) : det(det), bec(bec), lyr(lyr), eta(eta), phi(phi), numTracks(std::vector<int>()), plane(-1) {}
            FPGATrackSimModuleId moduleId() const {
                return std::tie(det,bec,lyr,eta,phi);
            }
            bool operator==(const Module& m) const
            {
                return std::tie(det,bec,lyr,eta,phi) == std::tie(m.det,m.bec,m.lyr,m.eta,m.phi);
            }
            bool operator<(const Module m) const
            {
                return std::tie(det,bec,lyr,eta,phi) < std::tie(m.det,m.bec,m.lyr,m.eta,m.phi);
            }
            bool operator>(const Module m) const
            {
                return std::tie(det,bec,lyr,eta,phi) > std::tie(m.det,m.bec,m.lyr,m.eta,m.phi);
            }
        };

	// Flags
	Gaudi::Property<int> m_maxEvents {this, "maxEvents", 10000, "Max Events"};
	Gaudi::Property<int> m_region {this, "region", 0, "Region"};
	Gaudi::Property<float> m_trim {this, "trim", 0.1, "trim modules with less than given percent of tracks"};
	Gaudi::Property<std::string> m_outFileName {this, "OutFileName", "", "naming convention for maps"};
	Gaudi::Property<std::string> m_keystring {this, "KeyString", "strip,barrel,2", "key layer to use for subrmap"};
	Gaudi::Property<std::string> m_keystring2 {this, "KeyString2", "", "second key layer for 2D slicing"};
	Gaudi::Property<int> m_nSlices {this, "nSlices", -1, "default is full granularity/maximum number of slices possible"};
	Gaudi::Property<bool> m_doSpacePoints {this, "doSpacePoints", false, "Use 9L instead of 8L configuration for first stage (for example)"};
	Gaudi::Property<float> m_globalTrim {this, "globalTrim", 0.1, "Trimming applied globally to the key layer before determining slice boundaries"};
	Gaudi::Property<std::string> m_description {this, "description", "", "tag description"};
	Gaudi::Property<std::string> m_geoTag {this, "GeometryVersion", "ATLAS-P2-ITK-22-02-00", "Geometry tag that this set of maps is for. TODO can we store/read from wrappers?"};
	Gaudi::Property<bool> m_remapModules {this, "remapModules", false, "Allow maps to be drawn that slice modules more finely, by remapping module indices"};
	Gaudi::Property<bool> m_drawSlices {this, "drawSlices", false, "Draw the huge 2D slice histograms"};

        // Instance of the module remap object.
        FPGATrackSimModuleRelabel* m_moduleRelabel = nullptr;

        // For Subregion map making (zslice map)
        std::map<FPGATrackSimModuleId, Module> m_modules;
        std::map<int, std::vector<Module*>> m_track2modules, m_slice2modules;
        std::map<int, int> m_track2slice;

        // Event storage
        std::vector<FPGATrackSimHit> m_pbHits, m_peHits, m_sbHits, m_seHits, m_allHits;

        int m_pbmax = -1; // pixel barrel largest layer index
        int m_sbmax = -1;
        std::vector<int> m_pemax = {-1,-1}; // [positive endcap maxLayer, negative endcap maxLayer]
        std::vector<int> m_semax = {-1,-1};


        /* 3D vector that assigns planes for pmap. indices are [reg][plane][physical layers]
        -> first two chars are DetType and DetZone, pb = pixel barrel, se = strip endcap
        -> then the layer number is given, followed by a +/- if positive/negative endcap
        -> -1 means the layer is not used
        -> Example: se67+ = Strip Positve Endcap layer 67
        */
        const std::vector<std::vector<std::vector<std::string>>>* m_planes;
        const std::vector< std::vector< std::vector<std::string> > > m_planes_default = //first stage
        {
            { // region 0
                {"pb4"},{"sb0"},{"sb2"},{"sb3"},{"sb4"},{"sb5"},{"sb6"},{"sb7"} // layers that make up each plane.
                //{"pb0","pe0+","pe1+","pe2+"},{"pb1","pe17+","pe18+"},{"pb2"},{"pb3"},{"pb4"} // pixel only
            },
            { // region 1
                {"pb4"},{"sb0"},{"sb2"},{"sb3"},{"sb4"},{"sb5"},{"sb6"},{"sb7"}
            },
            { // region 2
                {"-1"},{"-1"},{"-1"},{"-1"},{"-1"},{"-1"},{"-1"},{"-1"}
            },
            { // region 3
                {"pb4","pe66+","pe67+","pe68+","pe69+"},{"se5+"},{"se6+"},{"se7+"},{"se8+"},{"se9+"},{"se10+"},{"se11+"}
            },
            { // region 4
                {"pe34+","pe12+"},{"pe36+","pe29+"},{"pe8+","pe40+","pe30+"},{"pe56+","pe65+"},{"pe10+","pe6+"},{"pe11+","pe7+"},{"pe31+","pe32+"},{"pe9+","pe53+"}
            },
            { // region 5 - same eta as region 0, different phi
                {"pb4"},{"sb0"},{"sb2"},{"sb3"},{"sb4"},{"sb5"},{"sb6"},{"sb7"}
            },
            { // region 6 - same eta as region 0, different phi
                {"pb4"},{"sb0"},{"sb2"},{"sb3"},{"sb4"},{"sb5"},{"sb6"},{"sb7"}
            },
            { // region 7 - same eta as region 0, different phi
                {"pb4"},{"sb0"},{"sb2"},{"sb3"},{"sb4"},{"sb5"},{"sb6"},{"sb7"}
            },
        };


        // 9L version of the above for spacepoints. There must be a better way to implement this without
        // needing to duplicate the entire structure, but I'm not sure how without using the pmap files...
        const std::vector< std::vector< std::vector<std::string> > > m_planes_sp = //first stage
        {
            { // region 0
                {"pb4"},{"sb0"},{"sb1"},{"sb2"},{"sb3"},{"sb4"},{"sb5"},{"sb6"},{"sb7"} // layers that make up each plane.
                //{"pb0","pe0+","pe1+","pe2+"},{"pb1","pe17+","pe18+"},{"pb2"},{"pb3"},{"pb4"} // pixel only
            },
            { // region 1
                {"pb4"},{"sb0"},{"sb1"},{"sb2"},{"sb3"},{"sb4"},{"sb5"},{"sb6"},{"sb7"}
            },
            { // region 2
                {"-1"},{"-1"},{"-1"},{"-1"},{"-1"},{"-1"},{"-1"},{"-1"},{"-1"}
            },
            { // region 3
                {"pb4","pe66+","pe67+","pe68+","pe69+"},{"se4+"},{"se5+"},{"se6+"},{"se7+"},{"se8+"},{"se9+"},{"se10+"},{"se11+"}
            },
            { // region 4
                //{"pe34+","pe12+"},{"pe36+","pe29+"},{"pe8+","pe40+","pe30+"},{"pe56+","pe65+"},{"pe10+","pe6+"},{"pe11+","pe7+"},{"pe31+","pe32+"},{"pe9+","pe53+"},{"pe30+"}
                {"pe4+", "pe5+", "pe6+"},
                {"pe7+", "pe8+", "pe9+"},
                {"pe10+", "pe11+", "pe12+", "pe29+"},
                {"pe30+", "pe31+", "pe32+", "pe33+"},
                {"pe34+"},
                {"pe36+"},
                {"pe38+", "pe40+", "pe42+", "pe53+"},
                {"pe54+", "pe55+"},
                {"pe56+", "pe57+", "pe65+"}
            },
            { // region 5 - same eta as region 0, different phi
                {"pb4"},{"sb0"},{"sb1"},{"sb2"},{"sb3"},{"sb4"},{"sb5"},{"sb6"},{"sb7"}
            },
            { // region 6 - same eta as region 0, different phi
                {"pb4"},{"sb0"},{"sb1"},{"sb2"},{"sb3"},{"sb4"},{"sb5"},{"sb6"},{"sb7"}
            },
            { // region 7 - same eta as region 0, different phi
                {"pb4"},{"sb0"},{"sb1"},{"sb2"},{"sb3"},{"sb4"},{"sb5"},{"sb6"},{"sb7"}
            },
        };

        const std::vector <std::vector <std::vector <std::string> > > m_planes2 = // second stage
        {
            { // region 0
                {"pb0"},{"pb1"},{"pb2"},{"pb3"},{"pb4"},{"sb0"},{"sb1"},{"sb2"},{"sb3"},{"sb4"},{"sb5"},{"sb6"},{"sb7"}
            },
            { // region 1
                {"pb0"},{"pb1"},{"pb2"},{"pb3"},{"pb4"},{"sb0"},{"sb1"},{"sb2"},{"sb3"},{"sb4"},{"sb5"},{"sb6"},{"sb7"}
            },
            { // region 2
                {"-1"},{"-1"},{"-1"},{"-1"},{"-1"},{"-1"},{"-1"},{"-1"},{"-1"},{"-1"},{"-1"},{"-1"},{"-1"}
            },
            { // region 3
                {"pb2"},{"pb3","pe58+"},{"pb4","pe66+","pe67+","pe68+","pe69+"},{"se2+"},{"se3+"},{"se4+"},{"se5+"},{"se6+"},{"se7+"},{"se8+"},{"se9+"},{"se10+"},{"se11+"}
            },
            { // region 4
                {"pe0+"},{"pe1+"},{"pe2+"},{"pe3+"},{"pe4+"},{"pe5+"},{"pe6+"},{"pe7+"},{"pe8+"},{"pe9+"},{"pe10+"},{"pe11+"},{"pe12+"} // dummy values to avoid "vector::_M_range_check:"
            },
            { // region 5 - same eta as region 0, different phi
                {"pb0"},{"pb1"},{"pb2"},{"pb3"},{"pb4"},{"sb0"},{"sb1"},{"sb2"},{"sb3"},{"sb4"},{"sb5"},{"sb6"},{"sb7"}
            },
            { // region 6 - same eta as region 0, different phi
                {"pb0"},{"pb1"},{"pb2"},{"pb3"},{"pb4"},{"sb0"},{"sb1"},{"sb2"},{"sb3"},{"sb4"},{"sb5"},{"sb6"},{"sb7"}
            },
            { // region 7 - same eta as region 0, different phi
                {"pb0"},{"pb1"},{"pb2"},{"pb3"},{"pb4"},{"sb0"},{"sb1"},{"sb2"},{"sb3"},{"sb4"},{"sb5"},{"sb6"},{"sb7"}
            },
        };

        std::map <std::string, std::set<int>> m_keylayer; // key layer used in z-slicing, defined by user with KeyString run arg and set using parseKeyString()
        std::map <std::string, std::set<int>> m_keylayer2; // for 2D slicing

        bool m_key2 = false;
        std::map <int, int> m_key_etamods; // eta module values of the key layer. each etamod = 1 slice for full granulatiy slicing
        std::set <int> m_key_etamods2; // for 2D slicing
        std::set <int> m_usedTracks; // tracks that hit the key layer
        std::vector <std::vector < std::vector<float> > > m_radii; // used to calculate mean radii per layer for each slice, [slice][plane][hit]
        std::vector <std::vector < std::vector<float> > > m_z; // used to calculate median z per layer for each slice, [slice][plane][hit]
        // output map files and monitoring
        std::ofstream m_pmap, m_rmap, m_subrmap, m_etapat, m_radfile, m_zedfile;
        TFile *m_monitorFile;


        // TODO make automatic
        const std::vector<uint32_t> m_diskIndex =  {0,17,47,58,66}; // number of disks per layer in ATLAS-P2-ITK-22-02-00:  [17, 30, 11, 8, 9] --> [0,17,47,58,66]

        StatusCode readInputs(bool & done);
        StatusCode writePmapAndRmap(std::vector<FPGATrackSimHit> const & pbHits, std::vector<FPGATrackSimHit> const & peHits, std::vector<FPGATrackSimHit> const & sbHits, std::vector<FPGATrackSimHit> const & seHits, int region);
        StatusCode writeSubrmap(std::vector<FPGATrackSimHit> const & allHits);
        StatusCode writeEtaPatterns(); // writes txt file used in the FPGATrackSimEtaPatternFilterTool to filter roads based on eta module
        StatusCode writeRadiiFile(std::vector<FPGATrackSimHit> const & allHits); // writes txt file with mean radii per layer for each slice, used in 1D Hough Transform
        StatusCode writeMedianZFile(std::vector<FPGATrackSimHit> const & allHits); // writes txt file with median z per layer for each slice, used in 1D Hough Transform

        // Helpers
        void drawSlices(std::vector<FPGATrackSimHit> const & allHits);
        std::map <std::string, SiliconTech> m_det2tech = { {"pixel",SiliconTech::pixel},  {"strip",SiliconTech::strip} }; // for parsing KeyString
        std::map <std::string, DetectorZone> m_bec2zone = { {"barrel",DetectorZone::barrel},  {"posEndcap",DetectorZone::posEndcap}, {"negEndcap",DetectorZone::negEndcap} };
        bool isOnKeyLayer(int keynum, SiliconTech det, DetectorZone bec, int lyr); // returns if hit is on a key layer or not. keynum is either 1 or 2 for the first or second keylayer (if using 2D slicing)
        int findPlane(const std::vector<std::vector<std::string>>& planes, std::string test);
        std::string makeRmapLines(std::vector<FPGATrackSimHit> const & hits, SiliconTech det, DetectorZone bec, int max);
        std::string makeSubrmapLines(std::vector<Module*> const & allmods, SiliconTech det, DetectorZone bec, int max);
        void parseKeyString(); // sets m_keylayer and m_keylayer2 based on the Keystring and Keystring2 run args

};

#endif // FPGATRACKSIMMAPMAKERALG_H
