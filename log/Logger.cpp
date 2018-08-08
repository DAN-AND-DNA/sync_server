#include <sync_server/log/Logger.h>
#include <fstream>
#include <sstream>
#include <stack>
//#include <string>


namespace dan
{
namespace log
{

Logger::Logger() noexcept
{
}

void Logger::Restore(std::vector<std::string>* pstOpenids)
{
    std::ifstream stIn("test.log");
    std::ostringstream stOs;
    std::stack<std::string> stStack;

  // printf("=================================\n");
 
    std::string strS;
    while(getline(stIn, strS))
    {
  //       printf("%s\n", strS.c_str());
        if(strS.find("pre:") != std::string::npos)
        {
     //       printf("find pre\n");
            stStack.push(strS);
        }
        if(strS.find("done:") != std::string::npos)
        {
            stStack.pop();
        }
    }


    //printf("==dan--------------%lu-\n",stStack.size());         
    //printf("=================================\n");

    while(!stStack.empty())
    {
        // 有失败 逐个处理
        if(stStack.top().find("pre:EXPIRE") != std::string::npos)
        {
            // 重新拿这几个openid 消息 写db 或者 mq
            uint64_t a[10]= {0};
            int j = 0;
            auto i = stStack.top().find(" ");

            while(i != stStack.top().npos)
            {
                
                a[j] = i;
                printf("%lu\n", a[j]);
                i = stStack.top().find(" ", i + 1);
                j++;
            }

            for(int m = 0; m < j-1 ; ++m)
                pstOpenids->push_back(stStack.top().substr(a[m]+1, a[m+1]-a[m]-1));

        }
        else if(stStack.top().find("pre:sync") != std::string::npos)
        {
            // 处理写mq或者db 未果

        }

        stStack.pop();
    }
}


}
}
