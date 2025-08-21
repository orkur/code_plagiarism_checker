import json
import re
import sys
from typing import (Dict, List)



class Node:
    id: str | None
    name: str | None
    kind: str | None
    value: str | None
    operator: str | None
    qual_type: str | None
    children: list['Node']
    is_used: bool | None

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
    def create_node(json_data, cut_unused_variables=False) -> 'Node | None':
        node = Node()
        if json_data.get('id') is not None:
            node.id = json_data['id']
            node.name = json_data.get('name')
            node.kind = json_data.get('kind')
            node.value = json_data.get('value', None)
            node.operator = json_data.get('opcode', None)
            node.qual_type = json_data.get('type', {}).get('qualType', None)
            node.is_used = json_data.get('isUsed', False)
            if cut_unused_variables and (node.kind == 'VarDecl' or node.kind == "FunctionDecl") and node.is_used == False:
                return None
            children = []
            not_parsed_children = json_data.get('inner', [])
            for not_parsed_child in not_parsed_children:
                child = Node.create_node(not_parsed_child, cut_unused_variables)
                if child is not None:
                    children.append(child)
            node.children = children
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

    graph[node.id] = sorted(child.id for child in node.children)
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

def extract_main(input_data, cut_unused_variables=False):
    children = []
    for node in input_data["inner"]:
        if is_user_made(node):
            children.append(Node.create_node(node, cut_unused_variables))
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

def extract_on_same_level_markers(input_data, cut_unused_variables=False):
    children = []
    save_node = False
    marker_labels = []

    for node in input_data["inner"]:
        potential_marker_node = node.get("inner", [{}])[0]
        name = potential_marker_node.get("name", '')
        if is_end_marker(name) and len(potential_marker_node.get("inner", [])) == 2:
            marker_labels.remove(extract_marker_label(name))
            save_node = False if len(marker_labels) == 0 else True

        if save_node:
            children.append(Node.create_node(node, cut_unused_variables))

        if is_start_marker(name) and len(potential_marker_node.get("inner", [])) == 2:
            save_node = True
            marker = extract_marker_label(name)
            marker_labels.append(marker)
    return children

def extract_everywhere(input_data, cut_unused_variables=False):
    all_marked = []

    def recurse(node):
        if "inner" in node and (node.get("kind") == "TranslationUnitDecl" or is_user_made(node)):
            result = extract_on_same_level_markers(node, cut_unused_variables)
            if result:
                all_marked.extend(result)
            else:
                for child in node["inner"]:
                    recurse(child)

    recurse(input_data)

    root = Node()
    root.children = all_marked
    return root

if __name__ == '__main__':

    if len(sys.argv) < 3:
        print("Usage: python3 parse_ast.py <input_ast.json> <output_graph.json> [markers y/N] [cut-unused y/N]")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2]
    using_markers = len(sys.argv) >= 4 and sys.argv[3].lower() == "y"
    cut_unused = len(sys.argv) >= 5 and sys.argv[4].lower() == "y"

    with open(input_path) as f:
        data = json.load(f)
        print(f"[INFO] Parsing main from {input_path}")
    if not using_markers:
        main_node = extract_main(data, cut_unused_variables=cut_unused)
    else:
        main_node = extract_everywhere(data, cut_unused_variables=cut_unused)

        main_node.id = "main"
        graph = to_graph_dict(main_node)
        with open(output_path, "w") as out:
            json.dump(graph, out, indent=2)
        print(f"[INFO] Wrote graph to {output_path}")
