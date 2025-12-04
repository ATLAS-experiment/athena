/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***********************************************************************************************
//             PixelCalibration - Program to run the pixel charge calibration (not IBL)
//                           ---------------------------------------
//     begin                : 01 11 2023
//     email                : sergi.rodriguez@cern.ch
//***********************************************************************************************


#include "Calib.h"

bool Calib::totFitting    (const pix::PixelMapping &pm, const std::string &inTotFile, std::map<unsigned int , std::vector<std::unique_ptr<CalibFrontEndInfo>> > &map_info){
    
    if (inTotFile.empty()) return false;    
    
    TFile totFile(inTotFile.c_str(),"READ");
    if (not totFile.IsOpen()) {
        printf("Error - File %s could not be opened.\n",inTotFile.c_str());
        totFile.Close();
        return false;
    } else {
        printf("File %s opened.\n",inTotFile.c_str());
        printf("Running TOT calibration...\n");
    }   
    
    // Creating here the histograms with this scope ownership
    std::vector< std::vector< std::unique_ptr<TH1F> > > histogramsTOT;
    std::vector< std::vector< std::unique_ptr<TH1F> > > histogramsTOTsig;
    for(unsigned int FE = 0; FE < m_nFE; FE++){
        
        histogramsTOT.push_back(    std::vector< std::unique_ptr<TH1F> >() );
        histogramsTOTsig.push_back( std::vector< std::unique_ptr<TH1F> >() );
        
        //Here we combine long and ganged pixels 
        for(unsigned int pixel=0; pixel<2; pixel++){
            TString title = "FE"+std::to_string(FE)+"_pixType"+std::to_string(pixel);
            std::unique_ptr<TH1F> htot = std::make_unique<TH1F>(title+"_tot", title+"_tot", m_totnbins, m_totLo, m_totHi);
            htot->SetDirectory(0);
            std::unique_ptr<TH1F> htotsig = std::make_unique<TH1F>(title+"_totsig", title+"_totsig", m_totsigNBins, m_totsigLo, m_totsigHi);
            htotsig->SetDirectory(0);
            //cppcheck-suppress containerOutOfBounds
            histogramsTOT.at(FE).push_back(std::move(htot));
            //cppcheck-suppress containerOutOfBounds
            histogramsTOTsig.at(FE).push_back(std::move(htotsig));
        }
    }     
      
    // Start looping over the ROD, MOD and charges
    TIter rodItr=getRodIterator(totFile);
    TKey* rodKey;

    while ((rodKey=static_cast<TKey*>(rodItr()))) {
        TDirectoryFile* rodDir = static_cast<TDirectoryFile*>(rodKey->ReadObj());
        TIter modItr = getModuleIterator(rodDir);
        const TString rodName(rodKey->GetName());
        printf("%s\n",rodName.Data());
        TKey* modKey;
        while ((modKey=static_cast<TKey*>(modItr()))) {
            const TString modName(modKey->GetName());
            
            if (not moduleInPart(modName)) continue;
            if (not pm.contains(std::string(modName))) continue;
            
            if( m_runOneMOD and strcmp(modName, m_testMOD) != 0){
                continue;
            }    
            
            printf("  -> %s\n",modName.Data());
            
            //creates arrays for the Tgraph
            std::array<std::array<float, m_ncharge>, m_nFE> totArrI{};
            std::array<std::array<float, m_ncharge>, m_nFE> totErrArrI{};
            std::array<std::array<float, m_ncharge>, m_nFE> totSigArrI{};
            std::array<std::array<float, m_ncharge>, m_nFE> totSigErrArrI{};
            std::array<std::array<float, m_ncharge>, m_nFE> totLongArrI{};
            std::array<std::array<float, m_ncharge>, m_nFE> totErrLongArrI{};             
            
            // loop over charges
            for (int c=0; c<m_ncharge; ++c) {
                
                // Get TH2 for a given charge
                std::unique_ptr<TH2F> h2dTOTmean(get2DHistogramFromPath(rodDir,modName, "TOT_MEAN", c));
                std::unique_ptr<TH2F> h2dTOTsig(get2DHistogramFromPath(rodDir,modName, "TOT_SIGMA", c));
                if(!h2dTOTmean or !h2dTOTsig)  {
                    return false;
                }
                h2dTOTmean->SetDirectory(0);                
                h2dTOTsig->SetDirectory(0); 
                // loop over pixels
                for (unsigned int ieta = 0; ieta < m_etaBins; ieta++) {
                    for (unsigned int iphi = 0; iphi < m_phiBins; iphi++) {
                        float totmean = h2dTOTmean->GetBinContent(ieta + 1, iphi + 1);
                        float totsig  = h2dTOTsig ->GetBinContent(ieta + 1, iphi + 1);   
                        
                        if (totmean<0.1) { 
                            continue; 
                        }
                                             
                        int FE = chipId(iphi, ieta);
                        int pixel= pixelType(iphi, ieta, true);
                        
                        if(FE<0){
                            return false;
                        }

                        histogramsTOT.at(FE).at(pixel)->Fill(totmean);
                        histogramsTOTsig.at(FE).at(pixel)->Fill(totsig);
                        
                    }
                }
                
                // free memory
                h2dTOTmean.reset();
                h2dTOTsig.reset();         
                
                //filling arrays
                for(unsigned int FE = 0; FE < m_nFE; FE++){
                    for(unsigned int pixel = 0; pixel <2; pixel++){
                        
                        if(pixel == 0){
                            totArrI.at(FE).at(c)       = histogramsTOT.at(FE).at(pixel)->GetMean();
                            totErrArrI.at(FE).at(c)    = histogramsTOT.at(FE).at(pixel)->GetMeanError(); 
                            totSigArrI.at(FE).at(c)    = std::sqrt(std::pow(histogramsTOTsig.at(FE).at(pixel)->GetMean()     ,2)+std::pow(histogramsTOT.at(FE).at(pixel)->GetRMS()     ,2));
                            totSigErrArrI.at(FE).at(c) = std::sqrt(std::pow(histogramsTOTsig.at(FE).at(pixel)->GetMeanError(),2)+std::pow(histogramsTOT.at(FE).at(pixel)->GetRMSError(),2));
                            
                            if(totSigErrArrI.at(FE).at(c) > 1.0){
                                totArrI.at(FE).at(c) = 0.0;
                            }
                            
                        }
                        else{
                            totLongArrI.at(FE).at(c)    = histogramsTOT.at(FE).at(pixel)->GetMean();
                            totErrLongArrI.at(FE).at(c) = histogramsTOT.at(FE).at(pixel)->GetMeanError(); 
                            
                            if(totErrLongArrI.at(FE).at(c) > 1.0){
                                totLongArrI.at(FE).at(c) = 0.0;
                            }                            
                                                       
                        }
                        
                        //reset histogram for next iteration
                        histogramsTOT.at(FE).at(pixel)->Reset("ICESM");
                        histogramsTOTsig.at(FE).at(pixel)->Reset("ICESM");
                    }
                }
            } // End of charge.
            
            // loop over FE and create a graph for fitting            
            for(unsigned int FE = 0; FE < m_nFE; FE++) {

                TString subdir(((FE < 10) ? "FE0" : "FE") +std::to_string(FE));
                
                std::vector<float> v_Q;
                std::vector<float> v_Qerr;
                std::vector<float> v_TOT;
                std::vector<float> v_TOTerr;
                std::vector<float> v_TOTsig;
                std::vector<float> v_TOTsigerr;
                std::vector<float> v_TOTlong;
                std::vector<float> v_TOTlongerr;
                
                std::copy(std::begin(m_chargeArr)        , std::end(m_chargeArr)        , std::back_inserter(v_Q)           );
                std::copy(std::begin(m_chargeErrArr)     , std::end(m_chargeErrArr)     , std::back_inserter(v_Qerr)        );
                std::copy(std::begin(totArrI[FE])        , std::end(totArrI[FE])        , std::back_inserter(v_TOT)         );
                std::copy(std::begin(totErrArrI[FE])     , std::end(totErrArrI[FE])     , std::back_inserter(v_TOTerr)      );
                std::copy(std::begin(totSigArrI[FE])     , std::end(totSigArrI[FE])     , std::back_inserter(v_TOTsig)      );
                std::copy(std::begin(totSigErrArrI[FE])  , std::end(totSigErrArrI[FE])  , std::back_inserter(v_TOTsigerr)   );
                std::copy(std::begin(totLongArrI[FE])    , std::end(totLongArrI[FE])    , std::back_inserter(v_TOTlong)     );
                std::copy(std::begin(totErrLongArrI[FE]) , std::end(totErrLongArrI[FE]) , std::back_inserter(v_TOTlongerr)  );
                
                std::vector<float> pixNormalParams;
                std::vector<float> pixNormalParamsQuality;
                std::vector<float> pixSigParams;
                std::vector<float> pixSigParamsQuality;
                std::vector<float> pixLongParams;
                std::vector<float> pixLongParamsQuality;
                
                //For normal pixels and sig
                uint8_t n_fit = 0;
                do{
                    int vecsize = v_Q.size();
                    
                    std::unique_ptr<TGraphErrors> graphnormal = std::make_unique<TGraphErrors>(vecsize, &v_Q.at(0), &v_TOT.at(0)    , &v_Qerr.at(0), &v_TOTerr.at(0)    );   
                    std::unique_ptr<TGraphErrors> graphsig    = std::make_unique<TGraphErrors>(vecsize, &v_Q.at(0), &v_TOTsig.at(0) , &v_Qerr.at(0), &v_TOTsigerr.at(0) );   
                    
                    std::unique_ptr<TF1> functnormal    = std::make_unique<TF1>("normal"    ,new funcTot , m_chargeArr[m_qthresh]-100, m_chargeArr[m_ncharge-1]+100, 3);
                    std::unique_ptr<TF1> functnormalsig = std::make_unique<TF1>("normal_sig",new funcDisp, m_chargeArr[m_qthresh]-100, m_chargeArr[m_ncharge-1]+100, 2);
                    
                    graphnormal->Fit(functnormal.get()   ,"MRQ");
                    graphsig   ->Fit(functnormalsig.get(),"MRQ");
                    
                    pixNormalParams = getParams(functnormal.get()   ,3 );
                    pixSigParams    = getParams(functnormalsig.get(),2 );
                    
                    pixNormalParamsQuality = getParams_quality(functnormal.get()   );
                    pixSigParamsQuality    = getParams_quality(functnormalsig.get());
                    
                    if(m_savefile){
                        
                        m_wFile->cd();
                        if( !m_wFile->Get(rodName+"/"+modName+"/TOTfits/"+subdir) ){
                            m_wFile->mkdir(rodName+"/"+modName+"/TOTfits/"+subdir,rodName);
                        }

                        m_wFile->cd(rodName+"/"+modName+"/TOTfits/"+subdir);
                        
                        graphTitles(graphnormal, TString(modName+" - "+subdir+" - normal pixels: Fit: "+std::to_string(n_fit)).Data(), "TOT");
                        graphTitles(graphsig   , TString(modName+" - "+subdir+" - normal pixels: Fit: "+std::to_string(n_fit)).Data(), "Charge smearing");
                        
                        graphnormal->Write(TString("normal_fit_"+std::to_string(n_fit)), TObject::kWriteDelete);
                        graphsig->Write(TString("smearing_fit_"+std::to_string(n_fit)), TObject::kWriteDelete);  
                        n_fit++;                     
                    }
                                        
                    functnormal.reset();
                    functnormalsig.reset();
                    
                    graphnormal.reset();
                    graphsig.reset();
                    
                }while(reFit_normalPix(pixNormalParams, v_Q, v_Qerr, v_TOT, v_TOTerr, v_TOTsig, v_TOTsigerr, FE )     );
                
                
                // Since we have modified the vector size we need to clear it and refill it for the long and gange pixels
                v_Q.clear();
                v_Qerr.clear();
                
                std::copy(std::begin(m_chargeArr)        , std::end(m_chargeArr)        , std::back_inserter(v_Q)           );
                std::copy(std::begin(m_chargeErrArr)     , std::end(m_chargeErrArr)     , std::back_inserter(v_Qerr)        );
                
                
                //For long and ganged pixels (combined)
                n_fit = 0;
                do{
                    
                    //size will be modified in the condition..
                    int vecsize = v_Q.size();
                    
                    std::unique_ptr<TGraphErrors> graflong   = std::make_unique<TGraphErrors>(vecsize, &v_Q.at(0), &v_TOTlong.at(0), &v_Qerr.at(0), &v_TOTlongerr.at(0) );
                    
                    std::unique_ptr<TF1> functlong      = std::make_unique<TF1>("long"      ,new funcTot , m_chargeArr[m_qthresh]-100, m_chargeArr[m_ncharge-1]+100, 3);
                    
                    graflong  ->Fit(functlong.get() ,"MRQ");
                    
                    pixLongParams = getParams(functlong.get() ,3 );
                    pixLongParamsQuality = getParams_quality(functlong.get() );

                    if(m_savefile){
                        
                        m_wFile->cd();
                        if( !m_wFile->Get(rodName+"/"+modName+"/TOTfits/"+subdir) ){
                            m_wFile->mkdir(rodName+"/"+modName+"/TOTfits/"+subdir,rodName);
                        }

                        m_wFile->cd(rodName+"/"+modName+"/TOTfits/"+subdir);
                        
                        graphTitles(graflong, TString(modName+" - "+subdir+" - long+ganged pixels: Fit: "+std::to_string(n_fit)).Data(), "TOT");
                        
                        graflong->Write(TString("long_ganged_fit_"+std::to_string(n_fit)), TObject::kWriteDelete);
                        n_fit++;                     
                    }
                    
                    //delete the TF1                
                    functlong.reset();

                    //delete the graphs
                    graflong.reset();                    
                    
                // no need to loop here.. leaving to improve it in the future - might need refitting    
                }while( false);
                
                
                // Find the module it belong to
                int modID = pm.getID(std::string(modName));
                auto itr = map_info.find( modID );
                
                if (itr != map_info.end()) {
                    (itr->second).at(FE)->set_NormalParams( pixNormalParams);
                    (itr->second).at(FE)->set_LongParams  ( pixLongParams  );
                    (itr->second).at(FE)->set_SigParams   ( pixSigParams   );
                    
                    (itr->second).at(FE)->set_times_fitted   ( n_fit   );
                    
                    (itr->second).at(FE)->set_NormalParamsQuality( pixNormalParamsQuality);
                    (itr->second).at(FE)->set_LongParamsQuality  ( pixLongParamsQuality  );
                    (itr->second).at(FE)->set_SigParamsQuality   ( pixSigParamsQuality   );
                }
                else{
                    printf("Error - Module not found in fitting step... Skipping.\n");
                    return false;
                }
                
            } // End of FE loop
        } // End of MOD
    } // End of ROD
    
    // remove from memory
    for(unsigned int FE = 0; FE < m_nFE; FE++) {
        for(unsigned int pixel=0; pixel<2; pixel++) {
            histogramsTOT.at(FE).at(pixel).reset();
            histogramsTOTsig.at(FE).at(pixel).reset();
        }
    }    

    totFile.Close();
    return true;
}



bool Calib::reFit_normalPix(std::vector<float> &params, std::vector<float> &q, std::vector<float> &qerr, std::vector<float> &tot, std::vector<float> &toterr, std::vector<float> &sig, std::vector<float> &sigerr, const unsigned int fe){
    
    float vecFit_size = q.size()+1;
    // float vecFit_size = q.size() - m_qthresh;
    float stopFit = m_ncharge/2.0;
    // float stopFit = (m_ncharge - m_qthresh)/2.0;
    if(vecFit_size < stopFit) {

        // Default values for the fit
        params.at(0) = 0;
        params.at(1) = -28284.3;
        params.at(2) = 0;
        
        printf("reFit_normalPix: Refitting skipped. Not enough points to fit.\n");
        
        return false;
    }
    
    float parAI0 = params.at(0);
    float parEI0 = params.at(1);
    float parCI0 = params.at(2);
    
    std::vector<float> v_discrepancy;
    
    for(unsigned int i = 0; i < q.size(); i++){
        float discrepancy = std::abs( 1 - ( (parAI0 * parEI0 - parCI0 * tot.at(i)) / (tot.at(i) - parAI0) ) / q.at(i) );
        
        if( i < m_qthresh ){
            discrepancy = 0.0;
        }
        v_discrepancy.push_back(discrepancy);
    }
    
    auto itr_max = std::max_element(v_discrepancy.begin(),v_discrepancy.end());
    
    if(*itr_max > m_chi_error){
        
        size_t n_max = std::distance(v_discrepancy.begin(), itr_max);
        printf("FE %02u Refitted, removing charge %5.0f with chi_error %7.5f\n", fe ,q.at(n_max),*itr_max);
        q.erase(q.begin()+n_max);
        qerr.erase(qerr.begin()+n_max);
        tot.erase(tot.begin()+n_max);
        toterr.erase(toterr.begin()+n_max);
        sig.erase(sig.begin()+n_max);
        sigerr.erase(sigerr.begin()+n_max);
        
        return true;
    }
    
    return false;
    
}


bool Calib::fillTiming(const pix::PixelMapping &pm, const std::string &inTimFile, std::map<unsigned int, std::vector<std::unique_ptr<CalibFrontEndInfo>> > &map_info) {
    
    if (inTimFile.empty()) return false;
    
    TFile timFile(inTimFile.c_str(),"READ");
    if (not timFile.IsOpen()) {
        printf("Error - File %s could not be opened.\n",inTimFile.c_str());
        return false;
    } else {
        printf("File %s opened.\n",inTimFile.c_str());
        printf("Running timming calibration...\n");
    }
    
    // Creating here the histograms with this scope owner
    std::vector< std::vector< std::unique_ptr<TH1F> > > histogramsTIM;
    for(unsigned int FE = 0; FE < m_nFE; FE++){
        
        histogramsTIM.push_back( std::vector< std::unique_ptr<TH1F> >() );
        
        for(unsigned int pixel=0; pixel<3; pixel++){
            TString title = "FE"+std::to_string(FE)+"_pixType"+std::to_string(pixel);
            std::unique_ptr<TH1F> h = std::make_unique<TH1F>(title+"_thr", title+"_thr", m_timnbins, m_timLo, m_timHi);
            h->SetDirectory(0);
            //cppcheck-suppress containerOutOfBounds
            histogramsTIM.at(FE).push_back(std::move(h));
        }
    } 

    //Will strat looping over the RODs
    TIter rodItr = getRodIterator(timFile);
    TKey* rodKey;
    while ((rodKey=static_cast<TKey*>(rodItr()))) {
        const TString rodName(rodKey->GetName());
        TDirectoryFile* rodDir = static_cast<TDirectoryFile*>(rodKey->ReadObj());
        TKey* modKey;
        TIter modItr=getModuleIterator(rodDir);

        // Looping over the MODs of each ROD
        while ((modKey=static_cast<TKey*>(modItr()))) {
            TString modName(modKey->GetName());
            
            if ( not moduleInPart(modName)){
                continue;
            } 
            if ( not pm.contains(std::string(modName))){
                printf("Error - Module %s not found in the PixelMapping tool\n",modName.Data());
                continue;
            } 
            
            if( m_runOneMOD and strcmp(modName, m_testMOD) != 0){
                continue;
            }           
            
            std::unique_ptr<TH2F> h2dTim(get2DHistogramFromPath(rodDir,modName, "SCURVE_MEAN"));
            h2dTim->SetDirectory(0);
            
            for (unsigned int ieta = 0; ieta < m_etaBins; ieta++) {
                for (unsigned int iphi = 0; iphi < m_phiBins; iphi++) {
                    
                    float tim = h2dTim->GetBinContent(ieta + 1, iphi + 1);
                    
                    if (tim<0.5) { 
                        continue; 
                    }
                    
                    int FE = chipId(iphi, ieta);
                    int pixel= pixelType(iphi, ieta);
                    
                    if(FE<0){
                        return false;
                    }
                    
                    histogramsTIM.at(FE).at(pixel)->Fill(tim);
                }
            }

            // Freeing memory of TH2F         
            h2dTim.reset();
            
            int modID = pm.getID(std::string(modName));
            auto itr = map_info.find( modID );
            if (itr == map_info.end()) {
                printf("Calib::fillTiming: ERROR - Mod ID= %16s not found. Creating it -----> Inform Pixel Offline Software Experts... \n",std::string(modName).c_str());
                
                map_info[modID] = std::vector<std::unique_ptr<CalibFrontEndInfo>> ();
                
                for(unsigned int FE = 0; FE < m_nFE; FE++){
                    
                    map_info[modID].push_back( std::unique_ptr<CalibFrontEndInfo>() );
                    std::unique_ptr<CalibFrontEndInfo> p = std::make_unique<CalibFrontEndInfo>(modID,FE,std::string(modName),std::string(rodKey->GetName()));
                    map_info[modID].at(FE) = std::move(p);
                    
                    for(unsigned int pixel=0; pixel<3; pixel++){
                        
                        // Saving information for the calibration
                        int tim_mean = histogramsTIM.at(FE).at(pixel)->GetMean();
                        
                        // Reset histograms for next front end
                        histogramsTIM.at(FE).at(pixel)->Reset("ICESM");
                        
                        if(pixel == 0){ // normal
                            map_info[modID].at(FE)->set_NormalIntime(tim_mean);
                        }
                        else if(pixel == 1){ // long
                            map_info[modID].at(FE)->set_LongIntime(tim_mean);
                        }
                        else if(pixel == 2){ // ganged
                            map_info[modID].at(FE)->set_GangedIntime(tim_mean);
                        }
                        else{
                            printf("Error - Bad pixel in Calib::fillTiming\n");
                            return false;
                        }
                    }
                }                 
                
            }
            else{
                for(unsigned int FE = 0; FE < m_nFE; FE++){
                    TString subdir(((FE < 10) ? "FE0" : "FE") +std::to_string(FE));
                    for(unsigned int pixel=0; pixel<3; pixel++){
                        
                        // Saving information for the calibration
                        int tim_mean = histogramsTIM.at(FE).at(pixel)->GetMean();

                        if(m_savefile){
                            
                            m_wFile->cd();
                            if( !m_wFile->Get(rodName+"/"+modName+"/Thresholds/"+subdir) ){
                                m_wFile->mkdir(rodName+"/"+modName+"/Thresholds/"+subdir,rodName);
                            }

                            m_wFile->cd(rodName+"/"+modName+"/Thresholds/"+subdir);

                            histogramsTIM.at(FE).at(pixel)->SetTitle("Intime;Pixel intime;Counts");

                            std::string type = "";
                            if(pixel == 0) type = "normal";
                            else if(pixel == 1) type = "long";
                            else if(pixel == 2) type = "ganged";
                            
                            histogramsTIM.at(FE).at(pixel)->Write(TString("intime_"+type).Data(), TObject::kWriteDelete);
                        }
                        
                        // Reset histograms for next front end
                        histogramsTIM.at(FE).at(pixel)->Reset("ICESM");
                        
                        if(pixel == 0){ // normal
                            (itr->second).at(FE)->set_NormalIntime(tim_mean);
                        }
                        else if(pixel == 1){ // long
                            (itr->second).at(FE)->set_LongIntime(tim_mean);
                        }
                        else if(pixel == 2){ // ganged
                            (itr->second).at(FE)->set_GangedIntime(tim_mean);
                        }
                        else{
                            printf("Error - Bad pixel in Calib::fillTiming\n");
                            return false;
                        }
                    } // End of pixel type loop
                } // End of FE loop                     
            }
        } // End of MOD loop  
    }// End of ROD loop 
    
    for(unsigned int FE = 0; FE < m_nFE; FE++) {
        for(unsigned int pixel=0; pixel<3; pixel++) {
            histogramsTIM.at(FE).at(pixel).reset();
        }
    }
    
    //Closing file - not needed anymore
    timFile.Close();
    printf("DONE with threshold calibration.\n");
    return true;
}



bool Calib::fillThresholds(const pix::PixelMapping &pm, const std::string &inThrFile, std::map<unsigned int, std::vector<std::unique_ptr<CalibFrontEndInfo>> > &map_info) {
    
    if (inThrFile.empty()) return false;
    
    TFile riThrFile(inThrFile.c_str(),"READ");
    if (not riThrFile.IsOpen()) {
        printf("Error - File %s could not be opened.\n",inThrFile.c_str());
        return false;
    } else {
        printf("File %s opened.\n",inThrFile.c_str());
        printf("Running threshold calibration...\n");
    }
    
    // Creating here the histograms with this scope owner
    std::vector< std::vector< std::unique_ptr<TH1F> > > histogramsTHR;
    std::vector< std::vector< std::unique_ptr<TH1F> > > histogramsSIG;
    for(unsigned int FE = 0; FE < m_nFE; FE++){
        
        histogramsTHR.push_back( std::vector< std::unique_ptr<TH1F> >() );
        histogramsSIG.push_back( std::vector< std::unique_ptr<TH1F> >() );
        
        for(unsigned int pixel=0; pixel<3; pixel++){
            TString title = "FE"+std::to_string(FE)+"_pixType"+std::to_string(pixel);
            std::unique_ptr<TH1F> hthr = std::make_unique<TH1F>(title+"_thr", title+"_thr", m_thrnbins, m_thrLo, m_thrHi);
            hthr->SetDirectory(0);
            std::unique_ptr<TH1F> hsig = std::make_unique<TH1F>(title+"_sig", title+"_sig", m_thrnbins, m_sigLo, m_sigHi);
            hsig->SetDirectory(0);
            //cppcheck-suppress containerOutOfBounds
            histogramsTHR.at(FE).push_back(std::move(hthr));
            //cppcheck-suppress containerOutOfBounds
            histogramsSIG.at(FE).push_back(std::move(hsig));
        }
    } 

    //Will start looping over the RODs
    TIter rodItr = getRodIterator(riThrFile);
    TKey* rodKey;
    while ((rodKey=static_cast<TKey*>(rodItr()))) {
        const TString rodName(rodKey->GetName());
        TDirectoryFile* rodDir = (TDirectoryFile*)rodKey->ReadObj();
        TKey* modKey;
        TIter modItr=getModuleIterator(rodDir);

        // Looping over the MODs of each ROD
        while ((modKey=static_cast<TKey*>(modItr()))) {
            TString modName(modKey->GetName());
            
            if ( not moduleInPart(modName)){
                continue;
            } 
            if ( not pm.contains(std::string(modName))){
                printf("Error - Module %s not found in the PixelMapping tool\n",modName.Data());
                continue;
            } 
            
            if( m_runOneMOD and strcmp(modName, m_testMOD) != 0){
                continue;
            } 
            
            // pixel discriminator threshold
            std::unique_ptr<TH2F> h2dThr(get2DHistogramFromPath(rodDir,modName, "SCURVE_MEAN"));
            h2dThr->SetDirectory(0);
            
            // Getting histogram for noise
            std::unique_ptr<TH2F>h2dSig(get2DHistogramFromPath(rodDir,modName, "SCURVE_SIGMA"));
            h2dSig->SetDirectory(0);
            
            for (unsigned int ieta = 0; ieta < m_etaBins; ieta++) {
                for (unsigned int iphi = 0; iphi < m_phiBins; iphi++) {
                    
                    float thr = h2dThr->GetBinContent(ieta + 1, iphi + 1);
                    float sig = h2dSig->GetBinContent(ieta + 1, iphi + 1); 
                    
                    if (thr == 0 || thr > 10000 || sig == 0 || sig > 1000) {
                        continue;
                    }
                    
                    int FE = chipId(iphi, ieta);
                    int pixel= pixelType(iphi, ieta);
                    
                    if(FE<0){
                        return false;
                    }

                    histogramsTHR.at(FE).at(pixel)->Fill(thr);
                    histogramsSIG.at(FE).at(pixel)->Fill(sig);
                    
                }
            }

            // Freeing memory of TH2F         
            h2dThr.reset();
            h2dSig.reset();
            
            int modID = pm.getID(std::string(modName));
            auto itr = map_info.find( modID );

            // Map should be empty and therefore we need to create the key - if the key is repeated then it will throw an error
            if (itr == map_info.end()) {

                map_info[modID] = std::vector<std::unique_ptr<CalibFrontEndInfo>> ();
                
                for(unsigned int FE = 0; FE < m_nFE; FE++){
                    TString subdir(((FE < 10) ? "FE0" : "FE") +std::to_string(FE));
                    map_info[modID].push_back( std::unique_ptr<CalibFrontEndInfo>() );
                    std::unique_ptr<CalibFrontEndInfo> p = std::make_unique<CalibFrontEndInfo>(modID,FE,std::string(modName),std::string(rodKey->GetName()));
                    map_info[modID].at(FE) = std::move(p);
                    
                    for(unsigned int pixel=0; pixel<3; pixel++){
                        
                        // Saving information for the calibration
                        int thr_mean = histogramsTHR.at(FE).at(pixel)->GetMean();
                        int thr_rms  = histogramsTHR.at(FE).at(pixel)->GetRMS();
                        int sig_mean = histogramsSIG.at(FE).at(pixel)->GetMean();

                        if(m_savefile){
                            
                            m_wFile->cd();
                            if( !m_wFile->Get(rodName+"/"+modName+"/Thresholds/"+subdir) ){
                                m_wFile->mkdir(rodName+"/"+modName+"/Thresholds/"+subdir,rodName);
                            }

                            m_wFile->cd(rodName+"/"+modName+"/Thresholds/"+subdir);

                            histogramsTHR.at(FE).at(pixel)->SetTitle("Threshold;Pixel threshold;Counts");
                            histogramsSIG.at(FE).at(pixel)->SetTitle("Sigma;Pixel sigma;Counts");

                            std::string type = "";
                            if(pixel == 0) type = "normal";
                            else if(pixel == 1) type = "long";
                            else if(pixel == 2) type = "ganged";
                            
                            histogramsTHR.at(FE).at(pixel)->Write(TString("thres_"+type).Data(), TObject::kWriteDelete);
                            histogramsSIG.at(FE).at(pixel)->Write(TString("sigma_"+type).Data(), TObject::kWriteDelete);  
                        }                        
                        
                        // Reset histograms for next front end
                        histogramsTHR.at(FE).at(pixel)->Reset("ICESM");
                        histogramsSIG.at(FE).at(pixel)->Reset("ICESM");
                        
                        if(pixel == 0){ // normal
                            map_info[modID].at(FE)->set_NormalThreshold(thr_mean);
                            map_info[modID].at(FE)->set_NormalRms(thr_rms);
                            map_info[modID].at(FE)->set_NormalNoise(sig_mean);
                        }
                        else if(pixel == 1){ // long
                            map_info[modID].at(FE)->set_LongThreshold(thr_mean);
                            map_info[modID].at(FE)->set_LongRms(thr_rms);
                            map_info[modID].at(FE)->set_LongNoise(sig_mean);                            
                        }
                        else if(pixel == 2){ // ganged
                            map_info[modID].at(FE)->set_GangedThreshold(thr_mean);
                            map_info[modID].at(FE)->set_GangedRms(thr_rms);
                            map_info[modID].at(FE)->set_GangedNoise(sig_mean);                            
                        }
                        else{
                            printf("Calib::fillThresholds: ERROR - Bad pixel in Calib::fillThresholds\n");
                            return false;
                        }
                    }
                }                 
                
            }
            else{
                printf("Calib::fillThresholds: ERROR - REPEATED MOD ID: %s! Contact Offline team\n",std::string(modName).c_str());
                return false;
            }
        } // End of MOD loop 
    } // End of ROD loop 
    
    for(unsigned int FE = 0; FE < m_nFE; FE++) {
        for(unsigned int pixel=0; pixel<3; pixel++) {
            histogramsTHR.at(FE).at(pixel).reset();
            histogramsSIG.at(FE).at(pixel).reset();
        }
    }
    
    //Closing file - not needed anymore
    riThrFile.Close();
    printf("DONE with threshold calibration.\n");
    return true;
}


int Calib::chipId(int iphi, int ieta) {
    int circ = -1;
    if (iphi < 160) {
        circ = (int)(ieta / 18);
    } else {
        circ = 15 - (int)(ieta / 18);
    } // FE15, FE14, ... FE8
    
    if (circ>15){
        printf("Error - FE id error: %d, setting it to -1",circ);
        circ = -1;//error
    } 
    return circ;
}


int Calib::pixelType(int iphi, int ieta, bool isForTOT) {
    
    // normal pixels ( by default )
    int pixtype = 0;
    
    // define long pixels
    if (ieta % 18 == 0 || ieta % 18 == 17) {
        pixtype = 1;
    } 
    // define ganged pixels
    if (iphi > 152 && iphi < 160 && iphi % 2 == 1) {
        pixtype = 2;
    } 
    if (iphi > 159 && iphi < 167 && iphi % 2 == 0) {
        pixtype = 2;
    }
    
    if(isForTOT and pixtype == 2 ) pixtype=1;
    return pixtype;
}

TIter Calib::getRodIterator(const TFile & inputFile) {
    TDirectoryFile* scanDir = static_cast<TDirectoryFile*>((static_cast<TKey*>(inputFile.GetListOfKeys()->First()))->ReadObj());
    TList* rodKeyList = static_cast<TList*>(scanDir->GetListOfKeys());
    return TIter(rodKeyList);
}

TIter Calib::getModuleIterator( TDirectoryFile* rodDir) {
    TList* modKeyList = static_cast<TList*>(rodDir->GetListOfKeys());
    return TIter(modKeyList);
}

TH2F* Calib::get2DHistogramFromPath( TDirectoryFile* rodDir, const TString & moduleName, const TString & histName, int charge) {
    TString suffix = (charge<0) ? ("") : (TString("/C") + charge);
    TString fullHistoPath = moduleName + "/" + histName + "/A0/B0" + suffix;
    TDirectoryFile *histDir = static_cast<TDirectoryFile *>(rodDir->GetDirectory(fullHistoPath));
    
    if(!histDir){
        printf("Error - Directory \"%s\" not found. Exiting..\n",fullHistoPath.Data());
        return nullptr;
    }
    TH2F *pTH2 = static_cast<TH2F*>((static_cast<TKey*>(histDir->GetListOfKeys()->First()))->ReadObj());
    pTH2->SetDirectory(0);
    
    return pTH2;
}

bool Calib::moduleInPart(const TString & modName) {
    if (modName == "DSP_ERRORS") {
        return false;
    }
    return modName.BeginsWith(m_MODprefixes[m_whichPart]);
}


std::vector<float> Calib::getParams(const TF1 *f, unsigned int params){
    std::vector<float> v;
    for(unsigned int i = 0; i<params; i++){
        v.push_back(f->GetParameter(i));
    }
    
    return v;
}

std::vector<float> Calib::getParams_quality(const TF1 *f){
    std::vector<float> v;
    
    v.push_back(f->GetChisquare());
    v.push_back(f->GetNDF());
    
    return v;
}

void Calib::graphTitles(const std::unique_ptr<TGraphErrors> &graph, const std::string &name, const std::string &Yname){
    graph->SetTitle(TString(name)+";Charge;"+TString(Yname));
    graph->SetMarkerStyle(20);
}
        

