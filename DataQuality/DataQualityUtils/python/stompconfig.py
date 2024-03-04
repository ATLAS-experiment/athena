# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


def config():
    import os
    with open(os.environ.get('DQU_MQINFO', '/afs/cern.ch/user/a/atlasdqm/atlas/mqinfo')) as f:
        return {'username': 'atlasdqm',
                'passcode': f.read().strip()}
    raise RuntimeError('Unable to read STOMP connection info')
