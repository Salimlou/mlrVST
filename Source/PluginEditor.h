/*
  ==============================================================================

    This file was auto-generated by the Jucer!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#ifndef __PLUGINEDITOR_H_4ACCBAA__
#define __PLUGINEDITOR_H_4ACCBAA__



#include "../JuceLibraryCode/JuceHeader.h"
#include "../JuceLibraryCode/JucePluginCharacteristics.h"
#include "PluginProcessor.h"
#include "SampleStripControl.h"
#include "PresetPanel.h"
#include "SettingsPanel.h"
#include "timedButton.h"
#include "mlrVSTLookAndFeel.h"

#ifndef PAD_AMOUNT
#define PAD_AMOUNT 10
#endif

#ifndef GUI_WIDTH
#define GUI_WIDTH 1024
#endif

#ifndef GUI_HEIGHT
#define GUI_HEIGHT 650
#endif


class WaveformControl;

//==============================================================================
/** This is the editor component that our filter will display.
*/
class mlrVSTAudioProcessorEditor  : public AudioProcessorEditor,
                                    public SliderListener,
									public ButtonListener,
                                    public ComboBoxListener,
                                    public Timer, 
                                    public FileDragAndDropTarget
{
public:
    mlrVSTAudioProcessorEditor (mlrVSTAudioProcessor* ownerFilter,
                                const int &newNumChannels,
                                const int &newNumStrips);
    ~mlrVSTAudioProcessorEditor();

    //==============================================================================
    void timerCallback();
    void paint(Graphics& g);

    // These are required so that the WaveformControls
    // can handle sample loading by Drag n' Drop 
    bool isInterestedInFileDrag(const StringArray&) { return true; }
    void filesDropped(const StringArray&, int, int) { }


    // listeners
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
    void sliderValueChanged (Slider*);
	void buttonClicked(Button*);

    /* These just forward the various requests for sample information
       to the AudioProcessor (which holds the sample pool). */
    int getSamplePoolSize(const int &poolID) { return getProcessor()->getSamplePoolSize(poolID); }
    String getSampleName(const int &index, const int &poolID) { return getProcessor()->getSampleName(index, poolID); }
    // AudioSample* getSample(const int &index) { return getProcessor()->getSample(index); }
    // AudioSample* getLatestSample() { return getProcessor()->getLatestSample(); }

    /* This returns the index of a file if sucessfully loaded or exists
       already and returns -1 if the loading failed.
    */
    int loadSampleFromFile(File &sampleFile) { return getProcessor()->addNewSample(sampleFile); }
    
    // Pass SampleStripControl messages back to the plugin processor
    File getSampleSourceFile(const int &index) const { return getProcessor()->getSample(index)->getSampleFile(); }

    void calcInitialPlaySpeed(const int &stripID) const { getProcessor()->calcInitialPlaySpeed(stripID); }
    void updatePlaySpeedForNewSelection(const int &stripID) { getProcessor()->calcPlaySpeedForSelectionChange(stripID); }
    void modPlaySpeed(const double &factor, const int &stripID) { getProcessor()->modPlaySpeed(factor, stripID); }
    AudioSample * getAudioSample(const int &poolIndex, const int &poolID) const { return getProcessor()->getAudioSample(poolIndex, poolID); }

    void switchChannels(const int &newChan, const int &stripID) const { getProcessor()->switchChannels(newChan, stripID); }
    Colour getChannelColour(const int &chan) const { return getProcessor()->getChannelColour(chan); }

    XmlElement getPresetList() const { return getProcessor()->getPresetList(); }
    XmlElement getSetlist() const { return getProcessor()->getSetlist(); }
    void setSetlist(const XmlElement &newSetlist) { getProcessor()->setSetlist(newSetlist); }
    void setCurrentPreset(const int &id) { getProcessor()->switchPreset(id); }
    void updateGlobalSetting(const int &parameterID, const void *newValue);
    const void* getGlobalSetting(const int &parameterID) const;

private:

    Typeface::Ptr typeSilk;
    Font fontSilk;
    float fontSize;

    mlrVSTLookAndFeel myLookAndFeel;
    menuLookandFeel menuLF;

    // these are just helpers for positioning
    int xPosition, yPosition;

    // volume controls (and labels)
    Slider masterGainSlider; Label masterSliderLabel;
    OwnedArray<Slider> slidersArray;
    OwnedArray<Label> slidersLabelArray;
    // this just sets it all up
    void buildSliders();


    // bpm slider components
    Label bpmLabel; Slider bpmSlider;
    // quantise options components
    Label quantiseLabel; ComboBox quantiseSettingsCbox;
    void setUpTempoUI();



	DrawableButton debugButton;
    TextButton loadFilesBtn, addPresetBtn;
    TimedButton resampleBtn, recordBtn;
    ToggleButton toggleSetlistBtn, toggleSettingsBtn;
	ListBox fileList;

    Label precountLbl, recordLengthLbl, bankLbl;


    void setUpRecordResampleUI();
    Slider recordLengthSldr, recordPrecountSldr, recordBankSldr;
    Slider resampleLengthSldr, resamplePrecountSldr, resampleBankSldr;



    Rectangle<int> presetPanelBounds;
    PresetPanel presetPanel;

    Rectangle<int> settingsPanelBounds;
    SettingsPanel settingsPanel;

	AudioFormatManager formatManager;

	// Store the waveform controls/strips in array. 
    // For a standard monome64 this is 7
    OwnedArray<SampleStripControl> sampleStripControlArray;
    const int numStrips;
    void buildSampleStripControls();
    const int waveformControlHeight, waveformControlWidth;

    //////////////
    // Settings //
    //////////////

    // This is the number of seperate channels. In pratical terms, this
    // is just the number of samples that can be played at once.
	int numChannels;
    bool useExternalTempo;

    // this object is used to store bpm information from the host
    AudioPlayHead::CurrentPositionInfo lastDisplayedPosition;

    mlrVSTAudioProcessor* getProcessor() const { return static_cast <mlrVSTAudioProcessor*> (getAudioProcessor()); }

    JUCE_LEAK_DETECTOR(mlrVSTAudioProcessorEditor);  
};


#endif  // __PLUGINEDITOR_H_4ACCBAA__
