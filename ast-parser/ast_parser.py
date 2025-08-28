import json
import re
from typing import (Dict, List)
import argparse


class Node:
    id: str | None
    name: str | None
    kind: str | None
    value: str | None
    operator: str | None
    qual_type: str | None
    children: list['Node']
    is_used: bool | None
    in_marker: bool | None

    def __init__(self):
        self.id = None
        self.name = None
        self.kind = None
        self.value = None
        self.operator = None
        self.qual_type = None
        self.children = []
        self.is_used = None

    @staticmethod
    def create_node(json_data, optimise=False) -> 'Node | None':
        node = Node()
        if json_data.get('id') is not None:
            node.id = json_data['id']
            node.name = json_data.get('name')
            node.kind = json_data.get('kind')
            node.value = json_data.get('value', None)
            node.operator = json_data.get('opcode', None)
            node.qual_type = json_data.get('type', {}).get('qualType', None)
            node.is_used = json_data.get('isUsed', False)
            if optimise and (node.kind == 'VarDecl' or (node.kind == "FunctionDecl" and node.name != "main")) and node.is_used == False:
                return None
            children = []
            not_parsed_children = json_data.get('inner', [])
            for not_parsed_child in not_parsed_children:
                child = Node.create_node(not_parsed_child, optimise)
                if child is not None:
                    children.append(child)
            node.children = children
            if optimise and (node.kind == 'DeclStmt' or node.kind == "CompoundStmt") and len(node.children) == 0:
                return None
            if optimise and (node.kind == "CompoundStmt" or node.kind == "ImplicitCastExpr") and len(node.children) == 1:
                return children[0]
            return node
        else:
            return None
    def to_dict(self):
        return {
            "id": self.id,
            "name": self.name,
            "kind": self.kind,
            "value": self.value,
            "operator": self.operator,
            "qual_type": self.qual_type,
            "is_used": self.is_used,
            "children": [child.to_dict() for child in self.children]
        }

def to_graph_dict(node: 'Node', graph: Dict[str, List[str]] | None = None) -> Dict[str, List[str]]:
    if graph is None:
        graph = {}

    graph[node.id] = [child.id for child in node.children]
    for child in node.children:
        to_graph_dict(child, graph)

    return graph

def is_user_made(node):
    if node.get("kind") == "UsingDirectiveDecl":
        return False

    if node.get("loc", {}).get("includedFrom") is not None:
        return False

    range_ = node.get("range", {})
    begin = range_.get("begin", {})
    end = range_.get("end", {})

    return begin != {} and end != {} and 'spellingLoc' not in begin and 'spellingLoc' not in end

def extract_main(input_data, optimise=False):
    children = []
    for node in input_data["inner"]:
        if is_user_made(node):
            child = Node.create_node(node, optimise)
            if child is not None:
                children.append(child)
    node = Node()
    node.children = children
    return node

def is_start_marker(name):
    return re.match(r"^__marker_[a-zA-Z0-9_]+_start__$", name) is not None

def is_end_marker(name):
    return re.match(r"^__marker_[a-zA-Z0-9_]+_end__$", name) is not None

def extract_marker_label(name):
    match = re.match(r"^__marker_([a-zA-Z0-9_]+)_(start|end)__$", name)
    return match.group(1) if match else None

def extract_on_same_level_markers(input_data, optimise=False):
    children = []
    save_node = False
    marker_labels = []

    for node in input_data["inner"]:
        potential_marker_node = node.get("inner", [{}])[0]
        name = potential_marker_node.get("name")
        if name is None:
            name = node.get("name", "")
        if is_end_marker(name):
            marker_labels.remove(extract_marker_label(name))
            save_node = False if len(marker_labels) == 0 else True
            node["saved"] = True

        if save_node:
            child = Node.create_node(node, optimise)
            if child is not None:
                children.append(child)
            node["saved"] = True

        if is_start_marker(name):
            save_node = True
            marker = extract_marker_label(name)
            marker_labels.append(marker)
            node["saved"] = True
    return children

def extract_everywhere(input_data, optimise=False):
    all_marked = []

    def recurse(node):
        if "inner" in node and (node.get("kind") == "TranslationUnitDecl" or is_user_made(node)):
            result = extract_on_same_level_markers(node, optimise)
            if result:
                all_marked.extend(result)
            for child in node["inner"]:
                if not child.get("saved", False):
                    recurse(child)

    recurse(input_data)

    root = Node()
    root.children = all_marked
    return root

if __name__ == '__main__':

    parser = argparse.ArgumentParser(
        prog="ast_parser.py",
        description="Convert Clang AST (JSON) into a lightweight adjacency map for similarity checks.",
    )
    parser.add_argument("input",  help="path to input AST dump (JSON) produced by Clang")
    parser.add_argument("output", help="path to output graph (JSON) with adjacency map")
    parser.add_argument(
        "-m", "--markers",
        action="store_true",
        help="extract only code fragments delimited by markers "
             "(__marker_<label>_start__/__marker_<label>_end__)"
    )
    parser.add_argument(
        "--optimise",
        action="store_true",
        help="optimise graph by deleting unused variables, functions, unnecessary brackets, casting."
    )
    parser.add_argument(
        "--emit",
        choices=["lite", "full"],
        default="lite",
        help="output format: 'lite' (adjacency map only) or 'full' (full metadata)"
    )

    args = parser.parse_args()

    input_path = args.input
    output_path = args.output
    using_markers = args.markers
    optimise = args.optimise
    emit = args.emit

    with open(input_path) as f:
        data = json.load(f)
        print(f"[INFO] Parsing main from {input_path}")
    if not using_markers:
        main_node = extract_main(data, optimise=optimise)
    else:
        main_node = extract_everywhere(data, optimise=optimise)

    main_node.id = "main"
    if emit == "lite":
        graph = to_graph_dict(main_node)
        with open(output_path, "w") as out:
            print(f"[INFO] Wrote graph to {output_path}")
            json.dump(graph, out, indent=2)
    if emit == "full":
        full_output_path = output_path.replace(".json", ".labels.json")
        with open(full_output_path, "w") as out:
            print(f"[INFO] Wrote full graph to {full_output_path}")
            json.dump(main_node.to_dict(), out, indent=2)

