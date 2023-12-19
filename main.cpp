#include <iostream>
#include <ostream>
#include <fstream>
#include <string>
#include <cstdio>

#include "cxxopts.hpp"

struct config {
    std::string createscript;
    std::string output;
};

using count_t = unsigned long;

void acc_resut(const std::string &s, count_t &w, count_t &b, count_t &d, count_t &g, count_t &t);

void parse_options(config &conf, int argc, char *argv[]);

int main(int argc, char *argv[]) {

	bool is_verbose = false;

	parse_options(is_verbose, argc, argv);

	size_t done = 0;

	std::istream *inputfile = &std::cin;

	std::string s0;

	count_t white, black, draw, games, stars;
	white = black = draw = games = stars = 0;

	do{
		std::getline(*inputfile, s0);
		acc_resut(s0, white, black, draw, games, stars);		
		done += s0.size() + 1;
		if( done % 1000000 == 0 ) {
			//verbose_print(std::format("\rParsed {} MBytes", done/1000000));
		}
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

void acc_resut(const std::string &s, count_t &w, count_t &b, count_t &d, count_t &g, count_t &t) {
	if( s.starts_with("[Result") ) {
		g++;
		auto i = s.find_first_of('-', 9);
		if( i == s.npos ) {
			t++;
			return;
		}
		auto c = s.at(i - 1);
		switch(c) {
			case '1':
				w++;
				break;
			case '0':
				b++;
				break;
			case '2':
				d++;
				break;
		}

	}
}

void parse_options(config &conf, int argc, char *argv[]) {
	cxxopts::Options opts("","");
	opts.add_options()
		("c,create-script, "Database creation script in case of new dump file", cxxopts::value<std::string>()->default_value("ddl.sql"))
        ("output", "Database file to dump data. If new requires option --create-script", cxxopts::value<std::string>()->default_value("pgn.dump"));

	auto res = opts.parse(argc, argv);

	conf.createscript = res["create-script"].as<std::string>();
}
