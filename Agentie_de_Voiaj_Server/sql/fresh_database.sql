-- ========================================
-- Fresh Database Creation Script
-- ========================================
-- This script will:
-- 1. Check if Agentie_de_Voiaj database exists
-- 2. Drop it if it exists (with proper cleanup)
-- 3. Create a fresh database
-- 4. Set it as the current database
-- ========================================

USE master;
GO

-- Check if database exists and drop it
IF EXISTS (SELECT name FROM sys.databases WHERE name = 'Agentie_de_Voiaj')
BEGIN
    PRINT 'Database Agentie_de_Voiaj exists. Dropping it...';
    
    -- Close all connections to the database
    ALTER DATABASE Agentie_de_Voiaj SET SINGLE_USER WITH ROLLBACK IMMEDIATE;
    
    -- Drop the database
    DROP DATABASE Agentie_de_Voiaj;
    
    PRINT 'Database Agentie_de_Voiaj dropped successfully.';
END
ELSE
BEGIN
    PRINT 'Database Agentie_de_Voiaj does not exist.';
END

-- Create fresh database
PRINT 'Creating fresh database Agentie_de_Voiaj...';
CREATE DATABASE Agentie_de_Voiaj;
GO

-- Set the new database as current and create tables
USE Agentie_de_Voiaj;
GO

-- Ensure dbo schema exists and is set as default
IF NOT EXISTS (SELECT * FROM sys.schemas WHERE name = 'dbo')
BEGIN
    EXEC('CREATE SCHEMA dbo');
    PRINT 'Created dbo schema';
END
GO

-- Set dbo as default schema for current user
EXEC('ALTER USER [dbo] DEFAULT_SCHEMA = dbo');
GO

-- Create tables immediately after database creation
PRINT 'Creating database tables...';
PRINT 'Note: Run setup_database.bat after this to create tables and data';

PRINT '========================================';
PRINT 'Fresh database Agentie_de_Voiaj created!';
PRINT 'Current database: ' + DB_NAME();
PRINT '========================================';
GO

-- Verify database creation
IF EXISTS (SELECT name FROM sys.databases WHERE name = 'Agentie_de_Voiaj')
BEGIN
    PRINT '✅ Database creation verified successfully!';
    PRINT 'Ready for table creation scripts.';
END
ELSE
BEGIN
    PRINT '❌ ERROR: Database creation failed!';
END
GO
