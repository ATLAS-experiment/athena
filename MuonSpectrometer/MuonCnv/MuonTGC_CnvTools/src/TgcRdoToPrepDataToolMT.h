/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CNVTOOLS_TGCRDOTOPREPDATATOOLMT_H
#define MUONTGC_CNVTOOLS_TGCRDOTOPREPDATATOOLMT_H

#include "MuonCnvToolInterfaces/IMuonRdoToPrepDataTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"

#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonRDO/TgcRdo.h"
#include "MuonTrigCoinData/TgcCoinDataContainer.h"
#include "MuonPrepRawData/MuonPrepDataCollection_Cache.h"
#include "MuonTrigCoinData/MuonTrigCoinData_Cache.h"
#include "MuonRDO/TgcRdoContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonTGC_Cabling/MuonTGC_CablingSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/HandleKeyArray.h"
#include "StoreGate/UpdateHandleKey.h"
#include "StoreGate/UpdateHandle.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "CxxUtils/CachedValue.h"
#include "xAODMuonPrepData/TgcStripContainer.h"

#include <string>
#include <vector>
#include <atomic>

namespace MuonGM 
{
  class TgcReadoutElement; 
}

namespace Muon 
{
  // Typedef the two update handle arrays that can be used to match handle key functionality
  // Requested to not use StoreGate template for UpdateHandleKeyArray
  typedef SG::HandleKeyArray<SG::UpdateHandle<TgcPrepDataCollection_Cache>, SG::UpdateHandleKey<TgcPrepDataCollection_Cache>, Gaudi::DataHandle::Reader > TgcPrdUpdateHandles;
  typedef SG::HandleKeyArray<SG::UpdateHandle<TgcCoinDataCollection_Cache>, SG::UpdateHandleKey<TgcCoinDataCollection_Cache>, Gaudi::DataHandle::Reader > TgcCoinUpdateHandles;

  /** @class TgcRdoToPrepDataToolMT 
   *  This is the algorithm that convert TGCRdo To TGCPrepdata as a tool.
   * 
   * @author Susumu Oda <Susumu.Oda@cern.ch> 
   * @author  Edward Moyse 
   * 
   * This class was developed by Takashi Kubota. 
   */  

  class TgcRdoToPrepDataToolMT : public extends<AthAlgTool, IMuonRdoToPrepDataTool>
    {
    public:
      /** Constructor */
      TgcRdoToPrepDataToolMT(const std::string& t, const std::string& n, const IInterface* p);
      
      /** Destructor */
      virtual ~TgcRdoToPrepDataToolMT()=default;
      
      /** Standard AthAlgTool initialize method */
      virtual StatusCode initialize() override;
      /** Standard AthAlgTool finalize method */
      virtual StatusCode finalize() override;
      
      /** Decode RDO to PRD  
       *  A vector of IdentifierHash are passed in, and the data corresponding to this list (i.e. in a Region of Interest) are converted.  
       *  @param requestedIdHashVect          Vector of hashes to convert i.e. the hashes of ROD collections in a 'Region of Interest'  
       *  @return selectedIdHashVect This is the subset of requestedIdVect which were actually found to contain data   
       *  (i.e. if you want you can use this vector of hashes to optimise the retrieval of data in subsequent steps.) */ 
      virtual StatusCode decode(const EventContext& ctx,
                                const std::vector<IdentifierHash>& idVect) const override;

      virtual StatusCode provideEmptyContainer(const EventContext& ctx) const override;
    private:
      /** The number of recorded Bunch Crossings (BCs) FOR HITS is 3 (Previous, Current, and Next BCs) */
      static constexpr int NBC_HIT = 3;
      /* Run1,2: The number of recorded Bunch Crossings (BCs) FOR TRIGGER (HPT/SL) is 3 (Previous, Current, Next BCs) */
      /* Run3: The number of recorded Bunch Crossings (BCs) FOR TRIGGER (HPT/SL) is 4 (Previous, Current, Next, and Next-to-Next BCs) */
      static constexpr int NBC_TRIG = 4;

      struct State {
        /** TgcPrepRawData (hit PRD) containers */
        std::array<TgcPrepDataContainer*, NBC_HIT +1> tgcPrepDataContainer{};   // +1 for AllBCs
        
        using TempPrepDataContainer = std::vector<std::unique_ptr<TgcPrepDataCollection>>;
        std::array<TempPrepDataContainer, NBC_HIT +1> tgcPrepDataCollections{};
        /** TgcCoinData (coincidence PRD) containers */ 
        std::array<TgcCoinDataContainer*, NBC_TRIG> tgcCoinDataContainer{};

        using TempCoinDataContainer = std::vector<std::unique_ptr<TgcCoinDataCollection>>;
        std::array<TempCoinDataContainer, NBC_TRIG> tgcCoinDataCollections{};

        /// Handle for the xAOD container 
        SG::WriteHandle<xAOD::TgcStripContainer> m_xaodHandle{};

        const MuonGM::MuonDetectorManager* muDetMgr{nullptr};
      };
      template<class ContType, class CollType> StatusCode transferData(ContType& container,
                                                                       std::vector<std::unique_ptr<CollType>>&& coll) const;

      StatusCode setupState(const EventContext& ctx, State& state) const;

      struct CablingInfo {
        ServiceHandle<MuonTGC_CablingSvc> m_tgcCabling{"MuonTGC_CablingSvc", "TgcRdoToPrepDataToolMT"};
        /** Conversion from hash to onlineId */  
        std::vector<uint16_t> m_hashToOnlineId;
        int m_MAX_N_ROD = 0;
      };
      CxxUtils::CachedValue<CablingInfo> m_cablingInfo;

      /** Sub detector IDs are 103 and 104 for TGC A side and C side, respectively */
      enum SUB_DETCTOR_ID {
        ASIDE = 103,
        CSIDE = 104
      };
      
      /** SLB bit position 
      /code
      Large R   <--> Small R
      Large phi <--> Small phi for A side forward chambers and C side backward chambers
      Small phi <--> Large phi for A side backward chambers and C side forward chambers
      A-input :  40 -  75,  42 -  73 are valid.
      B-input :  76 - 111,  78 - 109 are valid.
      C-input : 112 - 155, 118 - 149 are valid. 
      D-input : 156 - 199, 162 - 193 are valid. 

      Channel in this code : Large R                  15  14  13  12  11 ...   0  15 ...   3   2   1   0                 Small R
                                                           (it is better to be reverted to avoid confusion)
          ASD channel order    :  15 ... ...   0 (there are shifts dependent on position)
      PS board channel     :                           0   1   2   3   4 ...  15  16 ...  28  29  30  31 
      A-Input              :                  40  41  42  43  44  45  46 ...  57  58 ...  70  71  72  73  74  75
      B-Input              :                  76  77  78  79  80  81  82 ...  93  94 ... 106 107 108 109 110 111
      C-Input              : 112 113 114 115 116 117 118 119 120 121 122 ... 133 134 ... 146 147 148 149 150 151 152 153 154 155
      D-Input              : 156 157 158 159 160 161 162 163 164 165 166 ... 177 178 ... 190 191 192 193 194 195 196 197 198 199
      /endcode
      - https://twiki.cern.ch/twiki/bin/view/Main/TgcDocument
      - https://twiki.cern.ch/twiki/pub/Main/TgcDocument/celladdress2_asic_rev2.pdf
      - https://twiki.cern.ch/twiki/pub/Main/TgcDocument/psboard_table_v070119.xls
      - https://twiki.cern.ch/twiki/pub/Main/TgcDocument/psboard_table_v070119.pdf
      */
      enum BIT_POS {
    BIT_POS_ASD_SIZE = 16,
    BIT_POS_NUM_ASD = 2,
    BIT_POS_INPUT_SIZE = BIT_POS_ASD_SIZE*BIT_POS_NUM_ASD,
    BIT_POS_OFFSET_LARGE_R = BIT_POS_ASD_SIZE,
    BIT_POS_OFFSET_LARGE_PHIFOR_A_FWD_C_BWD = BIT_POS_OFFSET_LARGE_R,
    BIT_POS_A_INPUT_ORIGIN =  73,
    BIT_POS_B_INPUT_ORIGIN = 109,
    BIT_POS_C_INPUT_ORIGIN = 149,
    BIT_POS_D_INPUT_ORIGIN = 193,
    
    /* CHannel starts from 00 at the small R side (the same as ASD). */
    BIT_POS_B_INPUT_LARGE_R_CH15 = BIT_POS_B_INPUT_ORIGIN - BIT_POS_OFFSET_LARGE_R - 15, //  78
    BIT_POS_A_INPUT_LARGE_R_CH08 = BIT_POS_A_INPUT_ORIGIN - BIT_POS_OFFSET_LARGE_R -  8, //  49
    BIT_POS_B_INPUT_LARGE_R_CH07 = BIT_POS_B_INPUT_ORIGIN - BIT_POS_OFFSET_LARGE_R -  7, //  86
    BIT_POS_A_INPUT_LARGE_R_CH00 = BIT_POS_A_INPUT_ORIGIN - BIT_POS_OFFSET_LARGE_R,      //  57
    BIT_POS_B_INPUT_SMALL_R_CH15 = BIT_POS_B_INPUT_ORIGIN                          - 15, //  94
    BIT_POS_A_INPUT_SMALL_R_CH08 = BIT_POS_A_INPUT_ORIGIN                          -  8, //  65
    BIT_POS_B_INPUT_SMALL_R_CH07 = BIT_POS_B_INPUT_ORIGIN                          -  7, // 102
    BIT_POS_A_INPUT_SMALL_R_CH00 = BIT_POS_A_INPUT_ORIGIN,                               //  73

    BIT_POS_B_INPUT_SMALL_R_CH05 = BIT_POS_B_INPUT_ORIGIN                          -  5, // 104
    BIT_POS_A_INPUT_SMALL_R_CH03 = BIT_POS_A_INPUT_ORIGIN                          -  3, //  70
    BIT_POS_B_INPUT_LARGE_R_CH12 = BIT_POS_B_INPUT_ORIGIN - BIT_POS_OFFSET_LARGE_R - 12, //  81

    BIT_POS_A_INPUT_SMALL_R_CH04 = BIT_POS_A_INPUT_ORIGIN                          -  4, //  69
    BIT_POS_A_INPUT_LARGE_R_CH12 = BIT_POS_A_INPUT_ORIGIN - BIT_POS_OFFSET_LARGE_R - 12, //  45
    BIT_POS_A_INPUT_LARGE_R_CH04 = BIT_POS_A_INPUT_ORIGIN - BIT_POS_OFFSET_LARGE_R -  4, //  53
    BIT_POS_A_INPUT_SMALL_R_CH12 = BIT_POS_A_INPUT_ORIGIN                          - 12, //  61

    /* CHannel starts from 00 
       at the small phi side for A side forward chambers and C side backward chambers and 
       at the large phi side for A side backward chambers and C side forward chambers (the same as ASD). */
    BIT_POS_B_INPUT_LARGE_PHI_FOR_A_FWD_C_BWD_CH15 = BIT_POS_B_INPUT_LARGE_R_CH15,   //  78
    BIT_POS_A_INPUT_LARGE_PHI_FOR_A_FWD_C_BWD_CH08 = BIT_POS_A_INPUT_LARGE_R_CH08,   //  49
    BIT_POS_B_INPUT_LARGE_PHI_FOR_A_FWD_C_BWD_CH07 = BIT_POS_B_INPUT_LARGE_R_CH07,   //  86
    BIT_POS_A_INPUT_LARGE_PHI_FOR_A_FWD_C_BWD_CH00 = BIT_POS_A_INPUT_LARGE_R_CH00,   //  57
    BIT_POS_B_INPUT_SMALL_PHI_FOR_A_FWD_C_BWD_CH15 = BIT_POS_B_INPUT_SMALL_R_CH15,   //  94
    BIT_POS_A_INPUT_SMALL_PHI_FOR_A_FWD_C_BWD_CH08 = BIT_POS_A_INPUT_SMALL_R_CH08,   //  65
    BIT_POS_B_INPUT_SMALL_PHI_FOR_A_FWD_C_BWD_CH07 = BIT_POS_B_INPUT_SMALL_R_CH07,   // 102
    BIT_POS_A_INPUT_SMALL_PHI_FOR_A_FWD_C_BWD_CH00 = BIT_POS_A_INPUT_SMALL_R_CH00,   //  73

    BIT_POS_A_INPUT_LARGE_PHI_FOR_A_FWD_C_BWD_CH12 = BIT_POS_A_INPUT_LARGE_R_CH12,   //  45
    BIT_POS_A_INPUT_LARGE_PHI_FOR_A_FWD_C_BWD_CH04 = BIT_POS_A_INPUT_LARGE_R_CH04,   //  53
    BIT_POS_A_INPUT_SMALL_PHI_FOR_A_FWD_C_BWD_CH12 = BIT_POS_A_INPUT_SMALL_R_CH12,   //  61
    BIT_POS_A_INPUT_SMALL_PHI_FOR_A_FWD_C_BWD_CH04 = BIT_POS_A_INPUT_SMALL_R_CH04    //  69
      };
      
      /** Bit map sizes */
      enum MAP_SIZE {
        WT_MAP_SIZE = 3*BIT_POS_INPUT_SIZE,
        ST_MAP_SIZE = 2*BIT_POS_INPUT_SIZE,
        SD_MAP_SIZE = 2*BIT_POS_INPUT_SIZE,
        WD_MAP_SIZE = 2*BIT_POS_INPUT_SIZE
      };

    
      /** Select decoder based on RDO type (Hit or Coincidence (Tracklet, HiPt and SL)) */
      void selectDecoder(State& state, const TgcRawData& rd, const TgcRdo* rdoColl) const;

      /** Decode RDO's of Hit */
      StatusCode decodeHits(State& state, const TgcRawData& rd) const;
      /** Decode RDO's of Tracklet */
      StatusCode decodeTracklet(State& state, const TgcRawData& rd) const;
      /** Decode RDO's of Tracklet EIFI */
      StatusCode decodeTrackletEIFI(State& state, const TgcRawData& rd) const;
      /** Decode RDO's of HiPt */
      StatusCode decodeHiPt(State& state, const TgcRawData& rd) const;
      /** Decode RDO's of Inner */
      StatusCode decodeInner(State& state, const TgcRawData& rd) const;
      /** Decode RDO's of SectorLogic */
      StatusCode decodeSL(State& state, const TgcRawData& rd, const TgcRdo* rdoColl) const;
      
      /** Get bitpos from channel and SlbType */
      static int getbitpos(int channel, TgcRawData::SlbType slbType) ;
      /** Get channel from bitpos and SlbType */
      static int getchannel(int bitpos, TgcRawData::SlbType slbType) ;

      /** Get r from eta and z */
      static bool getRfromEtaZ(const double eta, const double z, double& r) ;
      /** Get eta from r and z */
      static bool getEtafromRZ(const double r, const double z, double& eta) ;
      
      /** Check the rdo is already converted or not */
      static bool isAlreadyConverted(const std::vector<const TgcRdo*>& decodedRdoCollVec,
                                    const std::vector<const TgcRdo*>& rdoCollVec,
                                    const TgcRdo* rdoColl) ;
      
      /** Check the IdHash is already requested or not */
      static bool isRequested(const std::vector<IdentifierHash>& requestedIdHashVect,
                       IdentifierHash tgcHashId) ;
      
      /** Check offline ID is OK for TgcReadoutElement */
      bool isOfflineIdOKForTgcReadoutElement(const MuonGM::TgcReadoutElement* descriptor, const Identifier channelId) const;
      
      /** Retrieve slbId, subMatrix and position from Tracklet RDO */
      bool getTrackletInfo(const TgcRawData& rd,
                           int& tmp_slbId, int& tmp_subMatrix, int& tmp_position) const;
      
      /** Get ROI row from RDO */
      static int getRoiRow(const TgcRawData& rd) ;
      /** Check SL RDO is at the chamber boundary */
      bool isIncludedInChamberBoundary(const TgcRawData& rd) const;
      
      /** Get bitPos etc of TGC3 wire for HiPt */
      static void getBitPosOutWire(const TgcRawData& rd, int& slbsubMatrix, std::array<int, 2>& bitpos_o) ;
      /** Get bitPos etc of TGC1 wire for HiPt */
      void getBitPosInWire(const TgcRawData& rd, 
                           const int DeltaBeforeConvert,
                           std::array<int, 4>& bitpos_i, 
                           std::array<int, 4>& slbchannel_i, 
                           std::array<int, 4>& slbId_in, 
                           std::array<int, 4>& sbLoc_in, 
                           int& sswId_i,
                           const std::array<int, 2>& bitpos_o, 
                           std::array<int, 2>& slbchannel_o, 
                           const int slbId_o) const;
      /** Get bitPos etc of TGC3 strip for HiPt */
      static void getBitPosOutStrip(const TgcRawData& rd, int& slbsubMatrix, std::array<int, 2>& bitpos_o) ;
      /** Get bitPos etc of TGC1 strip for HiPt */
      void getBitPosInStrip(const TgcRawData& rd, 
                            const int DeltaBeforeConvert,
                            std::array<int, 4>& bitpos_i, 
                            std::array<int, 4>& slbchannel_i, 
                            int& sbLoc_i, 
                            int& sswId_i,
                            const std::array<int, 2>& bitpos_o, 
                            std::array<int, 2>& slbchannel_o) const;
      /** Get bitPos etc of wire for SL */
      void getBitPosWire(const TgcRawData& rd, 
                         const int hitId_w, 
                         const int sub_w, 
                         int& subMatrix_w, 
                         std::array<int, 3>& bitpos_w) const;
      /** Get bitPos etc of strip for SL */
      static void getBitPosStrip(const int hitId_s, 
                                 const int sub_s, 
                                 int& subMatrix_s, 
                                 std::array<int, 3>& bitpos_s) ;
      
      /** Get delta (sagitta) before converion for HiPt */
      static int getDeltaBeforeConvert(const TgcRawData& rd) ;
      
      /** Check if a chamber in BigWheel is a backward chamber or a forward chamber */
      static bool isBackwardBW(const TgcRawData& rd) ;
      
      /** Get wire geometry (width, r, z) for SL */ 
      bool getSLWireGeometry(const std::array<Identifier, 3>& channelId_wire, 
                             double& width_wire, 
                             double& r_wire, 
                             double& z_wire) const;
      /** Get strip geometry (width, theta) for SL */ 
      bool getSLStripGeometry(const std::array<Identifier, 3>& channelId_strip, 
                              const bool isBackWard, 
                              const bool isAside, 
                              double& width_strip, 
                              double& theta_strip) const;
      /** Get position and offline ID of TGC3 wire for HiPt */ 
      bool getPosAndIdWireOut(const std::array<const MuonGM::TgcReadoutElement*, 2>& descriptor_o, 
                              const std::array<Identifier, 2>& channelIdOut,
                              const std::array<int, 2>& gasGap_o, 
                              const std::array<int, 2>& channel_o,
                              double& width_o, 
                              double& hit_position_o, 
                              Amg::Vector2D& tmp_hitPos_o,
                              Identifier& channelIdOut_tmp) const;
      /** Get position and offline ID of TGC3 strip for HiPt */ 
      bool getPosAndIdStripOut(const std::array<const MuonGM::TgcReadoutElement*, 2>& descriptor_o, 
                               const std::array<Identifier,2 >& channelIdOut,
                               const std::array<int, 2>& gasGap_o, 
                               const std::array<int, 2>& channel_o,
                               double& width_o, 
                               double& hit_position_o, 
                               Amg::Vector2D& tmp_hitPos_o,
                               Identifier& channelIdOut_tmp,
                               const bool isBackward, const bool isAside) const;
     
      /** Get position and offline ID of TGC1 wire for HiPt */ 
      bool getPosAndIdWireIn(const std::array<const MuonGM::TgcReadoutElement*, 4>& descriptor_i, 
                             const std::array<Identifier,4 >& channelIdIn,
                             const std::array<int, 4>& gasGap_i, 
                             const std::array<int, 4>& channel_i,
                             double& width_i, double& hit_position_i, 
                             Amg::Vector2D& tmp_hitPos_i,
                             Identifier& channelIdIn_tmp) const;
      /** Get position and offline ID of TGC1 strip for HiPt */ 
      bool getPosAndIdStripIn(const std::array<const MuonGM::TgcReadoutElement*, 4>& descriptor_i, 
                              const std::array<Identifier, 4>& channelIdIn,
                              const std::array<int, 4>& gasGap_i, 
                              const std::array<int, 4>& channel_i,
                              double& width_i, double& hit_position_i, 
                              Amg::Vector2D& tmp_hitPos_i,
                              Identifier& channelIdIn_tmp,
                              const bool isBackward, const bool isAside) const;
      
      /** Get ReadoutID of HiPt from RDOHighPtID */  
      bool getHiPtIds(const TgcRawData& rd, int& sswId_o, int& sbLoc_o, int& slbId_o) const;

      /** Get ReadoutID of SL from RDO */  
      bool getSLIds(const bool isStrip, 
                    const TgcRawData& rd, 
                    std::array<Identifier, 3>& channelId, 
                    int& index, int& chip, int& hitId, int& sub, int& sswId, int& sbLoc, int& subMatrix, 
                    std::array<int, 3>& bitpos, 
                    const bool isBoundary=false, const TgcRdo* rdoColl=0, 
                    const int index_w=-1, const int chip_w=-1, const int hitId_w=-1, const int sub_w=-1) const;
      /** Get strip sbLoc of Endcap chamber boundary from HiPt Strip */
      bool getSbLocOfEndcapStripBoundaryFromHiPt(const TgcRawData& rd,
                                                 int& sbLoc,
                         const TgcRdo* rdoColl, 
                         const int index_w, const int chip_w, const int hitId_w, const int sub_w) const;
      /** Get strip sbLoc of Endcap chamber boundary from Tracklet Strip */
      bool getSbLocOfEndcapStripBoundaryFromTracklet(const TgcRawData& rd,
                                                     int& sbLoc,
                             const TgcRdo* rdoColl, 
                             const int index_w, const int chip_w, const int hitId_w, const int sub_w) const;
      /** Get trackletIds of three Tracklet Strip candidates in the Endcap boudary */ 
      static void getEndcapStripCandidateTrackletIds(const int roi, int &trackletIdStripFirst, 
                          int &trackletIdStripSecond, int &trackletIdStripThird) ;

      const CablingInfo*  getCabling() const;

      /** Get SL local position */
      static const Amg::Vector2D* getSLLocalPosition(const MuonGM::TgcReadoutElement* readout, const Identifier, const double eta, const double phi) ; 

      SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_muDetMgrKey {this, "DetectorManagerKey", "MuonDetectorManager", "Key of input MuonDetectorManager condition data"}; 

      ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
      
      /** TgcPrepRawData container key for current BC */ 
      Gaudi::Property<std::string> m_outputCollectionLocation{this, "OutputCollection", "TGC_Measurements"};      
      
      /** TgcCoinData container key for current BC */ 
      Gaudi::Property<std::string> m_outputCoinCollectionLocation{this, "OutputCoinCollection", "TrigT1CoinDataCollection"};
      
      /** Identifier hash offset */
      Gaudi::Property<int> m_tgcOffset{this, "TGCHashIdOffset", 26000};

      /** Switch for the decoding of TGC RDO into TgcPrepData */
      Gaudi::Property<bool> m_decodeData{this, "DecodeData", true}; 
      /** Switch for the coincince decoding */
      Gaudi::Property<bool> m_fillCoinData{this, "FillCoinData", true}; 
      
      /** Switch for error message disabling on one invalid channel in 
      sector A09 seen in 2008 data, at least run 79772 - 91800. 
      bug #48828: TgcRdoToTgcDigit WARNING ElementID not found for 
      sub=103 rod=9 ssw=6 slb=20 bitpos=151 +offset=0 orFlag=0
      */
      Gaudi::Property<bool> m_show_warning_level_invalid_A09_SSW6_hit{this, "show_warning_level_invalid_A09_SSW6_hit", false};

      /** Flag for dropping PRD's with zero widths */
      Gaudi::Property<bool> m_dropPrdsWithZeroWidth{this, "dropPrdsWithZeroWidth", true};
      /** Cut value for zero widths */ 
      const static double s_cutDropPrdsWithZeroWidth; // 0.1 mm

      /** long to count the numbers of RDOs and PRDs */
      mutable std::atomic<long> m_nHitRDOs{0};
      mutable std::atomic<long> m_nHitPRDs{0};
      mutable std::atomic<long> m_nTrackletRDOs{0}; 
      mutable std::atomic<long> m_nTrackletPRDs{0}; 
      mutable std::atomic<long> m_nTrackletEIFIRDOs{0}; 
      mutable std::atomic<long> m_nTrackletEIFIPRDs{0}; 
      mutable std::atomic<long> m_nHiPtRDOs{0}; 
      mutable std::atomic<long> m_nHiPtPRDs{0}; 
      mutable std::atomic<long> m_nSLRDOs{0};  
      mutable std::atomic<long> m_nSLPRDs{0}; 

      SG::ReadHandleKey<TgcRdoContainer> m_rdoContainerKey{this, "RDOContainer", "TGCRDO" ,"TgcRdoContainer to retrieve"};

      // Write handle keys for CoinDataContainers, need 3, for current, previous and next BC
      SG::WriteHandleKeyArray<Muon::TgcCoinDataContainer> m_outputCoinKeys{this, "outputCoinKey", {}};
      // Write handle keys for PrepDataContainers, need 4, for current, previous, next and all BC
      SG::WriteHandleKeyArray<Muon::TgcPrepDataContainer> m_outputprepdataKeys{this, "prepDataKeys", {}};

      SG::WriteHandleKey<xAOD::TgcStripContainer>   m_xAODKey  {this, "xAODKey", "", "If empty, do not produce xAOD, otherwise this is the key of the output xAOD MDT PRD container"};


      /// Keys for the PRD cache containers, 4 needed for different BC
      TgcPrdUpdateHandles m_prdContainerCacheKeys{this, "UpdateKeysPrd", {}};
      /// Keys for the Coin cache containers, 3 needed for different BC
      TgcCoinUpdateHandles m_coinContainerCacheKeys{this, "UpdateKeysCoin", {}};

      // TgcPrepRawData container cache key prefix (code automatically creates three keys with from this string if it is non-empty)
      Gaudi::Property<std::string> m_prdContainerCacheKeyStr{this, "PrdCacheString", "", "Prefix for names of PRD cache collections"};
      // TgcCoinData container cache key prefix (code automatically creates three keys with from this string if it is non-empty)
      Gaudi::Property<std::string> m_coinContainerCacheKeyStr{this, "CoinCacheString", "", "Prefix for names of Coin cache collections"};

      /** Avoid compiler warning **/
      StatusCode decode(const EventContext& ctx, const std::vector<uint32_t>& robIds) const override; 
      // Run3->Run2 conversion of rodId and sector
      void convertToRun2(const TgcRawData* rd, uint16_t& newrodId, uint16_t& newsector) const {
        newrodId = rd->rodId();
        newsector = rd->sector();
        if (rd->rodId()>12){ // Run3 rodID is 17..19 while Run2 rodId is 1..12
          if(rd->isForward()){
            newrodId = rd->sector() / 2 + 1 + (rd->rodId()-17) * 4;
            newsector = (rd->sector() + (rd->rodId()-17)*8) % 2;
          }else{
            newrodId = rd->sector() / 4 + 1 + (rd->rodId()-17) * 4;
            newsector = (rd->sector() + (rd->rodId()-17)*16) % 4;
          }
        }
      }
      void convertToRun2(const TgcRawData& rd, uint16_t& newrodId, uint16_t& newsector) const {
    convertToRun2(&rd,newrodId,newsector);
      }

   }; 
} // end of namespace

#endif // MUONTGC_CNVTOOLS_TGCRDOTOPREPDATATOOLMT_H
