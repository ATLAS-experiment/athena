/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <unistd.h>
#include <memory>

using namespace std;

int main(int argc, char *argv[])
{

    int nfiles = argc - 2;
    cout << " TRT_StrawStatus_merge. nfiles= " << nfiles << endl;

    // nBarrelStraws (= 1642) + nEndcapStraws (= 3840) = 5482
    // 6 index refers to categories of accumulation: last index: 0 - all hits, 1 - hits on track, 2 - all HT (TR) hits, 3 - HT (TR) hits on track
    // 32 refers to the phi coordinate index
    //heap-allocated
    std::unique_ptr<int[][5482][6]> accumulateHits(new int[32][5482][6]);

    int nevents = 0; // we first need to read the first line in each file to get the total number of events
    for (int irun = 0; irun < nfiles; irun++)
    {
        std::string filename = argv[irun + 2];
        printf("reading file %s \n", filename.c_str()); // debug printout
        FILE *f = fopen(filename.c_str(), "r");
        if (f==nullptr)
        {
            fprintf(stderr," - file %s missing\n", filename.c_str());
            exit(1);
        }
        int tmp[10]{};
        int nItems = fscanf(f, "%d %d %d %d %d %d %d %d %d\n", tmp, tmp + 1, tmp + 2, tmp + 3, tmp + 4, tmp + 5, tmp + 6, tmp + 7, tmp + 8);
        if (nItems !=9){
          std::cerr<<"TRTCalib_StrawStatus_merge: Incorrect fline in "<<filename<<std::endl;
          continue;
        }
        nevents += tmp[8];
        fclose(f);
    }
    printf("read %d events from %d files\n", nevents, nfiles);

    FILE *fout = fopen(argv[1], "w"); // The output merged file.
    if (fout == nullptr)
    {
        fprintf(stderr,"could not open file %s for writing, EXIT", argv[1]);
        exit(1);
    }
    printf("writing to merged file: %s\n", argv[1]);
    for (int i = 0; i < 8; i++)
        fprintf(fout, "%d ", 0);
    fprintf(fout, "%d\n", nevents); // write total number of events in the first line

    for (int side = 0; side < 2; side++)
    { // merge separately on the two sides

        cout << " reset status for all straws on side " << side * 2 - 1 << endl;
        for (int j = 0; j < 32; j++)
            for (int k = 0; k < 5482; k++)
                for (int m = 0; m < 6; m++)
                    accumulateHits[j][k][m] = 0;

        for (int irun = 0; irun < nfiles; irun++)
        { // loop again over input files
            char filename[1000];
            snprintf(filename, 999, "%s", argv[irun + 2]);
            FILE *f = fopen(filename, "r");
            if (!f)
                continue;
            int tmp[10]{};
            int count(0);
            // read the first line in this file
            int nItems = fscanf(f, "%d %d %d %d %d %d %d %d %d\n", tmp, tmp + 1, tmp + 2, tmp + 3, tmp + 4, tmp + 5, tmp + 6, tmp + 7, tmp + 8);
            if (nItems !=9){
              std::cerr<<"TRTCalib_StrawStatus_merge: Incorrect line in "<<filename<<std::endl;
              continue;
            }
            // read the rest of the lines in this file
            while (fscanf(f, "%d %d %d %d %d %d %d %d %d\n", tmp, tmp + 1, tmp + 2, tmp + 3, tmp + 4, tmp + 5, tmp + 6, tmp + 7, tmp + 8) == 9)
            {

                if (tmp[0] < 0 && side == 1)
                    continue; // only count A side
                if (tmp[0] > 0 && side == 0)
                    continue; // only count C side
                count++;

                for (int k = 0; k < 6; k++)
                    //coverity[tainted_data]
                    accumulateHits[tmp[1]][tmp[2]][k] += tmp[3 + k];
            }
            fclose(f);
            printf("read %7d lines from file %s \n", count, argv[irun + 2]);
        } // end loop over input files

        // write counts to merged output file
        for (int j = 0; j < 32; j++)
            for (int k = 0; k < 5482; k++)
            {
                int bec = 2 * side - 1;
                if (k >= 1642)
                    bec *= 2;
                fprintf(fout, "%d %d %d", bec, j, k);
                for (int m = 0; m < 6; m++)
                    fprintf(fout, " %d", accumulateHits[j][k][m]);
                fprintf(fout, "\n");
            }
    } // end loop over the two sides

    fclose(fout);
    printf("closed merged file: %s\n", argv[1]);
    exit(0);
}
