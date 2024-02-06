#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <unistd.h>

using namespace std;


int main(int argc, char *argv[]) {

  int nfiles=argc-2;
  cout << " TRT_StrawStatus_merge. nfiles= " << nfiles << endl;
  //  int m_accumulateHits[2][32][5482][6]; // same as in the athena package TRT_StrawStatus.cxx. Can crash due to too large array.
  int m_accumulateHits[32][5482][6]; // So reduce to one array for each side


  int nevents = 0; // we first need to read the first line in each file to get the total number of events
  for (int irun = 0; irun<nfiles; irun++) {
     char filename[1000];
     sprintf(filename, "%s", argv[irun+2]); 
     printf("reading file %s \n", filename); // debug printout
     FILE *f = fopen( filename, "r");
     if (f==0) {  printf("file %s missing\n", filename); continue; }
     int tmp[10];
     fscanf(f, "%d %d %d %d %d %d %d %d %d\n", tmp, tmp+1, tmp+2, tmp+3, tmp+4, tmp+5, tmp+6, tmp+7, tmp+8); 
     nevents += tmp[8];
     fclose(f);
  }
  printf("read %d events from %d files\n", nevents, nfiles);

  FILE *fout = fopen(argv[1], "w"); // The output merged file.
  if (fout==0) { printf("could not open file %s for writing, EXIT", argv[1]); return 0; }
  printf("writing to merged file: %s\n", argv[1]);
  for (int i=0; i<8; i++) fprintf(fout, "%d ", 0); fprintf(fout, "%d\n", nevents); //write total number of events in the first line

  for( int side=0;side<2;side++){  //merge separately on the two sides

    cout << " reset status for all straws on side " << side*2-1 << endl;
    for (int j=0; j<32; j++) for (int k=0; k<5482; k++) for (int m=0; m<6; m++) m_accumulateHits[j][k][m] = 0;

    for (int irun = 0; irun<nfiles; irun++) {  //loop again over input files
      char filename[1000];
      sprintf(filename, "%s", argv[irun+2]); 
      FILE *f = fopen( filename, "r");
      if(!f) continue;			
      int tmp[10];
      int count(0);
     // read the first line in this file
      fscanf(f, "%d %d %d %d %d %d %d %d %d\n", tmp, tmp+1, tmp+2, tmp+3, tmp+4, tmp+5, tmp+6, tmp+7, tmp+8); 
      // read the rest of the lines in this file
      while (fscanf(f, "%d %d %d %d %d %d %d %d %d\n", tmp, tmp+1, tmp+2, tmp+3, tmp+4, tmp+5, tmp+6, tmp+7, tmp+8)==9) { 

	if(tmp[0]<0 && side==1) continue; //only count A side
	if(tmp[0]>0 && side==0) continue; //only count C side
        count++;
				
        for (int k=0; k<6; k++) m_accumulateHits[tmp[1]][tmp[2]][k] += tmp[3+k];

      } 
      fclose(f);
      printf("read %7d lines from file %s \n", count, argv[irun+2]);
    } // end loop over input files


		
    // write counts to merged output file
    for (int j=0; j<32; j++) for (int k=0; k<5482; k++) {
      int bec = 2*side -1;
      if (k>=1642) bec *= 2;
      fprintf(fout, "%d %d %d", bec, j, k);
      for (int m=0; m<6; m++) fprintf(fout, " %d", m_accumulateHits[j][k][m] ); fprintf(fout, "\n");
    }
  } //end loop over the two sides

  fclose(fout);
  printf("closed merged file: %s\n", argv[1]);
  return 0;
}

