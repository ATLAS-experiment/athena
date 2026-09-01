# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Declaring an RPC fragment in one place.

A fragment's boundary has to be described to several components: the gate
(which keys to unpack, with what encoding and schema), the pack algorithm
(which keys to read back, and where to stage the reply), and the loop manager's
menu. Writing those out by hand does not survive many fragments -- and the
failure mode is unpleasant, because a disagreement between the loop manager's
declared inputs and the gate's actual ones lets a request past the cheap
up-front check only to fail inside the scheduler.

So a fragment is declared once and everything else is derived from it. Two
parts of the declaration are derived, and one is not:

- the **input keys** are whatever the payload reads but does not produce. An
  unmet read has nowhere to come from except the client, so this is not a
  heuristic; it is the definition.
- the **StoreGate type** of every boundary comes from the handle that reads or
  writes it. ``ComponentAccumulator.getIO()`` reports type, key and mode for
  every handle in a CA, so the type is already stated in the one place that
  cannot drift from the code -- the algorithm's own handle declaration. It is
  needed for the scheduler dependencies, and for nothing else.
- the **encoding and schema** are stated by hand, per boundary, and there is no
  default. They are a wire contract, and deriving a wire format from a host
  type is how you get a boundary whose serialisation changes silently when
  somebody edits a handle. What a payload *is* on the wire is a decision, so
  the fragment makes it explicitly.

What is left to state besides that is the fragment's name and which of the keys
it produces should go back to the client. Neither is derivable: the second is a
choice, and an intermediate that stays in the store is a legitimate one.
"""

from AthenaCommon.CFElements import parOR, seqAND
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging

_msg = logging.getLogger("RpcFragments")

#: Stores whose contents a client never supplies: a conditions read is served
#: by AthCondSeq and a detector-store read by the geometry, so neither is an
#: unmet dependency in the sense that matters here.
_NON_EVENT_STORES = frozenset(("ConditionStore+", "DetectorStore+"))


def handlePath(key):
    """The full path of a key as getIO() reports it, e.g. 'StoreGateSvc+a'.

    getIO() gives a plain string for a handle left at its C++ default and a
    DataHandle for one the configuration set (ComponentAccumulator.py:305), so
    both have to be accepted.
    """
    return str(getattr(key, "Path", key))


def _split_store(path):
    """'StoreGateSvc+a' -> ('StoreGateSvc+', 'a'); 'a' -> ('', 'a')."""
    store, plus, key = path.rpartition("+")
    return store + plus, key


def _is_conditions(key, store):
    """Is this handle reading conditions rather than event data?

    Two ways to tell, and both are needed. A key can carry its store in the
    path, which is the easy case. But a ReadCondHandleKey whose configured key
    is a COOL folder -- '/Indet/Align' -- has no prefix at all, and taking it
    for event data is not a harmless mistake: it becomes a wire input, and the
    client is asked to send the alignment. The handle itself knows:
    AtlasSemantics.VarHandleKeySemantics sets IsCondition from the C++ handle
    type, which is the authority here rather than the shape of the string.
    """
    return store in _NON_EVENT_STORES or getattr(key, "IsCondition", False)


def deriveBoundaries(payload, outputs, request_key, boundaries=None,
                     extra_inputs=(), ignore_inputs=(), produces=()):
    """Work out which keys cross a fragment's boundary, and their types.

    :param payload: the payload ComponentAccumulator
    :param outputs: keys to return to the client
    :param request_key: the request descriptor's key, which every gate reads and
        which must never be mistaken for a wire input
    :param boundaries: ``{key: Codec}``, needed because a *converting* codec's
        boundary is a name on the wire rather than a StoreGate key. For those,
        the keys the payload actually reads and writes are the ones the codec
        materialises, and this is what maps them back to the boundary carrying
        them. Every other codec's boundary key is its StoreGate key and this
        changes nothing.
    :param extra_inputs: keys to treat as client inputs even though nothing in
        the payload reads them -- for a fragment whose consumer is added later
    :param ignore_inputs: unmet reads that are *not* wire inputs, because the
        payload tolerates their absence. "Unmet read" is otherwise a sound
        definition of a wire input, and this is the one thing it cannot see --
        an optional dependency looks exactly like a required one from the
        outside. Naming them here is a claim that the payload copes, so it is
        deliberately explicit rather than a heuristic about which types look
        optional.
    :param produces: keys the payload writes that ``getIO()`` cannot see.
        It reports single handles, ExtraInputs and ExtraOutputs, but not
        ``SG::VarHandleKeyArray`` properties -- so an algorithm writing through
        a WriteHandleKeyArray looks, from Python, as though it writes nothing,
        and its keys would be charged to the client as unmet reads. The
        scheduler is not fooled (AthCommonDataStore registers array handles as
        real dependencies after initialize), which is exactly why this shows up
        as a wrong boundary rather than as a broken job.
    :return: ``(inputs, types)``, where inputs is the derived list of client
        input keys and types maps every boundary key to its StoreGate type

    Raises ValueError if a declared output is not produced, if two consumers of
    one key disagree about its type, or if an input's type cannot be determined.
    """
    reads, writes, types = {}, set(), {}
    for handle in payload.getIO():
        store, key = _split_store(handlePath(handle["key"]))
        if not key or _is_conditions(handle["key"], store):
            continue
        if handle["mode"] == "W":
            writes.add(key)
            # A writer is authoritative: it names the concrete type, where a
            # reader may name a base class.
            types[key] = handle["type"]
        else:
            reads.setdefault(key, set()).add(handle["type"])

    stale_writes = [k for k in produces if k in writes]
    if stale_writes:
        raise ValueError(
            f"produces names {sorted(stale_writes)}, which the payload already "
            "declares in a way the derivation can see. Either the handle "
            "stopped being an array or the key is wrong; either way the "
            "exemption is now hiding nothing")
    writes.update(produces)

    # For a converting codec the boundary is a name on the wire and the objects
    # live under keys of the fragment's choosing, so translate those back before
    # deciding what is unmet. For everything else this map is empty.
    boundaries = dict(boundaries or {})
    carried = {}
    for boundary_key, codec in boundaries.items():
        for sg_key in codec.sgKeys(boundary_key):
            if sg_key != boundary_key:
                carried[sg_key] = boundary_key

    unmet = set(carried.get(k, k) for k in reads if k not in writes)
    stale = [k for k in ignore_inputs if k not in unmet]
    if stale:
        raise ValueError(
            f"ignore_inputs names {sorted(stale)}, which the payload does not "
            "leave unmet. Either the key is wrong or the payload changed; "
            "either way the exemption is now hiding nothing")
    inputs = sorted(unmet - {request_key} - set(ignore_inputs))
    inputs += [k for k in extra_inputs if k not in inputs]

    for key in inputs:
        codec = boundaries.get(key)
        if codec is not None and codec.schema_source != "derived":
            # This boundary states its own schema, or needs none, and the
            # scheduler learns the key from the codec -- so there is nothing
            # left for a StoreGate type to be wanted for.
            continue
        if key in types:
            continue
        candidates = reads.get(key, set())
        if len(candidates) != 1:
            raise ValueError(
                f"cannot derive the type of RPC input '{key}': "
                + (f"its consumers disagree ({sorted(candidates)})"
                   if candidates else "nothing in the payload reads it")
                + ". Declare it explicitly with types={'" + key + "': ...}")
        types[key] = next(iter(candidates))

    def _produced(key):
        codec = boundaries.get(key)
        wanted = codec.sgKeys(key) if codec is not None else [key]
        return all(sg_key in writes for sg_key in wanted)

    missing = [key for key in outputs if not _produced(key)]
    if missing:
        raise ValueError(
            f"the payload does not produce {missing}; it writes "
            f"{sorted(writes)}")
    return inputs, types


def boundarySpecs(declarations):
    """Boundaries as the loop manager's description of them.

    Only a service parses these; the algorithms are configured with codec tools
    and parse nothing. See RpcBoundary.h.
    """
    return [codec.spec(key, sg_type)
            for key, sg_type, codec in declarations]


def boundaryTools(prefix, declarations, direction):
    """One configured codec per boundary.

    Named from the algorithm and the key, so a message from a codec says which
    boundary of which fragment it came from.

    :param direction: ``"Write"`` for the instances that decode into the store,
        ``"Read"`` for those that encode out of it. The same codec class serves
        both, so the instance has to be told which it is -- and that is also
        what decides whether the keys it touches are declared to the scheduler
        as outputs or as inputs.

    There is deliberately nothing here that builds a scheduler dependency. The
    codec declares what it touches itself: with ordinary handle keys where it
    has a C++ type to name, and with a key built from a CLID it resolved at
    initialize() where it has none. Assembling a second declaration here would
    be a second thing that could disagree with the first.
    """
    return [codec.tool(f"{prefix}_{key}", key, direction, sg_type)
            for key, sg_type, codec in declarations]


def RpcRequestAlgCfg(name, fragment, target, **kwargs):
    """The client side of a fragment, from the fragment's own declaration.

    The client instantiates the *same* codec components the server does, the
    other way round: it encodes what the server declares as inputs and decodes
    what the server declares as outputs. Nothing is restated, so the two ends
    cannot disagree -- which is the whole argument for both ends being Athena.

    A client in another language shares the .proto files and nothing else; what
    ListSequences advertises is the wire contract, which is all such a client
    could act on. There is deliberately no way to turn an encoding string back
    into a component: naming a codec names a class, and both ends of a boundary
    import the same declaration.
    """
    alg = CompFactory.AthExRpc.RpcRequestAlg(
        name,
        SequenceName=fragment.name,
        Target=target,
        # Mirror image of the server: the client encodes what the server
        # declares as inputs and decodes what it declares as outputs.
        Inputs=boundaryTools(name, fragment.inputs, "Read"),
        Outputs=boundaryTools(name, fragment.outputs, "Write"),
        **kwargs)
    return alg


class RpcFragment:
    """The declaration of one entry on the server's menu.

    Usually built for you by :class:`RpcMenu`; construct one directly only to
    state a boundary that cannot be derived.

    :param name: the sequence name, which is also what a request asks for
    :param inputs: ``(key, sg_type, codec)`` the client must supply
    :param outputs: the same, returned to the client
    :param reply_key: where the pack algorithm stages the reply; derived from
        the name unless given. A fragment with no outputs needs none, and gets
        no pack algorithm at all.

    Note there is no list of supported types anywhere: a boundary is a key, a
    StoreGate type and a codec descriptor, and the descriptor names a component
    directly rather than a string to be looked up.
    """

    def __init__(self, name, inputs=(), outputs=(), reply_key=None):
        self.name = name
        self.inputs = [tuple(d) for d in inputs]
        self.outputs = [tuple(d) for d in outputs]
        if outputs and reply_key is None:
            reply_key = "RpcReply" + name.removeprefix("RpcSeq")
        self.reply_key = reply_key

    @property
    def input_specs(self):
        return boundarySpecs(self.inputs)

    @property
    def output_specs(self):
        return boundarySpecs(self.outputs)

    def gate(self, request_key):
        """The algorithm that selects this fragment and unpacks its inputs."""
        alg = CompFactory.AthExRpc.RpcGateAlg(
            f"{self.name}Gate",
            SequenceName=self.name,
            Request=request_key,
        )
        alg.Inputs = boundaryTools(alg.getName(), self.inputs, "Write")
        return alg

    def pack(self, request_key):
        """The algorithm that reads this fragment's outputs back, if any."""
        if not self.outputs:
            return None
        alg = CompFactory.AthExRpc.RpcPackAlg(
            f"{self.name}Pack",
            Request=request_key,
            Reply=self.reply_key,
        )
        # Only the outputs. What orders this algorithm after the whole payload
        # is that a fragment is a seqAND, which CFElements builds with
        # Sequential=True and to which the pack algorithm is appended last.
        alg.Outputs = boundaryTools(alg.getName(), self.outputs, "Read")
        return alg


def RpcPayloadCfg(*algorithms):
    """Convenience for the common case: a payload that is just some algorithms.

    Only for fragments simple enough not to need anything else. Anything real
    should pass its own ``SomethingCfg(flags)`` accumulator straight to
    :func:`RpcFragmentCfg`.
    """
    cfg = ComponentAccumulator()
    for algorithm in algorithms:
        cfg.addEventAlgo(algorithm)
    return cfg


def RpcFragmentCfg(flags, fragment, payload, parent, request_key):
    """Assemble one fragment: gate, the payload, then pack.

    ``payload`` is a ComponentAccumulator, not a list of algorithms, so that an
    existing subsystem configuration can be dropped in whole -- with its
    services, conditions algorithms, private tools and any internal sequence
    structure intact. Taking a list of configurables instead would force every
    caller to decompose a perfectly good CA into raw components and lose
    everything that is not an event algorithm.

    Its algorithms are merged into this fragment's sequence, between the gate
    and the pack algorithm; everything else in it merges normally. They stay
    ordinary Athena algorithms with ordinary typed handles, and the same
    configuration would work in a normal job -- which is the property the
    gate/pack pair exists to preserve.

    ``parent`` is the sequence the fragment hangs under, passed as a
    configurable rather than a name so that each fragment's accumulator is
    self-contained and they can be merged in any order.
    """
    cfg = ComponentAccumulator()
    cfg.addSequence(parent, parentName="AthAlgSeq")
    cfg.addSequence(seqAND(fragment.name), parentName=parent.name)
    cfg.addEventAlgo(fragment.gate(request_key), sequenceName=fragment.name)
    cfg.merge(payload, sequenceName=fragment.name)
    pack = fragment.pack(request_key)
    if pack is not None:
        cfg.addEventAlgo(pack, sequenceName=fragment.name)
    return cfg


#: Defaults for a job that serves one menu, which is every job so far.
RPC_TOP = "RpcTop"              #: the parOR every fragment hangs under
RPC_REQUEST_KEY = "RpcRequest"  #: where the loop manager stages the request


class RpcMenu:
    """Every fragment a server offers, and the configuration that follows.

    One call per fragment, and the menu the loop manager is told about and the
    sequences it drives come out of the same object -- so they cannot disagree,
    which is the failure this module exists to prevent.

    Typical use::

        menu = RpcMenu()
        menu.add("RpcSeqSum", RpcSumPayloadCfg(flags),
                 outputs=["total"],
                 boundaries={"addends": Protobuf("athexrpc.demo.v1.Ints"),
                             "total": Protobuf("athexrpc.demo.v1.Ints")})
        cfg.merge(menu.build(flags))
    """

    def __init__(self, request_key=RPC_REQUEST_KEY, top=RPC_TOP):
        self.request_key = request_key
        self.top = top
        self.fragments = []
        self._payloads = {}

    def add(self, name, payload, outputs=(), boundaries=None, types=None,
            inputs=None, ignore_inputs=(), produces=(), reply_key=None):
        """Offer one fragment.

        :param name: the sequence name, which is what a request asks for
        :param payload: the payload ComponentAccumulator, configured exactly as
            it would be for a normal job
        :param outputs: the keys to send back to the client. Not derivable: an
            intermediate that stays in the store is a legitimate choice.
        :param boundaries: ``{key: Codec}`` for every key that crosses, inputs
            and outputs alike -- e.g. ``{"addends": Protobuf(INTS)}``. Required,
            and a key without an entry is an error rather than a default; see
            the module docstring and RpcCodecs.
        :param types: overrides for derived StoreGate types,
            ``{key: type_name}``. Needed when a consumer reads a base class
            rather than the concrete type, which is the one case the derivation
            gets wrong rather than refusing.
        :param inputs: if given, an assertion rather than a declaration: these
            keys exactly, and a mismatch with what the payload actually leaves
            unmet is an error. Use it on a fragment whose contract matters.
        :param ignore_inputs: unmet reads the payload tolerates being absent,
            so they are not asked of the client. See deriveBoundaries.
        :param produces: keys the payload writes through a handle array, which
            Python cannot see. See deriveBoundaries.
        """
        types = dict(types or {})
        boundaries = dict(boundaries or {})
        derived, derived_types = deriveBoundaries(
            payload, outputs, self.request_key, boundaries=boundaries,
            extra_inputs=[k for k in (inputs or ()) if k in types],
            ignore_inputs=ignore_inputs, produces=produces)
        derived_types.update(types)

        if inputs is not None and sorted(inputs) != sorted(derived):
            raise ValueError(
                f"fragment '{name}' declares inputs {sorted(inputs)} but its "
                f"payload leaves {sorted(derived)} unmet")

        crossing = list(derived) + list(outputs)
        undeclared = [k for k in crossing if k not in boundaries]
        if undeclared:
            raise ValueError(
                f"fragment '{name}' does not say how {sorted(undeclared)} "
                "cross: give each a codec in boundaries=. There is no default; "
                "a wire format is not derivable from a C++ type")
        unused = [k for k in boundaries if k not in crossing]
        if unused:
            # A boundary declared for a key that does not cross is a rename
            # somebody half-finished. Left alone it is silent, and the key it
            # was meant to describe falls back to... nothing, above.
            raise ValueError(
                f"fragment '{name}' declares boundaries for {sorted(unused)}, "
                f"which do not cross; it crosses {sorted(crossing)}")

        def declare(keys):
            # Only a codec that takes its schema from the StoreGate type wants
            # one. For the rest there is nothing to look up and nothing that
            # needs it: the scheduler learns the key from the codec.
            return [(key, derived_types.get(key, ""), boundaries[key])
                    for key in keys]

        fragment = RpcFragment(name,
                               inputs=declare(derived),
                               outputs=declare(list(outputs)),
                               reply_key=reply_key)
        # Said out loud because a derived boundary is otherwise invisible: a
        # forgotten addCondAlgo turns an internal read into a mandatory client
        # input, and the first sign of it would be every request failing.
        _msg.info("RPC fragment %s: in %s, out %s", name,
                  fragment.input_specs or "-", fragment.output_specs or "-")
        self.fragments.append(fragment)
        self._payloads[name] = payload
        return fragment

    def fragment(self, name):
        """The declaration of one fragment, for a client that wants to call it.

        A client configured from this cannot disagree with the server about any
        part of the boundary, because there is only one declaration of it. See
        RpcRequestAlgCfg.
        """
        for fragment in self.fragments:
            if fragment.name == name:
                return fragment
        raise KeyError(f"this menu offers no fragment '{name}'; it offers "
                       f"{[f.name for f in self.fragments]}")

    def build(self, flags):
        """The parOR, every fragment's sequence, and the algorithms between."""
        cfg = ComponentAccumulator()
        cfg.addSequence(parOR(self.top), parentName="AthAlgSeq")
        for fragment in self.fragments:
            cfg.merge(RpcFragmentCfg(flags, fragment,
                                     self._payloads[fragment.name],
                                     parOR(self.top), self.request_key))
        return cfg

    @property
    def properties(self):
        """The loop manager's view, from the same declarations."""
        props = RpcMenuProperties(self.fragments)
        props["RequestKey"] = self.request_key
        return props


def RpcMenuProperties(fragments):
    """The loop manager's view of the menu, derived from the same declarations."""
    return {
        "Sequences": [f.name for f in fragments],
        "Inputs": {f.name: f.input_specs for f in fragments if f.inputs},
        "Outputs": {f.name: f.output_specs for f in fragments if f.outputs},
        "ReplyKeys": {f.name: f.reply_key for f in fragments if f.reply_key},
    }
