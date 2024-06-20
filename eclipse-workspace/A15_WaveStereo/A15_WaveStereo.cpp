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
    //Anzahl Kanaele
    const int channels=2;
    const int sampleRate=48000;
    const char* outfilename="foo.wav";

    cout << "wav_writer..." << endl;

    SndfileHandle outfile(outfilename, SFM_WRITE, format, channels, sampleRate);
    if (not outfile) return -1;

    // prepare a 5 seconds buffer and write it
    const int size = sampleRate*5 * channels;;
    float sample[size], factorleft = 3.0, factorright = 4.0;

    for (int i=0; i<size - 1; i++) {
    	if(i%2)
    	{
    	sample[i]=sin(float(i)/size*M_PI*3000) * sin(float(i)/size*M_PI*factorleft);
    	}
    	else
    	{
    	sample[i] = sin(float(i)/size * M_PI * 4000) * sin(float(i)/size*M_PI*factorright);
    	}
    }
    outfile.write(&sample[0], size);

    cout << "done!" << endl;
    return 0;
}
