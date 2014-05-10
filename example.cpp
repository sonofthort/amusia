#include <iostream>
#include <sstream>
#include <ctime>
#include "amusia.h"

// Created by Eric Thortsen 05/08/2014

int main()
{
	using namespace amusia;

	srand( static_cast< unsigned int >( time( nullptr ) ) );
	
	float	n = 2000,
			notelengthSeconds = 1 / 8.0f,
			k = 341;

	std::ostringstream name;
	name << "curlicue_" << (int)k << '_' << (int)n << ".wav";
	WaveBuilder wav( name.str().c_str(), 1, 48000 );
	wav.reserve( n * notelengthSeconds );
	note::Note note_;
	arpeggio::Arpeggio arpeggio_;	

	for ( float i = 0; i < n; i += 1 )
	{
		switch ( (int)i % 64 )
		{
		case 0:
			note_ = note::c;
			arpeggio_ = arpeggio::majorMajorSeven;
			std::cout << "c major\n";
			break;
		case 15:
			note_ = note::a;
			arpeggio_ = arpeggio::minorSeven;
			std::cout << "a minor\n";
			break;
		case 31:
			note_ = note::f;
			arpeggio_ = arpeggio::majorMajorSeven;
			std::cout << "f major\n";
			break;
		case 47:
			note_ = note::g;
			arpeggio_ = arpeggio::majorSeven;
			std::cout << "g major\n";
			break;
		}

		const float normalizedA = curlicueNormalized( i, k );
		const float frequency = arpeggio_( note_, floor( normalizedA * arpeggio_.numPositions * 3 ) + arpeggio_.numPositions * 7 );
		wav.addNote( frequency, .75, notelengthSeconds, voice::sine );
	}

	wav.save();

    return 0;
}