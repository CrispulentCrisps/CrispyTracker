#pragma once
class SoundGenerator
{
public:
	int TotalVolume;
	int Hz;
	int NoteIndex;//Going from C0 to C8
	int NotePos;//Position of note in channel
	int NoteIndex;
	float Tuning;

	bool IsPlaying;
};