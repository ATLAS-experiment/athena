# EventSelectorAthenaPool

AthenaPool is a toolkit to support use of LCG POOL as a persistence technology in Athena.
It consists of several packages: PoolSvc, AthenaPoolCnvSvc, AthenaPoolUtilities,
EventSelectorAthenaPool (this one), OutputStreamAthenaPool, RootConversions, and StlAthenaPoolCnv.

The EventSelectorAthenaPool package reimplements the Gaudi IEvtSelector interface and more.
The package also contains equivalent support (via CondProxyProvider) for
conditions data input from POOL.

## Examples

The package Database/AthenaPOOL/AthenaPoolExample contains running examples of algorithms writing and
reading Data Objects using AthenaPool.

## Author

Peter van Gemmeren <gemmeren@anl.gov>