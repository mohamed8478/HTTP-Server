#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
using namespace std;

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  
  // Only one server_fd declaration
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }

  // Set SO_REUSEADDR option
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }
  
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);
  
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }
  
  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }
  
  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  
  std::cout << "Waiting for a client to connect...\n";
  
  int client = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
  std::cout << "Client connected\n";
  
  // After "Client connected"
char buffer[4096] = {0};
recv(client, buffer, sizeof(buffer) - 1, 0);

// Extract the request line
std::string request(buffer);
std::istringstream request_stream(request);
std::string method, path, version;
request_stream >> method >> path >> version;

// Decide response based on path
    // Check if it's a GET /echo/{str} request
    std::string prefix = "/echo/";
    if (method == "GET" && path == "/") {
      std::string body = "";
      std::ostringstream response;
      response << "HTTP/1.1 200 OK\r\n";
      response << "Content-Length: " << body.length() << "\r\n";
      response << "Content-Type: text/plain\r\n";
      response << "\r\n";
      response << body;
      std::string response_str = response.str();
      send(client, response_str.c_str(), response_str.length(), 0);
  }
  else if (method == "GET" && path.rfind("/echo/", 0) == 0) {
      std::string echoed_str = path.substr(6);
      std::string response_body = echoed_str;
      std::ostringstream response;
      response << "HTTP/1.1 200 OK\r\n";
      response << "Content-Type: text/plain\r\n";
      response << "Content-Length: " << response_body.length() << "\r\n";
      response << "\r\n";
      response << response_body;
      std::string response_str = response.str();
      send(client, response_str.c_str(), response_str.length(), 0);
  }
  else {
      const char* not_found = "HTTP/1.1 404 Not Found\r\n\r\n";
      send(client, not_found, strlen(not_found), 0);
  }
  



  close(server_fd);

  return 0;
}
