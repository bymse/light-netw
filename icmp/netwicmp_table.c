#include "netwicmp.h"

#define RCVD_AFTER_TO ~123


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
                     const char *state_str);

static inline const char *statestr_for(int code) {
    switch (code) {
        case Noerr:
            return "OK";
        case DestinationUnreach:
            return "Destination Unreachable";
        case Timerr:
            return "Timeout, over "TOSTR(RCV_TO_SEC)" s";
        case RCVD_AFTER_TO:
            return "Recived after timeout";
        default:
            return "";
    }
}

void print_ping_table(size_t packets_c, icmp_transmis packets[packets_c]) {
    int sent_no = -1;
    int rcvd_no = -1;
    const sockaddr_storage *to = NULL;
    const sockaddr_storage *from = NULL;
    clock_t time_span = -1;

    int state_code = 0;

    PRINT_TABLE_HEADER();
    for (int index = 0; index < packets_c; ++index) {
        if (packets[index].state == Sent) {
            __auto_type sent_packet = packets[index];

            sent_no = sent_packet.header.sequence_no;
            to = &sent_packet.addr;

            if (index < packets_c - 1) {
                __auto_type next_packet = packets[index + 1];

                if (next_packet.state == Recived
                    && next_packet.header.sequence_no == sent_packet.header.sequence_no) {

                    rcvd_no = next_packet.header.sequence_no;
                    from = &next_packet.addr;
                    time_span = next_packet.time - sent_packet.time;
                    state_code = next_packet.header.type;
                    index++;
                } else state_code = Timerr;
            } else state_code = Timerr;

        } else if (packets[index].state == Recived) {
            __auto_type rcvd_packet = packets[index];

            rcvd_no = rcvd_packet.header.sequence_no;
            from = &rcvd_packet.addr;
            state_code = rcvd_packet.header.type == 0
                         ? RCVD_AFTER_TO
                         : rcvd_packet.header.type;

            for (int back_index = index - 1; back_index >= 0; --back_index) {
                __auto_type prev_packet = packets[back_index];
                if (prev_packet.state == Sent &&
                    prev_packet.header.sequence_no == rcvd_no) {

                    time_span = rcvd_packet.time - prev_packet.time;
                    break;
                }
            }

        } else {
            WRITE("Invalid packet state");
            continue;
        }

        print_table_row(sent_no,
                        to,
                        from,
                        rcvd_no,
                        time_span,
                        statestr_for(state_code));

        sent_no = -1;
        rcvd_no = -1;
        to = NULL;
        from = NULL;
        time_span = -1;
    }
}

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

    char time_span_str[12 + 1] = "---";

    if (time_span >= 0) {
        sprintf(time_span_str, "%li", time_span);
    }

    RAW_PRINT(TABLE_COLUMN_FORMAT, sent_no_str, toaddr_str, fromaddr_str, rcvd_no_str, time_span_str, state_str);
}