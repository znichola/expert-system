#pragma once
#include <string>
#include <functional>
#include <unordered_map>

class WebServer {
public:
    using Handler = std::function<std::string(const std::string& queryString)>;

    WebServer(const InputOptions &opts);
    ~WebServer();

    // Register routes
    void registerGetRoutes();

    // Start the server (blocking)
    void start();
    void stop();

private:
    int server_fd;
    const InputOptions opts;

    enum class Status { OK=200, NOT_FOUND=404, SERVER_ERROR=500 };
    
    // Route maps for GET
    std::unordered_map<std::string, Handler> get_routes;

    std::string parsePath(const std::string& request) const;
    std::string parseQueryString(const std::string& request) const;
    std::string parseMethod(const std::string& request) const;
    std::string readFullFequest(int client);
    std::string constructHTMLResponse(Status status, const std::string& body) const;
 
    class Route {
        std::string path;
        Handler handler;
    };
};
