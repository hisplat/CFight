

#include "logging.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <iomanip>
#include <iostream>

#if defined(ANDROID)
#include "android/log.h"
#include <utils/SystemClock.h>
#endif

#ifdef ANDROID
#define GETTID()    gettid()
#else
#define GETTID()    syscall(SYS_gettid)
#endif

/*
static unsigned int GetTickCount(void)
{
    struct sysinfo  si;
    sysinfo(&si);
    return si.uptime;
}
*/

namespace logging {

static std::ostream* g_logging_stream = &std::cout;

LogMessage::LogMessage(const char * tag, const char * file, const char * func, int line, bool fatal, bool condition)
    : file_(file)
    , func_(func)
    , tag_(tag)
    , line_(line)
    , fatal_(fatal)
    , condition_(condition)
{
}

LogMessage::LogMessage(const char * tag)
    : tag_(tag)
    , line_(0)
    , fatal_(false)
    , condition_(true)
{
}

LogMessage::~LogMessage()
{
    if (!condition_) {
        return;
    }
    std::ostringstream  timeoss;
    std::ostringstream   oss;
    oss << "[" << getpid() << ":" << GETTID() << "] ";

    const char * t = getThreadLoggingTag();
    if (t != NULL) {
        oss << "(" << t << ") ";
    }

    struct tm local_time;
    struct timeval tv;
    memset(&local_time, 0, sizeof(local_time));
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &local_time);

#if defined(ANDROID)
    // struct sysinfo  si;
    // sysinfo(&si);
    // oss << '[' << si.uptime << "] ";

    oss << '[' << android::uptimeMillis() << "] ";
#endif
    timeoss << '[';
    timeoss << std::setfill('0')
            << std::setw(2) << local_time.tm_hour
            << ':'
            << std::setw(2) << local_time.tm_min
            << ':'
            << std::setw(2) << local_time.tm_sec
            << ':'
            << std::setw(6) << tv.tv_usec
            << "] ";

    if (!file_.empty()) {
        size_t  pos = file_.find_last_of("/\\");
        if (pos != std::string::npos)
            file_ = file_.substr(pos + 1);
        oss << '[' << file_ << ':' << func_ << ':' << line_ << "] ";
    }

    std::string st = stream_.str();
    size_t pos = st.find_last_not_of("\r\n ");
    if (pos != std::string::npos)
        st = st.substr(0, pos + 1);
    oss << ' ' << st;
    oss << TERMC_NONE;
    oss << std::endl;

#if defined(ANDROID)
    __android_log_write(ANDROID_LOG_ERROR, tag_.c_str(), oss.str().c_str());
#endif
    // printf("%s",  oss.str().c_str());
    // fflush(stdout);
    (*g_logging_stream) << timeoss.str() << oss.str();
    (*g_logging_stream).flush();

    if (fatal_) {
        abort();
    }
}


static pthread_key_t    g_key = (pthread_key_t)(-1);
void setThreadLoggingTag(const char * tag)
{
    if (g_key == (pthread_key_t)(-1)) {
        pthread_key_create(&g_key, NULL);
    }
    pthread_setspecific(g_key, tag);
}

const char * getThreadLoggingTag()
{
    if (g_key == (pthread_key_t)(-1)) {
        return NULL;
    }
    return (const char *)pthread_getspecific(g_key);
}

void setLoggingStream(std::ostream& o)
{
    g_logging_stream = &o;
}

} // namespace

void logging_printf(const char * filename, const char * func, int line, const char * fmt, ...)
{
    char temp[4096];
    va_list args; 
    va_start(args, fmt);
    vsnprintf(temp, sizeof(temp), fmt, args);
    va_end(args);
    if (filename == NULL || func == NULL) {
        ::logging::LogMessage("CF").stream() << temp;
    } else {
        ::logging::LogMessage("CF", filename, func, line).stream() << temp;
    }  
}

