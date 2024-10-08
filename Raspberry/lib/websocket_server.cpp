#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <iostream>
#include <set>
#include <fstream>
#include <unordered_map>
#include <boost/algorithm/string.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>
#include <nlohmann/json.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using json = nlohmann::json;

std::set<connection_hdl, std::owner_less<connection_hdl>> clients;
std::string settings_file = "settings.txt";
std::string properties_file = "properties.txt";
server print_server;

struct PropertyDefinition {
    std::string type;
    std::vector<std::string> options;
};

std::string settings_to_json(const std::unordered_map<std::string, std::string>& settings);

std::unordered_map<std::string, std::string> php_unserialize(const std::string& data) {
    std::unordered_map<std::string, std::string> settings;
    size_t pos = 0;

    if (data.substr(0, 2) == "a:") {
        pos = data.find('{') + 1;
    } else {
        std::cerr << "Unexpected character in serialized data: " << data[pos] << std::endl;
        return settings;
    }

    while (pos < data.size()) {
        if (data[pos] == 's') {
            pos += 2;
            size_t len_pos = data.find(':', pos);
            if (len_pos == std::string::npos) break;
            size_t key_len = std::stoul(data.substr(pos, len_pos - pos));
            pos = len_pos + 2;
            std::string key = data.substr(pos, key_len);
            pos += key_len + 2;

            char type = data[pos];
            pos += 2;

            if (type == 's') {
                size_t val_len_pos = data.find(':', pos);
                if (val_len_pos == std::string::npos) break;
                size_t val_len = std::stoul(data.substr(pos, val_len_pos - pos));
                pos = val_len_pos + 2;
                std::string value = data.substr(pos, val_len);
                pos += val_len + 2;
                settings[key] = value;
            } else if (type == 'i') {
                size_t val_end_pos = data.find(';', pos);
                if (val_end_pos == std::string::npos) break;
                std::string value = data.substr(pos, val_end_pos - pos);
                pos = val_end_pos + 1;
                settings[key] = value;
            } else {
                std::cerr << "Unsupported type found in serialized data: " << type << std::endl;
                break;
            }
        } else {
            std::cerr << "Unexpected character in serialized data: " << data[pos] << std::endl;
            break;
        }
    }
    return settings;
}

std::unordered_map<std::string, std::string> read_settings() {
    std::unordered_map<std::string, std::string> settings;
    std::ifstream file(settings_file);

    if (!file) {
        std::cerr << "Failed to open settings file: " << settings_file << std::endl;
        return settings;
    }

    std::string serialized_data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::cout << "Serialized data read: " << serialized_data << std::endl;
    settings = php_unserialize(serialized_data);

    std::cout << "Settings read: " << settings_to_json(settings) << std::endl;
    return settings;
}

std::unordered_map<std::string, PropertyDefinition> read_properties() {
    std::unordered_map<std::string, PropertyDefinition> properties;
    std::ifstream file(properties_file);

    if (!file) {
        std::cerr << "Failed to open properties file: " << properties_file << std::endl;
        return properties;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, type;
        if (std::getline(iss, key, ':') && std::getline(iss, type)) {
            PropertyDefinition propDef;
            if (type.find("enum:") == 0) {
                propDef.type = "enum";
                std::string options = type.substr(5);
                boost::split(propDef.options, options, boost::is_any_of(","));
            } else {
                propDef.type = type;
            }
            properties[key] = propDef;
        }
    }

    return properties;
}

std::string settings_to_json(const std::unordered_map<std::string, std::string>& settings) {
    json j;
    for (const auto& pair : settings) {
        j[pair.first] = pair.second;
    }
    return j.dump();
}

void broadcast_settings() {
    auto settings = read_settings();
    std::string message = settings_to_json(settings);
    std::cout << "Broadcasting message: " << message << std::endl;

    for (auto it : clients) {
        print_server.send(it, message, websocketpp::frame::opcode::text);
    }
}

void on_open(connection_hdl hdl) {
    clients.insert(hdl);
    broadcast_settings();
}

void on_close(connection_hdl hdl) {
    clients.erase(hdl);
}

void on_message(connection_hdl hdl, server::message_ptr msg) {
    std::string payload = msg->get_payload();
    if (payload == "update") {
        broadcast_settings();
    }
}

void run_websocket_server() {
    print_server.set_open_handler(&on_open);
    print_server.set_close_handler(&on_close);
    print_server.set_message_handler(&on_message);

    print_server.init_asio();
    print_server.listen(6789);
    print_server.start_accept();
    print_server.run();
}

int main() {
    try {
        websocketpp::lib::thread server_thread(&run_websocket_server);
        server_thread.join();
    } catch (websocketpp::exception const & e) {
        std::cout << "WebSocket++ exception: " << e.what() << std::endl;
    }
    return 0;
}
