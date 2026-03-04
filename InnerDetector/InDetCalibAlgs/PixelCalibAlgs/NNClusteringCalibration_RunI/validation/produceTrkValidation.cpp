/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "writeNtuple.C"

#include "TFile.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TDirectory.h"

#include <vector>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

using namespace std;


vector<TString>  
readFileNames( const char * fileName ){
  // read from a file a list of input filenames
  // and return a vector of string where the names are stores
  vector<TString> allFiles;
  string line;
  ifstream myfile (fileName);
  if (myfile.is_open()){
    while ( !myfile.eof() ){
	    getline (myfile,line);
	    if(myfile.eof()) break;
      TString Sline = static_cast<TString>(line);
      cout << Sline << endl;
	    allFiles.push_back(Sline);
	  }
    myfile.close();
  } else cout << "Unable to open file " << fileName << endl;
  return allFiles;
}



int 
countFiles(  vector<TString> j4Files , int maxEvents){
  // return number of files files in the list (vector<TString> j4Files) 
  // you need to run on in order to run on maxEvents 
  // return j4Files.size();
  if(maxEvents == -1) return j4Files.size();
  int totEntr=0;
  for(int nf=0; nf<j4Files.size(); nf++){
    TFile*  f =TFile::Open(j4Files.at(nf));
    if(f->GetSize()==0) continue;
    TTree*  t = (TTree*)f->Get("qcd");
    if(!t)continue;
    double Nj4 = (double)t->GetEntries();
    totEntr+=Nj4;
    f->Close();
    delete f;
    if(totEntr >= maxEvents) {
      if(nf==0)nf++;
      return nf;
    }
  }
  return j4Files.size();
}




int main ( int argc, char *argv[] )
{
  
 
  int     nParticles;
  bool    useTrack;
  int     nevents;
  TString outName;
  
  if ( argc != 5 ){ // argc should be 1 for correct execution
    // We print argv[0] assuming it is the program name
    cout<<"usage: "<< argv[0] <<" <nParticlesTraining> <useTrack?> <nevents> <outfilename.root> \n";
    return 0;
  }else{
    
    if(  strcmp(argv[2], "false") &&  strcmp(argv[2], "true") ){
	
	cout << "set useTrack parameter true or false" << endl;
	return 0;

	}
    
    nParticles     =  atoi(argv[1]);
    if( !strcmp(argv[2], "false") ){useTrack=false;}else{useTrack=true;}
    nevents        =  atoi(argv[3]);
    outName        =  argv[4];
       
    
  }

  const char* inName;
  if(!useTrack){inName="files.txt";}else{inName="filesOnTrack.txt";}
  
  TChain* tC = new TChain("Validation/NNinput");
  
  cout << "reading NNinput from " << inName << endl;
  
  vector<TString> files =  readFileNames(inName);
  int   totFiles = countFiles(files,nevents );
  for(int nf = 0; nf< totFiles; nf++) { tC->Add(files.at(nf)); } 
      

  TString    pathWithoutTracks  = "/afs/cern.ch/user/g/giacinto/scratch0/PixelClusterisationTF/jetNet/jetNetJune2011/withoutTracks/";
  TString    pathWithTracks     = "/afs/cern.ch/user/g/giacinto/scratch0/PixelClusterisationTF/prepareJetNetNtupleApril2011/withTracks/";


  writeNtuple* l = new writeNtuple(tC);
  
  l->m_pathWeightsWithoutTracks = pathWithoutTracks;
  l->m_pathWeightsWithTracks    = pathWithTracks;
  try{
    l->Loop(nParticles, useTrack, outName );
  } 
  catch (const std::exception& e) {
    std::cerr << "Standard exception: " << e.what() << '\n';
    delete l;
    return 1;
  } 
  catch (...) {
    std::cerr << "Unknown exception caught\n";
    delete l;
    return 1;
  } 
  delete l;
  
  return 0;
  

}
