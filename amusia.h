#pragma once

#include <sndfile.hh>
#include <cmath>
#include <vector>

namespace amusia
{
	// first declare some generic helpers
	
	const float pi = 3.14159265358979323846F, pi2 = pi * 2;

	float granularize( float value, float size )
	{
		const float steps = floor( value / size );
		return steps * size;
	}

	//  0 < value <= 1
	float granularize( float value, float max, float n )
	{
		const float stepSize = max / n;
		return granularize( value * max, stepSize );
	}

	float curlicue( float i, float k )
	{
		// a = i^2 * k, with a little modular arithmetic to get around overflow (angular arithmetic is modular, by 2*pi)
		return fmod( fmod( i * fmod( i, pi2 ), pi2 ) * k, pi2 );
	}

	namespace scale
	{
		struct EqualTemperament
		{
			EqualTemperament( float notesPerOctave, float adjuster = 0 ) :
				notesPerOctave( notesPerOctave ),
				adjuster( adjuster )
			{}

			float operator()( float n ) const
			{
				return powf( 2, floorf( n + adjuster ) / notesPerOctave );
			}

			const float notesPerOctave;
			const float adjuster;
		};

		float equalTemperament( float n, float notesPerOctave, float adjuster = 0 )
		{
			return EqualTemperament( notesPerOctave, adjuster )( n );
		}

		// tuned to standard tuning ( A440 )
		const auto twelveToneEqualTemperament = EqualTemperament( 12, 0.3764f );
	}

	namespace note
	{
		struct Note
		{
			Note( float first = 0 ) :
				first( first )
			{}
			
			bool operator ==( const Note &other ) const
			{
				return first == other.first;
			}

			bool operator !=( const Note &other ) const
			{
				return first != other.first;
			}

			template < class Octave >
			float operator()( Octave octave ) const
			{
				return scale::twelveToneEqualTemperament( static_cast< float >( first + octave * 12 ) );
			}

			float first;
		};

		const Note
			c		= Note( 0 ),
			cSharp	= Note( 1 ),
			dFlat	= Note( 1 ),
			d		= Note( 2 ),
			dSharp	= Note( 3 ),
			eFlat	= Note( 3 ),
			e		= Note( 4 ),
			eSharp	= Note( 5 ),
			fFlat	= Note( 4 ),
			f		= Note( 5 ),
			fSharp	= Note( 6 ),
			gFlat	= Note( 6 ),
			g		= Note( 7 ),
			gSharp	= Note( 8 ),
			aFlat	= Note( 8 ),
			a		= Note( 9 ),
			aSharp	= Note( 10 ),
			bFlat	= Note( 10 ),
			b		= Note( 11 ),
			bSharp	= Note( 12 ),
			cFlat	= Note( 11 );
	}

	namespace scale
	{
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

			float operator()( note::Note note_, float position ) const
			{
				const auto n = static_cast< std::size_t >( floor( position ) ) % numPositions;
				return twelveToneEqualTemperament( note_.first + positions[ n ] + 12 * floor( position / numPositions ) );
			}

			std::size_t positions[ 12 ];
			std::size_t numPositions;
		};

		const Scale
			major			= Scale( 0, 2, 4, 5, 7, 9, 11 ),
			minor			= Scale( 0, 2, 3, 5, 7, 8, 10 ),
			diminished		= Scale( 0, 3, 6, 9 ),
			augmented		= Scale( 0, 4, 8 ),
			harmonicMinor	= Scale( 0, 2, 3, 5, 7, 8, 11 ),
			majorBlues		= Scale( 0, 2, 3, 4, 7, 9, 10 );
	}

	namespace arpeggio
	{
		typedef scale::Scale Arpeggio;

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

	// returns value between 0 and 1 instead of 0 and 2*pi
	float curlicueNormalized( float i, float k )
	{
		return fmod( fmod( i * fmod( i, pi2 ), pi2 ) * k, pi2 ) / pi2;
	}

	namespace voice
	{
		// a voice is a function taking frequency and time, and returning a number between -1 and 1
		// as time progresses, the voice function should "draw" a wave

		const auto sine = []( float frequency, float time )
		{
			return sinf( frequency * time * pi2 );
		};

		const auto square = []( float frequency, float time )
		{
			return sine( frequency, time ) > 0 ? 1.0f : -1.0f;
		};

		const auto sawtooth = []( float frequency, float time )
		{
			return fmod( frequency * time, 2 ) - 1;
		};

		const auto silent = []( float, float )
		{
			return 0.0f;
		};

		struct Steps
		{
			Steps( float numSteps ) :
				numSteps( numSteps )
			{}

			float operator()( float frequency, float time ) const
			{
				return granularize( sine( frequency, time ) + 1, numSteps ) - 1;
			}

			const float numSteps;
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

		float getDurationSeconds() const
		{
			return durationSeconds;
		}

		template <	class Frequency,	// numeric
					class Amplitude,	// numeric, between 0 and 1 inclusive
					class Seconds,		// numeric
					class Voice >		// float( float frequency, float time )
		void addNote( Frequency frequency, Amplitude amplitude, Seconds seconds, Voice voice )
		{
			const float	fFrequency	= static_cast< float >( frequency ),
						fAmplitude	= static_cast< float >( amplitude ),
						fSampleRate	= static_cast< float >( getSampleRate() ),
						numPoints	= static_cast< float >( fSampleRate * seconds );

			data.reserve( data.size() + static_cast< std::size_t >( numPoints ) );

			for ( float i = 0; i < numPoints; i += 1 )
			{
				data.push_back( (float)voice( fFrequency, durationSeconds + i / fSampleRate ) * fAmplitude );
			}

			durationSeconds += seconds;
		}

		template < class Seconds > // numeric
		void addRest( Seconds seconds )
		{
			addNote( 0, 0, seconds, []( float, float ) { return 0.0F; } );
		}

		bool save()
		{
			return data.size() == file.write( data.data(), data.size() );
		}

	private:

		SndfileHandle file;
		std::vector< float > data;
		float durationSeconds;
	};
}