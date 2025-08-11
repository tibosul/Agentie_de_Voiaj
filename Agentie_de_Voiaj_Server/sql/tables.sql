IF EXISTS (
    SELECT 1
    FROM sys.objects
    WHERE object_id = OBJECT_ID(N'dbo.Users')
      AND type = 'U'
)
    DROP TABLE dbo.Users;

CREATE TABLE dbo.Users
(
    User_ID INT PRIMARY KEY IDENTITY(1,1),
    Username VARCHAR(50) NOT NULL UNIQUE,
    Password_Hash VARCHAR(255) NOT NULL,
    Email VARCHAR(100) UNIQUE,
    First_Name VARCHAR(50),
    Last_Name VARCHAR(50),
    Phone VARCHAR(15),
    Date_Created DATETIME DEFAULT GETDATE(),
    Date_Modified DATETIME DEFAULT GETDATE()
);

----------------------------------------------------------------------------


IF EXISTS (
    SELECT 1
    FROM sys.objects
    WHERE object_id = OBJECT_ID(N'dbo.Destinations')
      AND type = 'U'
)
    DROP TABLE dbo.Destinations;

CREATE TABLE dbo.Destinations
(
    Destination_ID INT PRIMARY KEY IDENTITY(1,1),
    Name VARCHAR(100) NOT NULL,
    Country VARCHAR(100) NOT NULL,
    Description TEXT,
    Image_Path VARCHAR(255),
    Date_Created DATETIME DEFAULT GETDATE(),
    Date_Modified DATETIME DEFAULT GETDATE()
);

-------------------------------------------------------------------------------


IF EXISTS (
    SELECT 1
    FROM sys.objects
    WHERE object_id = OBJECT_ID(N'dbo.Types_of_Transport')
      AND type = 'U'
)
    DROP TABLE dbo.Types_of_Transport;

CREATE TABLE dbo.Types_of_Transport
    (
        Transport_Type_ID INT PRIMARY KEY IDENTITY(1,1),
        Name VARCHAR(100) NOT NULL,
        Description TEXT,
        Date_Created DATETIME DEFAULT GETDATE(),
        Date_Modified DATETIME DEFAULT GETDATE()
    );

------------------------------------------------------------------------------------


IF EXISTS (
    SELECT 1
    FROM sys.objects
    WHERE object_id = OBJECT_ID(N'dbo.Types_of_Accommodation')
      AND type = 'U'
)
    DROP TABLE dbo.Types_of_Accommodation;

CREATE TABLE dbo.Types_of_Accommodation
    (
        Accommodation_Type_ID INT PRIMARY KEY IDENTITY(1,1),
        Name VARCHAR(100) NOT NULL,
        Description TEXT,
        Date_Created DATETIME DEFAULT GETDATE(),
        Date_Modified DATETIME DEFAULT GETDATE()
    );


-------------------------------------------------------------------------------------


IF EXISTS (
    SELECT 1
    FROM sys.objects
    WHERE object_id = OBJECT_ID(N'dbo.Accommodations')
      AND type = 'U'
)
    DROP TABLE dbo.Accommodations;

CREATE TABLE dbo.Accommodations
    (
        Accommodation_ID INT PRIMARY KEY IDENTITY(1,1),
        Name VARCHAR(100) NOT NULL,
        Destination_ID INT NOT NULL,
        FOREIGN KEY (Destination_ID) REFERENCES dbo.Destinations(Destination_ID),
        Type_of_Accommodation INT NOT NULL,
        FOREIGN KEY (Type_of_Accommodation) REFERENCES dbo.Types_of_Accommodation(Accommodation_Type_ID),
        Category VARCHAR(10),
        Address VARCHAR(255),
        Facilities TEXT,
        Rating DECIMAL(4, 2) CHECK (Rating >= 0 AND Rating <= 10),
        Description TEXT,
        Date_Created DATETIME DEFAULT GETDATE(),
        Date_Modified DATETIME DEFAULT GETDATE()
    );

-------------------------------------------------------------------------------------


IF EXISTS (
    SELECT 1
    FROM sys.objects
    WHERE object_id = OBJECT_ID(N'dbo.Offers')
      AND type = 'U'
)
    DROP TABLE dbo.Offers;

CREATE TABLE dbo.Offers
    (
        Offer_ID INT PRIMARY KEY IDENTITY(1,1),
        Name VARCHAR(150) NOT NULL,
        Destination_ID INT NOT NULL,
        FOREIGN KEY (Destination_ID) REFERENCES dbo.Destinations(Destination_ID),
        Accommodation_ID INT NOT NULL,
        FOREIGN KEY (Accommodation_ID) REFERENCES dbo.Accommodations(Accommodation_ID),
        Types_of_Transport_ID INT NOT NULL,
        FOREIGN KEY (Types_of_Transport_ID) REFERENCES dbo.Types_of_Transport(Transport_Type_ID),
        Price_per_Person DECIMAL(10, 2) NOT NULL,
        Duration_Days INT NOT NULL,
        Departure_Date DATE NOT NULL,
        Return_Date DATE NOT NULL,
        Total_Seats INT NOT NULL,
        Reserved_Seats INT NOT NULL DEFAULT 0,
        Included_Services TEXT,
        Description TEXT,
        Status VARCHAR(20) NOT NULL DEFAULT 'active',
        CHECK (Status IN ('active', 'inactive', 'expired')),
        Date_Created DATETIME DEFAULT GETDATE(),
        Date_Modified DATETIME DEFAULT GETDATE()
    );

-------------------------------------------------------------------------------

IF EXISTS (
    SELECT 1
    FROM sys.objects
    WHERE object_id = OBJECT_ID(N'dbo.Reservations')
      AND type = 'U'
)
    DROP TABLE dbo.Reservations;

CREATE TABLE dbo.Reservations
    (
        Reservation_ID INT PRIMARY KEY IDENTITY(1,1),
        User_ID INT NOT NULL,
        FOREIGN KEY (User_ID) REFERENCES dbo.Users(User_ID),
        Offer_ID INT NOT NULL,
        FOREIGN KEY (Offer_ID) REFERENCES dbo.Offers(Offer_ID),
        Number_of_Persons INT NOT NULL,
        Total_Price DECIMAL(10,2) NOT NULL,
        Reservation_Date DATETIME DEFAULT GETDATE(),
        Status VARCHAR(20) NOT NULL,
        CHECK (Status IN ('pending', 'confirmed', 'paid', 'cancelled')),
        Notes TEXT
    );

-------------------------------------------------------------------------------

IF EXISTS (
    SELECT 1
    FROM sys.objects
    WHERE object_id = OBJECT_ID(N'dbo.Reservation_Persons')
      AND type = 'U'
)
    DROP TABLE dbo.Reservation_Persons;

CREATE TABLE dbo.Reservation_Persons
    (
        Reservation_Person_ID INT PRIMARY KEY IDENTITY(1,1),
        Reservation_ID INT NOT NULL,
        FOREIGN KEY (Reservation_ID) REFERENCES dbo.Reservations(Reservation_ID),
        Full_Name VARCHAR(100) NOT NULL,
        CNP VARCHAR(15) NOT NULL,
        Birth_Date DATE NOT NULL,
        Person_Type VARCHAR(20) NOT NULL
    );