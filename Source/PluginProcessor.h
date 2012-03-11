/*
  ==============================================================================

    This file was auto-generated by the Jucer!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#ifndef __PLUGINPROCESSOR_H_526ED7A9__
#define __PLUGINPROCESSOR_H_526ED7A9__

#define THUMBNAIL_WIDTH 720


#include "../JuceLibraryCode/JuceHeader.h"
#include "../JuceLibraryCode/JucePluginCharacteristics.h"
#include "AudioSample.h"
#include "SampleStrip.h"
#include "OSCHandler.h"

//==============================================================================
class mlrVSTAudioProcessor : public AudioProcessor,
                             public Timer
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
        pMasterGainParam = 0,
        totalNumParams
    };

    // These settings are independent of preset
    enum GlobalSettings
    {
        sUseExternalTempo,
        sNumChannels,
        sCurrentBPM,
        sOSCPrefix,
        sMonitorInputs,
        sResamplePrecount,
        sResampleLength,
        sResampleBank,
        sRecordPrecount,
        sRecordLength,
        sRecordBank
    };

    enum SamplePool
    {
        pSamplePool,
        pResamplePool,
        pRecordPool
    };


    // adds a sample to the sample pool
    int addNewSample(File &sampleFile);
    AudioSample * getAudioSample(const int &samplePoolIndex, const int &poolID);
    int getSamplePoolSize(const int &index)
    {
        switch (index)
        {
        case pSamplePool : return samplePool.size();
        case pResamplePool : return 8;      // TODO: make this a variable
        case pRecordPool : return 8;
        default : jassertfalse; return -1;
        }

    }
    

    // TODO: bounds checking?
    String getSampleName(const int &index, const int &poolID)
    {
        switch (poolID)
        {
        case pSamplePool: 
            jassert(index < samplePool.size());
            return samplePool[index]->getSampleName();
        case pResamplePool :
            jassert(index < resamplePool.size());
            return resamplePool[index]->getSampleName();
        default : 
            jassertfalse; return "error: pool not found";
        }
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

    void processOSCKeyPress(const int &monomeCol, const int &monomeRow, const bool &state);

    // set up the channels (can be used to change number of channels
    void buildChannelArray(const int &newNumChannels);
    void buildSampleStripArray(const int &numSampleStrips);

    SampleStrip* getSampleStrip(const int &index);
    int getNumSampleStrips();

    float getChannelGain(const int &chan)
    { 
        jassert(chan < channelGains.size());
        return channelGains[chan];
    }
    void setChannelGain(const int &chan, const float &newGain)
    { 
        jassert(chan < channelGains.size());
        channelGains.set(chan, newGain);
    }
    Colour getChannelColour(const int &chan)
    { 
        jassert(chan < channelColours.size());
        return channelColours[chan];
    }


    void setSampleStripParameter(const int &parameterID, const void *newValue, const int &stripID)
    {
        sampleStripArray[stripID]->setSampleStripParam(parameterID, newValue);
    }
    void toggleSampleStripParameter(const int &parameterID, const int &stripID)
    {
        sampleStripArray[stripID]->toggleSampleStripParam(parameterID);
    }
    const void* getSampleStripParameter(const int &parameterID, const int &stripID)
    {
        return sampleStripArray[stripID]->getSampleStripParam(parameterID);   
    }


    // Global Settings stuff
    void updateGlobalSetting(const int &settingID, const void *newVal);
    const void* getGlobalSetting(const int &settingID) const;
    String getGlobalSettingName(const int &settingID) const;


    // Preset stuff
    void savePreset(const String &presetName);
    void switchPreset(const int &id);
    XmlElement getPresetList() const { return presetList; }
    
    XmlElement getSetlist() const { return setlist; }
    void setSetlist(const XmlElement &newSetlist) {
        setlist = newSetlist;
        DBG(setlist.createDocument(String::empty));
    }

    // Recording / resampling stuff
    void startRecording();
    void startResampling();

    float getRecordingPrecountPercent()
    {
        if (recordPrecountPosition <= 0 || recordPrecountLengthInSamples <= 0)
            return 0.0;
        else
            return (float) (recordPrecountPosition) / (float) (recordPrecountLengthInSamples);
    }
    float getRecordingPercent()
    {
        if (recordPosition >= recordLengthInSamples || recordLengthInSamples <= 0)
            return 0.0;
        else
            return (float) (recordPosition) / (float) (recordLengthInSamples);
    }

    float getResamplingPrecountPercent()
    {
        if (resamplePrecountPosition <= 0 || resamplePrecountLengthInSamples <= 0)
            return 0.0;
        else
            return (float) (resamplePrecountPosition) / (float) (resamplePrecountLengthInSamples);
    }
    float getResamplingPercent()
    {
        if (resamplePosition >= resampleLengthInSamples || resampleLengthInSamples <= 0)
            return 0.0;
        else
            return (float) (resamplePosition) / (float) (resampleLengthInSamples);
    }
private:

    /* OSC messages from the monome are converted to MIDI messages
       and stored in monomeState to be mixed in with the main MIDI
       buffer.
    */
    MidiKeyboardState monomeState;

    // Store collection of AudioSamples in memory
    OwnedArray<AudioSample> samplePool;         // for sample files (.wavs etc)
    OwnedArray<AudioSample> resamplePool;       // for recorded internal sounds
    OwnedArray<AudioSample> recordPool;         // for external recordings


    // Max number of channels
    const int maxChannels;

    float masterGain;
    // These are the seperate audio channels
    const float defaultChannelGain;
    // individual channel volumes
    Array<float> channelGains;
    Array<Colour> channelColours;

    /* These track the seperate SampleStrips (related
       to the GUI component SampleStripControl). */
    OwnedArray<SampleStrip> sampleStripArray;
    int numSampleStrips;

    // Send and receive OSC messages through this
    String OSCPrefix;
    OSCHandler oscMsgHandler;
    


    AudioSampleBuffer stripContrib;

    // TODO: make this an array
    AudioSampleBuffer resampleBuffer;
    int resampleLength, resamplePrecountLength;                     // length in bars
    int resampleLengthInSamples, resamplePrecountLengthInSamples;   // length in samples
    int resamplePosition, resamplePrecountPosition; // track resampling progress
    int resampleBank;                               // which slot to use
    bool isResampling;

    AudioSampleBuffer recordBuffer;
    int recordPosition, recordPrecountPosition;
    int recordLengthInSamples, recordPrecountLengthInSamples;
    int recordLength, recordPrecountLength;
    int recordBank;
    bool isRecording;

    bool monitorInputs;

    /////////////////////
    // PRESET HANDLING //
    /////////////////////
    // this is a unique list of possible presets (used internally)
    XmlElement presetList;      
    // this is an ordered list of consisting of a selection from presetList
    XmlElement setlist;         


    /////////////////////
    // GLOBAL SETTINGS //
    /////////////////////
    bool useExternalTempo;
    int numChannels;
    double currentBPM;

    ///////////////////////////
    // BUTTON / LED TRACKING //
    ///////////////////////////

    /* Store which LED column is currently being used
       for displaying playback position.
    */
    Array<int> playbackLEDPosition;

    // Boolean grid which stores the status of button presses
    // where the indices correspond to the row, and col:
    // i.e. buttonStatus[row]->getUnchecked[col] or similar
    OwnedArray<Array<bool> > buttonStatus;

    // This is true when top left button is held down, 
    // so strips can be used to stop, reverse sample etc.
    bool stripModifier;

    void setMonomeStatusGrids(const int &width, const int &height);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (mlrVSTAudioProcessor);
};

#endif  // __PLUGINPROCESSOR_H_526ED7A9__
