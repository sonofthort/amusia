#include <iostream>
#include <sstream>
#include <ctime>
#include "amusia.h"

// Created by Eric Thortsen 05/08/2014

int main()
{
	using namespace amusia;

	srand( static_cast< unsigned int >( time( nullptr ) ) );
	
	double	n = 2000,
			notelengthSeconds = 1 / 8.0f,
			k = 777;

	std::ostringstream name;
	name << "curlicue_" << (int)k << '_' << (int)n << ".wav";

	WaveBuilder wave( name.str().c_str(), 1, 48000 );
	wave.reserve( n * notelengthSeconds );

	double note;
	Arpeggio arpeggio;	

	for ( float i = 0; i < n; i += 1 )
	{
		switch ( (int)i % 64 )
		{
		case 0:
			note = notes::c;
			arpeggio = scales::majorBlues;
			std::cout << "c major\n";
			break;
		case 15:
			note = notes::a;
			arpeggio = arpeggios::minorSeven;
			std::cout << "a minor\n";
			break;
		case 31:
			note = notes::f;
			arpeggio = arpeggios::majorMajorSeven;
			std::cout << "f major\n";
			break;
		case 47:
			note = notes::g;
			arpeggio = arpeggios::majorSeven;
			std::cout << "g major\n";
			break;
		}

		const double
			normalizedA = curlicueNormalized( i, k ),
			frequency = arpeggio( note + 12 * 7, floor( normalizedA * arpeggio.numPositions * 3 ) );

		wave.addNote( frequency, .75, notelengthSeconds, voices::Steps( 8 ) );
	}

	wave.save();

    return 0;
}