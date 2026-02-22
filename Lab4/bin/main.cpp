#include <functional>
#pragma once
#include "../lib/ArgParser.h"

#include <iostream>
#include <numeric>

struct Options {
    bool sum = false;
    bool mult = false;
};

int main(int argc, char** argv) {
    std::vector<std::string> vvod;
    for (int i = 0; i < argc; i++)
    {
       vvod.push_back(argv[i]);
    }
    
    Options opt;
    //мой код нормальны?
    std::vector<int>as;
    as.push_back(12);
    as.push_back(13);
    as.push_back(14);
    as.push_back(15);
    as.push_back(16);
    as.push_back(17);
    as.push_back(18);
    as.push_back(19);
    as.push_back(20);
    as.push_back(21);   

    
    std::vector<int> values;
    
    ArgumentParser::ArgParser parser("Program");
    parser.AddIntArgument({"N"}).MultiValue(1).Positional().StoreValues(&values);
    parser.AddFlag({"sum", "add args"}).StoreValue(&opt.sum);
    parser.AddFlag({"mult", "multiply args"}).StoreValue(&opt.mult);
    
    if(!parser.Parse(vvod)) {
        std::cout << "Wrong argument" << std::endl;
        std::cout << parser.Help() << std::endl;
        return 1;
    }

    
    if(opt.sum) {
        std::cout << "Result: " << std::accumulate(values.begin(), values.end(), 0) << std::endl;
    } else if(opt.mult) {
        std::cout << "Result: " << std::accumulate(values.begin(), values.end(), 1, std::multiplies<int>()) << std::endl;
    } else {
        std::cout << "No one options had chosen" << std::endl;
        std::cout << parser.Help();
        return 1;
    }
    //labwork4 -N=1 -N=2 -N=3 -sum

    return 0;

}
