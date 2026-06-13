/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// -------------------------------------------------------------------
// Persistent representation of TBEvent/TBTrackInfo
// See: https://twiki.cern.ch/twiki/bin/view/Atlas/TransientPersistentSeparation#TP_converters_for_component_type
// Author: Iftach Sadeh (iftach.sadeh@NOSPAMTODAYcern.ch) , February 2010
// -------------------------------------------------------------------
#ifndef TBTRACKINFO_P1_H
#define TBTRACKINFO_P1_H


class TBTrackInfo_p1
{

public:

	// variables from TB_tree
	int		tb_Trigger = 0;
	int		tb_trk_nTracks = 0;
	int		tb_trk_nTracksTRT = 0;

	float	tb_sADC_S1 = 0;
	float	tb_sADC_muHalo = 0;
	float	tb_sADC_muTag = 0;
	float	tb_sADC_C2 = 0;
	float	tb_sADC_TRTSci = 0;

	std::vector<int>	tb_trk_nPixelHits;  // max arrays size (see TBTrackInfoFromTag) = [60]
	std::vector<int>	tb_trk_nSctHits;    // max arrays size (see TBTrackInfoFromTag) = [60]
	std::vector<int>	tb_trk_nTrtHitsTRT; // max arrays size (see TBTrackInfoFromTag) = [60]

	std::vector< std::vector<int> >	tb_trk_Trt_HLTRT; // max arrays size (see TBTrackInfoFromTag) = [60][100]


	// variables from TileRec_h1000
	float	tileRec_XchN1 = 0;
	float	tileRec_XchN2 = 0;
	float	tileRec_Xcha0 = 0;
	float	tileRec_Xcha1 = 0;

	std::vector<int>			tileRec_btdc1;  // max arrays size (see TBTrackInfoFromTag) = [16]
	std::vector<int>			tileRec_GainC0; // max arrays size (see TBTrackInfoFromTag) = [48]
	std::vector<int>			tileRec_GainC1; // max arrays size (see TBTrackInfoFromTag) = [48]
	std::vector<int>			tileRec_GainC2; // max arrays size (see TBTrackInfoFromTag) = [48]

	std::vector<float>		tileRec_MuBack; // max arrays size (see TBTrackInfoFromTag) = [14]
	std::vector<float>		tileRec_TfitC0; // max arrays size (see TBTrackInfoFromTag) = [48]
	std::vector<float>		tileRec_TfitC1; // max arrays size (see TBTrackInfoFromTag) = [48]
	std::vector<float>		tileRec_TfitC2; // max arrays size (see TBTrackInfoFromTag) = [48]
	std::vector<float>		tileRec_EfitC0; // max arrays size (see TBTrackInfoFromTag) = [48]
	std::vector<float>		tileRec_EfitC1; // max arrays size (see TBTrackInfoFromTag) = [48]
	std::vector<float>		tileRec_EfitC2; // max arrays size (see TBTrackInfoFromTag) = [48]
	std::vector<float>		tileRec_Chi2C0; // max arrays size (see TBTrackInfoFromTag) = [48]
	std::vector<float>		tileRec_Chi2C1; // max arrays size (see TBTrackInfoFromTag) = [48]
	std::vector<float>		tileRec_Chi2C2; // max arrays size (see TBTrackInfoFromTag) = [48]

	std::vector< std::vector<int> >		tileRec_SampleC0;  // max arrays size (see TBTrackInfoFromTag) = [48][9]
	std::vector< std::vector<int> >		tileRec_SampleC1;  // max arrays size (see TBTrackInfoFromTag) = [48][9]
	std::vector< std::vector<int> >		tileRec_SampleC2;  // max arrays size (see TBTrackInfoFromTag) = [48][9]
};


#endif
