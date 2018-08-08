#pragma once

#include <vector>
#include <string>



namespace dan
{
namespace log
{


class Logger
{
public:
    Logger() noexcept;

    void Restore(std::vector<std::string>* pstOpenid);

};


}
}
