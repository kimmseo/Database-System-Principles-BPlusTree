//
// Created by Minseo on 2/7/2025.
//

#include <iostream>
#include <sstream>
#include "BPlusTree.h"
#include "Definitions.h"

std::string introMessage(int aOrder) {
    std::ostringstream oss;
    oss << "B+ Tree of Order " << aOrder << std::endl;
    oss << "C++ implementation" << std::endl;
    oss << "To build a B+ tree of a different order, start again and enter the order" << std::endl;
    oss << "as an integer argument:  bpt <order>  " << std::endl;
    oss << "(" << MIN_ORDER << " <= order <= " << MAX_ORDER << ")." << std::endl;
    oss << "To start with input from a file of newline-delimited integers," << std::endl;
    oss << "start again and enter the order followed by the filename:" << std::endl;
    oss << "bpt <order> <inputfile> ." << std::endl << std::endl;
    return oss.str();
}

std::string usageMessage() {
    std::string message =
        "Enter any of the following commands after the prompt > :\n"
        "\ti <k>  -- Insert <k> (an integer, <k> >= 0) as both key and value).\n"
        "\ti <k> <v> -- Insert (integer) value <v> under (integer) key <k> (<k> >= 0).\n"
        "\tf <k>  -- Find the value under key <k>.\n"
        "\tp <k> -- Print the path from the root to key k and its associated value.\n"
        "\tr <k1> <k2> -- Print the keys and values found in the range [<k1>, <k2>]\n"
        "\td <k>  -- Delete key <k> and its associated value.\n"
        "\tx -- Destroy the whole tree.  Start again with an empty tree of the same order.\n"
        "\tt -- Print the B+ tree.\n"
        "\tl -- Print the keys of the leaves (bottom row of the tree).\n"
        "\tm -- Print tree info (number of levels, number of nodes, root content).\n"
        "\tv -- Toggle output of pointer addresses (\"verbose\") in tree and leaves.\n"
        "\tq -- Quit. (Or use Ctl-D.)\n"
        "\t? -- Print this help message.\n\n";
    return message;
}

int getOrder(int argc, const char* argv[]) {
    if (argc > 1) {
        int order = 0;
        std::istringstream iss(argv[1]);
        if ((iss >> order) && iss.eof() && order >= MIN_ORDER && order <= MAX_ORDER) {
            return order;
        } else {
            std::cerr << "Invalid order specification: " << argv[1] << std::endl;
            std::cerr << "Order must be an integer such that " << MIN_ORDER
                      << " <= <order> <= " << MAX_ORDER << std::endl;
            std::cerr << "Proceeding with order " << DEFAULT_ORDER << std::endl;
        }
    }
    return DEFAULT_ORDER;
}

int main(int argc, const char* argv[]) {
    char instruction;
    double key = 0;
    bool quit = false;
    bool verbose = false;
    int order = getOrder(argc, argv);
    std::cout << introMessage(order);
    BPlusTree tree(order);
    // Load data
    // Edit if needed
    std::string filename = "../Src/games.txt";
    int keyColumn = 3;  // Column 3 (FG_PCT_home) as key
    tree.bulkLoadFromCSV(filename, keyColumn);
    std::cout << "\n--- Bulk Loading ---\n";
    double bulkTime = tree.bulkLoadFromCSV(filename, keyColumn);
    std::cout << "Bulk Loading Time: " << bulkTime << " seconds.\n";

    std::cout << "\n--- Normal Insertion ---\n";
    BPlusTree normalTree(order);
    double normalTime = normalTree.normalInsertFromCSV(filename, keyColumn);
    std::cout << "Normal Insertion Time: " << normalTime << " seconds.\n";

    std::cout << "\nChecking size of each record:\n";
    std::cout << "sizeof(gameRecord): " << sizeof(gameRecord) << std::endl;
    std::cout << "sizeof(*gameRecord): " << sizeof(gameRecord*) << std::endl;

    // **Comparison**
    std::cout << "\n--- Comparison ---\n";
    std::cout << " - Bulk Loading Time: " << bulkTime << " sec\n";
    std::cout << " - Normal Insertion Time: " << normalTime << " sec\n";
    std::cout << (normalTime < bulkTime ? "Normal insertion was faster."
                                        : "Bulk loading was faster.")
              << "\n";
    if (argc > 2) {
        tree.readInputFromFile(argv[2]);
        std::cout << "Input from file " << argv[2] << ":" << std::endl;
        tree.print();
    }

    std::cout << std::endl;
    std::cout << usageMessage();
    while (!quit) {
        std::cout << "> ";
        std::cin >> instruction;
        switch (instruction) {
            case 'd':
                std::cin >> key;
                std::cout << "\n--- Bulk ---\n";
                tree.remove(key);
                tree.print(verbose);
                std::cout << "\n--- Normal ---\n";
                normalTree.remove(key);
                normalTree.print(verbose);
                break;
            case 'f':
                std::cin >> key;
                std::cout << "\n--- Bulk ---\n";
                tree.printValue(key);
                std::cout << "\n--- Normal ---\n";
                normalTree.printValue(key);
                break;
            case 'l':
                std::cout << "\n--- Bulk ---\n";
                tree.printLeaves(verbose);
                std::cout << "\n--- Normal ---\n";
                normalTree.printLeaves(verbose);
                break;
            case 'p':
                std::cin >> key;
                std::cout << "\n--- Bulk ---\n";
                tree.printPathTo(key, verbose);
                std::cout << "\n--- Normal ---\n";
                normalTree.printPathTo(key, verbose);
                break;
            case 'q':
                quit = true;
                break;
            case 'r': {
                double key2;
                std::cin >> key;
                std::cin >> key2;
                std::cout << "\n--- Bulk ---\n";
                // tree.printRange(key, key2);
                tree.printRangeWithStats(key, key2);
                std::cout << "\n--- Normal ---\n";
                // normalTree.printRange(key, key2);
                normalTree.printRangeWithStats(key, key2);
                break;
            }
            case 't':
                std::cout << "\n--- Bulk ---\n";
                tree.print(verbose);
                std::cout << "\n--- Normal ---\n";
                normalTree.print(verbose);
                break;
            case 'v':
                verbose = !verbose;
                tree.print(verbose);
                break;
            case 'x':
                tree.destroyTree();
                tree.print();
                break;
            case 'm':
                std::cout << "\n--- Parameter n of B+ tree ---\n";
                std::cout << "Order: " << order << std::endl;
                std::cout << "\n--- Bulk ---\n";
                tree.printTreeInfo();
                std::cout << "\n--- Normal ---\n";
                normalTree.printTreeInfo();
                break;
            case '?':
                std::cout << usageMessage();
                break;
            default:
                std::cin.ignore(256, '\n');
                std::cout << usageMessage();
                break;
        }
    }
    return 0;
}
