//
// STARTER CODE: util.h
//
// TODO:  Write your own header and fill in all functions below.
//
#include <iostream>
#include <fstream>
#include <map>
#include <queue>          // std::priority_queue
#include <vector>         // std::vector
#include <functional>     // std::greater
#include <string>
#include "bitstream.h"
#include "hashmap.h"
#pragma once

struct HuffmanNode {
    int character;
    int count;
    HuffmanNode* zero;
    HuffmanNode* one;
};

//
// *This method frees the memory allocated for the Huffman tree.
//
void freeTree(HuffmanNode* node) {
    
    if(node == nullptr){
        return;
    } else{
        freeTree(node->zero);
        freeTree(node->one);
        delete node;
    }
    
}

//
// *This function builds the frequency map.  
//  If isFile is true, read from the file with name filename.  
//  If isFile is false, read directly from string filename.
//
void buildFrequencyMap(string filename, bool isFile, hashmap &map) {
    int keyFrequency = 0;
    bool emptyFile = true;
    if(isFile){
        ifstream myFile(filename);
        if(!myFile){
            return;
        }  
        char ch;        
        while(myFile.get(ch)){
            emptyFile = false;
            if(map.containsKey(ch)){
                keyFrequency = map.get(ch);
                map.put(ch, keyFrequency + 1);
            } else{
                map.put(ch, 1);
            }
        }
        if(emptyFile){
            map.put(PSEUDO_EOF, 1);
        }
        map.put(PSEUDO_EOF, 1);
        myFile.close();
    } else{
        if(filename.size() == 0){
            map.put(PSEUDO_EOF, 1);
            return;
        }
        for(char ch: filename){
            if(map.containsKey(ch)){
                keyFrequency = map.get(ch);
                map.put(ch, keyFrequency + 1);
            } else{
                map.put(ch, 1);
            }  
        }
        map.put(PSEUDO_EOF, 1);
    }
}

struct prioritize {
    bool operator()(HuffmanNode*& h1, HuffmanNode*& h2) {
        return h1->count > h2->count;
    }
};

//
// *This function builds an encoding tree from the frequency map.
//
HuffmanNode* buildEncodingTree(hashmap &map) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, prioritize> pq;
    vector<int> keyList = map.keys();
    
    for(int key: keyList){
        HuffmanNode* leaf = new HuffmanNode;
        leaf->character = key;
        leaf->count = map.get(key);
        leaf->zero = NULL;
        leaf->one = NULL;
        pq.push(leaf);
    }

    while(pq.size() > 1){
        HuffmanNode* nodeZero = pq.top();
        pq.pop();
        HuffmanNode* nodeOne = pq.top();
        pq.pop();
        int count = nodeZero->count + nodeOne->count;
        HuffmanNode* newNode = new HuffmanNode;
        newNode->count = count;
        newNode->zero = nodeZero;
        newNode->one = nodeOne;
        newNode->character = NOT_A_CHAR;
        pq.push(newNode);
    }
    return pq.top(); 
}

void traverse(HuffmanNode* tree, string path, map<int,string>& encodingMap){
    if(tree == nullptr){
        return;
    }
    if(tree->zero == nullptr && tree->one == nullptr){
        if(tree->character != NOT_A_CHAR){
            encodingMap[tree->character] = path;
        }
    } else{
        traverse(tree->zero, path + "0", encodingMap);
        traverse(tree->one, path + "1", encodingMap);
    }
}

//
// *This function builds the encoding map from an encoding tree.
//
map<int,string> buildEncodingMap(HuffmanNode* tree) {
    map<int,string> encodingMap;
    traverse(tree, "", encodingMap);
    return encodingMap;  // TO DO: update this return
}

//
// *This function encodes the data in the input stream into the output stream
// using the encodingMap.  This function calculates the number of bits
// written to the output stream and sets result to the size parameter, which is
// passed by reference.  This function also returns a string representation of
// the output file, which is particularly useful for testing.
//
string encode(ifstream& input, map<int,string> &encodingMap,
              ofbitstream& output, int &size, bool makeFile) {
    char ch;
    int intBit;
    string buffer = "";
    string bits = "";
    while(input.get(ch)){
        bits += encodingMap[ch];
    }
    bits += encodingMap[PSEUDO_EOF];
    size = bits.size();
    if(makeFile){
        for(char bit: bits){
            if(bit == '0'){
                intBit = 0;
            } else if(bit == '1'){
                intBit = 1;
            }
            output.writeBit(intBit);
        }
    } 
    return bits;
}


//
// *This function decodes the input stream and writes the result to the output
// stream using the encodingTree.  This function also returns a string
// representation of the output file, which is particularly useful for testing.
//
string decode(ifbitstream &input, HuffmanNode* encodingTree, ofstream &output) {
    string bits;
    string buffer;
    while(true){
        int bit = input.readBit();
        if(bit == -1){
            break;
        }
        if(bit == 0){
            bits += '0';
        } else if(bit == 1){
            bits += '1';
        }
    }
    HuffmanNode* cur = encodingTree;
    for(char bit: bits){
        if(bit == '0'){
            cur = cur->zero;
        } else{
            cur = cur->one;
        }
        if(cur->zero == nullptr && cur->one == nullptr){
            if(cur->character == PSEUDO_EOF){
                break;
            }
            buffer += cur->character;
            output.put(cur->character);
            cur = encodingTree;
        }
    }
    return buffer;  
}

//
// *This function completes the entire compression process.  Given a file,
// filename, this function (1) builds a frequency map; (2) builds an encoding
// tree; (3) builds an encoding map; (4) encodes the file (don't forget to
// include the frequency map in the header of the output file).  This function
// should create a compressed file named (filename + ".huf") and should also
// return a string version of the bit pattern.
//
string compress(string filename) {
    hashmap frequencyMap;
    buildFrequencyMap(filename, true, frequencyMap);

    HuffmanNode* encodingTree = buildEncodingTree(frequencyMap);

    map<int, string> encodingMap = buildEncodingMap(encodingTree);
    freeTree(encodingTree);

    ifstream input(filename);
    ofbitstream output(filename + ".huf");
    output << frequencyMap;
    int size;
    
    string encodedStr = encode(input, encodingMap, output, size, true);
    return encodedStr;  
}

//
// *This function completes the entire decompression process.  Given the file,
// filename (which should end with ".huf"), (1) extract the header and build
// the frequency map; (2) build an encoding tree from the frequency map; (3)
// using the encoding tree to decode the file.  This function should create a
// compressed file using the following convention.
// If filename = "example.txt.huf", then the uncompressed file should be named
// "example_unc.txt".  The function should return a string version of the
// uncompressed file.  Note: this function should reverse what the compress
// function does.
//
string decompress(string filename) {
    hashmap frequencyMap;
    ifbitstream input(filename);
    input >> frequencyMap;

    HuffmanNode* encodingTree = buildEncodingTree(frequencyMap);

    for(int i = 0; i < 8; i++){
        filename.pop_back();
    }
    ofstream output(filename + "_unc.txt");

    string decodedStr = decode(input, encodingTree, output);
    freeTree(encodingTree);
    return decodedStr;  
}
