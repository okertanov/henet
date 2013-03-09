/*
 * File:   haepoll.h
 * Author: okertanov@gmail.com
 * Created: 9 March 2013
 */

#ifndef _HENET_H_
#define _HENET_H_

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <functional>
#include <regex>
#include <atomic>
#include <exception>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

namespace ha
{

struct connection
{
    int domain;     // PF_UNIX, PF_INET
    int family;     // AF_UNSPEC, AF_INET, AF_INET6, AF_UNIX/AF_LOCAL, AF_PACKET
    int type;       // SOCK_STREAM, SOCK_DGRAM, SOCK_RAW
    int protocol;   // IPPROTO_TCP, IPPROTO_UDP

    int port;       // 0 - 65535
    in_addr addr;   // INADDR_ANY
};

class socket
{
    public:
        explicit socket();
        socket(int domain, int type, int protocol);
        socket(int socket);
        ~socket();

        int operator=(int);
        operator int();
        operator int*();

    private:
        int socket_;
};

class address
{
    public:
        explicit address();
        address(short family, unsigned short port, in_addr addr);
        ~address();

        operator sockaddr();
        operator sockaddr*();

        operator sockaddr_in();
        operator sockaddr_in*();

        size_t size();

    private:
        sockaddr sockaddr_;
};

class server
{
    public:
        explicit server();
        virtual ~server();

        server& bind(std::string conn);
        server& listen();
        server& dispatch(std::function<void(socket, address)> fn);

    private:
        connection parse_connection_string(std::string conn);
        std::vector<std::string> split_connection_string(std::string conn);

    private:
        connection conn_ctx_;
        address bind_addr_;
        socket bind_sock_;
};

} /* namespace ha */

std::ostream& operator<< (std::ostream &out, ha::socket &s);
std::istream& operator>> (std::istream &in, ha::socket &s);

std::ostream& operator<< (std::ostream &out, ha::address &a);
std::istream& operator>> (std::istream &in, ha::address &a);

#endif  /* _HENET_H_ */

