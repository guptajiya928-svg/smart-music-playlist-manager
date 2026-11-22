
# SoundFlow – Smart Music Playlist Manager

SoundFlow is a Smart Music Playlist Manager developed using C++ and Data Structures & Algorithms.

The project helps users manage songs, organize playlists according to moods, and explore music through intelligent playlist features.

## Technologies Used

- C++
- Data Structures and Algorithms
- HTML
- CSS
- JavaScript
- REST APIs
- JSON

## Project Structure

```text
Backend/
│
├── include/
│   ├── Song.h
│   ├── Playlist.h
│   ├── SmartShuffle.h
│   ├── Recommendation.h
│   ├── DurationPlanner.h
│   └── CSVExporter.h
│
├── src/
│   ├── Playlist.cpp
│   ├── MoodClassifier.cpp
│   ├── SmartShuffle.cpp
│   ├── Recommendation.cpp
│   ├── DurationPlanner.cpp
│   └── CSVExporter.cpp
│
├── main.cpp
├── main_backup.cpp
├── httplib.h
├── nlohmann/
└── README.md
```

## Core Concepts

- Linked List for playlist management
- Stack for Undo and Redo operations
- Weighted Graph for Smart Shuffle
- Priority Queue for mood-based recommendations
- Dynamic Programming for duration-based playlist generation
- CSV handling for playlist export
- JSON for song data storage

## Main Features

- Add, delete and search songs
- Mood-based song classification
- Smart Shuffle+
- Mood-based recommendations
- Target-duration playlist generation
- Undo and Redo operations
- CSV playlist export
- Duplicate song detection
- Song data persistence

## Project Purpose

The main objective of SoundFlow is to demonstrate the practical application of Data Structures, Algorithms and Object-Oriented Programming in a real-world music management application.
