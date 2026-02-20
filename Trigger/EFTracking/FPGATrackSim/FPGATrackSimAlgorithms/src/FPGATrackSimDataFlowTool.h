// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimDATAFLOWTOOL_H
#define FPGATrackSimDATAFLOWTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "FPGATrackSimObjects/FPGATrackSimTypes.h"

#include "TH1.h"

#include <fstream>

class FPGATrackSimDataFlowInfo;
class FPGATrackSimHit;
class FPGATrackSimCluster;
class FPGATrackSimLogicalEventInputHeader;
class FPGATrackSimRoad;
class FPGATrackSimTrack;

class IFPGATrackSimEventSelectionSvc;
class IFPGATrackSimMappingSvc;

/////////////////////////////////////////////////////////////////////////////
class FPGATrackSimDataFlowTool: public AthAlgTool
{
    public:

        FPGATrackSimDataFlowTool(std::string const &, std::string const &, IInterface const *);
        virtual ~FPGATrackSimDataFlowTool() = default;

        virtual StatusCode initialize() override;

        StatusCode calculateDataFlow(FPGATrackSimDataFlowInfo* info, FPGATrackSimLogicalEventInputHeader const * header_1st,
                                     std::vector<FPGATrackSimCluster> const & clusters_1st,
                                     const std::vector<FPGATrackSimRoad> & roads_1st, std::vector<FPGATrackSimTrack> const & tracks_1st,
                                     const std::vector<FPGATrackSimRoad> & roads_2nd, std::vector<FPGATrackSimTrack> const & tracks_2nd);

        StatusCode getDataFlowInfo(FPGATrackSimDataFlowInfo const & info);

        virtual StatusCode finalize() override;

    private:

        StatusCode makeDataFlowTable();

        StatusCode addDataFlow(float const n, std::string const & key, bool const isInt = true);
        StatusCode printDataFlow(std::string const & key, int const div = 1);

        void addTableBreak(unsigned const n = 1);
        void findAndReplaceAll(std::string & data, std::string const & toFind, std::string const & replaceStr) const;
        void setMaxAcceptance(TH1* h, double const max_frac, double& max_value) const;
        double roundTo(double const v, int const nSigDigits) const;

        void setHistDir(std::string const & dir) { m_dir = "/MONITOROUT" + dir; }
        std::string const & getHistDir() { return m_dir; }
        void clearHistDir() { m_dir = ""; }
        StatusCode regHist (std::string const & dir, TH1* h) { return m_tHistSvc->regHist (dir + h->GetName(), h); }

        ServiceHandle<ITHistSvc>                m_tHistSvc{this,"THistSvc","THistSvc"};
        ServiceHandle<IFPGATrackSimMappingSvc>       m_FPGATrackSimMapping{this,"FPGATrackSimMappingSvc","FPGATrackSimMappingSvc"};
        ServiceHandle<IFPGATrackSimEventSelectionSvc>    m_evtSel{this,"FPGATrackSimEventSelectionSvc","FPGATrackSimEventSelectionSvc"};


	Gaudi::Property<bool> m_runSecondStage {this, "RunSecondStage", false, "flag to enable running the second stage fitting"};
	Gaudi::Property<std::string> m_outputtag {this, "outputTag", "", "Extra string to use in output folder names - default none"};
	Gaudi::Property<float> m_cut_chi2ndof {this, "Chi2ndofCut", 40., "cut on Chi2 of FPGATrackSimTrack"};

        // data flow output
        std::string const m_dataFlowTxtName = "./dataflow.txt";
        std::string const m_dataFlowTeXName = "./dataflow.tex";
        std::ofstream m_dataFlowTxt;
        std::ofstream m_dataFlowTeX;

        // histogram directory
        std::string m_dir;

        size_t m_nEvents = 0;
        unsigned m_nLayers_1st = 0U; // alias to m_FPGATrackSimMapping->PlaneMap1stStage()->getNLogiLayers();

        // constants used for data flow table
        unsigned const m_nSigDigits = 2;
        unsigned const m_tableTypeWidth = 32;
        unsigned const m_tableDataWidth = 9;
        double   const m_max_frac = 0.99; // Maximum acceptance fraction of data mimicking data loss

        std::unordered_map<std::string, int>   m_dataFlowDataI_min;
        std::unordered_map<std::string, int>   m_dataFlowDataI_max;
        std::unordered_map<std::string, float> m_dataFlowDataF_min;
        std::unordered_map<std::string, float> m_dataFlowDataF_max;

        std::unordered_map<std::string, TH1I*> m_dataFlowHistsI;
        std::unordered_map<std::string, TH1F*> m_dataFlowHistsF;
};

#endif // FPGATrackSimDataFlowTool_h
