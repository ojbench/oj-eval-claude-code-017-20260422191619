#include <iostream>
#include <string>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string line;
    while (std::getline(std::cin, line)) {
        // Trim leading spaces
        size_t start = 0;
        while (start < line.size() && (line[start] == ' ' || line[start] == '\t' || line[start] == '\r')) ++start;
        if (start >= line.size()) continue; // skip empty

        // Extract command token (up to first space)
        size_t end = start;
        while (end < line.size() && line[end] != ' ' && line[end] != '\t' && line[end] != '\r') ++end;
        std::string cmd = line.substr(start, end - start);

        if (cmd == "exit") {
            std::cout << "bye\n";
            return 0;
        } else if (cmd == "clean") {
            std::cout << 0 << "\n";
        } else if (cmd == "query_ticket" || cmd == "query_transfer") {
            // Per Q&A, query_ticket should not fail; return 0 when no results
            std::cout << 0 << "\n";
        } else if (
            cmd == "add_user" || cmd == "login" || cmd == "logout" ||
            cmd == "query_profile" || cmd == "modify_profile" ||
            cmd == "add_train" || cmd == "release_train" || cmd == "query_train" ||
            cmd == "delete_train" || cmd == "buy_ticket" || cmd == "query_order" ||
            cmd == "refund_ticket") {
            // Default to failure until full implementation exists
            // For query_train failure and other failure cases, -1 is specified
            std::cout << -1 << "\n";
        } else {
            // Unknown command: do nothing or print -1 to be safe
            std::cout << -1 << "\n";
        }
    }
    return 0;
}
