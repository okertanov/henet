/*
 * File:   hetest.cpp
 * Author: okertanov@gmail.com
 * Created: 9 March 2013
 */

#include "henet.h"

int main(int argc, char** argv)
{
    int rc = EXIT_SUCCESS;

    try
    {
        std::cout << "Starting..." << std::endl;

        std::cout << "Launching server thread..." << std::endl;

        std::thread server_thread([&]()
        {
            try
            {
                ha::server server;
                server.bind("tcp::8080").listen();
                server.accept_epoll([&](ha::socket s, ha::address a, std::mutex& m)
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

