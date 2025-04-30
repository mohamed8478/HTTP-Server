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
#include <map>
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
    // Declare the headers map
    map<string, string> headers;

    // Extract headers
    size_t header_start = request.find("\r\n") + 2;
    size_t header_end = request.find("\r\n\r\n");
  
    string header_str = request.substr(header_start, header_end - header_start);
  
    stringstream ss(header_str);
    string header_line;
    while (getline(ss, header_line)) {
      size_t colon_pos = header_line.find(": ");
      if (colon_pos != string::npos) {
        string header_name = header_line.substr(0, colon_pos);
        string header_value = header_line.substr(colon_pos + 2);
        headers[header_name] = header_value;
      }
    }
  
    // Handle /user-agent request
    if (method == "GET" && path == "/user-agent") {
      string user_agent = headers["User-Agent"];
      stringstream response;
      response << "HTTP/1.1 200 OK\r\n";
      response << "Content-Type: text/plain\r\n";
      response << "Content-Length: " << user_agent.length() << "\r\n";
      response << "\r\n";
      response << user_agent;
  
      string response_str = response.str();
      send(client, response_str.c_str(), response_str.length(), 0);
    }
    else {
      const char* not_found = "HTTP/1.1 404 Not Found\r\n\r\n";
      send(client, not_found, strlen(not_found), 0);
    }
  



  close(server_fd);

  return 0;
}
