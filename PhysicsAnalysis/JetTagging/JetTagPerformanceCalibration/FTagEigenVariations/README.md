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

## Reduction

`ftag::ev::reduce` keeps the leading `m - 1` eigenvariations and replaces the others by
one variation, so that Medium and Tight analyses get `m` nuisance parameters. In every bin
`u`, the merged variation is

    e_merged(u) = s(u) * sqrt(sum_{j merged} e_j(u)^2)

with `s(u)` the sign of the leading merged eigenvariation in that bin. Every bin keeps its
full uncertainty, while the correlations between bins are approximated. Loose keeps all
eigenvariations, which is the same as `m` at least their number.

`ftag::ev::reductionQuality` compares any set of variations with the full covariance: the
uncertainty kept in each bin, and the largest change of a bin-to-bin correlation.

## Recomposition

`ftag::ev::recompose` writes each source as a combination of the variations, by least
squares over all variations together. The coefficients then express each eigenvector
nuisance parameter in terms of the sources. Without reduction and without statistical
uncertainty this is exact. For each variation, `describedFraction` is the sum of its squared
coefficients: without reduction it is 1 minus its statistical part, and after a merge it can
also exceed 1, since the merged variation is not independent of the kept ones.

## Names

`ftag::ev::eigenName` and `ftag::ev::sourceName` give the systematic names the
BTaggingEfficiencyTool uses: `FT_EFF_Eigen_<flavour>_<k>`, and `FT_EFF_<source>` with spaces
replaced by `_`.

## Tests

The unit tests in `test/` check the decomposition, reduction, recomposition and names on
small inputs with known answers. Run them with `ctest -R gt_Eigen` from the build
directory.
