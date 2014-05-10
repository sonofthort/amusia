#pragma once

#include <sndfile.hh>
#include <cmath>
#include <vector>

namespace amusia
{
	// first declare some generic helpers
	
	const double tau = 6.283185307179586476925286766559;

	double granularize( double value, double size )
	{
		const double steps = floor( value / size );
		return steps * size;
	}

	//  0 < value <= 1
	double granularize( double value, double max, double n )
	{
		const double stepSize = max / n;
		return granularize( value * max, stepSize );
	}

	double curlicue( double i, double k )
	{
		// a = i^2 * k, with a little modular arithmetic to get around overflow (angular arithmetic is modular, by 2*pi)
		return fmod( fmod( i * fmod( i, tau ), tau ) * k, tau );
	}

	// returns value between 0 and 1 instead of 0 and tau
	double curlicueNormalized( double i, double k )
	{
		return curlicue( i, k ) / tau;
	}

	namespace scales
	{
		struct EqualTemperament
		{
			EqualTemperament( double notesPerOctave, double adjuster = 0 ) :
				notesPerOctave( notesPerOctave ),
				adjuster( adjuster )
			{}

			double operator()( double n ) const
			{
				return pow( 2, floor( n + adjuster ) / notesPerOctave );
			}

			const double notesPerOctave;
			const double adjuster;
		};

		double equalTemperament( double n, double notesPerOctave, double adjuster = 0 )
		{
			return EqualTemperament( notesPerOctave, adjuster )( n );
		}

		// tuned to standard tuning ( A440 )
		const auto twelveToneEqualTemperament = EqualTemperament( 12, 0.3764f );
	}

	namespace notes
	{
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
	}
	
	struct Scale
	{
		Scale()
		{
			memset( positions, 0, sizeof( positions ) );
			numPositions = 1;
		}

		template < class First, class... Positions >
		Scale( First first, Positions... positions )
		{
			static const auto numPositions = sizeof...( Positions ) + 1;
			static_assert( numPositions <= 12, "Max 12 positions" );
			const std::size_t copyBuffer[ 12 ] = { static_cast< std::size_t >( first ),
				static_cast< std::size_t >( positions )... };
			memcpy( this->positions, copyBuffer, sizeof( copyBuffer ) );
			this->numPositions = numPositions;
		}
			
		bool operator ==( const Scale &other ) const
		{
			return numPositions == other.numPositions &&
				0 == memcmp( positions, other.positions, sizeof( std::size_t ) * numPositions );
		}

		bool operator !=( const Scale &other ) const
		{
			return !( *this == other );
		}

		double operator()( double note, double position ) const
		{
			const auto n = static_cast< std::size_t >( floor( position ) ) % numPositions;
			return scales::twelveToneEqualTemperament( note + positions[ n ] + 12 * floor( position / numPositions ) );
		}

		std::size_t positions[ 12 ];
		std::size_t numPositions;
	};

	namespace scales
	{
		const Scale
			major			= Scale( 0, 2, 4, 5, 7, 9, 11 ),
			minor			= Scale( 0, 2, 3, 5, 7, 8, 10 ),
			diminished		= Scale( 0, 3, 6, 9 ),
			augmented		= Scale( 0, 4, 8 ),
			harmonicMinor	= Scale( 0, 2, 3, 5, 7, 8, 11 ),
			majorBlues		= Scale( 0, 2, 4, 7, 9, 10 );
	}

	typedef Scale Arpeggio;

	namespace arpeggios
	{
		const Arpeggio
			major				= Arpeggio( 0, 4, 7 ),
			minor				= Arpeggio( 0, 3, 7 ),
			diminished			= Arpeggio( 0, 3, 6 ),
			augmented			= Arpeggio( 0, 4, 8 ),
			majorSeven			= Arpeggio( 0, 4, 7, 10 ),
			minorSeven			= Arpeggio( 0, 3, 7, 10 ),
			majorMajorSeven		= Arpeggio( 0, 4, 7, 11 ),
			minorMajorSeven		= Arpeggio( 0, 3, 7, 11 ),
			ambiguous			= Arpeggio( 0, 3, 4, 7 ),
			ambiguousSeven		= Arpeggio( 0, 3, 4, 7, 10 );
	}

	namespace voices
	{
		// a voice is a function taking frequency and time, and returning a number between -1 and 1
		// as time progresses, the voice function should "draw" a wave at the given frequency

		const auto sine = []( double frequency, double time )
		{
			return sin( frequency * time * tau );
		};

		const auto square = []( double frequency, double time )
		{
			return sine( frequency, time ) > 0 ? 1.0 : -1.0;
		};

		const auto sawtooth = []( double frequency, double time )
		{
			return fmod( frequency * time, 2 ) - 1.0;
		};

		const auto silent = []( double, double )
		{
			return 0.0;
		};

		struct Steps
		{
			Steps( double numSteps ) :
				numSteps( numSteps )
			{}

			double operator()( double frequency, double time ) const
			{
				return granularize( sine( frequency, time ) + 1, 2 / numSteps ) - 1;
			}

			const double numSteps;
		};
	}

	struct WaveBuilder
	{
		WaveBuilder( const char *fileName, int numChannels, int sampleRate, int format = SF_FORMAT_WAV | SF_FORMAT_PCM_16 ) :
			file( fileName, SFM_WRITE, format, numChannels, sampleRate ),
			durationSeconds( 0 )
		{}

		// makes a huge performance improvement if reserved for whole length before adding notes
		template < class Seconds >
		void reserve( Seconds seconds )
		{
			data.reserve( (std::size_t)( getSampleRate() * seconds ) );
		}

		int getSampleRate() const
		{
			return file.samplerate();
		}

		int getNumChannels() const
		{
			return file.channels();
		}

		double getDurationSeconds() const
		{
			return durationSeconds;
		}

		template <	class Frequency,	// numeric
					class Amplitude,	// numeric, between 0 and 1 inclusive
					class Seconds,		// numeric
					class Voice >		// double( double frequency, double time )
		void addNote( Frequency frequency, Amplitude amplitude, Seconds seconds, Voice voice )
		{
			const double
				dFrequency	= static_cast< double >( frequency ),
				dAmplitude	= static_cast< double >( amplitude ),
				dSampleRate	= static_cast< double >( getSampleRate() ),
				numPoints	= static_cast< double >( dSampleRate * seconds );

			data.reserve( data.size() + static_cast< std::size_t >( numPoints ) );

			for ( double i = 0; i < numPoints; i += 1 )
			{
				data.push_back( static_cast< float >( voice( dFrequency, durationSeconds + i / dSampleRate ) * dAmplitude ) );
			}

			durationSeconds += seconds;
		}

		template < class Seconds > // numeric
		void addRest( Seconds seconds )
		{
			addNote( 0, 0, seconds, voices::silent );
		}

		bool save()
		{
			return data.size() == file.write( data.data(), data.size() );
		}

	private:

		SndfileHandle file;
		std::vector< float > data;
		double durationSeconds;
	};
}