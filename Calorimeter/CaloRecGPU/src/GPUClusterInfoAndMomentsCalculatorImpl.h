//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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

namespace ClusterMomentsCalculator
{

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

      shift = (a_orig + b_orig + c_orig) / 3.f;
      a = a_orig - shift;
      b = b_orig - shift;
      c = c_orig - shift;
      scale = max(fabsf(a), max(fabsf(b), max(fabsf(c), max(fabsf(d_orig), max(fabsf(e_orig), fabsf(f_orig))))));
      if (scale == 0.f)
        {
          scale = 1.f;
        }
      a /= scale;
      b /= scale;
      c /= scale;
      d = d_orig / scale;
      e = e_orig / scale;
      f = f_orig / scale;
    }

    ///@brief Calculate shifted and scaled eigenvalues of the matrix, in ascending value.
    ///
    ///To get the actual eigenvalues, you should multiply by @c scale and then add @c shift.
    CUDA_HOS_DEV void get_eigenvalues(float & e_1, float & e_2, float & e_3) const
    {
      using namespace std;

      const float c_0 = a * b * c + 2.f * d * f * e - a * e * e - b * f * f - c * d * d;
      const float c_1 = a * b - d * d + a * c - f * f + b * c - e * e;
      const float c_2 = a + b + c;

      constexpr float inv_3 = 1.f / 3.f;

      const float c_2_over_3 = c_2 * inv_3;

      const float a_over_3 = max((c_2 * c_2_over_3 - c_1) * inv_3, 0.f);

      const float half_b = 0.5f * (c_0 + c_2_over_3 * (2.f * c_2_over_3 * c_2_over_3 - c_1));

      const float q = max(a_over_3 * a_over_3 * a_over_3 - half_b * half_b, 0.f);

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

      e_1 = c_2_over_3 - rho * (cos_theta + sqrt_3 * sin_theta);
      e_2 = c_2_over_3 - rho * (cos_theta - sqrt_3 * sin_theta);
      e_3 = c_2_over_3 + 2.f * rho * cos_theta;
    }

    CUDA_HOS_DEV static void cross_prod(float (&res)[3], const float a1, const float a2, const float a3, const float b1, const float b2, const float b3)
    {
      res[0] = a2 * b3 - a3 * b2;
      res[1] = a3 * b1 - a1 * b3;
      res[2] = a1 * b2 - a2 * b1;
    }

    CUDA_HOS_DEV static void cross_prod(float (&res)[3], const float (&x)[3], const float (&y)[3])
    {
      cross_prod(res, x[0], x[1], x[2], y[0], y[1], y[2]);
    }

    CUDA_HOS_DEV static float dot_prod(const float a1, const float a2, const float a3, const float b1, const float b2, const float b3)
    {
      return a1 * b1 + a2 * b2 + a3 * b3;
    }

    CUDA_HOS_DEV static float dot_prod(const float (&x)[3], const float (&y)[3])
    {
      return dot_prod(x[0], x[1], x[2], y[0], y[1], y[2]);
    }

    CUDA_HOS_DEV void extract_one(const float eigenvalue, float (&res)[3], float (&representative)[3]) const
    {
      using namespace std;

      const float diag_0 = a - eigenvalue;
      const float diag_1 = b - eigenvalue;
      const float diag_2 = c - eigenvalue;

      float vec_1[3], vec_2[3];

      if (fabsf(diag_0) > fabsf(diag_1) && fabsf(diag_0) > fabsf(diag_2))
        {
          representative[0] = diag_0;
          representative[1] = d;
          representative[2] = f;

          vec_1[0] = d;
          vec_1[1] = diag_1;
          vec_1[2] = e;

          vec_2[0] = f;
          vec_2[1] = e;
          vec_2[2] = diag_2;
        }
      else if (/*(fabsf(diag_0) <= fabsf(diag_1) || fabsf(diag_0) <= fabsf(diag_2)) &&*/ fabsf(diag_1) > fabsf(diag_2))
        {
          representative[0] = d;
          representative[1] = diag_1;
          representative[2] = e;

          vec_1[0] = f;
          vec_1[1] = e;
          vec_1[2] = diag_2;

          vec_2[0] = diag_0;
          vec_2[1] = d;
          vec_2[2] = f;

        }
      else /*if ((fabsf(diag_0) <= fabsf(diag_1) || fabsf(diag_0) <= fabsf(diag_2)) && fabsf(diag_1) <= fabsf(diag_2))*/
        {
          representative[0] = f;
          representative[1] = e;
          representative[2] = diag_2;

          vec_1[0] = diag_0;
          vec_1[1] = d;
          vec_1[2] = f;

          vec_2[0] = d;
          vec_2[1] = diag_1;
          vec_2[2] = e;
        }

      cross_prod(res, representative, vec_1);
      cross_prod(vec_1, representative, vec_2);
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
    
    static constexpr float s_typical_epsilon = 5e-4;

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

          if (d_min <= 2 * epsilon * d_1)
            {
#ifdef __CUDA_ARCH__
              const float base_norm = rnorm3df(res[j][0], res[j][1], res[j][2]);
#else
              const float base_norm = 1.f / hypot(res[j][0], res[j][1], res[j][2]);
#endif
              const float extra_factor = 1.f - dot_prod(res[k], res[j]);

              const float norm = base_norm / extra_factor;

              res[j][0] *= norm;
              res[j][1] *= norm;
              res[j][2] *= norm;
            }
          else
            {
              float extra_vector[3];

              extract_one(second_e, res[j], extra_vector);
            }

          cross_prod(res[1], res[2], res[0]);

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
          eigenvalues[0] = eigenvalues[0] * scale + shift;
          eigenvalues[1] = eigenvalues[1] * scale + shift;
          eigenvalues[2] = eigenvalues[2] * scale + shift;
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

  void calculateClusterPropertiesAndMoments(CaloRecGPU::EventDataHolder & holder,
                                            const CaloRecGPU::ConstantDataHolder & instance_data,
                                            const CMCOptionsHolder & options,
                                            const IGPUKernelSizeOptimizer & optimizer,
                                            const bool synchronize = false,
                                            CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream = {},
                                            const bool defer_instead_of_oversize = false);
}
#endif