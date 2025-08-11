IF EXISTS (
    SELECT 1
    FROM sys.objects
    WHERE object_id = OBJECT_ID(N'dbo.tr_Update_Reserved_Seats')
      AND type = 'TR'
)
    DROP TRIGGER dbo.tr_Update_Reserved_Seats;
GO

CREATE TRIGGER tr_Update_Reserved_Seats
ON Reservations AFTER INSERT
AS
BEGIN
    UPDATE Offers 
    SET Reserved_Seats = Reserved_Seats + i.Number_of_Persons
    FROM Offers o
    INNER JOIN inserted i ON o.Offer_ID = i.Offer_ID
END
GO

----------------------------------------------------------------------------------

IF EXISTS (
    SELECT 1
    FROM sys.objects
    WHERE object_id = OBJECT_ID(N'dbo.tr_Update_Seats_on_Cancellation')
      AND type = 'TR'
)
    DROP TRIGGER dbo.tr_Update_Seats_on_Cancellation;
GO

CREATE TRIGGER tr_Update_Seats_on_Cancellation
ON Reservations 
AFTER UPDATE
AS
BEGIN
    -- Când status se schimbă în 'cancelled', eliberează locurile
    IF UPDATE(Status)
    BEGIN
        UPDATE Offers 
        SET Reserved_Seats = Reserved_Seats - d.Number_of_Persons
        FROM Offers o
        INNER JOIN deleted d ON o.Offer_ID = d.Offer_ID
        INNER JOIN inserted i ON d.Reservation_ID = i.Reservation_ID
        WHERE d.Status != 'cancelled' AND i.Status = 'cancelled'
    END
END
GO