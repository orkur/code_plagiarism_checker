#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <tree_isomorphism.h>
#include <sstream>
using json = nlohmann::json;
using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <tree1.json> <tree2.json> [--metrics M1,M2,...]\n";
        return 1;
    }

    Graph g1 = read_graph_from_json(argv[1]);
    Graph g2 = read_graph_from_json(argv[2]);

    const Node root1 = "main";
    const Node root2 = "main";
    bool strict = true, ted = true, lev = true;
    for (int i = 3; i < argc; ++i) {
        string arg = argv[i];
        if (arg.rfind("--metrics", 0) == 0) {
            string metrics;
            auto pos = arg.find('=');
            if (pos != string::npos) {
                metrics = arg.substr(pos + 1);
            } else if (i + 1 < argc) {
                metrics = argv[++i];
            }

            strict = ted = lev = false;
            stringstream ss(metrics);
            string token;
            while (getline(ss, token, ',')) {
                if (token == "STRICT") strict = true;
                else if (token == "LEV") lev = true;
                else if (token == "TED") ted = true;
            }
        }
    }

    for (auto &[key, value]: similarity_between_graphs(root1, root2, g1, g2, strict, lev, ted))
        cout << key << ": " << value << endl;
    return 0;
}
