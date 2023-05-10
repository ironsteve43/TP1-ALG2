#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <cstring>
#include <cstdlib>
#include <map>
#include <unordered_map>
#include <unistd.h>

using namespace std;

struct Node {
    int index;  // index of the node in the trie
    int depth;  // depth of the node in the trie
    char c;     // character associated with the node
    Node* parent;   // pointer to the parent node
    unordered_map<char, Node*> children; // pointers to the children nodes
    
    Node(int index, int depth, char c, Node* parent) {
        this->index = index;
        this->depth = depth;
        this->c = c;
        this->parent = parent;
    }
};

class Trie {
public:
    Trie() {
        root = new Node(0, 0, '\0', nullptr);
        nodes.push_back(root);
    }
    
    // insert a new string into the trie
    void insert(string s) {
        Node* current = root;
        for (char c : s) {
            if (current->children.find(c) == current->children.end()) {
                Node* node = new Node(nodes.size(), current->depth+1, c, current);
                current->children[c] = node;
                nodes.push_back(node);
            }
            current = current->children[c];
        }
    }
    
    // get the index of a string in the trie
    int getIndex(string s) {
        Node* current = root;
        for (char c : s) {
            if (current->children.find(c) == current->children.end()) {
                return -1;
            }
            current = current->children[c];
        }
        return current->index;
    }
    
    // get the vector of nodes in the trie
    vector<Node*> getNodes() {
        return nodes;
    }
    
private:
    Node* root;
    vector<Node*> nodes;
};

int compress(string input_file, string output_file)
{
    // read input file
    string text;
    ifstream in(input_file);
    if (in.is_open()) {
        in.seekg(0, ios::end);
        text.reserve(in.tellg());
        in.seekg(0, ios::beg);
        text.assign((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
        in.close();
    } else {
        cerr << "Unable to open input file\n";
        return 1;
    }
    
    // compress the text using LZ78
    Trie trie;
    vector<pair<int, char>> code;   // (index, next char) pairs
    string buffer;
    for (char c : text) {
        buffer += c;
        int index = trie.getIndex(buffer);
        if (index == -1) {
            // add new string to the trie
            code.push_back({trie.getIndex(buffer.substr(0, buffer.length()-1)), c});
            trie.insert(buffer);
            buffer = "";
        }
    }
    if (buffer.length() > 0) {
        code.push_back({trie.getIndex(buffer), '\0'});
    }
    
    // write the compressed code to output file
    ofstream out(output_file);
    if (out.is_open()) {
        for (auto p : code) {
            out << p.first << '^' << p.second;
        }
        out.close();
    } else {
        cerr << "Unable to open output file\n";
        return 1;
    }
    
    cout << "Compression complete\n";
    return 0;
}

void decompress(string input_file, string output_file) {
    // read input file
    string compressed_code;
    ifstream in(input_file);
    if (in.is_open()) {
        in.seekg(0, ios::end);
        compressed_code.reserve(in.tellg());
        in.seekg(0, ios::beg);
        compressed_code.assign((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
        in.close();
    } else {
        cerr << "Unable to open input file\n";
        return;
    }

    // decompress the code using LZ78
    unordered_map<int, string> dictionary = {{0, ""}};
    string buffer = "";
    string decompressed_text = "";
    int count = 0;
    for (int i = 0; i < compressed_code.size(); i += 2) {
        string index = "";
        while(compressed_code[i] != '^')
        {
            index = index + compressed_code[i];
            i++;
        }
        int iindex = stoi(index);
        char c;
        if(compressed_code[i+1] != (char)0)
        {
            c = compressed_code[i+1];
        }
        else
        {
            c = (char)32;
        }
        if (dictionary.find(iindex) == dictionary.end()) {
            buffer += buffer[0];
            buffer += c;
        } else {
            buffer = dictionary[iindex] + c;
        }
        decompressed_text += buffer;
        dictionary[dictionary.size()] = buffer;
        buffer = "";
    }

    // write the decompressed text to output file
    ofstream out(output_file);
    if (out.is_open()) {
        out << decompressed_text;
        out.close();
    } else {
        cerr << "Unable to open output file\n";
        return;
    }

    cout << "Decompression complete\n";
}


int main(int argc, char** argv) {
    // check if the correct number of arguments were passed
    if (argc < 3 || argc > 5) {
        cerr << "Usage: " << argv[0] << " -c/-x <input_file> [-o <output_file>]" << endl;
        return 1;
    }

    // parse command line arguments
    string operation = argv[1];
    string input_file = argv[2];
    string output_file = (argc > 3 && strcmp(argv[3], "-o") == 0) ? argv[4] : "";

    // check if input file exists and can be opened
    ifstream in(input_file);
    if (!in.is_open()) {
        cerr << "Unable to open input file " << input_file << endl;
        return 1;
    }
    in.close();

    // compress or decompress the file
    if (operation == "-c") {
        compress(input_file, output_file.empty() ? input_file.substr(0, input_file.find_last_of('.')) + ".lz78" : output_file);
    } else if (operation == "-x") {
        decompress(input_file, output_file.empty() ? input_file.substr(0, input_file.find_last_of('.')) + "_d.txt": output_file);
    } else {
        cerr << "Invalid operation " << operation << ". Must be -c or -x." << endl;
        return 1;
    }

    return 0;
}

