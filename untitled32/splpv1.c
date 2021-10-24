/*
 * SPLPv1.c
 * The file is part of practical task for System programming course.
 * This file contains validation of SPLPv1 protocol.
 */

/*
  Ôàìèëèÿ, Èìÿ, Îò÷åñòâî
  Ïîëûâÿíûé. Ãëåá. Àíäðååâè÷
  ¹ ãðóïïû - 13
*/



/*
---------------------------------------------------------------------------------------------------------------------------
# |      STATE      |         DESCRIPTION       |           ALLOWED MESSAGES            | NEW STATE | EXAMPLE
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
1 | INIT            | initial state             | A->B     CONNECT                      |     2     |
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
2 | CONNECTING      | client is waiting for con-| A<-B     CONNECT_OK                   |     3     |
  |                 | nection approval from srv |                                       |           |
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
3 | CONNECTED       | Connection is established | A->B     GET_VER                      |     4     |
  |                 |                           |        -------------------------------+-----------+----------------------
  |                 |                           |          One of the following:        |     5     |
  |                 |                           |          - GET_DATA                   |           |
  |                 |                           |          - GET_FILE                   |           |
  |                 |                           |          - GET_COMMAND                |           |
  |                 |                           |        -------------------------------+-----------+----------------------
  |                 |                           |          GET_B64                      |     6     |
  |                 |                           |        ------------------------------------------------------------------
  |                 |                           |          DISCONNECT                   |     7     |
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
4 | WAITING_VER     | Client is waiting for     | A<-B     VERSION ver                  |     3     | VERSION 2
  |                 | server to provide version |          Where ver is an integer (>0) |           |
  |                 | information               |          value. Only a single space   |           |
  |                 |                           |          is allowed in the message    |           |
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
5 | WAITING_DATA    | Client is waiting for a   | A<-B     CMD data CMD                 |     3     | GET_DATA a GET_DATA
  |                 | response from server      |                                       |           |
  |                 |                           |          CMD - command sent by the    |           |
  |                 |                           |           client in previous message  |           |
  |                 |                           |          data - string which contains |           |
  |                 |                           |           the following allowed cha-  |           |
  |                 |                           |           racters: small latin letter,|           |
  |                 |                           |           digits and '.'              |           |
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
6 | WAITING_B64_DATA| Client is waiting for a   | A<-B     B64: data                    |     3     | B64: SGVsbG8=
  |                 | response from server.     |          where data is a base64 string|           |
  |                 |                           |          only 1 space is allowed      |           |
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
7 | DISCONNECTING   | Client is waiting for     | A<-B     DISCONNECT_OK                |     1     |
  |                 | server to close the       |                                       |           |
  |                 | connection                |                                       |           |
---------------------------------------------------------------------------------------------------------------------------
IN CASE OF INVALID MESSAGE THE STATE SHOULD BE RESET TO 1 (INIT)
*/


#include "splpv1.h"
#include <string.h>

/* FUNCTION:  validate_message
 *
 * PURPOSE:
 *    This function is called for each SPLPv1 message between client
 *    and server
 *
 * PARAMETERS:
 *    msg - pointer to a structure which stores information about
 *    message
 *
 * RETURN VALUE:
 *    MESSAGE_VALID if the message is correct
 *    MESSAGE_INVALID if the message is incorrect or out of protocol
 *    state
 */

const char base64[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
                       0, 0, 0, 1, 1, 1,
                       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1,
                       1, 1, 1, 1, 1, 1,
                       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                       0, 0, 0, 0, 0, 0,
                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                       0, 0, 0, 0, 0, 0,
                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                       0, 0, 0, 0, 0, 0,
                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                       0, 0, 0, 0};

const char data[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
                     1, 1, 1, 1, 1,
                     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0};

int state = 1;
const char *commands[] = {"GET_DATA", "GET_FILE", "GET_COMMAND"};
const int commandsLength[] = {8, 8, 11};
int commandIndex;

enum test_status validate_message(struct Message *msg) {
    char *message = msg->text_message;
    if (msg->direction == 0) {
        if (state == 1) {
            if (!strcmp(message, "CONNECT")) {
                state = 2;
                return MESSAGE_VALID;
            }
        } else if (state == 3) {
            if (!strcmp(message, "GET_VER")) {
                state = 4;
                return MESSAGE_VALID;
            } else if (!strcmp(message, "GET_DATA")) {
                commandIndex = 0;
                state = 5;
                return MESSAGE_VALID;
            } else if (!strcmp(message, "GET_FILE")) {
                commandIndex = 1;
                state = 5;
                return MESSAGE_VALID;
            } else if (!strcmp(message, "GET_COMMAND")) {
                commandIndex = 2;
                state = 5;
                return MESSAGE_VALID;
            } else if (!strcmp(message, "GET_B64")) {
                state = 6;
                return MESSAGE_VALID;
            } else if (!strcmp(message, "DISCONNECT")) {
                state = 7;
                return MESSAGE_VALID;
            }
        }
    } else if (msg->direction == 1) {
        if (state == 2) {
            if (!strcmp(message, "CONNECT_OK")) {
                state = 3;
                return MESSAGE_VALID;
            }
        } else if (state == 7) {
            if (!strcmp(message, "DISCONNECT_OK")) {
                state = 1;
                return MESSAGE_VALID;
            }
        } else if (state == 4) {
            if (!strncmp(message, "VERSION ", 8)) {
                message += 8;
                if (*message > 48 && *message < 58) {
                    for (++message; *message != '\0'; ++message) {
                        if (*message < 48 || *message > 57) {
                            state = 1;
                            return MESSAGE_INVALID;
                        }
                    }
                    state = 3;
                    return MESSAGE_VALID;
                }
            }
        } else if (state == 5) {
            if (!strncmp(message, commands[commandIndex], commandsLength[commandIndex])) {
                message += commandsLength[commandIndex];
                if (*message == ' ') {
                    ++message;
                    char *spaceCheck;
                    while (data[*message]) {
                        ++message;
                    }
                    if (spaceCheck = *message == ' ') {
                        spaceCheck = message + 1;
                    }

                    if (spaceCheck) {
                        if (!strcmp(spaceCheck, commands[commandIndex])) {
                            state = 3;
                            return MESSAGE_VALID;
                        }
                    }
                    ++message;
                }
            }
        } else if (state == 6 && (!strncmp(message, "B64: ", 5))) {
            message += 5;
            char *begin = message;
            while (base64[*message]) {
                ++message;
            }
            char check = 0;
            while ((check < 2) && (message[check] == '=')) {
                ++check;
            }
            if ((message - begin + check) % 4 == 0 && message[check] == 0) {
                state = 3;
                return MESSAGE_VALID;
            }
            ++message;
        }
    }

    state = 1;

    return MESSAGE_INVALID;
}