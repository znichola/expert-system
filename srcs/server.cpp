#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <csignal>

#include "expert-system.hpp"
#include "parser.hpp"
#include "server.hpp"

static std::string urlDecode(const std::string &src);
static std::string genGraphImg(const Digraph &digraph);


WebServer* g_server = nullptr;
bool running = true;

void handleSigint(int) {
    std::cout << "\nSigint...\n";
    if (g_server) g_server->stop();
    exit(0);
}

WebServer::WebServer(const InputOptions &opts) : opts(opts) {

    prefillRuleset = opts.file ? getFileInput(opts.file) : "";

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); exit(1); }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt"); exit(1); }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(opts.port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {perror("bind"); exit(1); }
    if (listen(server_fd, 10) < 0) { perror("listen"); exit(1); }

    std::cout << "Server listening on port " << opts.port << "...\n";

    registerGetRoutes();
}

WebServer::~WebServer() {
    if (server_fd != -1 && close(server_fd) != 0) perror("destructor close");
}

static std::string favicon();

void WebServer::registerGetRoutes() {
    get_routes["/"] = [this](std::string queryParam) -> std::string {
        (void)queryParam;
        std::ostringstream body;
        body << "<h1>Expert System</h1>\n"
        << "<p>Enter your ruleset here</p>\n"
        << "<form action=\"evaluate\" method=\"get\">\n"
        << "    <textarea name=\"rules\" placeholder=\"Enter your ruleset here...\">" << prefillRuleset << "</textarea><br>\n"
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
            rules = "# No rules submitted.";
        }

        std::ostringstream report;
        std::string img;

        try {
            std::vector<Token> tokens = tokenizer(rules);
            auto [rules, facts, queries] = parseTokens(tokens);
            Digraph digraph = makeDigraph(facts, rules);
            digraph.isExplain = opts.isExplain;
            img = genGraphImg(digraph);
            digraph.applyWorldAssumption(opts.isOpenWorldAssumption);

            auto [conclusion, explanation, isError] = digraph.solveEverythingNoThrow(queries);
            report << "CONCLUSION\n"  << conclusion << "\n"
                   << "EXPLANATION\n" << explanation;
            (void)isError;

        } catch (std::exception &e) {
            report << "Error: " << e.what() << std::endl;
        }


        std::ostringstream body;
        body << "<h1>Evaluation</h1>\n"
            << "<p>Submitted rules</p>\n"
            << "<pre>" << rules << "</pre>\n"
            << "<p>RESULTS</p>\n"
            << "<pre>" << report.str() << "</pre>\n"
            << "<p>Node digraph</p>"
            << img
            << "<a href=\"/\">Back</a>\n";

        return constructHTMLResponse(Status::OK, body.str());
    };

    get_routes["/favicon.ico"] = [this](std::string queryParam) -> std::string {
        (void)queryParam;
        (void)this;
        std::ostringstream response;
        std::string icon = favicon();
        response << "HTTP/1.1 200\r\n"
                << "Content-Type: image/svg+xml; charset=UTF-8\r\n"
                << "Connection : close\r\n"
                << "Content-Length: " << icon.size() << "\r\n"
                << "Cache-Control: public, max-age=31536000, immutable\r\n"
                << "\r\n"
                << icon;
        std::cout << "Returning icon\n";
        return response.str();
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
             << "    body { font-family: sans-serif; margin: 2em auto; text-align: center; color: #748873; background-color: #E5E0D8; max-width: 70ch; } \n"
             << "    textarea { width: 90%; height: 200px; margin: 1em 0; font-family: monospace; }\n"
             << "    button { padding: 0.5em 1.5em; font-size: 1em; cursor: pointer; color: #D1A980; }\n"
             << "    pre { text-align: left; overflow-x: auto; background-color: #F8F8F8; padding: 1.1em; border: solid; }\n"
             << "    a { color: #D1A980; }\n a:visited { color: #b48e68; }\n"
             << "    img { border: solid; display: block; width: 100%; width: -moz-available; width: -webkit-fill-available; width: stretch; margin: auto; }\n"
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

                std::cout   << "\nREQUEST\n"
                            // << "method: {" << method << "}\n"
                            << "path: {" << path << "}\n"
                            << "queryStrings: {"<< queryString << "}\n"
                            // << "requst {\n" << request 
                            ;

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

// Utils / helpers

static std::string urlDecode(const std::string &src) {
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

// #define WITH_GRAPHVIZ

#ifdef WITH_GRAPHVIZ 
#include <graphviz/gvc.h>

static std::string base64_encode(const std::string &in);
static void graphvisToFilePNG(const std::string &input, FILE *out);
static std::string imgHTMLFromRawImg(const std::string &rawImg);

static std::string genGraphImg(const Digraph &digraph) {
    (void)digraph;

    char* buffer = nullptr;
    size_t size = 0;
    
    // Create memory stream to store resulting png image of the graph
    FILE* memfile = open_memstream(&buffer, &size);
    if (!memfile) {
        perror("open_memstream");
        return "";
    }

    const std::string &dotFile = digraph.toDot();
    graphvisToFilePNG(dotFile, memfile);

    fclose(memfile);

    std::string pngGraph(buffer, size);

    free(buffer);

    return imgHTMLFromRawImg(base64_encode(pngGraph));
}

static std::string imgHTMLFromRawImg(const std::string &rawImg) {
    return "<img alt=\"My Image\" src=\"data:image/png;base64," + rawImg + "\">\n";
}

static void graphvisToFilePNG(const std::string &input, FILE *out) {
    GVC_t *gvc = gvContext();

    Agraph_t *g = agmemread(input.c_str());
    if (!g) {
        std::cerr << "Error: could not parse graph spec.\n";
        gvFreeContext(gvc);
        return;
    }

    gvLayout(gvc, g, "dot");
    gvRender(gvc, g, "png", out);
    gvFreeLayout(gvc, g);
    agclose(g);
    gvFreeContext(gvc);
}

//https://stackoverflow.com/a/34571089/5155484

typedef unsigned char uchar;
static const std::string b = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";//=
static std::string base64_encode(const std::string &in) {
    std::string out;

    int val=0, valb=-6;
    for (uchar c : in) {
        val = (val<<8) + c;
        valb += 8;
        while (valb>=0) {
            out.push_back(b[(val>>valb)&0x3F]);
            valb-=6;
        }
    }
    if (valb>-6) out.push_back(b[((val<<8)>>(valb+8))&0x3F]);
    while (out.size()%4) out.push_back('=');
    return out;
}

#else

static std::string genGraphImg(const Digraph &digraph) {
    (void)digraph;
    return "<div style='border: solid; background-color: white; padding: 1em;'>Install graphviz for cool graphs</div>";
}

#endif

static std::string favicon() {
    return {R"DELIM(
<svg xmlns="http://www.w3.org/2000/svg" width="138.7" height="163.6">
  <g stroke-linecap="round">
    <path fill="none" stroke="#748873" stroke-width="4" d="m80 29 27 39M80 29l27 39"/>
    <path fill="#748873" fill-rule="evenodd" d="m107 68-13-7 11-8 2 15"/>
    <path fill="none" stroke="#748873" stroke-width="4" d="m107 68-13-7m13 7-13-7m0 0 11-8m-11 8 11-8m0 0 2 15m-2-15 2 15m0 0s0 0 0 0m0 0s0 0 0 0"/>
  </g>
  <g stroke-linecap="round">
    <path fill="none" stroke="#748873" stroke-width="4" d="m56 70 14 31M56 70l14 31"/>
    <path fill="#748873" fill-rule="evenodd" d="M70 101 58 91l12-5v15"/>
    <path fill="none" stroke="#748873" stroke-width="4" d="M70 101 58 91m12 10L58 91m0 0 12-5m-12 5 12-5m0 0v15m0-15v15m0 0s0 0 0 0m0 0s0 0 0 0"/>
  </g>
  <g stroke-linecap="round">
    <path fill="none" stroke="#748873" stroke-width="4" d="m115 88 1 38m-1-38 1 38"/>
    <path fill="#748873" fill-rule="evenodd" d="m116 126-6-14h12l-6 14"/>
    <path fill="none" stroke="#748873" stroke-width="4" d="m116 126-6-14m6 14-6-14m0 0h12m-12 0h12m0 0-6 14m6-14-6 14m0 0s0 0 0 0m0 0s0 0 0 0"/>
  </g>
  <g stroke-linecap="round">
    <path fill="none" stroke="#748873" stroke-width="4" d="m81 113 24 21m-24-21 24 21"/>
    <path fill="#748873" fill-rule="evenodd" d="m105 134-15-5 9-9 6 14"/>
    <path fill="none" stroke="#748873" stroke-width="4" d="m105 134-15-5m15 5-15-5m0 0 9-9m-9 9 9-9m0 0 6 14m-6-14 6 14m0 0s0 0 0 0m0 0s0 0 0 0"/>
  </g>
  <g stroke-linecap="round">
    <path fill="#748873" d="M91 23a14 14 0 0 1-5 11 12 12 0 0 1-3 2 12 12 0 0 1-7 1 12 12 0 0 1-7-5 14 14 0 0 1-3-9 15 15 0 0 1 0-4 14 14 0 0 1 4-6 12 12 0 0 1 10-3 12 12 0 0 1 8 5 14 14 0 0 1 3 6v2c0 1 0 0 0 0"/>
    <path fill="none" stroke="#748873" d="M91 23a14 14 0 0 1-5 11 12 12 0 0 1-3 2 12 12 0 0 1-7 1 12 12 0 0 1-7-5 14 14 0 0 1-3-9 15 15 0 0 1 0-4 14 14 0 0 1 4-6 12 12 0 0 1 10-3 12 12 0 0 1 8 5 14 14 0 0 1 3 6v2c0 1 0 0 0 0"/>
  </g>
  <g stroke-linecap="round">
    <path fill="#748873" d="M128 80a14 14 0 0 1-5 10 12 12 0 0 1-4 3 12 12 0 0 1-6 0 12 12 0 0 1-8-4 14 14 0 0 1-3-9 15 15 0 0 1 1-5 14 14 0 0 1 4-5 12 12 0 0 1 10-3 12 12 0 0 1 8 4 14 14 0 0 1 2 7l1 2"/>
    <path fill="none" stroke="#748873" d="M128 80a14 14 0 0 1-5 10 12 12 0 0 1-4 3 12 12 0 0 1-6 0 12 12 0 0 1-8-4 14 14 0 0 1-3-9 15 15 0 0 1 1-5 14 14 0 0 1 4-5 12 12 0 0 1 10-3 12 12 0 0 1 8 4 14 14 0 0 1 2 7l1 2s0 0 0 0"/>
  </g>
  <g stroke-linecap="round">
    <path fill="#748873" d="M66 67a14 14 0 0 1-5 10 12 12 0 0 1-3 2 12 12 0 0 1-7 1 12 12 0 0 1-7-5 14 14 0 0 1-3-8 15 15 0 0 1 0-5 14 14 0 0 1 4-6 12 12 0 0 1 10-2 12 12 0 0 1 8 4 14 14 0 0 1 3 6v3s0-1 0 0"/>
    <path fill="none" stroke="#748873" d="M66 67a14 14 0 0 1-5 10 12 12 0 0 1-3 2 12 12 0 0 1-7 1 12 12 0 0 1-7-5 14 14 0 0 1-3-8 15 15 0 0 1 0-5 14 14 0 0 1 4-6 12 12 0 0 1 10-2 12 12 0 0 1 8 4 14 14 0 0 1 3 6v3s0-1 0 0"/>
  </g>
  <g stroke-linecap="round">
    <path fill="#748873" d="M90 110a14 14 0 0 1-5 10 12 12 0 0 1-3 2 12 12 0 0 1-7 1 12 12 0 0 1-7-5 14 14 0 0 1-3-8 15 15 0 0 1 0-5 14 14 0 0 1 4-6 12 12 0 0 1 10-3 12 12 0 0 1 8 5 14 14 0 0 1 3 6v3s0-1 0 0"/>
    <path fill="none" stroke="#748873" d="M90 110a14 14 0 0 1-5 10 12 12 0 0 1-3 2 12 12 0 0 1-7 1 12 12 0 0 1-7-5 14 14 0 0 1-3-8 15 15 0 0 1 0-5 14 14 0 0 1 4-6 12 12 0 0 1 10-3 12 12 0 0 1 8 5 14 14 0 0 1 3 6v3s0-1 0 0"/>
  </g>
  <g stroke-linecap="round">
    <path fill="#748873" d="M35 118a14 14 0 0 1-4 10 12 12 0 0 1-4 2 12 12 0 0 1-7 1 12 12 0 0 1-7-5 14 14 0 0 1-3-8 15 15 0 0 1 1-5 14 14 0 0 1 4-6 12 12 0 0 1 10-2 12 12 0 0 1 7 4 14 14 0 0 1 3 6v3s0-1 0 0"/>
    <path fill="none" stroke="#748873" d="M35 118a14 14 0 0 1-4 10 12 12 0 0 1-4 2 12 12 0 0 1-7 1 12 12 0 0 1-7-5 14 14 0 0 1-3-8 15 15 0 0 1 1-5 14 14 0 0 1 4-6 12 12 0 0 1 10-2 12 12 0 0 1 7 4 14 14 0 0 1 3 6v3s0-1 0 0"/>
  </g>
  <g stroke-linecap="round">
    <path fill="#748873" d="M129 140a14 14 0 0 1-5 10 12 12 0 0 1-4 3 12 12 0 0 1-6 0 12 12 0 0 1-8-4 14 14 0 0 1-3-9 15 15 0 0 1 1-4 14 14 0 0 1 4-6 12 12 0 0 1 10-3 12 12 0 0 1 8 5 14 14 0 0 1 2 6l1 2c0 1 0 0 0 0"/>
    <path fill="none" stroke="#748873" d="M129 140a14 14 0 0 1-5 10 12 12 0 0 1-4 3 12 12 0 0 1-6 0 12 12 0 0 1-8-4 14 14 0 0 1-3-9 15 15 0 0 1 1-4 14 14 0 0 1 4-6 12 12 0 0 1 10-3 12 12 0 0 1 8 5 14 14 0 0 1 2 6l1 2c0 1 0 0 0 0"/>
  </g>
  <g stroke-linecap="round">
    <path fill="none" stroke="#748873" stroke-width="4" d="M77 27 62 58m15-31L62 58"/>
    <path fill="#748873" fill-rule="evenodd" d="M62 58V43l11 5-11 10"/>
    <path fill="none" stroke="#748873" stroke-width="4" d="M62 58V43m0 15V43m0 0 11 5m-11-5 11 5m0 0L62 58m11-10L62 58m0 0s0 0 0 0m0 0s0 0 0 0"/>
  </g>
  <g stroke-linecap="round">
    <path fill="none" stroke="#748873" stroke-width="4" d="m50 77-20 31m20-31-20 31"/>
    <path fill="#748873" fill-rule="evenodd" d="m30 108 2-15 11 7-13 8"/>
    <path fill="none" stroke="#748873" stroke-width="4" d="m30 108 2-15m-2 15 2-15m0 0 11 7m-11-7 11 7m0 0-13 8m13-8-13 8m0 0s0 0 0 0m0 0s0 0 0 0"/>
  </g>
</svg>)DELIM"};
}