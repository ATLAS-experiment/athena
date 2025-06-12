// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimNNTRACKTOOL_H
#define FPGATrackSimNNTRACKTOOL_H

/**
 * @file FPGATrackSimNNTrackTool.h
 * @author Elliott Cheu
 * @date April 27 2021
 * @brief Utilize NN score to build track candidates
 *
 */

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "FPGATrackSimAlgorithms/OnnxRuntimeBase.h"

#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimMultiTruth.h"
#include "FPGATrackSimObjects/FPGATrackSimTruthTrack.h"
#include "FPGATrackSimBanks/FPGATrackSimSectorBank.h"

#include "GaudiKernel/ITHistSvc.h"

#include "FPGATrackSimAlgorithms/FPGATrackSimTrackingToolBase.h"

#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"


class IFPGATrackSimMappingSvc;

class FPGATrackSimNNTrackTool : public FPGATrackSimTrackingToolBase, public OnnxRuntimeBase
{
  using OnnxRuntimeBase::OnnxRuntimeBase;

  public:

	///////////////////////////////////////////////////////////////////////
	// AthAlgTool

	FPGATrackSimNNTrackTool(const std::string&, const std::string&, const IInterface*);

	virtual StatusCode initialize() override;
	StatusCode getTracks_1st(std::vector<std::shared_ptr<const FPGATrackSimRoad>> &roads, std::vector<FPGATrackSimTrack> &tracks);
	StatusCode getTracks_2nd(std::vector<std::shared_ptr<const FPGATrackSimRoad>> &roads, std::vector<FPGATrackSimTrack> &tracks);
        StatusCode setTrackParameters(std::vector<FPGATrackSimTrack> &tracks, bool isFirst);

	static float getXScale() { return 1015.;};
	static float getYScale() { return 1015.;};
	static float getZScale() { return 3000.;};
        static float getQoverPtScale() { return 0.001;};
        static float getEtaScale() { return 5.0;};
        static float getPhiScale() { return 3.15;};
        static float getD0Scale() { return 2.0;};
        static float getZ0Scale() { return 200.;};        
  
	// Flags
	Gaudi::Property <unsigned int> m_minNumberOfRealHitsInATrack{ this, "MinNumberOfRealHitsInATrack", 4, "Minimum number of real hits in a track candidate to process" };
	Gaudi::Property <bool> m_doGNNTracking{ this, "doGNNTracking", false, "Flag to turn on GNN Tracking configuration for road-to-track" };

  private:

	ServiceHandle<IFPGATrackSimMappingSvc> m_FPGATrackSimMapping{this, "FPGATrackSimMappingSvc", ""};
	ServiceHandle<ITHistSvc> m_tHistSvc{this, "THistSvc","THistSvc"};

	OnnxRuntimeBase m_paramNN_1st;
	OnnxRuntimeBase m_paramNN_2nd;
	OnnxRuntimeBase m_fakeNN_1st;
	OnnxRuntimeBase m_fakeNN_2nd;

	bool m_useParamNN_1st = true;
	bool m_useParamNN_2nd = true;

	std::vector<float> m_x; // x position of hit in road
	std::vector<float> m_y; // y pos
	std::vector<float> m_z; // z pos
	std::vector<float> m_barcodefrac; // truth barcode fraction for the hit
	std::vector<int> m_barcode; // truth barcode for the hit
	std::vector<int> m_eventindex; // event index for the hit
	std::vector<unsigned int> m_isPixel; // is hit pixel? if 0 it is strip
	std::vector<unsigned int> m_layer; // layer ID
	std::vector<unsigned int> m_isBarrel; // is hit in barrel? if 0 it is endcap
	std::vector<unsigned int> m_etawidth;
	std::vector<unsigned int> m_phiwidth;
	std::vector<unsigned int> m_etamodule;
	std::vector<unsigned int> m_phimodule;
	std::vector<unsigned int> m_ID; // ID hash for hit

	std::vector<float> m_truth_d0;
	std::vector<float> m_truth_z0;
	std::vector<float> m_truth_pt;
	std::vector<float> m_truth_eta;
	std::vector<float> m_truth_phi;
	std::vector<float> m_truth_pdg;
	std::vector<int> m_truth_q;
	std::vector<int> m_truth_barcode;
	std::vector<int> m_truth_eventindex;

	//////////////////////////////////////////////////////////////////
	// NN stuff
	std::vector<const char*> m_input_node_names;
	std::vector<int64_t> m_input_node_dims;
	std::vector<const char*> m_output_node_names;

    void compute_truth(FPGATrackSimTrack & newtrk) const;

};


#endif // FPGATrackSimNNTRACKTOOL_H
