#!/usr/bin/env python

from os import makedirs
from os.path import isdir
from argparse import ArgumentParser
from typing import Mapping, Union, TextIO

from DQDefects import DefectsDB
from DQDefects.virtual_logic import DefectLogic

DEPENDSON_DIR = "dependson"

class Node(object):
    def __init__(self, name: str):
        self.name = name
        self.children = set()
        self.parents = set()

    def get_name(self) -> str:
        return self.name

    @property
    def virtual(self) -> bool:
        return bool(self.children)

    @property
    def primary(self) -> bool:
        return not self.virtual

    @property
    def has_primary_children(self) -> bool:
        return any(c.primary for c in self.children)

    @property
    def only_primary_children(self) -> bool:
        return all(c.primary for c in self.children)

    def __repr__(self):
        return (f'<Node {self.name} parents="{[n.name for n in self.parents]}" '
                f'children="{[n.name for n in self.children]}">')

    def dot(self, current_node=False, tooltip: str ="", viewing_dependents: bool =False) -> str:
        import html
        color = "grey" if self.primary else "white"
        if current_node:
            color = "darkslategray2" if viewing_dependents else "gold"

        tooltip = ("[Virtual]" if self.virtual else "[Primary]") + " " + html.escape(tooltip, True)
        label = self.name


        if current_node:
            # Clicking on the current node toggles between views
            if viewing_dependents:
                url = f"../{self.name}.svg"
                label += r"\n[Deep dependants]"
            else:
                url = f"{DEPENDSON_DIR}/{self.name}.svg"
                label += r"\n[Dependencies]"
        else:
            url = "%s.svg" % self.name

        return f'{self.name} [fillcolor={color}, style=filled, URL="{url}", tooltip="{tooltip}", label="{label}"];'

def build_tree(all_logic: Mapping[str, DefectLogic]) -> dict[str, Node]:

    all_nodes: dict[str, Node] = {}

    def make_primary(current_logic: str) -> Node:
        if current_logic in all_nodes:
            return all_nodes[current_logic]
        all_nodes[current_logic] = node = Node(current_logic)
        return node

    def explore_virtual(current_logic: str) -> Node:
        if current_logic in all_nodes:
            return all_nodes[current_logic]

        this_logic = all_logic[current_logic]
        all_nodes[current_logic] = node = Node(current_logic)

        for name in this_logic.clauses:
            if name in all_logic:
                child = explore_virtual(name)
            else:
                child = make_primary(name)

            child.parents.add(node)
            node.children.add(child)

        return node

    for name in all_logic:
        explore_virtual(name)

    return all_nodes

def walk(node: Node, visited: set[Node], visit_primary: bool, fd: TextIO):
    visited.add(node)

    for subnode in sorted(node.children, key=Node.get_name):
        if subnode.virtual or (subnode.primary and visit_primary):
            print(f"{node.name} -> {subnode.name}", file=fd)
            walk(subnode, visited, visit_primary, fd)

def dump_visited(fd: TextIO, current_node: Node, nodes: set[Node],
                 descs: Mapping[Union[str, int], str], parents: bool):
    for node in sorted(nodes, key=Node.get_name):
        at_current_node = node.name == current_node.name
        description = descs.get(node.name, node.name)
        # Line breaks in defect descriptions mess up the svg generation
        description = description.replace('\n', ' ')
        print(node.dot(at_current_node, description, parents), file=fd)

def build_dependency_graph(output_dir: str, node: Node, descs: Mapping[Union[str, int], str]):

    with open("%s/%s.dot" % (output_dir, node.name), "w") as fd:
        print(f"strict digraph {node.name} {{", file=fd)
        print("rankdir=LR;\n", file=fd)

        visited = set()

        for parent in sorted(node.parents, key=Node.get_name):
            print(f"{parent.name} -> {node.name}", file=fd)
            visited.add(parent)

        walk(node, visited, node.has_primary_children, fd)

        dump_visited(fd, node, visited, descs, parents=False)

        print("}", file=fd)

def build_parent_tree(output_dir: str, node: Node, descs: Mapping[Union[str, int], str]):

    visited = set([node])

    with open(f"{output_dir}/{DEPENDSON_DIR}/{node.name}.dot", "w") as fd:
        def walk_parents(node):
            visited.add(node)
            for parent in node.parents:
                print(f"{parent.name} -> {node.name}", file=fd)
                walk_parents(parent)

        print(f"strict digraph {node.name} {{", file=fd)
        print("rankdir=LR;\n", file=fd)

        walk_parents(node)

        dump_visited(fd, node, visited, descs, parents=True)

        print("}", file=fd)

def render_all_flags(output_dir: str, all_nodes: dict[str, Node], descs: Mapping[Union[str, int], str]):

    path = "%s/%s" % (output_dir, DEPENDSON_DIR)
    if not isdir(path):
        makedirs(path)

    for _, node in sorted(all_nodes.items()):
        build_dependency_graph(output_dir, node, descs)
        build_parent_tree(output_dir, node, descs)

def main():

    parser = ArgumentParser(description="Dump defect viewer information")

    parser.add_argument("-t", "--tag", default="HEAD", help="Defect tag")
    parser.add_argument("-o", "--output", default=".", help="Directory to dump files")

    args = parser.parse_args()

    # Instantiate the Defect DB
    db = DefectsDB(tag=args.tag)

    descs = db.all_defect_descriptions

    # Build node objects
    all_nodes = build_tree(db.virtual_defect_logics)

    # Write outputs
    output_dir = "%s/%s" % (args.output, args.tag)
    render_all_flags(output_dir, all_nodes, descs)

if __name__ == "__main__":
    main()
