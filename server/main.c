
#include "game.h"
#include <time.h>
#include <getopt.h>
#include "logging.h"

unsigned int simple_hash(const char * v)
{
    unsigned int hash = 1315423911;
    while (*v) {
        hash ^= ((hash << 5) + (*v++) + (hash >> 2));
    }

    return (hash & 0x7FFFFFFF);
}

int main(int argc, char * argv[])
{
    unsigned int seed = 0;

    int c;
    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"seed", required_argument, 0, 's'},
            {0, 0, 0, 0}
        };
        c = getopt_long(argc, argv, "s:", long_options, &option_index);
        if (c == -1) {
            break;
        }
        switch (c) {
        case 's':
            seed = simple_hash(optarg);
            break;
        default:
            break;
        }
    }
    cf_log("seed = %u\n", seed);
    return main_loop(11233, seed);
}

