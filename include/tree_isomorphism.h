#pragma once

#include <string>
#include <vector>
#include <unordered_map>

using Node = std::string;
using Graph = std::unordered_map<Node, std::vector<Node>>;

struct TreeNode {
    std::string label;
    std::vector<TreeNode*> children;
};

std::string create_canonical_form(const Node& root, const Graph& graph);

Graph read_graph_from_json(const std::string& filename);

std::unordered_map<std::string, double> similarity_between_graphs(const Node& a, const Node& b, const Graph& graph1, const Graph& graph2);