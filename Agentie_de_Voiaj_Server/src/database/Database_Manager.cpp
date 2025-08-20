#include "database/Database_Manager.h"
#include <iostream>
#include <algorithm>
#include <cctype>

using namespace Utils::Exceptions;

// Constructor
Database::Database_Manager::Database_Manager() 
    : is_connected(false), is_demo_mode(false)
{
    initialize_handles();
}

Database::Database_Manager::Database_Manager(const std::string& server, const std::string& database, 
    const std::string& username, const std::string& password)
    : server(server), database(database), username(username), password(password),
    is_connected(false), is_demo_mode(false)
{
    // Check if this is a dummy instance (demo mode)
    if (server == "dummy" && database == "dummy")
    {
        is_demo_mode = true;
        Utils::Logger::warning("Database_Manager initialized in DEMO MODE - using mock data");
        return;
    }
    
    initialize_handles();
    connection_string = build_connection_string();
}

// Destructor
Database::Database_Manager::~Database_Manager()
{
    disconnect();
    cleanup_handles();
}

// Initialize Qt SQL database connection
bool Database::Database_Manager::initialize_handles()
{
    // Generate unique connection name
    static int connection_counter = 0;
    connection_name = QString("AgentieVoiajConnection_%1").arg(++connection_counter);
    
    // Add SQL Server driver
    db = QSqlDatabase::addDatabase("QODBC", connection_name);
    
    if (!db.isValid())
    {
        log_error("initialize_handles", "Failed to add ODBC database driver");
        return false;
    }
    
    return true;
}

// Cleanup Qt SQL database connection
void Database::Database_Manager::cleanup_handles()
{
    if (db.isOpen())
    {
        db.close();
    }
    
    if (QSqlDatabase::contains(connection_name))
    {
        QSqlDatabase::removeDatabase(connection_name);
    }
}

// Build connection string
std::string Database::Database_Manager::build_connection_string() const
{
    std::stringstream ss;
    ss << "DRIVER={ODBC Driver 17 for SQL Server};"
       << "SERVER=" << server << ";";
    
    // Use Windows Authentication if username/password are empty
    if (username.empty() && password.empty())
    {
        ss << "DATABASE=" << database << ";"
           << "Trusted_Connection=yes;"
           << "Connection Timeout=30;";
    }
    else
    {
        ss << "DATABASE=" << database << ";"
           << "UID=" << username << ";"
           << "PWD=" << password << ";"
           << "Trusted_Connection=no;"
           << "Connection Timeout=30;";
    }
    
    return ss.str();
}

// Connection methods
bool Database::Database_Manager::connect()
{
    std::lock_guard<std::mutex> lock(db_mutex);
    
    if (is_connected)
    {
        return true;
    }

    if (connection_string.empty())
    {
        log_error("connect", "Connection string is empty");
        return false;
    }

    // Set connection string for Qt SQL
    db.setDatabaseName(QString::fromStdString(connection_string));

    if (db.open())
    {
        is_connected = true;
        Utils::Logger::info("Database connection successful to: " + server + "\\" + database);
        return true;
    }
    else
    {
        QSqlError error = db.lastError();
        std::string error_msg = get_sql_error(error);
        std::string detailed_error = "Connection failed to " + server + "\\" + database + ": " + error_msg;
        Utils::Logger::error(detailed_error);
        throw DatabaseException(detailed_error, error.nativeErrorCode().toInt());
    }
}

bool Database::Database_Manager::connect(const std::string& server, const std::string& database,
    const std::string& username, const std::string& password)
{
    set_configuration_params(server, database, username, password);
    return connect();
}

bool Database::Database_Manager::disconnect()
{
    std::lock_guard<std::mutex> lock(db_mutex);
    
    if (!is_connected)
    {
        return true;
    }

    db.close();
    is_connected = false;
    
    return !db.isOpen();
}

bool Database::Database_Manager::is_connection_alive() const
{
    return is_connected && db.isOpen() && db.isValid();
}

bool Database::Database_Manager::database_exists() const
{
    if (!is_connected) return false;
    
    try
    {
        // Try to query the database to see if it exists
        QSqlQuery query(db);
        bool success = query.exec("SELECT 1");
        return success;
    }
    catch (...)
    {
        return false;
    }
}

bool Database::Database_Manager::reconnect()
{
    disconnect();
    return connect();
}

void Database::Database_Manager::set_configuration_params(const std::string& server, const std::string& database,
    const std::string& username, const std::string& password)
{
    this->server = server;
    this->database = database;
    this->username = username;
    this->password = password;
    this->connection_string = build_connection_string();
}

std::string Database::Database_Manager::get_connection_string() const
{
    return connection_string;
}

// Query execution methods
Database::Query_Result Database::Database_Manager::execute_query(const std::string& query)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    
    if (!is_connected)
    {
        return Query_Result(Result_Type::ERROR_CONNECTION, "Not connected to database");
    }

    QSqlQuery sqlQuery(db);
    
    if (!sqlQuery.exec(QString::fromStdString(query)))
    {
        QSqlError error = sqlQuery.lastError();
        std::string error_msg = get_sql_error(error);
        throw DatabaseException("Query execution failed: " + error_msg, error.nativeErrorCode().toInt());
    }

    // Check if this is a SELECT query
    std::string upper_query = query;
    std::transform(upper_query.begin(), upper_query.end(), upper_query.begin(), ::toupper);
    
    if (upper_query.find("SELECT") == 0)
    {
        return process_select_result();
    }
    
    if (upper_query.find("SELECT") == 0)
    {
        return process_select_result(sqlQuery);
    }
    else
    {
        return process_execution_result(sqlQuery);
    }
}

Database::Query_Result Database::Database_Manager::execute_select(const std::string& query)
{
    return execute_query(query);
}

Database::Query_Result Database::Database_Manager::execute_insert(const std::string& query)
{
    return execute_query(query);
}

Database::Query_Result Database::Database_Manager::execute_update(const std::string& query)
{
    return execute_query(query);
}

Database::Query_Result Database::Database_Manager::execute_delete(const std::string& query)
{
    return execute_query(query);
}

// Process SELECT result
Database::Query_Result Database::Database_Manager::process_select_result(QSqlQuery& query)
{
    Query_Result result;
    
    if (!query.isActive())
    {
        return Query_Result(Result_Type::ERROR_EXECUTION, "Query is not active");
    }
    
    QSqlRecord record = query.record();
    int columns = record.count();

    // Fetch rows
    while (query.next())
    {
        std::map<std::string, std::string> row;
        
        for (int i = 0; i < columns; i++)
        {
            QString column_name = record.fieldName(i);
            QVariant value = query.value(i);
            row[column_name.toStdString()] = value.toString().toStdString();
        }
        
        result.data.push_back(row);
    }
    
    result.type = Result_Type::SUCCESS;
    result.affected_rows = result.data.size();
    return result;
// Process non-SELECT result
Database::Query_Result Database::Database_Manager::process_execution_result(QSqlQuery& query)
{
    Database::Query_Result result;
    result.affected_rows = query.numRowsAffected();
    result.type = Result_Type::SUCCESS;
    return result;
}

// Error handling
bool Database::Database_Manager::handle_sql_error(const QSqlError& error)
{
    std::string error_msg = get_sql_error(error);
    log_error("SQL Error", error_msg);
    return false;
}

std::string Database::Database_Manager::get_sql_error(const QSqlError& error)
{
    if (!error.isValid())
    {
        return "No error";
    }
    
    std::stringstream ss;
    ss << "Database Error: " << error.databaseText().toStdString();
    if (!error.driverText().isEmpty())
    {
        ss << ", Driver Error: " << error.driverText().toStdString();
    }
    if (!error.nativeErrorCode().isEmpty())
    {
        ss << ", Native Error Code: " << error.nativeErrorCode().toStdString();
    }
    
    return ss.str();
}

std::string Database::Database_Manager::get_last_error()
{
    return get_sql_error(db.lastError());
}

void Database::Database_Manager::log_error(const std::string& operation, const std::string& error)
{
    Utils::Logger::error("[Database] " + operation + ": " + error);
}

// Utility methods
std::string Database::Database_Manager::escape_string(const std::string& input)
{
    return Utils::String::escape_SQL(input);
}

std::string Database::Database_Manager::format_date_for_sql(const std::string& date)
{
    return "'" + date + "'";
}

bool Database::Database_Manager::validate_connection_params()
{
    return !server.empty() && !database.empty() && !username.empty();
}

// Transaction methods
bool Database::Database_Manager::begin_transaction()
{
    std::lock_guard<std::mutex> lock(db_mutex);
    SQLRETURN ret = SQLSetConnectAttr(hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_IS_UINTEGER);
    return SQL_SUCCEEDED(ret);
}

bool Database::Database_Manager::commit_transaction()
{
    std::lock_guard<std::mutex> lock(db_mutex);
    SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    SQLSetConnectAttr(hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, SQL_IS_UINTEGER);
    return SQL_SUCCEEDED(ret);
}

bool Database::Database_Manager::rollback_transaction()
{
    std::lock_guard<std::mutex> lock(db_mutex);
    SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_ROLLBACK);
    SQLSetConnectAttr(hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, SQL_IS_UINTEGER);
    return SQL_SUCCEEDED(ret);
}

Database::Query_Result Database::Database_Manager::execute_transaction(const std::vector<std::string>& queries)
{
    if (!begin_transaction())
    {
        return Query_Result(Result_Type::ERROR_EXECUTION, "Failed to begin transaction");
    }

    for (const auto& query : queries)
    {
        Query_Result result = execute_query(query);
        if (!result.is_success())
        {
            rollback_transaction();
            return result;
        }
    }

    if (!commit_transaction())
    {
        rollback_transaction();
        return Query_Result(Result_Type::ERROR_EXECUTION, "Failed to commit transaction");
    }

    return Query_Result(Result_Type::SUCCESS, "Transaction completed successfully");
}

// User authentication and management
Database::Query_Result Database::Database_Manager::authenticate_user(const std::string& username, const std::string& password)
{
    // Demo mode - return mock authentication
    if (is_demo_mode)
    {
        if (!Utils::Validation::is_valid_username(username) || Utils::String::is_empty(password))
        {
            return Query_Result(Result_Type::ERROR_CONSTRAINT, "Invalid username or password format");
        }
        
        // Mock successful authentication for demo users
        if ((username == "admin" && password == "admin123") || 
            (username == "demo" && password == "demo123") ||
            (username == "test" && password == "test123"))
        {
            Query_Result result(Result_Type::SUCCESS, "Demo authentication successful");
            std::map<std::string, std::string> user_data;
            user_data["ID"] = (username == "admin") ? "1" : "2";
            user_data["Username"] = username;
            user_data["Email"] = username + "@demo.com";
            user_data["First_Name"] = "Demo";
            user_data["Last_Name"] = "User";
            user_data["Phone"] = "0700000000";
            result.data.push_back(user_data);
            
            Utils::Logger::info("DEMO MODE: Authentication successful for demo user: " + username);
            return result;
        }
        else
        {
            Utils::Logger::warning("DEMO MODE: Authentication failed for user: " + username);
            return Query_Result(Result_Type::DB_ERROR_NO_DATA, "Authentication failed - invalid credentials");
        }
    }
    
    // Regular database authentication
    if (!Utils::Validation::is_valid_username(username) || Utils::String::is_empty(password))
    {
        return Query_Result(Result_Type::ERROR_CONSTRAINT, "Invalid username or password format");
    }
    
    // First get the user's salt from database
    std::string salt_query = "SELECT Password_Salt FROM Users WHERE Username = '" + escape_string(username) + "'";
    Query_Result salt_result = execute_select(salt_query);
    if (!salt_result.is_success() || salt_result.data.empty())
    {
        return Query_Result(Result_Type::DB_ERROR_NO_DATA, "Invalid username or password");
    }
    
    std::string stored_salt = salt_result.data[0]["Password_Salt"];
    std::string hashed_password = hash_password(password, stored_salt);
    
    std::string query = "SELECT User_ID, Username, Email, First_Name, Last_Name, Phone FROM Users WHERE Username = '" 
                       + escape_string(username) + "' AND Password_Hash = '" + escape_string(hashed_password) + "'";
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::register_user(const User_Data& user_data)
{
    // Demo mode - return mock registration
    if (is_demo_mode)
    {
        // Basic validation
        if (!Utils::Validation::is_valid_username(user_data.username))
        {
            return Database::Query_Result(Result_Type::ERROR_CONSTRAINT, "Invalid username format");
        }
        if (!Utils::Validation::is_valid_email(user_data.email))
        {
            return Database::Query_Result(Result_Type::ERROR_CONSTRAINT, "Invalid email format");
        }
        if (!Utils::Validation::is_valid_password(user_data.password_hash))
        {
            return Database::Query_Result(Result_Type::ERROR_CONSTRAINT, "Invalid password format");
        }
        
        // Mock successful registration (simulate username already exists for some cases)
        if (user_data.username == "admin" || user_data.username == "test")
        {
            Utils::Logger::warning("DEMO MODE: Registration failed - username already exists: " + user_data.username);
            return Query_Result(Result_Type::ERROR_CONSTRAINT, "Username already exists");
        }
        
        Utils::Logger::info("DEMO MODE: Registration successful for new user: " + user_data.username);
        return Query_Result(Result_Type::SUCCESS, "Demo user registration successful");
    }
    
    // Regular database registration - validate user data
    if (!Utils::Validation::is_valid_username(user_data.username))
    {
        return Database::Query_Result(Result_Type::ERROR_CONSTRAINT, "Invalid username format");
    }
    if (!Utils::Validation::is_valid_email(user_data.email))
    {
        return Database::Query_Result(Result_Type::ERROR_CONSTRAINT, "Invalid email format");
    }
    if (!Utils::Validation::is_valid_password(user_data.password_hash))
    {
        return Database::Query_Result(Result_Type::ERROR_CONSTRAINT, "Invalid password format");
    }
    if (!user_data.phone_number.empty() && !Utils::Validation::is_valid_phone_number(user_data.phone_number))
    {
        return Database::Query_Result(Result_Type::ERROR_CONSTRAINT, "Invalid phone number format");
    }
    
    // Generate a random salt for this user
    std::string salt = Utils::Crypto::generate_salt();
    std::string hashed_password = hash_password(user_data.password_hash, salt);
    
    std::stringstream query;
    query << "INSERT INTO Users (Username, Password_Hash, Password_Salt, Email, First_Name, Last_Name, Phone) VALUES ('"
          << escape_string(user_data.username) << "', '"
          << escape_string(hashed_password) << "', '"
          << escape_string(salt) << "', '"
          << escape_string(user_data.email) << "', '"
          << escape_string(user_data.first_name) << "', '"
          << escape_string(user_data.last_name) << "', '"
          << escape_string(user_data.phone_number) << "')";
    
    return execute_insert(query.str());
}

Database::Query_Result Database::Database_Manager::get_user_by_id(int user_id)
{
    std::string query = "SELECT User_ID, Username, Email, First_Name, Last_Name, Phone, Date_Created, Date_Modified FROM Users WHERE User_ID = " + std::to_string(user_id);
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::get_user_by_username(const std::string& username)
{
    std::string query = "SELECT User_ID, Username, Email, First_Name, Last_Name, Phone, Date_Created, Date_Modified FROM Users WHERE Username = '" + escape_string(username) + "'";
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::update_user(const User_Data& user)
{
    std::stringstream query;
    query << "UPDATE Users SET "
          << "Email = '" << escape_string(user.email) << "', "
          << "First_Name = '" << escape_string(user.first_name) << "', "
          << "Last_Name = '" << escape_string(user.last_name) << "', "
          << "Phone = '" << escape_string(user.phone_number) << "', "
          << "Date_Modified = GETDATE() "
          << "WHERE User_ID = " << user.id;
    
    return execute_update(query.str());
}

Database::Query_Result Database::Database_Manager::delete_user(int user_id)
{
    std::string query = "DELETE FROM Users WHERE User_ID = " + std::to_string(user_id);
    return execute_delete(query);
}

Database::Query_Result Database::Database_Manager::change_password(int user_id, const std::string& old_password, const std::string& new_password)
{
    // First verify old password
    Database::Query_Result user_result = get_user_by_id(user_id);
    if (!user_result.is_success() || user_result.data.empty())
    {
        return Query_Result(Result_Type::DB_ERROR_NO_DATA, "User not found");
    }
    
    // Get current salt to verify old password
    std::string current_salt_query = "SELECT Password_Salt FROM Users WHERE User_ID = " + std::to_string(user_id);
    Query_Result salt_result = execute_select(current_salt_query);
    if (!salt_result.is_success() || salt_result.data.empty())
    {
        return Query_Result(Result_Type::DB_ERROR_NO_DATA, "User salt not found");
    }
    
    std::string current_salt = salt_result.data[0]["Password_Salt"];
    std::string old_hash = hash_password(old_password, current_salt);
    
    std::string verify_query = "SELECT User_ID FROM Users WHERE User_ID = " + std::to_string(user_id) + 
                              " AND Password_Hash = '" + escape_string(old_hash) + "'";
    Query_Result verify_result = execute_select(verify_query);
    
    if (!verify_result.is_success() || verify_result.data.empty())
    {
        return Query_Result(Result_Type::ERROR_EXECUTION, "Invalid old password");
    }
    
    // Generate new salt and hash for new password
    std::string new_salt = Utils::Crypto::generate_salt();
    std::string new_hash = hash_password(new_password, new_salt);
    std::string update_query = "UPDATE Users SET Password_Hash = '" + escape_string(new_hash) + 
                              "', Password_Salt = '" + escape_string(new_salt) +
                              "', Date_Modified = GETDATE() WHERE User_ID = " + std::to_string(user_id);
    
    return execute_update(update_query);
}

// Static utility methods
std::string Database::Database_Manager::hash_password(const std::string& password, const std::string& salt)
{
    return Utils::Crypto::hash_password(password, salt);
}

std::string Database::Database_Manager::generate_salt()
{
    return Utils::Crypto::generate_salt();
}

bool Database::Database_Manager::validate_email(const std::string& email)
{
    return Utils::Validation::is_valid_email(email);
}

bool Database::Database_Manager::validate_cnp(const std::string& cnp)
{
    return Utils::Validation::is_valid_cnp(cnp);
}

// Destination management
Database::Query_Result Database::Database_Manager::get_all_destinations()
{
    std::string query = "SELECT Destination_ID, Name, Country, Description, Image_Path, Date_Created, Date_Modified FROM Destinations ORDER BY Name";
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::get_destination_by_id(int destination_id)
{
    std::string query = "SELECT Destination_ID, Name, Country, Description, Image_Path, Date_Created, Date_Modified FROM Destinations WHERE Destination_ID = " + std::to_string(destination_id);
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::add_destination(const Destination_Data& destination)
{
    std::stringstream query;
    query << "INSERT INTO Destinations (Name, Country, Description, Image_Path) VALUES ('"
          << escape_string(destination.name) << "', '"
          << escape_string(destination.country) << "', '"
          << escape_string(destination.description) << "', '"
          << escape_string(destination.image_path) << "')";
    
    return execute_insert(query.str());
}

Database::Query_Result Database::Database_Manager::update_destination(const Destination_Data& destination)
{
    std::stringstream query;
    query << "UPDATE Destinations SET "
          << "Name = '" << escape_string(destination.name) << "', "
          << "Country = '" << escape_string(destination.country) << "', "
          << "Description = '" << escape_string(destination.description) << "', "
          << "Image_Path = '" << escape_string(destination.image_path) << "', "
          << "Date_Modified = GETDATE() "
          << "WHERE Destination_ID = " << destination.id;
    
    return execute_update(query.str());
}

Database::Query_Result Database::Database_Manager::delete_destination(int destination_id)
{
    std::string query = "DELETE FROM Destinations WHERE Destination_ID = " + std::to_string(destination_id);
    return execute_delete(query);
}

// Transport types management
Database::Query_Result Database::Database_Manager::get_all_transport_types()
{
    std::string query = "SELECT Transport_Type_ID, Name, Description, Date_Created, Date_Modified FROM Types_of_Transport ORDER BY Name";
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::get_transport_type_by_id(int transport_type_id)
{
    std::string query = "SELECT Transport_Type_ID, Name, Description, Date_Created, Date_Modified FROM Types_of_Transport WHERE Transport_Type_ID = " + std::to_string(transport_type_id);
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::add_transport_type(const Transport_Type_Data& transport_type)
{
    std::stringstream query;
    query << "INSERT INTO Types_of_Transport (Name, Description) VALUES ('"
          << escape_string(transport_type.name) << "', '"
          << escape_string(transport_type.description) << "')";
    
    return execute_insert(query.str());
}

Database::Query_Result Database::Database_Manager::update_transport_type(const Transport_Type_Data& transport_type)
{
    std::stringstream query;
    query << "UPDATE Types_of_Transport SET "
          << "Name = '" << escape_string(transport_type.name) << "', "
          << "Description = '" << escape_string(transport_type.description) << "', "
          << "Date_Modified = GETDATE() "
          << "WHERE Transport_Type_ID = " << transport_type.id;
    
    return execute_update(query.str());
}

Database::Query_Result Database::Database_Manager::delete_transport_type(int transport_type_id)
{
    std::string query = "DELETE FROM Types_of_Transport WHERE Transport_Type_ID = " + std::to_string(transport_type_id);
    return execute_delete(query);
}

// Accommodation types management
Database::Query_Result Database::Database_Manager::get_all_accommodation_types()
{
    std::string query = "SELECT Accommodation_Type_ID, Name, Description, Date_Created, Date_Modified FROM Types_of_Accommodation ORDER BY Name";
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::get_accommodation_type_by_id(int accommodation_type_id)
{
    std::string query = "SELECT Accommodation_Type_ID, Name, Description, Date_Created, Date_Modified FROM Types_of_Accommodation WHERE Accommodation_Type_ID = " + std::to_string(accommodation_type_id);
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::add_accommodation_type(const Accommodation_Type_Data& accommodation_type)
{
    std::stringstream query;
    query << "INSERT INTO Types_of_Accommodation (Name, Description) VALUES ('"
          << escape_string(accommodation_type.name) << "', '"
          << escape_string(accommodation_type.description) << "')";
    
    return execute_insert(query.str());
}

Database::Query_Result Database::Database_Manager::update_accommodation_type(const Accommodation_Type_Data& accommodation_type)
{
    std::stringstream query;
    query << "UPDATE Types_of_Accommodation SET "
          << "Name = '" << escape_string(accommodation_type.name) << "', "
          << "Description = '" << escape_string(accommodation_type.description) << "', "
          << "Date_Modified = GETDATE() "
          << "WHERE Accommodation_Type_ID = " << accommodation_type.id;
    
    return execute_update(query.str());
}

Database::Query_Result Database::Database_Manager::delete_accommodation_type(int accommodation_type_id)
{
    std::string query = "DELETE FROM Types_of_Accommodation WHERE Accommodation_Type_ID = " + std::to_string(accommodation_type_id);
    return execute_delete(query);
}

// Accommodation management
Database::Query_Result Database::Database_Manager::get_accommodations_by_destination(int destination_id)
{
    std::string query = "SELECT a.Accommodation_ID, a.Name, a.Destination_ID, a.Type_of_Accommodation, "
                       "a.Category, a.Address, a.Facilities, a.Rating, a.Description, a.Date_Created, a.Date_Modified, "
                       "at.Name as Type_Name FROM Accommodations a "
                       "LEFT JOIN Types_of_Accommodation at ON a.Type_of_Accommodation = at.Accommodation_Type_ID "
                       "WHERE a.Destination_ID = " + std::to_string(destination_id) + " ORDER BY a.Name";
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::get_accommodation_by_id(int accommodation_id)
{
    std::string query = "SELECT a.Accommodation_ID, a.Name, a.Destination_ID, a.Type_of_Accommodation, "
                       "a.Category, a.Address, a.Facilities, a.Rating, a.Description, a.Date_Created, a.Date_Modified, "
                       "at.Name as Type_Name FROM Accommodations a "
                       "LEFT JOIN Types_of_Accommodation at ON a.Type_of_Accommodation = at.Accommodation_Type_ID "
                       "WHERE a.Accommodation_ID = " + std::to_string(accommodation_id);
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::add_accommodation(const Accommodation_Data& accommodation)
{
    std::stringstream query;
    query << "INSERT INTO Accommodations (Name, Destination_ID, Type_of_Accommodation, Category, "
          << "Address, Facilities, Rating, Description) VALUES ('"
          << escape_string(accommodation.name) << "', "
          << accommodation.destination_id << ", "
          << accommodation.accommodation_type_id << ", '"
          << escape_string(accommodation.category) << "', '"
          << escape_string(accommodation.address) << "', '"
          << escape_string(accommodation.facilities) << "', "
          << accommodation.rating << ", '"
          << escape_string(accommodation.description) << "')";
    
    return execute_insert(query.str());
}

Database::Query_Result Database::Database_Manager::update_accommodation(const Accommodation_Data& accommodation)
{
    std::stringstream query;
    query << "UPDATE Accommodations SET "
          << "Name = '" << escape_string(accommodation.name) << "', "
          << "Destination_ID = " << accommodation.destination_id << ", "
          << "Type_of_Accommodation = " << accommodation.accommodation_type_id << ", "
          << "Category = '" << escape_string(accommodation.category) << "', "
          << "Address = '" << escape_string(accommodation.address) << "', "
          << "Facilities = '" << escape_string(accommodation.facilities) << "', "
          << "Rating = " << accommodation.rating << ", "
          << "Description = '" << escape_string(accommodation.description) << "', "
          << "Date_Modified = GETDATE() "
          << "WHERE Accommodation_ID = " << accommodation.id;
    
    return execute_update(query.str());
}

Database::Query_Result Database::Database_Manager::delete_accommodation(int accommodation_id)
{
    std::string query = "DELETE FROM Accommodations WHERE Accommodation_ID = " + std::to_string(accommodation_id);
    return execute_delete(query);
}

// Offer management
Database::Query_Result Database::Database_Manager::get_all_offers()
{
    std::string query = "SELECT o.Offer_ID, o.Name, o.Destination_ID, o.Accommodation_ID, o.Types_of_Transport_ID, "
                       "o.Price_per_Person, o.Duration_Days, o.Departure_Date, o.Return_Date, o.Total_Seats, "
                       "o.Reserved_Seats, o.Included_Services, o.Description, o.Status, o.Date_Created, o.Date_Modified, "
                       "d.Name as Destination_Name, d.Country, a.Name as Accommodation_Name, t.Name as Transport_Name "
                       "FROM Offers o "
                       "LEFT JOIN Destinations d ON o.Destination_ID = d.Destination_ID "
                       "LEFT JOIN Accommodations a ON o.Accommodation_ID = a.Accommodation_ID "
                       "LEFT JOIN Types_of_Transport t ON o.Types_of_Transport_ID = t.Transport_Type_ID "
                       "ORDER BY o.Departure_Date";
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::get_available_offers()
{
    std::string query = "SELECT o.Offer_ID, o.Name, o.Destination_ID, o.Accommodation_ID, o.Types_of_Transport_ID, "
                       "o.Price_per_Person, o.Duration_Days, o.Departure_Date, o.Return_Date, o.Total_Seats, "
                       "o.Reserved_Seats, o.Included_Services, o.Description, o.Status, o.Date_Created, o.Date_Modified, "
                       "d.Name as Destination_Name, d.Country, a.Name as Accommodation_Name, t.Name as Transport_Name "
                       "FROM Offers o "
                       "LEFT JOIN Destinations d ON o.Destination_ID = d.Destination_ID "
                       "LEFT JOIN Accommodations a ON o.Accommodation_ID = a.Accommodation_ID "
                       "LEFT JOIN Types_of_Transport t ON o.Types_of_Transport_ID = t.Transport_Type_ID "
                       "WHERE o.Status = 'active' AND o.Reserved_Seats < o.Total_Seats AND o.Departure_Date > GETDATE() "
                       "ORDER BY o.Departure_Date";
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::get_offer_by_id(int offer_id)
{
    std::string query = "SELECT o.Offer_ID, o.Name, o.Destination_ID, o.Accommodation_ID, o.Types_of_Transport_ID, "
                       "o.Price_per_Person, o.Duration_Days, o.Departure_Date, o.Return_Date, o.Total_Seats, "
                       "o.Reserved_Seats, o.Included_Services, o.Description, o.Status, o.Date_Created, o.Date_Modified, "
                       "d.Name as Destination_Name, d.Country, a.Name as Accommodation_Name, t.Name as Transport_Name "
                       "FROM Offers o "
                       "LEFT JOIN Destinations d ON o.Destination_ID = d.Destination_ID "
                       "LEFT JOIN Accommodations a ON o.Accommodation_ID = a.Accommodation_ID "
                       "LEFT JOIN Types_of_Transport t ON o.Types_of_Transport_ID = t.Transport_Type_ID "
                       "WHERE o.Offer_ID = " + std::to_string(offer_id);
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::search_offers(const std::string& destination, double min_price, double max_price,
    const std::string& start_date, const std::string& end_date)
{
    // Validate search parameters
    if (!Utils::Validation::is_valid_price_range(min_price, max_price))
    {
        return Query_Result(Result_Type::ERROR_CONSTRAINT, "Invalid price range");
    }
    
    if (!start_date.empty() && !Utils::DateTime::is_valid_date(start_date))
    {
        return Query_Result(Result_Type::ERROR_CONSTRAINT, "Invalid start date format");
    }
    
    if (!end_date.empty() && !Utils::DateTime::is_valid_date(end_date))
    {
        return Query_Result(Result_Type::ERROR_CONSTRAINT, "Invalid end date format");
    }
    
    std::stringstream query;
    query << "SELECT o.Offer_ID, o.Name, o.Destination_ID, o.Accommodation_ID, o.Types_of_Transport_ID, "
          << "o.Price_per_Person, o.Duration_Days, o.Departure_Date, o.Return_Date, o.Total_Seats, "
          << "o.Reserved_Seats, o.Included_Services, o.Description, o.Status, o.Date_Created, o.Date_Modified, "
          << "d.Name as Destination_Name, d.Country, a.Name as Accommodation_Name, t.Name as Transport_Name "
          << "FROM Offers o "
          << "LEFT JOIN Destinations d ON o.Destination_ID = d.Destination_ID "
          << "LEFT JOIN Accommodations a ON o.Accommodation_ID = a.Accommodation_ID "
          << "LEFT JOIN Types_of_Transport t ON o.Types_of_Transport_ID = t.Transport_Type_ID "
          << "WHERE o.Status = 'active' AND o.Reserved_Seats < o.Total_Seats";

    if (!Utils::String::is_empty(destination))
    {
        query << " AND (d.Name LIKE '%" << escape_string(destination) << "%' OR d.Country LIKE '%" << escape_string(destination) << "%')";
    }
    
    if (min_price > 0)
    {
        query << " AND o.Price_per_Person >= " << Utils::Conversion::double_to_string(min_price);
    }
    
    if (max_price > 0)
    {
        query << " AND o.Price_per_Person <= " << Utils::Conversion::double_to_string(max_price);
    }
    
    if (!Utils::String::is_empty(start_date))
    {
        query << " AND o.Departure_Date >= '" << escape_string(start_date) << "'";
    }
    
    if (!Utils::String::is_empty(end_date))
    {
        query << " AND o.Return_Date <= '" << escape_string(end_date) << "'";
    }
    
    query << " ORDER BY o.Departure_Date";
    
    return execute_select(query.str());
}

Database::Query_Result Database::Database_Manager::add_offer(const Offer_Data& offer)
{
    std::stringstream query;
    query << "INSERT INTO Offers (Name, Destination_ID, Accommodation_ID, Types_of_Transport_ID, "
          << "Price_per_Person, Duration_Days, Departure_Date, Return_Date, Total_Seats, Reserved_Seats, "
          << "Included_Services, Description, Status) VALUES ('"
          << escape_string(offer.name) << "', "
          << offer.destination_id << ", "
          << offer.accommodation_id << ", "
          << offer.transport_type_id << ", "
          << offer.price_per_person << ", "
          << offer.duration_days << ", '"
          << escape_string(offer.departure_date) << "', '"
          << escape_string(offer.return_date) << "', "
          << offer.total_seats << ", "
          << offer.reserved_seats << ", '"
          << escape_string(offer.included_services) << "', '"
          << escape_string(offer.description) << "', '"
          << escape_string(offer.status) << "')";
    
    return execute_insert(query.str());
}

Database::Query_Result Database::Database_Manager::update_offer(const Offer_Data& offer)
{
    std::stringstream query;
    query << "UPDATE Offers SET "
          << "Name = '" << escape_string(offer.name) << "', "
          << "Destination_ID = " << offer.destination_id << ", "
          << "Accommodation_ID = " << offer.accommodation_id << ", "
          << "Types_of_Transport_ID = " << offer.transport_type_id << ", "
          << "Price_per_Person = " << offer.price_per_person << ", "
          << "Duration_Days = " << offer.duration_days << ", "
          << "Departure_Date = '" << escape_string(offer.departure_date) << "', "
          << "Return_Date = '" << escape_string(offer.return_date) << "', "
          << "Total_Seats = " << offer.total_seats << ", "
          << "Reserved_Seats = " << offer.reserved_seats << ", "
          << "Included_Services = '" << escape_string(offer.included_services) << "', "
          << "Description = '" << escape_string(offer.description) << "', "
          << "Status = '" << escape_string(offer.status) << "', "
          << "Date_Modified = GETDATE() "
          << "WHERE Offer_ID = " << offer.id;
    
    return execute_update(query.str());
}

Database::Query_Result Database::Database_Manager::delete_offer(int offer_id)
{
    std::string query = "DELETE FROM Offers WHERE Offer_ID = " + std::to_string(offer_id);
    return execute_delete(query);
}

// Reservation management
Database::Query_Result Database::Database_Manager::book_offer(int user_id, int offer_id, int person_count)
{
    // Validate parameters
    if (!Utils::Validation::is_valid_person_count(person_count))
    {
        return Database::Query_Result(Database::Result_Type::ERROR_CONSTRAINT, "Invalid person count");
    }
    
    // Begin transaction FIRST to ensure atomic operation
    if (!begin_transaction())
    {
        return Query_Result(Result_Type::ERROR_EXECUTION, "Failed to begin transaction");
    }
    
    // ATOMICALLY check availability and reserve seats with row locking
    // Use SELECT FOR UPDATE to lock the row until transaction commits
    std::stringstream lock_query;
    lock_query << "SELECT Total_Seats, Reserved_Seats, Price_per_Person "
               << "FROM Offers WITH (UPDLOCK, ROWLOCK) WHERE Offer_ID = " << offer_id;
    
    Query_Result offer_result = execute_query(lock_query.str());
    if (!offer_result.is_success() || offer_result.data.empty())
    {
        rollback_transaction();
        return Query_Result(Result_Type::DB_ERROR_NO_DATA, "Offer not found");
    }
    
    auto offer_data = offer_result.data[0];
    int total_seats = Utils::Conversion::string_to_int(offer_data["Total_Seats"]);
    int reserved_seats = Utils::Conversion::string_to_int(offer_data["Reserved_Seats"]);
    int available_seats = total_seats - reserved_seats;
    
    // Check seat availability within the locked transaction
    if (person_count > available_seats)
    {
        rollback_transaction();
        return Query_Result(Result_Type::ERROR_CONSTRAINT, "Not enough available seats");
    }
    
    double price_per_person = Utils::Conversion::string_to_double(offer_data["Price_per_Person"]);
    double total_price = price_per_person * person_count;
    
    // Insert reservation first
    std::stringstream insert_query;
    insert_query << "INSERT INTO Reservations (User_ID, Offer_ID, Number_of_Persons, Total_Price, Status) VALUES ("
                 << user_id << ", " << offer_id << ", " << person_count << ", " << total_price << ", 'pending')";
    
    Query_Result insert_result = execute_query(insert_query.str());
    if (!insert_result.is_success())
    {
        rollback_transaction();
        return insert_result;
    }
    
    // Update offer reserved seats with constraint check in SQL
    std::stringstream update_query;
    update_query << "UPDATE Offers SET Reserved_Seats = Reserved_Seats + " << person_count 
                 << " WHERE Offer_ID = " << offer_id 
                 << " AND Reserved_Seats + " << person_count << " <= Total_Seats";
    
    Query_Result update_result = execute_query(update_query.str());
    if (!update_result.is_success())
    {
        rollback_transaction();
        return Query_Result(Result_Type::ERROR_EXECUTION, "Failed to update reserved seats");
    }
    
    // Verify the update affected a row (seat constraint was satisfied)
    if (update_result.affected_rows == 0)
    {
        rollback_transaction();
        return Query_Result(Result_Type::ERROR_CONSTRAINT, "Not enough available seats - concurrent booking detected");
    }
    
    if (!commit_transaction())
    {
        rollback_transaction();
        return Query_Result(Result_Type::ERROR_EXECUTION, "Failed to commit transaction");
    }
    
    return Query_Result(Result_Type::SUCCESS, "Booking created successfully");
}

Database::Query_Result Database::Database_Manager::get_user_reservations(int user_id)
{
    std::string query = "SELECT r.Reservation_ID, r.User_ID, r.Offer_ID, r.Number_of_Persons, r.Total_Price, "
                       "r.Reservation_Date, r.Status, r.Notes, "
                       "o.Name as Offer_Name, d.Name as Destination_Name, d.Country "
                       "FROM Reservations r "
                       "LEFT JOIN Offers o ON r.Offer_ID = o.Offer_ID "
                       "LEFT JOIN Destinations d ON o.Destination_ID = d.Destination_ID "
                       "WHERE r.User_ID = " + std::to_string(user_id) + " ORDER BY r.Reservation_Date DESC";
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::get_offer_reservations(int offer_id)
{
    std::string query = "SELECT r.Reservation_ID, r.User_ID, r.Offer_ID, r.Number_of_Persons, r.Total_Price, "
                       "r.Reservation_Date, r.Status, r.Notes, "
                       "u.Username, u.First_Name, u.Last_Name, u.Email "
                       "FROM Reservations r "
                       "LEFT JOIN Users u ON r.User_ID = u.User_ID "
                       "WHERE r.Offer_ID = " + std::to_string(offer_id) + " ORDER BY r.Reservation_Date";
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::get_reservation_by_id(int reservation_id)
{
    std::string query = "SELECT r.Reservation_ID, r.User_ID, r.Offer_ID, r.Number_of_Persons, r.Total_Price, "
                       "r.Reservation_Date, r.Status, r.Notes, "
                       "o.Name as Offer_Name, d.Name as Destination_Name, d.Country, "
                       "u.Username, u.First_Name, u.Last_Name, u.Email "
                       "FROM Reservations r "
                       "LEFT JOIN Offers o ON r.Offer_ID = o.Offer_ID "
                       "LEFT JOIN Destinations d ON o.Destination_ID = d.Destination_ID "
                       "LEFT JOIN Users u ON r.User_ID = u.User_ID "
                       "WHERE r.Reservation_ID = " + std::to_string(reservation_id);
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::cancel_reservation(int reservation_id)
{
    // Get reservation details first
    Database::Query_Result reservation_result = get_reservation_by_id(reservation_id);
    if (!reservation_result.is_success() || reservation_result.data.empty())
    {
        return Database::Query_Result(Database::Result_Type::DB_ERROR_NO_DATA, "Reservation not found");
    }
    
    auto reservation_data = reservation_result.data[0];
    int offer_id = Utils::Conversion::string_to_int(reservation_data["Offer_ID"]);
    int person_count = Utils::Conversion::string_to_int(reservation_data["Number_of_Persons"]);
    std::string current_status = reservation_data["Status"];
    
    if (current_status == "cancelled")
    {
        return Query_Result(Result_Type::ERROR_CONSTRAINT, "Reservation already cancelled");
    }
    
    // Begin transaction
    if (!begin_transaction())
    {
        return Query_Result(Result_Type::ERROR_EXECUTION, "Failed to begin transaction");
    }
    
    // Update reservation status
    std::string update_reservation_query = "UPDATE Reservations SET Status = 'cancelled' WHERE Reservation_ID = " + std::to_string(reservation_id);
    Query_Result update_result = execute_query(update_reservation_query);
    if (!update_result.is_success())
    {
        rollback_transaction();
        return update_result;
    }
    
    // Update offer available seats
    std::stringstream update_offer_query;
    update_offer_query << "UPDATE Offers SET Reserved_Seats = Reserved_Seats - " << person_count 
                      << " WHERE Offer_ID = " << offer_id;
    
    Query_Result seats_result = execute_query(update_offer_query.str());
    if (!seats_result.is_success())
    {
        rollback_transaction();
        return seats_result;
    }
    
    if (!commit_transaction())
    {
        rollback_transaction();
        return Query_Result(Result_Type::ERROR_EXECUTION, "Failed to commit transaction");
    }
    
    return Query_Result(Result_Type::SUCCESS, "Reservation cancelled successfully");
}

Database::Query_Result Database::Database_Manager::update_reservation_status(int reservation_id, const std::string& status)
{
    std::string query = "UPDATE Reservations SET Status = '" + escape_string(status) + 
                       "' WHERE Reservation_ID = " + std::to_string(reservation_id);
    return execute_update(query);
}

// Reservation persons management
Database::Query_Result Database::Database_Manager::add_reservation_person(const Reservation_Person_Data& person_data)
{
    std::stringstream query;
    query << "INSERT INTO Reservation_Persons (Reservation_ID, Full_Name, CNP, Birth_Date, Person_Type) VALUES ("
          << person_data.reservation_id << ", '"
          << escape_string(person_data.full_name) << "', '"
          << escape_string(person_data.cnp) << "', '"
          << escape_string(person_data.birth_date) << "', '"
          << escape_string(person_data.person_type) << "')";
    
    return execute_insert(query.str());
}

Database::Query_Result Database::Database_Manager::get_reservation_persons(int reservation_id)
{
    std::string query = "SELECT Reservation_Person_ID, Reservation_ID, Full_Name, CNP, Birth_Date, Person_Type "
                       "FROM Reservation_Persons WHERE Reservation_ID = " + std::to_string(reservation_id) + 
                       " ORDER BY Reservation_Person_ID";
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::update_reservation_person(const Reservation_Person_Data& person_data)
{
    std::stringstream query;
    query << "UPDATE Reservation_Persons SET "
          << "Full_Name = '" << escape_string(person_data.full_name) << "', "
          << "CNP = '" << escape_string(person_data.cnp) << "', "
          << "Birth_Date = '" << escape_string(person_data.birth_date) << "', "
          << "Person_Type = '" << escape_string(person_data.person_type) << "' "
          << "WHERE Reservation_Person_ID = " << person_data.id;
    
    return execute_update(query.str());
}

Database::Query_Result Database::Database_Manager::delete_reservation_person(int person_id)
{
    std::string query = "DELETE FROM Reservation_Persons WHERE Reservation_Person_ID = " + std::to_string(person_id);
    return execute_delete(query);
}

// Statistics and reports
Database::Query_Result Database::Database_Manager::get_popular_destinations(int limit)
{
    std::string query = "SELECT TOP " + Utils::Conversion::int_to_string(limit) + 
                       " d.Destination_ID, d.Name, d.Country, COUNT(r.Reservation_ID) as Booking_Count "
                       "FROM Destinations d "
                       "LEFT JOIN Offers o ON d.Destination_ID = o.Destination_ID "
                       "LEFT JOIN Reservations r ON o.Offer_ID = r.Offer_ID AND r.Status != 'cancelled' "
                       "GROUP BY d.Destination_ID, d.Name, d.Country "
                       "ORDER BY Booking_Count DESC";
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::get_revenue_report(const std::string& start_date, const std::string& end_date)
{
    std::stringstream query;
    query << "SELECT "
          << "COUNT(r.Reservation_ID) as Total_Reservations, "
          << "SUM(r.Total_Price) as Total_Revenue, "
          << "AVG(r.Total_Price) as Average_Booking_Value, "
          << "SUM(r.Number_of_Persons) as Total_Persons "
          << "FROM Reservations r "
          << "WHERE r.Status IN ('confirmed', 'paid')";
    
    if (!start_date.empty())
    {
        query << " AND r.Reservation_Date >= '" << escape_string(start_date) << "'";
    }
    
    if (!end_date.empty())
    {
        query << " AND r.Reservation_Date <= '" << escape_string(end_date) << "'";
    }
    
    return execute_select(query.str());
}

Database::Query_Result Database::Database_Manager::get_user_statistics()
{
    std::string query = "SELECT "
                       "COUNT(*) as Total_Users, "
                       "COUNT(CASE WHEN Date_Created >= DATEADD(month, -1, GETDATE()) THEN 1 END) as New_Users_This_Month, "
                       "COUNT(CASE WHEN Date_Created >= DATEADD(week, -1, GETDATE()) THEN 1 END) as New_Users_This_Week "
                       "FROM Users";
    return execute_select(query);
}

Database::Query_Result Database::Database_Manager::get_booking_statistics()
{
    std::string query = "SELECT "
                       "COUNT(*) as Total_Bookings, "
                       "COUNT(CASE WHEN Status = 'pending' THEN 1 END) as Pending_Bookings, "
                       "COUNT(CASE WHEN Status = 'confirmed' THEN 1 END) as Confirmed_Bookings, "
                       "COUNT(CASE WHEN Status = 'paid' THEN 1 END) as Paid_Bookings, "
                       "COUNT(CASE WHEN Status = 'cancelled' THEN 1 END) as Cancelled_Bookings, "
                       "COUNT(CASE WHEN Reservation_Date >= DATEADD(month, -1, GETDATE()) THEN 1 END) as Bookings_This_Month "
                       "FROM Reservations";
    return execute_select(query);
}

// Additional utility methods
Database::Query_Result Database::Database_Manager::execute_prepared(const std::string& query, const std::vector<std::pair<std::string, std::string>>& params)
{
    // Basic implementation - for production, use proper prepared statements
    std::string prepared_query = query;
    for (const auto& param : params)
    {
        size_t pos = prepared_query.find(param.first);
        if (pos != std::string::npos)
        {
            prepared_query.replace(pos, param.first.length(), escape_string(param.second));
        }
    }
    return execute_query(prepared_query);
}

Database::Query_Result Database::Database_Manager::execute_stored_procedure(const std::string& procedure_name, const std::vector<std::string>& params)
{
    std::stringstream query;
    query << "EXEC " << procedure_name;
    
    for (size_t i = 0; i < params.size(); ++i)
    {
        if (i > 0)
            query << ",";
        query << " '" << escape_string(params[i]) << "'";
    }
    
    return execute_query(query.str());
}

bool Database::Database_Manager::table_exists(const std::string& table_name)
{
    std::string query = "SELECT 1 FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_NAME = '" + escape_string(table_name) + "'";
    Database::Query_Result result = execute_select(query);
    return result.is_success() && !result.data.empty();
}

std::vector<std::string> Database::Database_Manager::get_table_columns(const std::string& table_name)
{
    std::vector<std::string> columns;
    std::string query = "SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_NAME = '" + 
                       escape_string(table_name) + "' ORDER BY ORDINAL_POSITION";
    Database::Query_Result result = execute_select(query);
    
    if (result.is_success())
    {
        for (const auto& row : result.data)
        {
            columns.push_back(row.at("COLUMN_NAME"));
        }
    }
    
    return columns;
}

bool Database::Database_Manager::create_tables_if_not_exists()
{
    std::vector<std::string> create_queries = {
        get_create_users_table_sql(),
        get_create_destinations_table_sql(),
        get_create_transport_types_table_sql(),
        get_create_accommodation_types_table_sql(),
        get_create_accommodations_table_sql(),
        get_create_offers_table_sql(),
        get_create_reservations_table_sql(),
        get_create_reservation_persons_table_sql(),
        get_create_indexes_sql()
    };
    
    for (const auto& query : create_queries)
    {
        Query_Result result = execute_query(query);
        if (!result.is_success())
        {
            log_error("create_tables_if_not_exists", "Failed to execute: " + query);
            return false;
        }
    }
    
    return true;
}

bool Database::Database_Manager::retry_operation(std::function<bool()> operation, int max_attempts)
{
    for (int attempt = 1; attempt <= max_attempts; ++attempt)
    {
        if (operation())
        {
            return true;
        }
        
        if (attempt < max_attempts)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_DELAY_MS));
        }
    }
    
    return false;
}

// Private table creation methods
std::string Database::Database_Manager::get_create_users_table_sql()
{
    return R"(
        IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'dbo.Users') AND type = 'U')
        BEGIN
            CREATE TABLE dbo.Users (
                User_ID INT PRIMARY KEY IDENTITY(1,1),
                Username VARCHAR(50) NOT NULL UNIQUE,
                Password_Hash VARCHAR(255) NOT NULL,
                Email VARCHAR(100) UNIQUE,
                First_Name VARCHAR(50),
                Last_Name VARCHAR(50),
                Phone VARCHAR(15),
                Date_Created DATETIME DEFAULT GETDATE(),
                Date_Modified DATETIME DEFAULT GETDATE()
            )
        END
    )";
}

std::string Database::Database_Manager::get_create_destinations_table_sql()
{
    return R"(
        IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'dbo.Destinations') AND type = 'U')
        BEGIN
            CREATE TABLE dbo.Destinations (
                Destination_ID INT PRIMARY KEY IDENTITY(1,1),
                Name VARCHAR(100) NOT NULL,
                Country VARCHAR(100) NOT NULL,
                Description TEXT,
                Image_Path VARCHAR(255),
                Date_Created DATETIME DEFAULT GETDATE(),
                Date_Modified DATETIME DEFAULT GETDATE()
            )
        END
    )";
}

std::string Database::Database_Manager::get_create_transport_types_table_sql()
{
    return R"(
        IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'dbo.Types_of_Transport') AND type = 'U')
        BEGIN
            CREATE TABLE dbo.Types_of_Transport (
                Transport_Type_ID INT PRIMARY KEY IDENTITY(1,1),
                Name VARCHAR(100) NOT NULL,
                Description TEXT,
                Date_Created DATETIME DEFAULT GETDATE(),
                Date_Modified DATETIME DEFAULT GETDATE()
            )
        END
    )";
}

std::string Database::Database_Manager::get_create_accommodation_types_table_sql()
{
    return R"(
        IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'dbo.Types_of_Accommodation') AND type = 'U')
        BEGIN
            CREATE TABLE dbo.Types_of_Accommodation (
                Accommodation_Type_ID INT PRIMARY KEY IDENTITY(1,1),
                Name VARCHAR(100) NOT NULL,
                Description TEXT,
                Date_Created DATETIME DEFAULT GETDATE(),
                Date_Modified DATETIME DEFAULT GETDATE()
            )
        END
    )";
}

std::string Database::Database_Manager::get_create_accommodations_table_sql()
{
    return R"(
        IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'dbo.Accommodations') AND type = 'U')
        BEGIN
            CREATE TABLE dbo.Accommodations (
                Accommodation_ID INT PRIMARY KEY IDENTITY(1,1),
                Name VARCHAR(100) NOT NULL,
                Destination_ID INT NOT NULL,
                Type_of_Accommodation INT NOT NULL,
                Category VARCHAR(10),
                Address VARCHAR(255),
                Facilities TEXT,
                Rating DECIMAL(4, 2) CHECK (Rating >= 0 AND Rating <= 10),
                Description TEXT,
                Date_Created DATETIME DEFAULT GETDATE(),
                Date_Modified DATETIME DEFAULT GETDATE(),
                FOREIGN KEY (Destination_ID) REFERENCES dbo.Destinations(Destination_ID),
                FOREIGN KEY (Type_of_Accommodation) REFERENCES dbo.Types_of_Accommodation(Accommodation_Type_ID)
            )
        END
    )";
}

std::string Database::Database_Manager::get_create_offers_table_sql()
{
    return R"(
        IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'dbo.Offers') AND type = 'U')
        BEGIN
            CREATE TABLE dbo.Offers (
                Offer_ID INT PRIMARY KEY IDENTITY(1,1),
                Name VARCHAR(150) NOT NULL,
                Destination_ID INT NOT NULL,
                Accommodation_ID INT NOT NULL,
                Types_of_Transport_ID INT NOT NULL,
                Price_per_Person DECIMAL(10, 2) NOT NULL,
                Duration_Days INT NOT NULL,
                Departure_Date DATE NOT NULL,
                Return_Date DATE NOT NULL,
                Total_Seats INT NOT NULL,
                Reserved_Seats INT NOT NULL DEFAULT 0,
                Included_Services TEXT,
                Description TEXT,
                Status VARCHAR(20) NOT NULL DEFAULT 'active',
                Date_Created DATETIME DEFAULT GETDATE(),
                Date_Modified DATETIME DEFAULT GETDATE(),
                FOREIGN KEY (Destination_ID) REFERENCES dbo.Destinations(Destination_ID),
                FOREIGN KEY (Accommodation_ID) REFERENCES dbo.Accommodations(Accommodation_ID),
                FOREIGN KEY (Types_of_Transport_ID) REFERENCES dbo.Types_of_Transport(Transport_Type_ID),
                CHECK (Status IN ('active', 'inactive', 'expired'))
            )
        END
    )";
}

std::string Database::Database_Manager::get_create_reservations_table_sql()
{
    return R"(
        IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'dbo.Reservations') AND type = 'U')
        BEGIN
            CREATE TABLE dbo.Reservations (
                Reservation_ID INT PRIMARY KEY IDENTITY(1,1),
                User_ID INT NOT NULL,
                Offer_ID INT NOT NULL,
                Number_of_Persons INT NOT NULL,
                Total_Price DECIMAL(10,2) NOT NULL,
                Reservation_Date DATETIME DEFAULT GETDATE(),
                Status VARCHAR(20) NOT NULL,
                Notes TEXT,
                FOREIGN KEY (User_ID) REFERENCES dbo.Users(User_ID),
                FOREIGN KEY (Offer_ID) REFERENCES dbo.Offers(Offer_ID),
                CHECK (Status IN ('pending', 'confirmed', 'paid', 'cancelled'))
            )
        END
    )";
}

std::string Database::Database_Manager::get_create_reservation_persons_table_sql()
{
    return R"(
        IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'dbo.Reservation_Persons') AND type = 'U')
        BEGIN
            CREATE TABLE dbo.Reservation_Persons (
                Reservation_Person_ID INT PRIMARY KEY IDENTITY(1,1),
                Reservation_ID INT NOT NULL,
                Full_Name VARCHAR(100) NOT NULL,
                CNP VARCHAR(15) NOT NULL,
                Birth_Date DATE NOT NULL,
                Person_Type VARCHAR(20) NOT NULL,
                FOREIGN KEY (Reservation_ID) REFERENCES dbo.Reservations(Reservation_ID)
            )
        END
    )";
}

std::string Database::Database_Manager::get_create_indexes_sql()
{
    return R"(
        IF NOT EXISTS (SELECT * FROM sys.indexes WHERE name = 'IX_Users_Username')
            CREATE INDEX IX_Users_Username ON Users(Username);
        
        IF NOT EXISTS (SELECT * FROM sys.indexes WHERE name = 'IX_Offers_Destination')
            CREATE INDEX IX_Offers_Destination ON Offers(Destination_ID);
        
        IF NOT EXISTS (SELECT * FROM sys.indexes WHERE name = 'IX_Offers_Price')
            CREATE INDEX IX_Offers_Price ON Offers(Price_per_Person);
        
        IF NOT EXISTS (SELECT * FROM sys.indexes WHERE name = 'IX_Offers_Status')
            CREATE INDEX IX_Offers_Status ON Offers(Status);
        
        IF NOT EXISTS (SELECT * FROM sys.indexes WHERE name = 'IX_Reservations_User')
            CREATE INDEX IX_Reservations_User ON Reservations(User_ID);
        
        IF NOT EXISTS (SELECT * FROM sys.indexes WHERE name = 'IX_Offers_Destination_Price')
            CREATE INDEX IX_Offers_Destination_Price ON Offers(Destination_ID, Price_per_Person);
    )";
}

// Demo mode utilities implementation
void Database::Database_Manager::enable_demo_mode()
{
    is_demo_mode = true;
    Utils::Logger::warning("Database_Manager switched to DEMO MODE");
}

bool Database::Database_Manager::is_running_in_demo_mode() const
{
    return is_demo_mode;
}

Database::Query_Result Database::Database_Manager::create_mock_response(const std::string& operation)
{
    Utils::Logger::info("DEMO MODE: Creating mock response for operation: " + operation);
    
    if (operation == "get_destinations")
    {
        Query_Result result(Result_Type::SUCCESS, "Demo destinations retrieved");
        
        // Mock destination 1
        std::map<std::string, std::string> dest1;
        dest1["Destination_ID"] = "1";
        dest1["Name"] = "Paris";
        dest1["Country"] = "France";
        dest1["Description"] = "City of Light - Demo destination";
        result.data.push_back(dest1);
        
        // Mock destination 2
        std::map<std::string, std::string> dest2;
        dest2["Destination_ID"] = "2";
        dest2["Name"] = "Rome";
        dest2["Country"] = "Italy";  
        dest2["Description"] = "Eternal City - Demo destination";
        result.data.push_back(dest2);
        
        return result;
    }
    else if (operation == "get_offers")
    {
        Query_Result result(Result_Type::SUCCESS, "Demo offers retrieved");
        
        // Mock offer 1
        std::map<std::string, std::string> offer1;
        offer1["Offer_ID"] = "1";
        offer1["Name"] = "Paris Weekend";
        offer1["Destination"] = "Paris";
        offer1["Price_per_Person"] = "299.99";
        offer1["Duration_Days"] = "3";
        offer1["Available_Seats"] = "10";
        result.data.push_back(offer1);
        
        // Mock offer 2  
        std::map<std::string, std::string> offer2;
        offer2["Offer_ID"] = "2";
        offer2["Name"] = "Rome Adventure";
        offer2["Destination"] = "Rome";
        offer2["Price_per_Person"] = "449.99";
        offer2["Duration_Days"] = "5";
        offer2["Available_Seats"] = "8";
        result.data.push_back(offer2);
        
        return result;
    }
    else
    {
        return Query_Result(Result_Type::SUCCESS, "Demo mode: Operation '" + operation + "' completed successfully");
    }
}