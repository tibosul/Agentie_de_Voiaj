#include "utils.h"
#include <iostream>

int main()
{
    std::cout << "=== Testing Utils Library ===" << std::endl;

    // Test string utilities
    std::cout << "Trimmed: '" << Utils::String::trim("  Hello World  ") << "'" << std::endl;

    // Test date/time
    std::cout << "Current date: " << Utils::DateTime::get_current_date() << std::endl;

    // Test crypto
    std::string salt = Utils::Crypto::generate_salt();
    std::string hash = Utils::Crypto::hash_password("test123", salt);
    std::cout << "Salt: " << salt << std::endl;
    std::cout << "Password hash: " << hash << std::endl;

    // Test JSON
    std::string success = Utils::JSON::create_success_response("test data", "Success!");
    std::cout << "JSON: " << success << std::endl;

    return 0;
}