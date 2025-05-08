/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** 
    @class CaloNoiseCompCondAlg
    @brief algo to compute electronic and pile up noise in MeV
    @ based on old CaloNoiseTool, only WorkMode=1 implemented
*/

#ifndef CALOCONDPHYSALGS_CaloNoiseCompCondAlg_H
#define CALOCONDPHYSALGS_CaloNoiseCompCondAlg_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "CaloDetDescr/CaloDetDescrManager.h" //read handle
#include "CaloIdentifier/CaloCell_ID.h" //use of enum in namespace
#include "CaloConditions/CaloNoise.h" //write handle
#include "CaloIdentifier/CaloGain.h" //use of enum in namespace
#include "LArCabling/LArOnOffIdMapping.h" //read handle
#include "LArElecCalib/ILArShape.h" //read handle
#include "LArElecCalib/ILArPedestal.h" //read handle
#include "LArElecCalib/ILArfSampl.h" //read handle
#include "LArElecCalib/ILArMinBias.h" //read handle
#include "LArElecCalib/ILArAutoCorr.h" //read handle
#include "LArRawConditions/LArADC2MeV.h" //read handle
#include "LArRawConditions/LArOFC.h" //read handle
#include "AthenaKernel/IOVSvcDefs.h"

class AtlasDetectorID;
class CaloIdManager;
class CaloCell;

typedef std::vector< std::vector<float> > VectorContainer;
typedef std::vector< float >              SingleContainer;

enum VALUE_ENUM{BADVALUE=-999,BADVALUE_TO_RETURN=-1};
enum DATABASE_ENUM{iADC2MEV,iSIGMANOISE,iAUTOCORR,iOFC,iSHAPE,
		   iMINBIASRMS,iFSAMPL,nDATABASE};
enum CalorimeterNoiseType {
    ELECTRONICNOISE             =  100, /* return the electronic noise */
    ELECTRONICNOISE_HIGHESTGAIN =  101, /* same in highest gain  */
    PILEUPNOISE                 =  200, /* return the pileup noise     */
    PILEUPNOISE_HIGHESTGAIN     =  201, /* return the pileup noise     */
    TOTALNOISE                  =  300, /* return the total noise      */
    TOTALNOISE_HIGHESTGAIN      =  301, /* return the total noise      */
    JOBOPTION                   = 9999  /* defined by jobOptions       */ };

enum CalorimeterNoiseSymmetryHandling {
    ONLYRIGHTSIDEINTEGRATION =  0, /* integral pdf from E to infinity (or -infinity to E is E<0)  */
    ABSOLUTEENERGYVALUE     =  1, /* integral pdf from abs(E) to infinity     */
    MAXSYMMETRYHANDLING   = 9999  /* defined by jobOptions       */ };

namespace CLHEP { class HepRandomEngine; }

class CaloNoiseCompCondAlg: public AthAlgorithm {

   public:

    /** Standard Athena-Algorithm Constructor */
    CaloNoiseCompCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
    /** Default Destructor */
    virtual ~CaloNoiseCompCondAlg() = default;
      
    /** standard Athena-Algorithm method */
    virtual StatusCode          initialize() override final;
    /** standard Athena-Algorithm method */
    virtual StatusCode          execute() override final;
    /** standard Athena-Algorithm method */
    virtual StatusCode          finalize() override final {return StatusCode::SUCCESS;};

    //-------------- user interfaces ------------------------------------------
    // Note on NMinBias : if you use the interface without it or if you take -1, 
    //                    the returned data will be the one calculated at the 
    //                    initialization with the default value (0) or the 
    //                    specified one (with the property m_Nminbias).
    //                    In all cases, the NMinBias used to calculate the OFCs 
    //                    is NOT known by CaloNoiseCompCondAlg (CaloNoiseCompCondAlg always 
    //                    recomputes OFCs for its own needs)


  
    
 
    //== ELECTRONIC NOISE ==  
 
    //''''''''' functions to use currently
 
    float elecNoiseRMS(const CaloCell* caloCell);
      //Returns the sigma of the electronic noise, finding itself the right gain 
      //from the energy of the given caloCell.
 
 
    float elecNoiseRMS(const CaloDetDescrElement* caloDDE, 
          	     const CaloGain::CaloGain gain);
      //Returns the sigma of the electronic noise for the given caloDDE, gain and
      //the number of minimum bias events per bunch crossing. 
 
    std::vector<float> 
      elecNoiseRMS3gains(const CaloDetDescrElement* caloDDE);
      //Returns a vector containing the sigma of the electronic noise for the 
      //given caloDDE, for each gain of the considered calorimeter 
      //(eg: vector of 3 sigma3 gains for LAr)
 
 
 
    //== PILEUP NOISE ==  
 
    float pileupNoiseRMS(const CaloCell* caloCell, 
          	       const float Nminbias=-1); 
    float pileupNoiseRMS(const CaloDetDescrElement* caloDDE, 
          	       const float Nminbias=-1);
    //Nminbias is the number of minimum-bias events per bunch-crossing
    // (default is 0 (set in the constructor) ) 
 
    //== TOTAL NOISE ==   
 
    float totalNoiseRMS(const CaloCell* caloCell, 
          	      const float Nminbias=-1); 
    float totalNoiseRMS(const CaloDetDescrElement* caloDDE, 
                        const CaloGain::CaloGain gain,
          	      const float Nminbias=-1);
    float totalNoiseRMSHighestGain(const CaloCell* caloCell, 
          			 const float Nminbias=-1);
    float totalNoiseRMSHighestGain(const CaloDetDescrElement* caloDDE, 
          			 const float Nminbias=-1);
 
 
 
    //== GAIN == 
    CaloGain::CaloGain estimatedGain(const CaloCell* caloCell);
    CaloGain::CaloGain estimatedGain(const CaloCell* caloCell,
          			   const CaloDetDescrElement* caloDDE);
    CaloGain::CaloGain estimatedGain(const CaloDetDescrElement* caloDDE,
          			   const float &energy);
 
    //== ADDITIONNAL STUFF ==
    // these functions will be replaced soon by dedicated tools,
    // so please AVOID TO USING THEM
 
    //float eScale(const CaloDetDescrElement* caloDDE);
      //Returns the scale between RawChannels and Cells, i.e 
      //what returns LArG3Escale
 
    float adc2mev(const CaloDetDescrElement* caloDDE,
          	const CaloGain::CaloGain gain);
      //Returns adc2mev factor for the given caloDDE and gain
 
    float adc2mev(const Identifier& id,const CaloGain::CaloGain gain);
      //Returns adc2mev factor for the given ID and gain
      //SHOULD NOT BE USED ANYMORE, prefer the one above with caloDDE

   private: 
  
    std::string   m_ReturnNoiseName;
 
 
    //Constants
    static constexpr int m_nCalos=4;// number of calorimeters
    static constexpr int m_nGains=3;// number of gains
    float m_LowGainThresh[m_nCalos]{};
    float m_HighGainThresh[m_nCalos]{};
    CaloGain::CaloGain  m_highestGain[m_nCalos]{};
 
    bool m_diagnostic[m_nGains]{};
    int m_nCellsWithProblem[m_nGains]{};
    int m_nReason[5000][m_nGains]{};
    int m_itReason[10][m_nGains]{};
    int m_idHash[5000][m_nGains]{};
    int m_reason[5000][10][m_nGains]{};
    std::string m_reasonName[10];
 
    //Identifiers
    const AtlasDetectorID* m_atlas_id{};
 
    const CaloIdManager* m_calo_id_man{};
 
    const LArEM_Base_ID*    m_lar_em_id{};
    const LArHEC_Base_ID*   m_lar_hec_id{};
    const LArFCAL_Base_ID*  m_lar_fcal_id{};
     
 
    const CaloCell_ID* m_calocell_id{};
    const CaloCell_SuperCell_ID* m_calosupercell_id{};
 
    IdentifierHash m_LArHashMax;
    IdentifierHash m_TileHashMax;
    IdentifierHash m_CaloHashMax;
    IdentifierHash m_CaloHashMin;
 
 
 
    // Properties
    IntegerProperty m_deltaBunch{this, "deltaBunch", 1};
    UnsignedIntegerProperty m_firstSample{this, "firstSample", 0};
    BooleanProperty  m_UseSymmetry{this, "UseSymmetry", true};
    BooleanProperty  m_DiagnosticHG{this, "DiagnosticHG", false};
    BooleanProperty  m_DiagnosticMG{this, "DiagnosticMG", false};
    BooleanProperty  m_DiagnosticLG{this, "DiagnosticLG", false};
    BooleanProperty  m_DumpDatabaseHG{this, "DumpDatabaseHG", false};
    BooleanProperty  m_DumpDatabaseMG{this, "DumpDatabaseMG", false};
    BooleanProperty  m_DumpDatabaseLG{this, "DumpDatabaseLG", false};
    FloatProperty m_Nminbias{this, "NMinBias", -1};
  
 
    //Database  
 
    float  m_Adc2MeVFactor{};
    LArVectorProxy m_OFC;
    LArVectorProxy m_Shape;
    LArVectorProxy m_AutoCorr;
    float m_c[32][32]{};
    float m_RMSpedestal{};
    int m_nsamples{};
    float  m_SigmaNoise{};
    float m_fSampl{};
    double m_AdcPerMev{};
    float m_MinBiasRMS{};

    SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey {this,"CaloDetDescrManager", "CaloDetDescrManager", "SG Key for CaloDetDescrManager in the Condition Store" };
    const CaloDetDescrManager* m_calo_dd_man=nullptr; 

    SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
    const LArOnOffIdMapping *m_cabling=nullptr;
 
    SG::ReadCondHandleKey<LArADC2MeV> m_adc2mevKey{this, "ADC2MeVKey", "LArADC2MeV", "SG Key of the LArADC2MeV CDO" };
  
    SG::ReadCondHandleKey<ILArPedestal> m_pedestalKey{this,"PedestalKey","LArPedestal","SG Key of LArPedestal object"};
    const ILArPedestal *m_ped=nullptr;
 
    SG::ReadCondHandleKey<ILArAutoCorr> m_acorrKey{this,"AutocorrKey","LArAutoCorrSym","SG Key of LArAutoCorr object"};
    const ILArAutoCorr *m_acorr=nullptr;
 
    SG::ReadCondHandleKey<ILArOFC> m_LArOFCObjKey{this, "OFKey","LArOFC", "SG Key of LAr OFCs"};
    const ILArOFC *m_ofccond=nullptr;
     
    SG::ReadCondHandleKey<ILArShape> m_shapeKey{this,"ShapeKey","LArShapeSym","SG Key of Shape conditions object"};
    const ILArShape *m_shapecond=nullptr;
 
    SG::ReadCondHandleKey<ILArfSampl> m_fSamplKey{this,"fSamplKey","LArfSamplSym","SG Key of LArfSampl object"};
    const ILArfSampl *m_fsamplcond = nullptr;
 
    SG::ReadCondHandleKey<ILArMinBias> m_LArMinBiasObjKey{this, "LArMinBiasKey", "LArMinBiasSym", "SG Key of LArMinBias"};
    const ILArMinBias *m_minbias = nullptr;
 
    // Output containers
    SG::WriteCondHandleKey<CaloNoise> m_outputElecKey{this, "OutputElecKey", "elecNoise", "SG Key of resulting noise CDO"};
    SG::WriteCondHandleKey<CaloNoise> m_outputPileupKey{this, "OutputPileupKey", "pileupNoise", "SG Key of resulting noise CDO"};

 
    //Containers    
    std::vector<IdentifierHash>  m_indexContainer;
      // = vector indexed with all the hashids, 
      //containing which index (of container) should be used with a hashid
    std::vector<IdentifierHash>  m_idSymmCaloHashContainer;
      //used only to initialize m_indexOfContainer 
 
 
    VectorContainer              m_elecNoiseRAWContainer;
    VectorContainer              m_elecNoiseCELLContainer;
    SingleContainer              m_pileupNoiseContainer;
    VectorContainer              m_adc2mevContainer;
 
 
    //Functions
    StatusCode initContainers();
    StatusCode initData(const LArADC2MeV *adc2mev);
    StatusCode initIndex();
    StatusCode initAdc2MeV(const LArADC2MeV *adc2mev);
    StatusCode initElecNoise();
    StatusCode initPileUpNoise();
 
    bool checkIfConnected(const Identifier &id);
    void commonCalculations(float & OFC_AC_OFC,float & OFC_OFC, int icase, unsigned int firstSample=0);
    StatusCode retrieveCellDatabase(const IdentifierHash & idCaloHash,
          			  const Identifier & id,int igain,
                                  std::vector<bool> &retrieve);
    StatusCode checkCellDatabase(const Identifier & id, int igain,
                               std::vector<bool> &retrieve);
    void updateDiagnostic(int reason, const std::string &reason_name,int igain, bool &noiseOK);
 
    std::vector<float> 
      calculateElecNoiseForLAR(const IdentifierHash &idCaloHash);
 
    float
      calculatePileUpNoise(const IdentifierHash &idCaloHash,
          		 const float &Nminbias);
 
    int  index(const IdentifierHash &idCaloHash);  
    CaloCell_ID::SUBCALO caloNum(const IdentifierHash idCaloHash);  
    bool isBadValue(float tested_value); 
 
    CaloGain::CaloGain estimatedLArGain(const CaloCell_ID::SUBCALO &iCalo,
          			      const CaloDetDescrElement* caloDDE,
          			      const float &energy);



};

//------------------------------------------------------------

inline bool 
CaloNoiseCompCondAlg::isBadValue(float tested_value) 
{
  if(tested_value<BADVALUE+1) return true;
  return false;
}

//////////////////////////////////////////////////

inline CaloCell_ID::SUBCALO  
CaloNoiseCompCondAlg::caloNum(const IdentifierHash idCaloHash)
{
  return 
    static_cast<CaloCell_ID::SUBCALO>(m_calocell_id->sub_calo(idCaloHash));
}

#endif

