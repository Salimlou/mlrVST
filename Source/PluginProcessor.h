/*
  ==============================================================================

    This file was auto-generated by the Jucer!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#ifndef __PLUGINPROCESSOR_H_526ED7A9__
#define __PLUGINPROCESSOR_H_526ED7A9__


#include "../JuceLibraryCode/JuceHeader.h"
#include "../JuceLibraryCode/JucePluginCharacteristics.h"
#include "ChannelProcessor.h"
#include "AudioSample.h"
#include "SampleStrip.h"

#include "OSCHandler.h"

//==============================================================================
/**
*/
class mlrVSTAudioProcessor : public AudioProcessor, public Timer
{
public:
    //==============================================================================
    mlrVSTAudioProcessor();
    ~mlrVSTAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
    void reset();

    //==============================================================================
    bool hasEditor() const                  { return true; }
    AudioProcessorEditor* createEditor();

    //==============================================================================
    const String getName() const            { return JucePlugin_Name; }

    int getNumParameters();
    float getParameter (int index);
    void setParameter (int index, float newValue);
    const String getParameterName (int index);
    const String getParameterText (int index);

    const String getInputChannelName (int channelIndex) const;
    const String getOutputChannelName (int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    bool acceptsMidi() const;
    bool producesMidi() const;

    //==============================================================================
    int getNumPrograms()                                                { return 0; }
    int getCurrentProgram()                                             { return 0; }
    void setCurrentProgram (int /*index*/)                              { }
    const String getProgramName (int /*index*/)                         { return String::empty; }
    void changeProgramName (int /*index*/, const String& /*newName*/)   { }

    ChannelProcessor* getChannelProcessor(const int &index)
    {
        return channelProcessorArray[index];
    }

    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);


    // this keeps a copy of the last set of time info that was acquired during an audio
    // callback - the UI component will read this and display it.
    AudioPlayHead::CurrentPositionInfo lastPosInfo;

    void timerCallback();

    //==============================================================================
    // Note we may not need all these parameters, but if the host is to allow them,
    // to be automatable, we need to allow for the possibility.
    enum Parameters
    {
        masterGainParam = 0,
        delayParam,
        channel0GainParam,
        channel1GainParam,
        channel2GainParam,
        channel3GainParam,
        channel4GainParam,
        channel5GainParam,
        channel6GainParam,
        channel7GainParam,
        totalNumParams
    };

    float masterGain;
    // individual channel volumes
    Array<float> channelGains;

    // adds a sample to the sample pool
    int addNewSample(File &sampleFile);
    AudioSample * getAudioSample(const int &samplePoolIndex);
    int getSamplePoolSize() { return samplePool.size(); }

    // TODO: bounds checking?
    String getSampleName(const int &index)
    {
        jassert(index < samplePool.size());
        return samplePool[index]->getSampleName();
    }

    // Returns a pointer to the sample in the sample pool at the specified index
    AudioSample* getSample(const int &index)
    {
        jassert(index < samplePool.size());
        return samplePool[index];
    }
    
    AudioSample* getLatestSample() { return samplePool.getLast(); }

    void calcInitialPlaySpeed(const int &stripID);
    void calcPlaySpeedForNewBPM(const int &stripID);
    void calcPlaySpeedForSelectionChange(const int &stripID);
    void modPlaySpeed(const double &factor, const int &stripID);
    void switchChannels(const int &newChan, const int &stripID);

    void processOSCKeyPress(const int &monomeCol, const int &monomeRow, const int &state);

    // set up the channels (can be used to change number of channels
    void buildChannelProcessorArray(const int &newNumChannels);
    void buildSampleStripArray(const int &numSampleStrips);

    SampleStrip* getSampleStrip(const int &index);

    void setSampleStripParameter(const int &parameterID, const void *newValue, const int &stripID)
    {
        sampleStripArray[stripID]->setSampleStripParam(parameterID, newValue);
    }

    const void* getSampleStripParameter(const int &parameterID, const int &stripID)
    {
        return sampleStripArray[stripID]->getSampleStripParam(parameterID);   
    }


    // Preset stuff
    void savePreset(const String &presetName);

private:

    /* OSC messages from the monome are converted to MIDI messages
       and stored in monomeState to be mixed in with the main MIDI
       buffer.
    */
    MidiKeyboardState monomeState;

    // Store collection of AudioSamples in memory
    OwnedArray<AudioSample> samplePool;

    double currentBPM;

    // Current / max number of channels
    const int maxChannels;
    int numChannels;

    // These are the seperate audio channels
    OwnedArray<ChannelProcessor> channelProcessorArray; 
    const float defaultChannelGain;
        
    /* These track the seperate SampleStrips (related
       to the GUI component SampleStripControl). */
    OwnedArray<SampleStrip> sampleStripArray;
    int numSampleStrips;

    // Send and receive OSC messages through this
    OSCHandler oscMsgHandler;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (mlrVSTAudioProcessor);
};

#endif  // __PLUGINPROCESSOR_H_526ED7A9__
