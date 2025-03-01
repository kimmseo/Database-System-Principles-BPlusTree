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
        "\tr <k1> <k2> -- Print the keys and values found in the range [<k1>, <k2>].\n"
        "\td <k>  -- Delete key <k> and its associated value.\n"
        "\tx -- Destroy the whole tree. Start again with an empty tree of the same order.\n"
        "\tt -- Print the entire B+ tree.\n"
        "\tl -- Print the keys of the leaves (bottom row of the tree).\n"
        "\tm -- Print tree info (number of levels, number of nodes, root content).\n"
        "\tv -- Toggle output of pointer addresses (\"verbose\") in tree and leaves.\n"
        "\tS <filename> -- Save the current B+ tree structure to <filename>.\n"
        "\tL <filename> -- Load a B+ tree structure from <filename>.\n"
        "\tq -- Quit. (Or use Ctrl-D.)\n"
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
    int key = 0;
    bool quit = false;
    bool verbose = false;
    int order = getOrder(argc, argv);
    std::cout << introMessage(order);
    std::cout << usageMessage();
    BPlusTree tree(order);
    if (argc > 2) {
        tree.readInputFromFile(argv[2]);
        std::cout << "Input from file " << argv[2] << ":" << std::endl;
        tree.print();
    }
    while (!quit) {
        std::cout << "> ";
        std::cin >> instruction;
        switch (instruction) {
            case 'd':
                std::cin >> key;
                tree.remove(key);
                tree.print(verbose);
                break;
            case 'i': {
                char buffer[BUFFER_SIZE];
                int value = 0;
                std::cin.getline(buffer, BUFFER_SIZE);
                int count = sscanf(buffer, "%d %d", &key, &value);
                if (count == 1) {
                    value = key;
                }
                // std::cin >> key;
                // if (key < 0) {
                // std::cout << usageMessage();
                // }
                tree.insert(key, value);
                tree.print(verbose);
                break;
            }
            case 'f':
                std::cin >> key;
                tree.printValue(key);
                break;
            case 'l':
                tree.printLeaves(verbose);
                break;
            case 'p':
                std::cin >> key;
                tree.printPathTo(key, verbose);
                break;
            case 'q':
                quit = true;
                break;
            case 'r': {
                int key2;
                std::cin >> key;
                std::cin >> key2;
                tree.printRange(key, key2);
                break;
            }
            case 't':
                tree.print(verbose);
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
                tree.printTreeInfo();
                break;
            case '?':
                std::cout << usageMessage();
                break;
            case 'S': {
                // Save
                std::string filename;
                std::cin >> filename; // read the filename from the user
                tree.saveToDisk(filename);
                break;
            }
            case 'L': {
                // Load
                std::string filename;
                std::cin >> filename; // read the filename
                tree.destroyTree();   // optional: clear existing in-memory tree
                tree.loadFromDisk(filename);
                // maybe print or do something
                tree.print(verbose);
                break;
            }
            default:
                std::cin.ignore(256, '\n');
                std::cout << usageMessage();
                break;
        }
    }
    return 0;
}
