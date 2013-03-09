/*
 * File:   hetest.cpp
 * Author: okertanov@gmail.com
 * Created: 9 March 2013
 */

#include <iostream>
#include <string>
#include <thread>
#include <cstdlib>

#include "henet.h"

int main(int argc, char** argv)
{
    int rc = EXIT_SUCCESS;

    try
    {
        std::cout << "Starting..." << std::endl;

        std::cout << "Launching server thread..." << std::endl;
        std::thread server_thread([]()
        {
            try
            {
                ha::server server;
                server.bind("tcp:localhost:8080").listen();
                server.dispatch([](ha::socket s, ha::address a)
                {
                    std::cout << "dispatch: connected endpoint:" << a << std::endl;
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

