#include "TimeStamp.msg"

struct Header
{
    unsigned long seq;
    string frame_id;
    TimeStamp stamp;
};