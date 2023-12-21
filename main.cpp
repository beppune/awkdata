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

struct sqlite3_deleter {

    void operator()(sqlite3 *db) const {
        sqlite3_close(db);
    }
};

using count_t = unsigned long;
using sqlite_ptr = std::unique_ptr<sqlite3,sqlite3_deleter>;
#define RESULT_WHITE 1
#define RESULT_BLACK 2
#define RESULT_DRAW  0

const char * const CREATE_DDL =
    "CREATE TABLE games("
        "id     INTEGER PRIMARY KEY AUTOINCREMENT,"
        "result INT NOT NULL"
    ");";

const char * const INSERT_DML = 
    "INSERT INTO games(result) VALUES(?);";

int acc_resut(const std::string &s, count_t &w, count_t &b, count_t &d, count_t &g, count_t &t);

void parse_options(config &conf, int argc, char *argv[]);

sqlite_ptr open_db(const std::string &c);

void dump_result(sqlite_ptr &db, int result);

int main(int argc, char *argv[]) {

    config conf;

    parse_options(conf, argc, argv);

    auto db = open_db(conf.output);
    
    if( db == nullptr) {
       exit(2);
    }

    sqlite3_stmt *stmt;
    int result = sqlite3_prepare(db.get(), INSERT_DML, -1, &stmt, nullptr);
    if( result != SQLITE_OK ) {
        std::cerr << "PREPARE: " << sqlite3_errstr(result) << "\n";
        exit(2);
    }

	size_t done = 0;

	std::istream *inputfile = &std::cin;

	std::string s0;

	count_t white, black, draw, games, stars;
	white = black = draw = games = stars = 0;

	do{
		std::getline(*inputfile, s0);
		result = acc_resut(s0, white, black, draw, games, stars);		
        if( result == 100 ) continue;
		done += s0.size() + 1;
        result = sqlite3_bind_int( stmt, 1, result);
        if( result != SQLITE_OK ) {
            std::cerr << "BIND: " << sqlite3_errstr(result) << "\n";
            exit(1);
        }
        result = sqlite3_step( stmt );
        if( result != SQLITE_DONE ) {
            std::cerr << "STEP: " << sqlite3_errstr(result) << "\n";
            exit(1);
        }
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
	} while( inputfile->good() );

    sqlite3_finalize(stmt);

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
        return nullptr;
    }

    res = sqlite3_exec(db, CREATE_DDL, nullptr, nullptr, nullptr);
    if( res != SQLITE_OK ) {
        std::cerr << sqlite3_errstr(res) << "\n";
        return nullptr;
    }

    return sqlite_ptr(db);
    
}


