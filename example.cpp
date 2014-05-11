#include <iostream>
#include <sstream>
#include <ctime>
#include "amusia.h"

int main()
{
	using namespace amusia;

	srand(static_cast<unsigned int>(time(nullptr)));

	// 2130 can produce a slow melody
	double k = 3;

	std::ostringstream name;
	name << "curlicue_" << (int)k << ".wav";

	WaveBuilder wave(name.str().c_str(), 1, 48000);

	std::size_t i;

	const auto voice = [](double frequency, double time) -> double {
		const double x = voices::getX(frequency, time);
		return sin(cos(x*2)+sin(x*3)+x);
	};

	auto chord = [&i, &wave, &voice, k](double note, Scale scale, std::size_t n) {
		return Sequence([&i, &wave, &voice, k, note, scale, n]() {
			for (const auto end = i + n; i < end; ++i) {
				wave.addNote(scale(notes::octave(note, 7), floor(curlicueNormalized(static_cast<double>(i), k) * scale.numPositions * 3)),
					.75, 1 / 8.0, voice);
			}
		});
	};

	// YOU NEVER GIVE ME YOUR MONEY

	auto firstPassage = chain(
		chord(notes::a, arpeggios::minorSeven, 32),
		chord(notes::d, arpeggios::minor, 32),
		chord(notes::g, arpeggios::major, 32),
		chord(notes::c, arpeggios::major, 32),
		chord(notes::f, arpeggios::majorMajorSeven, 32),
		chord(notes::d, arpeggios::minorSix, 16),
		chord(notes::e, arpeggios::majorSeven, 16),
		chord(notes::a, arpeggios::minor, 64));

	auto secondPassage = chain(
		chord(notes::a, arpeggios::minorSeven, 32),
		chord(notes::d, arpeggios::minor, 32),
		chord(notes::g, arpeggios::major, 32),
		chord(notes::c, arpeggios::major, 32),
		chord(notes::f, arpeggios::majorMajorSeven, 32),
		chord(notes::d, arpeggios::minorSix, 16),
		chord(notes::e, arpeggios::majorSeven, 16),
		chord(notes::a, arpeggios::minor, 32),
		chord(notes::c, arpeggios::major, 8),
		chord(notes::g, arpeggios::majorSeven, 8),
		chord(notes::c, arpeggios::major, 16));

	auto thirdPassage = chain(
		chord(notes::c, arpeggios::major, 32),
		chord(notes::e, arpeggios::majorSeven, 32),
		chord(notes::a, arpeggios::minor, 32),
		chord(notes::c, arpeggios::majorSeven, 32),
		chord(notes::f, arpeggios::major, 24),
		chord(notes::g, arpeggios::major, 16),
		chord(notes::c, arpeggios::major, 24));

	chain(
		repeat(firstPassage, 2),
		secondPassage,
		repeat(thirdPassage, 2))();

    return 0;
}