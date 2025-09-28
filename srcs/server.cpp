#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <csignal>

#include <graphviz/gvc.h>

#include "server.hpp"

std::string urlDecode(const std::string &src);

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

    registerGetRoutes();
}

WebServer::~WebServer() {
    if (server_fd != -1 && close(server_fd) != 0) perror("destructor close");
}

void WebServer::registerGetRoutes() {
    get_routes["/"] = [this](std::string queryParam) -> std::string {
        (void)queryParam;
        std::ostringstream body;
        body << "<h1>Expert System</h1>\n"
        << "<p>Enter your ruleset here</p>\n"
        << "<form action=\"evaluate\" method=\"get\">\n"
        << "    <textarea name=\"rules\" placeholder=\"Paste your ruleset here...\"></textarea><br>\n"
        << "    <button type=\"submit\">Submit</button>\n"
        << "</form>\n";
        return constructHTMLResponse(Status::OK, body.str());
    };

    get_routes["/evaluate"] = [this](std::string queryParam) -> std::string {
        std::string rules;
        std::string key = "rules=";
        size_t pos = queryParam.find(key);
        if (pos != std::string::npos) {
            rules = urlDecode(queryParam.substr(pos + key.length()));
        } else {
            rules = "No rules submitted.";
        }

        std::ostringstream body;
        body << "<h1>Submitted Ruleset</h1>\n"
            << "<pre style=\"text-align: left;\">" << rules << "</pre>\n"
            << "<a href=\"/\">Back</a>\n";

        return constructHTMLResponse(Status::OK, body.str());
    };
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

// #748873
// #D1A980
// #E5E0D8
// #F8F8F8

    std::ostringstream fullHtml;
    fullHtml << "<!DOCTYPE html>\n"
             << "<html lang=\"en\">\n"
             << "<head>\n"
             << "  <meta charset=\"UTF-8\">\n"
             << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
             << "  <title>Expert System</title>\n"
             << "  <style>\n"
             << "    body { font-family: sans-serif; margin: 2em; text-align: center; color: #748873; background-color: #E5E0D8}\n"
             << "    textarea { width: 90%; height: 200px; margin: 1em 0; font-family: monospace; }\n"
             << "    button { padding: 0.5em 1.5em; font-size: 1em; cursor: pointer; }\n"
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

                std::cout   << "\nREQUEST\nmethod: {" << method
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

std::string urlDecode(const std::string &src) {
    std::string result;
    result.reserve(src.size());
    for (size_t i = 0; i < src.size(); i++) {
        if (src[i] == '+') {
            result.push_back(' ');
        } else if (src[i] == '%' && i + 2 < src.size() &&
                   std::isxdigit(src[i + 1]) && std::isxdigit(src[i + 2])) {
            std::string hex = src.substr(i + 1, 2);
            char decoded = static_cast<char>(std::stoi(hex, nullptr, 16));
            result.push_back(decoded);
            i += 2;
        } else {
            result.push_back(src[i]);
        }
    }
    return result;
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


