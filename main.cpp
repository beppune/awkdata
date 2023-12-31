#include <iostream>
#include <ostream>
#include <fstream>
#include <string>
#include <cstdio>

#include "cxxopts.hpp"

extern "C" {
#include "sqlite3/sqlite3.h"
}

struct config {
    std::string createscript;
    std::string output;
};

void sqlite3_deleter(sqlite3 *db) {
    sqlite3_close(db);
    db = nullptr;
};

void stmt_finalizer(sqlite3_stmt *p) {
    sqlite3_finalize(p);
    p = nullptr;
};

using count_t = unsigned long;
using sqlite_ptr = std::unique_ptr<sqlite3, void(&)(sqlite3 *)>;
using stmt_ptr = std::unique_ptr<sqlite3_stmt, void(&)(sqlite3_stmt*)>;

#define RESULT_WHITE 1
#define RESULT_BLACK 2
#define RESULT_DRAW  0

const char * const CREATE_DDL =
"CREATE TABLE games("
"id     INTEGER PRIMARY KEY AUTOINCREMENT,"
"result TEXT NOT NULL"
");";

const char * const INSERT_DML = 
"INSERT INTO games(result) VALUES(?);";

int acc_resut(const std::string &s, count_t &w, count_t &b, count_t &d, count_t &g, count_t &t);

void parse_options(config &conf, int argc, char *argv[]);

sqlite_ptr open_db(const std::string &c);

stmt_ptr prepare_st(sqlite_ptr &sq, const char *SQL);

void exec_st(stmt_ptr &st, int data);

void dump_result(sqlite_ptr &db, int result);

int main(int argc, char *argv[]) {

    config conf;

    parse_options(conf, argc, argv);

    auto db = open_db(conf.output);

    if( db == nullptr) {
        exit(2);
    }

    auto stmt = prepare_st(db, INSERT_DML);

    size_t done = 0;

    std::istream *inputfile = &std::cin;

    std::string s0;

    count_t white, black, draw, games, stars;
    white = black = draw = games = stars = 0;

    int result;

    do{
        std::getline(*inputfile, s0);
        result = acc_resut(s0, white, black, draw, games, stars);		
        if( result == 100 ) continue;

        exec_st(stmt, result);

        done += s0.size() + 1;
    } while( inputfile->good() );

    if( inputfile->bad() ) {
        std::cerr << "Input file critical error" << std::endl;
        return 1;
    }

    std::cout << "\rGames: " << games << "\n"
        << "White: " << white
        << ". Black: " << black
        << ". Draw: " << draw
        << ". Unknown: " << stars << std::endl;

    if( games != ( white + black + draw + stars ) ) {
        std::cout << "Warning: games count!" << std::endl;
    }

    return 0;
}

int acc_resut(const std::string &s, count_t &w, count_t &b, count_t &d, count_t &g, count_t &t) {
    if( s.starts_with("[Result") ) {
        g++;
        auto i = s.find_first_of('-', 9);
        if( i == s.npos ) {
            t++;
            return -1;
        }
        auto c = s.at(i - 1);
        switch(c) {
            case '1':
                w++;
                return RESULT_WHITE;
            case '0':
                b++;
                return RESULT_BLACK;
            case '2':
                d++;
                return RESULT_DRAW;
        }

    }
    return 100;
}

void parse_options(config &conf, int argc, char *argv[]) {
    cxxopts::Options opts("","");
    opts.add_options()
        ("output", "Database file to dump data. If new requires option --create-script", cxxopts::value<std::string>()->default_value("pgn.dump"));

    auto res = opts.parse(argc, argv);

    conf.output = std::move(res["output"].as<std::string>());
}

sqlite_ptr open_db(const std::string &c) {

    sqlite3 *db;
    int res = sqlite3_open_v2(c.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if( res != SQLITE_OK ) {
        std::cerr << sqlite3_errstr(res) << "\n";
        return sqlite_ptr(nullptr, sqlite3_deleter);
    }

    res = sqlite3_exec(db, CREATE_DDL, nullptr, nullptr, nullptr);
    if( res != SQLITE_OK ) {
        std::cerr << sqlite3_errstr(res) << "\n";
        return sqlite_ptr(nullptr, sqlite3_deleter);
    }

    return sqlite_ptr(db, sqlite3_deleter);

}

stmt_ptr prepare_st(sqlite_ptr &db, const char *SQL) {
    sqlite3_stmt *stmt;

    int result = sqlite3_prepare(db.get(), SQL, -1, &stmt, nullptr);
    if( result != SQLITE_OK ) {
        std::cerr << "PREPARE: " << sqlite3_errstr(result) << "\n";
        exit(2);
    }

    return stmt_ptr(stmt, stmt_finalizer);

}

void exec_st(stmt_ptr &stmt, int data) {

    int result = -1;
    switch(data) {
        case RESULT_WHITE:
            result = sqlite3_bind_text( stmt.get(), 1, "W", -1, SQLITE_STATIC);
            break;

        case RESULT_BLACK:
            result = sqlite3_bind_text( stmt.get(), 1, "B", -1, SQLITE_STATIC);
            break;
        
        case RESULT_DRAW:
            result = sqlite3_bind_text( stmt.get(), 1, "D", -1, SQLITE_STATIC);
            break;
    }

    if( result != SQLITE_OK ) {
        std::cerr << "BIND: " << sqlite3_errstr(data) << "\n";
        exit(1);
    }
    result = sqlite3_step( stmt.get() );
    if( result != SQLITE_DONE ) {
        std::cerr << "STEP: " << sqlite3_errstr(data) << "\n";
        exit(1);
    }
    sqlite3_reset(stmt.get());
}

