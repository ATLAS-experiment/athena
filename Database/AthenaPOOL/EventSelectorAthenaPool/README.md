# EventSelectorAthenaPool

AthenaPool is a toolkit to support use of LCG POOL as a persistence technology in Athena.
It consists of several packages: PoolSvc, AthenaPoolCnvSvc, AthenaPoolUtilities,
EventSelectorAthenaPool (this one), OutputStreamAthenaPool, RootConversions, and StlAthenaPoolCnv.

The EventSelectorAthenaPool package reimplements the Gaudi IEvtSelector interface and more.
The package also contains equivalent support (via CondProxyProvider) for
conditions data input from POOL.

## Configuration

The component-level configuration functions are provided by this package:

- `EventSelectorAthenaPool.EventSelectorAthenaPoolConfig.EventSelectorAthenaPoolCfg`
  configures the event selector, including shared I/O and run-number overrides.
  With `flags.Input.SecondaryFiles`, it instead configures a
  `DoubleEventSelectorAthenaPool` named `EventSelector` and an
  `EventSelectorAthenaPool` named `SecondaryEventSelector`, preserving the overlay
  run-number and event-skipping settings.
- `EventSelectorAthenaPool.AthenaPoolAddressProviderSvcConfig.AthenaPoolAddressProviderSvcCfg`
  configures the address provider and its supporting services. With secondary
  files, it registers separate primary and secondary address providers with
  `ProxyProviderSvc`. Their names and selector keys are fixed; other keyword
  arguments apply to both providers.

These replace the imports from `AthenaPoolCnvSvc.PoolReadConfig` and
`AthenaPoolCnvSvc.PoolCommonConfig`, respectively. The higher-level
`AthenaPoolCnvSvc.PoolReadConfig.PoolReadCfg` remains the entry point for configuring
POOL file reading.

## Examples

The package Database/AthenaPOOL/AthenaPoolExample contains running examples of algorithms writing and
reading Data Objects using AthenaPool.

## Author

Peter van Gemmeren <gemmeren@anl.gov>