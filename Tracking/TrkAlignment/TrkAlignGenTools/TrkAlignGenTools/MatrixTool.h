/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKALIGNGENTOOLS_MATRIXTOOL_H
#define TRKALIGNGENTOOLS_MATRIXTOOL_H

/** @file MatrixTool.h
    @class MatrixTool

    @brief Based on SiGlobalChi2Algs.  
    AlgTool used to create a large matrix and vector for storing first- and 
    second-derivative terms needed to solve for alignment parameters.  Also 
    provides for methods to add terms to existing matrix entries, and to 
    solve for alignment parameters by inverting matrix and multiplying by 
    vector.
    
    @author Robert Harrington <roberth@bu.edu>, Daniel Kollar <daniel.kollar@cern.ch>
    @date 1/5/08
*/

#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "TrkAlignInterfaces/IMatrixTool.h"

#include <TMatrixDSym.h>
#include "CLHEP/Matrix/SymMatrix.h"

#include <string>

class TString;

/** @class MatrixTool
    
    @brief AlgTool used to create a large matrix and vector for storing first- 
    and second-derivative terms needed to solve for alignment parameters.  
    Also provides for methods to add terms to existing matrix entries, and to 
    solve for alignment parameters by inverting matrix and multiplying by 
    vector.
    
    Uses the double inheritance structure; concrete tools should inherit from 
    this class and AlgTool with virtual public inheritance.   
*/

namespace Trk {

  class AlSymMatBase;
  class AlSymMat;
  class AlMat;
  class AlVec;

  class IAlignModuleTool;

  class MatrixTool : public AthAlgTool, virtual public IMatrixTool {    
  public:

    enum SolveOption { 
      NONE                 = 0, //!< not solve in any case (to be used when ipc)
      SOLVE                = 1, //!< solving after data accumulation (LAPACK)
      SOLVE_FAST           = 2, //!< Fast (Eigen method) solving after data accumulation
      DIRECT_SOLVE         = 3, //!< direct solving (LAPACK), already available matrix & vector
      DIRECT_SOLVE_FAST    = 4, //!< direct Fast (Eigen method) solving, already available matrix & vector
      DIRECT_SOLVE_CLUSTER = 5, //!< computation of alignment parameters from SCALAPAK already solved matrix
      SOLVE_ROOT           = 6, //!< computation using ROOT
      SOLVE_CLHEP          = 7  //!< computation using CLHEP
    };

    /** Constructor */
    MatrixTool(const std::string& type, const std::string& name,
	       const IInterface* parent);

    /** Virtual destructor */
    virtual ~MatrixTool();
    
    /** initialize */
    StatusCode initialize();
    
    /** initialize */
    StatusCode finalize();
    
    /** allocates memory for big matrix and big vector */
    StatusCode allocateMatrix(int nDoF=0); 
    
    /** reads/writes matrix entries from/to binary files as necessary*/
    void prepareBinaryFiles(int solveOption);

    /** adds first derivative to vector */
    void addFirstDerivatives(AlVec* vector);

    /** adds first derivative to vector for only some entries */
    void addFirstDerivatives(std::list<int,double>& derivatives);

    void addFirstDerivative(int irow, double firstderiv);  

    /** adds second derivatives to matrix */
    void addSecondDerivatives(AlSymMatBase* matrix);

    /** adds first derivative to vector for only some entries */
    void addSecondDerivatives(std::list<std::pair<int,int>,double >& derivatives);

    void addSecondDerivative(int irow, int icol, double secondderiv);


    /** accumulates derivates from files.  Flag decides if it is binary or TFiles */
    bool accumulateFromFiles();
    
    /** accumulates derivates from binary files */
    bool accumulateFromBinaries();

    /** solves for alignment parameters */
    int solve();

    
    /** Store Files in a tfile */
    void storeInTFile(const TString& filename);

    /** Store Files in a tfile */
    bool accumulateFromTFiles();

    void printModuleSolution(std::ostream & os, const AlignModule * module, const CLHEP::HepSymMatrix * cov) const;
    void printGlobalSolution(std::ostream & os, const CLHEP::HepSymMatrix * cov);
    void printGlobalSolution(std::ostream & os, const TMatrixDSym * cov);

  private:

    // private methods
    int solveROOT();
    int solveCLHEP();
    int solveLapack();
    int solveSparseEigen();
    int solveLocal();

    StatusCode spuriousRemoval();
    static int        fillVecMods();
    void       postSolvingLapack(AlVec * dChi2, AlSymMat * d2Chi2, AlVec &w, AlMat &z, int size);

    void writeHitmap();
    void readHitmaps();

    // private variables
    PublicToolHandle<IAlignModuleTool> m_alignModuleTool{
      this, "AlignModuleTool", "Trk::AlignModuleTool/AlignModuleTool"};

    /** matrix to contain second derivative terms to be used for alignment */
    AlSymMatBase* m_bigmatrix = nullptr;
    
    /** vector to contain first derivative terms to be used for alignment */
    AlVec* m_bigvector = nullptr;
    
    /** flag to use AlSpaMat for the big matrix (default is AlSymMat) */
    Gaudi::Property<bool> m_useSparse{this, "UseSparse", false};

    Gaudi::Property<bool> m_diagonalize{this, "Diagonalize", true,
      "run diagonalization instead of inversion"};
    Gaudi::Property<double> m_eigenvaluethreshold
      {this, "EigenvalueThreshold", 0., "cut on the minimum eigenvalue"};

    Gaudi::Property<int> m_solveOption
      {this, "SolveOption", NONE, "solving option"};
    Gaudi::Property<int> m_modcut{this, "ModCut", 0,
      "cut on the weak modes which number is <par_modcut"};
    Gaudi::Property<int> m_minNumHits{this, "MinNumHitsPerModule", 0,
      "cut on the minimum number of hits per module"};
    Gaudi::Property<int> m_minNumTrks{this, "MinNumTrksPerModule", 0,
      "cut on the minimum number of tracks per module"};
    Gaudi::Property<float> m_pullcut{this, "PullCut", 1.0,
      "pull cut for the automatic weak mode removal method"};
    Gaudi::Property<float> m_eigenvalueStep{this, "EigenvalueStep", 1e3,
      "eigenvalue step for the second pass in the automatic weak mode removal method"};
    Gaudi::Property<float> m_Align_db_step{this, "AlignCorrDBStep", 10.,
      "corr in the diagonal basis step for the third pass in the auto weak mode removal method"};
    
    Gaudi::Property<bool> m_calDet{this, "MatrixDet", false,
      "compute bigmatrix's determinant ?"};
    Gaudi::Property<bool> m_wSqMatrix{this, "WriteSquareMatrix", false,
      "write a triangular matrix by default (true: square format) ?"};
    Gaudi::Property<bool> m_writeMat{this, "WriteMat", true,
      "write big matrix and vector into files ?"};
    Gaudi::Property<bool> m_writeMatTxt{this, "WriteMatTxt", true,
      "also write big matrix and vector into txt files ?"};
    Gaudi::Property<bool> m_writeEigenMat{this, "WriteEigenMat", true,
      "write eigenvalues and eigenvectors into files ?"};
    Gaudi::Property<bool> m_writeEigenMatTxt{this, "WriteEigenMatTxt", true,
      "also write eigenvalues and eigenvectors into txt files ?"};
    Gaudi::Property<bool> m_writeModuleNames{this, "WriteModuleNames", false,
      "write module name instead of Identifier to vector file"};

    Gaudi::Property<bool> m_writeHitmap{this, "WriteHitmap", false,
      "write hitmap into file"};
    Gaudi::Property<bool> m_writeHitmapTxt{this, "WriteHitmapTxt", false,
      "write hitmap into text file"};
    Gaudi::Property<bool> m_readHitmaps{this, "ReadHitmaps", false,
      "accumulate hitymap from files"};

    Gaudi::Property<bool> m_writeTFile{this, "WriteTFile", false,
      "write out files to a root file"};
    Gaudi::Property<bool> m_readTFiles{this, "ReadTFile", false,
      "if True then files will be read from TFiles instead of Binary files"};

    Gaudi::Property<bool> m_runLocal{this, "RunLocalMethod", true,
      "Run solving using Local method"};

    double m_scale = -1.;        //!< scale for big matrix and vector normalization
    Gaudi::Property<bool> m_scaleMatrix{this, "ScaleMatrix", false,
      "scale matrix by number of hits before solving"};
    
    Gaudi::Property<double> m_softEigenmodeCut{this, "SoftEigenmodeCut", 0.,
      "add constant to diagonal to effectively cut on weak eigenmodes"};

    Gaudi::Property<double> m_removeSpurious{this, "RemoveSpurious", false,
      "run spurious removal"};

    Gaudi::Property<double> m_calculateFullCovariance
      {this, "CalculateFullCovariance", true,
       "calculate full covariance matrix for Lapack"};

    Gaudi::Property<std::string> m_pathbin{this, "PathBinName", "./",
      "path binary files (in/out)"};
    Gaudi::Property<std::string> m_pathtxt{this, "PathTxtName", "./",
      "path ascii files (in/out)"};
    Gaudi::Property<std::string> m_prefixName{this, "PrefixName", "",
      "prefix string to filenames"};

    Gaudi::Property<std::string> m_tfileName
      {this, "TFileName", "AlignmentTFile.root", "prefix string to filenames"};

    Gaudi::Property<std::string> m_scalaMatName
      {this, "ScalapackMatrixName", "eigenvectors.bin", "Scalapack matrix name"};
    Gaudi::Property<std::string> m_scalaVecName
      {this, "ScalapackVectorName", "eigenvalues.bin", "Scalapack vector name"};

    Gaudi::Property<std::vector<std::string>> m_inputMatrixFiles
      {this, "InputMatrixFiles", {"matrix.bin"},
       "input binary files containing matrix terms"};
    Gaudi::Property<std::vector<std::string>> m_inputVectorFiles
      {this, "InputVectorFiles", {"vector.bin"},
       "input binary files containing vector terms"};

    Gaudi::Property<std::vector<std::string>> m_inputHitmapFiles
      {this, "InputHitmapFiles", {"hitmap.bin"},
       "input binary files containing the hitmaps"};

    Gaudi::Property<std::vector<std::string>> m_inputTFiles
      {this, "InputTFiles", {"AlignmentTFile.root"},
       "input binary files containing matrix terms"};

    std::vector<int> m_activeIndices{}; //!< vector of indices which pass the min-hits cut
    int m_aNDoF = 0;                    //!< number of active DoF (size of m_activeIndices)

    Gaudi::Property<int> m_maxReadErrors{this, "MaxReadErrors", 10,
      "maximum number of reading TFile errors"};

    //To skip IBL or Pixel Alignment
    Gaudi::Property<bool> m_AlignIBLbutNotPixel
      {this, "AlignIBLbutNotPixel", false};
    Gaudi::Property<bool> m_AlignPixelbutNotIBL
      {this, "AlignPixelbutNotIBL", false};
    //To Skip Solving of SCT ECA Last Disk
    Gaudi::Property<bool> m_DeactivateSCT_ECA_LastDisk
      {this, "DeactivateSCT_ECA_LastDisk", false};

    //By Pixel DoF
    Gaudi::Property<bool> m_Remove_Pixel_Tx{this, "Remove_Pixel_Tx", false};
    Gaudi::Property<bool> m_Remove_Pixel_Ty{this, "Remove_Pixel_Ty", false};
    Gaudi::Property<bool> m_Remove_Pixel_Tz{this, "Remove_Pixel_Tz", false};
    Gaudi::Property<bool> m_Remove_Pixel_Rx{this, "Remove_Pixel_Rx", false};
    Gaudi::Property<bool> m_Remove_Pixel_Ry{this, "Remove_Pixel_Ry", false};
    Gaudi::Property<bool> m_Remove_Pixel_Rz{this, "Remove_Pixel_Rz", false};

    //By IBL DoF
    Gaudi::Property<bool> m_Remove_IBL_Tx{this, "Remove_IBL_Tx", false};
    Gaudi::Property<bool> m_Remove_IBL_Ty{this, "Remove_IBL_Ty", false};
    Gaudi::Property<bool> m_Remove_IBL_Tz{this, "Remove_IBL_Tz", false};
    Gaudi::Property<bool> m_Remove_IBL_Rx{this, "Remove_IBL_Rx", false};
    Gaudi::Property<bool> m_Remove_IBL_Ry{this, "Remove_IBL_Ry", false};
    Gaudi::Property<bool> m_Remove_IBL_Rz{this, "Remove_IBL_Rz", false};

  }; // end of class

} // End of namespace 



#endif // TRKALIGNGENTOOLS_MATRIXTOOL_H



