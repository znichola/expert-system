#pragma once
#include <string>
#include <functional>
#include <unordered_map>

class WebServer {
public:
    using Handler = std::function<std::string(const std::string& body)>;

    WebServer(int port = 8080);
    ~WebServer();

    // Register routes
    void get(const std::string& path, Handler handler);

    // Start the server (blocking)
    void start();
    void stop();

private:
    int server_fd;

    // Route maps for GET
    std::unordered_map<std::string, Handler> get_routes;

    std::string parse_path(const std::string& request);
    std::string parse_method(const std::string& request);
    std::string parse_body(const std::string& request);
    std::string read_request(int client);
};
