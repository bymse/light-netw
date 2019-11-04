#ifndef NETWORKS_NETWOPTIONS_H
#define NETWORKS_NETWOPTIONS_H

#include "netwcommon.h"

#define ADDR_FLAG 'a'
#define PROTOCOL_FLAG 'r'
#define PORT_FLAG 'p'
#define TYPE_FLAG 't'
#define INPUT_PATH 'i'
#define OUTPUT_PATH 'o'


#define OPT_STR (char[]){':', ADDR_FLAG, ':', PORT_FLAG, ':', INPUT_PATH, ':', TYPE_FLAG, ':', OUTPUT_PATH, ':', PROTOCOL_FLAG, ':' ,'\0'}

error_code parse_flags(int argc, char *argv[], netwopts *options);

#endif //NETWORKS_NETWOPTIONS_H
