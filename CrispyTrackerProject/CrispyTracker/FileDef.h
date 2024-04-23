#pragma once
#include "Patterns.h"
typedef struct Module {
	std::string AuthorName;
	std::string TrackName;
	std::string TracDesc;
	std::vector<Sample> samples;
	std::vector<Instrument> inst;
	std::vector<Patterns> patterns;
};

typedef struct Inst {

};