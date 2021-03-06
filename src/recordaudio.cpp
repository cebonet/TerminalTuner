#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <sstream>
#include "recordaudio.h"

#define WINDOW_SIZE 1024
#define WIN_HAMMING 1

#define NUM_CHANNELS    (1)
/* #define DITHER_FLAG     (paDitherOff)  */
#define DITHER_FLAG     (0) /**/
/* Select sample format. */
#define PA_SAMPLE_TYPE  paFloat32
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"

using namespace std;

recordaudio::recordaudio(bool verbose){
    err = Pa_Initialize();
        
    cout << "Select recording device - press enter for default:" << endl;
    for (int i = 0, end = Pa_GetDeviceCount(); i != end; ++i) {
        PaDeviceInfo const* info = Pa_GetDeviceInfo(i);
        if (!info) continue;
        cout << i << ") " <<  info->name << endl;
    }
    
    if (std::cin.peek() == '\n') {  //check if next character is newline
        device = Pa_GetDefaultInputDevice(); /* default input device */
    } else if (!(std::cin >> device)) { //be sure to handle invalid input
        if (device > Pa_GetDeviceCount() || device < 0){
            cout << "Not a valid device" << endl;
        }else{
            std::cout << "Invalid input.\n";
        }
        device = -1;
    }
    inputParameters.device = device; /* default input device */
    
}

bool recordaudio::openStream(){
    if (device == -1){
        return false;
    }

    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No input device.\n");
    }

    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    samplerate = Pa_GetDeviceInfo(inputParameters.device)->defaultSampleRate;
    
    if( err != paNoError ) 
        error();
    
    err = Pa_OpenStream(
              &stream,
              &inputParameters,
              NULL,     /* &outputParameters, */
              samplerate , 
              WINDOW_SIZE,
              paClipOff,/* we won't output out of range samples so don't bother clipping them */
              NULL,     /* no callback, use blocking API */
              NULL );   /* no callback, so no callback userData */
    
    err = Pa_StartStream( stream );
    cout << "Now recording!!" << endl;
    if( err != paNoError ) 
        error();

    return true;
}


bool recordaudio::getAudioStream(double *buffer){
    err = Pa_StartStream( stream );
    err = Pa_ReadStream(stream, buffer, WINDOW_SIZE);
    if( err != paNoError ){
        error();
        return false;
    }
    return true;
}

void recordaudio::closeStream(){
    err = Pa_CloseStream( stream );
    if( err != paNoError ) 
        error();
}

void recordaudio::error(){
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
}


