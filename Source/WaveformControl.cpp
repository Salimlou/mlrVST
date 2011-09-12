/*
==============================================================================

WaveformControl.cpp
Created: 6 Sep 2011 12:38:13pm
Author:  Hemmer

Custom component to display a waveform (corresponding to an mlr row)

==============================================================================
*/

#include "WaveformControl.h"
#include "PluginEditor.h"

WaveformControl::WaveformControl(const int &id) :
    	thumbnailCache(5),
        thumbnail(512, formatManager, thumbnailCache),
        backgroundColour(Colours::black),
        trackNumberLbl("track number", String(waveformID)),
        filenameLbl("filename", "No File"),
        waveformID(id),
        currentSample(),
        currentChannel(0), numChannels(0),
        channelButtonArray(),
        selectionStart(0), selectionEnd(0),
        isReversed(false),
        numChunks(8), chunkSize(0),
        thumbnailLength(0.0)
{
    // TODO: doesn't get bounds until initialised
    Rectangle<int> waveformShape = getBoundsInParent();

    addAndMakeVisible(&filenameLbl);
    filenameLbl.setColour(Label::backgroundColourId, Colours::white);
    filenameLbl.setColour(Label::textColourId, Colours::black);
    filenameLbl.setJustificationType(Justification::right);
    // TODO: make sure this is same as parent
    filenameLbl.setBounds(0, 0, 300, 15);
    filenameLbl.setFont(10.0f);

    formatManager.registerBasicFormats();
    thumbnail.addChangeListener (this);

    addAndMakeVisible(&trackNumberLbl);
    trackNumberLbl.setBounds(0, 0, 15, 15);
    trackNumberLbl.setColour(Label::backgroundColourId, Colours::black);
    trackNumberLbl.setColour(Label::textColourId, Colours::white);
    trackNumberLbl.setFont(10.0f);


}

WaveformControl::~WaveformControl()
{
    thumbnail.removeChangeListener (this);
}


void WaveformControl::buttonClicked(Button *btn){

    // see if any of the channel selection buttons were chosen
    for(int i = 0; i < channelButtonArray.size(); ++i){
        if(channelButtonArray[i] == btn){
            backgroundColour = channelButtonArray[i]->getBackgroundColour();
            currentChannel = i;
            repaint();
        }
    }
}

void WaveformControl::clearChannelList()
{
    // if there are existing buttons, remove them
    if(channelButtonArray.size() != 0) channelButtonArray.clear();
    numChannels = 0;
}


/* Used to (re)add Buttons to control the switching channels.
   Useful if we want to change the total number of channels at runtime. */
void WaveformControl::addChannel(const int &id, const Colour &col)
{

    channelButtonArray.add(new DrawableButton("button" + String(numChannels), DrawableButton::ImageRaw));
    addAndMakeVisible(channelButtonArray.getLast());
    channelButtonArray.getLast()->setBounds(15 + numChannels*15, 0, 15, 15);
    channelButtonArray.getLast()->addListener(this);
    channelButtonArray.getLast()->setBackgroundColours(col, col);


    // once we've added all the buttons, add the filename
    // TODO NOW: filenameLbl.setBounds((i+1)*15, 0, 500, 15);

    // reset channel to first channel just in case 
    // old channel setting no longer exists
    currentChannel = 0;
    backgroundColour = channelButtonArray.getFirst()->getBackgroundColour();
    // let the UI know
    repaint();

    ++numChannels;

}


void WaveformControl::setZoomFactor (double amount)
{
    //if (thumbnail.getTotalLength() > 0)
    //{
    //    double timeDisplayed = jmax (0.001, (thumbnail.getTotalLength() - startTime) * (1.0 - jlimit (0.0, 1.0, amount)));
    //    endTime = startTime + timeDisplayed;
    //    repaint();
    //}
}

void WaveformControl::mouseWheelMove (const MouseEvent&, float wheelIncrementX, float wheelIncrementY)
{
    //if (thumbnail.getTotalLength() > 0)
    //{
    //    double newStart = startTime + (wheelIncrementX + wheelIncrementY) * (endTime - startTime) / 10.0;
    //    newStart = jlimit (0.0, thumbnail.getTotalLength() - (endTime - startTime), newStart);
    //    endTime = newStart + (endTime - startTime);
    //    startTime = newStart;
    //    repaint();
    //}
}

void WaveformControl::mouseDown(const MouseEvent &e){

    if(e.mods == ModifierKeys::rightButtonModifier){

        mlrVSTAudioProcessorEditor *demoPage = findParentComponentOfClass((mlrVSTAudioProcessorEditor*) 0);
        
        if(demoPage != 0){
            int currentSamplePoolSize = demoPage->getSamplePoolSize();

            // only show the menu if we have samples in the pool
            if(currentSamplePoolSize != 0)
            {
                // TODO: can this be cached and only repopulated when the sample pool changes?
                PopupMenu p = PopupMenu();
                
                // TODO: add option to select "no file?
                // TODO: middle click to delete sample under cursor in menu?

                // for each sample, add it to the list
                for(int i = 0; i < currentSamplePoolSize; ++i)
                {
                    // +1 is because 0 is result for no item clicked
                    String iFileName = demoPage->getSampleName(i);
                    p.addItem(i+1, iFileName);
                }

                // show the menu and store choice 
                int fileChoice = p.showMenu(PopupMenu::Options().withTargetComponent(&filenameLbl));

                // if a menu option has been chosen
                if (fileChoice != 0)
                {
                    // -1 is to correct for +1 in for loop above
                    --fileChoice;       
                    // update choice of sample
                    setAudioSample(demoPage->getSample(fileChoice));
                }
            }
        }
    }
}

void WaveformControl::paint(Graphics& g)
{
	g.fillAll (backgroundColour);
    g.setColour (Colours::white);

    if (thumbnail.getTotalLength() > 0)
    {
        //g.setColour (Colours::black);
        //thumbnail.drawChannel(g, getLocalBounds().reduced(2, 2),
        //                      0, thumbnailLength, 0, 1.0f);

        //g.setColour (Colours::white);
        //
        //thumbnail.drawChannel(g, getLocalBounds().reduced(2, 2),
        //                      0, thumbnailLength, 1, 1.0f);

        thumbnail.drawChannels (g, getLocalBounds().reduced (2, 2),
                                0, thumbnailLength, 1.0f);
    }
    else
    {
        g.setFont (12.0f);
        g.drawFittedText ("(No audio file selected)", 0, 0, getWidth(), getHeight(),
                            Justification::centred, 2);
    }
}

void WaveformControl::changeListenerCallback (ChangeBroadcaster*)
{
    // this method is called by the thumbnail when it has changed, so we should repaint it..
    repaint();
}

bool WaveformControl::isInterestedInFileDrag (const StringArray& /*files*/)
{
    return true;
}

void WaveformControl::filesDropped (const StringArray& files, int /*x*/, int /*y*/)
{
    // get pointer to parent class
    mlrVSTAudioProcessorEditor* pluginUI = findParentComponentOfClass ((mlrVSTAudioProcessorEditor*) 0);

    // try to add each of the loaded files to the sample pool
    for(int i = 0; i < files.size(); ++i)
    {
        File currentSample(files[i]);
        DBG("Dragged file: " << files[i]);
        pluginUI->loadSampleFromFile(currentSample);
    }

    // set latest sample in the pool as the current sample
    // DESIGN: is this correct behaviour?
    setAudioSample(pluginUI->getLatestSample());
}

void WaveformControl::setAudioSample(AudioSample* sample)
{
    // this is now the new audio sample
    currentSample = sample;

    // by default, the whole sample is selected
    // TODO: check math of rounding here
    selectionStart = 0;
    selectionEnd = currentSample->getSampleLength();
    chunkSize = (int) ((selectionEnd - selectionStart) / numChunks);

    // get temporary reference to the File object of the
    // sample to update labels / thumbnails
    File sampleFile = currentSample->getSampleFile();
    thumbnail.setSource(new FileInputSource (sampleFile));
    thumbnailLength = thumbnail.getTotalLength();

    // update filename label
    filenameLbl.setText(sampleFile.getFileName(), false);

    DBG("Waveform strip " + String(waveformID) + ": file selected " + sampleFile.getFileName());

}