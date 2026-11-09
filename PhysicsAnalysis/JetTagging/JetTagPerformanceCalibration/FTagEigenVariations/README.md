# FTagEigenVariations

New implementation of the eigenvector decomposition of the flavour tagging scale factor
uncertainties. It is developed next to the current one in `CalibrationDataInterface` and
`xAODBTaggingEfficiency`, which it will replace once validated.

The core library `FTagEigenVariationsLib` (namespace `ftag::ev`) depends only on Eigen. It
works on plain per-bin numbers, so any calibration format can feed it through an adapter.

## Decomposition

`ftag::ev::decompose` takes one or more flavour blocks (the per-flavour strategy uses one,
the global strategy all of them) and a list of uncertainty sources:

* each block gives the statistical covariance of its scale factors;
* each source gives its shift of the scale factor per bin, in every block it affects;
* a source is either correlated across all bins, or comes with a group id per bin: bins
  with the same id are correlated, bins with different ids are not. A grouped source is
  split into one source per group, named `<source>_bin<id>`.

The total covariance over all bins of all blocks is

    V = V_stat + sum_s d_s d_s^T

with `d_s` the shift vector of source `s`. Its eigenvectors `u_k` with eigenvalues
`lambda_k` give the independent variations `sqrt(lambda_k) u_k`, which add back up to `V`.

Conventions:

* eigenvalues are sorted in descending order;
* eigenvalues at or below `1e-12` times the largest one are dropped;
* the largest entry (in absolute value) of each `u_k` is positive;
* bins without any uncertainty are kept out of the solve, so the variations are exactly
  zero there;
* sources named in `excluded` are left out of the covariance;
* inconsistent input throws `std::invalid_argument`.

## Tests

`test/gt_EigenDecomposition.cxx` checks the decomposition on small inputs with known
answers. Run it with `ctest -R gt_EigenDecomposition` from the build directory.
