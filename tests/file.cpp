#include <gtest/gtest.h>

#include <cxxmidi/file.hpp>
#include <cxxmidi/note.hpp>

class FileTestFixture : public ::testing::Test {
 protected:
  cxxmidi::File CreateTestFile() {
    const uint32_t dt = cxxmidi::converters::Us2dt(
        100000,  // 0.1s
        500000,  // default tempo [us/quarternote]
        500);    // default time division [us/quarternote]

    cxxmidi::File file;

    // create 3 tracks
    for (int t = 0; t < 3; t++) {
      cxxmidi::Track& track = file.AddTrack();

      for (int i = 0; i < 10; i++) {
        track.push_back(
            cxxmidi::Event(0,                             // deltatime
                           cxxmidi::Message::kNoteOn,     // message type
                           cxxmidi::Note::MiddleC() + i,  // note
                           100 + i));                     // velocity [0...127]
        track.push_back(
            cxxmidi::Event(dt,                            // deltatime
                           cxxmidi::Message::kNoteOn,     // message type
                           cxxmidi::Note::MiddleC() + i,  // note
                           0));  // velocity=0 => note off
      }
      track.push_back(cxxmidi::Event(0,  // deltatime
                                     cxxmidi::Message::kMeta,
                                     cxxmidi::Message::EndOfTrack));
    }

    return file;
  }

  bool CompareFiles(const std::string& p1, const std::string& p2) {
    std::ifstream f1(p1, std::ifstream::binary | std::ifstream::ate);
    std::ifstream f2(p2, std::ifstream::binary | std::ifstream::ate);

    EXPECT_FALSE(f1.fail());
    EXPECT_FALSE(f2.fail());
    EXPECT_EQ(f1.tellg(), f2.tellg()) << "size mismatch";

    // seek back to beginning and use std::equal to compare contents
    f1.seekg(0, std::ifstream::beg);
    f2.seekg(0, std::ifstream::beg);
    return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
                      std::istreambuf_iterator<char>(),
                      std::istreambuf_iterator<char>(f2.rdbuf()));
  }
};

TEST_F(FileTestFixture, Creation) {
  cxxmidi::File file = CreateTestFile();
  EXPECT_EQ(file.Duration().count(), 1000000);
  EXPECT_EQ(file.Tracks(), static_cast<size_t>(3));
}

TEST_F(FileTestFixture, ReadWrite) {
  std::array<std::string, 6> test_files = {
      "../../cxxmidi/music/Abracadabra.mid",
      "../../cxxmidi/music/chopin.mid",
      "../../cxxmidi/music/c_major_scale.mid",
      "../../cxxmidi/music/fantaisie.mid",
      "../../cxxmidi/music/mahavishnu.mid",
      "../../cxxmidi/music/MIDI_sample.mid"};
  cxxmidi::File file;
  for (auto const& f : test_files) {
    file.Load(f.c_str());
    file.SaveAs("test.mid");

    // copmpare original and saved file
    EXPECT_TRUE(CompareFiles(f, "test.mid")) << f;
  }
  // Note: This test will work only for files that were generatated optimally.
}

TEST(FILE, ReadSomeTestFiles) {
  cxxmidi::File file;
  file.Load("../../cxxmidi/music/chopin.mid");
  EXPECT_EQ(file.Tracks(), 8);
  EXPECT_EQ(file[0].size(), 1433);
  EXPECT_EQ(file[1].size(), 1877);
  EXPECT_EQ(file[2].size(), 2061);
  EXPECT_EQ(file[3].size(), 2);
  EXPECT_EQ(file[4].size(), 2);
  EXPECT_EQ(file[5].size(), 2);
  EXPECT_EQ(file[6].size(), 2);
  EXPECT_EQ(file[7].size(), 2);

  file.Load("../../cxxmidi/music/MIDI_sample.mid");
  EXPECT_EQ(file.Tracks(), 6);
  EXPECT_EQ(file[0].size(), 4);
  EXPECT_EQ(file[1].size(), 303);
  EXPECT_EQ(file[3].size(), 161);
  EXPECT_EQ(file[4].size(), 803);
  EXPECT_EQ(file[5].size(), 365);
}
