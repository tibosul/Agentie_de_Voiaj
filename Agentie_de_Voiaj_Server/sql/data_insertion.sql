-- Clear existing data (in correct order due to foreign keys)
DELETE FROM Reservation_Persons;
DELETE FROM Reservations;
DELETE FROM Offers;
DELETE FROM Accommodations;
DELETE FROM Types_of_Accommodation;
DELETE FROM Types_of_Transport;
DELETE FROM Destinations;
DELETE FROM Users;

-------------------------------------------------------------

-- Reset identity columns
DBCC CHECKIDENT ('Reservation_Persons', RESEED, 0);
DBCC CHECKIDENT ('Reservations', RESEED, 0);
DBCC CHECKIDENT ('Offers', RESEED, 0);
DBCC CHECKIDENT ('Accommodations', RESEED, 0);
DBCC CHECKIDENT ('Types_of_Accommodation', RESEED, 0);
DBCC CHECKIDENT ('Types_of_Transport', RESEED, 0);
DBCC CHECKIDENT ('Destinations', RESEED, 0);
DBCC CHECKIDENT ('Users', RESEED, 0);

--------------------------------------------------------------

-- ======================================
-- 1. USERS (Sample test users)
-- ======================================
INSERT INTO Users (Username, Password_Hash, Email, First_Name, Last_Name, Phone) VALUES
('admin', 'hashed_admin_pass', 'admin@agency.com', 'Admin', 'System', '0721000000'),
('john_doe', 'hashed_pass_1', 'john.doe@email.com', 'John', 'Doe', '0721111111'),
('mary_smith', 'hashed_pass_2', 'mary.smith@gmail.com', 'Mary', 'Smith', '0722222222'),
('robert_jones', 'hashed_pass_3', 'robert.jones@yahoo.com', 'Robert', 'Jones', '0723333333'),
('sarah_wilson', 'hashed_pass_4', 'sarah.wilson@outlook.com', 'Sarah', 'Wilson', '0724444444'),
('test_user', 'hashed_test_pass', 'test@test.com', 'Test', 'User', '0725555555');

-- ======================================
-- 2. DESTINATIONS
-- ======================================
INSERT INTO Destinations (Name, Country, Description, Image_Path) VALUES
('Paris', 'France', 'City of lights with Eiffel Tower, Louvre and unique romantic atmosphere', '/images/destinations/paris.jpg'),
('Rome', 'Italy', 'The eternal city with Colosseum, Vatican and traditional Italian cuisine', '/images/destinations/rome.jpg'),
('Barcelona', 'Spain', 'Vibrant city with Gaudi architecture, beaches and lively nightlife', '/images/destinations/barcelona.jpg'),
('London', 'United Kingdom', 'British capital with Big Ben, British Museum and traditional culture', '/images/destinations/london.jpg'),
('Amsterdam', 'Netherlands', 'City of canals, museums and bicycles', '/images/destinations/amsterdam.jpg'),
('Prague', 'Czech Republic', 'Golden city with medieval architecture and excellent beer', '/images/destinations/prague.jpg'),
('Vienna', 'Austria', 'City of classical music and imperial palaces', '/images/destinations/vienna.jpg'),
('Budapest', 'Hungary', 'Pearl of Danube with thermal baths and magnificent architecture', '/images/destinations/budapest.jpg'),
('Berlin', 'Germany', 'Modern capital with rich history and vibrant cultural scene', '/images/destinations/berlin.jpg'),
('Venice', 'Italy', 'Floating city with romantic canals and unique architecture', '/images/destinations/venice.jpg');

-- ======================================
-- 3. TYPES OF TRANSPORT
-- ======================================
INSERT INTO Types_of_Transport (Name, Description) VALUES
('Airplane', 'Fast and comfortable transport, ideal for long distances'),
('Train', 'Relaxing transport with beautiful scenery along the way'),
('Bus', 'Economic transport with stops at tourist attractions'),
('Private Car', 'Maximum flexibility in trip planning'),
('Cruise Ship', 'Perfect combination of transport and luxury accommodation'),
('Ferry', 'Maritime transport with scenic sea views');

-- ======================================
-- 4. TYPES OF ACCOMMODATION
-- ======================================
INSERT INTO Types_of_Accommodation (Name, Description) VALUES
('Hotel', 'Standard accommodation with complete services'),
('Guesthouse', 'Family accommodation with intimate atmosphere'),
('Hostel', 'Economic accommodation for young travelers'),
('Apartment', 'Private space with kitchen and living room'),
('Resort', 'Luxury complex with multiple facilities'),
('Boutique Hotel', 'Design hotel with unique character');

-- ======================================
-- 5. ACCOMMODATIONS (Concrete accommodations)
-- ======================================

-- PARIS (ID: 1)
INSERT INTO Accommodations (Name, Destination_ID, Type_of_Accommodation, Category, Address, Facilities, Rating, Description) VALUES
('Hotel Le Meurice', 1, 1, '5*', '228 Rue de Rivoli, Paris', 'Spa, Michelin star restaurant, Concierge, 24h Room Service, Free WiFi', 9.5, 'Luxury hotel in the heart of Paris with Tuileries view'),
('Hotel Ibis Paris Centre', 1, 1, '3*', '15 Rue de la Republique, Paris', 'Free WiFi, Breakfast, 24h Reception, Air Conditioning', 7.8, 'Modern hotel in central Paris, close to attractions'),
('Hostel MIJE Fourcy', 1, 3, '-', '6 Rue de Fourcy, Marais, Paris', 'Free WiFi, Shared Kitchen, Laundry, Luggage Storage', 6.5, 'Hostel in historic Marais district'),
('Champs Elysees Apartment', 1, 4, '-', '45 Avenue des Champs-Elysees, Paris', 'Full Kitchen, Living Room, WiFi, Arc de Triomphe view', 8.2, 'Elegant apartment on the famous avenue'),
('Le Bristol Paris', 1, 5, '5*', '112 Rue du Faubourg Saint-Honore, Paris', 'Spa, Pool, Michelin restaurants, Garden, Concierge', 9.7, 'Palace hotel with exceptional luxury services');

-- ROME (ID: 2)
INSERT INTO Accommodations (Name, Destination_ID, Type_of_Accommodation, Category, Address, Facilities, Rating, Description) VALUES
('Hotel Hassler Roma', 2, 1, '5*', 'Piazza Trinita dei Monti, Rome', 'Spa, Rooftop restaurant, Concierge, Free WiFi, Fitness center', 9.3, 'Luxury hotel overlooking Spanish Steps'),
('Hotel Artemide', 2, 1, '4*', 'Via Nazionale 22, Rome', 'Spa, Restaurant, Free WiFi, Business center', 8.1, 'Elegant hotel near Termini Station'),
('The RomeHello', 2, 3, '-', 'Via Palestro 49, Rome', 'Free WiFi, Shared kitchen, Bar, Luggage storage', 7.2, 'Modern hostel near main attractions'),
('Vatican Apartment', 2, 4, '-', 'Via Crescenzio 45, Rome', 'Kitchen, Living room, WiFi, Vatican view', 8.5, 'Cozy apartment near Vatican City'),
('Rome Cavalieri Waldorf Astoria', 2, 5, '5*', 'Via Alberto Cadlolo 101, Rome', 'Spa, Pool, Michelin restaurant, Garden, Art collection', 9.1, 'Resort-style luxury hotel with panoramic views');

-- BARCELONA (ID: 3)
INSERT INTO Accommodations (Name, Destination_ID, Type_of_Accommodation, Category, Address, Facilities, Rating, Description) VALUES
('Hotel Arts Barcelona', 3, 1, '5*', 'Carrer de la Marina 19-21, Barcelona', 'Spa, Pool, Beach access, Michelin restaurant, Free WiFi', 9.2, 'Iconic skyscraper hotel by the beach'),
('Hotel Barcelona Center', 3, 1, '4*', 'Carrer de Balmes 103, Barcelona', 'Restaurant, Free WiFi, Fitness center, Business center', 8.3, 'Central hotel near Passeig de Gracia'),
('Generator Barcelona', 3, 3, '-', 'Carrer de Corsega 377, Barcelona', 'Free WiFi, Bar, Shared kitchen, Rooftop terrace', 7.8, 'Stylish hostel in trendy neighborhood'),
('Gothic Quarter Apartment', 3, 4, '-', 'Carrer del Call 4, Barcelona', 'Kitchen, Living room, WiFi, Historic location', 8.0, 'Charming apartment in medieval quarter'),
('Majestic Hotel & Spa Barcelona', 3, 5, '5*', 'Passeig de Gracia 68, Barcelona', 'Spa, Pool, Restaurant, Fitness, Concierge', 9.0, 'Grand hotel on famous shopping street');

-- LONDON (ID: 4)
INSERT INTO Accommodations (Name, Destination_ID, Type_of_Accommodation, Category, Address, Facilities, Rating, Description) VALUES
('The Ritz London', 4, 1, '5*', '150 Piccadilly, London', 'Spa, Restaurant, Afternoon tea, Concierge, Free WiFi', 9.4, 'Legendary luxury hotel in Mayfair'),
('Premier Inn London City', 4, 1, '3*', '1 Harrow Pl, London', 'Restaurant, Free WiFi, Business center', 7.9, 'Budget-friendly hotel in the City'),
('YHA London Central', 4, 3, '-', '104 Bolsover St, London', 'Shared kitchen, Common room, WiFi, Luggage storage', 6.8, 'Central hostel near Oxford Circus'),
('Covent Garden Apartment', 4, 4, '-', '12 King Street, Covent Garden, London', 'Kitchen, Living room, WiFi, Theater district', 8.4, 'Stylish apartment in entertainment district'),
('Claridges', 4, 5, '5*', 'Brook St, Mayfair, London', 'Spa, Restaurant, Art Deco design, Concierge', 9.6, 'Art Deco masterpiece in Mayfair');

-- AMSTERDAM (ID: 5)
INSERT INTO Accommodations (Name, Destination_ID, Type_of_Accommodation, Category, Address, Facilities, Rating, Description) VALUES
('Waldorf Astoria Amsterdam', 5, 1, '5*', 'Herengracht 542-556, Amsterdam', 'Spa, Restaurant, Canal views, Concierge, Free WiFi', 9.1, 'Luxury hotel on historic canal'),
('Hotel V Nesplein', 5, 1, '4*', 'Nes 49, Amsterdam', 'Restaurant, Bar, Free WiFi, Fitness center', 8.2, 'Trendy hotel near Red Light District'),
('ClinkNOORD Hostel', 5, 3, '-', 'Badhuiskade 3, Amsterdam', 'Free WiFi, Bar, Shared kitchen, Bike rental', 7.5, 'Industrial-style hostel across from Central Station'),
('Canal Ring Apartment', 5, 4, '-', 'Prinsengracht 127, Amsterdam', 'Kitchen, Living room, WiFi, Canal view', 8.3, 'Historic apartment on famous canal'),
('Conservatorium Hotel', 5, 6, '5*', 'Van Baerlestraat 27, Amsterdam', 'Restaurant, Bar, Modern design, Museum quarter', 9.3, 'Contemporary design hotel near museums');

-- ======================================
-- 6. OFFERS (Travel packages)
-- ======================================

-- PARIS OFFERS
INSERT INTO Offers (Name, Destination_ID, Accommodation_ID, Types_of_Transport_ID, Price_per_Person, Duration_Days, Departure_Date, Return_Date, Total_Seats, Reserved_Seats, Included_Services, Description, Status) VALUES
('Paris Luxury Experience', 1, 1, 1, 1850.00, 5, '2025-09-15', '2025-09-20', 20, 0, 'Breakfast, Airport transfer, City tour, Louvre tickets', 'Exclusive 5-day luxury Paris experience with premium accommodation', 'active'),
('Paris City Break', 1, 2, 1, 720.00, 3, '2025-10-01', '2025-10-04', 40, 5, 'Breakfast, Airport transfer', 'Perfect weekend getaway to the City of Lights', 'active'),
('Paris Budget Adventure', 1, 3, 3, 350.00, 4, '2025-09-20', '2025-09-24', 35, 8, 'Bus transfer, Walking tour', 'Affordable Paris discovery for young travelers', 'active'),
('Paris Apartment Stay', 1, 4, 2, 980.00, 6, '2025-11-10', '2025-11-16', 8, 2, 'Train tickets, Metro pass, Local guide', 'Independent stay in elegant Champs Elysees apartment', 'active'),
('Paris Palace Week', 1, 5, 1, 2400.00, 7, '2025-12-01', '2025-12-08', 15, 0, 'All meals, Spa access, Private tours, Transfers', 'Ultimate luxury week at Le Bristol Palace', 'active');

-- ROME OFFERS
INSERT INTO Offers (Name, Destination_ID, Accommodation_ID, Types_of_Transport_ID, Price_per_Person, Duration_Days, Departure_Date, Return_Date, Total_Seats, Reserved_Seats, Included_Services, Description, Status) VALUES
('Rome Imperial Tour', 2, 6, 1, 1650.00, 5, '2025-09-25', '2025-09-30', 25, 3, 'Breakfast, Colosseum VIP tour, Vatican tour, Transfers', 'Discover the eternal city with exclusive access', 'active'),
('Rome Classic', 2, 7, 1, 890.00, 4, '2025-10-15', '2025-10-19', 30, 7, 'Breakfast, City tour, Airport transfer', 'Essential Rome experience with central accommodation', 'active'),
('Rome Backpacker Special', 2, 8, 3, 420.00, 5, '2025-09-30', '2025-10-05', 28, 12, 'Bus transfer, Walking tours, Breakfast', 'Budget-friendly Roman adventure', 'active'),
('Rome Vatican Apartment', 2, 9, 2, 1150.00, 6, '2025-11-20', '2025-11-26', 6, 1, 'Train tickets, Vatican tour, Local guide', 'Stay near Vatican with apartment comfort', 'active'),
('Rome Resort Luxury', 2, 10, 1, 2100.00, 7, '2025-12-15', '2025-12-22', 18, 0, 'All meals, Spa, Art tours, Private transfers', 'Luxury resort experience in Rome', 'active');

-- BARCELONA OFFERS
INSERT INTO Offers (Name, Destination_ID, Accommodation_ID, Types_of_Transport_ID, Price_per_Person, Duration_Days, Departure_Date, Return_Date, Total_Seats, Reserved_Seats, Included_Services, Description, Status) VALUES
('Barcelona Beach & Culture', 3, 11, 1, 1420.00, 5, '2025-10-05', '2025-10-10', 22, 6, 'Breakfast, Sagrada Familia tour, Beach access, Transfers', 'Perfect mix of culture and relaxation', 'active'),
('Barcelona City Explorer', 3, 12, 1, 950.00, 4, '2025-10-20', '2025-10-24', 35, 9, 'Breakfast, Park Guell tour, Metro pass', 'Explore Gaudi masterpieces and Catalan culture', 'active'),
('Barcelona Youth Adventure', 3, 13, 3, 380.00, 4, '2025-10-10', '2025-10-14', 32, 15, 'Bus transfer, Walking tours, Hostel breakfast', 'Young travelers Barcelona discovery', 'active'),
('Barcelona Gothic Experience', 3, 14, 2, 820.00, 5, '2025-11-05', '2025-11-10', 10, 3, 'Train tickets, Gothic quarter tour, Tapas tour', 'Historic Barcelona from medieval apartment', 'active'),
('Barcelona Luxury Escape', 3, 15, 1, 1980.00, 6, '2025-12-20', '2025-12-26', 16, 0, 'All meals, Spa, Private tours, Shopping tour', 'Luxury Barcelona with premium services', 'active');

-- LONDON OFFERS
INSERT INTO Offers (Name, Destination_ID, Accommodation_ID, Types_of_Transport_ID, Price_per_Person, Duration_Days, Departure_Date, Return_Date, Total_Seats, Reserved_Seats, Included_Services, Description, Status) VALUES
('London Royal Experience', 4, 16, 1, 2200.00, 5, '2025-09-18', '2025-09-23', 18, 2, 'Afternoon tea, Tower of London VIP, Theater tickets, Transfers', 'Royal treatment in iconic London', 'active'),
('London Business Trip', 4, 17, 1, 780.00, 3, '2025-10-25', '2025-10-28', 45, 11, 'Breakfast, Oyster card, Airport transfer', 'Efficient city break for business or leisure', 'active'),
('London Budget Explorer', 4, 18, 3, 450.00, 4, '2025-10-15', '2025-10-19', 30, 18, 'Bus transfer, Walking tours, Hostel breakfast', 'Affordable London discovery', 'active'),
('London Theater District', 4, 19, 6, 1100.00, 5, '2025-11-15', '2025-11-20', 12, 4, 'Ferry crossing, Theater tickets, Local guide', 'West End theater experience with scenic arrival', 'active'),
('London Luxury Weekend', 4, 20, 1, 2850.00, 4, '2025-12-10', '2025-12-14', 12, 0, 'All meals, Spa, Private tours, Shopping assistant', 'Ultimate luxury London experience', 'active');

-- AMSTERDAM OFFERS
INSERT INTO Offers (Name, Destination_ID, Accommodation_ID, Types_of_Transport_ID, Price_per_Person, Duration_Days, Departure_Date, Return_Date, Total_Seats, Reserved_Seats, Included_Services, Description, Status) VALUES
('Amsterdam Canal Cruise Luxury', 5, 21, 1, 1680.00, 4, '2025-09-28', '2025-10-02', 20, 4, 'Canal cruise, Van Gogh Museum, Breakfast, Transfers', 'Luxury canal-side experience', 'active'),
('Amsterdam Art & Culture', 5, 22, 2, 850.00, 4, '2025-10-30', '2025-11-03', 28, 8, 'Museum passes, Bike rental, Breakfast', 'Cultural immersion in artistic Amsterdam', 'active'),
('Amsterdam Backpacker Fun', 5, 23, 3, 320.00, 3, '2025-10-20', '2025-10-23', 25, 12, 'Bus transfer, Bike tour, Hostel breakfast', 'Budget Amsterdam adventure', 'active'),
('Amsterdam Canal House', 5, 24, 2, 920.00, 5, '2025-11-25', '2025-11-30', 8, 2, 'Train tickets, Canal tour, Bike rental', 'Authentic Amsterdam in historic canal house', 'active'),
('Amsterdam Design Weekend', 5, 25, 1, 1450.00, 3, '2025-12-05', '2025-12-08', 15, 0, 'Design tours, Museum access, Breakfast', 'Contemporary Amsterdam in design hotel', 'active');

-- ======================================
-- 7. SAMPLE RESERVATIONS
-- ======================================
INSERT INTO Reservations (User_ID, Offer_ID, Number_of_Persons, Total_Price, Status, Notes) VALUES
(2, 1, 2, 3700.00, 'confirmed', 'Anniversary trip, requested room with Eiffel Tower view'),
(3, 3, 1, 350.00, 'paid', 'Solo traveler, first time in Paris'),
(4, 7, 2, 1780.00, 'confirmed', 'Honeymoon trip to Rome'),
(5, 11, 3, 4260.00, 'pending', 'Family trip with teenage daughter'),
(2, 18, 1, 450.00, 'paid', 'Solo business trip to London'),
(6, 21, 2, 3360.00, 'confirmed', 'Couple celebrating 10th anniversary');

-- ======================================
-- 8. RESERVATION PERSONS
-- ======================================
INSERT INTO Reservation_Persons (Reservation_ID, Full_Name, CNP, Birth_Date, Person_Type) VALUES
-- Reservation 1: Paris Luxury (2 persons)
(1, 'John Doe', '1801023123456', '1980-01-02', 'adult'),
(1, 'Jane Doe', '2851215234567', '1985-12-15', 'adult'),

-- Reservation 2: Paris Budget (1 person)
(2, 'Mary Smith', '2920308345678', '1992-03-08', 'adult'),

-- Reservation 3: Rome Imperial (2 persons)
(3, 'Robert Jones', '1750612456789', '1975-06-12', 'adult'),
(3, 'Linda Jones', '2780924567890', '1978-09-24', 'adult'),

-- Reservation 4: Barcelona Beach (3 persons)
(4, 'Sarah Wilson', '2870515678901', '1987-05-15', 'adult'),
(4, 'Michael Wilson', '1820701789012', '1982-07-01', 'adult'),
(4, 'Emma Wilson', '3060318890123', '2006-03-18', 'child'),

-- Reservation 5: London Budget (1 person)
(5, 'John Doe', '1801023123456', '1980-01-02', 'adult'),

-- Reservation 6: Amsterdam Canal (2 persons)
(6, 'Test User', '1901010901234', '1990-01-01', 'adult'),
(6, 'Test Partner', '2881225012345', '1988-12-25', 'adult');