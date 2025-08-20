# Agentie de Voiaj - Travel Agency Application

## Overview

Agentie de Voiaj is a comprehensive travel agency management system consisting of a C++ server and a Qt-based client application. The system allows users to search for travel offers, make reservations, and manage their travel profiles.

## Architecture

### Server (C++ Socket-based)
- **Technology**: C++ with Windows Sockets (Winsock2)
- **Communication**: TCP/IP on port 8080
- **Database**: SQL Server with ODBC connectivity
- **Protocol**: JSON-based message format
- **Features**: Multi-threaded client handling, connection pooling, demo mode fallback

### Client (Qt GUI Application)
- **Technology**: Qt 6 with C++
- **UI Framework**: Qt Widgets with custom styling
- **Communication**: TCP client connecting to server
- **Features**: Modern UI with dark/light themes, responsive design, offline caching

## Features

### Core Functionality
- ✅ **Travel Offers**: Search and browse travel packages
- ✅ **Reservations**: Book and manage travel reservations
- ✅ **User Management**: User profiles and authentication
- ✅ **Real-time Communication**: Live client-server data exchange
- ✅ **Offline Support**: Local caching for offline viewing

### Technical Features
- ✅ **Multi-threading**: Concurrent client handling
- ✅ **Connection Pooling**: Efficient resource management
- ✅ **Error Handling**: Comprehensive error logging and recovery
- ✅ **Demo Mode**: Fallback operation without database
- ✅ **Responsive UI**: Modern, user-friendly interface

## Prerequisites

### Required Software
- **Visual Studio 2019/2022** with C++ development tools
- **Qt 6.x** with MSVC compiler
- **SQL Server 2019/2022** (Express edition supported)
- **vcpkg** package manager
- **Windows 10/11** (x64)

### Required Packages (via vcpkg)
```bash
vcpkg install nlohmann-json:x64-windows
vcpkg install openssl:x64-windows
```

## Project Structure

```
Agentie_de_Voiaj/
├── Agentie_de_Voiaj.sln          # Visual Studio solution
├── Agentie_de_Voiaj_Server/      # Server project
│   ├── include/                   # Header files
│   ├── src/                       # Source files
│   ├── sql/                       # Database scripts
│   └── tests/                     # Unit tests
├── Agentie_de_Voiaj_Client/      # Client project
│   ├── include/                   # Header files
│   ├── src/                       # Source files
│   ├── resources/                 # UI resources and styles
│   └── tests/                     # Unit and integration tests
├── build.bat                      # Build script
├── run_server.bat                 # Server startup script
├── run_client.bat                 # Client startup script
├── run_tests.bat                  # Test execution script
├── setup_database.bat             # Database setup script
├── cleanup.bat                    # Cleanup script
└── README.md                      # This file
```

## Quick Start

### 0. See Available Scripts
```bash
# List all available scripts and their purposes
list_scripts.bat
```

### 1. Build the Project
```bash
# Build both server and client
build.bat

# OR: Complete fresh start (cleans everything and rebuilds)
reset_all.bat
```

### 2. Quick System Test
```bash
# Test if system is ready
quick_test.bat
```

### 3. Setup Database
```bash
# For a completely fresh start (optional - deletes existing data)
fresh_database.bat

# Create database and tables
setup_database.bat
```

### 4. Start Server
```bash
# Start the server on port 8080
run_server.bat
```

### 5. Start Client
```bash
# Start the client application
run_client.bat
```

## Detailed Setup Instructions

### Database Configuration
1. **Install SQL Server** (Express edition is sufficient)
2. **Enable TCP/IP** in SQL Server Configuration Manager
3. **Run quick test**: `quick_test.bat` to verify SQL Server accessibility
4. **Run setup script**: `setup_database.bat` to create database and tables
5. **Verify connection**: Server will automatically detect SQL Server instance

### Build Configuration
1. **Open solution** in Visual Studio
2. **Set Qt paths** in project properties
3. **Configure vcpkg** integration
4. **Build solution** or use `build.bat`

### Network Configuration
- **Server Port**: 8080 (configurable in `config.h`)
- **Client Connection**: 127.0.0.1:8080
- **Firewall**: Ensure port 8080 is accessible

## Testing

### Run All Tests
```bash
run_tests.bat
```

### Individual Test Suites
- **Server Unit Tests**: `Agentie_de_Voiaj_Server\tests\unit_tests.exe`
- **Client Unit Tests**: `Agentie_de_Voiaj_Client\tests\client_tests.exe`
- **Integration Tests**: `Agentie_de_Voiaj_Client\tests\integration_tests.exe`

### Test Coverage
- ✅ **Protocol Handler**: Message processing and routing
- ✅ **Database Manager**: CRUD operations and error handling
- ✅ **Socket Server**: Connection management and threading
- ✅ **Client Models**: Data management and caching
- ✅ **UI Components**: User interaction and state management
- ✅ **Integration**: End-to-end client-server communication

## API Documentation

### Message Format
All communication uses JSON format:

```json
{
  "type": "SEARCH_OFFERS",
  "data": {
    "destination": "Paris",
    "min_price": 100.0,
    "max_price": 1000.0
  }
}
```

### Supported Commands
- `SEARCH_OFFERS` - Search travel offers
- `BOOK_OFFER` - Make a reservation
- `GET_USER_RESERVATIONS` - Retrieve user bookings
- `CANCEL_RESERVATION` - Cancel a booking
- `UPDATE_USER_INFO` - Update user profile

## Troubleshooting

### Common Issues

#### Build Errors
- **Qt not found**: Verify Qt installation and paths
- **vcpkg packages missing**: Run `vcpkg install` commands
- **MSVC compiler**: Ensure C++17 support is enabled

#### Runtime Errors
- **Server won't start**: Check port availability and firewall
- **Database connection failed**: Verify SQL Server and ODBC configuration
- **Client can't connect**: Ensure server is running on correct port

#### Database Issues
- **Tables not found**: Run `setup_database.bat`
- **Permission denied**: Check SQL Server authentication settings
- **Connection timeout**: Verify network configuration
- **Database not found**: Server automatically creates database if it doesn't exist
- **SQL Server not accessible**: Run `quick_test.bat` to diagnose connection issues
- **Database creation fails**: Run `test_database.bat` for step-by-step debugging
- **Reset issues**: Use `fresh_database.bat` then `setup_database.bat` separately

### Debug Mode
- **Server logs**: Check console output for detailed information
- **Client logs**: Enable debug logging in application settings
- **Network traces**: Use Wireshark or similar tools for packet analysis

## Development

### Adding New Features
1. **Server**: Add handlers in `Protocol_Handler.cpp`
2. **Database**: Create new functions in `Database_Manager.cpp`
3. **Client**: Extend models and UI components
4. **Tests**: Add corresponding test cases

### Code Style
- **C++**: Follow modern C++17+ practices
- **Qt**: Use Qt naming conventions and patterns
- **Comments**: Document complex logic and public APIs
- **Error Handling**: Comprehensive error checking and logging

### Performance Considerations
- **Connection pooling**: Reuse database connections
- **Caching**: Implement client-side data caching
- **Async operations**: Use non-blocking I/O where possible
- **Memory management**: Proper RAII and smart pointer usage

## Deployment

### Production Setup
1. **Server**: Deploy as Windows service
2. **Database**: Use dedicated SQL Server instance
3. **Network**: Configure load balancing if needed
4. **Monitoring**: Implement health checks and logging

### Configuration Files
- **Server config**: `config.h` for server settings
- **Client config**: Application settings for connection details
- **Database config**: ODBC data source configuration

## Contributing

### Development Workflow
1. **Fork** the repository
2. **Create** feature branch
3. **Implement** changes with tests
4. **Submit** pull request

### Quality Standards
- **Code coverage**: Maintain >80% test coverage
- **Documentation**: Update README and inline comments
- **Performance**: Benchmark critical paths
- **Security**: Validate all user inputs

## License

This project is proprietary software. All rights reserved.

## Support

For technical support or questions:
- **Issues**: Create GitHub issue
- **Documentation**: Check this README and inline code comments
- **Community**: Join development discussions

---

**Version**: 2.0.0  
**Last Updated**: January 2024  
**Status**: Production Ready (95% Complete)

