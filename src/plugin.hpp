#include "rack0.hpp"
#include <memory>
#include <iostream>
#include <string>
#include <cstdio>

using namespace rack;

// notes by abdul karim from the Noun Project

// Forward-declare the Plugin, defined in Template.cpp
extern Plugin *pluginInstance;

// Forward-declare each Model, defined in each module source file
extern Model *modelGVerbModule;
extern Model *modelDuckModule;
extern Model *modelCV0to10Module;
extern Model *modelCVS0to10Module;
extern Model *modelCV5to5Module;
extern Model *modelCVMmtModule;
extern Model *modelCVTglModule;
extern Model *modelAudioInterface16;
extern Model *modelPianoRollModule;
extern Model *modelSongRollModule;
extern Model *modelButtonTest;
extern Model *modelSEQAdapterModule;

template<typename ... Args>
std::string stringf( const std::string& format, Args ... args )
{
    size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf( new char[ size ] ); 
    snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}