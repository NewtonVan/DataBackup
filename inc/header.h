#ifndef HEADER_H
#define HEADER_H

// linux
#include <sys/stat.h>

// C++
#include <string>

class Header {
public:
    Header &GetInstance();

    std::string getFilePath();
    std::string getSymbol();
    size_t getLenFilePath();
    size_t getLenSymbol();
    ino_t getIno();
    mode_t getMode();
    nlink_t getNumLink();
    uid_t getUid();
    gid_t getGid();
    timespec getAccessTime();
    timespec getModifyTime();
    ulong getNumBlock();
    uint getPadding();
	
	void setFilePath(const std::string &file_path);
    void setSymbol(const std::string &ln_path);
    void setLenFilePath(const size_t file_path_len);
    void setLenSymbol(const size_t ln_path_len);
    void setIno(const ino_t st_ino);
    void setMode(const mode_t st_mode);
    void setNumLink(const nlink_t st_nlink);
    void setUid(const uid_t st_uid);
    void setGid(const gid_t st_gid);
    void setAccessTime(const timespec access_time);
    void setModifyTime(const timespec modify_time);
    void setNumBlock(const ulong block_num);
    void setPadding(const uint padding);
private:
    // 单例支持
    Header() = default;
    ~Header() = default;
    Header(const Header &h) = delete;
    const Header &operator=(const Header &h) = delete;
private:
    //若用数组保存路径，长度可能溢出
    //因此将路径保存在Header后面，并在Header中记录长度
    std::string        file_path_;            //文件的相对路径
    std::string        ln_path_;            //软链接内容
    //要保存的信息
    size_t            file_path_len_;        //相对路径的长度
    size_t            ln_path_len_;        //软连接内容的长度
    ino_t            st_ino_;              //索引节点号
    mode_t            st_mode_;            //文件类型、访问权限
    nlink_t            st_nlink_;            //硬链接数
    uid_t            st_uid_;              //文件uid
    gid_t            st_gid_;            //文件gid
    timespec        st_atime_;            //最后访问时间
    timespec        st_mtime_;            //最后修改时间
    ulong            block_num_;            //数据块的数量
    uint            padding_;            //最后一个数据块中补0的个数
};

#endif