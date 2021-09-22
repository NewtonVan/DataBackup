#include "copier.h"

void Copy(int fd_src, int fd_dst)
{
    int buf[CP_BLOCK_SIZE+3];
    ssize_t buf_lth;
    Header &h = Header::GetInstance();
    ulong block_num = h.getNumBlock();

    for (ulong i = block_num-1; i >= 0; --i){
        buf_lth = read(fd_src, buf, CP_BLOCK_SIZE);
        if (-1 == buf_lth){
            throw CopyException(h.getFilePath(), "Error reading when Copying");
        }

        if (0 == i){
            memset(buf+buf_lth, 0, h.getPadding());
        }

        if (CP_BLOCK_SIZE != write(fd_dst, buf, CP_BLOCK_SIZE)){
            throw CopyException(h.getFilePath(), "Error writing when Copying");
        }
    }
}