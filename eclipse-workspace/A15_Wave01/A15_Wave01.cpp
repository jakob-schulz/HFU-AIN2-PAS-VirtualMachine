//============================================================================
// Name        : A15_Wave01.cpp
// Author      : EC
// Version     :
// Copyright   :
// Description : generates simple WAV-samples
//============================================================================


#include "sndfile.hh"
#include <iostream>

#include <cmath>
using namespace std;

int main()
{
    const int format=SF_FORMAT_WAV | SF_FORMAT_PCM_16;
//  const int format=SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    const int channels=1;
    const int sampleRate=48000;
    const char* outfilename="foo.wav";

    cout << "wav_writer..." << endl;

    SndfileHandle outfile(outfilename, SFM_WRITE, format, channels, sampleRate);
    if (not outfile) return -1;

    // prepare a 5 seconds buffer and write it
    const int size = sampleRate*5;
    float sample[size], factor=1.0;
    for (int i=0; i<size; i++) {
    	sample[i]=sin(float(i)/size*M_PI*3000) * factor;
    }
    outfile.write(&sample[0], size);

    cout << "done!" << endl;
    return 0;
}
