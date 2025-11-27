/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***********************************************************************************************
//             PixelCalibration - Program to execute the pixel charge calibration (not IBL)
//                           ---------------------------------------
//     begin                : 01 11 2023
//     email                : sergi.rodriguez@cern.ch
//***********************************************************************************************

#include "ChargeCalibration/pixel/tools/CalibFrontEndInfo.h"
#include "ChargeCalibration/pixel/tools/Calib.h"
#include "ChargeCalibration/common/PixelMapping.h"
#include "PathResolver/PathResolver.h"

#include "TFile.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <memory>

using pix::PixelMapping; 

void printError(){
    printf("ERROR - Argument not expected or wrongly set:\n\n");
    printf("Valid format is: ./PixelCalib.exe [Blayer, L1, L2, disk] THR=SCAN_Sxxxxxxxxx THRintime=SCAN_Sxxxxxxxxx TOT=SCAN_Sxxxxxxxxx directory_path=path/to/file/ [saveInfo]\n");
    printf("\n\t i.e: ./PixelCalib.exe Blayer THR=SCAN_S000087719 THR_intime=SCAN_S000087717 TOT=SCAN_S000087710 directory_path=/eos/user/x/xxxx/\n");
    printf("\nThe example will run Blayer calibration using the scans: SCAN_S000087719.root, SCAN_S000087717.root and SCAN_S000087710.root stored in /eos/user/x/xxxx/\n");    
    printf("\nNOTE: If you type as an argument another layer or file at the same time\n\t e.i: './PixelCalib.exe Blayer L1 THR=SCAN_S000087719 THR_intime=SCAN_S000087717 TOT=SCAN_S000087710 directory_path=/eos/user/x/xxxx/'   -   Blayer and L1 are in the arguments\n");    
    printf("It will run the last valid layer in the arguments (in the example above will be \"L1\" layer) \n");    
}


int main(int argc, char *argv[]) {
    
    int whichPart = -1;
    bool saveInfo = false;
    std::string THR = "THR";
    std::string THRintime = "THRintime";
    std::string TOT = "TOT";
    std::string dpath = "directory_path";
    std::vector<std::string> sWhichPart = {"Blayer","L1","L2","disk"};
    
    for(int i=1; i<argc; i++){
        std::string aux(argv[i]);
        //0=BLayer, 1=L1, 2=L2, 3=disk
        if(aux.compare("Blayer") == 0)      whichPart = 0;
        else if(aux.compare("L1") == 0)     whichPart = 1;
        else if(aux.compare("L2") == 0)     whichPart = 2;       
        else if(aux.compare("disk") == 0)   whichPart = 3;
        else if(aux.compare("saveInfo") == 0)   saveInfo = true;
        else if(THRintime.compare(aux.substr(0,aux.find("="))) == 0)  THRintime = aux.substr(aux.find("=")+1);
        else if(THR.compare(aux.substr(0,aux.find("="))) == 0)        THR = aux.substr(aux.find("=")+1);
        else if(TOT.compare(aux.substr(0,aux.find("="))) == 0)        TOT = aux.substr(aux.find("=")+1);
        else if(dpath.compare(aux.substr(0,aux.find("="))) == 0)      dpath = aux.substr(aux.find("=")+1);
        else{
            printError();
            return 1;
        } 
    }
    
    printf("%-14s = %s\n","Directory path",dpath.c_str());
    printf("%-14s = %d - %s\n","Pixel part",whichPart, (whichPart < 0 or whichPart > 3) ? "-1" : sWhichPart.at(whichPart).c_str());
    printf("%-14s = %s.root\n","THR",THR.c_str());
    printf("%-14s = %s.root\n","THRintime",THRintime.c_str());
    printf("%-14s = %s.root\n","TOT",TOT.c_str());    
    printf("%-14s = %s\n\n\n","Save root file",saveInfo ? "True" : "False" );    
    
    
    bool correctArgc = (whichPart < 0 or whichPart > 3) or (THR.compare("THR") == 0) or (THRintime.compare("THRintime") == 0) or (TOT.compare("TOT") == 0) or (dpath.compare("directory_path") == 0);
    
    if(correctArgc){
        printf("Cannot continue, one arguments is incorrect or not filled correctly...\n");
        printf("Helper below:\n**********************\n\n");
        printError(); 
        return 1;
    }
    
    std::string thres_f = dpath+THR+".root";
    std::string timin_f = dpath+THRintime+".root";
    std::string totin_f = dpath+TOT+".root";
    
    
    // creating the object for the pixel mapping 
    PixelMapping pixmap(PathResolver::find_file("PixelCalibAlgs/mapping.csv", "DATAPATH"));
    
    // object to store all the necessary calibration information 
    std::map<unsigned int ,  std::vector<std::unique_ptr<CalibFrontEndInfo>> > map_values;
    
    std::unique_ptr<TFile> wFile = std::make_unique<TFile>();
    
    time_t start, end;
    
    time(&start); 
    //Setting up the Calibration functions.
    printf("Time to calculate threshold calibration\n");
    std::string moduleName = "";
    Calib Calibration(whichPart,saveInfo, moduleName);
    if(!(Calibration.fillThresholds(pixmap ,thres_f ,map_values )) ){
        printf("Error - The threshold calibration was not properly finished.\n");
        return 1;
    }
    time(&end);
    printf("Time taken for threshold calibration:%7.1f seconds\n",double(end - start));
    
    if(map_values.size() == 0){

        // If we are running over just one module, and it is not created already in the map means that this is not the correct layer
        if( std::strcmp(moduleName.c_str(), "") != 0 ){
            printf("main::main: Running only one module: %s - It does not belong to %s\n",moduleName.c_str(), sWhichPart.at(whichPart).c_str());
            return 0;
        }
        printf("main::main: ERROR - Size of filled map is 0. Does the %s exist in the %s file?\n",sWhichPart.at(whichPart).c_str(),thres_f.c_str());
        return 1;
    }
    
    time(&start); 
    //Setting up the Timing functions.
    printf("Time to take timing calibration\n");
    if(!(Calibration.fillTiming(pixmap ,timin_f ,map_values )) ){
        printf("Error - The timing calibration was not properly finished.\n");
        return 1;
    }
    time(&end);
    printf("Time taken for timing calibration:%7.1f seconds\n",double(end - start));
    
    time(&start); 
    //Setting up the tot functions.
    printf("Time to take TOT calibration\n");
    if(!(Calibration.totFitting(pixmap ,totin_f ,map_values )) ){
        printf("Error - The TOT calibration was not properly finished.\n");
        return 1;
    }
    time(&end);
    printf("Time taken for TOT calibration:%7.1f seconds\n",double(end - start));
    
    std::ofstream myFile("calibration_"+sWhichPart.at(whichPart)+".txt");
    //cppcheck-suppress invalidPrintfArgType_uint
    printf("Total MODs:%4lu\n",map_values.size());
    for(const auto & [key, MOD] : map_values){
        
        for(const auto& FE : MOD){
            std::stringstream sstr = FE->printDBformat();
            printf("%s\n",sstr.str().c_str());  
            myFile << sstr.str() << "\n";    
        }
    }    
    
    myFile.close();

    printf("********** JOB finished **********\n");
    return 0;
}
