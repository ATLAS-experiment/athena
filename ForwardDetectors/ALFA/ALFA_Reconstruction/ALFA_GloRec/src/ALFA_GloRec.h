/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ALFA_GloRec_h
#define ALFA_GloRec_h

#include "AthenaBaseComps/AthAlgorithm.h"

//root
#include "TObjArray.h"

//stl
#include <string>
#include <vector>

class ALFA_GloRecEvCollection;
class AlfaTrackCand;
class TH1F;
class TH2F;


/////////////////////////////////////////////////////////
//
//       ALFA_GloRec class declaration
//
//////////////////////////////////////////////////////////
class ALFA_GloRec : public AthAlgorithm
{
public:
	ALFA_GloRec (const std::string& name, ISvcLocator* pSvcLocator);
	~ALFA_GloRec();

private:



    ////////////////////////////////
    //  general members
    ////////////////////////////////
	ALFA_GloRecEvCollection* m_pGloRecEvCollection{};


    //////////////////////////////////////////////
    //  steerable members
    //////////////////////////////////////////////
    int m_iDataType = 0;            
    std::string m_strTrackPathPatterns;
    TObjArray m_TObjArrTrackPathPatterns{};
    std::string m_strGloRecAnalysisFile;
    std::string m_strGloRecCollectionName;   
    std::string m_strLocRecCorrCollectionName;
    std::string m_TruthCollectionName;   
        

    //////////////////////////////////////////////
    //  methods 
    //////////////////////////////////////////////
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();
    StatusCode Truth_info();
    void InitHistos();
    void FillTrackCandHistos(AlfaTrackCand * trkcand);
    void WriteHistos();
 

    /////////////////////////////////////////
    // generated vertices and momenta
    /////////////////////////////////////////
    double m_px_g_pos{};
    double m_py_g_pos{};
    double m_pz_g_pos{};
    double m_x_g_pos{};
    double m_y_g_pos{};
    double m_z_g_pos{};

    double m_px_g_neg{};
    double m_py_g_neg{};
    double m_pz_g_neg{};
    double m_x_g_neg{};
    double m_y_g_neg{};
    double m_z_g_neg{};

    ////////////////////////////////////////
    //  histograms
    ////////////////////////////////////////
    TH1F * m_th1_x_g{};
    TH1F * m_th1_y_g{};
    TH1F * m_th1_xslope_g{};
    TH1F * m_th1_yslope_g{};

	  TH1F * m_th1_xnearuppotresiduals{};
    TH1F * m_th1_ynearuppotresiduals{};
  	TH1F * m_th1_xfaruppotresiduals{};
    TH1F * m_th1_yfaruppotresiduals{};
    TH1F * m_th1_xnearlwpotresiduals{};
    TH1F * m_th1_ynearlwpotresiduals{};
  	TH1F * m_th1_xfarlwpotresiduals{};
    TH1F * m_th1_yfarlwpotresiduals{};

    TH2F * m_th2_truexvsrecx{};
    TH2F * m_th2_trueyvsrecy{};
    TH2F * m_th2_truexslopevsrecxslope{};
    TH2F * m_th2_trueyslopevsrecyslope{};
    TH1F * m_th1_recxovertruex{};
    TH1F * m_th1_recyovertruey{};
    TH1F * m_th1_recxslopeovertruexslope{};
    TH1F * m_th1_recyslopeovertrueyslope{};
    TH1F * m_th1_recxminustruex{};
    TH1F * m_th1_recyminustruey{};
    TH1F * m_th1_recxslopeminustruexslope{};
    TH1F * m_th1_recyslopeminustrueyslope{};     

    TH2F * m_th2_extrapxvsrecxnearpot{};   
    TH2F * m_th2_extrapyvsrecynearpot{};
    TH1F * m_th1_recxoverextrapxnearpot{};
    TH1F * m_th1_recyoverextrapynearpot{};
    TH1F * m_th1_recxminusextrapxnearpot{};
    TH1F * m_th1_recyminusextrapynearpot{};

    TH2F * m_th2_extrapxvsrecxfarpot{};   
    TH2F * m_th2_extrapyvsrecyfarpot{};
    TH1F * m_th1_recxoverextrapxfarpot{};
    TH1F * m_th1_recyoverextrapyfarpot{};
    TH1F * m_th1_recxminusextrapxfarpot{};
    TH1F * m_th1_recyminusextrapyfarpot{};

};

#endif // ALFA_GloRec_h
