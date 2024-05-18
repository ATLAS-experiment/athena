/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_VERSIONS_TRACKSUMMARY_V1_H
#define XAODTRACKING_VERSIONS_TRACKSUMMARY_V1_H
#include <cstdint>
#include <vector>
#include "AthLinks/ElementLink.h"
#include "AthContainers/AuxElement.h"
#include "EventPrimitives/EventPrimitives.h"
#include "xAODTracking/TrackingPrimitives.h"


namespace xAOD
{
  /**
   * @brief Track Summary for Acts MultiTrajectory
   **/

  class TrackSummary_v1 : public SG::AuxElement
  {
  private:
    static const SG::AuxElement::Accessor<std::vector<double> > s_paramsAcc, s_covParamsAcc;
  public:
    TrackSummary_v1() = default;
    /**
     * access track backend vector of const element
     **/

    template <std::size_t measdim = 6>
    Eigen::Map<const Eigen::Matrix<double, measdim, 1>> paramsEigen() const
    {
      return Eigen::Map<const Eigen::Matrix<double, measdim, 1>>{s_paramsAcc(*this).data()};
    }
    /**
     * access parameters of non const element
     **/
    template <std::size_t measdim = 6>
    Eigen::Map<Eigen::Matrix<double, measdim, 1>> paramsEigen()
    {
      return Eigen::Map<Eigen::Matrix<double, measdim, 1>>{s_paramsAcc(*this).data()};
    }

    /**
     * access track parameters as plain vector
     **/
    const std::vector<double> &params() const;
    /**
     * access set parameters from plain vector
     **/
    void setParams(const std::vector<double> &m);

    /**
     * access track covariance matrix (flattened, rows layout) of const element
     **/
    template <std::size_t measdim = 6>
    Eigen::Map<const Eigen::Matrix<double, measdim, measdim>> covParamsEigen() const
    {
      return Eigen::Map<const Eigen::Matrix<double, measdim, measdim>>{s_covParamsAcc(*this).data()};
    }

    /**
     * access track covariance matrix (flattened, rows layout)
     **/
    template <std::size_t measdim = 6>
    Eigen::Map<Eigen::Matrix<double, measdim, measdim>> covParamsEigen()
    {
      return Eigen::Map<Eigen::Matrix<double, measdim, measdim>>{s_covParamsAcc(*this).data()};
    }

    /**
     * access track covariance as plain vector
     **/
    const std::vector<double> &covParams() const;
    /**
     * access set covariance from plain vector
     **/
    void setCovParams(const std::vector<double> &m);

    /**
     * access nMeasurements
     **/
    unsigned int nMeasurements() const;
    /**
     * access set nMeasurements
     **/
    void setnMeasurements(unsigned int m);
    /**
     * @brief pointers API needed by MTJ
     */
    const unsigned int* nMeasurementsPtr() const;
    unsigned int* nMeasurementsPtr();

    /**
     * access nHoles
     **/
    unsigned int nHoles() const;
    /**
     * access set nHoles
     **/
    void setnHoles(unsigned int m);
    /**
     * @brief pointers API needed by MTJ
     */
    const unsigned int* nHolesPtr() const;
    unsigned int* nHolesPtr();



    /**
     * access chi2
     **/
    float chi2f() const;
    /**
     * access set chi2
     **/
    void setChi2f(float m);
    /**
    * @brief pointers API needed by MTJ
    */
    const float* chi2fPtr() const;
    float* chi2fPtr();

    /**
     * access ndf
     **/
    unsigned int ndf() const;
    /**
     * access set ndf
     **/
    void setNdf(unsigned int m);
    /**
     * @brief pointers API needed by MTJ
     */
    const unsigned int* ndfPtr() const;
    unsigned int* ndfPtr();


    /**
     * access nOutliers
     **/
    unsigned int nOutliers() const;
    /**
     * access set nOutliers
     **/
    void setnOutliers(unsigned int m);
    /**
     * @brief pointers API needed by MTJ
     */
    const unsigned int* nOutliersPtr() const;
    unsigned int* nOutliersPtr();

    /**
     * access nSharedHits
     **/
    unsigned int nSharedHits() const;
    /**
     * access set nSharedHits
     **/
    void setnSharedHits(unsigned int m);
    /**
     * @brief pointers API needed by MTJ
     */
    const unsigned int* nSharedHitsPtr() const;
    unsigned int* nSharedHitsPtr();


    /**
      * @brief index of the tip of the TrackStates linked list in MultiTrajectory
      */
    unsigned int tipIndex() const;

    /**
      * @brief Set the tip index
      * @see tipIndex() for explanation
      */
    void setTipIndex(unsigned int);

    /**
      * @brief pointers API needed by MTJ
      */
    const unsigned int* tipIndexPtr() const;
    unsigned int* tipIndexPtr();

    /**
      * @brief index of the stem of the TrackStates linked list in MultiTrajectory
      * steemIndex is needed reverse lookup in MultiTrajectory
      */
    unsigned int stemIndex() const;

    /**
      * @brief Set the stem index
      * @see stemIndex() for explanation
      */
    void setStemIndex(unsigned int);

    /**
      * @brief pointers API needed by MTJ
      */
    const unsigned int* stemIndexPtr() const;
    unsigned int* stemIndexPtr();


    /**
      * @brief index of the surfaces in the surfaces collection
      */
    unsigned int surfaceIndex() const;

    /**
      * @brief Set the index in surface container
      */
    void setSurfaceIndex(unsigned int);



    /**
    * particle hypothesis access
    */
    const uint8_t& particleHypothesis() const;
    void setParticleHypothesis(const uint8_t& ); 

    /**
     * @brief resize internal arrays to store params (to capacity sz) & convariances (to capacity sz x sz)
     */
    void resize(size_t sz = 6);

    /**
     * @brief retrieve the size of the internal vectors for the data summary
     */
    size_t size() const;
  };
}
#endif
