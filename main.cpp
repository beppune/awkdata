#include <iostream>
#include <ostream>
#include <fstream>
#include <string>

using count_t = unsigned long;

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

int main(int argc, char *argv[]) {

	std::istream *inputfile = &std::cin;

	std::string s0;

	count_t white, black, draw, games, stars;
	white = black = draw = games = stars = 0;

	do{
		std::getline(*inputfile, s0);
		acc_resut(s0, white, black, draw, games, stars);
		
	} while( inputfile->good() );

	if( inputfile->bad() ) {
		std::cerr << "Input file critical error" << std::endl;
		return 1;
	}

	std::cout << "\nGames: " << games << "\n"
		<< "White: " << white
		<< ". Black: " << black
		<< ". Draw: " << draw
		<< ". Unknown: " << stars << std::endl;

	if( games != ( white + black + draw + stars ) ) {
		std::cout << "Warning: games count!" << std::endl;
	}

	return 0;
}
