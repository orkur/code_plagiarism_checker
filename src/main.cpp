#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <tree_isomorphism.h>
using json = nlohmann::json;
using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: file1.json file2.json\n";
        return 1;
    }

    Graph g1 = read_graph_from_json(argv[1]);
    Graph g2 = read_graph_from_json(argv[2]);

    const Node root1 = "main";
    const Node root2 = "main";

    for (auto &[key, value]: similarity_between_graphs(root1, root2, g1, g2))
        cout << key << ": " << value << endl;
    return 0;
}
