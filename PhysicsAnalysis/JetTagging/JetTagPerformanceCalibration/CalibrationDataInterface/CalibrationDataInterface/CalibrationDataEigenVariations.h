/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////
// CalibrationDataEigenVariations.h, (c) ATLAS Detector software
//////////////////////////////////////////////////////////////////////

#ifndef ANALYSISCALIBRATIONDATAINTERFACEEVVARIATIONS_H
#define ANALYSISCALIBRATIONDATAINTERFACEEVVARIATIONS_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include "TMatrixD.h"
#include "TMatrixDSym.h"
#include "CxxUtils/checker_macros.h"

class TH1;

namespace Analysis
{
  class CalibrationDataHistogramContainer;

  class CalibrationDataEigenVariations {
  public:
    typedef std::set<size_t> IndexSet;
    typedef std::set<IndexSet> IndexSuperSet;

    /** normal constructor. The second argument, if true, will attempt to retrieve a 'recommended' set of uncertainties to be excluded from EV decomposition */
    CalibrationDataEigenVariations(const std::string& cdipath, const std::string& tagger, const std::string& wp, const std::string& jetcollection, CalibrationDataHistogramContainer* cnt, bool excludeRecommendedUncertaintySet = false, bool base = true); 
    virtual ~CalibrationDataEigenVariations();

    /** exclude the source of uncertainty indicated by  name  from eigenvector calculations */
    void excludeNamedUncertainty(const std::string& name, CalibrationDataContainer* cnt); 
    /** carry out the eigenvector computations. Exclusion of any source of uncertainty has to
	be done before calling this method */
    virtual void initialize(double min_variance = 1.0E-20); // <------ this default value for min_variance can be tuned - some combinations of CDI file and systematic strategy can use a higher threshold

    /** remove all variations in the given set */
    void removeVariations(const IndexSet &set); 
    /** remove all variations in any of the given sets */
    void removeVariations(const IndexSuperSet &set); 

    /** merge all variations starting from the given index */
    void mergeVariationsFrom(const size_t& index); 
    
    /** merge all variations in the given set */
    void mergeVariations(const IndexSet &set); 
    /** merge all variations in any of the given sets */
    void mergeVariations(const IndexSuperSet &set); 

    /** retrieve the number of named variations */
    unsigned int getNumberOfNamedVariations() const;
    /** list the named variations */
    std::vector<std::string> listNamedVariations() const; 
    /** retrieve the integer index corresponding to the named variation.
	This can be used to speed up computations by avoiding string comparisons.
        Note that this function is not protected against passing an invalid name. */
    unsigned int getNamedVariationIndex(const std::string& name) const;

    /** retrieve the number of eigenvector variations */
    unsigned int getNumberOfEigenVariations();

    /** obtain the "up" and "down" variations for the given eigenvector number.
	The return value will be false if the eigenvector number is invalid. */
    bool getEigenvectorVariation(unsigned int variation, TH1*& up, TH1*& down);

    /** obtain the "up" and "down" variations for the named uncertainty.
	The return value will be false if the given name is not listed as
	being excluded from the eigenvector calculations. */
    bool getNamedVariation(const std::string& name, TH1*& up, TH1*& down);
    /** obtain the "up" and "down" variations for the source uncertainty
	pointed to by the given index (which is assumed to correspond to the
	value retrieved using getNamedVariationIndex()).
	The return value will be false if the index is out of bounds. */
    bool getNamedVariation(unsigned int nameIndex, TH1*& up, TH1*& down);
    /** flag whether the given index corresponds to an extrapolation variation */
    bool isExtrapolationVariation(unsigned int nameIndex) const;

    /** also provide (some) access to the underlying information:
	covariance matrix corresponding to eigenvector variations */
    virtual TMatrixDSym getEigenCovarianceMatrix(); 
    /** covariance matrix corresponding to eigenvector variations constructed from
	the eigen-variation */
    TMatrixDSym getEigenCovarianceMatrixFromVariations();
    /** matrix to remove unecessary rows and columns from covariance */
    TMatrixD    getJacobianReductionMatrix();

    /** Eigenvector recomposition method.*/
    bool EigenVectorRecomposition(const std::string& label,
				  std::map<std::string, std::map<std::string, float>> &coefficientMap);

    void setVerbose(bool);

  private:
    /** container object containing the basic information */
    CalibrationDataHistogramContainer* m_cnt;
  
  protected:

    /** flag whether the initialization has been carried out */
    bool m_initialized;
    bool m_validate; // <------ flag that says you want to validate the systematic strategies, and the merging schemes

    /** named variations */
    std::map<std::string, unsigned int> m_namedIndices;
    std::vector<std::pair<TH1*, TH1*> > m_named; // <---- In the CalibrationDataGlobalEigenVariations, this will store regular variations as well

    /** named variation index for the special case of extrapolation uncertainties */
    int m_namedExtrapolation;

    /** eigenvector variations */
    std::vector<std::pair<TH1*, TH1*> > m_eigen; // <----- In the CalibrationDataGlobalEigenVariations, this will store the **combined** variations FOR PRUNING ONLY ("reporting" is done differently)

    /** indicate whether statistical uncertainties are stored as variations */
    bool m_statVariations;
    
    // /** @ data members needed for eigenvector method **/
    // /** the map stores the int which is needed to access the other vector<> objects **/
    // mutable std::map<std::string, unsigned int> m_eigenvectorMethod_index;
    // std::vector<TMatrixT<double> > m_eigenvectorMethod_matrix;
    // std::vector<std::vector<TObject*> > m_eigenvectorMethod_uncUpProvider;
    // std::vector<std::vector<TObject*> > m_eigenvectorMethod_uncDownProvider;
    // std::vector<std::vector<TObject*> > m_eigenvectorMethod_uncProvider;

    std::string m_cdipath;
    std::string m_taggername;
    std::string m_wp;
    std::string m_jetauthor;
    // The reduction scheme can be said to "capture" X % of the variance by computing m_capturedvariance/m_totalvariance
    // total variance is the sum of all the eigenvalues found through eigenvector decomposition
    // the "captured" variance is the sum of the eigenvalue of the retained eigenvectors (after pruning)
    double m_totalvariance;
    double m_capturedvariance; 

    bool m_verbose;

  };



  class CalibrationDataGlobalEigenVariations : public CalibrationDataEigenVariations {
    public:
      CalibrationDataGlobalEigenVariations(const std::string& cdipath, const std::string& tagger, const std::string& wp, const std::string& jetcollection, const std::vector<std::string>& flavours, CalibrationDataHistogramContainer* cnt, bool excludeRecommendedUncertaintySet = false) ;
      ~CalibrationDataGlobalEigenVariations();

      using CalibrationDataEigenVariations::getEigenCovarianceMatrix;
      TMatrixDSym getEigenCovarianceMatrix();

      void initialize(double min_variance = 1.0E-6) ; // <------ this min_variance threshold value holds generally for the global eigenvariations strategy - but not necessarily set in stone, can depend on CDI file as well

      TMatrixD getJacobianReductionMatrix(TMatrixDSym& cov);

      void excludeNamedUncertainty(const std::string& name, const std::string& flavour);

      std::vector<std::string> listNamedVariations(const std::string& flavour) const;
      unsigned int getNamedVariationIndex(const std::string& name, const std::string& flavour) const;
      bool getNamedVariation(const std::string& flavour, const std::string& name, TH1*& up, TH1*& down); // <--- Get the named variation index by flavour and variation name
      bool getNamedVariation(unsigned int nameIndex, const std::string& flavour, TH1*& up, TH1*& down); // <--- This does the actual retrieval, just also enforces that initialization was done first
      
      bool getEigenvectorVariation(const std::string& flavour, unsigned int variation, TH1*& up, TH1*& down);
      unsigned int getNumberOfEigenVariations(const std::string& flavour);
      bool isExtrapolationVariation(unsigned int nameIndex, const std::string& flavour) const;

      void mergeVariationsFrom(const size_t& index, std::string& flav);
      
      void mergeVariations(const IndexSet &set, std::string& flav);
      void mergeVariations(const IndexSuperSet &set, std::string& flav);

      void removeVariations(const IndexSet &set, std::string& flav);
      void removeVariations(const IndexSuperSet &set, std::string& flav);
    
    private:
      
      int m_blockmatrixsize; // store the concatenated length of all binned flavour calibrations, i.e. dim(B) + dim(C) + dim(Light) + dim(T)
      
      std::map<std::string, Analysis::CalibrationDataHistogramContainer*> m_histcontainers; // map of flavour to container, useful for when you need to actually quote a variation! (correct binning here per flavour)
      std::set<std::string> m_all_shared_systematics;
      std::set<std::string> m_only_shared_systematics;
      std::map<std::string, std::vector<int>> m_flavour_combinations;
      

      std::map<std::string, std::map<std::string, unsigned int>> m_flav_namedIndices; // replace the m_namedIndices, still can use m_named to store? Yes.
      std::map<std::string, std::vector<std::pair<TH1*,TH1*>>> m_flav_named; // On second second thought, this is needed to keep indices consistent between flavours...
      std::map<std::string, int> m_flav_namedExtrapolation; // store the index of the flavour container's extrapolation named uncertainty index - replaces m_namedExtrapolation
      std::map<std::string, std::vector<std::pair<TH1*, TH1*>>> m_flav_eigen; // replace m_eigen when it comes to RETURNING VARIATIONS with proper binning (i.e. the "reporting" of up/down variations per flavour)
      std::vector<std::string> m_flavours;

  };

}

#endif // ANALYSISCALIBRATIONDATAINTERFACEEVVARIATIONS_H
