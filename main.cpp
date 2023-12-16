#include <iostream>
#include <ostream>
#include <fstream>
#include <string>

using count_t = unsigned long;

void acc_resut(const std::string &s, count_t &w, count_t &b, count_t &d, count_t &g) {
	if( s.starts_with("[Result") ) {
		g++;
		auto i = s.find_first_of('-', 9);
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

	if( argc < 2) {
		std::cerr << argv[0] << " <file.pgn>" << std::endl;
		return 1;
	}

	std::ifstream inputfile(argv[1]);

	std::string s0;

	count_t white, black, draw, games;
	white = black = draw = games = 0;

	do{
		std::getline(inputfile, s0);
		acc_resut(s0, white, black, draw, games);
	} while( inputfile.good() );

	std::cout << "Games: " << games << "\n"
		<< "White: " << white
		<< ". Black: " << black
		<< ". Draw: " << draw  << std::endl;

	if( games != ( white + black + draw ) ) {
		std::cout << "Warning: games count!" << std::endl;
	}

	return 0;
}
