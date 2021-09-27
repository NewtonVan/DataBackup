#include "json_parser.h"

using std::vector;
using std::string;

string& JsonParser::getMethod()
{
    return method_;
}

string& JsonParser::getSrc()
{
    return src_;
}

string& JsonParser::getDst()
{
    return dst_;
}

string& JsonParser::getJsonString()
{
    return json_str_;
}

void JsonParser::Decode(const std::string &input)
{
    try
    {
        root_.clear();
        reader_.parse(input, root_);
        seq_ = root_["seq"].asUInt64();
        method_ = root_["method"].asString();
        if (root_.isMember("source")){
            src_ = root_["source"].asString();
        }
        if (root_.isMember("target")){
            dst_ = root_["target"].asString();
        }
    }
    catch(Json::Exception &e)
    {
    }
    
}

void JsonParser::Encode(int err_code, vector<string> &file_nm)
{
    try
    {
        root_.clear();
        root_["ack"] = Json::UInt64(seq_);
        root_["errcode"] = Json::Value(err_code);
        if (file_nm.size()){
            root_["filename"] = Json::arrayValue;
            for (auto str : file_nm){
                root_["filename"].append(str);
            }
        }

        json_str_ = writer_.write(root_);
    }
    catch(const std::exception& e)
    {
    }
}