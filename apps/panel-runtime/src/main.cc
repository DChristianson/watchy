#include "watchpanel/watchpanel.h"
#include "watchpanel/svg_canvas.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

namespace {

std::atomic<bool> running(true);
std::atomic<int> listenSocket(-1);

void HandleShutdownSignal(int) { running = false; }

std::string ReadFile(const std::string &path) {
  std::ifstream in(path, std::ios::binary);
  std::ostringstream contents;
  contents << in.rdbuf();
  return contents.str();
}

void SendResponse(int client, const std::string &status, const std::string &contentType,
                   const std::string &body) {
  std::ostringstream response;
  response << "HTTP/1.1 " << status << "\r\n"
            << "Content-Type: " << contentType << "\r\n"
            << "Content-Length: " << body.size() << "\r\n"
            << "Cache-Control: no-store\r\n"
            << "Connection: close\r\n\r\n"
            << body;
  const std::string out = response.str();
  send(client, out.data(), out.size(), 0);
}

const char *kIndexHtmlTemplate = R"HTML(<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>watchy panel preview</title>
<style>
  body { background: #111; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }
  img { image-rendering: pixelated; width: 512px; height: 512px; background: #000; }
</style>
</head>
<body>
<img id="panel" src="/panel.svg">
<script>
setInterval(function () {
  document.getElementById('panel').src = '/panel.svg?t=' + Date.now();
}, %REFRESH_MS%);
</script>
</body>
</html>
)HTML";

// Single-threaded HTTP/1.0 server: serves the auto-refreshing preview page
// at "/" and the latest rendered frame at "/panel.svg". Good enough for a
// local dev preview; not meant to handle concurrent load.
void ServeForever(int port, const std::string &svgPath, long refreshMs) {
  const int server = socket(AF_INET, SOCK_STREAM, 0);
  if (server < 0) {
    std::cerr << "web preview: failed to create socket" << std::endl;
    return;
  }

  const int reuse = 1;
  setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(static_cast<uint16_t>(port));

  if (bind(server, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
    std::cerr << "web preview: failed to bind port " << port << " (preview disabled)" << std::endl;
    close(server);
    return;
  }
  listen(server, 4);
  listenSocket = server;

  std::string indexHtml(kIndexHtmlTemplate);
  const std::string placeholder = "%REFRESH_MS%";
  indexHtml.replace(indexHtml.find(placeholder), placeholder.size(), std::to_string(refreshMs));

  std::cout << "web preview: http://localhost:" << port << std::endl;

  while (running) {
    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);
    const int client = accept(server, reinterpret_cast<sockaddr *>(&clientAddr), &clientLen);
    if (client < 0) continue;

    char requestBuf[1024] = {0};
    recv(client, requestBuf, sizeof(requestBuf) - 1, 0);
    const std::string request(requestBuf);

    if (request.rfind("GET /panel.svg", 0) == 0) {
      SendResponse(client, "200 OK", "image/svg+xml", ReadFile(svgPath));
    } else {
      SendResponse(client, "200 OK", "text/html", indexHtml);
    }
    close(client);
  }
}

}  // namespace

int main(int argc, char **argv) {
  const char *page = argc > 1 ? argv[1] : "configs/pages/weather.xml";
  const int port = argc > 2 ? std::stoi(argv[2]) : 8080;
  const std::string config = "configs/runtime/config.json";
  const std::string secrets = "configs/runtime/secrets.json";
  const std::string outPath = "./out.svg";
  const long refreshMs = 2000;

  std::signal(SIGINT, HandleShutdownSignal);
  std::signal(SIGTERM, HandleShutdownSignal);
  std::signal(SIGPIPE, SIG_IGN);

  watchpanel::SvgCanvas canvas(64, 64);
  watchpanel::WatchPanel panel(&canvas, config, secrets);
  if (panel.Load(page) != 0) {
    std::cerr << "failed to load panel page: " << page << std::endl;
    return 1;
  }

  std::thread server(ServeForever, port, outPath, refreshMs);

  std::cout << "panel-runtime: rendering " << page << " every " << refreshMs << "ms" << std::endl;

  while (running) {
    panel.Update();
    canvas.Clear();
    panel.Draw();
    canvas.Save(outPath.c_str());
    std::this_thread::sleep_for(std::chrono::milliseconds(refreshMs));
  }

  // The server thread is blocked in accept(); shutdown()/close() its
  // listening socket to unblock it so the thread can actually exit.
  const int fd = listenSocket.load();
  if (fd >= 0) {
    shutdown(fd, SHUT_RDWR);
    close(fd);
  }
  server.join();
  std::cout << "panel-runtime: stopped" << std::endl;
  return 0;
}
