/*
==============================================================================

This file was auto-generated by the Jucer!

It contains the basic startup code for a Juce application.

==============================================================================
*/

// use this to help track down memory leaks (SLOW)
// #include <vld.h>

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "OSCHandler.h"

//==============================================================================
mlrVSTAudioProcessor::mlrVSTAudioProcessor() :
    currentBPM(120.0), channelProcessorArray(),
    maxChannels(8), numChannels(maxChannels),
    sampleStripArray(), numSampleStrips(7),
    channelGains(), defaultChannelGain(0.8f),
    samplePool(),               // sample pool is initially empty
    oscMsgHandler(this),
    setList("SETLIST")
{
    
    DBG("starting OSC thread");
    
    // start listening for messages
    oscMsgHandler.startThread();

    //File test("C:\\Users\\Hemmer\\Desktop\\funky.wav");
    //samplePool.add(new AudioSample(test));
    samplePool.clear();

    // Set up some default values..
    masterGain = 1.0f;

    // create our SampleStrip objects 
    buildSampleStripArray(numSampleStrips);
    // add our channel processors
    buildChannelProcessorArray(numChannels);

    //savePreset("testaods");

    lastPosInfo.resetToDefault();

    startTimer(200);
}

mlrVSTAudioProcessor::~mlrVSTAudioProcessor()
{
    // be polite!
    oscMsgHandler.clearGrid();

    samplePool.clear(true);
    sampleStripArray.clear(true);

    DBG("Processor destructor finished.");
}

// returns the index of a file if sucessfully loaded (or pre-existing)
// returns -1 if the loading failed
int mlrVSTAudioProcessor::addNewSample(File &sampleFile)
{

    // if the sample already exists, return its (existing) index
    for(int i = 0; i < samplePool.size(); ++i)
    {
        if (samplePool[i]->getSampleFile() == sampleFile)
        {
            DBG("Sample already loaded!");
            return i;
        }
    }

    // load to load the Sample
    try{
        samplePool.add(new AudioSample(sampleFile));
        DBG("Sample Loaded: " + samplePool.getLast()->getSampleName());

        // if it is sucessful, it will be the last sample in the pool
        int newSampleIndex = samplePool.size() - 1;
        return newSampleIndex;
    }
    catch(String errString)
    {
        DBG(errString);
        // return a fail code
        return -1;
    }



}


void mlrVSTAudioProcessor::buildChannelProcessorArray(const int &newNumChannels)
{
    // update the number of channels
    numChannels = newNumChannels;

    // make sure all channels stop playing
    for (int c = 0; c < channelProcessorArray.size(); c++)
        channelProcessorArray[c]->stopSamplePlaying();
    // clear MIDI queue
    monomeState.reset();
    // make sure we're not doing any audio processing while (re)building it
    suspendProcessing(true);

    
    // reset the list of channels
    channelProcessorArray.clear();
    for (int c = 0; c < numChannels; c++)
    {   
        Colour channelColour((float) (0.1f * c), 0.5f, 0.5f, 1.0f);
        channelProcessorArray.add(new ChannelProcessor(c, channelColour, this, numSampleStrips));
    }

    // reset the gains to the default
    channelGains.clear();
    for (int c = 0; c < maxChannels; c++)
        channelGains.add(defaultChannelGain);

    // and make sure each strip is reset to the first channel
    for(int strip = 0; strip < sampleStripArray.size(); ++strip)
    {
        int initialChannel = 0;
        sampleStripArray[strip]->setSampleStripParam(SampleStrip::ParamCurrentChannel, &initialChannel);
    }

    DBG("Channel processor array built");

    // resume processing
    suspendProcessing(false);
}


//==============================================================================
int mlrVSTAudioProcessor::getNumParameters()
{
    return totalNumParams;
}

float mlrVSTAudioProcessor::getParameter(int index)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    switch (index)
    {
        case masterGainParam:
            return masterGain;
        case channel0GainParam:
            return channelGains[0];
        case channel1GainParam:
            return channelGains[1];
        case channel2GainParam:
            return channelGains[2];
        case channel3GainParam:
            return channelGains[3];
        case channel4GainParam:
            return channelGains[4];
        case channel5GainParam:
            return channelGains[5];
        case channel6GainParam:
            return channelGains[6];
        case channel7GainParam:
            return channelGains[7];

        default:            return 0.0f;
    }
}

void mlrVSTAudioProcessor::setParameter(int index, float newValue)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    switch (index)
    {
        case masterGainParam:       masterGain = newValue;  break;

            // TODO: there might be a neater way to do this!
        case channel0GainParam:
            channelGains.set(0, newValue);    break;

        case channel1GainParam:
            channelGains.set(1, newValue);    break;

        case channel2GainParam:
            channelGains.set(2, newValue);    break;

        case channel3GainParam:
            channelGains.set(3, newValue);    break;

        case channel4GainParam:
            channelGains.set(4, newValue);    break;

        case channel5GainParam:
            channelGains.set(5, newValue);    break;

        case channel6GainParam:
            channelGains.set(6, newValue);    break;

        case channel7GainParam:
            channelGains.set(7, newValue);    break;

        default:                    break;
    }
}

const String mlrVSTAudioProcessor::getParameterName(int index)
{
    switch (index)
    {
        case masterGainParam:       return "master gain";

        case delayParam:            return "delay";

        case channel0GainParam:     return "channel 0 gain";

        case channel1GainParam:     return "channel 1 gain";

        case channel2GainParam:     return "channel 2 gain";

        case channel3GainParam:     return "channel 3 gain";

        case channel4GainParam:     return "channel 4 gain";

        case channel5GainParam:     return "channel 5 gain";

        case channel6GainParam:     return "channel 6 gain";

        case channel7GainParam:     return "channel 7 gain";

        default:            break;
    }

    return String::empty;
}

const String mlrVSTAudioProcessor::getParameterText(int index)
{
    return String(getParameter(index), 2);
}

//==============================================================================
void mlrVSTAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    // TODO: does ChannelProcessor need this?
    //synth.setCurrentPlaybackSampleRate (sampleRate);
    monomeState.reset();
}

void mlrVSTAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    monomeState.reset();
}

void mlrVSTAudioProcessor::reset()
{
    // Use this method as the place to clear any delay lines, buffers, etc, as it
    // means there's been a break in the audio's continuity.
}

void mlrVSTAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    if (!isSuspended())
    {
        // ask the host for the current time so we can display it...
        AudioPlayHead::CurrentPositionInfo newTime;

        if (getPlayHead() != 0 && getPlayHead()->getCurrentPosition(newTime))
        {
            // Successfully got the current time from the host..
            lastPosInfo = newTime;
            if (currentBPM != lastPosInfo.bpm && lastPosInfo.bpm > 1.0)
            {
                // If the tempo has changed, adjust the playspeeds accordingly
                currentBPM = lastPosInfo.bpm;
                for (int s = 0; s < sampleStripArray.size(); ++s)
                    calcPlaySpeedForNewBPM(s);
            }
        }
        else
        {
            // If the host fails to fill-in the current time, we'll just clear it to a default..
            lastPosInfo.resetToDefault();
        }


        const int numSamples = buffer.getNumSamples();
        int channel, dp = 0;

        /* this adds the OSC messages from the monome which have been converted
        to midi messages (where MIDI channel is row, note number is column) */
        monomeState.processNextMidiBuffer(midiMessages, 0, numSamples, true);


        // for each channel, add its contributions
        // Remember to set the correct sample
        for (int c = 0; c < channelProcessorArray.size(); c++)
        {
            channelProcessorArray[c]->setBPM(currentBPM);
            channelProcessorArray[c]->getCurrentPlaybackPercentage();
            channelProcessorArray[c]->renderNextBlock(buffer, midiMessages, 0, numSamples);
        }

        // Go through the outgoing data, and apply our master gain to it...
        for (channel = 0; channel < getNumInputChannels(); ++channel)
            buffer.applyGain(channel, 0, buffer.getNumSamples(), masterGain);


        // In case we have more outputs than inputs, we'll clear any output
        // channels that didn't contain input data, (because these aren't
        // guaranteed to be empty - they may contain garbage).
        for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
            buffer.clear(i, 0, buffer.getNumSamples());

    }
    else
    {
        //silence
    }
}

//==============================================================================
AudioProcessorEditor* mlrVSTAudioProcessor::createEditor()
{
    return new mlrVSTAudioProcessorEditor(this, numChannels);
}

//==============================================================================
void mlrVSTAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    // This method stores parameters in the memory block

    // Create an outer XML element..
    XmlElement xml("GLOBALSETTINGS");

    // add some attributes to it..

    xml.setAttribute("master gain", masterGain);
    for (int c = 0; c < channelGains.size(); c++)
    {
        String name("channel gain ");
        name += c;
        xml.setAttribute(name, channelGains[c]);
    }
     
    xml.setAttribute("current preset", "none");

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary(xml, destData);
}

void mlrVSTAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // This getXmlFromBinary() helper function retrieves our XML from the binary blob..
    ScopedPointer<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != 0)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName("GLOBALSETTINGS"))
        {
            // ok, now pull out our parameters...
            masterGain  = (float) xmlState->getDoubleAttribute("master gain", masterGain);
            for (int c = 0; c < channelGains.size(); c++)
            {
                String name("channel gain ");
                name += c;
                channelGains.set(c, (float) xmlState->getDoubleAttribute(name, channelGains[c]));
            }
        }
    }
}

const String mlrVSTAudioProcessor::getInputChannelName(const int channelIndex) const
{
    return String(channelIndex + 1);
}

const String mlrVSTAudioProcessor::getOutputChannelName(const int channelIndex) const
{
    return String(channelIndex + 1);
}

bool mlrVSTAudioProcessor::isInputChannelStereoPair(int /*index*/) const
{
    return true;
}

bool mlrVSTAudioProcessor::isOutputChannelStereoPair(int /*index*/) const
{
    return true;
}

bool mlrVSTAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool mlrVSTAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new mlrVSTAudioProcessor();
}


//////////////////////
// OSC Stuff        //
//////////////////////

void mlrVSTAudioProcessor::timerCallback()
{

    // TODO: this is a HORRIBLE way to do this, and needs fixing
    oscMsgHandler.clearGrid();

    for (int row = 0; row < sampleStripArray.size(); ++row)
    {
        float percentage = *static_cast<const float*>
            (getSampleStripParameter(SampleStrip::ParamPlaybackPercentage, row));
        int numChunks = *static_cast<const int*>
            (getSampleStripParameter(SampleStrip::ParamNumChunks, row));

        bool isPlaying = *static_cast<const bool*>
            (getSampleStripParameter(SampleStrip::ParamIsPlaying, row));

        if (isPlaying)
            oscMsgHandler.setLED((int)(percentage * numChunks), row + 1, 1);
    }

    
}

void mlrVSTAudioProcessor::processOSCKeyPress(const int &monomeCol, const int &monomeRow, const int &state)
{

    if (monomeRow == 0)
    {
        // TODO control mappings for top row 
    } 
    else if (monomeRow <= numSampleStrips && monomeRow >= 0)
    {

        /* Only pass on messages that are from the allowed range of columns.
           NOTE: MIDI messages may still be passed from other sources that
           are outside this range so the channelProcessor must be aware of 
           numChunks too to filter these. The -1 is because we are treating
           the second row as the first "effective" row.
        */
        int effectiveMonomeRow = monomeRow - 1;

        int numChunks = *static_cast<const int*>
                        (sampleStripArray[effectiveMonomeRow]->getSampleStripParam(SampleStrip::ParamNumChunks));

        if (monomeCol < numChunks)
        {
            bool isLatched = *static_cast<const bool*>
                (sampleStripArray[effectiveMonomeRow]->getSampleStripParam(SampleStrip::ParamIsLatched));

            // The +1 here is because midi channels start at 1 not 0!
            if (state) monomeState.noteOn(effectiveMonomeRow + 1, monomeCol, 1.0f);
            else monomeState.noteOff(effectiveMonomeRow + 1, monomeCol);
        }
    }

}


///////////////////////
// SampleStrip stuff //
///////////////////////
void mlrVSTAudioProcessor::buildSampleStripArray(const int &newNumSampleStrips)
{
    // make sure we're not using the sampleStripArray while (re)building it
    suspendProcessing(true);


    sampleStripArray.clear();
    numSampleStrips = newNumSampleStrips;
    
    for (int i = 0; i < numSampleStrips; ++i)
    {
        sampleStripArray.add(new SampleStrip());    
    }

    DBG("SampleStrip array built");

    // resume processing
    suspendProcessing(false);
}


void mlrVSTAudioProcessor::calcInitialPlaySpeed(const int &stripID)
{
    // TODO insert proper host speed here
    sampleStripArray[stripID]->findInitialPlaySpeed(currentBPM, 44100.0);
}
void mlrVSTAudioProcessor::calcPlaySpeedForNewBPM(const int &stripID)
{
    sampleStripArray[stripID]->updatePlaySpeedForBPMChange(currentBPM);
}
void mlrVSTAudioProcessor::calcPlaySpeedForSelectionChange(const int &stripID)
{
    sampleStripArray[stripID]->updatePlaySpeedForSelectionChange();
}
void mlrVSTAudioProcessor::modPlaySpeed(const double &factor, const int &stripID)
{
    sampleStripArray[stripID]->modPlaySpeed(factor);
}

AudioSample * mlrVSTAudioProcessor::getAudioSample(const int &samplePoolIndex)
{
    if (samplePoolIndex >= 0 && samplePoolIndex < samplePool.size())
        return samplePool[samplePoolIndex];
    else return 0;
}
SampleStrip* mlrVSTAudioProcessor::getSampleStrip(const int &index) 
{
    jassert( index < sampleStripArray.size() );
    SampleStrip *tempStrip = sampleStripArray[index];
    return tempStrip;
}



// Not yet possible to switch channels while playing
void mlrVSTAudioProcessor::switchChannels(const int &newChan, const int &stripID)
{
    int currentChannel = *static_cast<const int*>
        (sampleStripArray[stripID]->getSampleStripParam(SampleStrip::ParamCurrentChannel));

    bool isPlaying = *static_cast<const bool*>
        (sampleStripArray[stripID]->getSampleStripParam(SampleStrip::ParamIsPlaying));

    // Stop the current channel if playing
    if (isPlaying)
        channelProcessorArray[currentChannel]->stopSamplePlaying();

    // Let the strip now about the new channel
    sampleStripArray[stripID]->setSampleStripParam(SampleStrip::ParamCurrentChannel, &newChan);

    //double currentPosition = channelProcessorArray[currentChannel]->stopSamplePlaying();
    //if (isPlaying)
    //{
    //    // restart the new channel
    //    channelProcessorArray[newChan]->startSamplePlaying(currentPosition, sampleStripArray[stripID]);
    //}
    
}


/////////////////////
// Preset Handling //
/////////////////////

void mlrVSTAudioProcessor::savePreset(const String &presetName)
{
    // Create an outer XML element..
    XmlElement newPreset("PRESET");

    newPreset.setAttribute("name", presetName);

    // Store settings of each strip
    for (int strip = 0; strip < sampleStripArray.size(); ++strip)
    {
        XmlElement *stripXml = new XmlElement("STRIP");
        stripXml->setAttribute("id", strip);

        // write all parameters to XML
        for (int param = 0; param < SampleStrip::NumGUIParams; ++param)
        {
            int paramType = sampleStripArray[strip]->getParameterType(param);
            String paramName = sampleStripArray[strip]->getParameterName(param);

            const void *p = sampleStripArray[strip]->getSampleStripParam(param);

            switch (paramType)
            {
            case SampleStrip::TypeBool :
                stripXml->setAttribute(paramName, (int)(*static_cast<const bool*>(p)));
                break;

            case SampleStrip::TypeInt :
                stripXml->setAttribute(paramName, *static_cast<const int*>(p));
                break;

            case SampleStrip::TypeDouble :
                stripXml->setAttribute(paramName, *static_cast<const double*>(p));
                break;

            case SampleStrip::TypeFloat :
                stripXml->setAttribute(paramName, (double)(*static_cast<const float*>(p)));
                break;

            case SampleStrip::TypeAudioSample :
                {
                    const AudioSample * sample = static_cast<const AudioSample*>(p);
                    if (sample)
                    {
                        String samplePath = sample->getSampleFile().getFullPathName();
                        stripXml->setAttribute(paramName, samplePath);
                    }
                    break;
                }
            default : jassertfalse;
            }
        }

        newPreset.addChildElement(stripXml);
    }
     
    //File testFile("C:\\Users\\Hemmer\Desktop\\test.xml");
    

    
    XmlElement* p = setList.getFirstChildElement();

    // See if a preset of this name exists in the set list
    bool duplicateFound = false;
    while (p != nullptr)
    {
        String pName = p->getStringAttribute("name");

        // If it does, replace it
        if (pName == presetName)
        {
            setList.replaceChildElement(p, new XmlElement(newPreset));
            duplicateFound = true;
            break;
        }
        else
            p = p->getNextElement();
    }

    // Otherwise, add the new preset
    if (!duplicateFound)
        setList.addChildElement(new XmlElement(newPreset));

    DBG(setList.createDocument(String::empty));
}