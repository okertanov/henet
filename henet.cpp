/*
 * File:   haepoll.cpp
 * Author: okertanov@gmail.com
 * Created: 9 March 2013
 */

#include "henet.h"

namespace ha
{

socket::socket()
    : socket_(-1)
{
}

socket::socket(int socket)
    : socket_(socket)
{
}

socket::socket(int domain, int type, int protocol)
    : socket_(::socket(domain, type, protocol))
{
    if (socket_ < 0)
    {
        throw std::runtime_error("Invalid socket.");
    }
}

socket::socket(const socket& other)
    : socket_(-1)
{
    // Call assigment operator
    *this = other;
}

socket::socket(socket&& other)
    : socket_(-1)
{
    // Call move assigment operator
    *this = std::move(other);
}

socket::~socket()
{
    close();
}

void socket::close()
{
    if (socket_ >= 0)
    {
        ::close(socket_);
        socket_ = -1;
    }
}

socket& socket::operator=(const socket& other)
{
    if (this != &other)
    {
        socket_ = other.socket_;
    }

    return *this;
}

socket& socket::operator=(socket&& other)
{
    if (this != &other)
    {
        socket_ = other.socket_;
        other.socket_ = -1;
    }

    return *this;
}

int socket::operator=(int socket)
{
    socket_ = socket;

    return socket_;
}

socket::operator const int() const
{
    return socket_;
}

socket::operator const int*() const
{
    return &socket_;
}

address::address()
    : sockaddr_({0})
{
}

address::address(sockaddr saddr)
    : sockaddr_(saddr)
{
}

address::address(short family, in_addr addr, unsigned short port)
    : sockaddr_({0})
{
    sockaddr_in* paddr = ((sockaddr_in*)&sockaddr_);
    paddr->sin_family = family;
    paddr->sin_addr = addr;
    paddr->sin_port = htons(port);
}

address::address(std::string addr, unsigned short port)
    : sockaddr_({0})
{
    sockaddr_in saddr;
    ::inet_pton(AF_INET, addr.c_str(), &saddr.sin_addr);

    sockaddr_in* paddr = ((sockaddr_in*)&sockaddr_);
    paddr->sin_family = saddr.sin_family;
    paddr->sin_addr = saddr.sin_addr;
    paddr->sin_port = htons(port);
}

address::address(const address& other)
    : sockaddr_({0})
{
    // Call assigment operator
    *this = other;
}

address::address(address&& other)
    : sockaddr_({0})
{
    // Call move assigment operator
    *this = std::move(other);
}

address::~address()
{
    ::memset(&sockaddr_, 0, sizeof(sockaddr));
}

address& address::operator=(const address& other)
{
    if (this != &other)
    {
        sockaddr_ = other.sockaddr_;
    }

    return *this;
}

address& address::operator=(address&& other)
{
    if (this != &other)
    {
        sockaddr_ = other.sockaddr_;
        ::memset(&other.sockaddr_, 0, sizeof(sockaddr));
    }

    return *this;
}

std::string address::str() const
{
    const sockaddr_in* sa = operator const sockaddr_in*();
    char str[INET_ADDRSTRLEN] = {0};
    const char* rc = ::inet_ntop(sa->sin_family, &sa->sin_addr.s_addr, str, INET_ADDRSTRLEN);

    if (rc == 0)
    {
        throw std::runtime_error(::strerror(errno));
    }

    return std::string(str);
}

address::operator const sockaddr() const
{
    return sockaddr_;
}

address::operator const sockaddr*() const
{
    return &sockaddr_;
}

address::operator const sockaddr_in() const
{
    return (*((sockaddr_in*)&sockaddr_));
}

address::operator const sockaddr_in*() const
{
    return ((sockaddr_in*)&sockaddr_);
}

size_t address::size() const
{
    return sizeof(sockaddr_);
}

server::server()
{
}

server::~server()
{
}

const server& server::bind(std::string conn)
{
    conn_ctx_ = parse_connection_string(conn);

    bind_addr_ = std::move(address(conn_ctx_.family, conn_ctx_.addr, conn_ctx_.port));
    bind_sock_ = std::move(socket(conn_ctx_.family, conn_ctx_.type, conn_ctx_.protocol));

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

const server& server::listen() const
{
    int rc = ::listen(bind_sock_, SOMAXCONN);

    if (rc != 0)
    {
        throw std::runtime_error("Listening socket failed.");
    }

    return *this;
}

std::pair<socket, address> server::accept() const
{
    sockaddr saddr;
    size_t saddr_sz = sizeof(sockaddr);
    socket sock(::accept(bind_sock_, &saddr, &saddr_sz));
    address addr(saddr);

    return std::make_pair(std::move(sock), std::move(addr));
}

const server& server::dispatch(std::function<void(socket, address)> fn) const
{
    return dispatch_impl(fn, false);
}

const server& server::dispatch_async(std::function<void(socket, address)> fn) const
{
    return dispatch_impl(fn, true);
}

const server& server::dispatch_impl(std::function<void(socket, address)> fn, bool async) const
{
    std::atomic<bool> stop_cond(false);

    while ( !stop_cond )
    {
        auto ac = accept();

        if (!async)
        {
            fn(std::move(ac.first), std::move(ac.second));
        }
        else
        {
            std::thread worker([&]()
            {
                fn(std::move(ac.first), std::move(ac.second));
            });
        }
    }

    return *this;
}

connection server::parse_connection_string(std::string conn) const
{
    connection connection;
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

std::vector<std::string> server::split_connection_string(std::string conn) const
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
    out << (int)s;
    return out;
}

std::istream& operator>> (std::istream &in, ha::socket &s)
{
    return in;
}

std::ostream& operator<< (std::ostream &out, ha::address &a)
{
    out << a.str();
    return out;
}

std::istream& operator>> (std::istream &in, ha::address &a)
{
    return in;
}

