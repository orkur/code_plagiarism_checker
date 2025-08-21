#include "tree_isomorphism.h"

#include <fstream>
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;

//AHU algorithm
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

int calculate_levenshtein_distance(const string& g1, const string& g2) {
    vector lev(g1.size() + 1, vector<int>(g2.size() + 1));
    for (int i = 0; i <= g1.size(); i++)
        lev[i][0] = i;
    for (int i = 0; i <= g2.size(); i++)
        lev[0][i] = i;

    for (int i = 1; i <= g1.size(); i++)
        for (int j = 1; j <= g2.size(); j++)
            if (g1[i-1] == g2[j-1])
                lev[i][j] = lev[i - 1][j - 1];
            else
                lev[i][j] = 1 + min({lev[i - 1][j], lev[i][j - 1], lev[i - 1][j - 1]});
    const int ans = lev[g1.size()][g2.size()];
    return ans;
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

void define_local_roots(const unordered_map<TreeNode*, TreeNode*>& most_left, TreeNode* node, TreeNode* father_label, vector<TreeNode*>& local_roots) {
    for (const auto& i : node->children)
        define_local_roots(most_left, i, node, local_roots);

    if (father_label->empty() || most_left.at(node) != most_left.at(father_label))
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
    const unordered_map<TreeNode*, int>& lmd1, const unordered_map<TreeNode*, int>& lmd2,
    const vector<TreeNode*>& post1, const vector<TreeNode*>& post2,
    vector<vector<int>>& treedist,
    TreeNode* tn1, TreeNode* tn2) {


    constexpr int insert_cost = 1, delete_cost = 1;
    auto rename_cost = [](TreeNode* a, TreeNode* b) {
        return 0; // not implemented further logic
    };
    int n1 = idx1.at(tn1);
    int m1 = idx2.at(tn2);
    int i0 = lmd1.at(post1[n1]);
    int j0 = lmd2.at(post2[m1]);
    int rows = n1 - i0 + 2;
    int cols = m1 - j0 + 2;

    vector fd(rows, vector<int>(cols));

    fd[0][0] = 0;
    for (int i = 1; i < rows; i++) fd[i][0] = fd[i-1][0] + delete_cost;
    for (int j = 1; j < cols; j++) fd[0][j] = fd[0][j-1] + insert_cost;

    for (int i = 1; i < rows; i++) {
        for (int j = 1; j < cols; j++) {
            TreeNode* node1 = post1[i0 + i - 1];
            TreeNode* node2 = post2[j0 + j - 1];
            int l1 = lmd1.at(node1);
            int l2 = lmd2.at(node2);
            int treex = idx1.at(node1);
            int treey = idx2.at(node2);

            if (l1 == i0 && l2 == j0) {
                fd[i][j] = min({
                    fd[i - 1][j] + delete_cost,
                    fd[i][j - 1] + insert_cost,
                    fd[i - 1][j - 1] + rename_cost(node1, node2)
                });
                treedist[treex][treey] = fd[i][j];
            } else {
                int x = l1 - i0 + 1;
                int y = l2 - j0 + 1;
                fd[i][j] = min({
                    fd[i - 1][j] + delete_cost,
                    fd[i][j - 1] + insert_cost,
                    fd[x - 1][y - 1] + treedist[treex][treey]
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

    vector<TreeNode*> post1, post2, roots1, roots2;
    rename_to_number_tree(post1, t1);
    rename_to_number_tree(post2, t2);

    auto emptyNode = TreeNode();
    define_local_roots(most_left1, t1, &emptyNode, roots1);
    define_local_roots(most_left2, t2, &emptyNode, roots2);

    unordered_map<TreeNode*, int> idx1, idx2;
    unordered_map<TreeNode*, int> lmd1, lmd2;

    for (int i = 0; i < post1.size(); ++i) {
        TreeNode* node = post1[i];
        idx1[node] = i;
    }
    for (TreeNode* node : post1)
        lmd1[node] = idx1[most_left1[node]];

    for (int i = 0; i < post2.size(); ++i) {
        TreeNode* node = post2[i];
        idx2[node] = i;
    }
    for (TreeNode* node : post2)
        lmd2[node] = idx2[most_left2[node]];

    int n = post1.size(), m = post2.size();
    vector treedist(n, vector(m, 0));

    for (TreeNode * i : roots1)
        for (TreeNode * j : roots2)
            tree_dist_helper(idx1, idx2, lmd1, lmd2, post1, post2, treedist, i, j);

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

unordered_map<string, double> similarity_between_graphs(const Node& a, const Node& b, const Graph& graph1, const Graph& graph2) {
    unordered_map<string, double> similarity;
    const string first_canonical = create_canonical_form(a, graph1);
    const string second_canonical = create_canonical_form(b, graph2);
    similarity["strict similarity"] = first_canonical == second_canonical;
    similarity["Levenshtein distance"] = 1.0 - calculate_levenshtein_distance(first_canonical, second_canonical)/static_cast<double>(max(first_canonical.size(), second_canonical.size()));
    TreeNode* tree1 = build_tree(a, graph1), *tree2 = build_tree(b, graph2);
    vector<TreeNode*> post1, post2;
    rename_to_number_tree(post1, tree1);
    rename_to_number_tree(post2, tree2);

    const int ted = calculate_tree_edit_distance(tree1, tree2);
    const unsigned int max_size = post1.size() + post2.size();
    similarity["TED"] = 1.0 - static_cast<double>(ted) / max_size;
    delete_tree(tree1);
    delete_tree(tree2);
    return similarity;
}