/*
  ==============================================================================

    WaveformControl.h
    Created: 6 Sep 2011 12:38:13pm
    Author:  Hemmer

    Custom component to display a waveform (corresponding to an mlr row)

  ==============================================================================
*/

#ifndef __WAVEFORMCONTROL_H_E96F19F8__
#define __WAVEFORMCONTROL_H_E96F19F8__

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JuceLibraryCode/JucePluginCharacteristics.h"
#include "AudioSample.h"

class WaveformControl  : public Component,
                         public ChangeListener,
                         public ButtonListener,
                         public FileDragAndDropTarget
{
public:
    WaveformControl(const int &id);
	~WaveformControl();

    //void setFile (const File& file);
	void setZoomFactor (double amount);
	void mouseWheelMove (const MouseEvent&, float wheelIncrementX, float wheelIncrementY);
    void mouseDown(const MouseEvent&);

    void paint (Graphics& g);
    void changeListenerCallback (ChangeBroadcaster*);
    void buttonClicked(Button *btn);

    // This allows us to load samples by Drag n' Drop
    bool isInterestedInFileDrag (const StringArray& /*files*/);
    void filesDropped (const StringArray& files, int /*x*/, int /*y*/);

    // if the number of channels changes, we can update the strips
    void addChannel(const int &id, const Colour &col);
    void clearChannelList();

    AudioSample* getCurrentSample() const { return currentSample; }
        
    // how many chunks to break the sample into (default 8 for standard monome).
    int getNumChunks() const { return numChunks; }
    int getSelectionStart() const { return selectionStart; }
    int getChunkSize() const { return chunkSize; }

private:

    void setAudioSample(AudioSample* sample);

    // which strip we are representing
    int waveformID;

    // GUI components
    Label trackNumberLbl, filenameLbl;
    OwnedArray<DrawableButton> channelButtonArray;
        
    int currentChannel;     // which channel audio is currently going to
    int numChannels;        // total number of channels available
    
    /* ============================================================
       properties about the waveformcontrols handling of the sample
       note these are waveform strip independent, i.e. they are not
       properties of the AudioSample. This means we can have
       different start points on different rows for the same sample
       ============================================================*/
    bool isReversed;
    // these allow a subsection of the sample to be selected
    // NOTE: by default the whole sample is selected
    int selectionStart, selectionEnd; 
    // how many chunks to break the sample into (default 8 for 
    // standard monome) and what size (in samples) they are
    int numChunks, chunkSize;


    // stuff for drawing waveforms
    // TODO: should this be associated with the AudioSample?
    AudioFormatManager formatManager;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    double thumbnailLength;

    // main strip background colour
    Colour backgroundColour;

    // Pointer to the sample object
    AudioSample* currentSample;
};



#endif  // __WAVEFORMCONTROL_H_E96F19F8__
