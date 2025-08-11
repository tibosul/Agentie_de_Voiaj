-- Să nu poți rezerva în trecut
ALTER TABLE Offers ADD CONSTRAINT CK_Departure_Future 
CHECK (Departure_Date >= CAST(GETDATE() AS DATE));

-- Numărul de persoane să fie pozitiv
ALTER TABLE Reservations ADD CONSTRAINT CK_Persons_Positive 
CHECK (Number_of_Persons > 0);

-- Prețul să fie pozitiv
ALTER TABLE Offers ADD CONSTRAINT CK_Price_Positive 
CHECK (Price_per_Person > 0);