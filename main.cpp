#include <iostream>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include "read_server.h"
#include <sys/ioctl.h>
#include <chrono>
#include <sstream>

using namespace boost::asio;
using ip::tcp;

const int BUFF_SIZE = 1000;

std::string read_server(tcp::socket& server_sock);
unsigned long write_server(tcp::socket& server_sock, const char* buf, unsigned long count);
unsigned long write_server(tcp::socket& server_sock, const std::string& msg);
void receive_file(tcp::socket& server_sock, const std::string& filename);
void output_percentage(unsigned long current, unsigned long target);
void output_percentage2(unsigned long current, unsigned long target);

int main(int argc, char** argv)
{
    try
    {
        if(argc != 3)
        {
            throw std::invalid_argument("Usage: <file_client> <server_ip> <file_path>");
        }

        auto ip = argv[1];

        io_service io_service;

        tcp::socket sock(io_service);
        tcp::resolver resolver(io_service);
        //connect(sock, resolver.resolve(argv[1], "9000"));
        connect(sock, resolver.resolve(tcp::endpoint(ip::address_v4::from_string(ip), 9000)));

        receive_file(sock, argv[2]);

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

        auto temp_path = boost::filesystem::path(filepath);
        auto file_name = temp_path.filename().string();

        std::cout << "Receiving file: " << file_name << " ... " << std::endl;

        std::ofstream output_file(file_name);

        unsigned long bytes_received = 0;
        char input_buffer[BUFF_SIZE];

        //auto start = std::chrono::steady_clock::now();

        while(auto count = read_server(server_sock, input_buffer))
        {
            bytes_received += count;
            output_file.write(input_buffer, count);
            //output_percentage(bytes_received, file_size);
            //count = read_server(server_sock, input_buffer);
        }

        output_file.close();

        //auto end = std::chrono::steady_clock::now();
        //auto diff = end - start;
        //std::cout << std::endl;

        std::cout << "All done! - received: " << bytes_received << " bytes out of " << file_size << ". " << file_size - bytes_received << " remaining..." << std::endl;
        //std::cout << "Download took " << std::chrono::duration<double, std::milli>(diff).count() / 1000 << " sec " << std::endl;

    }else
    {
        std::cout << "File was not found on the server!" << std::endl;
    }
}

// Nedenstående funktioner er blot for at lave sjovt output...

void output_percentage(unsigned long current, unsigned long target)
{

    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    auto cols = w.ws_col;

    auto p = static_cast<int>(((float)current / target) * 100);

    std::stringstream out;

    if(p % 2 == 0)
    {
        auto nr_of_cols = static_cast<int>((cols * ( (float) p / 100 ))) - 2;

        for(int i = 0; i < nr_of_cols; ++i)
        {
            out << "x";
        }

        std::cout << "\r" << "[" << out.str() << "]";

    }
}

void output_percentage2(unsigned long current, unsigned long target)
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    auto cols = w.ws_col;
    auto nr_or_cols = static_cast<int>(0.3 * cols);

    auto p = static_cast<int>(((float)current / target) * 100);

    auto display_cols = static_cast<int>((nr_or_cols * ( (float) p / 100 )));

    std::stringstream out;

    for(int i = 0; i < display_cols; ++i)
    {
        out << "x";
    }

    for(int i = 0; i < (nr_or_cols - display_cols); ++i)
    {
        out << " ";
    }

    std::cout << "\r" << "[" << out.str() << "] - " << p << "%";
}
