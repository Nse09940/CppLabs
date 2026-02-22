#include "ArgParser.h"
#include <iostream>

namespace ArgumentParser {

ArgParser::ArgParser(std::string) {
}

bool ArgParser::SetFlag(std::string x) {
    if (mm_flags.find(x) != mm_flags.end()) {
        *mm_flags[x] = true;
        return true;
    } else {
        return false;
    }
}

bool ArgParser::IsDigit(std::string x) {
    for (auto sym : x) {
        if (!std::isdigit(sym)) {
            return false;
        }
    }
    return true;
}

bool ArgParser::SetArgumentValue(std::string name, std::string value) {
    if (mm_str.find(name) != mm_str.end()) {
        if (*mm_str[name] == "_INT") {
            if (IsDigit(value)) {
                value = "_INT" + value;
                *mm_str[name] = value;
                return true;
            } else {
                return false;
            }
        }
        *mm_str[name] = value;
        return true;
    } else if (mm_vec.find(name) != mm_vec.end()) {
        if (IsDigit(value)) {
            std::vector<int>& aa = *mm_vec[name];
            aa.push_back(stoi(value));
            Flag_Len_Vec--;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

ArgParser& ArgParser::AddFlag(char x, std::initializer_list<std::string> params) {
    LastAddArgs.clear();
    std::string name_save = std::string(1, x);
    mm_flags[name_save] = new bool(false);
    LastAddArgs.push_back(name_save);
    for (auto name : params) {
        LastAddArgs.push_back(name);
        mm_flags[name] = mm_flags[name_save];
    }
    return *this;
}
ArgParser& ArgParser::AddFlag(std::initializer_list<std::string> params) {
    LastAddArgs.clear();
    
    std::string name_save;
    for (auto name : params) {
        LastAddArgs.push_back(name);
        if(name_save.empty()){
            name_save = name;
            mm_flags[name] = new bool(false);
        }else{
            mm_flags[name] = mm_flags[name_save];
        }
    }
    return *this;
}

bool ArgParser::GetFlag(std::string value) {
    return *mm_flags[value];
}

bool ArgParser::Parse(const std::vector<std::string>& args) {
    if (args.empty()) return true;

    for (size_t i = 1; i < args.size(); ++i) {
        const std::string& arg = args[i];

        if (arg.starts_with("--")) {
            int pos = arg.find('=');
            std::string name = arg.substr(2, pos - 2);
            if (pos != std::string::npos) {
                std::string value = arg.substr(pos + 1);
                SetArgumentValue(name, value);
            } else {
                if (!SetFlag(name)) {
                    return false;
                }
            }
        } else if (arg.starts_with("-")) {
            int pos = arg.find('=');
            std::string name = arg.substr(1, pos - 1);
            if (pos != std::string::npos) {
                std::string value = arg.substr(pos + 1);
                SetArgumentValue(name, value);
            } else {
                if (!SetFlag(name)) {
                    return false;
                }
            }
        } else {
            if (!Flag_Pos || mm_vec.empty()) {
                return false;
            }
            (*(*mm_vec.begin()).second).push_back(stoi(arg));
        }
    }

    for (auto MapValue : mm_str) {
        if (MapValue.second->empty() || *MapValue.second == "_INT") {
            return false;
        }
    }
    if (Flag_Len_Vec > 0) {
        return false;
    }
    return true;
}

ArgParser& ArgParser::AddStringArgument(char shortName, std::initializer_list<std::string> params) {
    LastAddArgs.clear();
    std::string aa(1, shortName);
    LastAddArgs.push_back(aa);
    mm_str[aa] = new std::string;
    for (const auto& param : params) {
        LastAddArgs.push_back(param);
        mm_str[param] = mm_str[aa];
    }
    return *this;
}

ArgParser& ArgParser::AddStringArgument(std::initializer_list<std::string> params) {
    LastAddArgs.clear();
    std::string aa;
    int i = 0;
    for (auto param : params) {
        LastAddArgs.push_back(param);
        if (i == 0) {
            mm_str[param] = new std::string;
            aa = param;
        } else {
            mm_str[param] = mm_str[aa];
        }
        i++;
    }
    return *this;
}

ArgParser& ArgParser::Default(const std::string value) {
    *mm_str[LastAddArgs[0]] = value;
    return *this;
}

ArgParser& ArgParser::Default(const bool value) {
    *mm_flags[LastAddArgs[0]] = value;
    return *this;
}
ArgParser& ArgParser::Default(const char* value1) {
    std::string value = value1;
    *mm_str[LastAddArgs[0]] = value;
    return *this;
}

std::string ArgParser::GetStringValue(std::string value) {
    auto x = mm_str[value];
    return *x;
}

void ArgParser::StoreValue(std::string* value) {
    delete mm_str[LastAddArgs[0]];
    for (auto name : LastAddArgs) {
        mm_str[name] = value;
    }
}

void ArgParser::StoreValue(bool* value) {
    delete mm_flags[LastAddArgs[0]];
    for (auto name : LastAddArgs) {
        mm_flags[name] = value;
    }
}

void ArgParser::StoreValues(std::vector<int>* value) {
    delete mm_vec[LastAddArgs[0]];
    for (auto name : LastAddArgs) {
        mm_vec[name] = value;
    }
}

ArgParser& ArgParser::AddIntArgument(char shortName, std::initializer_list<std::string> params) {
    LastAddArgs.clear();
    std::string aa(1, shortName);
    LastAddArgs.push_back(aa);
    mm_str[aa] = new std::string("_INT");
    for (const auto& param : params) {
        mm_str[param] = mm_str[aa];
    }
    return *this;
}

ArgParser& ArgParser::AddIntArgument(std::initializer_list<std::string> params) {
    LastAddArgs.clear();
    std::string aa;
    int i = 0;
    std::string def ="_INT";
    for (auto param : params) {
        LastAddArgs.push_back(param);
        if (i == 0) {
            mm_parse.setValue(param, new StringValue(&def));
            aa = param;
        } else {
            auto strPtr = static_cast<StringValue*>(mm_parse.getValue("stringKey"));
            mm_parse.setValue(param, strPtr);
        }
        i++;
    }
    return *this;
}

long long ArgParser::GetIntValue(std::string x, int ind) {
    std::string xx;
    auto strPtr = static_cast<StringValue*>(mm_parse.getValue(x));
    xx = *(strPtr->value);
    return std::stoll(xx.substr(4));
}

ArgParser& ArgParser::MultiValue(int valee) {
    delete mm_str[LastAddArgs[0]];
    for (auto xx : LastAddArgs) {
        mm_str.erase(xx);
    }
    std::string value;
    for (auto name : LastAddArgs) {
        if (value.empty()) {
            value = name;
            mm_vec[name] = new std::vector<int>();
        } else {
            mm_vec[name] = mm_vec[value];
        }
    }
    if (valee != 1) {
        Flag_Len_Vec = valee;
    }
    return *this;
}

ArgParser& ArgParser::Positional() {
    Flag_Pos = 1;
    return *this;
}

void ArgParser::AddHelp(const char str, std::initializer_list<std::string> params) {
    AddFlag(str, params);
}

bool ArgParser::Help() {
    std::cout << "Description of the ArgParser class methods:\n";

std::cout << "1. Parse - method that returns a bool. Parses the arguments.\n";
std::cout << "2. GetStringValue - method that returns the value of an argument by key for string.\n";
std::cout << "3. GetIntValue - method that returns the value of an argument by key for int.\n";
std::cout << "4. AddStringArgument - method that returns the parser, allows adding a string argument.\n";
std::cout << "5. AddIntArgument - method that returns the parser, allows adding an int argument.\n";
std::cout << "6. MultiValue() - method that returns the parser, allows adding arguments as an array.\n";
std::cout << "7. Default - method that returns the parser, sets the default value for the element passed in the argument.\n";
std::cout << "8. StoreValue(value); - method that returns the parser, the last added argument has the alias value.\n";
std::cout << "9. AddFlag('f', \"flag1\"); - allows adding flags.\n";
std::cout << "10. Default() - sets the default value for the argument.\n";

    return true;
}

} 
