
#pragma once

#define TERMC_RED     "\033[1;31m"
#define TERMC_GREEN   "\033[1;32m"
#define TERMC_YELLOW  "\033[1;33m"
#define TERMC_BLUE    "\033[1;34m"
#define TERMC_PINK    "\033[1;35m"
#define TERMC_NONE    "\033[m"

#define TERMC_DRED     "\033[0;31m"
#define TERMC_DGREEN   "\033[0;32m"
#define TERMC_DYELLOW  "\033[0;33m"
#define TERMC_DBLUE    "\033[0;34m"
#define TERMC_DPINK    "\033[0;35m"

#define TERMC_BRED     "\033[1;31;40m"
#define TERMC_BGREEN   "\033[1;32;40m"
#define TERMC_BYELLOW  "\033[1;33;40m"
#define TERMC_BBLUE    "\033[1;34;40m"
#define TERMC_BPINK    "\033[1;35;40m"


#if defined(__GNUC__)
#   define __FUNC__     ((const char *) (__PRETTY_FUNCTION__))
#elif defined (__STDC_VERSION__) && __STDC_VERSION__ >= 19901L
#   define __FUNC__     ((const char *) (__func__))
#else
#   define __FUNC__     ((const char *) (__FUNCTION__))
#endif


#ifdef __cplusplus

#include <sstream>
#include <string>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

namespace logging {

class LogMessageVoidify {
public:
    LogMessageVoidify() {}
    void operator&(std::ostream&) {}
};

class LogMessage {
public:
    LogMessage(const char * tag, const char * file, const char * func, int line, bool fatal = false, bool condition = true);
    LogMessage(const char * tag);
    ~LogMessage();

    std::ostream&   stream() { return stream_; }

private:
    LogMessage() {}
    std::ostringstream  stream_;
    std::string file_;
    std::string func_;
    std::string tag_;
    int         line_;
    bool        fatal_;
    bool        condition_;
};

class ScopeTracer {
public:
    ScopeTracer(const char * func, const char * name, int line) : name_(name), func_(func), line_(line) {
        LogMessage("Tracer").stream() << TERMC_RED << "Into scope: " << name << " (" << func << ':' << line << ")" << TERMC_NONE;
        gettimeofday(&start_, NULL);

    }
    ~ScopeTracer() {
        struct timeval  tv;
        gettimeofday(&tv, NULL);
        long long tu = ((long long)tv.tv_sec * 1000000 + tv.tv_usec) - ((long long)start_.tv_sec * 1000000 + start_.tv_usec);
        LogMessage("Tracer").stream() << TERMC_GREEN << "Leave scope: " << name_ << " (" << func_ << ':' << line_ << ")" << TERMC_BLUE << " Time costs: " << tu << " us." << TERMC_NONE;
    }
private:
    const char * name_;
    const char * func_;
    int          line_;
    struct timeval  start_;
};

class TimerScopeTracer {
public:
    TimerScopeTracer(const char * func, const char * name, int line, long long dt) : name_(name), func_(func), line_(line), difftime_(dt) {
        // LogMessage().stream() << TERMC_RED << "Into scope: " << name << " (" << func << ':' << line << ")" << TERMC_NONE;
        gettimeofday(&start_, NULL);
    }
    ~TimerScopeTracer() {
        struct timeval  tv;
        gettimeofday(&tv, NULL);
        long long tu = ((long long)tv.tv_sec * 1000000 + tv.tv_usec) - ((long long)start_.tv_sec * 1000000 + start_.tv_usec);
        if (tu >= difftime_) {
            LogMessage("TimerTracer").stream() << TERMC_PINK << "Leave scope: " << name_ << " (" << func_ << ':' << line_ << ")" << TERMC_BLUE << " Time costs: " << tu << " us." << TERMC_NONE;
        }
    }
private:
    const char * name_;
    const char * func_;
    int          line_;
    struct timeval  start_;
    long long    difftime_;
};

void setThreadLoggingTag(const char * tag);
const char * getThreadLoggingTag();

void setLoggingStream(std::ostream& o);

} // namespace logging

#define ILOG(tag)   ::logging::LogMessageVoidify() & ::logging::LogMessage(#tag, __FILE__, __FUNC__, __LINE__).stream()
#define SLOG(tag)   ::logging::LogMessageVoidify() & ::logging::LogMessage(#tag).stream()
#define DLOG_IF(condition)   ::logging::LogMessageVoidify() & ::logging::LogMessage("HiRPC", __FILE__, __FUNC__, __LINE__, false, (condition)).stream()
#define NFLOG()   ::logging::LogMessageVoidify() & ::logging::LogMessage("HiRPC", __FILE__, __FUNC__, __LINE__).stream()
#define FATAL(tag)  ::logging::LogMessageVoidify() & ::logging::LogMessage(#tag, __FILE__, __FUNC__, __LINE__, true).stream() << TERMC_RED << "FATAL: "
#define TODO()  ::logging::LogMessageVoidify() & ::logging::LogMessage("HiRPC", __FILE__, __FUNC__, __LINE__, true).stream() << TERMC_RED << "[TODO] "
#define RUN_HERE()  ILOG(Debug) << TERMC_BYELLOW << "Run here! "
#define ATTENTION()  ILOG(Debug) << TERMC_RED << "[ATTENTION] "
#define NF_HERE()  NFLOG() << "Run here! "
#define PERROR() NFLOG() << "[" << errno << "] " << strerror(errno) << ". "
#define ALERT(tag)  ILOG(tag) << TERMC_RED << "Alert! " << TERMC_NONE
#define ScopeTrace(x)   ::logging::ScopeTracer x ## __LINE__(__FUNC__, #x, __LINE__)
#define DCHECK(condition)   if (!(condition)) FATAL("Assertion") << "Check failed. (" << #condition << ") "
#define TimerScopeTrace(x, t)   ::logging::TimerScopeTracer x ## __LINE__(__FUNC__, #x, __LINE__, t)

#endif

#ifdef __cplusplus
extern "C" {
#endif
void logging_printf(const char * filename, const char * func, int line, const char * fmt, ...);
#ifdef __cplusplus
}
#endif
#define cf_log(fmt, ...) logging_printf(__FILE__, __FUNC__, __LINE__, TERMC_NONE fmt, ##__VA_ARGS__)
#define cf_debug(fmt, ...) logging_printf(__FILE__, __FUNC__, __LINE__, TERMC_GREEN fmt, ##__VA_ARGS__)
#define cf_warning(fmt, ...) logging_printf(__FILE__, __FUNC__, __LINE__, TERMC_YELLOW fmt, ##__VA_ARGS__)
#define cf_error(fmt, ...) logging_printf(__FILE__, __FUNC__, __LINE__, TERMC_RED fmt, ##__VA_ARGS__)


