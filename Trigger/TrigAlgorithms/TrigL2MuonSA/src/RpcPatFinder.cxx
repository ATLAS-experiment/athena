/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcPatFinder.h"

#include <GaudiKernel/IMessageSvc.h>
#include <array>
#include <cstddef>
#include <functional>
#include <math.h>
#include <bitset>
#include <iostream>

#include "AthenaBaseComps/AthMsgStreamMacros.h"

// Original author: Massimo Corradi

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

void TrigL2MuonSA::RpcPatFinder::addHit(const std::string& stationName,
					int stationEta,
					bool  measuresPhi,
					unsigned  int gasGap,
					unsigned  int doubletR,
					double gPosX,
					double gPosY,
					double gPosZ,
                                        TrigL2MuonSA::RpcLayerHits& rpcLayerHits ) const{
  
  int ilay=0;
  // BO 
  if (stationName.substr(0,2)=="BO") ilay=4;
  // doubletR
  ilay+=2*(doubletR-1);
  // BML7 special chamber with 1 RPC doublet (doubletR=1 but RPC2) :
  if (stationName.substr(0,3)=="BML" && stationEta==7) ilay+=2;
  // gasGap
  ilay+=gasGap-1;

  double R=std::sqrt(gPosX*gPosX+gPosY*gPosY);
  const double Phi=std::atan2(gPosY,gPosX);

  if (!measuresPhi){
    // if eta measurement then save Z/R
    R = calibR(stationName,R, Phi);  
    double x=gPosZ/R;
    rpcLayerHits.hits_in_layer_eta.at(ilay).push_back(x);
    rpcLayerHits.hits_in_layer_R.at(ilay).push_back(R);//mod!
    rpcLayerHits.hits_in_layer_Z.at(ilay).push_back(gPosZ);//mod!
   }else{
    // if phi measurement then save phi
    rpcLayerHits.hits_in_layer_phi.at(ilay).push_back(Phi);
  }
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

bool TrigL2MuonSA::RpcPatFinder::findPatternEta(
            std::array<std::reference_wrapper<double>, 3>& result_aw, 
            std::array<std::reference_wrapper<double>, 3>& result_bw, 
            const TrigL2MuonSA::RpcLayerHits& rpcLayerHits) const{
  
    const std::vector<std::vector<double>>&  rpc_x {rpcLayerHits.hits_in_layer_eta};

    int  layer_end {5};
    if(rpc_x.at(6).size()+rpc_x.at(7).size() >0) layer_end = 7;//special "feet" towers

    // reset parameters
    std::bitset<8> result_pat{};
    double result_dMM{9999}, result_dMO{9999};
    int nHits_pat{0};
    std::array<size_t, 8> result_index{};

    // Loop on start layer
    for (int l_start=0; l_start<layer_end; l_start++){
        // Loop on hits of start layer, for each hit try a new pattern
        for (size_t i_start = 0; i_start < rpc_x.at(l_start).size(); ++i_start){

            // Initialize a new pattern
            int nHits=1;
            std::bitset<8> pat (1<<l_start); // bit pattern of hit layers
            std::array<size_t, 8> index {};
            index[l_start] = i_start;
            double dMO{9999}; // lowest deltaX between two consecutive hits, when having at least one of the two hit in BO
            double dMM{9999}; // lowest deltaX between two consecutive hits, when having both hits in BM, or both in BO (but different doublets)

            int current_l = l_start;
            double current_x = rpc_x.at(l_start).at(i_start); // set current_x to the starting hit

            // ----- add compatible hits in other layers ----//
            // loop on test layers:
            for (int l_test=l_start+1; l_test<=layer_end; l_test++){
                double min_delta {999};   // min deltaX in this test laeyr
                const std::vector<double>& test_layer_hits {rpc_x.at(l_test)};

                for (size_t i_test = 0; i_test < test_layer_hits.size(); ++i_test){
                    double delta=-1;
                    // check if within the road
                    if (deltaOK(current_l,l_test,current_x, test_layer_hits.at(i_test),false,delta)){
                        // if closest hit we keep it as best hit for this test layer
                        if (delta < min_delta) {
                            min_delta = delta;
                            index[l_test] = i_test;
                        }
                    }
                }
                if (min_delta < 998){   //we found at least one hit in the window
                    current_l = l_test;	
                    current_x = test_layer_hits.at(index[l_test]);
                    nHits+=1;
                    pat.set(l_test);
                    dMO = (l_start<4 and l_test>=4) ? std::min(dMO, min_delta) : dMO;
                    dMM = (l_start<2 and l_test>=2 and l_test<4) or (l_start>=4 and l_start<5 and l_test>=6) ? std::min(dMM, min_delta) : dMM;
                }
            }//for l_test

            // if longest pattern found, update result
            if (nHits>nHits_pat) { 
                nHits_pat=nHits;
                result_pat=pat;
                result_dMO=dMO; 
                result_dMM=dMM; 
                result_index=index;
            }else if (nHits==nHits_pat) { 
                // if same lenght but smallest dMM/dMO, update result
                if (dMM<result_dMM or (dMM==result_dMM and dMO<result_dMO)){
                    result_pat=pat;
                    result_dMO=dMO;
                    result_dMM=dMM;
                    result_index=index;
                }
            }
        }//for i_start
    }//for l_start

    if (nHits_pat>=2) {
        abcal(result_pat, result_index, result_aw, result_bw, rpcLayerHits);
        if(msgLevel(MSG::VERBOSE)){
            std::ostringstream ossR, ossZ;
            bool isFirst{true};
            for (int i=0; i<8; ++i){
                if(result_pat.test(i)){
                    if (isFirst){
                        ossR << rpcLayerHits.hits_in_layer_R.at(i).at(result_index[i]);
                        ossZ << rpcLayerHits.hits_in_layer_Z.at(i).at(result_index[i]);
                        isFirst=false;
                    }else{
                        ossR << "," << rpcLayerHits.hits_in_layer_R.at(i).at(result_index[i]);
                        ossZ << "," << rpcLayerHits.hits_in_layer_Z.at(i).at(result_index[i]);
                    }
                }
            }
            std::ostringstream oss;
            std::copy(result_index.begin(), result_index.end(), std::ostream_iterator<int>(oss, " "));
            ATH_MSG_VERBOSE("patfinder: BEST pat= " << result_pat << " nHit: " << nHits_pat << " Idx: " << oss.str()
                <<"  dMM= "<<result_dMM <<"  dMO= "<<result_dMO << " R_hits: " << ossR.str() << " Z_hits: " << ossZ.str()
                <<" Slopes: " << result_aw[0] << "," << result_aw[1] << "," << result_aw[2] << " Offsets: " << result_bw[0] << "," << result_bw[1] << "," << result_bw[2]);
        }
        return true;
    }
    return false;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
bool TrigL2MuonSA::RpcPatFinder::findPatternPhi(double &phi_middle, 
                                                double &phi_outer, 
                                                const TrigL2MuonSA::RpcLayerHits& rpcLayerHits) const{
  const int N_layers=8;

  const std::vector<std::vector<double>>& rpc_phi {rpcLayerHits.hits_in_layer_phi};
  
  int l_start_max=2; //max layer of first hit
  if (rpc_phi.at(6).size()+rpc_phi.at(7).size()>0) l_start_max=5; // special "feet" towers

  // reset parameters
  phi_middle=0;
  phi_outer=0;
  double result_dMM{9999}, result_dMO{9999};
  int nHits_pat=0;

  // Loop on start layer
  for (int l_start=0; l_start<=l_start_max; l_start++){
    // Loop on hits of start layer, for each hit try a new pattern
    for (const double& phi_start : rpc_phi.at(l_start)){
        // Initialize a new pattern
        int nHits=1;
        double dMO{9999}; // lowest deltaX between two consecutive hits, when having at least one of the two hit in BO
        double dMM{9999}; // lowest deltaX between two consecutive hits, when having both hits in BM, or both in BO (but different doublets)

        int current_l = l_start;
        double current_phi = phi_start; // set current_x to the starting hit

        // ----- add compatible hits in other layers ----//
            // loop on test layers:
            for (int l_test=l_start+1; l_test<N_layers; l_test++){
                double min_delta {999};   // min deltaX in this test laeyr
                double layer_phi {0.};

                for (const double& phi_test : rpc_phi.at(l_test)){
                    double delta=-1;
                    // check if within the road
                    if (deltaOK(current_l,l_test,current_phi, phi_test,true,delta)){
                        // if closest hit we keep it as best hit for this test layer
                        if (delta < min_delta) {
                            min_delta = delta;
                            layer_phi = phi_test;
                        }
                    }
                }
                if (min_delta < 998){   //we found at least one hit in the window
                    current_l = l_test;	
                    current_phi = layer_phi;
                    nHits+=1;
                    dMO = (l_start<4 and l_test>=4) ? std::min(dMO, min_delta) : dMO;
                    dMM = (l_start<2 and l_test>=2 and l_test<4) or (l_start>=4 and l_start<5 and l_test>=6) ? std::min(dMM, min_delta) : dMM;
                }
            }

            // if longest pattern found and the last hit is in a layer > BM doublet1, update result
            if (nHits>nHits_pat and current_l > 1) { 
                nHits_pat=nHits;
                result_dMO=dMO; 
                result_dMM=dMM; 
                phi_middle=phi_start;
                phi_outer=current_phi;
            }else if (nHits==nHits_pat and current_l > 1) { 
                // if same lenght but smallest dMM/dMO, update result
                if (dMM<result_dMM or (dMM==result_dMM and dMO<result_dMO)){
                    result_dMO=dMO;
                    result_dMM=dMM;
                    phi_middle=phi_start;
                    phi_outer=current_phi;
                }
            }
        }//for i_start
    }//for l_start
	
    if (nHits_pat>2) {
        ATH_MSG_DEBUG("patfinder: BEST phi path dMM= "<<result_dMM <<"  dMO= "<<result_dMO 
                <<"  phi_middle= "<<phi_middle <<"  phi_outer= "<<phi_outer);
        return true;
    }
    return false;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

bool  TrigL2MuonSA::RpcPatFinder::deltaOK(int l1, int l2, double x1, double x2, int isphi, double &delta) const{
  
  
  // ROAD tuned for ~20 GeV 
  /*
  double delta_gasgap_eta = 0.004;
  double delta_lowpt_eta = 0.005;
  double delta_highpt_eta = 0.012;
  double delta_feet_eta = 0.02;

  double delta_gasgap_phi = 0.004;
  double delta_lowpt_phi = 0.005;
  double delta_highpt_phi = 0.008;
  double delta_feet_phi = 0.02;
  */

  //OPEN road 

  const double delta_gasgap_eta = 0.01;
  const double delta_lowpt_eta = 0.05;
  const double delta_highpt_eta = 0.1;
  const double delta_feet_eta = 0.05;

  const double delta_gasgap_phi = 0.01;
  const double delta_lowpt_phi = 0.03;
  const double delta_highpt_phi = 0.04;
  const double delta_feet_phi = 0.03;

  // calculate delta-eta or delta-phi 
  if(isphi) delta=std::abs(std::acos(std::cos(x2-x1)));
  else delta=std::abs(x2-x1); 

  double delta_max=0;
  if (l1>l2) {
    int tmp=l2;
    l2=l1;
    l1=tmp;
  }
  // calculate delta_max
  if (isphi){
    if (l2-l1==1&&(l1==0||l1==2||l1==4||l1==6)){
      delta_max=delta_gasgap_phi;
    } else if (l1<2&&l2<4) {
      delta_max=delta_lowpt_phi;
    }else if (l1<4&&l2>=4) {
      delta_max=delta_highpt_phi;
    }else if (l1<6&&l1>=4&&l2>=6) {
      delta_max=delta_feet_phi;
    }
  } else {
    if (l2-l1==1&&(l1==0||l1==2||l1==4||l1==6)){
      delta_max=delta_gasgap_eta;
    } else if (l1<2&&l2>=2&&l2<4) {
      delta_max=delta_lowpt_eta;
    }else if (l1<4&&l2>=4) {
      delta_max=delta_highpt_eta;
    }else if (l1<6&&l1>=4&&l2>=6) {
      delta_max=delta_feet_eta;
    }
  }

  // evaluate the result

  bool OK=false;
  if (delta<delta_max) OK=true;

  return OK;
  
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

double TrigL2MuonSA::RpcPatFinder::calibR(const std::string& stationName, double R, double Phi) const{
  double DeltaPhi, temp_phi;
  double calibPhi = std::acos(std::cos(Phi)); // 0 < Phi < 2PI
  
  if(std::string::npos != stationName.rfind('L')){//For Large , SP
    DeltaPhi= 999; temp_phi=9999;
    for(int inum=0;inum < 8;inum++){
      temp_phi = std::abs((inum * M_PI/4.0 )- calibPhi);
      if(temp_phi < DeltaPhi)      DeltaPhi = temp_phi;
    }
  }else if(std::string::npos != stationName.rfind('S') ||
	   std::string::npos != stationName.rfind('F') ||
	   std::string::npos != stationName.rfind('G')   ){
    DeltaPhi= 999; temp_phi=9999;

    for(int inum=0;inum < 8;inum++){
      temp_phi = std::abs(inum *(M_PI/4.0 )+(M_PI/8.0) - calibPhi);
      if(temp_phi < DeltaPhi)      DeltaPhi = temp_phi;
    }//for end
  }else return R;

  double calibR = R *std::cos(DeltaPhi);
  
  return calibR;
}//calbR()

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

void TrigL2MuonSA::RpcPatFinder::abcal(const std::bitset<8>& result_pat, 
                                       const std::array<size_t, 8>& index, 
                                       std::array<std::reference_wrapper<double>, 3>& aw, 
                                       std::array<std::reference_wrapper<double>, 3>& bw, 
                                       const TrigL2MuonSA::RpcLayerHits& rpcLayerHits) const{
    const float ZERO_LIMIT = 1.e-5;
    const std::vector<std::vector<double>>& rpc_R {rpcLayerHits.hits_in_layer_R};
    const std::vector<std::vector<double>>& rpc_Z {rpcLayerHits.hits_in_layer_Z};

    // doublet companion                                    
    auto getCompanion = [](const int& l) -> int {
        return (l%2==0) ? l+1 : l-1;
    };

    auto getAvgRZ= [&rpc_R, &rpc_Z, &result_pat, &index]
                    (double& R, double& Z, const int& l, const int& companion) -> void {
    
        if (result_pat.test(companion)){
            R = (rpc_R.at(l).at(index[l]) + rpc_R.at(companion).at(index[companion])) / 2.0;
            Z = (rpc_Z.at(l).at(index[l]) + rpc_Z.at(companion).at(index[companion])) / 2.0;
            return;
        }
        R = rpc_R.at(l).at(index[l]);
        Z = rpc_Z.at(l).at(index[l]);
    };

    /// Calculate inner and middle coefficients
    int l1 {4}, l2 {0};
    double R1{0}, R2{0}, Z1{0}, Z2{0};
    for(int i=0; i<4; i++){
        if(!result_pat.test(i)) continue;
        l1 = std::min(l1, i);
        l2 = std::max(l2, i);
    }
    const int comp1{getCompanion(l1)}, comp2{getCompanion(l2)};
    if (l1 != 4 and l1 != l2 and l2 != comp1){// we have two hits in BM and not in the same doublet 
        
        getAvgRZ(R1, Z1, l1, comp1);
        getAvgRZ(R2, Z2, l2, comp2);

        ///Inner
        if(((result_pat & std::bitset<8>("00001111")).count() > 3) and std::abs(Z2-Z1) > ZERO_LIMIT){

            double theta_m {std::atan2(R1, Z1)};
            double theta_t {std::atan2(R2-R1, Z2-Z1)};
            double theta_f {(theta_m+theta_t)/2.};

            aw[0].get() = std::tan(theta_f);
            bw[0].get() = R1 - Z1*aw[0].get();

        }else{
            aw[0].get() = R1/Z1;
            bw[0].get() = 0.;
        }
        
        ///Middle
        if (std::abs(Z2-Z1) > ZERO_LIMIT){
            aw[1].get() = (R2-R1)/(Z2-Z1);
            bw[1].get() = R2 - Z2*aw[1].get();
        }
        else{// if hits have very close z, we use only the earlier hit
            aw[1].get() = R1/Z1;
            bw[1].get() = 0.;
        }
    }
    else if (l1 != 4){// either we have two hits in the same doublet or only one hit
        getAvgRZ(R1, Z1, l1, comp1);
        aw[0].get() = R1/Z1;
        bw[0].get() = 0.;
        aw[1].get() = aw[0].get();
        bw[1].get() = 0.;

    }// if no hits in BM, we will extrapolate the inner and BM slope using the first hit in BO

    /// Calculate outer coefficients
    int l3 = 8, l4 = 0;
    double R3{0}, R4{0}, Z3{0}, Z4{0};
    for(int i=2; i<8; i++){
        if(!result_pat.test(i)) continue;
        l3 = std::min(l3, i);
        l4 = std::max(l4, i);
    }
    const int comp3{getCompanion(l3)}, comp4{getCompanion(l4)};
    if ( l3 != 8 and l3 != l4 and l4 != comp3){

        getAvgRZ(R3, Z3, l3, comp3);
        getAvgRZ(R4, Z4, l4, comp4);

        if (std::abs(Z4-Z3) > ZERO_LIMIT){
            aw[2].get() = (R4-R3)/(Z4-Z3);
            bw[2].get() = R4 - Z4*aw[2].get();
        }
        else{// hits with very close Z, we use onl one hit.
            aw[2].get() = R4/Z4;
            bw[2].get() = 0.;
        }
    }
    else if (l3 != 8){ // we have only one hit or they are in the same doublet
        getAvgRZ(R3, Z3, l3, comp3);
        aw[2].get() = R3/Z3;
        bw[2].get() = 0.;
    }// if no hits in outer layers, we will extrapolate the outer slope using the last hit in BM

    if (std::abs(aw[0].get()) < ZERO_LIMIT){    // if no hits in BM, we use the first in in BO
        getAvgRZ(R3, Z3, l3, comp3);
        aw[0].get() =  R3/Z3;
        aw[1].get() = aw[0].get();
    }
    if (std::abs(aw[2].get()) < ZERO_LIMIT){    // if no hits in outer layers, we use the last in BM
        getAvgRZ(R2, Z2, l2, comp2);
        aw[2].get() = R2/Z2;
    }
                                    

}//abcal()

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

