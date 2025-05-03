#include <iostream>
#include <regex>
#include "utility.h"
#include "parser.h"

#include <string>
#include <fstream>
using namespace std;

extern Abc_Frame_t * pAbc;

int maxMatchNumber = 0;
void removeGateNames(const std::string& filePath) {
    std::ifstream inputFile(filePath);
    std::string outputFilePath = filePath + ".modified";
    std::ofstream outputFile(outputFilePath);

    std::regex gateNameRegex("(\\bnot\\b|\\band\\b|\\bnand\\b|\\bor\\b|\\bxor\\b|\\bxnor\\b|\\bbuf\\b|\\bnor\\b)\\s*(\\w+)");

    for (std::string line; std::getline(inputFile, line); ) {
        std::string modifiedLine = std::regex_replace(line, gateNameRegex, "$1");

        outputFile << modifiedLine << std::endl;
    }

    inputFile.close();
    outputFile.close();
    rename(outputFilePath.c_str(), filePath.c_str());
}


InputStructure parseInput(const string& inputPath) {
    InputStructure result;
    ifstream inputFile;
    inputFile.open(inputPath);
    string cir1VerilogPath, cir2VerilogPath;
    inputFile >> cir1VerilogPath;
    int busNum = 0;
    inputFile >> busNum;
    for(int i = 0 ; i < busNum ; i++){
        int busSz = 0;
        inputFile >> busSz;
        vector<string> busVector;
        for(int q = 0 ; q < busSz ; q++){
            string busName;
            inputFile >> busName;
            busVector.push_back(busName);
        }
        result.cir1Bus.push_back(busVector);
    }
    inputFile >> cir2VerilogPath;
    inputFile >> busNum;
    for(int i = 0 ; i < busNum ; i++){
        int busSz = 0;
        inputFile >> busSz;
        vector<string> busVector;
        for(int q = 0 ; q < busSz ; q++){
            string busName;
            inputFile >> busName;
            busVector.push_back(busName);
        }
        result.cir2Bus.push_back(busVector);
    }
//    removeGateNames(cir1VerilogPath);
//    removeGateNames(cir2VerilogPath);
    result.cir1AIGPath = cir1VerilogPath + ".aig";
    result.cir2AIGPath = cir2VerilogPath + ".aig";
    string cmd1 = produceABCCommand(cir1VerilogPath, result.cir1AIGPath);
    string cmd2 = produceABCCommand(cir2VerilogPath, result.cir2AIGPath);
    exeAbcCmd(cmd1, "parser");
    exeAbcCmd(cmd2, "parser");
    return result;
}

string produceABCCommand(const string& inputPath, const string& outputPath) {
    //TODO test which command can reduce most network space
    return "read " + inputPath +"; strash; " + simplify + "write_aiger -s " + outputPath;
}