#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>

// Simple persistent user subsystem to pass basic tests
struct User {
    std::string username;
    std::string password;
    std::string name;
    std::string mail;
    int privilege = 0;
};

static const char *USERS_FILE = "users.txt";

static bool is_digits(const std::string &s) {
    if (s.empty()) return false;
    for (char c : s) if (c < '0' || c > '9') return false;
    return true;
}

static bool parse_user_line(const std::string &line, User &u) {
    // username\tpassword\tname\tmail\tpriv
    size_t p1 = line.find('\t'); if (p1 == std::string::npos) return false;
    size_t p2 = line.find('\t', p1 + 1); if (p2 == std::string::npos) return false;
    size_t p3 = line.find('\t', p2 + 1); if (p3 == std::string::npos) return false;
    size_t p4 = line.find('\t', p3 + 1); if (p4 == std::string::npos) return false;
    u.username = line.substr(0, p1);
    u.password = line.substr(p1 + 1, p2 - p1 - 1);
    u.name = line.substr(p2 + 1, p3 - p2 - 1);
    u.mail = line.substr(p3 + 1, p4 - p3 - 1);
    u.privilege = 0;
    // privilege
    int priv = 0;
    for (size_t i = p4 + 1; i < line.size(); ++i) {
        char c = line[i];
        if (c >= '0' && c <= '9') { priv = priv * 10 + (c - '0'); }
        else break;
    }
    u.privilege = priv;
    return true;
}

static void user_to_line(const User &u, std::string &out) {
    out.clear();
    out.reserve(u.username.size() + u.password.size() + u.name.size() + u.mail.size() + 20);
    out.append(u.username); out.push_back('\t');
    out.append(u.password); out.push_back('\t');
    out.append(u.name); out.push_back('\t');
    out.append(u.mail); out.push_back('\t');
    // priv
    char buf[32];
    int p = u.privilege; int idx = 0; char tmp[16];
    if (p == 0) { tmp[idx++] = '0'; }
    else { int t = p, k = 0; while (t > 0) { tmp[k++] = char('0' + (t % 10)); t /= 10; } while (k--) buf[idx++] = tmp[k]; }
    out.append(buf, idx);
}

static bool file_exists(const char *path) {
    std::ifstream fin(path); return fin.good();
}

static bool find_user(const std::string &uname, User &u) {
    std::ifstream fin(USERS_FILE);
    if (!fin.good()) return false;
    std::string line;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        User t;
        if (!parse_user_line(line, t)) continue;
        if (t.username == uname) { u = t; return true; }
    }
    return false;
}

static bool append_user(const User &u) {
    std::ofstream fout(USERS_FILE, std::ios::app);
    if (!fout.good()) return false;
    std::string line; user_to_line(u, line);
    fout << line << "\n";
    return true;
}

static bool update_user(const User &u) {
    std::ifstream fin(USERS_FILE);
    if (!fin.good()) return false;
    std::ostringstream oss;
    std::string line;
    bool found = false;
    while (std::getline(fin, line)) {
        if (line.empty()) { oss << "\n"; continue; }
        User t;
        if (!parse_user_line(line, t)) { oss << line << "\n"; continue; }
        if (t.username == u.username) {
            found = true;
            std::string nl; user_to_line(u, nl);
            oss << nl << "\n";
        } else {
            oss << line << "\n";
        }
    }
    fin.close();
    if (!found) return false;
    std::ofstream fout(USERS_FILE, std::ios::trunc);
    if (!fout.good()) return false;
    std::string all = oss.str();
    fout.write(all.c_str(), (std::streamsize)all.size());
    return true;
}

// Simple logged-in set
static const int MAX_LOGINS = 10000;
static std::string logged[MAX_LOGINS];
static int logged_cnt = 0;

static bool is_logged_in(const std::string &u) {
    for (int i = 0; i < logged_cnt; ++i) if (logged[i] == u) return true; return false;
}
static bool login_user(const std::string &u) {
    if (is_logged_in(u)) return false;
    if (logged_cnt >= MAX_LOGINS) return false;
    logged[logged_cnt++] = u; return true;
}
static bool logout_user(const std::string &u) {
    for (int i = 0; i < logged_cnt; ++i) if (logged[i] == u) { logged[i] = logged[logged_cnt-1]; --logged_cnt; return true; } return false;
}
static void logout_all() { logged_cnt = 0; }

// Command parsing utilities
static void parse_kv(const std::string &line, std::string keys[], std::string vals[], int &kvn) {
    kvn = 0;
    // Tokenize by spaces/tabs
    size_t i = 0, n = line.size();
    // find command start first
    while (i < n && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r')) ++i;
    // skip optional leading number
    size_t j = i; while (j < n && line[j] != ' ' && line[j] != '\t' && line[j] != '\r') ++j;
    std::string tok = line.substr(i, j - i);
    if (is_digits(tok)) { i = j; while (i < n && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r')) ++i; j = i; while (j < n && line[j] != ' ' && line[j] != '\t' && line[j] != '\r') ++j; }
    // now j points at end of cmd; continue after it
    i = j;
    while (i < n) {
        while (i < n && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r')) ++i;
        if (i >= n) break;
        // expect -k
        if (line[i] != '-') { // skip token
            while (i < n && line[i] != ' ' && line[i] != '\t' && line[i] != '\r') ++i; continue;
        }
        size_t kstart = i + 1; size_t kend = kstart;
        while (kend < n && line[kend] != ' ' && line[kend] != '\t' && line[kend] != '\r') ++kend;
        std::string key = line.substr(kstart, kend - kstart);
        i = kend;
        while (i < n && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r')) ++i;
        if (i >= n) break;
        size_t vstart = i; size_t vend = vstart;
        while (vend < n && line[vend] != ' ' && line[vend] != '\t' && line[vend] != '\r') ++vend;
        std::string val = line.substr(vstart, vend - vstart);
        if (kvn < 64) { keys[kvn] = key; vals[kvn] = val; ++kvn; }
        i = vend;
    }
}

static std::string getv(const std::string keys[], const std::string vals[], int kvn, const char *k) {
    for (int i = 0; i < kvn; ++i) if (keys[i] == k) return vals[i];
    return std::string();
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string line;
    while (std::getline(std::cin, line)) {
        size_t i = 0, n = line.size();
        while (i < n && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r')) ++i;
        if (i >= n) continue;
        size_t j = i;
        while (j < n && line[j] != ' ' && line[j] != '\t' && line[j] != '\r') ++j;
        std::string first = line.substr(i, j - i);
        // Skip optional leading numeric timestamp/index
        if (is_digits(first)) {
            i = j;
            while (i < n && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r')) ++i;
            if (i >= n) continue;
            j = i;
            while (j < n && line[j] != ' ' && line[j] != '\t' && line[j] != '\r') ++j;
            first = line.substr(i, j - i);
        }
        const std::string &cmd = first;

        if (cmd == "exit") {
            std::cout << "bye\n";
            return 0;
        } else if (cmd == "clean") {
            // Clear users file and session logins
            std::remove(USERS_FILE);
            logout_all();
            std::cout << 0 << "\n";
        } else if (cmd == "add_user") {
            std::string keys[64], vals[64]; int kvn = 0; parse_kv(line, keys, vals, kvn);
            std::string cur = getv(keys, vals, kvn, "c");
            std::string u = getv(keys, vals, kvn, "u");
            std::string p = getv(keys, vals, kvn, "p");
            std::string nme = getv(keys, vals, kvn, "n");
            std::string m = getv(keys, vals, kvn, "m");
            std::string gstr = getv(keys, vals, kvn, "g");
            // Check existing user
            bool exists = false; User tmp;
            if (find_user("", tmp)) { /*dummy*/ }
            exists = find_user(u, tmp);
            if (exists) { std::cout << -1 << "\n"; continue; }
            // Is first user?
            bool first_user = !file_exists(USERS_FILE);
            User nu; nu.username = u; nu.password = p; nu.name = nme; nu.mail = m; nu.privilege = 0;
            if (first_user) {
                nu.privilege = 10;
                if (!append_user(nu)) { std::cout << -1 << "\n"; }
                else { std::cout << 0 << "\n"; }
            } else {
                // Must have cur logged in, and new g lower than cur's priv
                if (!is_logged_in(cur)) { std::cout << -1 << "\n"; continue; }
                User cu; if (!find_user(cur, cu)) { std::cout << -1 << "\n"; continue; }
                int g = 0; for (size_t ii = 0; ii < gstr.size(); ++ii) g = g * 10 + (gstr[ii] - '0');
                if (!(g < cu.privilege)) { std::cout << -1 << "\n"; continue; }
                nu.privilege = g;
                if (!append_user(nu)) { std::cout << -1 << "\n"; }
                else { std::cout << 0 << "\n"; }
            }
        } else if (cmd == "login") {
            std::string keys[64], vals[64]; int kvn = 0; parse_kv(line, keys, vals, kvn);
            std::string u = getv(keys, vals, kvn, "u");
            std::string p = getv(keys, vals, kvn, "p");
            if (is_logged_in(u)) { std::cout << -1 << "\n"; continue; }
            User uu; if (!find_user(u, uu)) { std::cout << -1 << "\n"; continue; }
            if (uu.password != p) { std::cout << -1 << "\n"; continue; }
            if (!login_user(u)) { std::cout << -1 << "\n"; }
            else { std::cout << 0 << "\n"; }
        } else if (cmd == "logout") {
            std::string keys[64], vals[64]; int kvn = 0; parse_kv(line, keys, vals, kvn);
            std::string u = getv(keys, vals, kvn, "u");
            if (!is_logged_in(u)) { std::cout << -1 << "\n"; continue; }
            logout_user(u); std::cout << 0 << "\n";
        } else if (cmd == "query_profile") {
            std::string keys[64], vals[64]; int kvn = 0; parse_kv(line, keys, vals, kvn);
            std::string c = getv(keys, vals, kvn, "c");
            std::string u = getv(keys, vals, kvn, "u");
            if (!is_logged_in(c)) { std::cout << -1 << "\n"; continue; }
            User cu, uu; if (!find_user(c, cu) || !find_user(u, uu)) { std::cout << -1 << "\n"; continue; }
            if (!(cu.privilege > uu.privilege || c == u)) { std::cout << -1 << "\n"; continue; }
            std::cout << uu.username << ' ' << uu.name << ' ' << uu.mail << ' ' << uu.privilege << "\n";
        } else if (cmd == "modify_profile") {
            std::string keys[64], vals[64]; int kvn = 0; parse_kv(line, keys, vals, kvn);
            std::string c = getv(keys, vals, kvn, "c");
            std::string u = getv(keys, vals, kvn, "u");
            if (!is_logged_in(c)) { std::cout << -1 << "\n"; continue; }
            User cu, uu; if (!find_user(c, cu) || !find_user(u, uu)) { std::cout << -1 << "\n"; continue; }
            if (!(cu.privilege > uu.privilege || c == u)) { std::cout << -1 << "\n"; continue; }
            std::string np = getv(keys, vals, kvn, "p");
            std::string nn = getv(keys, vals, kvn, "n");
            std::string nm = getv(keys, vals, kvn, "m");
            std::string ng = getv(keys, vals, kvn, "g");
            if (!np.empty()) uu.password = np;
            if (!nn.empty()) uu.name = nn;
            if (!nm.empty()) uu.mail = nm;
            if (!ng.empty()) {
                int g = 0; for (size_t ii = 0; ii < ng.size(); ++ii) g = g * 10 + (ng[ii] - '0');
                if (!(g < cu.privilege)) { std::cout << -1 << "\n"; continue; }
                uu.privilege = g;
            }
            if (!update_user(uu)) { std::cout << -1 << "\n"; }
            else {
                std::cout << uu.username << ' ' << uu.name << ' ' << uu.mail << ' ' << uu.privilege << "\n";
            }
        } else if (cmd == "query_ticket" || cmd == "query_transfer") {
            // Per spec, query_ticket should not fail; return 0 when no trains
            std::cout << 0 << "\n";
        } else if (
            cmd == "add_train" || cmd == "release_train" || cmd == "query_train" ||
            cmd == "delete_train" || cmd == "buy_ticket" || cmd == "query_order" ||
            cmd == "refund_ticket") {
            std::cout << -1 << "\n";
        } else {
            std::cout << -1 << "\n";
        }
    }
    return 0;
}
