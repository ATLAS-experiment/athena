JetH5Writer Module
===============

This is a package to write event-wise HDF5 files.
Files are split into C++ libraries which can be used directly in C++ code and wrapper `Alg` components which can be configured via `ComponentAccumulator`.

If you are confused about the structure of HDF5 files, consider inspecting it with the `h5ls` utility included in `AthAnalysis`.
