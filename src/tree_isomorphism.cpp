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

void postorder(TreeNode* node,
               vector<TreeNode*>& post_list,
               unordered_map<TreeNode*, int>& index_map) {
    for (TreeNode* child : node->children)
        postorder(child, post_list, index_map);
    index_map[node] = static_cast<int>(post_list.size());
    post_list.push_back(node);
}


// TODO add rename_cost based on type of node (eg VarDecl, Stmt, ...)
// TODO doesn't work properly
int calculate_tree_edit_distance(TreeNode* t1, TreeNode* t2) {
    vector<TreeNode*> post1, post2;
    unordered_map<TreeNode*, int> idx1, idx2;
    postorder(t1, post1, idx1);
    postorder(t2, post2, idx2);

    const size_t n = post1.size();
    const size_t m = post2.size();

    vector dp(n + 1, vector<int>(m + 1));
    constexpr int rename_cost = 0, insert_cost = 1, delete_cost = 1;
    dp[0][0] = 0;
    for (int i = 1; i <= n; i++) dp[i][0] = dp[i-1][0] + delete_cost;
    for (int j = 1; j <= m; j++) dp[0][j] = dp[0][j-1] + insert_cost;

    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {

            dp[i][j] = min({
                dp[i - 1][j] + delete_cost,
                dp[i][j - 1] + insert_cost,
                dp[i - 1][j - 1] + rename_cost
            });
        }
    }
    return dp[n][m];
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
    unordered_map<TreeNode*, int> idx1, idx2;
    postorder(tree1, post1, idx1);
    postorder(tree2, post2, idx2);

    const int ted = calculate_tree_edit_distance(tree1, tree2);
    const unsigned int max_size = max(post1.size(), post2.size());
    similarity["TED"] = 1.0 - static_cast<double>(ted) / max_size;
    delete_tree(tree1);
    delete_tree(tree2);
    return similarity;
}