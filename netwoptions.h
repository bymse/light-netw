#ifndef NETWORKS_NETWOPTIONS_H
#define NETWORKS_NETWOPTIONS_H

#include "netwcommon.h"

#define ADDR_FLAG "a"
#define PORT_FLAG "p"
#define DATA_FLAG "d"
#define TYPE_FLAG "t"

#define GCH(flag) flag[0]

#define OPT_STR ":"\
                  "%"ADDR_FLAG":" \
                  "%"PORT_FLAG":" \
                  "%"DATA_FLAG":" \
                  "%"TYPE_FLAG":" \


error_code parse_flags(int argc, char *argv[], netwopts *options);

#endif //NETWORKS_NETWOPTIONS_H
