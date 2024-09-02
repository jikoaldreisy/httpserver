#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

//Create and Bind a socket
int main() {
  int server_fd, new_socket;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  char buffer[1024] = {0};

  //create socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0))==0) { 
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(8080);

  //Bind socket to the network address and sin_port
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  // Listen for incoming connections
  if (listen(server_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  while(1) {
    printf("\nWaiting for a connection...\n");

    // Accept an incoming connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
    perror("accept");
      exit(EXIT_FAILURE);
    }

    // Read the request
    read(new_socket, buffer, 1024);
    printf("Request: %s\n", buffer);
    
    // Parse the request
    char *method = strtok(buffer, " ");
    char *path = strtok(NULL, " ");

    if (strcmp(path, "/") == 0) {
      path = "/index.html";
    }

    char *file_path = path + 1;

    // Read the requested file
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
      // File not found, send 404 response
      char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found";
      send(new_socket, not_found, strlen(not_found), 0);
    } else {
    // File found, send 200 response
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *file_content = malloc(file_size);
    fread(file_content, 1, file_size, file);

    char header[1024];
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n", file_size);
    send(new_socket, header, strlen(header), 0);
    send(new_socket, file_content, file_size, 0);

    free(file_content);
    fclose(file);
    }

    // Close the connection
    close(new_socket);
  }
  
  return 0;
}
