#pragma once
#include <string>
#include <functional>
#include <unordered_map>

class WebServer {
public:
    using Handler = std::function<std::string(const std::string& queryString)>;

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

    std::string parsePath(const std::string& request) const;
    std::string parseQueryString(const std::string& request) const;
    std::string parseMethod(const std::string& request) const;
    std::string readFullFequest(int client);
};
