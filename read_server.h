#ifndef READ_SERVER_H
#define READ_SERVER_H

#include <boost/asio.hpp>

template<size_t N>
unsigned long read_server(boost::asio::ip::tcp::socket& sock, char(&buff)[N])
{
    using namespace boost::asio;
    boost::system::error_code ignored_error;

    auto n = boost::asio::read(sock, buffer(&buff[0], N), ignored_error);
    if(ignored_error)
    {
        std::cout << "Error! n is " << n << " " << ignored_error.message() << std::endl;
    }

    return (unsigned long)n;
}

#endif // READ_SERVER_H
