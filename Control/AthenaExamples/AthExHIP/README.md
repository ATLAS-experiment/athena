# HIP Example Package

This package is meant to hold code demonstrating how to use the HIP language directly
from Athena.

For the package to "do anything", HIP needs to be available in the build
environment. If you have a HIP compiler present this should be detected at cmake time, e.g.
```
-- Looking for a HIP compiler
-- Looking for a HIP compiler - /opt/rocm-6.3.2/lib/llvm/bin/clang++
-- The HIP compiler identification is Clang 18.0.0
-- Detecting HIP compiler ABI info
-- Detecting HIP compiler ABI info - done
-- Check for working HIP compiler: /opt/rocm-6.3.2/lib/llvm/bin/clang++ - skipped
-- Detecting HIP compile features
-- Detecting HIP compile features - done
```

The unit test will then be available to run after compilation, or directly as
python python/LinearTransformExampleConfig.py
