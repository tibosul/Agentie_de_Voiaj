IF EXISTS (
    SELECT 1
    FROM sys.objects
    WHERE object_id = OBJECT_ID(N'sp_Get_Destination_Stats')
      AND type = 'P'
)
    DROP PROCEDURE sp_Get_Destination_Stats;
GO

CREATE PROCEDURE sp_Get_Destination_Stats
AS
BEGIN
    SELECT 
        d.Name as Destination,
        COUNT(o.Offer_ID) as Total_Offers,
        AVG(o.Price_per_Person) as Avg_Price,
        SUM(r.Number_of_Persons) as Total_Reservations
    FROM Destinations d
    LEFT JOIN Offers o ON d.Destination_ID = o.Destination_ID
    LEFT JOIN Reservations r ON o.Offer_ID = r.Offer_ID AND r.Status != 'cancelled'
    GROUP BY d.Destination_ID, d.Name
    ORDER BY Total_Reservations DESC
END
GO