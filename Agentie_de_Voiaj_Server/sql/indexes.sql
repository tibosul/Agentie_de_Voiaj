-- Pentru căutări după destinație (cel mai frecvent)
IF EXISTS (
    SELECT 1
    FROM sys.indexes
    WHERE name = 'IX_Offers_Destination'
)
    DROP INDEX IX_Offers_Destination ON Offers;

CREATE INDEX IX_Offers_Destination ON Offers(Destination_ID);

-- Pentru căutări după preț (filtrare)
IF EXISTS (
    SELECT 1
    FROM sys.indexes
    WHERE name = 'IX_Offers_Price'
)
    DROP INDEX IX_Offers_Price ON Offers;

CREATE INDEX IX_Offers_Price ON Offers(Price_per_Person);

-- Pentru login rapid
IF EXISTS (
    SELECT 1
    FROM sys.indexes
    WHERE name = 'IX_Users_Username'
)
    DROP INDEX IX_Users_Username ON Users;

CREATE INDEX IX_Users_Username ON Users(Username);

-- Pentru rezervările unui user
IF EXISTS (
    SELECT 1
    FROM sys.indexes
    WHERE name = 'IX_Reservations_User'
)
    DROP INDEX IX_Reservations_User ON Reservations;

CREATE INDEX IX_Reservations_User ON Reservations(User_ID);

-- Pentru status-ul ofertelor (doar active)
IF EXISTS (
    SELECT 1
    FROM sys.indexes
    WHERE name = 'IX_Offers_Status'
)
    DROP INDEX IX_Offers_Status ON Offers;

CREATE INDEX IX_Offers_Status ON Offers(Status);

-- Composite index pentru căutări complexe
IF EXISTS (
    SELECT 1
    FROM sys.indexes
    WHERE name = 'IX_Offers_Destination_Price'
)
    DROP INDEX IX_Offers_Destination_Price ON Offers;

CREATE INDEX IX_Offers_Destination_Price ON Offers(Destination_ID, Price_per_Person);