/********************************************
Compile: g++ ch3_13_Strptime_Bad.cpp -o example
Usage: ./example
 ********************************************/
#include <cstdio>
#include <ctime>
int main()
{
    const char *format = "%Y-%m-%d %H:%M";
    const char *timeStr = "1970-1-1 0:1";
    struct tm time_info = {0}; // 记住初始化
    if(NULL == strptime(timeStr, format, &time_info))
    {
        fprintf(stderr, "error\n");
    }
    else
    {
        time_t secondsFrom1970 = timegm(&time_info); // 60s
        fprintf(stdout, "time =[%s], time in seconds from 1970 is [%lu]\n", timeStr, secondsFrom1970);
        fprintf(stdout, "[%d]\n", time_info.tm_sec);
        fprintf(stdout, "[%d]\n", time_info.tm_min);
        fprintf(stdout, "[%d]\n", time_info.tm_hour);
        fprintf(stdout, "[%d]\n", time_info.tm_mday);
        fprintf(stdout, "[%d]\n", time_info.tm_mon);
        fprintf(stdout, "[%d]\n", time_info.tm_year);
        fprintf(stdout, "[%d]\n", time_info.tm_wday);
        fprintf(stdout, "[%d]\n", time_info.tm_yday);
        fprintf(stdout, "[%d]\n", time_info.tm_isdst);
    }
    return 0;
}
