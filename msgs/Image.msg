#include "std_msgs/Header.msg"

struct Image
{
    //Header header;
    unsigned long height;
    unsigned long width;
    string encoding;
    octet is_bigendian;
    unsigned long step;
    sequence<octet> data;
};

