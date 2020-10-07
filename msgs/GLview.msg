#include "std_msgs/Header.msg"
#include "std_msgs/Matrix.msg"

struct GLview
{
    //Header header;
    Matrix view;
    Matrix perspective;
};