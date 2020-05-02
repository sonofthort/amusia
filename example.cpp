#include <ctime>
#include <iostream>
#include <sstream>

#include "amusia/amusia.h"

int main() {
  auto make_track = [](double k, int octave, const auto &voice) {
    int i = 0;
    amusia::WaveMemoryBuilder wave;

    auto chord = [&](const amusia::NoteList& chord, std::size_t n) {
      return [chord, n, k, &wave, &i, &voice]() {
        for (const auto end = i + n; i < end; ++i) {
          wave.addNote(amusia::notes::frequency(
                           amusia::curlicueSelectFrom(i, k + 1, chord)),
                       amusia::curlicueNormalized(i, k + 4) * 0.3 + 0.3,
                       1 / 12.0, voice);
        }
      };
    };

    // YOU NEVER GIVE ME YOUR MONEY

    auto firstPassage =
        amusia::chain(chord(amusia::arpeggios::minorSeven.clone()
                                .translate(amusia::notes::a)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            32),
                      chord(amusia::arpeggios::minor.clone()
                                .translate(amusia::notes::d)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            32),
                      chord(amusia::arpeggios::major.clone()
                                .translate(amusia::notes::g)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            32),
                      chord(amusia::arpeggios::major.clone()
                                .translate(amusia::notes::c)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            32),
                      chord(amusia::arpeggios::majorMajorSeven.clone()
                                .translate(amusia::notes::f)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            32),
                      chord(amusia::arpeggios::minorSix.clone()
                                .translate(amusia::notes::d)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            16),
                      chord(amusia::arpeggios::majorSeven.clone()
                                .translate(amusia::notes::e)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            16),
                      chord(amusia::arpeggios::minor.clone()
                                .translate(amusia::notes::a)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            64));

    auto secondPassage =
        amusia::chain(chord(amusia::arpeggios::minorSeven.clone()
                                .translate(amusia::notes::a)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            32),
                      chord(amusia::arpeggios::minor.clone()
                                .translate(amusia::notes::d)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            32),
                      chord(amusia::arpeggios::major.clone()
                                .translate(amusia::notes::g)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            32),
                      chord(amusia::arpeggios::major.clone()
                                .translate(amusia::notes::c)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            32),
                      chord(amusia::arpeggios::majorMajorSeven.clone()
                                .translate(amusia::notes::f)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            32),
                      chord(amusia::arpeggios::minorSix.clone()
                                .translate(amusia::notes::d)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            16),
                      chord(amusia::arpeggios::majorSeven.clone()
                                .translate(amusia::notes::e)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            16),
                      chord(amusia::arpeggios::minor.clone()
                                .translate(amusia::notes::a)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            32),
                      chord(amusia::arpeggios::major.clone()
                                .translate(amusia::notes::c)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            8),
                      chord(amusia::arpeggios::majorSeven.clone()
                                .translate(amusia::notes::g)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            8),
                      chord(amusia::arpeggios::major.clone()
                                .translate(amusia::notes::c)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            16));

    auto thirdPassage =
        amusia::chain(chord(amusia::arpeggios::minorSeven.clone()
                                .translate(amusia::notes::a)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            32),
                      chord(amusia::arpeggios::majorSeven.clone()
                                .translate(amusia::notes::e)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            32),
                      chord(amusia::arpeggios::minor.clone()
                                .translate(amusia::notes::a)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            32),
                      chord(amusia::arpeggios::majorSeven.clone()
                                .translate(amusia::notes::c)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            32),
                      chord(amusia::arpeggios::major.clone()
                                .translate(amusia::notes::f)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            24),
                      chord(amusia::arpeggios::major.clone()
                                .translate(amusia::notes::g)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            16),
                      chord(amusia::arpeggios::major.clone()
                                .translate(amusia::notes::c)
                                .translate_octave(octave)
                                .extend(2)
                                .extend_root(2),
                            16));

    auto song = amusia::chain(amusia::repeat(firstPassage, 2), secondPassage,
                              amusia::repeat(thirdPassage, 2));

    song();

    return wave;
  };

  auto track1 = make_track(3, 6, amusia::voices::circular);
  auto track2 = make_track(7, 7, amusia::voices::square);
  track1.mix(track2);
  track1.toFile("curlicue.wav");

  return 0;
}
