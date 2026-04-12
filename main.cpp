#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <bitset>
#include <iostream>
// #include <string>

#include "raw_sockets.hpp"

#define ERROR -1
#define NOERROR 0

#define FONT_RED "\x1B[31m"
#define FONT_NORMAL "\x1B[0m"

#define KERMIT_INIT_MARKER 0b01111110

#define TIMEOUT_MS 3000  // 3 seconds for timeout

#define BUFFER_SIZE 32  // represented by 5 bits
#define MINIMUM_MESSAGE_SIZE 14

using std::cerr;
using std::cout;

const char* messages[] = {
    "pang", "\tpeng", "\t\tping", "\t\t\tpong", "\t\t\t\tpung",
};

enum MessageType {
    ack = 0,
    nack = 1,
    visualize = 2,
    initialize = 3,
    data = 4,
    txt = 5,
    jpg = 6,
    mp4 = 7,
    walk_right = 10,
    walk_left = 11,
    walk_up = 12,
    walk_down = 13,
    error = 15,
    end_transmission = 16,
};
typedef enum MessageType MessageType;

// message that follows the kermit protocol
struct KermitMessage {
    struct {
        unsigned char init_marker;
        unsigned char size : 5;  // max size is 32B!!!
        unsigned char sequence : 6;
        MessageType type : 5;
    } header;
    unsigned char crc;
    char data[BUFFER_SIZE];

    int writeData(const char* data, int data_size) {
        if (!data) {
            cerr << "Null pointer to data in " << __func__ << "()\n";
            return ERROR;
        }

        if (data_size > 32 /*max value for 5b number is 32*/) {
            cerr << "Value for data_size is out of range for a 5b number in "
                 << __func__ << "()\n";
            return ERROR;
        }

        memcpy(this->data, data, data_size);

        return NOERROR;
    }

    int sendMessage(int socket) {
        unsigned long message_struct_size =
            sizeof(this->header) + this->header.size + sizeof(this->crc);

        // just so there's no problem with the size of the message
        char frame[message_struct_size + MINIMUM_MESSAGE_SIZE];

        memcpy(frame, this, message_struct_size);
        if (message_struct_size < MINIMUM_MESSAGE_SIZE) {
            message_struct_size += MINIMUM_MESSAGE_SIZE - message_struct_size;
        }

        // cout << "message_struct_size: " << message_struct_size << "\n";
        if (send(socket, (const void*)this, message_struct_size, 0) == -1) {
            cerr << "error when sending message on " << __func__ << "()\n";
            return ERROR;
        }

        return NOERROR;
    }

    int receiveMessage(int socket) {
        int ret = recv(socket, this, sizeof(*this), 0);
        if (ret == -1) {
            cerr << "error when reading message on " << __func__ << "()\n";
            return ERROR;
        }

        if (ret < (int)sizeof(this->header) + (int)sizeof(this->crc)) {
            cerr << "message too small on" << __func__ << "()\n";
            return ERROR;
        }

        if (this->header.init_marker != KERMIT_INIT_MARKER) {
            cerr << "unrecognized header on " << __func__ << "()\n";
            return ERROR;
        }

        return NOERROR;
    }

    void printHeader() {
        cerr << "init_marker: " << std::bitset<8>(this->header.init_marker)
             << "\n";
        cerr << "size: " << (int)this->header.size << "\n";
        cerr << "sequence: " << (int)this->header.sequence << "\n";
        cerr << "type: " << (int)this->header.type << "\n";
        cerr << "crc: " << (int)this->crc << "\n";
    }

    void printData() {
        // cerr << std::bitset<BUFFER_SIZE>(this->data) << "\n";
        cerr << "(" FONT_RED;
        cerr.write(this->data, this->header.size);
        cerr << FONT_NORMAL ")\n";
    }
};
typedef struct MessageHeader MessageHeader;

int runServer(int socket) {
    unsigned int count = 0;
    while (1) {
        const char* data = messages[count % 5];
        KermitMessage message = {
            .header =
                {
                    .init_marker = KERMIT_INIT_MARKER,
                    .size = (unsigned char)(strlen(data)),
                    .sequence = 0,
                    .type = MessageType::data,
                },
            .crc = 0,
            .data = {0},
        };

        if (message.writeData(data, strlen(data)) == -1) {
            cerr << "Message too big (for now)\n";
            exit(1);
        }

        if (message.sendMessage(socket) == -1) {
            int err = errno;
            cerr << "send() error on runServer(): " << strerror(err) << "("
                 << err << ")\n";
        }

        count++;
        if (count % 5 == 0) {
            break;
            // sleep(5);
        }
    }

    return 0;
}

int runClient(int socket) {
    while (1) {
        KermitMessage message;
        if (message.receiveMessage(socket) == ERROR) {
            continue;
        }

        if (message.header.init_marker != KERMIT_INIT_MARKER) {
            cerr << "got unrecognized init marker "
                 << std::bitset<8>(message.header.init_marker)
                 << " discarding...\n";
            continue;
        }

        cout << "received message: \n";
        message.printHeader();
        message.printData();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Usage: <program> --client|--user\n";
        exit(1);
    }

    cout << "Hello :)\n";
    int socket = cria_raw_socket((char*)"lo");
    // int socket = cria_raw_socket((char*)"enp3s0");
    if (socket == -1) {
        cerr << "Error when creating socket" << "\n";
        exit(1);
    }
    cerr << "Created socket with file descriptor: " << socket << "\n";

    // setting a timeout for the socket
    const int timeoutMillis = TIMEOUT_MS;
    struct timeval timeout = {
        .tv_sec = timeoutMillis / 1000,
        .tv_usec = (timeoutMillis % 1000) * 1000,
    };

    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout,
                   sizeof(timeout)) == -1) {
        cout << "error on setsockopt for timeout\n";
        exit(1);
    }

    if (strcmp(argv[1], "--server") == 0) {
        if (runServer(socket) == -1) {
            cout << "error when executing runServer()\n";
            exit(1);
        }
    } else if (strcmp(argv[1], "--client") == 0) {
        if (runClient(socket) == -1) {
            cout << "error when executing runClient()\n";
        }
    } else {
        cout << "unrecognized option";
    }

    close(socket);
    return 0;
}
