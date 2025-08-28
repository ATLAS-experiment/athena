/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXegAlgo  -  description
//                              -------------------
//     begin                : 24 02 2020
//     email                : antonio.jacques.costa@cern.ch ulla.blumenschein@cern.ch tong.qiu@cern.ch
//  ***************************************************************************/
#include <vector>
#include <mutex>

#include "L1CaloFEXSim/eFEXegAlgo.h"
#include "L1CaloFEXSim/eFEXegTOB.h"
#include "L1CaloFEXSim/eTowerContainer.h"
#include "L1CaloFEXSim/eTower.h"


namespace LVL1 {

    // default constructor for persistency
    eFEXegAlgo::eFEXegAlgo(const std::string& type, const std::string& name, const IInterface* parent):
            AthAlgTool(type, name, parent)
    {
        declareInterface<eFEXegAlgo>(this);
    }

    /** Destructor */
    eFEXegAlgo::~eFEXegAlgo()
    {
    }

    StatusCode eFEXegAlgo::initialize(){

        ATH_CHECK(m_eTowerContainerKey.initialize());
        ATH_CHECK( m_dmCorrectionsKey.initialize(SG::AllowEmpty) );

        return StatusCode::SUCCESS;
    }


    StatusCode eFEXegAlgo::safetyTest() const {

        SG::ReadHandle<eTowerContainer> eTowerContainer(m_eTowerContainerKey/*,ctx*/);
        if(!eTowerContainer.isValid()){
            ATH_MSG_FATAL("Could not retrieve container " << m_eTowerContainerKey.key() );
            return StatusCode::FAILURE;
        }

        return StatusCode::SUCCESS;
    }

    void eFEXegAlgo::setup(int inputTable[3][3], int efex_id, int fpga_id, int central_eta) {

        std::copy(&inputTable[0][0], &inputTable[0][0] + 9, &m_eFEXegAlgoTowerID[0][0]);

        m_efexid = efex_id;
        m_fpgaid = fpga_id;
        m_central_eta = central_eta;

        setSeed();

    }

    void LVL1::eFEXegAlgo::getCoreEMTowerET(unsigned int & et) {

        SG::ReadHandle<eTowerContainer> eTowerContainer(m_eTowerContainerKey/*,ctx*/);

        const LVL1::eTower * tmpTower = eTowerContainer->findTower(m_eFEXegAlgoTowerID[1][1]);
        et = tmpTower->getLayerTotalET(0) + tmpTower->getLayerTotalET(1) + tmpTower->getLayerTotalET(2) + tmpTower->getLayerTotalET(3);
    }

    void LVL1::eFEXegAlgo::getCoreHADTowerET(unsigned int & et) {

        SG::ReadHandle<eTowerContainer> eTowerContainer(m_eTowerContainerKey/*,ctx*/);

        const LVL1::eTower * tmpTower = eTowerContainer->findTower(m_eFEXegAlgoTowerID[1][1]);
        et = tmpTower->getLayerTotalET(4);
    }

    void LVL1::eFEXegAlgo::getRealPhi(float & phi) {

        SG::ReadHandle<eTowerContainer> eTowerContainer(m_eTowerContainerKey/*,ctx*/);
        phi = eTowerContainer->findTower(m_eFEXegAlgoTowerID[1][1])->phi();

    }

    void LVL1::eFEXegAlgo::getRealEta(float & eta) {

        SG::ReadHandle<eTowerContainer> eTowerContainer(m_eTowerContainerKey/*,ctx*/);

        eta = eTowerContainer->findTower(m_eFEXegAlgoTowerID[1][1])->eta() * eTowerContainer->findTower(m_eFEXegAlgoTowerID[1][1])->getPosNeg();

    }

    void eFEXegAlgo::getReta(std::vector<unsigned int> & retavec) {

        unsigned int coresum  = 0;   // 3x2 L2 sum : core
        unsigned int totalsum = 0;   // 7x3 L2 sum : total
        unsigned int envsum   = 0;   // total - core : env

        retavec.clear();  // clear the output vector before starting

        // window limits
        int iTotalStart = m_seedID-3;
        int iTotalEnd   = m_seedID+3;
        int iCoreStart  = m_seedID-1;
        int iCoreEnd    = m_seedID+1;
        int phiStart    = -999;
        int phiEnd      = -99;
        if (m_seed_UnD) {
            phiStart = 1;
            phiEnd = 2;
        } else {
            phiStart = 0;
            phiEnd = 1;
        }

        // 3x2 and 7x3 L2 sum
        for (int i=iTotalStart; i<=iTotalEnd; ++i) { // eta
            for(int j=0; j<=2; ++j) { // phi
                if (i>=iCoreStart && i <= iCoreEnd && j>=phiStart && j<=phiEnd) {
                    unsigned int tmp_et; getWindowET(2,j,i,tmp_et);
                    coresum += tmp_et;
                }

                unsigned int tmptot_et; getWindowET(2,j,i,tmptot_et);
                totalsum += tmptot_et;
            }
        }

        // get environment
        envsum = totalsum - coresum;

        // Overflow handling
        if (coresum > 0xffff) coresum = 0xffff;
        if (envsum > 0xffff)   envsum = 0xffff;

        // Return results
        retavec.push_back(coresum);
        retavec.push_back(envsum);

    }

    void eFEXegAlgo::getRhad(std::vector<unsigned int> & rhadvec) {

        unsigned int hadsum = 0; // 3x3 Towers Had
        unsigned int emsum = 0;  // (1x3 + 3x3 + 3x3 + 1x3) SCs EM

        rhadvec.clear();   // clear the output vector before starting

        int iCoreStart  = m_seedID-1;
        int iCoreEnd    = m_seedID+1;

        SG::ReadHandle<eTowerContainer> eTowerContainer(m_eTowerContainerKey/*,ctx*/);

        if(m_algoVersion==0) {
            // 3x3 Towers Had ; 1x3 L0 + 1x3 L3 EM
            for (int i=0; i<3; ++i) { // phi
                for (int j=0; j<=2; ++j) { // eta
                    if (((m_efexid%3 == 0) && (m_fpgaid == 0) && (m_central_eta == 0) && (j == 0)) || ((m_efexid%3 == 2) && (m_fpgaid == 3) && (m_central_eta == 5) && (j == 2))) {
                        continue;
                    } else {
                        const eTower * tTower = eTowerContainer->findTower(m_eFEXegAlgoTowerID[i][j]);
                        hadsum += tTower->getLayerTotalET(4);
                        if (j==1) {
                            emsum += ( tTower->getLayerTotalET(0) + tTower->getLayerTotalET(3) );
                        }
                    }
                }
            }

            // 3x3 SCs L1 and L2 sum
            for (int i=iCoreStart; i<=iCoreEnd; ++i) { // eta
                for(int j=0; j<=2; ++j) { // phi
                    unsigned int tmp_et_a, tmp_et_b;
                    getWindowET(1,j,i,tmp_et_a);
                    getWindowET(2,j,i,tmp_et_b);
                    emsum += ( tmp_et_a + tmp_et_b );
                }
            }
        } else {

            // see slide 4 of https://indico.cern.ch/event/1513502/contributions/6389265/attachments/3019474/5326755/Rate%20Reduction%20Methods%20Overview.pdf
            // Update 29-Apr-2025: removing 4 corners of EM3 from sum

            // EM cluster depends on UnD flag
            int phi2 = (m_seed_UnD > 0 ? 2 : 0);

            // 3x3 Towers Had + EM3 (excluding corners in EM3); 1x2 L0
            for (int i = 0; i < 3; ++i) { // phi
                for (int j = 0; j <= 2; ++j) { // eta
                    if (((m_efexid % 3 == 0) && (m_fpgaid == 0) && (m_central_eta == 0) && (j == 0)) ||
                        ((m_efexid % 3 == 2) && (m_fpgaid == 3) && (m_central_eta == 5) && (j == 2))) {
                        continue;
                    } else {
                        const eTower *tTower = eTowerContainer->findTower(m_eFEXegAlgoTowerID[i][j]);
                        hadsum += tTower->getLayerTotalET(4) + (((i==0&&j==0)||(i==2&&j==2)||(i==0&&j==2)||(i==2&&j==0)) ? 0 : tTower->getLayerTotalET(3));
                        // For PS add central tower + UnD phi neighbour
                        if (j == 1 && (i == 1 || i == phi2)) {
                            emsum += (tTower->getLayerTotalET(0));
                        }
                    }
                }
            }

            // 3x2 SCs L1 and L2 sum, so phi range depends on UnD flag
            int phiStart = 0;
            int phiEnd = 1;
            if (m_seed_UnD > 0) {
                phiStart = 1;
                phiEnd = 2;
            }
            for (int i = iCoreStart; i <= iCoreEnd; ++i) { // eta
                for (int j = phiStart; j <= phiEnd; ++j) { // phi
                    unsigned int tmp_et_a, tmp_et_b;
                    getWindowET(1, j, i, tmp_et_a);
                    getWindowET(2, j, i, tmp_et_b);
                    emsum += (tmp_et_a + tmp_et_b);
                }
            }
        }
        // Overflow handling
        if (emsum > 0xffff)   emsum = 0xffff;
        if (hadsum > 0xffff) hadsum = 0xffff;

        // Return results
        rhadvec.push_back(emsum);
        rhadvec.push_back(hadsum);

    }

    void LVL1::eFEXegAlgo::getWstot(std::vector<unsigned int> & output){
        unsigned int numer = 0;
        unsigned int den = 0;

        output.clear(); // clear the output vector before starting

        int iStart = m_seedID - 2;
        int iEnd = m_seedID + 2;

        for (int i = iStart; i <= iEnd; ++i) { // eta
            int diff = i - m_seedID;
            unsigned int weight = diff*diff;
            for (int j = 0; j <= 2; ++j) { // phi
                unsigned int eT;
                getWindowET(1, j, i, eT);
                // NB need to be careful as numer and den are defined such that wstot=numer/den,
                // but in the firmware (and therefore this bitwise code) we actually
                // check that den/numer < Threshold
                numer += eT*weight;
                den += eT;
            }
        }

        // Overflow handling
        //if (den > 0xffff)     den = 0xffff; - commented out so that denom can overflow, will then automatically pass all thresholds (see eFEXFPGA::SetIsoWP)
        if (numer > 0xffff) numer = 0xffff;

        // Return results
        output.push_back(den);
        output.push_back(numer);

    }

/// Return cell ET values used in cluster.
/// Placed in its own function to allow other classes to access these
    void LVL1::eFEXegAlgo::getClusterCells(std::vector<unsigned int> &cellETs) {

        int phiUpDownID = 0;
        if (m_seed_UnD) phiUpDownID = 2;

        // Initialise results vector
        cellETs.resize(16,0);
        // Fill vector with 2 PS cells, 6 L1, 6 L2, 2 L3
        // Presampler
        getWindowET(0, 1, 0, cellETs[0]);
        getWindowET(0, phiUpDownID, 0, cellETs[1]);
        // central phi Layer 1
        getWindowET(1, 1, m_seedID,     cellETs[2]);
        getWindowET(1, 1, m_seedID - 1, cellETs[3]);
        getWindowET(1, 1, m_seedID + 1, cellETs[4]);
        // top/bottom phi Layer 1
        getWindowET(1, phiUpDownID, m_seedID,     cellETs[5]);
        getWindowET(1, phiUpDownID, m_seedID - 1, cellETs[6]);
        getWindowET(1, phiUpDownID, m_seedID + 1, cellETs[7]);
        // central phi Layer 2
        getWindowET(2, 1, m_seedID,     cellETs[8]);
        getWindowET(2, 1, m_seedID - 1, cellETs[9]);
        getWindowET(2, 1, m_seedID + 1, cellETs[10]);
        // top/bottom phi Layer 2
        getWindowET(2, phiUpDownID, m_seedID,     cellETs[11]);
        getWindowET(2, phiUpDownID, m_seedID - 1, cellETs[12]);
        getWindowET(2, phiUpDownID, m_seedID + 1, cellETs[13]);
        // Layer 3
        getWindowET(3, 1, 0,           cellETs[14]);
        getWindowET(3, phiUpDownID, 0, cellETs[15]);

        return;

    }

    unsigned int LVL1::eFEXegAlgo::getET() {

        /// Get cells used in cluster
        std::vector<unsigned int> clusterCells;
        getClusterCells(clusterCells);


        /// Layer sums including dead material corrections
        unsigned int PS_ET = 0;

        if(m_algoVersion==0) {
            PS_ET = dmCorrection(clusterCells[0], 0)
                    + dmCorrection(clusterCells[1], 0);
        } else {
            // 2025 algoVersion only uses 1 PS scell, except at most extreme eta values
            PS_ET = dmCorrection(clusterCells[0], 0);
            if ( ((m_efexid%3) == 0 && m_fpgaid == 0) || ((m_efexid%3) == 2 && m_fpgaid == 3)) {
                PS_ET += dmCorrection(clusterCells[1], 0);
            }
        }
        unsigned int L1_ET = dmCorrection(clusterCells[2], 1)
                             + dmCorrection(clusterCells[3], 1)
                             + dmCorrection(clusterCells[4], 1)
                             + dmCorrection(clusterCells[5], 1)
                             + dmCorrection(clusterCells[6], 1)
                             + dmCorrection(clusterCells[7], 1);
        unsigned int L2_ET = dmCorrection(clusterCells[8], 2)
                             + dmCorrection(clusterCells[9], 2)
                             + dmCorrection(clusterCells[10], 2)
                             + dmCorrection(clusterCells[11], 2)
                             + dmCorrection(clusterCells[12], 2)
                             + dmCorrection(clusterCells[13], 2);
        unsigned int L3_ET = clusterCells[14] + clusterCells[15];

        /// Final ET sum
        unsigned int totET = PS_ET + L1_ET + L2_ET + L3_ET;

        // overflow handling
        if (totET > 0xffff) totET = 0xffff;

        return totET;

    }


    LVL1::eFEXegAlgo::Corrections::Corrections (const CondAttrListCollection& dmCorrections,
                                                MsgStream& msg)
    {
      for (const auto& p : dmCorrections) {
        if (p.first < 25 || p.first >= 50) continue;
        m_corrections[0][p.first - 25] = p.second["EmPS"].data<int>();
        m_corrections[1][p.first - 25] = p.second["EmFR"].data<int>();
        m_corrections[2][p.first - 25] = p.second["EmMD"].data<int>();
        if (msg.level() <= MSG::DEBUG) {
          msg << MSG::DEBUG << "DM Correction for etaIdx=" << (p.first - 25) << " : [" << m_corrections[0][p.first - 25] << ","
              << m_corrections[1][p.first - 25] << "," << m_corrections[2][p.first - 25] << "]"
              << endmsg;
        }
      }
    }

    unsigned int LVL1::eFEXegAlgo::dmCorrection (unsigned int ET, unsigned int layer) {
        /// Check corrections are required and layer is valid, otherwise do nothing
        if ( !m_dmCorr || layer > 2 ) return ET;

        /// Get correction factor
        /// Start by calculating RoI |eta| with range 0-24
        int efexEta = m_efexid%3;
        int ieta = 0;
        if (efexEta == 2) {  // Rightmost eFEX
            // m_central_eta has range 1-4 or 1-5
            ieta = 8 + m_fpgaid*4 + m_central_eta - 1;
        }
        else if (efexEta == 1 && m_fpgaid > 1) { // central eFEX, eta > 0
            // m_central_eta has range 1-4
            ieta = (m_fpgaid-2)*4 + m_central_eta - 1;
        }
        else if (efexEta == 1) { // central eFEX, eta < 0
            // m_central_eta had range 1-4
            ieta = (1-m_fpgaid)*4 + (4-m_central_eta);
        }
        else {   // Leftmost eFEX
            // m_central_eta has range 0-4 or 1-4
            ieta = 8 + 4*(3-m_fpgaid) + (4-m_central_eta);
        }

	static std::once_flag flag;
	std::call_once(flag, [&]() {

            if (!m_dmCorrectionsKey.empty()) {
                // replace m_corrections values with values from database ... only try this once
                SG::ReadCondHandle <CondAttrListCollection> dmCorrections{m_dmCorrectionsKey/*, ctx*/ };
                if (dmCorrections.isValid()) {
                    if(dmCorrections->size()==0 && Gaudi::Hive::currentContext().eventID().time_stamp()>1672527600) { // not an error for data before 2023 (will include MC21 and MC23a)
                        ATH_MSG_ERROR("No dead material corrections found in conditions database for this event in folder " << m_dmCorrectionsKey.key());
                        throw std::runtime_error("No dead material corrections found in database for this event");
                    }
                    m_corrections = Corrections (*dmCorrections.cptr(), msg());
                }
		ATH_MSG_INFO("Loaded DM Corrections from database");
            }
        });

        /// Retrieve the factor from table (eventually from DB)
        unsigned int factor = m_corrections.corr(layer, ieta);

        /** Calculate correction
            Factors are 7 bit words, highest bit corresponding to the most significant
             term in the correction (ET/2).
             So we'll work backwards from the top bit (bit 6) to implement it */

        unsigned int correction = ET;
        for (int bit = 6; bit >= 0; bit--) {
            correction /= 2;
            if (factor & (1<<bit))
                ET += correction;
        }
        /// And this should now be the corrected ET
        return ET;
    }


    std::unique_ptr<eFEXegTOB> LVL1::eFEXegAlgo::geteFEXegTOB() {

        std::unique_ptr<eFEXegTOB> out = std::make_unique<eFEXegTOB>();
        out->setET(getET());

        std::vector<unsigned int> temvector;
        getWstot(temvector);
        // For wstot, num and den seem switched around, but this is because the 'numerator' and 'denominator'
        // mean different things at different points in the processing chain
        // When the threshold comparison is done in the SetIsoWP function, we actually check Den/Num
        out->setWstotNum(temvector[1]);
        out->setWstotDen(temvector[0]);
        getRhad(temvector);
        out->setRhadEM(temvector[0]);
        out->setRhadHad(temvector[1]);
        getReta(temvector);
        out->setRetaCore(temvector[0]);
        out->setRetaEnv(temvector[1]);
        out->setSeedUnD(m_seed_UnD);
        out->setSeed(m_seedID);
        return out;
    }

    void LVL1::eFEXegAlgo::getWindowET(int layer, int jPhi, int SCID, unsigned int & outET) {

        SG::ReadHandle<eTowerContainer> eTowerContainer(m_eTowerContainerKey/*,ctx*/);

        if (SCID<0) { // left towers in eta
            if ((m_efexid%3 == 0) && (m_fpgaid == 0) && (m_central_eta == 0)) {
                outET = 0;
            } else {
                int etaID = 4+SCID;
                const eTower * tmpTower = eTowerContainer->findTower(m_eFEXegAlgoTowerID[jPhi][0]);
                if (layer==1 || layer==2) {
                    outET = tmpTower->getET(layer,etaID);
                } else if (layer==0 || layer==3 || layer==4) {
                    outET = tmpTower->getLayerTotalET(layer);
                }
            }
        } else if (SCID>=0 && SCID<4) { // central towers in eta
            const eTower * tmpTower = eTowerContainer->findTower(m_eFEXegAlgoTowerID[jPhi][1]);
            if (layer==1 || layer==2) {
                outET = tmpTower->getET(layer,SCID);
            } else if (layer==0 || layer==3 || layer==4) {
                outET = tmpTower->getLayerTotalET(layer);
            }
        } else if (SCID>=4){ // right towers in eta
            if ((m_efexid%3 == 2) && (m_fpgaid == 3) && (m_central_eta == 5)) {
                outET = 0;
            } else {
                int etaID = SCID-4;
                const eTower * tmpTower = eTowerContainer->findTower(m_eFEXegAlgoTowerID[jPhi][2]);
                if (layer==1 || layer==2) {
                    outET = tmpTower->getET(layer,etaID);
                } else if (layer==0 || layer==3 || layer==4) {
                    outET = tmpTower->getLayerTotalET(layer);
                }
            }
        }

        // overflow handling
        if (outET > 0xffff) outET = 0xffff;

    }


// Utility function to calculate and return jet discriminant sums for specified location
// Intended to allow xAOD TOBs to be decorated with this information
    void eFEXegAlgo::getSums(unsigned int seed, bool UnD,
                             std::vector<unsigned int> & RetaSums,
                             std::vector<unsigned int> & RhadSums,
                             std::vector<unsigned int> & WstotSums)
    {
        // Set seed parameters to supplied values
        m_seed_UnD = UnD;
        m_seedID = seed;
        m_hasSeed = true;

        // Now just call the 3 discriminant calculation methods
        getReta(RetaSums);
        getRhad(RhadSums);
        getWstot(WstotSums);

    }


// Find seed and UnD flag  
    void eFEXegAlgo::setSeed() {

        m_hasSeed = false;
        m_seed_UnD = false;
        unsigned int tmpID = 999;
        unsigned int maxET = 0;

        for (int i=0; i<4 ; ++i) {
            int iSeedL = i-1;
            int iSeedR = i+1;

            // eta ID of candidate seed
            unsigned int cETUp;
            getWindowET(2,2,i,cETUp);
            unsigned int iSeedET;
            getWindowET(2,1,i, iSeedET);
            unsigned int cETDown;
            getWindowET(2,0,i, cETDown);

            // left of candidate seed
            unsigned int lETUp;
            getWindowET(2,2,iSeedL,lETUp);
            unsigned int lET;
            getWindowET(2,1,iSeedL,lET);
            unsigned int lETDown;
            getWindowET(2,0,iSeedL,lETDown);

            // right of candidate seed
            unsigned int rETUp;
            getWindowET(2,2,iSeedR,rETUp);
            unsigned int rET;
            getWindowET(2,1,iSeedR,rET);
            unsigned int rETDown;
            getWindowET(2,0,iSeedR,rETDown);

            // greater or equal than for left and down cells, greater than for right and up ones
            if (iSeedET>=lET && iSeedET>rET
                && iSeedET>=lETUp    && iSeedET>cETUp    && iSeedET>rETUp
                && iSeedET>=lETDown && iSeedET>=cETDown && iSeedET>rETDown) {
                if (iSeedET>=maxET) { // if two maxima exist and have the same ET, keep the one to the right
                    maxET = iSeedET;
                    tmpID = i;
                }
            }
        }

        if(tmpID!=999) {
            m_seedID = tmpID;
            m_hasSeed = true;
            unsigned int tmp_et_up, tmp_et_down;
            getWindowET(2,2,m_seedID,tmp_et_up);
            getWindowET(2,0,m_seedID,tmp_et_down);
            if (tmp_et_up >= tmp_et_down) {
                m_seed_UnD = true; // go up if energy greater or equal to bottom
            }
        }
    }

} // namespace LVL1
