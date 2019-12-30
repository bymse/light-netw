#include "netwicmp.h"

/*
No. |To              |From            |No. |Time ms|State
1   |123.456.789.000 |987.765.432.100 |1   |10000  |Destination Unreachable   
 */
//                          No   To   From No  Tim State
#define TABLE_COLUMN_FORMAT "%4s|%16s|%16s|%4s|%12s|%s\r\n"
#define PRINT_TABLE_HEADER() RAW_PRINT("\r\n"TABLE_COLUMN_FORMAT, "No.", "Sent to", "Recived from", "No.", "Time ms", "State")

void print_table_row(int sent_no,
                     const sockaddr_storage *to,
                     const sockaddr_storage *from,
                     int rcvd_no,
                     clock_t time_span,
                     const char *state_str) {
    char toaddr_str[INET6_ADDRSTRLEN] = {0};
    char fromaddr_str[INET6_ADDRSTRLEN] = {0};

    if (to != NULL) {
        tostr_addr(to, toaddr_str);
    }

    if (from != NULL) {
        tostr_addr(from, fromaddr_str);
    }

    char sent_no_str[3 + 1] = {0};
    char rcvd_no_str[3 + 1] = {0};

    if (sent_no >= 0) {
        sprintf(sent_no_str, "%i", sent_no);
    }

    if (rcvd_no >= 0) {
        sprintf(rcvd_no_str, "%i", rcvd_no);
    }

    char time_span_str[12 + 1] = "No time";

    if (time_span >= 0) {
        sprintf(time_span_str, "%li", time_span);
    }

    RAW_PRINT(TABLE_COLUMN_FORMAT, sent_no_str, toaddr_str, fromaddr_str, rcvd_no_str, time_span_str, state_str);
}