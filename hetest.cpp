/*
 * File:   hetest.cpp
 * Author: okertanov@gmail.com
 * Created: 9 March 2013
 */

#include <thread>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <functional>

#include "henet.h"

int main(int argc, char** argv)
{
    int rc = EXIT_SUCCESS;

    try
    {
        std::cout << "Starting..." << std::endl;

        // Some primitive unit testing
        volatile char* a = 0;
        {
            // RAII technique to acquire/release memory block
            ha::scoped_resource<void*, size_t> mem(::malloc, 1, ::free);
            memset(mem, 65, 1);
            a = mem.get<char*>();
            std::cout << "Hello, " << "[" << *a << "]" << std::endl;
        }
        /// @warning Unsafe!
        std::cout << "Hello again, " << "[" << *a << "]" << std::endl;

        {
            ha::scoped_resource<int, const char*, int> fd(::open, argv[0], O_RDONLY, ::close);
            assert(fd != -1);
            int raw_fd1 = fd.get();
            assert(raw_fd1 != -1);

            int raw_fd2 = fd.operator  int();
            assert(raw_fd2 != -1);
        }

        {
            ha::mutex mutex;
            ha::scoped_resource<ha::mutex*, ha::mutex*> scoped_lock(
                [](ha::mutex* m) -> ha::mutex*
                {
                    return m->lock(), m;
                },
                &mutex,
                [](ha::mutex* m) -> void
                {
                    m->unlock();
                }
            );
        }

        std::cout << "Launching server thread..." << std::endl;
        std::thread server_thread([&]()
        {
            try
            {
                ha::server server;
                server.bind("tcp::8080").listen();
                server.dispatch_async([&](ha::socket s, ha::address a, std::mutex& m)
                {
                    // Notify connected
                    //std::cout << "Client connected. Endpoint: " << a << ", socket: " << s << std::endl;

                    // Read request
                    std::vector<unsigned char> raw_request = s.read();
                    const std::string str_request(raw_request.begin(), raw_request.end());

                    //std::cout << "Client request: " << std::endl << str_request << std::endl;

                    // Send reply
                    const char crlf[] = "\x0D\x0A";
                    std::stringstream stream;
                    stream << "HTTP/1.1 200 OK"                         << crlf
                           << "Server: henet"                           << crlf
                           << "Content-type: application/octet-stream"  << crlf
                           << "Content-Transfer-Encoding: 8bit"         << crlf
                           //<< "Content-Length: " << 0                 << crlf
                           << "Connection: close"                       << crlf
                           << crlf;

                    std::unique_lock<std::mutex> lock(m);
                    s.write(stream.str());
                    s.write_file(argv[0]);

                    //std::cout << "Reply sent: " << std::endl << str_reply << std::endl;
                });
            }
            catch(std::exception& e)
            {
                std::cerr << "Exception: " << e.what() << std::endl;

                throw;
            }
        });
        std::cout << "Server thread OK." << std::endl;

        std::cout << "Joining server thread..." << std::endl;
        server_thread.join();
        std::cout << "Server thread complete." << std::endl;

        std::cout << "Stopped." << std::endl;
    }
    catch(std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;

        rc = EXIT_FAILURE;
    }

    return rc;
}

