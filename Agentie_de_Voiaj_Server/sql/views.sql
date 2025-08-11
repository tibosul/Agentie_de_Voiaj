IF EXISTS (
    SELECT 1
    FROM sys.objects
    WHERE object_id = OBJECT_ID(N'dbo.v_Offers_Complete')
      AND type = 'V'
)
    DROP VIEW dbo.v_Offers_Complete;
GO

CREATE VIEW dbo.v_Offers_Complete AS
SELECT 
    o.Offer_ID, o.Name as Offer_Name, 
    d.Name as Destination, d.Country,
    a.Name as Accommodation_Name, a.Category, a.Rating,
    t.Name as Transport_Type,
    o.Price_per_Person, o.Duration_Days,
    o.Departure_Date, o.Return_Date,
    (o.Total_Seats - o.Reserved_Seats) as Available_Seats,
    o.Status
FROM Offers o
JOIN Destinations d ON o.Destination_ID = d.Destination_ID
JOIN Accommodations a ON o.Accommodation_ID = a.Accommodation_ID  
JOIN Types_of_Transport t ON o.Types_of_Transport_ID = t.Transport_Type_ID
WHERE o.Status = 'active';
GO

-----------------------------------------------------------------------------------

IF EXISTS (
    SELECT 1
    FROM sys.objects
    WHERE object_id = OBJECT_ID(N'dbo.v_User_Reservations')
      AND type = 'V'
)
    DROP VIEW dbo.v_User_Reservations;
GO

CREATE VIEW dbo.v_User_Reservations AS
SELECT 
    r.Reservation_ID, u.Username,
    o.Name as Offer_Name, d.Name as Destination,
    r.Number_of_Persons, r.Total_Price, r.Status,
    r.Reservation_Date
FROM Reservations r
JOIN Users u ON r.User_ID = u.User_ID
JOIN Offers o ON r.Offer_ID = o.Offer_ID
JOIN Destinations d ON o.Destination_ID = d.Destination_ID
JOIN Accommodations a ON o.Accommodation_ID = a.Accommodation_ID
JOIN Types_of_Transport t ON o.Types_of_Transport_ID = t.Transport_Type_ID
WHERE o.Status = 'active';
GO