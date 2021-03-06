/*
  ==============================================================================

    This file was auto-generated by the Jucer!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#ifndef __PLUGINPROCESSOR_H_526ED7A9__
#define __PLUGINPROCESSOR_H_526ED7A9__

#define THUMBNAIL_WIDTH 720
#define NUM_ROWS 8
#define NUM_COLS 8

#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioSample.h"
#include "OSCHandler.h"
#include "PatternRecording.h"
#include "SampleStrip.h"

//==============================================================================
class mlrVSTAudioProcessor : public AudioProcessor,
                             public ChangeBroadcaster,
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
        sPresetName,
        sNumChannels,
        sMonomeSize,
        sNumMonomeRows,
        sNumMonomeCols,
        sNumSampleStrips,
        sCurrentBPM,
        sQuantiseLevel,             // options None (-1), 1/1, 1/2, 1/4, etc
        sQuantiseMenuSelection,     // allows menu to be correctly selected
        sOSCPrefix,
        sMonitorInputs,
        sRecordPrecount,
        sRecordLength,
        sRecordBank,
        sResamplePrecount,
        sResampleLength,
        sResampleBank,
        sPatternPrecount,
        sPatternLength,
        sPatternBank,
        sRampLength,                // length of volume envelope (in samples)
        NumGlobalSettings
    };
    
    enum GlobalSettingType
    {
        TypeError = -1,
        TypeInt,
        TypeDouble,
        TypeFloat,
        TypeBool,
        TypeString,
        TypeAudioSample
    };

    enum GlobalSettingScope
    {
        ScopeError = -1,    // there's been an error somwhere
        ScopeNone,          // this setting is not saved
        ScopePreset,        // this setting is saved with every preset
        ScopeSetlist        // this setting is saved once (per setlist)
    };

    // do we save this setting once at top of setlist, with every
    // preset, or not at all?
    static int writeGlobalSettingToPreset(const int &settingID);
    // get the setting name (for writing in presets etc)
    static String getGlobalSettingName(const int &settingID);
    static int getGlobalSettingID(const String &settingName);
    // is this setting an int, bool, etc
    static int getGlobalSettingType(const int &parameterID);
    // setters / getters
    void updateGlobalSetting(const int &settingID, const void *newVal, const bool &notifyListeners = true);
    // TODO: should we have a getIntGlobalSetting etc?
    const void* getGlobalSetting(const int &settingID) const;
    


    enum MonomeSizes
    {
        eightByEight = 1,
        eightBySixteen,
        sixteenByEight,
        sixteenBySixteen,
        numSizes
    };

    enum SamplePool
    {
        mNoMapping,
        pSamplePool,
        pResamplePool,
        pRecordPool
    };

    // Mapping stuff ////////////////////

    // these are the possible options for mappings for the top row(s)
    enum TopRowMappings
    {
        tmNoMapping,
        tmModifierBtnA,
        tmModifierBtnB,
        tmStartRecording,
        tmStartResampling,
        tmStopAll,
        tmTapeStopAll,
        tmPatternModifierBtn,
        numTopRowMappings
    };

    // these are the various options for mapping normal
    // rows (for when the modifier is held)
    enum NormalRowMappings
    {
        nmNoMapping,
        nmFindBestTempo,
        nmToggleReverse,
		nmCycleThruChannels,
        nmDecVolume,
        nmIncVolume,
        nmDecPlayspeed,
        nmIncPlayspeed,
        nmHalvePlayspeed,
        nmDoublePlayspeed,
        nmSetNormalPlayspeed,
        nmStopPlayback,
        nmStopPlaybackTape,
        nmCycleThruRecordings,
        nmCycleThruResamplings,
        nmCycleThruFileSamples,
        numNormalRowMappings
    };

    enum PatternMappings
    {
        patmapNoMapping,
        patmapStartRecording,
        patmapStopRecording,
        patmapStartPlaying,
        patmapStopPlaying,
        patmapNumMappings
    };

    enum RowMappingType
    {
        rmNoBtn = -1,
        rmNormalRowMappingBtnA,
        rmNormalRowMappingBtnB,
        rmPatternBtn,
        rmTopRowMapping
    };

    //
    int getMonomeMapping(const int &rowType, const int &col) const;
    void setMonomeMapping(const int &rowType, const int &col, const int &newMapping);
    int getModifierBtnState() const { return currentStripModifier; }

    String getTopRowMappingName(const int &mappingID);
    String getNormalRowMappingName(const int &mappingID);
    String getPatternRowMappingName(const int &mappingID);

    // adds a sample to the sample pool
    int addNewSample(File &sampleFile);
    AudioSample * getAudioSample(const int &samplePoolIndex, const int &poolID);
    int getSamplePoolSize(const int &index) const
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
    String getSampleName(const int &index, const int &poolID) const
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
    void stopAllStrips(const int &stopMode);

    void processOSCKeyPress(const int &monomeCol, const int &monomeRow, const bool &state);

    // set up the channels (can be used to change number of channels
    void buildChannelArray(const int &newNumChannels);
    void buildSampleStripArray(const int &numSampleStrips);

    SampleStrip* getSampleStrip(const int &index);

    bool getChannelMuteStatus(const int &chan) const
    {
        jassert(chan < channelMutes.size());
        return channelMutes[chan];
    }
    void setChannelMute(const int &chan, const bool &state)
    {
        jassert(chan < channelMutes.size());
        channelMutes.set(chan, state);
    }
    float getChannelGain(const int &chan) const
    {
        jassert(chan < channelGains.size());
        return channelGains[chan];
    }
    void setChannelGain(const int &chan, const float &newGain)
    {
        jassert(chan < channelGains.size());
        channelGains.set(chan, newGain);
    }
    Colour getChannelColour(const int &chan) const
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
    const void* getSampleStripParameter(const int &parameterID, const int &stripID) const
    {
        return sampleStripArray[stripID]->getSampleStripParam(parameterID);
    }




    // Preset stuff
    void saveXmlSetlist(const File &setlistFile);
    void loadXmlSetlist(const File &setlistFile);
    void addPreset(const String &presetName);
    
    // giving a position within the list, try to 
    // load that preset / setlist item
    void renamePreset(const String &newName, const int & id);
    void removeSetlistItem(const int &id);
    void removePresetListItem(const int &id);
    void selectSetlistItem(const int &id);
    void selectPresetListItem(const int &id);

    void insetPresetIntoSetlist(const int &presetID, const int &indexToInsertAt);
    // this does the "heavy lifting" of actually loading the preset
    void loadPreset(XmlElement * presetToLoad);

    XmlElement getPresetList() const { return presetList; }
    XmlElement & getPresetListP() { return presetList; }

    XmlElement getSetlist() const { return setlist; }
    XmlElement & getSetlistP() { return setlist; }
    void setSetlist(const XmlElement &newSetlist) { setlist = newSetlist; }

    // Recording / resampling stuff
    void startRecording();
    float getRecordingPrecountPercent() const;
    float getRecordingPercent() const;
    void processRecordingBuffer(AudioSampleBuffer &input, const int &numSamples);
    bool areWeRecording() const { return isRecording; }

    void startResampling();
    float getResamplingPrecountPercent() const;
    float getResamplingPercent() const;
    void processResamplingBuffer(AudioSampleBuffer &input, const int &numSamples);
    bool areWeResampling() const { return isResampling; }

    void startPatternRecording() { patternRecordings[currentPatternBank]->startPatternRecording(); }
    float getPatternPrecountPercent() const { return patternRecordings[currentPatternBank]->getPatternPrecountPercent(); }
    float getPatternPercent() const { return patternRecordings[currentPatternBank]->getPatternPercent(); }
    bool areWePatternRecording() const { return patternRecordings[currentPatternBank]->isPatternRecording; }


    // here rate is the level of quantisation, so for 1 / 32th
    // note quantisation, quantisationLevel = 0.03125 etc
    void updateQuantizeSettings()
    {
        if (quantisationLevel < 0.0)
        {
            quantisationOn = false;
            quantisedBuffer.clear();
        }
        else
        {
            quantisationGap = (int) (getSampleRate() * (120.0 * quantisationLevel / currentBPM));
            quantRemaining = quantisationGap;
            quantisationOn = true;
        }
    }

    // this should be called if the BPM changes at all
    void changeBPM()
    {
        updateQuantizeSettings();
        for (int s = 0; s < sampleStripArray.size(); ++s)
            calcPlaySpeedForNewBPM(s);
    }

    // which types of audio files can we load
    const String getWildcardFormats() const { return "*.mp3,*.wav;*.flac;*.ogg;*.aif;*.aiff;*.caf"; }

private:

    // MIDI / quantisation //////////////////////////////////////
    // OSC messages from the monome are converted to MIDI messages.
    // quantisationLevel stores the fineness of the quantisation
    // so that 1/4 note quantisation is 0.25 etc. NOTE: any negative
    // values will be interpreted as no quantisation (-1.0 preferred).
    double quantisationLevel; bool quantisationOn;
    int quantisationGap, quantRemaining, quantiseMenuSelection;
    // quantised notes are stored in one buffer per strip
    MidiBuffer quantisedBuffer;
    // unquantised notes are collected all together
    MidiMessageCollector unquantisedCollector;


    // Sample Pools /////////////////////
    OwnedArray<AudioSample> samplePool;         // for sample files (.wavs etc)
    OwnedArray<AudioSample> resamplePool;       // for recorded internal sounds
    OwnedArray<AudioSample> recordPool;         // for external recordings
    OwnedArray<PatternRecording> patternRecordings;   // for pattern recordings


    // Channel Setup /////////////
    const int maxChannels;          // Max number of channels
    Array<float> channelGains;      // individual channel volumes
    Array<bool> channelMutes;       // are we on or off
    float masterGain;               // gain for combined signal
    const float defaultChannelGain; // initial channel gain level
    Array<Colour> channelColours;   // colours for channels in the GUI


    // Global settings /////////////////////////////////////////////////
    int numChannels;
    bool useExternalTempo;      // either use VST hosts tempo or our own
    double currentBPM;
    // Sometimes when starting / stopping / looping back we
    // get a discontinuity in the stream of samples. To avoid
    // this, we apply a short volume ramp at the start / end of
    // playback and at either end of a loop point:
    int rampLength;
    // How many sample strips
    int numSampleStrips;


    // SampleStrips ////////////////
    // These track the seperate SampleStrips (related to the GUI component
    // SampleStripControl). They control the audio for each strip
    OwnedArray<SampleStrip> sampleStripArray;


    // OSC ////////////////////////
    String OSCPrefix;           // prefix for incoming / outgoing OSC messages
    OSCHandler oscMsgHandler;   // Send and receive OSC messages through this


    // Audio Buffers /////////////////
    // this is for summing the contributions from SampleStrips
    AudioSampleBuffer stripContrib;
    // Store resampled information
    AudioSampleBuffer resampleBuffer;
    bool isResampling;
    int resampleLength, resamplePrecountLength;                     // length in bars
    int resampleLengthInSamples, resamplePrecountLengthInSamples;   // length in samples
    int resamplePosition, resamplePrecountPosition;     // track resampling progress
    int resampleBank, resampleBankSize;                 // which slot to use (how many are free)
    // Store recorded information
    AudioSampleBuffer recordBuffer;
    bool isRecording;
    int recordLength, recordPrecountLength;
    int recordLengthInSamples, recordPrecountLengthInSamples;
    int recordPosition, recordPrecountPosition;
    int recordBank, recordBankSize;
    // Pattern recorder information
    MidiBuffer patternRecorder;
    int currentPatternLength;
    int currentPatternPrecountLength;
    int currentPatternBank;
    int patternBankSize;


    // Preset Handling //////////////////////////////////////////
    String presetName;
    XmlElement presetList;  // this is a unique list of possible presets (used internally)
    XmlElement setlist;     // this is an ordered list of consisting of a entries from presetList

    // Mapping settings ////////////////////////////////////////
    Array<int> topRowMappings;
    OwnedArray< Array<int> > normalRowMappings;
    Array<int> patternMappings;
    // Tells us which of the modifier button is being held
    // so strips can be used to stop, reverse sample etc.
    // NOTE: -1 means no strip is held
    const int numModifierButtons;
    int currentStripModifier;

    void executeNormalRowMapping(const int &mappingID, const int &stripID, const bool &state);
    void executePatternRowMapping(const int &mappingID, const int &stripID, const bool &state);
    void setupDefaultRowMappings();


    // Misc ////////////////

    // what size of device are we using
    int monomeSize;
    int numMonomeRows, numMonomeCols;

    // Store which LED column is currently being used
    // for displaying playback position.
    Array<int> playbackLEDPosition;

    // Do we want to play incoming input?
    bool monitorInputs;

    void setMonomeStatusGrids(const int &width, const int &height);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (mlrVSTAudioProcessor);
};

#endif  // __PLUGINPROCESSOR_H_526ED7A9__
