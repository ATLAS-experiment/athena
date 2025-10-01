# PoolSvc

PoolSvc provides mechanisms for POOL connection management, input and output
catalog specification, and querying and control of POOL configuration options.
PoolSvc is used via its IPoolSvc interface.

## Overview

AthenaPool is a toolkit to support use of LCG POOL as a persistence technology in Athena.
It consists of several packages: PoolSvc (this one), AthenaPoolCnvSvc, AthenaPoolUtilities,
EventSelectorAthenaPool, OutputStreamAthenaPool, RootConversions, and StlAthenaPoolCnv.

## Examples

The package Database/AthenaPOOL/AthenaPoolExample contains running examples of algorithms writing and
reading Data Objects using PoolSvc.

## Author

Peter van Gemmeren <gemmeren@anl.gov>