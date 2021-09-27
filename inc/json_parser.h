#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <jsoncpp/json/json.h>
#include <string>
#include <vector>

#include "exception_interface.h" 

using std::string;
using std::vector;

struct DirEntry {
    string m_filename;
    string m_filetype;
    uint64_t m_inode;
    uint64_t m_size;
};

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
    string& getPath();
    void Decode(const std::string &input);
    void Encode(int err_code, vector<DirEntry> &file_nm);
private:
    Json::Reader reader_;
    Json::Value root_;
    Json::StyledWriter writer_;
    Json::Value item_;
    string method_;
    string src_;
    string dst_;
    string json_str_;
    string path_;
    uint64_t seq_;
};

#endif