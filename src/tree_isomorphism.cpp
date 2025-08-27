#include "tree_isomorphism.h"

#include <fstream>
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;

string create_canonical_form(const Node& n, const Graph& graph) {
    if (graph.at(n).empty()) return "()";
    vector<string> canonical;
    for (const auto& i : graph.at(n))
        canonical.push_back(create_canonical_form(i, graph));
    sort(canonical.begin(), canonical.end());
    string result = "(";
    for (const auto& i : canonical)
        result += i;
    return result+")";

}

int calculate_levenshtein_distance(const string& s, const string& t) {
    const size_t m = s.size(), n = t.size();
    if (m < n) return calculate_levenshtein_distance(t, s);

    vector<int> prev(n + 1), curr(n + 1);
    for (auto i = 0; i <= n; i++)
        prev[i] = i;

    for (int i = 1; i <= m; i++) {
        curr[0] = i;
        for (int j = 1; j <= n; j++)
            if (s[i - 1] == t[j - 1])
                curr[j] = prev[j - 1];
            else
                curr[j] = 1 + min({prev[j], curr[j - 1], prev[j - 1]});
        swap(prev, curr);
    }
    return prev[n];
}

TreeNode* build_tree(const Node& root, const Graph& graph) {
    auto* node = new TreeNode{root, {}};
    for (const auto& i : graph.at(root))
        node->children.emplace_back(build_tree(i, graph));
    return node;
}

void delete_tree(TreeNode* node) {
    for (TreeNode* child : node->children)
        delete_tree(child);
    delete node;
}

void define_local_roots(const unordered_map<TreeNode*, TreeNode*>& most_left, TreeNode* node, TreeNode* father_node, vector<TreeNode*>& local_roots) {
    for (const auto& i : node->children)
        define_local_roots(most_left, i, node, local_roots);

    if (father_node->empty() || most_left.at(node) != most_left.at(father_node))
        local_roots.push_back(node);
}

void define_most_left_leaf(unordered_map<TreeNode*, TreeNode*>& most_left, TreeNode* node) {
    if (node->children.empty()) {
        most_left[node] = node;
        return;
    }
    define_most_left_leaf(most_left, node->children[0]);
    most_left[node] = most_left[node->children[0]];
    for (size_t i = 1; i < node->children.size(); i++)
        define_most_left_leaf(most_left, node->children[i]);
}

void rename_to_number_tree(vector<TreeNode*>& v, TreeNode* node) {
    for (TreeNode* child : node->children)
        rename_to_number_tree(v, child);
    v.push_back(node);
}

void tree_dist_helper(
    const unordered_map<TreeNode*, int>& idx1, const unordered_map<TreeNode*, int>& idx2,
    const vector<int>& lmd1_by_idx, const vector<int>& lmd2_by_idx,
    vector<vector<int>>& treedist,
    TreeNode* tn1, TreeNode* tn2) {

    constexpr int insert_cost = 1, delete_cost = 1;
    auto rename_cost = [] {
        return 0; // not implemented further logic, place to potentially expand, ie checking by similar name types.
    };

    int i_end = idx1.at(tn1);
    int j_end = idx2.at(tn2);
    int i_start = lmd1_by_idx[i_end];
    int j_start = lmd2_by_idx[j_end];
    int rows = i_end - i_start + 2;
    int cols = j_end - j_start + 2;

    vector fd(rows, vector<int>(cols));

    fd[0][0] = 0;
    for (int i = 1; i < rows; i++) fd[i][0] = fd[i-1][0] + delete_cost;
    for (int j = 1; j < cols; j++) fd[0][j] = fd[0][j-1] + insert_cost;

    for (int i = 1; i < rows; i++) {
        const int idx_i = i_start + i - 1;
        const int lmd_i = lmd1_by_idx[idx_i];
        for (int j = 1; j < cols; j++) {
            const int idx_j = j_start + j - 1;
            const int lmd_j = lmd2_by_idx[idx_j];

            if (lmd_i == i_start && lmd_j == j_start) {
                fd[i][j] = min({
                    fd[i - 1][j] + delete_cost,
                    fd[i][j - 1] + insert_cost,
                    fd[i - 1][j - 1] + rename_cost()
                });
                treedist[idx_i][idx_j] = fd[i][j];
            } else {
                const int lmd_i_local = lmd_i - i_start + 1;
                const int lmd_j_local = lmd_j - j_start + 1;
                fd[i][j] = min({
                    fd[i - 1][j] + delete_cost,
                    fd[i][j - 1] + insert_cost,
                    fd[lmd_i_local - 1][lmd_j_local - 1] + treedist[idx_i][idx_j]
                });
            }
        }
    }
}

// Zhang-Shasha algorithm
int calculate_tree_edit_distance(TreeNode* t1, TreeNode* t2) {
    unordered_map<TreeNode*, TreeNode*> most_left1, most_left2;
    define_most_left_leaf(most_left1, t1);
    define_most_left_leaf(most_left2, t2);

    vector<TreeNode*> roots1, roots2;
    auto emptyNode = TreeNode();
    define_local_roots(most_left1, t1, &emptyNode, roots1);
    define_local_roots(most_left2, t2, &emptyNode, roots2);

    vector<TreeNode*> post1, post2;
    rename_to_number_tree(post1, t1);
    rename_to_number_tree(post2, t2);


    unordered_map<TreeNode*, int> idx1, idx2;
    int n = post1.size(), m = post2.size();
    idx1.reserve(n);
    idx2.reserve(m);

    vector<int> lmd1_by_idx(n), lmd2_by_idx(m);
    for (int i = 0; i < n; i++) {
        TreeNode* node = post1[i];
        idx1[node] = i;
        TreeNode* lm = most_left1[node];
        if (lm == node)
            lmd1_by_idx[i] = i;
        else
            lmd1_by_idx[i] = idx1.at(lm);
    }
    for (int j = 0; j < m; j++) {
        TreeNode* node = post2[j];
        idx2[node] = j;
        TreeNode* lm = most_left2[node];
        if (lm == node)
            lmd2_by_idx[j] = j;
        else
            lmd2_by_idx[j] = idx2.at(lm);
    }

    vector treedist(n, vector(m, 0));

    for (TreeNode * i : roots1)
        for (TreeNode * j : roots2)
            tree_dist_helper(idx1, idx2, lmd1_by_idx, lmd2_by_idx, treedist, i, j);

    return treedist[n-1][m-1];
}


Graph read_graph_from_json(const string& filename) {
    ifstream in(filename);
    json file;
    in >> file;

    Graph g;
    for (auto& [key, value] : file.items())
        g[key] = value.get<vector<string>>();

    return g;

}

unordered_map<string, double> similarity_between_graphs(const Node& a, const Node& b, const Graph& graph1, const Graph& graph2,
        bool do_strict = true, bool do_levenshtein = true, bool do_ted = true) {
    unordered_map<string, double> similarity;
    string first_canonical, second_canonical;
    if (do_strict || do_levenshtein) {
        first_canonical = create_canonical_form(a, graph1);
        second_canonical = create_canonical_form(b, graph2);
    }
    if (do_strict)
        similarity["strict similarity"] = first_canonical == second_canonical;
    if (do_levenshtein)
        similarity["Levenshtein distance"] = 1.0 - calculate_levenshtein_distance(first_canonical, second_canonical) /
                                             static_cast<double>(max(first_canonical.size(), second_canonical.size()));
    if (do_ted) {
        TreeNode* tree1 = build_tree(a, graph1), *tree2 = build_tree(b, graph2);
        vector<TreeNode*> post1, post2;
        rename_to_number_tree(post1, tree1);
        rename_to_number_tree(post2, tree2);

        const int ted = calculate_tree_edit_distance(tree1, tree2);
        const unsigned int max_size = post1.size() + post2.size();
        similarity["TED"] = 1.0 - static_cast<double>(ted) / max_size;
        delete_tree(tree1);
        delete_tree(tree2);
    }
    return similarity;
}