#include "pxt.h"

#include "SAMD21DAC.h"
#include "Synthesizer.h"
#include "DeviceConfig.h"

#ifndef HAS_SPEAKER
#define HAS_SPEAKER (PIN_SPEAKER != NC)
#endif

class WSynthesizer {
  public:
    SAMD21DMAC dmac;
    Synthesizer synth;
    SAMD21DAC dac;

    WSynthesizer()
        : dac(*lookupPin(PIN_SPEAKER), dmac, synth.output) {
        synth.setVolume(0);
    }
};
SINGLETON(WSynthesizer);

enum class SoundOutputDestination {
    //% block="speaker"
    Speaker = 0,
    //% block="pin"
    Pin = 1
};

namespace music {

int synthVolume = 400;
PwmPin pitchPin;

#if HAS_SPEAKER
SoundOutputDestination current = SoundOutputDestination::Speaker;
#else
SoundOutputDestination current = SoundOutputDestination::Pin;
#endif

/**
* Sets the output destination for sounds generated by the music package
* @param out the destination for sounds generated by the music package
*/
//% weight=2
//% blockId=music_set_output block="set output %out"
//% parts="speaker" blockGap=8 advanced=true
void setOutput(SoundOutputDestination out) {
    current = out;
}

/**
* Sets the pin to output sound to when the sound output is set to "pin"
* @param pin the pin to generate sound on
*/
//% weight=1
//% blockId=music_set_pitch_pin block="set pitch pin %pin"
//% parts="speaker" blockGap=8 advanced=true
void setPitchPin(PwmPin pin) {
    pitchPin = pin;
}

/**
* Sets the output volume of the on board speaker (if available)
* @param volume the volume 0...100, eg: 50
*/
//% weight=96
//% blockId=synth_set_volume block="set speaker volume %volume"
//% parts="speaker" blockGap=8
//% volume.min=0 volume.max=100
void setSpeakerVolume(int volume) {
    synthVolume = max(0, min(100, volume)) * 10.23;
}

/**
* Plays a tone through the pin for the given duration.
* @param frequency pitch of the tone to play in Hertz (Hz)
* @param ms tone duration in milliseconds (ms)
*/
//% help=music/play-tone weight=90
//% blockId=music_play_note block="play tone|at %note=device_note|for %duration=device_beat"
//% parts="headphone" async
//% blockNamespace=music
void playTone(int frequency, int ms) {
    if (current == SoundOutputDestination::Speaker) {
        auto synth = &getWSynthesizer()->synth;

        if (frequency <= 0) {
            synth->setVolume(0);
            fiber_sleep(ms);
        } else {
            synth->setVolume(synthVolume);

            if (ms > 0) {
                synth->setFrequency((float) frequency, ms);
                fiber_sleep(ms);
                synth->setVolume(0);
                fiber_sleep(20);
            } else {
                synth->setFrequency((float) frequency);                
            }
        }
    }
    else {
        if (NULL == pitchPin)
            pitchPin = getPin(PIN_A10);        
        if (frequency <= 0) {
            pitchPin->setAnalogValue(0);
        } else {
            pitchPin->setAnalogValue(512);
            pitchPin->setAnalogPeriodUs(1000000/frequency);
        }

        if (ms > 0) {
            fiber_sleep(ms);
            pitchPin->setAnalogValue(0);
            wait_ms(5);
        }
    }
}
}