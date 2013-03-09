/*
 * File:   haepoll.cpp
 * Author: okertanov@gmail.com
 * Created: 9 March 2013
 */

#include "henet.h"

namespace ha
{

socket::socket()
{
}

socket::socket(int socket)
{
    socket_ = socket;
}

socket::socket(int domain, int type, int protocol)
{
    socket_ = ::socket(domain, type, protocol);
    if (socket_ < 0)
    {
        throw std::runtime_error("Invalid socket.");
    }
}

socket::~socket()
{
    if (socket_ >= 0)
    {
        ::close(socket_);
    }
}

int socket::operator=(int socket)
{
    socket_ = socket;

    return socket_;
}

socket::operator int()
{
    return socket_;
}

socket::operator int*()
{
    return &socket_;
}

address::address()
{
}

address::address(short family, unsigned short port, in_addr addr)
{
    sockaddr_in* saddr = ((sockaddr_in*)&sockaddr_);
    saddr->sin_family = family;
    saddr->sin_port = htons(port);
    saddr->sin_addr = addr;
}

address::~address()
{
    ::memset(&sockaddr_, 0, sizeof(sockaddr));
}

address::operator sockaddr()
{
    return sockaddr_;
}

address::operator sockaddr*()
{
    return &sockaddr_;
}

address::operator sockaddr_in()
{
    return (*((sockaddr_in*)&sockaddr_));
}

address::operator sockaddr_in*()
{
    return ((sockaddr_in*)&sockaddr_);
}

size_t address::size()
{
    return sizeof(sockaddr_);
}

server::server()
{
}

server::~server()
{
}

server& server::bind(std::string conn)
{
    conn_ctx_ = parse_connection_string(conn);

    bind_addr_ = address(conn_ctx_.family, conn_ctx_.port, conn_ctx_.addr);
    bind_sock_ = socket(conn_ctx_.domain, conn_ctx_.type, conn_ctx_.protocol);

    int one = 1;
    int rc = ::setsockopt(bind_sock_, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    if ( rc < 0 )
    {
        throw std::runtime_error("Setting socket options failed.");
    }

    rc = ::bind(bind_sock_, bind_addr_, bind_addr_.size());
    if (rc != 0)
    {
        throw std::runtime_error("Binding socket failed.");
    }

    return *this;
}

server& server::listen()
{
    int rc = ::listen(bind_sock_, SOMAXCONN);

    if (rc != 0)
    {
        throw std::runtime_error("Listening socket failed.");
    }

    return *this;
}

server& server::dispatch(std::function<void(socket, address)> fn)
{
    std::atomic<bool> stop_cond(false);

    while ( !stop_cond )
    {
        address client_address;
        size_t client_address_sz = client_address.size();
        socket client_sock = accept(bind_sock_, (sockaddr*)&client_address, &client_address_sz);
    }

    return *this;
}

connection server::parse_connection_string(std::string conn)
{
    connection connection;
    connection.domain = PF_INET;
    connection.family = AF_INET;
    connection.type = SOCK_STREAM;
    connection.protocol = IPPROTO_TCP;
    connection.port = 0;
    connection.addr.s_addr = INADDR_ANY;

    std::vector<std::string> conn_parts  = split_connection_string(conn);

    if (conn_parts.size() <= 0 || conn_parts.size() > 3)
    {
        throw std::runtime_error("Invalid connection string.");
    }

    std::string protocol = conn_parts[0];
    std::string host = conn_parts[1];
    std::string port = conn_parts[2];

    if (protocol == "tcp")
    {
        connection.type = SOCK_STREAM;
        connection.protocol = IPPROTO_TCP;
    }
    else if (protocol == "udp")
    {
        connection.type = SOCK_DGRAM;
        connection.protocol = IPPROTO_UDP;
    }
    else
    {
        throw std::runtime_error("Invalid protocol parameter.");
    }

    if (host.length() > 0)
    {
        addrinfo *infos;
        addrinfo *info;
        int rc = getaddrinfo(host.c_str(), NULL, NULL, &infos);

        if (rc != 0)
        {
            throw std::runtime_error("Invalid host parameter.");
        }

        for (info = infos; info != NULL; info = info->ai_next)
        {
            sockaddr_in* in = (sockaddr_in*)info->ai_addr;
            connection.addr = in->sin_addr;
            break; // Yes, break at 1st.
        }
    }

    std::istringstream strstream(port);
    strstream >> connection.port;

    if (connection.port <= 0)
    {
        throw std::runtime_error("Invalid port parameter.");
    }

    return connection;
}

std::vector<std::string> server::split_connection_string(std::string conn)
{
    const std::string delimiter = ":";
    std::vector<std::string> parts;
    unsigned pos1 = 0, pos2 = 0;

    while(pos1 != std::string::npos && pos2 != std::string::npos)
    {
        if ( pos1 !=  std::string::npos)
        {
            auto s = conn.begin() + pos1;
            auto e = conn.end();

            pos2 = conn.find_first_of(delimiter, pos1);
            if ( pos2 != std::string::npos)
            {
                e = conn.begin() + pos2;
            }

            parts.push_back(std::string(s, e));
            pos1 = pos2 + 1;
        }
    }

    return parts;
}

} /* namespace ha */

std::ostream& operator<< (std::ostream &out, ha::socket &s)
{
    return out;
}

std::istream& operator>> (std::istream &in, ha::socket &s)
{
    return in;
}

std::ostream& operator<< (std::ostream &out, ha::address &a)
{
    return out;
}

std::istream& operator>> (std::istream &in, ha::address &a)
{
    return in;
}

