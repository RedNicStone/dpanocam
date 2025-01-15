
#ifndef DPANOCAM_ABSTRACT_IP_STREAM_H
#define DPANOCAM_ABSTRACT_IP_STREAM_H

#include <utility>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <cstdint>


class abstract_stream {
public:
    virtual ~abstract_stream() = default;
    virtual std::size_t read_some(const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec) = 0;
    virtual void async_read_some(const boost::asio::mutable_buffer& buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler) = 0;
    virtual void write(const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec) = 0;
};


class pain_stream : public abstract_stream {
public:
    explicit pain_stream(boost::asio::ip::tcp::socket socket) : socket(std::move(socket)) {}

    std::size_t read_some(const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec) override {
        return socket.read_some(buffer, ec);
    }

    void async_read_some(const boost::asio::mutable_buffer& buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler) override {
        socket.async_read_some(buffer, handler);
    }

    void write(const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec) override {
        boost::asio::write(socket, buffer, ec);
    }

    boost::asio::ip::tcp::socket& get_handle() {
        return socket;
    }

private:
    boost::asio::ip::tcp::socket socket;
};


class tls_stream : public abstract_stream {
public:
    explicit tls_stream(boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_stream)
            : ssl_stream(std::move(ssl_stream)) {}

    std::size_t read_some(const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec) override {
        return ssl_stream.read_some(buffer, ec);
    }

    void async_read_some(const boost::asio::mutable_buffer& buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler) override {
        ssl_stream.async_read_some(buffer, handler);
    }

    void write(const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec) override {
        boost::asio::write(ssl_stream, buffer, ec);
    }

    boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& get_handle() {
        return ssl_stream;
    }

private:
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_stream;
};


class serial_stream : public abstract_stream {
public:
    explicit serial_stream(boost::asio::serial_port& serial)
            : serial(serial) {}

    std::size_t read_some(const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec) override {
        return serial.read_some(buffer, ec);
    }

    void async_read_some(const boost::asio::mutable_buffer& buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler) override {
        serial.async_read_some(buffer, handler);
    }

    void write(const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec) override {
        boost::asio::write(serial, buffer, ec);
    }

    boost::asio::serial_port& get_handle() {
        return serial;
    }

private:
    boost::asio::serial_port& serial;
};



#endif //DPANOCAM_ABSTRACT_IP_STREAM_H
