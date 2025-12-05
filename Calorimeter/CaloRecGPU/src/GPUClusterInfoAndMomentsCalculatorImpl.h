//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_GPUCLUSTERINFOANDMOMENTSCALCULATOR_CUDA_H
#define CALORECGPU_GPUCLUSTERINFOANDMOMENTSCALCULATOR_CUDA_H

#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/DataHolders.h"
#include "CaloRecGPU/Helpers.h"

#include "CaloRecGPU/IGPUKernelSizeOptimizer.h"

#include <cmath>
#include <type_traits>

namespace ClusterMomentsCalculator
{
  struct RealSymmetricMatrixSolverIterative
  //Following the Eigen implementation too...
  {
    float a, b, c, d, e, f, scale;
    // +--     --+
    // | a  d  f |
    // | d  b  e |
    // | f  e  c |
    // +--     --+

    CUDA_HOS_DEV RealSymmetricMatrixSolverIterative(const float a_orig, const float b_orig, const float c_orig, const float d_orig, const float e_orig, const float f_orig)
    {
      using namespace std;

      const float max_ab = max( fabsf(a_orig),  fabsf(b_orig) );
      const float max_cd = max( fabsf(c_orig),  fabsf(d_orig) );
      const float max_ef = max( fabsf(e_orig),  fabsf(f_orig) );
      scale = max(max_ab, max(max_cd, max_ef) );
      if (scale == 0.f)
        {
          scale = 1.f;
        }
      const float inv_scale = 1.0f / scale;
      a = a_orig * inv_scale;
      b = b_orig * inv_scale;
      c = c_orig * inv_scale;
      d = d_orig * inv_scale;
      e = e_orig * inv_scale;
      f = f_orig * inv_scale;
    }

    static constexpr float s_typical_tolerance = std::numeric_limits<float>::min();

    //Uses temp_diag to store the diagonal
    //and temp_mat to store the tridiagonalized form of the matrix
    CUDA_HOS_DEV void tridiagonalize(float (&temp_diag)[3], float (&temp_subdiag)[2], float (&temp_mat)[3][3], const float tolerance = s_typical_tolerance)
    {
      using namespace std;

      temp_diag[0] = a;
      if (f * f <= tolerance)
        {
          temp_diag[1] = b;
          temp_diag[2] = c;

          temp_subdiag[0] = d;
          temp_subdiag[1] = e;

          temp_mat[0][0] =    1.f;
          temp_mat[0][1] =    0.f;
          temp_mat[0][2] =    0.f;
          
          temp_mat[1][0] =    0.f;
          temp_mat[1][1] =    1.f;
          temp_mat[1][2] =    0.f;
          
          temp_mat[2][0] =    0.f;
          temp_mat[2][1] =    0.f;
          temp_mat[2][2] =    1.f;
        }
      else
        {
          const float beta = hypot(d, f);
          
          const float inv_beta = 1.f / beta;

          const float em_0_1 = d * inv_beta;
          const float em_0_2 = f * inv_beta;

          const float q_w_1 = 2 * em_0_1 * e;
          const float q_c_1 = fmaf(2 * em_0_1, e, -q_w_1);
          const float q_w_2 = em_0_2 * c;
          const float q_c_2 = fmaf(em_0_2, c, -q_w_2);
          const float q_w_3 = em_0_2 * b;
          const float q_c_3 = fmaf(em_0_2, b, -q_w_3);

          const float q = CaloRecGPU::Helpers::sum_kahan_babushka_neumaier(q_w_1, q_w_2, -q_w_3, q_c_1, q_c_2, -q_c_3);

          temp_diag[1] = fmaf( em_0_2, q, b);
          temp_diag[2] = fmaf(-em_0_2, q, c);

          temp_subdiag[0] = beta;
          temp_subdiag[1] = fmaf(-em_0_1, q, e);

          temp_mat[0][0] =    1.f;
          temp_mat[0][1] =    0.f;
          temp_mat[0][2] =    0.f;
          
          temp_mat[1][0] =    0.f;
          temp_mat[1][1] =  em_0_1;
          temp_mat[1][2] =  em_0_2;
          
          temp_mat[2][0] =    0.f;
          temp_mat[2][1] =  em_0_2;
          temp_mat[2][2] = -em_0_1;
        }
    }

    CUDA_HOS_DEV void compute_iteration(const int start,
                                        const int end,
                                        float (&temp_diag)[3],
                                        float (&temp_subdiag)[2],
                                        float (&temp_mat)[3][3])
    {
      const float td = (temp_diag[end - 1] - temp_diag[end]) * 0.5f;

      const float ee = temp_subdiag[end - 1];

      float mu = temp_diag[end];

      if (td == 0.f)
        {
          mu -= fabsf(ee);
        }
      else if (ee != 0.f)
        {
          const float ee_2 = ee * ee;

          const float h = hypot(td, ee);

          const float factor = td + h * ((td >= 0.f) - (td < 0.f));

          if (ee_2 == 0.f)
            {
              mu -= ee / (factor / ee);
            }
          else
            {
              mu -= ee_2 / factor;
            }
        }

      float x = temp_diag[start] - mu;
      float z = temp_subdiag[start];

      for (int k = start; k < end && z != 0.f; ++k)
        {
          float givens_c, givens_s;

           /*if (z == 0.f)
            {
              givens_c = ((x >= 0.f) - (x < 0.f));
              givens_s = 0.f;
            }
          else */if (x == 0.f)
            {
              givens_c = 0.f;
              givens_s = (z < 0.f) - (z >= 0.f);
            }
          else if (fabsf(x) >= fabsf(z))
            {
              const float t = z / x;
              const float u = hypot(1.f, fabsf(t)) * ((x >= 0.f) - (x < 0.f));
              
              givens_c = 1.f / u;
              givens_s = -t * givens_c;
            }
          else
            {
              const float t = x / z;
              const float u = hypot(1.f, fabsf(t)) * ((z >= 0.f) - (z < 0.f));
              
              givens_s = -1.f / u;
              givens_c = -t * givens_s;
            }

          const float sdk  = CaloRecGPU::Helpers::product_sum_cornea_harrison_tang(givens_s,
                                                                       temp_diag[k],
                                                                       givens_c,
                                                                       temp_subdiag[k]);
                                                              
          const float dkp1 = CaloRecGPU::Helpers::product_sum_cornea_harrison_tang(givens_s,
                                                                       temp_subdiag[k],
                                                                       givens_c,
                                                                       temp_diag[k + 1]);

          temp_diag[k] = CaloRecGPU::Helpers::product_sum_cornea_harrison_tang(givens_c,
                                                                   CaloRecGPU::Helpers::product_sum_cornea_harrison_tang(givens_c,
                                                                                                   temp_diag[k],
                                                                                                   -givens_s,
                                                                                                   temp_subdiag[k]),
                                                                   -givens_s,
                                                                   CaloRecGPU::Helpers::product_sum_cornea_harrison_tang(givens_c,
                                                                                                   temp_subdiag[k],
                                                                                                   -givens_s,
                                                                                                   temp_diag[k + 1])
                                                                 );
          temp_diag[k + 1] = CaloRecGPU::Helpers::product_sum_cornea_harrison_tang(givens_s, sdk,  givens_c, dkp1);
          temp_subdiag[k]  = CaloRecGPU::Helpers::product_sum_cornea_harrison_tang(givens_c, sdk, -givens_s, dkp1);

          if (k > start)
            {
              temp_subdiag[k - 1] = CaloRecGPU::Helpers::product_sum_cornea_harrison_tang(givens_c,
                                                                              temp_subdiag[k - 1],
                                                                              -givens_s,
                                                                              z);
            }

          x = temp_subdiag[k];

          if (k < end - 1)
            {
              z = -givens_s * temp_subdiag[k + 1];

              temp_subdiag[k + 1] *= givens_c;
            }

          //We could skip if (c, s) == (1, 0)
          //Also, apply on the right means
          //we have to consider -s instead of s.
          for (int i = 0; i < 3; ++i)
            {
              float & c_1 = temp_mat[k]    [i];
              float & c_2 = temp_mat[k + 1][i];
              
              const float c_1_old = c_1;
              const float c_2_old = c_2;
               
              c_1 = CaloRecGPU::Helpers::product_sum_cornea_harrison_tang(givens_c, c_1_old, -givens_s, c_2_old);
              c_2 = CaloRecGPU::Helpers::product_sum_cornea_harrison_tang(givens_s, c_1_old,  givens_c, c_2_old);
            }
        }
    }


    static constexpr int   s_typical_max_iterations = 90;
    static constexpr float s_typical_near_zero = std::numeric_limits<float>::min();
    static constexpr float s_typical_epsilon = std::numeric_limits<float>::epsilon();

    CUDA_HOS_DEV void compute(float (&temp_diag)[3],
                              float (&temp_subdiag)[2],
                              float (&temp_mat)[3][3],
                              const float near_zero = s_typical_near_zero,
                              const float epsilon   = s_typical_epsilon,
                              const int   max_iter  = s_typical_max_iterations)
    {
      int iter_count = 0;
      int start = 0, end = 2;

      const float precision_inv = 1.f / epsilon;

      while (end > 0)
        {
          for (int i = start; i < end; ++i)
            {
              if (fabsf(temp_subdiag[i]) < near_zero)
                {
                  temp_subdiag[i] = 0.f;
                }
              else
                {
                  const float scaled_subdiag = precision_inv * temp_subdiag[i];
                  if (scaled_subdiag * scaled_subdiag <= fabsf(temp_diag[i]) + fabsf(temp_diag[i + 1]))
                    {
                      temp_subdiag[i] = 0.f;
                    }
                }
            }

          while (end > 0 && temp_subdiag[end - 1] == 0.f)
            {
              --end;
            }

          if (end <= 0)
            {
              break;
            }

          ++iter_count;

          if (iter_count > max_iter)
            {
              //printf("OUT OF ITERS! %d %d\n", start, end);
              break;
            }

          start = end - 1;

          while (start > 0 && temp_subdiag[start - 1] != 0.f)
            {
              --start;
            }

          compute_iteration(start, end, temp_diag, temp_subdiag, temp_mat);
        }

      //No need to sort eigenvalues and eigenvectors:
      //we are going to check them all anyway.
    }

    ///@brief Get the full eigenvalues and eigenvectors for this matrix.
    ///
    ///If @p rescale_and_reshift_values is @c true, the eigenvalues are scaled and shifted back to
    ///their proper value, given the original matrix.
    CUDA_HOS_DEV void get_solution(float (&eigenvalues)[3],
                                   float (&eigenvectors)[3][3],
                                   const float tolerance = s_typical_tolerance,
                                   const float near_zero = s_typical_near_zero,
                                   const float epsilon   = s_typical_epsilon,
                                   const int   max_iter  = s_typical_max_iterations )
    {
      float temp_subdiag[2];

      tridiagonalize(eigenvalues, temp_subdiag, eigenvectors, tolerance);
      //eigenvalues and eigenvectors are used temporarily to store the diagonal and the matrix.

      compute(eigenvalues, temp_subdiag, eigenvectors, near_zero, epsilon, max_iter);

      eigenvalues[0] *= scale;
      eigenvalues[1] *= scale;
      eigenvalues[2] *= scale;
    }
  };

  struct RealSymmetricMatrixSolver
//Taken from the Eigen implementation of direct_selfadjoint_eigenvalues
  {
    float a, b, c, d, e, f, shift, scale;
    // +--     --+
    // | a  d  f |
    // | d  b  e |
    // | f  e  c |
    // +--     --+

    CUDA_HOS_DEV RealSymmetricMatrixSolver(const float a_orig, const float b_orig, const float c_orig, const float d_orig, const float e_orig, const float f_orig)
    {
      using namespace std;

      shift = CaloRecGPU::Helpers::sum_kahan_babushka_neumaier(a_orig, b_orig, c_orig) / 3.f;
      a = a_orig - shift;
      b = b_orig - shift;
      c = c_orig - shift;
      const float max_ab = max( fabsf(a),       fabsf(b)      );
      const float max_cd = max( fabsf(c),       fabsf(d_orig) );
      const float max_ef = max( fabsf(e_orig),  fabsf(f_orig) );
      scale = max(max_ab, max(max_cd, max_ef) );
      if (scale == 0.f)
        {
          scale = 1.f;
        }
      const float inv_scale = 1.0f / scale;
      a *= inv_scale;
      b *= inv_scale;
      c *= inv_scale;
      d = d_orig * inv_scale;
      e = e_orig * inv_scale;
      f = f_orig * inv_scale;
    }

    ///@brief Calculate shifted and scaled eigenvalues of the matrix, in ascending value.
    ///
    ///To get the actual eigenvalues, you should multiply by @c scale and then add @c shift.
    CUDA_HOS_DEV void get_eigenvalues(float & e_1, float & e_2, float & e_3) const
    {
      using namespace std;

      const float ab      = a * b;
      const float corr_ab = fmaf(a, b, -ab);
      const float dd      = d * d;
      const float corr_dd = fmaf(d, d, -dd);
      const float ac      = a * c;
      const float corr_ac = fmaf(a, c, -ac);
      const float ff      = f * f;
      const float corr_ff = fmaf(f, f, -ff);
      const float bc      = b * c;
      const float corr_bc = fmaf(b, c, -bc);
      const float ee      = e * e;
      const float corr_ee = fmaf(e, e, -ee);

      const float c_0 = fmaf(2 * d, f * e,
                             CaloRecGPU::Helpers::product_sum_cornea_harrison_tang(ab,  c, -a, ee) +
                             CaloRecGPU::Helpers::product_sum_cornea_harrison_tang(-b, ff, -c, dd)   );
      //Consider using some other strategy to select the best pairs
      //to minimize error here?

      const float c_1 = CaloRecGPU::Helpers::sum_kahan_babushka_neumaier(ab, -dd, ac, -ff, bc, -ee, corr_ab, -corr_dd, corr_ac, -corr_ff, corr_bc, corr_ee);

      const float c_2 = CaloRecGPU::Helpers::sum_kahan_babushka_neumaier(a, b, c);

      constexpr float inv_3 = 1.f / 3.f;

      const float c_2_over_3 = c_2 * inv_3;

      const float a_over_3 = max(fma(c_2, c_2_over_3, -c_1) * inv_3, 0.f);

      const float half_b = 0.5f * (fma(c_2_over_3, fma(2.f * c_2_over_3, c_2_over_3, -c_1), c_0));

      const float q = max(CaloRecGPU::Helpers::product_sum_cornea_harrison_tang(a_over_3, a_over_3 * a_over_3, -half_b, half_b), 0.f);

      //None of what we are doing here is the best choice.
      //We would need to perform a proper, formal, numerical analysis
      //to figure out all possible errors and so on.

      const float rho = sqrtf(a_over_3);

#ifdef __CUDA_ARCH__
      const float theta = atan2f(1.0f, rsqrtf(q) * half_b) * inv_3;
#else
      const float theta = atan2f(sqrtf(q), half_b) * inv_3;
#endif

#ifdef __CUDA_ARCH__
      float sin_theta, cos_theta;
      sincosf(theta, &sin_theta, &cos_theta);
#else
      const float sin_theta = sinf(theta);
      const float cos_theta = cosf(theta);
#endif

      const float sqrt_3 = sqrtf(3.f);

      e_1 = fma(-rho, fma( sqrt_3, sin_theta, cos_theta), c_2_over_3);
      e_2 = fma(-rho, fma(-sqrt_3, sin_theta, cos_theta), c_2_over_3);
      e_3 = fma( rho, 2.0f * cos_theta,                   c_2_over_3);
    }

    CUDA_HOS_DEV void extract_one(const float eigenvalue, float (&res)[3], float (&representative)[3]) const
    {
      using namespace std;

      const float diag[3] = { a - eigenvalue,
                              b - eigenvalue,
                              c - eigenvalue
                            };

      float vec_1[3], vec_2[3];

      int   max_diag  = -1;
      float max_value = -1.f;

      for (int i = 0; i < 3; ++i)
        {
          const float this_diag = fabsf(diag[i]);
          if (this_diag >= max_value)
            {
              max_diag  = i;
              max_value = this_diag;
            }
        }

      if (max_diag == 0)
        {
          representative[0] = diag[0];
          representative[1] = d;
          representative[2] = f;

          vec_1[0] = d;
          vec_1[1] = diag[1];
          vec_1[2] = e;

          vec_2[0] = f;
          vec_2[1] = e;
          vec_2[2] = diag[2];
        }
      else if (max_diag == 1)
        {
          representative[0] = d;
          representative[1] = diag[1];
          representative[2] = e;

          vec_1[0] = f;
          vec_1[1] = e;
          vec_1[2] = diag[2];

          vec_2[0] = diag[0];
          vec_2[1] = d;
          vec_2[2] = f;

        }
      else /*if (max_diag == 2)*/
        {
          representative[0] = f;
          representative[1] = e;
          representative[2] = diag[2];

          vec_1[0] = diag[0];
          vec_1[1] = d;
          vec_1[2] = f;

          vec_2[0] = d;
          vec_2[1] = diag[1];
          vec_2[2] = e;
        }

      CaloRecGPU::Helpers::corrected_cross_product(res, representative, vec_1);
      CaloRecGPU::Helpers::corrected_cross_product(vec_1, representative, vec_2);
      //Can safely override previous value...

#ifdef __CUDA_ARCH__
      const float norm_1 = rnorm3df(res[0], res[1], res[2]);
      const float norm_2 = rnorm3df(vec_1[0], vec_1[1], vec_1[2]);
#else
      const float norm_1 = 1.f / hypot(res[0], res[1], res[2]);
      const float norm_2 = 1.f / hypot(vec_1[0], vec_1[1], vec_1[2]);
#endif

      if (norm_1 <= norm_2)
        //Greater magnitude -> multiply by a smaller value
        {
          res[0] *= norm_1;
          res[1] *= norm_1;
          res[2] *= norm_1;
        }
      else
        {
          res[0] = vec_1[0] * norm_2;
          res[1] = vec_1[1] * norm_2;
          res[2] = vec_1[2] * norm_2;
        }
    }

    static constexpr float s_typical_epsilon = std::numeric_limits<float>::epsilon();

    ///@brief Calculate the eigenvectors of the matrix,
    ///       using the (possibly unscaled) eigenvalues @p e_1, @p e_2, @p e_3
    ///       (in ascending order of magnitude) and @p epsilon to guard against undefined cases.
    CUDA_HOS_DEV void get_eigenvectors(float (&res)[3][3], const float e_1, const float e_2, const float e_3, const float epsilon = s_typical_epsilon) const
    {
      using namespace std;

      if (e_3 - e_1 <= epsilon)
        {
          res[0][0] = 1.f;
          res[0][1] = 0.f;
          res[0][2] = 0.f;

          res[1][0] = 0.f;
          res[1][1] = 1.f;
          res[1][2] = 0.f;

          res[2][0] = 0.f;
          res[2][1] = 0.f;
          res[2][2] = 1.f;
        }
      else
        {
          const float d_0 = e_3 - e_2;
          const float d_1 = e_2 - e_1;

          const float d_min = min(d_0, d_1);
          const float d_max = max(d_0, d_1);

          int k, j;
          float first_e, second_e;

          if (d_0 > d_1)
            {
              k = 2;
              j = 0;

              first_e  = e_3;
              second_e = e_1;
            }
          else
            {
              k = 0;
              j = 2;

              first_e  = e_1;
              second_e = e_3;
            }

          extract_one(first_e, res[k], res[j]);
#if USE_ORIGINAL_EIGEN
          if (d_min <= 2 * epsilon * d1)
            {
#ifdef __CUDA_ARCH__
              const float base_norm = rnorm3df(res[j][0], res[j][1], res[j][2]);
#else
              const float base_norm = 1.f / hypot(res[j][0], res[j][1], res[j][2]);
#endif
              const float extra_factor = 1.f - CaloRecGPU::Helpers::corrected_dot_product(res[k], res[j]);

              const float norm = base_norm / extra_factor;

              res[j][0] *= norm;
              res[j][1] *= norm;
              res[j][2] *= norm;
            }
#else
          if (d_min <= 2 * epsilon * d_max)
            //Eigen has d0 <= 2 * eps <= d1, but d0 is overwritten while d1 isn't...
            {
              //Eigen speaks about ortho-normalization
              //but does eivecs.col(l) -= eivecs.col(k).dot(eivecs.col(l))*eivecs.col(l)
              //which... does not ortho-normalize anything.

              const float prod = CaloRecGPU::Helpers::corrected_dot_product(res[k], res[j]);

              res[j][0] -= res[k][0] * prod;
              res[j][1] -= res[k][1] * prod;
              res[j][2] -= res[k][2] * prod;

#ifdef __CUDA_ARCH__
              const float norm = rnorm3df(res[j][0], res[j][1], res[j][2]);
#else
              const float norm = 1.f / hypot(res[j][0], res[j][1], res[j][2]);
#endif
              res[j][0] *= norm;
              res[j][1] *= norm;
              res[j][2] *= norm;
            }
#endif
          else
            {
              float extra_vector[3];

              extract_one(second_e, res[j], extra_vector);
            }

          CaloRecGPU::Helpers::corrected_cross_product(res[1], res[2], res[0]);

#ifdef __CUDA_ARCH__
          const float norm = rnorm3df(res[1][0], res[1][1], res[1][2]);
#else
          const float norm = 1.f / hypot(res[1][0], res[1][1], res[1][2]);
#endif

          res[1][0] *= norm;
          res[1][1] *= norm;
          res[1][2] *= norm;
        }
    }

    ///@brief Get the full eigenvalues and eigenvectors for this matrix.
    ///
    ///If @p rescale_and_reshift_values is @c true, the eigenvalues are scaled and shifted back to
    ///their proper value, given the original matrix.
    CUDA_HOS_DEV void get_solution(float (&eigenvalues)[3], float (&eigenvectors)[3][3], bool rescale_and_reshift_values = true, const float epsilon = s_typical_epsilon)
    {
      get_eigenvalues(eigenvalues[0], eigenvalues[1], eigenvalues[2]);
      get_eigenvectors(eigenvectors, eigenvalues[0], eigenvalues[1], eigenvalues[2], epsilon);

      if (rescale_and_reshift_values)
        {
          eigenvalues[0] = fmaf(eigenvalues[0], scale, shift);
          eigenvalues[1] = fmaf(eigenvalues[1], scale, shift);
          eigenvalues[2] = fmaf(eigenvalues[2], scale, shift);
        }
    }
  };

  struct ClusterMomentCalculationOptions
  {
    bool  use_abs_energy;
    bool  use_two_gaussian_noise;
    bool  skip_invalid_clusters;
    float min_LAr_quality;
    float max_axis_angle;
    float eta_inner_wheel;
    float min_l_longitudinal;
    float min_r_lateral;
  };

  struct CMCOptionsHolder
  {
    CaloRecGPU::Helpers::CPU_object<ClusterMomentCalculationOptions> m_options;

    CaloRecGPU::Helpers::CUDA_object<ClusterMomentCalculationOptions> m_options_dev;

    void allocate()
    {
      m_options.allocate();
    }

    void sendToGPU(const bool clear_CPU = false);
  };

  void register_kernels(IGPUKernelSizeOptimizer & optimizer);

  constexpr unsigned int num_time_measurements = 11;

  void calculateClusterPropertiesAndMoments(CaloRecGPU::EventDataHolder & holder,
                                            const CaloRecGPU::ConstantDataHolder & instance_data,
                                            const CMCOptionsHolder & options,
                                            const IGPUKernelSizeOptimizer & optimizer,
                                            size_t (&times)[num_time_measurements],
                                            const bool synchronize = false,
                                            CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream = {});
}

#endif
