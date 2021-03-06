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
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <set>
#include <list>
#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include <future>
#include <chrono>
#include <utility>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <functional>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/sendfile.h>
#include <sys/epoll.h>

namespace ha
{

struct connection_info
{
    int family;     // AF_UNSPEC, AF_INET, AF_INET6, AF_UNIX/AF_LOCAL, AF_PACKET
    int type;       // SOCK_STREAM, SOCK_DGRAM, SOCK_RAW
    int protocol;   // IPPROTO_TCP, IPPROTO_UDP

    int port;       // 0 - 65535
    in_addr addr;   // INADDR_ANY
};

class socket
{
    public:
        socket();
        socket(int domain, int type, int protocol);
        socket(int socket);
        socket(const socket& other);
        socket(socket&& other);
        virtual ~socket();

        std::vector<unsigned char> read() const;
        size_t write(const unsigned char* buffer, size_t size) const;
        size_t write(const std::vector<unsigned char>& buffer) const;
        size_t write(const std::string& buffer) const;
        size_t write_file(std::string filename) const;

        void reuse() const;
        void nonblocking() const;

        void close();

        socket& operator=(const socket&);
        socket& operator=(socket&&);

        int operator=(int);

        operator const int() const;
        operator const int*() const;

    private:
        int socket_;
};

class address
{
    public:
        address();
        address(sockaddr saddr);
        address(short family, in_addr addr, unsigned short port);
        address(std::string addr, unsigned short port);
        address(const address& other);
        address(address&& other);
        virtual ~address();

        std::string str() const;
        socklen_t size() const;

        address& operator=(const address&);
        address& operator=(address&&);

        operator const sockaddr() const;
        operator const sockaddr*() const;

        operator const sockaddr_in() const;
        operator const sockaddr_in*() const;

    private:
        sockaddr sockaddr_;
};

class mutex
{
    public:
        mutex();
        ~mutex();

        bool try_lock();
        void lock();
        void unlock();

        // No copy, no move
        mutex(const mutex&) = delete;
        mutex(mutex&&) = delete;
        mutex& operator=(const mutex&) = delete;
        mutex& operator=(mutex&&) = delete;

    private:
        pthread_mutex_t mutex_;
};

template <typename T, typename... A>
class scoped_resource
{
    public:
        typedef std::function<T (A...)> initializer_t;
        typedef std::function<void(T)> finalizer_t;
        typedef T resource_t;
        scoped_resource(initializer_t finit, A... args, finalizer_t final)
            : finit_(finit), final_(final), resource_(finit_(args...)) { };
        ~scoped_resource() { final_(resource_); }

        template <typename Y>
        Y get() const { return static_cast<Y>(resource_); }
        T get() const { return resource_; }
        operator T() const { return get(); }

        // No copy, no move
        scoped_resource(const scoped_resource&) = delete;
        scoped_resource(scoped_resource&&) = delete;
        scoped_resource& operator=(const scoped_resource&) = delete;
        scoped_resource& operator=(scoped_resource&&) = delete;

    private:
        const initializer_t finit_;
        const finalizer_t final_;
        T resource_;
};

enum class epoll_state
{
    EPOLL_READ,
    EPOLL_WRITE,
    EPOLL_CLOSE,
    EPOLL_ERROR,
    EPOLL_UNKNOWN
};

class epoll
{
    public:
        epoll();
        virtual ~epoll();

        const epoll& add_socket(const socket& sock);
        const epoll& remove_socket(const socket& sock);

        bool wait(unsigned long ms = 0);
        size_t dispatch(std::function<void(epoll_state, const socket&)> fn) const;

    private:
        static const size_t epoll_queue_size_hint = 1024;

        int epollfd_;
        std::vector<struct epoll_event> epoll_events_;
        std::vector<struct epoll_event> wait_events_;
};

class server
{
    public:
        server();
        virtual ~server();

        // No copy, no move
        server(const server&) = delete;
        server(server&&) = delete;
        server& operator=(const server&) = delete;
        server& operator=(server&&) = delete;

        const server& bind(std::string conn);
        const server& listen() const;
        const server& accept_block(std::function<void(socket, address, std::mutex&)> fn) const;
        const server& accept_async(std::function<void(socket, address, std::mutex&)> fn) const;
        const server& accept_epoll(std::function<void(socket, address, std::mutex&)> fn) const;

    private:
        connection_info parse_connection_string(std::string conn) const;
        std::vector<std::string> split_connection_string(std::string conn) const;
        std::pair<socket, address> accept() const;
        std::pair<socket, address> accept(const socket& s) const;

    private:
        connection_info conn_ctx_;
        address bind_addr_;
        socket bind_sock_;
        mutable std::mutex iomutex_;
};

class client
{
    public:
        client();
        virtual ~client();

        // No copy, no move
        client(const server&) = delete;
        client(server&&) = delete;
        client& operator=(const client&) = delete;
        client& operator=(client&&) = delete;

        const client& connect(std::string conn);
        const client& disconnect();

    private:
        socket socket_;
};

namespace util
{
    bool is_ignored_error(int ec);
} /* namespace util */

} /* namespace ha */

std::ostream& operator<< (std::ostream &out, ha::socket &s);
std::istream& operator>> (std::istream &in, ha::socket &s);

std::ostream& operator<< (std::ostream &out, ha::address &a);
std::istream& operator>> (std::istream &in, ha::address &a);

#endif  /* _HENET_H_ */

