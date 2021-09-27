#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <jsoncpp/json/json.h>
#include <string>
#include <vector>

#include "exception_interface.h" 

using std::string;
using std::vector;

class JsonParseException : public BaseException{
public:
    JsonParseException(const string &file_nm, const string &msg)
        : BaseException(file_nm, msg)
    {

    }
    string what()
    {
        return "JsonParse Exception : "+BaseException::what();
    }
};

class JsonParser{
public:
    JsonParser() = default;
    ~JsonParser() = default;
    string& getMethod();
    string& getSrc();
    string& getDst();
    string& getJsonString();
    void Decode(const std::string &input);
    void Encode(int err_code, vector<string> &file_nm);
private:
    Json::Reader reader_;
    Json::Value root_;
    Json::StyledWriter writer_;
    string method_;
    string src_;
    string dst_;
    string json_str_;
    uint64_t seq_;
};

#endif