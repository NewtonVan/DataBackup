#include "copier.h"

void Copy(Header &h, int fd_src, int fd_dst, int padding_on)
{
    char buf[CP_BLOCK_SIZE+3];
    ssize_t buf_lth;
    ulong block_num = h.getNumBlock();

    for (ulong i = 0; i < block_num; ++i){
        buf_lth = read(fd_src, buf, CP_BLOCK_SIZE);
        if (-1 == buf_lth){
            throw CopyException(h.getFilePath(), "Error reading when Copying");
        }

        if (block_num-1 == i){
            if (padding_on){
                memset(buf+buf_lth, 0, h.getPadding());
                buf_lth = CP_BLOCK_SIZE;
            } else{
                buf_lth -= h.getPadding();
            }
        }

        if (write(fd_dst, buf, buf_lth) != buf_lth){
            throw CopyException(h.getFilePath(), "Error writing when Copying");
        }
    }
}