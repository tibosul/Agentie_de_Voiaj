# Agentie de Voiaj - Travel Agency System

## Project Overview
C++ client-server application for a travel agency management system.

## Architecture
- **Server**: C++ socket server with JSON protocol (TCP/IP on port 8080)
- **Client**: Qt-based GUI application
- **Database**: SQL Server with ODBC connection
- **Communication**: JSON bidirectional messages

## Build Requirements
- Visual Studio 2019/2022
- Qt Framework (for client)
- vcpkg for dependencies:
  - nlohmann-json
  - openssl
- SQL Server (LocalDB or full instance)

## Key Components

### Server (Agentie_de_Voiaj_Server/)
- `Socket_Server.cpp/h` - TCP server and client handling
- `Protocol_Handler` - JSON message processing  
- `Database_Manager.cpp/h` - SQL Server operations
- `utils.cpp/h` - Utility functions (crypto, validation, JSON)
- `config.h` - Configuration constants
- `sql/` - Database schema and initial data

### Client (Agentie_de_Voiaj_Client/)  
- Qt-based GUI application
- Communicates with server via JSON over TCP

## Protocol Format
All messages are JSON:

**Request Example:**
```json
{
  "type": "AUTH",
  "username": "admin",
  "password": "test123"
}
```

**Response Example:**
```json
{
  "success": true,
  "message": "Login successful",
  "data": {
    "user_id": 123,
    "username": "admin"
  }
}
```

## Supported Commands (Client-only scope)
- AUTH/LOGIN - User authentication
- REGISTER/SIGNUP - User registration  
- GET_DESTINATIONS - Retrieve destinations
- GET_OFFERS - Get travel offers
- SEARCH_OFFERS - Search with filters
- BOOK_OFFER - Make reservation
- GET_USER_RESERVATIONS - User's bookings
- CANCEL_RESERVATION - Cancel booking
- GET_USER_INFO - Get user profile
- UPDATE_USER_INFO - Update user profile
- KEEPALIVE - Connection maintenance

**Note:** Admin features (manage offers, users) are NOT implemented - this is a college project focused on client functionality only.

## Development Notes
- Port: 8080 (configurable in config.h)
- Max connections: 100
- JSON parsing: nlohmann/json library
- Authentication: Password hashing with salt
- Database: Windows Authentication by default

## Build Instructions
1. Open `Agentie_de_Voiaj.sln` in Visual Studio
2. Ensure vcpkg dependencies are installed
3. Build solution (Debug/Release)
4. Run server first, then client

## Testing
Run `test_compile.bat` to verify header compilation.
Main.cpp includes comprehensive protocol tests.