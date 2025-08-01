#include <iostream>
#include <string>
#include <memory>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/filesystem.hpp>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace fs = boost::filesystem;

class HttpServer {
public:
    HttpServer(asio::io_context& io_context, unsigned short port, const std::string& doc_root)
        : acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
          socket_(io_context),
          doc_root_(doc_root) {
        acceptConnection();
        std::cout << "Server started on port " << port << std::endl;
        std::cout << "Document root: " << doc_root_ << std::endl;
    }

private:
    void acceptConnection() {
        acceptor_.async_accept(socket_,
            [this](boost::system::error_code ec) {
                if (!ec) {
                    std::make_shared<Session>(std::move(socket_), doc_root_)->start();
                }
                acceptConnection();
            });
    }

    class Session : public std::enable_shared_from_this<Session> {
    public:
        Session(asio::ip::tcp::socket socket, const std::string& doc_root)
            : socket_(std::move(socket)), doc_root_(doc_root) {}

        void start() {
            readRequest();
        }

    private:
        void readRequest() {
            auto self = shared_from_this();

            http::async_read(socket_, buffer_, request_,
                [self](beast::error_code ec, std::size_t bytes_transferred) {
                    boost::ignore_unused(bytes_transferred);
                    if (!ec)
                        self->processRequest();
                });
        }

        void processRequest() {
            response_.version(request_.version());
            response_.keep_alive(false);

            switch (request_.method()) {
                case http::verb::get:
                    handleGet();
                    break;
                default:
                    response_.result(http::status::bad_request);
                    response_.set(http::field::content_type, "text/plain");
                    beast::ostream(response_.body()) << "Invalid request method.\n";
            }

            writeResponse();
        }

        void handleGet() {
            std::string target = request_.target().to_string();
            if (target == "/")
                target = "/index.html";

            std::string path = doc_root_ + target;
            if (!fs::exists(path)) {
                response_.result(http::status::not_found);
                response_.set(http::field::content_type, "text/plain");
                beast::ostream(response_.body()) << "File not found: " << target << "\n";
                return;
            }

            try {
                std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
                if (!file) {
                    response_.result(http::status::internal_server_error);
                    response_.set(http::field::content_type, "text/plain");
                    beast::ostream(response_.body()) << "Error opening file: " << target << "\n";
                    return;
                }

                response_.result(http::status::ok);
                setContentType(path);

                // Read the file into the response body
                std::string body;
                char buffer[4096];
                while (file.read(buffer, sizeof(buffer))) {
                    body.append(buffer, file.gcount());
                }
                body.append(buffer, file.gcount());
                beast::ostream(response_.body()) << body;
            } catch (const std::exception& e) {
                response_.result(http::status::internal_server_error);
                response_.set(http::field::content_type, "text/plain");
                beast::ostream(response_.body()) << "Exception: " << e.what() << "\n";
            }
        }

        void setContentType(const std::string& path) {
            auto extension = fs::extension(path);
            if (extension == ".html" || extension == ".htm")
                response_.set(http::field::content_type, "text/html");
            else if (extension == ".css")
                response_.set(http::field::content_type, "text/css");
            else if (extension == ".js")
                response_.set(http::field::content_type, "application/javascript");
            else if (extension == ".json")
                response_.set(http::field::content_type, "application/json");
            else if (extension == ".png")
                response_.set(http::field::content_type, "image/png");
            else if (extension == ".jpg" || extension == ".jpeg")
                response_.set(http::field::content_type, "image/jpeg");
            else if (extension == ".gif")
                response_.set(http::field::content_type, "image/gif");
            else
                response_.set(http::field::content_type, "application/octet-stream");
        }

        void writeResponse() {
            auto self = shared_from_this();

            response_.set(http::field::content_length, response_.body().size());

            http::async_write(socket_, response_,
                [self](beast::error_code ec, std::size_t) {
                    self->socket_.shutdown(asio::ip::tcp::socket::shutdown_send, ec);
                });
        }

        asio::ip::tcp::socket socket_;
        beast::flat_buffer buffer_{8192};
        http::request<http::string_body> request_;
        http::response<http::dynamic_body> response_;
        std::string doc_root_;
    };

    asio::ip::tcp::acceptor acceptor_;
    asio::ip::tcp::socket socket_;
    std::string doc_root_;
};

int main(int argc, char* argv[]) {
    try {
        // Check command line arguments
        if (argc != 3) {
            std::cerr << "Usage: http_server <port> <doc_root>\n";
            std::cerr << "Example: http_server 8080 ./www\n";
            return 1;
        }

        auto const port = static_cast<unsigned short>(std::atoi(argv[1]));
        std::string doc_root = argv[2];

        // Create and verify the document root directory
        if (!fs::exists(doc_root)) {
            std::cerr << "Document root directory does not exist: " << doc_root << std::endl;
            return 1;
        }

        if (!fs::is_directory(doc_root)) {
            std::cerr << "Document root is not a directory: " << doc_root << std::endl;
            return 1;
        }

        // The io_context is required for all I/O
        asio::io_context io_context;

        // Create and launch the server
        HttpServer server(io_context, port, doc_root);

        // Run the I/O service
        io_context.run();

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}