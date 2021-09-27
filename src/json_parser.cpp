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

string& JsonParser::getPath()
{
    return path_;
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
        if (root_.isMember("path")){
            path_ = root_["path"].asString();
        }
    }
    catch(Json::Exception &e)
    {
    }
    
}

void JsonParser::Encode(int err_code, vector<DirEntry> &entries)
{
    try
    {
        root_.clear();
        root_["ack"] = Json::UInt64(seq_);
        root_["errcode"] = Json::Value(err_code);
        if (entries.size()){
            root_["trees"] = Json::arrayValue;
            item_ = Json::objectValue;
            for (auto e : entries){
                item_["filename"] = Json::Value(e.m_filename);
                item_["filetype"] = Json::Value(e.m_filetype);
                item_["inode"] = Json::UInt64(e.m_inode);
                item_["size"] = Json::UInt64(e.m_size);
                root_["trees"].append(item_);
            }
        }

        json_str_ = writer_.write(root_);
    }
    catch(const std::exception& e)
    {
    }
}