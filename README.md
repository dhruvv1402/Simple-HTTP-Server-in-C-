# Simple HTTP Server in C++

This project implements a basic HTTP server in C++ using Boost libraries. The server can handle GET requests and serve static HTML files from a designated directory.

## Features

- Handles basic GET requests
- Serves static HTML files
- Supports various content types (HTML, CSS, JavaScript, images, etc.)
- Provides proper error handling
- Uses asynchronous I/O with Boost.Asio

## Prerequisites

- C++14 compatible compiler
- CMake (version 3.10 or higher)
- Boost libraries (system, filesystem, beast)

## Building the Project

```bash
# Create a build directory
mkdir build
cd build

# Configure and build the project
cmake ..
make
```

## Running the Server

```bash
# Run the server on port 8080 with the www directory as document root
./http_server 8080 ./www
```

## Project Structure

- `main.cpp` - Main server implementation
- `CMakeLists.txt` - CMake build configuration
- `www/` - Directory containing static files to be served
  - `index.html` - Sample HTML file

## How It Works

The server uses Boost.Asio for asynchronous network operations and Boost.Beast for HTTP parsing. When a client connects, the server reads the HTTP request, processes it, and sends back an appropriate response. For GET requests, it looks for the requested file in the document root directory and serves it with the correct content type.

## Extending the Server

This is a basic implementation that can be extended with additional features:

- Support for more HTTP methods (POST, PUT, DELETE)
- HTTP/2 support
- WebSocket support
- Dynamic content generation
- Authentication and authorization
- Logging and metrics