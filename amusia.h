#pragma once

#include <array>
#include <cmath>
#include <functional>
#include <sndfile.hh>
#include <vector>

namespace amusia {
// first declare some generic helpers
constexpr double tau = 6.283185307179586476925286766559;
constexpr double phi = 0.61803398874989484820458683436564;

inline double granularize(double value, double size) {
  const double steps = std::floor(value / size);
  return steps * size;
}

//  0 < value <= 1, but left open for intentional abuse ;)
inline double granularize(double value, double max, double n) {
  const double stepSize = max / n;
  return granularize(value * max, stepSize);
}

inline double curlicue(double i, double k) {
  // a = i * i * k, with modular arithmetic to get around overflow (angular
  // arithmetic is modular by tau)
  return std::fmod(std::fmod(i * std::fmod(i, tau), tau) * k, tau);
}

// returns value between 0 and 1 instead of 0 and tau
inline double curlicueNormalized(double i, double k) {
  return curlicue(i, k) / tau;
}

template <class T>
T curlicueSelect(double i, double k, T n) {
  return static_cast<T>(std::floor(n * curlicueNormalized(i, k)));
}

inline bool curlicueOdds(double i, double k, double odds = 0.5) {
  return curlicueNormalized(i, k) < odds;
}

template <class ArrayLike>
auto& curlicueSelectFrom(double i, double k, const ArrayLike& values) {
  return values[curlicueSelect(i, k, std::size(values))];
}

namespace scales {
struct EqualTemperament {
  double operator()(double n) const {
    return std::pow(2, std::floor(n + adjuster) / notesPerOctave);
  }

  const double notesPerOctave;
  const double adjuster;
};

inline double equalTemperament(double n, double notesPerOctave,
                               double adjuster = 0) {
  return EqualTemperament{notesPerOctave, adjuster}(n);
}

// tuned to standard tuning (A440)
const auto twelveToneEqualTemperament = EqualTemperament{12, 0.3764f};
}  // namespace scales

namespace notes {
constexpr int c = 0;
constexpr int cSharp = 1;
constexpr int dFlat = 1;
constexpr int d = 2;
constexpr int dSharp = 3;
constexpr int eFlat = 3;
constexpr int e = 4;
constexpr int eSharp = 5;
constexpr int fFlat = 4;
constexpr int f = 5;
constexpr int fSharp = 6;
constexpr int gFlat = 6;
constexpr int g = 7;
constexpr int gSharp = 8;
constexpr int aFlat = 8;
constexpr int a = 9;
constexpr int aSharp = 10;
constexpr int bFlat = 10;
constexpr int b = 11;
constexpr int bSharp = 12;
constexpr int cFlat = 11;

inline double frequency(int note) {
  return scales::twelveToneEqualTemperament(static_cast<double>(note));
}

inline int octave(int note, int octaveAugment, int notes_per_octave = 12) {
  return note + octaveAugment * notes_per_octave;
}
}  // namespace notes

struct NoteList {
  NoteList() = default;
  NoteList(const NoteList&) = default;
  NoteList(NoteList&&) = default;
  NoteList(std::initializer_list<int> notes) : notes_(std::move(notes)) {}

  using iterator = std::vector<int>::iterator;
  using const_iterator = std::vector<int>::const_iterator;

  NoteList clone() const { return *this; }

  NoteList& push(int note) {
    notes_.push_back(note);
    return *this;
  }

  NoteList& push(std::initializer_list<int> notes) {
    notes_.insert(notes_.end(), std::move(notes));
    return *this;
  }

  NoteList& translate(int amount) {
    for (int& note : notes_) {
      note += amount;
    }
    return *this;
  }

  NoteList& translate_octave(int octave_amount, int notes_per_octave = 12) {
    return translate(octave_amount * notes_per_octave);
  }

  NoteList& extend(int number_of_octaves, int notes_per_octave = 12) {
    const std::size_t original_size = notes_.size();
    for (int octave = 1; octave <= number_of_octaves; ++octave) {
      const int offset = octave * notes_per_octave;
      for (std::size_t i = 0; i < original_size; ++i) {
        notes_.push_back(notes_[i] + offset);
      }
    }
    return *this;
  }

  NoteList& extend_root(int number_of_octaves = 1, int notes_per_octave = 12) {
    notes_.push_back(notes_[0] + number_of_octaves * notes_per_octave);
    return *this;
  }

  NoteList& sort() {
    std::sort(begin(), end());
    return *this;
  }

  iterator find(int note) { return std::find(begin(), end(), note); }

  const_iterator find(int note) const {
    return std::find(begin(), end(), note);
  }

  bool contains(int note) const { return find(note) != end(); }

  bool operator==(const NoteList& other) const {
    return notes_ == other.notes_;
  }

  bool operator!=(const NoteList& other) const {
    return notes_ != other.notes_;
  }

  bool operator<(const NoteList& other) const { return notes_ < other.notes_; }

  bool operator<=(const NoteList& other) const {
    return notes_ <= other.notes_;
  }

  bool operator>(const NoteList& other) const { return notes_ > other.notes_; }

  bool operator>=(const NoteList& other) const {
    return notes_ >= other.notes_;
  }

  iterator begin() { return notes_.begin(); }
  const_iterator begin() const { return notes_.begin(); }
  iterator end() { return notes_.end(); }
  const_iterator end() const { return notes_.end(); }

  const std::size_t size() const { return notes_.size(); }

  int& operator[](std::size_t index) { return notes_[index]; }
  const int& operator[](std::size_t index) const { return notes_[index]; }

 private:
  std::vector<int> notes_;
};

namespace scales {
const NoteList major = {0, 2, 4, 5, 7, 9, 11};
const NoteList minor = {0, 2, 3, 5, 7, 8, 10};
const NoteList harmonicMinor = {0, 2, 3, 5, 7, 8, 11};
const NoteList majorBlues = {0, 2, 4, 7, 9, 10};
const NoteList minorBlues = {0, 2, 3, 7, 8, 10};
}  // namespace scales

namespace arpeggios {
const NoteList major = {0, 4, 7};
const NoteList minor = {0, 3, 7};
const NoteList diminished = {0, 3, 6};
const NoteList diminishedSeven = {0, 3, 6, 9};
const NoteList augmented = {0, 4, 8};
const NoteList majorSix = {0, 4, 7, 9};
const NoteList minorSix = {0, 3, 7, 9};
const NoteList majorSeven = {0, 4, 7, 10};
const NoteList minorSeven = {0, 3, 7, 10};
const NoteList majorNine = {0, 4, 7, 10, 14};
const NoteList minorNine = {0, 3, 7, 10, 14};
const NoteList majorMajorSeven = {0, 4, 7, 11};
const NoteList minorMajorSeven = {0, 3, 7, 11};
const NoteList majorMajorNine = {0, 4, 7, 11, 13};
const NoteList minorMajorNine = {0, 3, 7, 11, 13};
}  // namespace arpeggios

// a voice is a function taking frequency and time, and returning a number
// between -1 and 1 as time progresses, the voice function should graph a wave
// at the given frequency think of it like a graphing function, ie y(x) =
// sin(x), where x = frequency * time * tau xForm functions are voice functions
// which only take 1 paremeter, x, and are converted to binary voice functions
// note: favor functors/llambdas when possible, this improves performance by
// increasing inlining capabilities
using Voice = std::function<double(double frequency, double time)>;

namespace voices {
inline double getX(double frequency, double time) {
  return frequency * time * tau;
}

// converts unary voice function double(double x) to binary voice form
// double(double frequency, double time)
template <class XForm>
auto xForm(XForm voice) {
  return [voice](double frequency, double time) {
    return voice(getX(frequency, time));
  };
}

const auto sine = xForm([](double x) { return sin(x); });
const auto cosine = xForm([](double x) { return cos(x); });
const auto square = xForm([](double x) { return sin(x) > 0 ? 1.0 : -1.0; });
const auto sawtooth = xForm([](double x) { return fmod(x, 2) - 1; });
const auto triangle = xForm([](double x) { return tan(sin(x)); });
const auto mushy = xForm([](double x) { return sin(x + cos(x)); });
const auto silent = [](double, double) { return 0.0; };

const auto circular = xForm([](double x) -> double {
  const double sinX = sin(x);
  return sinX < 0 ? -sqrt(-sinX) : sqrt(sinX);
});

const auto rockOrgan =
    xForm([](double x) { return (sin(2 * x) + sin(2 * x / 3)) * 0.5; });

template <class VoiceA, class VoiceB>
auto split(VoiceA a, VoiceB b) {
  return [a, b](double frequency, double time) {
    return sine(frequency, time) > 0 ? a(frequency, time) : b(frequency, time);
  };
}

template <class VoiceA, class VoiceB>
auto mix(VoiceA a, VoiceB b, double interval) {
  return [a, b, interval](double frequency, double time) {
    return fmod(time, interval) > (interval * 0.5) ? a(frequency, time)
                                                   : b(frequency, time);
  };
}

template <class VoiceA, class VoiceB>
auto multiply(VoiceA a, VoiceB b) {
  return [a, b](double frequency, double time) {
    return a(frequency, time) * b(frequency, time);
  };
}

template <class Voice_>
auto granularize(Voice_ voice, double n) {
  const double stepSize = 2 / n;
  return [voice, stepSize](double frequency, double time) {
    return ::amusia::granularize(voice(frequency, time) + 1, stepSize) - 1;
  };
}

template <class Voice_>
auto exponentiate(Voice_ voice, double exponent) {
  return [voice, exponent](double frequency, double time) {
    return pow(voice(frequency, time), exponent);
  };
}

template <class Voice_>
auto cube(Voice_ voice) {
  return exponentiate(voice, 3);
}

// works best with rational exponents
inline auto zappy(double exponent) {
  return xForm([exponent](double x) { return sin(x + sin(pow(x, exponent))); });
}

template <std::size_t dividend, std::size_t divisor>
const auto& zappy() {
  static const Voice voice = zappy(static_cast<double>(dividend) / divisor);
  return voice;
}

inline auto organ(double multiplier, double divisor) {
  const double divisorMinus1 = divisor - 1;
  return xForm([multiplier, divisor, divisorMinus1](double x) {
    return (divisorMinus1 * sin(x) + sin(x * multiplier)) / divisor;
  });
}

inline auto clarinet(double multiplier) {
  return xForm([multiplier](double x) { return sin(x + sin(multiplier * x)); });
}

const auto sine_split_sawtooth = split(sine, sawtooth);
const auto square_split_sawtooth = split(square, sawtooth);
const auto sine_x_sawtooth = multiply(sine, sawtooth);
const auto sine_cubed = cube(sine);
const auto& zappy_1_2 = zappy<1, 2>();
const auto& zappy_3_2 = zappy<3, 2>();
}  // namespace voices

template <class T>
struct BasicWaveFileBuilder {
  BasicWaveFileBuilder(const char* filename, int sampleRate = 48000,
                       int format = SF_FORMAT_WAV | SF_FORMAT_PCM_16)
      : file(filename, SFM_WRITE, format, 1, sampleRate), durationSeconds(0) {}

  int getSampleRate() const { return file.samplerate(); }

  int getNumChannels() const { return file.channels(); }

  double getDurationSeconds() const { return durationSeconds; }

  template <class Frequency,  // numeric
            class Amplitude,  // numeric, between 0 and 1 inclusive
            class Seconds,    // numeric
            class VoiceType>  // voice func
  void addNote(Frequency frequency, Amplitude amplitude, Seconds seconds,
               const VoiceType& voice) {
    const double dFrequency = static_cast<double>(frequency),
                 dAmplitude = static_cast<double>(amplitude),
                 dSampleRate = static_cast<double>(getSampleRate()),
                 numPoints = static_cast<double>(dSampleRate * seconds);

    buffer.reserve(static_cast<std::size_t>(numPoints));

    for (double i = 0; i < numPoints; i += 1.0) {
      buffer.push_back(static_cast<T>(
          voice(dFrequency, durationSeconds + i / dSampleRate) * dAmplitude));
    }

    durationSeconds += seconds;
    file.write(buffer.data(), buffer.size());
    buffer.clear();
  }

  template <class Seconds>  // numeric
  void addRest(Seconds seconds) {
    addNote(0, 0, seconds, voices::silent);
  }

 private:
  SndfileHandle file;
  std::vector<T> buffer;
  double durationSeconds = 0;
};

using WaveFileBuilder = BasicWaveFileBuilder<double>;

template <class T>
struct BasicWaveMemoryBuilder {
  BasicWaveMemoryBuilder(int sampleRate = 48000) : sampleRate(sampleRate) {
    // TODO: Check valid sample rate. Should not be <= 0 (and should not be
    // other invalid sample rates)
  }

  int getSampleRate() const { return sampleRate; }

  int getNumChannels() const { return 1; }

  double getDurationSeconds() const { return durationSeconds; }

  template <class Frequency,  // numeric
            class Amplitude,  // numeric, between 0 and 1 inclusive
            class Seconds,    // numeric
            class VoiceType>  // voice func
  void addNote(Frequency frequency, Amplitude amplitude, Seconds seconds,
               const VoiceType& voice) {
    const double dFrequency = static_cast<double>(frequency),
                 dAmplitude = static_cast<double>(amplitude),
                 dSampleRate = static_cast<double>(getSampleRate()),
                 numPoints = static_cast<double>(dSampleRate * seconds);

    for (double i = 0; i < numPoints; i += 1.0) {
      buffer.push_back(static_cast<T>(
          voice(dFrequency, durationSeconds + i / dSampleRate) * dAmplitude));
    }

    durationSeconds += seconds;
  }

  template <class Seconds>  // numeric
  void addRest(Seconds seconds) {
    addNote(0, 0, seconds, voices::silent);
  }

  bool toFile(const char* filename,
              int format = SF_FORMAT_WAV | SF_FORMAT_PCM_16) const {
    SndfileHandle file(filename, SFM_WRITE, format, 1, sampleRate);
    file.write(buffer.data(), buffer.size());
    // FIXME: Do real error handling
    return true;
  }

  void clear() {
    buffer.clear();
    durationSeconds = 0;
  }

  void mix(const BasicWaveMemoryBuilder& wave, T weight = T(0.5)) {
    const std::size_t n = std::min(buffer.size(), wave.buffer.size());
    const auto a = buffer.data();
    const auto b = wave.buffer.data();
    const T my_weight = 1 - weight;
    for (std::size_t i = 0; i < n; ++i) {
      a[i] = my_weight * a[i] + weight * b[i];
    }
  }

  static BasicWaveMemoryBuilder mix_to(
      std::vector<const BasicWaveMemoryBuilder*> waves) {
    if (waves.empty()) {
      return {};
    }
    auto shortest = waves[0];
    for (const auto wave : waves) {
      if (wave->getDurationSeconds() < shortest->getDurationSeconds()) {
        shortest = wave;
      }
    }
    auto result = *shortest;
    double i = 2;
    for (const auto wave : waves) {
      if (wave != shortest) {
        result.mix(*wave, 1 / i);
        i += 1;
      }
    }
    return result;
  }

 private:
  std::vector<T> buffer;
  int sampleRate = 0;
  double durationSeconds = 0;
};

using WaveMemoryBuilder = BasicWaveMemoryBuilder<double>;

// A sequence is just a function of the form void func()
// it would typically write some notes to a WaveBuilder
// and you would typically use llambdas to simplify this
//
// example:
// [frequencies, &wave]() { for (auto frequency : frequencies)
// wave.addNote(frequency, 0.75, 1 / 8.0, voices::sine ); }
using Sequence = std::function<void()>;

template <class... Sequences>
auto chain(Sequences... sequences) {
  std::array<Sequence, sizeof...(Sequences)> sequenceArray = {
      std::move(sequences)...};
  return [sequenceArray]() {
    for (const auto& sequence : sequenceArray) {
      sequence();
    }
  };
}

template <class SequenceType, class N>
auto repeat(SequenceType sequence, N n) {
  return [sequence, n]() {
    for (N i = 0; i < n; i = i + 1) {
      sequence();
    }
  };
}
}  // namespace amusia
