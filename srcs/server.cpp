
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <sstream>
#include <csignal>
#include <atomic>

#include <graphviz/gvc.h>

#include "server.hpp"

WebServer* g_server = nullptr;
std::atomic<bool> running{true};

void handle_sigint(int) {
    std::cout << "\nCtrl+C detected! Closing server...\n";
    if (g_server) g_server->stop();
    std::exit(0);
}

WebServer::WebServer(int port) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); exit(1); }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); exit(1); }
    if (listen(server_fd, 10) < 0) { perror("listen"); exit(1); }

    std::cout << "Server listening on port " << port << "...\n";
}

WebServer::~WebServer() {
    close(server_fd);
}

void WebServer::get(const std::string& path, Handler handler) {
    get_routes[path] = handler;
}

std::string WebServer::parse_path(const std::string& request) {
    size_t start = request.find(" ") + 1;
    size_t end = request.find(" ", start);
    return request.substr(start, end - start);
}

std::string WebServer::parse_method(const std::string& request) {
    size_t end = request.find(" ");
    return request.substr(0, end);
}

std::string WebServer::parse_body(const std::string& request) {
    size_t pos = request.find("\r\n\r\n");
    if (pos != std::string::npos) return request.substr(pos + 4);
    return "";
}

void WebServer::start() {
    std::signal(SIGINT, handle_sigint);
    g_server = this;
    while (true) {
        int client = accept(server_fd, nullptr, nullptr);
        if (client < 0) { perror("accept"); continue; }

        std::thread([this, client]() {
            std::string request = read_request(client);
            if (request.length() > 0) {

                std::string method = parse_method(request);
                std::string path = parse_path(request);
                std::string body = parse_body(request);

                std::string response_body = "404 Not Found";

                if (method == "GET" && get_routes.find(path) != get_routes.end())
                    response_body = get_routes[path](body);

                std::ostringstream response;
                response << "HTTP/1.1 200 OK\r\n"
                         << "Content-Type: text/plain\r\n"
                         << "Content-Length: " << response_body.size() << "\r\n"
                         << "\r\n"
                         << response_body;

                std::string res = response.str();
                send(client, res.c_str(), res.size(), 0);
            }
            close(client);
        }).detach();
    }
}

void WebServer::stop() {
    running = false;
    close(server_fd);
}


std::string WebServer::read_request(int client) {
    std::string request;
    char buffer[4096];
    bool method_checked = false;

    while (true) {
        int bytes = read(client, buffer, sizeof(buffer));
        if (bytes <= 0) break;
        request.append(buffer, bytes);

        if (method_checked == false && bytes > 3) {
            std::string method = parse_method(request);
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


