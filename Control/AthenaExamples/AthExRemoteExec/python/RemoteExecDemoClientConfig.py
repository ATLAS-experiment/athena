# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""An ordinary Athena job that calls the demonstration fragment.

This is the shape the whole exercise is about: a job with a normal event loop,
normal algorithms and normal typed handles, one of whose algorithms happens to
run somewhere else. Nothing in it is written twice -- the boundary is declared
once, by the server's fragment, and imported here.

The order is the mirror image of the server's::

    DemoNumbersAlg   writes ordinary integers                   (knows nothing)
    RemoteExecRequestAlg    encodes them, sends, decodes the reply
    DemoCheckAlg     reads ordinary integers, asserts the answer (knows nothing)

Turning those integers into a message and the reply back into integers is the
boundary's work rather than the client's, so the client is the work it does and
the one call. The codecs either side of the socket are the same configured
components, built from the server's own fragment declaration, which is what
makes the two ends unable to disagree about what the bytes mean -- and is the
property a server whose code is a different implementation cannot have.
"""

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from McEventSelector.McEventSelectorConfig import McEventSelectorCfg

from AthExRemoteExec.RemoteExecFragments import RemoteExecRequestAlgCfg
from AthExRemoteExec.RemoteExecHiveLoopConfig import RemoteExecMenuCfg

__all__ = ["RemoteExecDemoClientCfg"]


def RemoteExecDemoClientCfg(flags, target, addends=(3, 4), **kwargs):
    """Call RemoteExecSeqSum once per event and check what comes back.

    :param target: "host:port", or the path of a file the server writes its
        bound port into -- which is how a test can start the two in either
        order and let the OS choose the port.
    :param addends: what to add to the event number for each of the two
        numbers, so that no two events send the same request. A server that
        answered from a cache, or answered the wrong request, would be caught by
        DemoCheckAlg rather than by the job succeeding.
    """
    cfg = MainServicesCfg(flags)
    cfg.merge(McEventSelectorCfg(flags))

    cfg.addEventAlgo(CompFactory.AthExRemoteExec.DemoNumbersAlg(
        "RemoteExecDemoNumbers", Values=["a", "b"], Offsets=list(addends)))

    # The server's own declaration of the fragment, imported rather than
    # restated. Everything the client needs -- which keys cross, in which
    # direction, on what encoding, against what schema, and with which codec
    # components -- comes out of this one object, so the two ends cannot
    # disagree about any of it.
    fragment = RemoteExecMenuCfg(flags).fragment("RemoteExecSeqSum")
    cfg.addEventAlgo(RemoteExecRequestAlgCfg("RemoteExecDemoRequest", fragment,
                                      target=target, **kwargs))

    cfg.addEventAlgo(CompFactory.AthExRemoteExec.DemoCheckAlg(
        "RemoteExecDemoCheck", Values=["a", "b"], Result="sum"))
    return cfg


if __name__ == "__main__":
    import sys
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Exec.MaxEvents = 10
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1

    parser = flags.getArgumentParser()
    parser.add_argument("--target", required=True,
                        help="host:port, or a file the server writes its port "
                             "into")
    parser.add_argument("--ready-timeout", type=float, default=120.0,
                        help="Seconds to wait for the server to answer")
    args = flags.fillFromArgs(parser=parser)
    flags.lock()

    cfg = RemoteExecDemoClientCfg(flags, args.target, ReadyTimeout=args.ready_timeout)
    sys.exit(0 if cfg.run().isSuccess() else 1)
