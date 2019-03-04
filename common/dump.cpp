
#include "dump.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "logging.h"


void dump(const void * data, int len)
{
    const char *    p = (const char *)(data);
    int             i;
    int             j;
    int             t;
    int             k;

    char            temp[4096];
    std::string     line;

    for(i=0; i<len; i+=16)
    {
        snprintf(temp, sizeof(temp), "%08x: ", i + (int)(unsigned long)data);
        line += temp;
        t = 16;
        if(i + 16 > len)
            t = len - i;
        if(t == 16)
        {
            for(k=i; k<len; k++)
            {
                if(p[k] != '\0')
                    break;
            }
            if(k - i > 16 * 3)
            {
                k = k - (( k - i ) % 16);
                snprintf(temp, sizeof(temp), " 00 * %d lines\n", (k -i) / 16);
                line += temp;
                SLOG(Dump) << line;
                line = "";
                i = k - 16; 
                continue;
            }
        }
        for(j=0; j<t; j++)
        {
            snprintf(temp, sizeof(temp), "%02x ", p[i+j] & 0xff);
            line += temp;
            if((j % 4) == 3)
                line += " ";
        }
        for(j=t; j<16; j++)
        {
            line += "   ";
            if((j % 4) == 3)
                line += " ";
        }
        line += "\t";
        for(j=0; j<t; j++)
        {
            if((p[i+j] >= 36 && p[i+j] <= 126) || p[i+j] == '"') {
                snprintf(temp, sizeof(temp), "%c", p[i+j] & 0xff);
                line += temp;
            } else {
                line += ".";
            }
        }
        SLOG(Dump) << line << "\n";
        line = "";
    }
}


#if 0
int main()
{
    const char * a = "test";
    tools::dump(a, 4);
    return 0;
}
#endif

