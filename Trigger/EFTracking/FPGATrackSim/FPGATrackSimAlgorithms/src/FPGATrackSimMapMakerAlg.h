/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATRACKSIM_MAPMAKERALG_H
#define FPGATRACKSIM_MAPMAKERALG_H

/*
 * Produce Map files from wrappers.
 */

#include "AthenaBaseComps/AthAlgorithm.h" //inheritance
#include "GaudiKernel/ToolHandle.h"  //member
#include "FPGATrackSimObjects/FPGATrackSimTypes.h" //enum classes (e.g. SiliconTech, DetectorZone) in global namespace :-(
#include "FPGATrackSimObjects/FPGATrackSimEventInputHeader.h" //member
#include "FPGATrackSimInput/IFPGATrackSimEventInputHeaderTool.h" //tool handle template param
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"
#include "FPGATrackSimSGInput/IFPGATrackSimInputTool.h"

#include <fstream> //ofstream members
#include <tuple> //typedef
#include <map> //member
#include <set>  //member
#include <vector> //member


class FPGATrackSimModuleRelabel;
class FPGATrackSimHit;
class TFile;



class FPGATrackSimMapMakerAlg : public AthAlgorithm
{
    public:
        typedef std::tuple<SiliconTech, DetectorZone, int, int, int> FPGATrackSimModuleId;
        //
        FPGATrackSimMapMakerAlg(const std::string& name, ISvcLocator* pSvcLocator);
        virtual ~FPGATrackSimMapMakerAlg() = default;

        StatusCode initialize() override;
        StatusCode execute() override;
        StatusCode finalize() override;

    private:
        // Handles
        ToolHandle<IFPGATrackSimEventInputHeaderTool>    m_hitInputTool { this, "InputTool", "FPGATrackSimSGToRawHitsTool/FPGATrackSimInputTool", "HitInput Tool" };
        ToolHandle<IFPGATrackSimInputTool>               m_hitSGInputTool {this, "SGInputTool", "", "Input tool from SG"};

        ServiceHandle<IFPGATrackSimEventSelectionSvc>    m_evtSel {this, "eventSelector", "", "Event selection Svc"};

        FPGATrackSimEventInputHeader         m_eventHeader;

        struct Module {
            SiliconTech det{SiliconTech::strip};
            DetectorZone bec{DetectorZone::barrel};
            int lyr{};
            int eta{};
            int phi{};
            std::vector<int> numTracks; // used for trimming, indexed by slice
            int plane{-1};
            Module() = default;
            Module(SiliconTech det, DetectorZone bec, int lyr, int eta, int phi) : det(det), bec(bec), lyr(lyr), eta(eta), phi(phi) {}
            FPGATrackSimModuleId moduleId() const {
                return std::tie(det,bec,lyr,eta,phi);
            }
            bool operator==(const Module& m) const
            {
                return std::tie(det,bec,lyr,eta,phi) == std::tie(m.det,m.bec,m.lyr,m.eta,m.phi);
            }
            bool operator<(const Module & m) const
            {
                return std::tie(det,bec,lyr,eta,phi) < std::tie(m.det,m.bec,m.lyr,m.eta,m.phi);
            }
            bool operator>(const Module & m) const
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
        const std::vector<std::vector<std::string>>* m_planes{};
        const std::vector<std::vector<std::string>>* m_planes2{};

        // Fully generic "logical layers". these assign all pixel layers -> first stage and all strip layers -> second stage.
        // This won't scale well for the parts of the forward region that actually need to extrapolate into the second stage, but
        // it's good enough for now.
        const std::vector<std::vector<std::string>> m_planes_generic = {
            {"pb0", "pe0+","pe1+","pe2+","pe3+","pe4+","pe5+", "pe6+", "pe7+", "pe8+", "pe9+","pe10+","pe11+", "pe12+", "pe13+", "pe14+", "pe15+", "pe16+", },
            {"pb1", "pe19+", "pe23+", "pe25+", },
            {"pb2", "pe31+", "pe32+", "pe33+", "pe34+", "pe35+", "pe36+", "pe37+", "pe38+", "pe39+", "pe40+",
                    "pe41+", "pe42+", "pe43+", "pe44+", "pe45+", "pe46+",
                    "pe47+", "pe48+", "pe49+", "pe50+", "pe51+", "pe52+", "pe53+", "pe54+", "pe55+", "pe56+", },
            {"pb3", "pe57+","pe58+","pe59+", "pe60+", "pe61+", "pe62+", "pe63+", "pe64+", "pe65+", "pe66+","pe67+","pe68+","pe69+"},
            {"pb4", "pe70+","pe71+","pe72+", "pe73+", "pe74+", "pe75+", "pe76+", "pe77+", "pe78+", "pe79+",
                    "pe80+","pe81+","pe82+", "pe83+", "pe84+", "pe85+", "pe86+", "pe87+", "pe88+", "pe89+",}
        };

        // This is the second stage version.
        const std::vector<std::vector<std::string>> m_planes2_generic = {
            {"pb0", "pe0+","pe1+","pe2+","pe3+","pe4+","pe5+", "pe6+", "pe7+", "pe8+", "pe9+","pe10+","pe11+", "pe12+", "pe13+", "pe14+", "pe15+", "pe16+", },
            {"pb1", "pe19+",  "pe23+",  "pe25+", },
            {"pb2", "pe31+", "pe32+", "pe33+", "pe34+", "pe35+", "pe36+", "pe37+", "pe38+", "pe39+", "pe40+",
                    "pe41+", "pe42+", "pe43+", "pe44+", "pe45+", "pe46+",
                    "pe47+", "pe48+", "pe49+", "pe50+", "pe51+", "pe52+", "pe53+", "pe54+", "pe55+", "pe56+", },
            {"pb3", "pe57+","pe58+","pe59+", "pe60+", "pe61+", "pe62+", "pe63+", "pe64+", "pe65+", "pe66+","pe67+","pe68+","pe69+"},
            {"pb4", "pe70+","pe71+","pe72+", "pe73+", "pe74+", "pe75+", "pe76+", "pe77+", "pe78+", "pe79+",
                    "pe80+","pe81+","pe82+", "pe83+", "pe84+", "pe85+", "pe86+", "pe87+", "pe88+", "pe89+",},
            {"sb0", "se4+", "se0+"},
            {"sb1", "se5+", "se1+"},
            {"sb2", "se6+", "se2+"},
            {"sb3", "se7+", "se3+"},
            {"sb4", "se8+"},
            {"sb5", "se9+"},
            {"sb6", "se10+"},
            {"sb7", "se11+"}
        };

        // The generic assignments are the defaults-- but these are actually controlled from python.
        Gaudi::Property<std::vector<std::vector<std::string>>> m_overridePlanes {this, "planes", m_planes_generic, "Logical layer assignments" };
        Gaudi::Property<std::vector<std::vector<std::string>>> m_overridePlanes2 {this, "planes2", m_planes2_generic, "Logical layer assignments" };

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
        std::unique_ptr<TFile> m_monitorFile{};


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
        int findPlane(const std::vector<std::vector<std::string>>* planes, const std::string& test);
        std::string makeRmapLines(std::vector<FPGATrackSimHit> const & hits, SiliconTech det, DetectorZone bec, int max);
        std::string makeSubrmapLines(std::vector<Module*> const & allmods, SiliconTech det, DetectorZone bec, int max);
        void parseKeyString(); // sets m_keylayer and m_keylayer2 based on the Keystring and Keystring2 run args

};

#endif // FPGATRACKSIMMAPMAKERALG_H
