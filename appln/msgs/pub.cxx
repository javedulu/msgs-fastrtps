#include "transmit.h"

#include "BigSize.h"
#include <random>
#include <algorithm>
#include <iterator>
#include <functional>
#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <fastrtps/log/Log.h>
#include <fastrtps/Domain.h>

using eprosima::fastdds::dds::Log;

int main(int argc, char **argv)
{
    std::vector<std::string> whitelist;
    Log::SetVerbosity(Log::Info);
    std::shared_ptr<BigSize> m = std::make_shared<BigSize>(); // new BigSize();

    Transmitter<BigSize> t("BigSizeTopic"); 
    t.init();
    std::vector<uint8_t> fdata;
    for (int j=0;j<1000; j++)
    {
        //fdata.push_back(std::to_string(j)[0]);
        m->fData().push_back(0);
    }
    //m->fData(fdata);
    //t.init("10.20.1.24", 5100, false, whitelist);
    std::cout<<"Sending sample @ 1000msec - 10 samples"<< std::endl;
    t.run();
    for (int i=0;i<10;i++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        t.send(m);
    }
    Domain::stopAll();
    Log::Reset();
}
