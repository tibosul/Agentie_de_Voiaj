# Data Structures Organization

This document describes the reorganized data structure layout for the Agentie de Voiaj project.

## Overview

Data structures have been reorganized to ensure consistency between client and server, match the database schema exactly, and provide better maintainability by placing each structure in its own file.

## Database Schema Mapping

All data structures now map directly to database tables:

| Database Table | Data Structure | Server File | Client File |
|---|---|---|---|
| Users | User_Data | `/include/models/User_Data.h` | `/include/models/User_Data.h` |
| Destinations | Destination_Data | `/include/models/Destination_Data.h` | `/include/models/Destination_Data.h` |
| Types_of_Transport | Transport_Type_Data | `/include/models/Transport_Type_Data.h` | `/include/models/Transport_Type_Data.h` |
| Types_of_Accommodation | Accommodation_Type_Data | `/include/models/Accommodation_Type_Data.h` | `/include/models/Accommodation_Type_Data.h` |
| Accommodations | Accommodation_Data | `/include/models/Accommodation_Data.h` | `/include/models/Accommodation_Data.h` |
| Offers | Offer_Data | `/include/models/Offer_Data.h` | `/include/models/Offer_Data.h` |
| Reservations | Reservation_Data | `/include/models/Reservation_Data.h` | `/include/models/Reservation_Data.h` |
| Reservation_Persons | Reservation_Person_Data | `/include/models/Reservation_Person_Data.h` | `/include/models/Reservation_Person_Data.h` |

## Additional Client Structures

The client also includes:
- **Booking_Data** (`/include/models/Booking_Data.h`) - Used during the booking process before creating reservations

## Usage

### Server Side
```cpp
#include "models/All_Data_Structures.h"  // Include all structures
// or
#include "models/User_Data.h"            // Include specific structure
```

### Client Side
```cpp
#include "models/All_Data_Structures.h"  // Include all structures  
// or
#include "models/User_Data.h"            // Include specific structure
```

## Key Changes Made

1. **Extracted structures** from `Database_Manager.h` to individual files
2. **Renamed structures** to match database tables exactly:
   - `Trip_Data` → `Offer_Data` (matches Offers table)
   - `Booking_Data` concept split into `Reservation_Data` and `Booking_Data`
3. **Added missing structures** to client side
4. **Ensured field consistency** between database schema, server, and client
5. **Updated method signatures** in Database_Manager to use new structure names
6. **Added helper methods** to structures for common operations

## Type Compatibility

- **Server**: Uses `std::string` for string fields
- **Client**: Uses `QString` for string fields
- **Database fields**: All field names match exactly between structures and database columns

## File Organization

```
Server:
├── include/models/
│   ├── All_Data_Structures.h          # Common include file
│   ├── User_Data.h
│   ├── Destination_Data.h
│   ├── Transport_Type_Data.h
│   ├── Accommodation_Type_Data.h
│   ├── Accommodation_Data.h
│   ├── Offer_Data.h
│   ├── Reservation_Data.h
│   └── Reservation_Person_Data.h

Client:
├── include/models/
│   ├── All_Data_Structures.h          # Common include file
│   ├── User_Data.h
│   ├── Destination_Data.h
│   ├── Transport_Type_Data.h
│   ├── Accommodation_Type_Data.h
│   ├── Accommodation_Data.h
│   ├── Offer_Data.h
│   ├── Reservation_Data.h
│   ├── Reservation_Person_Data.h
│   ├── Booking_Data.h                 # Client-specific
│   ├── User_Model.h                   # Qt Model classes
│   ├── Offer_Model.h
│   ├── Reservation_Model.h
│   └── Destination_Model.h
```

This organization provides better maintainability, consistency, and makes it easier to add new data structures in the future.