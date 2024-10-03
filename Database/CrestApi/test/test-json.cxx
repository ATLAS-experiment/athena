#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include <iomanip> 

int main()
{
    // create the different JSON values with default values
    json j_null(json::value_t::null);
    json j_boolean(json::value_t::boolean);
    json j_number_integer(json::value_t::number_integer);
    json j_number_float(json::value_t::number_float);
    json j_object(json::value_t::object);
    json j_array(json::value_t::array);
    json j_string(json::value_t::string);

    // serialize the JSON values
    std::cout << j_null << '\n';
    std::cout << j_boolean << '\n';
    std::cout << j_number_integer << '\n';
    std::cout << j_number_float << '\n';
    std::cout << j_object << '\n';
    std::cout << j_array << '\n';
    std::cout << j_string << '\n';

    // create object from string literal
    json j = "{ \"happy\": true, \"pi\": 3.141 }"_json;

    // or even nicer with a raw string literal
    auto j2 = R"(
        {
            "happy": true,
            "pi": 3.141
        }
    )"_json;

    float pi = j2["pi"];
    std::cout << pi << '\n';

    //float rnd = random()/1000.;
    double rnd = 3.015999999999;
    std::cout << rnd << '\n';

    // Create a pointer to the double
    unsigned char* bytePtr = reinterpret_cast<unsigned char*>(&rnd);

    // Print the binary representation
    std::cout << "Binary representation of double: ";
    for (int i = sizeof(rnd) - 1; i >= 0; --i) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(bytePtr[i]);
    }
    std::cout << std::endl;

    j2["pi"] = rnd;
    std::cout << j2 << '\n';
    double rnd_json = j2["pi"];
    std::cout << "Binary representation of double read from json: ";
    unsigned char* bytePtr2 = reinterpret_cast<unsigned char*>(&rnd_json);
    for (int i = sizeof(rnd_json) - 1; i >= 0; --i) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(bytePtr2[i]);
    }
    std::cout << std::endl;
    auto j3 = R"(
        {
            "happy": true,
            "pi": 3.015999999999
        }
    )"_json;
    double rnd_json_2 = j3["pi"];
    std::cout << "Binary representation of double read from json: ";
    unsigned char* bytePtr3 = reinterpret_cast<unsigned char*>(&rnd_json_2);
    for (int i = sizeof(rnd_json_2) - 1; i >= 0; --i) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(bytePtr3[i]);
    }
    std::cout << std::endl;

    // Fill a json object with data
    json j4;
    j4["a_char"] = new char('c');
    j4["a_string"] = "string";
    j4["a_bool"] = true;
    j4["a_int"] = 42;
    j4["a_float"] = 3.141;
    j4["a_null"] = nullptr;
    j4["an_unsigned_int"] = 42u;
    j4["an_unsigned_long"] = 42ul;
    j4["an_unsigned_long_long"] = 42ull;

    std::cout << j4 << '\n';

    // Read back data using iterators
    for (json::iterator it = j4.begin(); it != j4.end(); ++it) {
        std::cout << it.key() << " : " << it.value() << "\n";
        if (it.key() == "a_char") {
            std::cout << "a_char is a char: " << it.value().is_string() << "\n";
            std::string a_str = it.value().get<std::string>();
            char a_char = a_str[0];
            std::cout << "a_char: " << a_char << "\n";
        }
        if (it.key() == "a_string") {
            std::cout << "a_string is a string: " << it.value().is_string() << "\n";
            std::string a_string = it.value().get<std::string>();
            std::cout << "a_string: " << a_string << "\n";
        }
        if (it.key() == "a_bool") {
            std::cout << "a_bool is a bool: " << it.value().is_boolean() << "\n";
            bool a_bool = it.value().get<bool>();
            std::cout << "a_bool: " << a_bool << "\n";
        }
        if (it.key() == "a_int") {
            std::cout << "a_int is an int: " << it.value().is_number_integer() << "\n";
            int a_int = it.value().get<int>();
            std::cout << "a_int: " << a_int << "\n";
        }
        if (it.key() == "a_float") {
            std::cout << "a_float is a float: " << it.value().is_number_float() << "\n";
            float a_float = it.value().get<float>();
            std::cout << "a_float: " << a_float << "\n";
        }
        if (it.key() == "a_unsigned_int") {
            std::cout << "a_unsigned_int is an unsigned int: " << it.value().is_number_unsigned() << "\n";
            unsigned int a_unsigned_int = it.value().get<unsigned int>();
            std::cout << "a_unsigned_int: " << a_unsigned_int << "\n";
        }
        if (it.key() == "a_unsigned_long") {
            std::cout << "a_unsigned_long is an unsigned long: " << it.value().is_number_unsigned() << "\n";
            unsigned long a_unsigned_long = it.value().get<unsigned long>();
            std::cout << "a_unsigned_long: " << a_unsigned_long << "\n";
        }
        if (it.key() == "a_unsigned_long_long") {
            std::cout << "a_unsigned_long_long is an unsigned long long: " << it.value().is_number_unsigned() << "\n";
            unsigned long long a_unsigned_long_long = it.value().get<unsigned long long>();
            std::cout << "a_unsigned_long_long: " << a_unsigned_long_long << "\n";
        }
        if (it.key() == "a_null") {
            std::cout << "a_null is null: " << it.value().is_null() << "\n";
        }
    
    }

}
