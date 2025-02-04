#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SGInputLoaderCfg(flags, Load=None, **kwargs):
    if Load:
        processed = set()
        for item in Load:
            if isinstance(item, tuple):
                type_name, key = item
            elif isinstance(item, str):
                if '/' in item:
                    type_name, key = item.split('/')
                elif '#' in item:
                    type_name, key = item.split('#')
                else:
                    raise ValueError("String entry should be a type-key pair split by '/' or '#'")
            else:
                raise ValueError('Unsupported type')

            # Append 'StoreGateSvc' by default
            if '+' not in key:
                key = f'StoreGateSvc+{key}'

            processed.add((type_name, key))

        kwargs.setdefault('Load', processed)

    acc = ComponentAccumulator()
    alg = CompFactory.SGInputLoader(**kwargs)
    if not flags.Common.isOnline and not any(flags.Input.Files) or flags.Input.Files==['_ATHENA_GENERIC_INPUTFILE_NAME_']:
        # eventloopmgr provides the EventInfo for inputless jobs, so add this as an extra output
        # TODO: Would like to remove legacy EventInfo from Athena entirely in the future, then would remove this here too.
        alg.ExtraOutputs.add( ('EventInfo', 'StoreGateSvc+McEventInfo') )
    acc.addEventAlgo(alg, primary=True)

    return acc
