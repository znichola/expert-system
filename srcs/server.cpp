#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <csignal>

#include <graphviz/gvc.h>

#include "server.hpp"

WebServer* g_server = nullptr;
bool running = true;

void handleSigint(int) {
    std::cout << "\nSigint...\n";
    if (g_server) g_server->stop();
    exit(0);
}

WebServer::WebServer(int port) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); exit(1); }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt"); exit(1); }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {perror("bind"); exit(1); }
    if (listen(server_fd, 10) < 0) { perror("listen"); exit(1); }

    std::cout << "Server listening on port " << port << "...\n";
}

WebServer::~WebServer() {
    if (server_fd != -1 && close(server_fd) != 0) perror("destructor close");
}

void WebServer::get(const std::string& path, Handler handler) {
    get_routes[path] = handler;
}

std::string WebServer::parsePath(const std::string& request) const {
    size_t start = request.find(' ') + 1;
    size_t end = request.find_first_of(" ?", start);
    return request.substr(start, end - start);
}

std::string WebServer::parseQueryString(const std::string &request) const {
    size_t endFirstLine = request.find("\r\n");
    size_t start = request.find('?');
    if (start == std::string::npos || start > endFirstLine)
        return "";
    start += 1;
    size_t end = request.find(" ", start);
    return request.substr(start, end - start);
}

std::string WebServer::parseMethod(const std::string& request) const {
    size_t end = request.find(" ");
    return request.substr(0, end);
}

std::string WebServer::constructHTMLResponse(Status status, const std::string& body="") const {
    auto defaultBody = [status]() -> std::string {
        switch (status) {
            case Status::OK:
                return "<h1>200 OK</h1>";
            case Status::NOT_FOUND:
                return "<h1>404 Not Found</h1>";
            case Status::SERVER_ERROR:
                return "<h1>500 Internal Server Error</h1>";
            default:
                return "<h1>Unknown Status<h1>";
        }
    };

    std::string htmlBody = body.empty() ? defaultBody() : body;

    std::ostringstream fullHtml;
    fullHtml << "<!DOCTYPE html>\n"
             << "<html lang=\"en\">\n"
             << "<head>\n"
             << "  <meta charset=\"UTF-8\">\n"
             << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
             << "  <title>WebServer Response</title>\n"
             << "  <style>\n"
             << "    body { font-family: sans-serif; margin: 2em; text-align: center; }\n"
             << "    h1 { color: #333; }\n"
             << "  </style>\n"
             << "</head>\n"
             << "<body>\n"
             << htmlBody << "\n"
             << "</body>\n"
             << "</html>";

    std::string responseBody = fullHtml.str();
    
    std::ostringstream response;
        response << "HTTP/1.1 " << static_cast<std::underlying_type<Status>::type>(status) << "\r\n"
                << "Content-Type: text/html; charset=UTF-8\r\n"
                << "Connection : close\r\n"
                << "Content-Length: " << responseBody.size() << "\r\n"
                << "\r\n"
                << responseBody;
    return response.str();
}

void WebServer::start() {
    g_server = this;
    std::signal(SIGINT, handleSigint);
    while (running) {
        int client = accept(server_fd, nullptr, nullptr);
        if (client < 0) {
            if (!running) perror("accept"); // for force shutdown 
            continue; 
        }

            std::string request = readFullFequest(client);
            if (request.length() > 0) {

                std::string method = parseMethod(request);
                std::string path = parsePath(request);
                std::string queryString = parseQueryString(request);

                std::cout   << "\nREQUEST\nmethod: {" <<  method
                            << "}\npath: {" << path
                            << "}\nqueryStrings: {"<< queryString
                            << "}\nrequst {\n" << request << "}\n";

                std::string response = constructHTMLResponse(Status::NOT_FOUND);

                if (method == "GET" && get_routes.find(path) != get_routes.end())
                    response = get_routes[path](queryString);

                send(client, response.c_str(), response.size(), 0);
            }
            close(client);
    }
}

void WebServer::stop() {
    running = false;
    std::cout<< "  ...closing server." << std::endl;

    if (server_fd != -1) {
        if (shutdown(server_fd, SHUT_RDWR)) { perror("shutdown"); } // shutdown accept, but seems a bit buggy to me
        if (close(server_fd) != 0) { perror("close"); }
        server_fd = -1;
    }
}


std::string WebServer::readFullFequest(int client) {
    std::string request;
    char buffer[4096];
    bool method_checked = false;

    while (true) {
        int bytes = read(client, buffer, sizeof(buffer));
        if (bytes <= 0) break;
        request.append(buffer, bytes);

        if (method_checked == false && bytes > 3) {
            std::string method = parseMethod(request);
            if (method != "GET") break;
        }

        if (request.find("\r\n\r\n") != std::string::npos)
            break;
    }

    return request;
}


void grapvis() {
    std::string dotSpec = "strict digraph {A -> B\nB -> C }";

    GVC_t *gvc = gvContext();

    Agraph_t *g = agmemread(dotSpec.c_str());
    if (!g) {
    std::cerr << "Error: could not parse graph spec.\n";
    gvFreeContext(gvc);
    return;
    }

    gvLayout(gvc, g, "dot");

    gvRender(gvc, g, "png", stdout);

    gvFreeLayout(gvc, g);

    agclose(g);

    gvFreeContext(gvc);
}


