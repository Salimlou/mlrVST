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

#include "osc/OscOutboundPacketStream.h"
#include "ip/IpEndpointName.h"

#include "OSCHandler.h"

//==============================================================================
mlrVSTAudioProcessor::mlrVSTAudioProcessor() :
    delayBuffer(2, 12000), currentBPM(120.0),
    channelProcessorArray(), numChannels(8),
    sampleStripArray(), numSampleStrips(7),
    channelGains(), defaultChannelGain(0.8f),
    samplePool(),               // sample pool is initially empty
    oscMsgListener(this)
{
    
    DBG("starting OSC thread");
    
    oscMsgListener.startThread();

    // TEST: oscpack sending data
    DBG("starting OSC tests");
    char buffer[1536];
    osc::OutboundPacketStream p(buffer, 1536);
    UdpTransmitSocket socket(IpEndpointName("localhost", 8080));
    p.Clear();
    p << osc::BeginMessage("/mlrvst/led") << 1 << 1 << 0 << osc::EndMessage;
    socket.Send(p.Data(), p.Size());
    DBG("finished OSC tests");
    // END TEST

    //File test("C:\\Users\\Hemmer\\Desktop\\funky.wav");
    //samplePool.add(new AudioSample(test));
    samplePool.clear();

    // Set up some default values..
    masterGain = 1.0f;
    delay = 0.5f;

    // create our SampleStrip objects 
    buildSampleStripArray(numSampleStrips);
    // add our channel processors
    buildChannelProcessorArray(numChannels);


    lastPosInfo.resetToDefault();
    delayPosition = 0;
}

mlrVSTAudioProcessor::~mlrVSTAudioProcessor()
{
    samplePool.clear();
    sampleStripArray.clear();

    DBG("Processor destructor finished.");
}

// returns true if the sample was sucessfully loaded (or pre-existing)
bool mlrVSTAudioProcessor::addNewSample(File &sampleFile)
{
    // avoid duplicates
    bool duplicateFound = false;
    for(int i = 0; i < samplePool.size(); ++i)
    {
        if (samplePool[i]->getSampleFile() == sampleFile) duplicateFound = true;
    }

    if (!duplicateFound)
    {
        try{
            samplePool.add(new AudioSample(sampleFile));
            DBG("Sample Loaded: " + samplePool.getLast()->getSampleName());
            return true;
        }
        catch(String errString)
        {
            DBG(errString);
            return false;
        }

    }
    else
    {
        DBG("Sample already loaded!");
        return true;
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
        channelProcessorArray.add(new ChannelProcessor(c, channelColour, this, sampleStripArray.getFirst(), numSampleStrips));
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
        case delayParam:
            return delay;
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

        case delayParam:            delay = newValue;  break;

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
    delayBuffer.clear();
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
    delayBuffer.clear();
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

        // Apply our delay effect to the new output..
        for (channel = 0; channel < getNumInputChannels(); ++channel)
        {
            float* channelData = buffer.getSampleData(channel);
            float* delayData = delayBuffer.getSampleData(jmin(channel, delayBuffer.getNumChannels() - 1));
            dp = delayPosition;

            for (int i = 0; i < numSamples; ++i)
            {
                const float in = channelData[i];
                channelData[i] += delayData[dp];
                delayData[dp] = (delayData[dp] + in) * delay;

                if (++dp > delayBuffer.getNumSamples())
                    dp = 0;
            }
        }

        delayPosition = dp;

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
    // You should use this method to store your parameters in the memory block.
    // Here's an example of how you can use XML to make it easy and more robust:

    // Create an outer XML element..
    XmlElement xml("MYPLUGINSETTINGS");

    // add some attributes to it..
    xml.setAttribute("master gain", masterGain);
    xml.setAttribute("delay", delay);

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
        if (xmlState->hasTagName("MYPLUGINSETTINGS"))
        {
            // ok, now pull out our parameters...
            masterGain  = (float) xmlState->getDoubleAttribute("master gain", masterGain);
            delay = (float) xmlState->getDoubleAttribute("delay", delay);
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

void mlrVSTAudioProcessor::calcPlaySpeed(const int &stripID, const bool &normalizeTempo)
{
    sampleStripArray[stripID]->findPlaySpeed(currentBPM, 44100.0, normalizeTempo);
}

void mlrVSTAudioProcessor::calcPlaySpeedForNewBPM(const int &stripID)
{
    sampleStripArray[stripID]->updatePlaySpeedForBPMChange(currentBPM);
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
            // only bother with note off if we aren't on latched mode
            else if (!isLatched) monomeState.noteOff(effectiveMonomeRow + 1, monomeCol);
        }
    }

}