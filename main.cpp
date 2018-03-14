#include <iostream>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include "read_server.h"

using namespace boost::asio;
using ip::tcp;

std::string read_server(tcp::socket& server_sock);
std::vector<char> read_server(tcp::socket& server_sock, unsigned long count);
unsigned long write_server(tcp::socket& server_sock, const char* buf, unsigned long count);
unsigned long write_server(tcp::socket& server_sock, const std::string& msg);
void receive_file(tcp::socket& server_sock, const std::string& filename);

int main(int argc, char** argv)
{
    try
    {
//        if(argc != 3)
//        {
//            throw std::invalid_argument("Usage: <file_client> <server_ip> <file_path>");
//        }

        io_service io_service;

        tcp::socket sock(io_service);
        tcp::resolver resolver(io_service);
        connect(sock, resolver.resolve("192.168.1.187", "9000"));

        receive_file(sock, "C:/Users/jonas/Desktop/C_Sharp_Succinctly.pdf");

        sock.shutdown(sock.shutdown_both);

    }catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

std::string read_server(tcp::socket& server_sock)
{
    streambuf read_buffer;
    read_until(server_sock, read_buffer, '\0');
    return { buffers_begin(read_buffer.data()), buffers_end(read_buffer.data())-1 };
}

std::vector<char> read_server(tcp::socket& server_sock, unsigned long count)
{
    streambuf read_buffer;
    boost::system::error_code ignored_error;
    auto n = boost::asio::read(server_sock, read_buffer, transfer_exactly(count), ignored_error);
    return { buffers_begin(read_buffer.data()), buffers_begin(read_buffer.data()) + n };
}

unsigned long write_server(tcp::socket& server_sock, const char* buf, unsigned long count)
{
    auto write_buf = buffer(buf, count);
    return static_cast<unsigned long>(write(server_sock, write_buf));
}

unsigned long write_server(tcp::socket& server_sock, const std::string& msg)
{
    return write_server(server_sock, msg.c_str(), msg.length() + 1 );
}

void receive_file(tcp::socket& server_sock, const std::string& filepath)
{

    write_server(server_sock, filepath);

    auto file_size = std::atol(read_server(server_sock).c_str());

    if(file_size)
    {
        std::cout << "Receiving... " << std::endl;

        auto temp_path = boost::filesystem::path(filepath);
        auto file_name = temp_path.filename().string();

        std::ofstream output_file(file_name);

        unsigned long bytes_received = 0;
        char input_buffer[1000];

        while(auto count = read_server(server_sock, input_buffer))
        {
            bytes_received += count;
            output_file.write(input_buffer, count);
        }

        output_file.close();

        std::cout << "All done! - Received: " << bytes_received << " bytes out of " << file_size << std::endl;

    }else
    {
        std::cout << "File was not found on the server!" << std::endl;
    }
}
