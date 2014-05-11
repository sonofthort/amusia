#pragma once

#include <sndfile.hh>
#include <functional>
#include <vector>
#include <array>
#include <cmath>

namespace amusia {
	// first declare some generic helpers
	const double tau = 6.283185307179586476925286766559;

	double granularize(double value, double size) {
		const double steps = floor(value / size);
		return steps * size;
	}

	//  0 < value <= 1, but left open for intentional abuse ;)
	double granularize(double value, double max, double n) {
		const double stepSize = max / n;
		return granularize(value * max, stepSize);
	}

	double curlicue(double i, double k) {
		// a = i * i * k, with modular arithmetic to get around overflow (angular arithmetic is modular by tau)
		return fmod(fmod(i * fmod(i, tau), tau) * k, tau);
	}

	// returns value between 0 and 1 instead of 0 and tau
	double curlicueNormalized(double i, double k) {
		return curlicue(i, k) / tau;
	}

	namespace scales {
		struct EqualTemperament {
			double operator()(double n) const {
				return pow(2, floor(n + adjuster) / notesPerOctave);
			}

			const double notesPerOctave;
			const double adjuster;
		};

		double equalTemperament(double n, double notesPerOctave, double adjuster = 0) {
			return EqualTemperament{notesPerOctave, adjuster}(n);
		}

		// tuned to standard tuning (A440)
		const auto twelveToneEqualTemperament = EqualTemperament{12, 0.3764f};
	}

	namespace notes {
		const double
			c		= 0,
			cSharp	= 1,
			dFlat	= 1,
			d		= 2,
			dSharp	= 3,
			eFlat	= 3,
			e		= 4,
			eSharp	= 5,
			fFlat	= 4,
			f		= 5,
			fSharp	= 6,
			gFlat	= 6,
			g		= 7,
			gSharp	= 8,
			aFlat	= 8,
			a		= 9,
			aSharp	= 10,
			bFlat	= 10,
			b		= 11,
			bSharp	= 12,
			cFlat	= 11;

		double frequency(double note) {
			return scales::twelveToneEqualTemperament(note);
		}

		double octave(double note, double octaveAugment) {
			return note + octaveAugment * 12;
		}
	}

	struct Scale {
		template <class First, class... Positions>
		Scale(First first, Positions... positions) {
			static const auto numPositions = sizeof...(Positions) + 1;
			static_assert(numPositions <= 12, "Max 12 positions");

			const std::size_t copyBuffer[12] = {static_cast< std::size_t >(first),
				static_cast< std::size_t >(positions)...};

			memcpy(this->positions, copyBuffer, sizeof(copyBuffer));
			this->numPositions = numPositions;
		}

		Scale() {
			memset(positions, 0, sizeof(positions));
			numPositions = 1;
		}
		
		bool operator==(const Scale &other) const {
			return numPositions == other.numPositions &&
				0 == memcmp(positions, other.positions, sizeof(std::size_t) * numPositions);
		}

		bool operator!=(const Scale &other) const {
			return !(*this == other);
		}

		double operator()(double note, double position) const {
			const auto n = static_cast< std::size_t >(floor(position)) % numPositions;
			return notes::frequency(note + positions[n] + 12 * floor(position / numPositions));
		}

		std::size_t positions[12],
			numPositions;
	};

	namespace scales {
		const Scale
			major			(0, 2, 4, 5, 7, 9, 11),
			minor			(0, 2, 3, 5, 7, 8, 10),
			diminished		(0, 3, 6, 9),
			augmented		(0, 4, 8),
			harmonicMinor	(0, 2, 3, 5, 7, 8, 11),
			majorBlues		(0, 2, 4, 7, 9, 10),
			minorBlues		(0, 2, 3, 7, 8, 10);
	}

	namespace arpeggios {
		const Scale
			major			(0, 4, 7),
			minor			(0, 3, 7),
			diminished		(0, 3, 6),
			augmented		(0, 4, 8),
			majorSix		(0, 4, 7, 9),
			minorSix		(0, 3, 7, 9),
			majorSeven		(0, 4, 7, 10),
			minorSeven		(0, 3, 7, 10),
			majorNine		(0, 4, 7, 10, 14),
			minorNine		(0, 3, 7, 10, 14),
			majorMajorSeven	(0, 4, 7, 11),
			minorMajorSeven	(0, 3, 7, 11),
			majorMajorNine	(0, 4, 7, 11, 13),
			minorMajorNine	(0, 3, 7, 11, 13),
			ambiguous		(0, 3, 4, 7),
			ambiguousSeven	(0, 3, 4, 7, 10);
	}
	
	// a voice is a function taking frequency and time, and returning a number between -1 and 1
	// as time progresses, the voice function should graph a wave at the given frequency
	// think of it like a graphing function, ie y(x) = sin(x), where x = frequency * time * tau
	// xForm functions are voice functions which only take 1 paremeter, x, and are converted to binary voice functions
	// note: favor functors/llambdas when possible, this improves performance by increasing inlining capabilities
	typedef std::function< double(double frequency, double time) > Voice;

	namespace voices {
		double getX(double frequency, double time) {
			return frequency * time * tau;
		}

		// converts unary voice function double(double x) to binary voice form double(double frequency, double time)
		template <class XForm>
		struct FromXForm {
			double operator()(double frequency, double time) const {
				return xForm(getX(frequency, time));
			}

			const XForm xForm;
		};

		template <class XForm>
		FromXForm< XForm > xForm( XForm voice ) {
			return {voice};
		}

		auto sine = xForm([](double x) {
			return sin(x);
		});

		auto cosine = xForm([](double x) {
			return cos(x);
		});

		auto square = xForm([](double x) {
			return sin(x) > 0 ? 1.0 : -1.0;
		});

		auto sawtooth = xForm([](double x) {
			return fmod(x, 2) - 1;
		});

		auto triangle = xForm([](double x) {
			return tan(sin(x));
		});

		auto circular = xForm([](double x) -> double {
			const double sinX = sin(x);
			return sinX < 0 ? -sqrt(-sinX) : sqrt(sinX);
		});

		auto mushy = xForm([](double x) {
			return sin(x + cos(x));
		});

		auto rockOrgan = xForm([](double x) {
			return (sin(2 * x) + sin(2 * x / 3)) * 0.5;
		});


		auto silent = [](double, double) {
			return 0.0;
		};
		
		template <class VoiceA, class VoiceB>
		Voice split(VoiceA a, VoiceB b) {
			return [a, b](double frequency, double time) {
				return sine(frequency, time) > 0 ? a(frequency, time) : b(frequency, time);
			};
		}

		template <class VoiceA, class VoiceB>
		Voice mix(VoiceA a, VoiceB b, double interval) {
			return [a, b, interval](double frequency, double time) {
				return fmod(time, interval) > (interval * 0.5) ? a(frequency, time) : b(frequency, time);
			};
		}

		template <class VoiceA, class VoiceB>
		Voice multiply(VoiceA a, VoiceB b) {
			return [a, b](double frequency, double time) {
				return a(frequency, time) * b(frequency, time);
			};
		}

		template <class Voice_>
		Voice granularize(Voice_ voice, double n) {
			const double stepSize = 2 / n;
			return [voice, stepSize](double frequency, double time) {
				return ::amusia::granularize(voice(frequency, time) + 1, stepSize) - 1;
			};
		}

		template <class Voice_>
		Voice exponentiate(Voice_ voice, double exponent) {
			return [voice, exponent](double frequency, double time) {
				return pow(voice(frequency, time), exponent);
			};
		}

		template <class Voice_>
		Voice cube(Voice_ voice) {
			return exponentiate(voice, 3);
		}
		
		// works best with rational exponents
		Voice zappy(double exponent) {
			return xForm([exponent](double x) {
				return sin(x + sin(pow(x, exponent)));
			});
		}

		template <std::size_t dividend, std::size_t divisor>
		const Voice &zappy() {
			static const Voice voice = zappy(static_cast< double >(dividend) / divisor);
			return voice;
		}

		Voice organ(double multiplier, double divisor) {
			const double divisorMinus1 = divisor - 1;
			return xForm([multiplier, divisor, divisorMinus1](double x) {
				return (divisorMinus1 * sin(x) + sin(x * multiplier)) / divisor;
			});
		}

		Voice clarinet(double multiplier) {
			return xForm([multiplier](double x) {
				return sin(x + sin(multiplier * x));
			});
		}

		const Voice
			sine_split_sawtooth = split(sine, sawtooth),
			square_split_sawtooth = split(square, sawtooth),
			sine_x_sawtooth = multiply(sine, sawtooth),
			sine_cubed = cube(sine),
			zappy_1_2 = zappy< 1, 2 >(),
			zappy_3_2 = zappy< 3, 2 >();
	}

	struct WaveBuilder {
		WaveBuilder(const char *fileName, int numChannels, int sampleRate, int format = SF_FORMAT_WAV | SF_FORMAT_PCM_16) :
			file(fileName, SFM_WRITE, format, numChannels, sampleRate),
			durationSeconds(0) {}

		int getSampleRate() const {
			return file.samplerate();
		}

		int getNumChannels() const {
			return file.channels();
		}

		double getDurationSeconds() const {
			return durationSeconds;
		}

		template < class Frequency,	// numeric
			class Amplitude,	// numeric, between 0 and 1 inclusive
			class Seconds,		// numeric
			class Voice_>		// voice func
		void addNote(Frequency frequency, Amplitude amplitude, Seconds seconds, const Voice_ &voice) {
			const double
				dFrequency	= static_cast< double >(frequency),
				dAmplitude	= static_cast< double >(amplitude),
				dSampleRate	= static_cast< double >(getSampleRate()),
				numPoints	= static_cast< double >(dSampleRate * seconds);

			buffer.reserve(static_cast< std::size_t >(numPoints));

			for (double i = 0; i < numPoints; i += 1) {
				buffer.push_back(static_cast< float >(voice(dFrequency, durationSeconds + i / dSampleRate) * dAmplitude));
			}

			durationSeconds += seconds;
			file.write(buffer.data(), buffer.size());
			buffer.clear();
		}

		template <class Seconds> // numeric
		void addRest(Seconds seconds) {
			addNote(0, 0, seconds, voices::silent);
		}

	private:

		SndfileHandle file;
		std::vector< float > buffer;
		double durationSeconds;
	};	

	// A sequence is just a function of the form void func()
	// it would typically write some notes to a WaveBuilder
	// and you would typically use llambdas to simplify this
	// 
	// example:
	// [frequencies, &wave]() { for (auto frequency : frequencies) wave.addNote(frequency, 0.75, 1 / 8.0, voices::sine ); }
	typedef std::function< void() > Sequence;

	template <class... Sequences>
	Sequence chain(Sequences... sequences) {
		std::array< Sequence, sizeof...(Sequences) > sequenceArray = {sequences...};
		return [sequenceArray]() {
			for (const auto &sequence : sequenceArray) {
				sequence();
			}
		};
	}

	template <class N>
	Sequence repeat(Sequence sequence, N n) {
		return [sequence, n]() {
			for (N i = 0; i < n; i = i + 1) {
				sequence();
			}
		};
	}
}