/*
 * File:   haepoll.cpp
 * Author: okertanov@gmail.com
 * Created: 9 March 2013
 */

#include <algorithm>


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
    // Call assignment operator
    *this = other;
}

socket::socket(socket&& other)
    : socket_(-1)
{
    // Call move assignment operator
    *this = std::move(other);
}

socket::~socket()
{
    close();
}

std::vector<unsigned char> socket::read() const
{
    const ssize_t buff_size = 8192;
    std::vector<unsigned char> buffer(buff_size);
    ssize_t total_read = 0L;
    ssize_t read = 0L;

    do
    {
        read = ::read(socket_, &buffer[total_read], buff_size);

        if(read > 0)
        {
            total_read += read;

            if (read >= buff_size)
            {
                buffer.resize(buff_size);
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
    while (read > 0);

    if (read < 0)
    {
        if (!util::is_ignored_error(errno))
        {
            throw std::runtime_error(std::string("read() exception: ") + ::strerror(errno));
        }
    }

    buffer.resize(total_read);

    return buffer;
}

size_t socket::write(const unsigned char* buffer, size_t size) const
{
    size_t rc = 0;

    if (buffer && size > 0)
    {
        ssize_t write_total = size;
        ssize_t write_remaining = 0L;
        ssize_t written_total = 0L;
        ssize_t written = 0L;

        do
        {
            written = ::write(socket_, &buffer[write_remaining], write_total);

            if(written > 0)
            {
                written_total += written;

                if(written < write_total)
                {
                    write_total -= written;
                    write_remaining += written;
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        while (written > 0);

        if (written < 0)
        {
            if (!util::is_ignored_error(errno))
            {
                throw std::runtime_error(std::string("write() exception: ") + ::strerror(errno));
            }
        }

        rc = static_cast<size_t>(written_total);
    }

    return rc;
}

size_t socket::write(const std::vector<unsigned char>& buffer) const
{
    return write(&buffer[0], buffer.size());
}

size_t socket::write(const std::string& buffer) const
{
    return write(reinterpret_cast<const unsigned char*>(buffer.c_str()), buffer.length());
}

size_t socket::write_file(std::string filename) const
{
    size_t rc = 0;

    try
    {
        // RAII technique to acquire/release file handle
        scoped_resource<int, const char*, int> fd(::open, filename.c_str(), O_RDONLY, ::close);

        if (fd == -1)
        {
            throw std::runtime_error(std::string("open() exception: ") + ::strerror(errno));
        }

        struct stat sb;
        int st = ::fstat(fd, &sb);

        if (st == -1)
        {
            throw std::runtime_error(std::string("stat() exception: ") + ::strerror(errno));
        }

        off_t foffset = 0;
        size_t file_size = sb.st_size;
        ssize_t written = ::sendfile(socket_, fd, &foffset, file_size);

        if (written < 0)
        {
            if (!util::is_ignored_error(errno))
            {
                throw std::runtime_error(std::string("sendfile() exception: ") + ::strerror(errno));
            }
        }

        rc = static_cast<size_t>(written);
    }
    catch(std::exception& e)
    {
        throw;
    }

    return rc;
}

void socket::reuse() const
{
    if (socket_ >= 0)
    {
        int reuse = 1;
        int rc = ::setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
        if ( rc < 0 )
        {
            throw std::runtime_error(
                    std::string("socket::reuse()exception: Setting socket options SO_REUSEADDR failed: ") +
                    + ::strerror(errno));
        }
    }
}

void socket::nonblocking() const
{
    if (socket_ >= 0)
    {
        int flags = ::fcntl(socket_, F_GETFL, 0);

        if (flags == -1)
        {
            throw std::runtime_error(std::string("fcntl() exception: ") + ::strerror(errno));
        }

        flags |= O_NONBLOCK;
        int rc = ::fcntl(socket_, F_SETFL, flags);

        if (rc == -1)
        {
            throw std::runtime_error(std::string("fcntl() exception: ") + ::strerror(errno));
        }
    }
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

    int rc = ::inet_pton(AF_INET, addr.c_str(), &saddr.sin_addr);

    if (rc > 0)
    {
        sockaddr_in* paddr = ((sockaddr_in*)&sockaddr_);
        paddr->sin_family = saddr.sin_family;
        paddr->sin_addr = saddr.sin_addr;
        paddr->sin_port = htons(port);
    }
    else
    {
        if (rc == 0)
        {
            throw std::runtime_error(std::string("inet_pton() exception: ") + "Network address is not valid.");
        }
        else
        {
            throw std::runtime_error(std::string("inet_pton() exception: ") + ::strerror(errno));
        }
    }
}

address::address(const address& other)
    : sockaddr_({0})
{
    // Call assignment operator
    *this = other;
}

address::address(address&& other)
    : sockaddr_({0})
{
    // Call move assignment operator
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

    /*if (rc == 0)
    {
        throw std::runtime_error(::strerror(errno));
    }*/

    return std::string(rc == 0 ? "<unknow address>" : str);
}

socklen_t address::size() const
{
    return sizeof(sockaddr_);
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

mutex::mutex()
{
    int rc = ::pthread_mutex_init(&mutex_, 0);

    if (rc)
    {
        throw std::runtime_error("mutex::mutex() error.");
    }
}

mutex::~mutex()
{
    int rc;

    do
    {
        rc = ::pthread_mutex_destroy(&mutex_);
    }
    while (rc == EINTR);
}

bool mutex::try_lock()
{
    int rc = 0;
    do
    {
        rc = ::pthread_mutex_trylock(&mutex_);
    }
    while (rc == EINTR);

    if(rc && (rc!=EBUSY))
    {
        throw std::runtime_error("mutex::try_lock error.");
    }

    return !rc;
}

void mutex::lock()
{
    int rc;

    do
    {
        rc = ::pthread_mutex_lock(&mutex_);
    }
    while (rc == EINTR);

    if(rc)
    {
        throw std::runtime_error("mutex::lock error.");
    }
}

void mutex::unlock()
{
    int rc;

    do
    {
        rc = ::pthread_mutex_unlock(&mutex_);
    }
    while (rc == EINTR);

    if(rc)
    {
        throw std::runtime_error("mutex::unlock error.");
    }
}

epoll::epoll()
    : epollfd_(-1),
      epoll_events_(),
      wait_events_()
{
    epollfd_ = ::epoll_create(epoll_queue_size_hint);

    if (epollfd_ < 0 )
    {
        throw std::runtime_error(std::string("epoll_create() exception: ") + ::strerror(errno));
    }

    epoll_events_.reserve(epoll_queue_size_hint);
    wait_events_.reserve(epoll_queue_size_hint);
}

epoll::~epoll()
{
    if (epollfd_ >= 0)
    {
        ::close(epollfd_);
        epollfd_ = -1;
    }
}

const epoll& epoll::add_socket(const socket& sock)
{
    struct epoll_event ev = { 0 };
    ev.data.fd = sock;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLET | EPOLLERR | EPOLLHUP | EPOLLRDHUP;

    int rc = ::epoll_ctl(epollfd_, EPOLL_CTL_ADD, sock, &ev);

    if (rc != 0)
    {
        throw std::runtime_error(std::string("epoll_ctl() exception: ") + ::strerror(errno));
    }

    epoll_events_.push_back(ev);

    return *this;
}

const epoll& epoll::remove_socket(const socket& sock)
{
    struct epoll_event ev = { 0 };

    int rc = ::epoll_ctl(epollfd_, EPOLL_CTL_DEL, sock, &ev);

    if (rc != 0)
    {
        throw std::runtime_error(std::string("epoll_ctl() exception: ") + ::strerror(errno));
    }

    auto from = std::remove_if(epoll_events_.begin(), epoll_events_.end(),
    [&sock](const std::vector<struct epoll_event>::value_type& el)
    {
        return el.data.fd == sock;
    });
    epoll_events_.erase(from, epoll_events_.end());

    return *this;
}

bool epoll::wait(unsigned long ms)
{
    bool rc = false;

    if (epoll_events_.size())
    {
        struct epoll_event ev = { 0 };
        wait_events_.resize(epoll_events_.size());
        std::fill(wait_events_.begin(), wait_events_.end(), ev);

        int erc = ::epoll_wait(epollfd_, &wait_events_[0], wait_events_.size(), ms ? ms : -1);

        if (erc < 0)
        {
            throw std::runtime_error(std::string("epoll_wait() exception: ") + ::strerror(errno));
        }

        rc = !!erc;
    }

    return rc;
}

size_t epoll::dispatch(std::function<void(epoll_state, const socket&)> fn) const
{
    if (wait_events_.size())
    {
        std::for_each(wait_events_.begin(), wait_events_.end(),
        [&fn](const std::vector<struct epoll_event>::value_type& el)
        {
            epoll_state estate =
                el.events & EPOLLERR    ? epoll_state::EPOLL_ERROR   :
                el.events & EPOLLIN     ? epoll_state::EPOLL_READ    :
                el.events & EPOLLPRI    ? epoll_state::EPOLL_READ    :
                el.events & EPOLLRDNORM ? epoll_state::EPOLL_READ    :
                el.events & EPOLLRDBAND ? epoll_state::EPOLL_READ    :
                el.events & EPOLLOUT    ? epoll_state::EPOLL_WRITE   :
                el.events & EPOLLWRNORM ? epoll_state::EPOLL_WRITE   :
                el.events & EPOLLWRBAND ? epoll_state::EPOLL_WRITE   :
                el.events & EPOLLHUP    ? epoll_state::EPOLL_CLOSE   :
                                          epoll_state::EPOLL_UNKNOWN;

            fn(estate, socket(el.data.fd));
        });
    }

    return wait_events_.size();
}

server::server()
{
    ::signal(SIGPIPE, SIG_IGN);
}

server::~server()
{
}

const server& server::bind(std::string conn)
{
    conn_ctx_ = parse_connection_string(conn);

    bind_addr_ = std::move(address(conn_ctx_.family, conn_ctx_.addr, conn_ctx_.port));
    bind_sock_ = std::move(socket(conn_ctx_.family, conn_ctx_.type, conn_ctx_.protocol));

    bind_sock_.reuse();

    #ifdef BSD
    int nosigpipe = 1;
    int rc2 = ::setsockopt(bind_sock_, SOL_SOCKET, SO_NOSIGPIPE, &nosigpipe, sizeof(int));
    if ( rc2 < 0 )
    {
        throw std::runtime_error("Setting socket options SO_NOSIGPIPE failed.");
    }
    #endif // BSD


    int rc3 = ::bind(bind_sock_, bind_addr_, bind_addr_.size());
    if (rc3 != 0)
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
    return accept(bind_sock_);
}

std::pair<socket, address> server::accept(const socket& sock_in) const
{
    sockaddr saddr;
    socklen_t saddr_sz = sizeof(sockaddr);
    socket sock_out(::accept(sock_in, &saddr, &saddr_sz));
    address addr(saddr);

    return std::make_pair(std::move(sock_out), std::move(addr));
}

const server& server::accept_block(std::function<void(socket, address, std::mutex&)> fn) const
{
    std::atomic<bool> stop_cond(false);

    while ( !stop_cond )
    {
        std::shared_ptr<std::pair<socket, address>> pac =
            std::make_shared<std::pair<socket, address>>
            (
                accept()
            );

        fn(std::move(pac->first), std::move(pac->second), std::ref(iomutex_));
    }

    return *this;
}


const server& server::accept_async(std::function<void(socket, address, std::mutex&)> fn) const
{
    std::atomic<bool> stop_cond(false);

    while ( !stop_cond )
    {
        std::shared_ptr<std::pair<socket, address>> pac =
            std::make_shared<std::pair<socket, address>>
            (
                accept()
            );

        std::thread worker([]
            (std::function<void(socket, address, std::mutex&)> fn,
             std::shared_ptr<std::pair<socket, address>> pac, std::mutex& m)
        {
            fn(std::move(pac->first), std::move(pac->second), std::ref(m));
        }, std::ref(fn), pac, std::ref(iomutex_));

        if (worker.joinable())
        {
            worker.detach();
        }
    }

    return *this;
}

const server& server::accept_epoll(std::function<void(socket, address, std::mutex&)> fn) const
{
    std::atomic<bool> stop_cond(false);

    epoll ep;
    bind_sock_.nonblocking();
    ep.add_socket(bind_sock_);

    while ( !stop_cond )
    {
        std::cerr << "epoll wait: ";
        bool wt = ep.wait(1000);
        std::cerr << "ok " << wt << std::endl;

        ep.dispatch([&](epoll_state state, const socket& sock)
        {
            if (state == epoll_state::EPOLL_READ || state == epoll_state::EPOLL_WRITE)
            {
                std::cerr << "epoll accept for: ";
                std::shared_ptr<std::pair<socket, address>> pac =
                    std::make_shared<std::pair<socket, address>>
                    (
                        accept(sock)
                    );
                std::cerr << "ok" << std::endl;

                std::thread worker([]
                    (std::function<void(socket, address, std::mutex&)> fn,
                     std::shared_ptr<std::pair<socket, address>> pac, std::mutex& m)
                {
                    std::cerr << "epoll inside worker for: ";
                    fn(std::move(pac->first), std::move(pac->second), std::ref(m));
                    std::cerr << "ok" << std::endl;
                }, std::ref(fn), pac, std::ref(iomutex_));

                if (worker.joinable())
                {
                    worker.detach();
                }
            }
        });
    }

    return *this;
}

connection_info server::parse_connection_string(std::string conn) const
{
    connection_info connection;
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

namespace util
{
    static std::set<int> ignored_errors = {EPIPE, ECONNRESET, EAGAIN};

    bool is_ignored_error(int ec)
    {
        return (ignored_errors.find(ec) != ignored_errors.end());
    }
} /* namespace util */

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

