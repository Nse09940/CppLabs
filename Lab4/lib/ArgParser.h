#include <iostream>
#include <vector>
#include <map>
#pragma once

/*
Parse - метод возвращающий bool.  Делает парсинг аргументов.
GetStringValue - метод возвращает значение аргумента по ключу для инт
GetIntValue- метод возвращает значение аргумента по ключу для стринг
AddStringArgument - метод возвращающий parser, позволяет добавить аргумент string
AddIntArgument  - метод возвращающий parser, позволяет добавить аргумент int
MultiValue() - метод возвращающий parser, позволяет добавлять аргументы в виде массива
Default -  метод возвращающий parser указывает дефолтное значение для элемента, переданного в аргументе
StoreValue(value); - метод возвращающий parser, последний add argument имеет псевдоним value
AddFlag('f', "flag1"); - позволяет добавлть флаги
Default()устанавливает значение аргумента по умолчанию
*/

#include <string>
#include <vector>
#include <map>
#include <initializer_list>
#include <cctype>

namespace ArgumentParser {




class BaseValue {
public:

};


class StringValue : public BaseValue {
public:
    const std::string* value;
    StringValue(std::string* val) : value(val) {}
};

class VectorValue : public BaseValue {
public:
    std::vector<int>* value;
    VectorValue(std::vector<int>* val) : value(val) {}
};

class BoolValue : public BaseValue {
public:
    bool* value;
    BoolValue(bool* val) : value(val) {}
};

class ArgMap {
public:
    void setValue(const std::string& key, BaseValue* value) {
        mm_values[key] = value;
    }

    BaseValue* getValue(const std::string& key) {
        return mm_values[key];
    }

private:
    std::map<std::string, BaseValue*> mm_values;
};





class ArgParser {
private:
    std::vector<std::string> LastAddArgs;
    int Flag_Pos = 0;
    int Flag_Len_Vec = 0;

    bool SetFlag(std::string x);
    bool IsDigit(std::string x);
    bool SetArgumentValue(std::string name, std::string value);
     //std::map<std::string, std::string*> mm_str;
     //std::map<std::string, std::vector<int>*> mm_vec;
     //std::map<std::string, bool*> mm_flags;
    ArgMap mm_parse;
    

public:
    ArgParser(std::string = "");

    ArgParser& AddFlag(char x, std::initializer_list<std::string> params);
    bool GetFlag(std::string value);
    
    bool Parse(const std::vector<std::string>& args);
    
    ArgParser& AddStringArgument(char shortName, std::initializer_list<std::string> params);
    ArgParser& AddStringArgument(std::initializer_list<std::string> params);
    ArgParser& Default(const std::string value);
    ArgParser& Default(const bool value);
    ArgParser& Default(const char* value1);
    
    std::string GetStringValue(std::string value);
    
    void StoreValue(std::string* value);
    void StoreValue(bool* value);
    void StoreValues(std::vector<int>* value);
    
    ArgParser& AddIntArgument(char shortName, std::initializer_list<std::string> params);
    ArgParser& AddIntArgument(std::initializer_list<std::string> params);
    long long GetIntValue(std::string x, int ind = -1);
    
    ArgParser& MultiValue(int valee = 1);
    ArgParser& Positional();
    ArgParser& AddFlag(std::initializer_list<std::string> params);
    
    void AddHelp(const char str, std::initializer_list<std::string> params);
    bool Help();


};
}
