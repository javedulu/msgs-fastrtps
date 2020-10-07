#include "transmit.h"
#include "receive.h"

#include "HelloWorld.h"
#include "BigSize.h"
#include "Image.h"

#include <fastrtps/log/Log.h>
#include <fastrtps/Domain.h>

using eprosima::fastdds::dds::Log;

void messageRead(BigSize bigsize)
{
    std::cout<<" Found HelloWorld to read .. "<< sizeof(bigsize.data()) << std::endl;
}

int main(int argc , char **argv)
{
    auto fp  = std::bind(messageRead, std::placeholders::_1);
    std::vector<std::string> whitelist;
    Log::SetVerbosity(Log::Info);
    Receiver<BigSize> r("BigSizeTopic"); //= new Receiver<BigSize>("BigSizeTopic");
    r.init();
    //r.init("10.20.1.24", 5100, false, whitelist);
    r.receive(fp);
    r.run();

    Domain::stopAll();
    Log::Reset();
}
