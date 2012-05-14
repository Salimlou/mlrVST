/*
  ==============================================================================

    mlrVST

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"



//==============================================================================
mlrVSTAudioProcessorEditor::mlrVSTAudioProcessorEditor (mlrVSTAudioProcessor* owner,
                                                        const int &newNumChannels,
                                                        const int &newNumStrips)
    : AudioProcessorEditor (owner),
    // Communication ////////////////////////
    parent(owner),
    // Style / positioning objects //////////
    myLookAndFeel(), menuLF(),
    xPosition(0), yPosition(0),

    // Fonts //////////////////
    fontSize(7.4f),
    mis(BinaryData::silkfont, BinaryData::silkfontSize, false),
    typeSilk(new CustomTypeface(mis)),
    fontSilk( typeSilk ),

    // Volume controls //////////////////////////////
    masterGainSlider("master gain"), masterSliderLabel("master", "MSTR"),
    slidersArray(), slidersMuteBtnArray(),

    // Tempo controls ///////////////////////////////////////
    bpmSlider("bpm slider"), bpmLabel(),
    quantiseSettingsCbox("quantise settings"), quantiseLabel(),

    // Buttons ////////////////////////////
    loadFilesBtn("load files", "LOAD FILES"),

    // Record / resample UI ////////////////////////////////////////
    precountLbl("precount", "precount"), recordLengthLbl("length", "length"), bankLbl("bank", "bank"),
    recordPrecountSldr(), recordLengthSldr(), recordBankSldr(),
    resamplePrecountSldr(), resampleLengthSldr(), resampleBankSldr(),
    recordBtn("RECORD", Colours::black, Colours::white),
    resampleBtn("RESAMPLE", Colours::black, Colours::white),

    // Misc ///////////////////////////////////////////
    lastDisplayedPosition(),
    debugButton("loadfile", DrawableButton::ImageRaw),    // temporary button

    // Presets //////////////////////////////////////////////////
    addPresetBtn("save preset", "Save Preset"), toggleSetlistBtn("Setlist"),
    presetPanelBounds(294, PAD_AMOUNT, THUMBNAIL_WIDTH, 725),
    presetPanel(presetPanelBounds, owner),

    // Settings ///////////////////////////////////////
    numChannels(newNumChannels), useExternalTempo(true), toggleSettingsBtn("Settings"),
    settingsPanelBounds(294, PAD_AMOUNT, THUMBNAIL_WIDTH, 725),
    settingsPanel(settingsPanelBounds, owner, this),

    // Mappings //////////////////////////////////////
    mappingPanelBounds(294, PAD_AMOUNT, THUMBNAIL_WIDTH, 725),
    toggleMappingBtn("Mappings"),
    mappingPanel(mappingPanelBounds, owner),

    // SampleStrip controls ///////////////////////////
    sampleStripControlArray(), numStrips(newNumStrips),
    waveformControlHeight( (GUI_HEIGHT - numStrips * PAD_AMOUNT) / numStrips),
    waveformControlWidth(THUMBNAIL_WIDTH)
{
    DBG("GUI loaded " << " strips of height " << waveformControlHeight);

    // these are compile time constants
    setSize(GUI_WIDTH, GUI_HEIGHT);

    setWantsKeyboardFocus(true);

    // set up volume sliders
    buildSliders();

    // add the bpm slider and quantise settings
    setupTempoUI();

    // useful UI debugging components
    addAndMakeVisible(&debugButton);
	debugButton.addListener(this);
	debugButton.setBackgroundColours(Colours::blue, Colours::black);
	debugButton.setBounds(50, 300, 50, 25);

    addAndMakeVisible(&loadFilesBtn);
	loadFilesBtn.addListener(this);
	loadFilesBtn.setBounds(50, 350, 70, 25);


    setUpRecordResampleUI();
    buildSampleStripControls();

    masterGainSlider.addListener(this);


    // Preset associated stuff
    addAndMakeVisible(&addPresetBtn);
    addPresetBtn.addListener(this);
    addPresetBtn.setBounds(130, 350, 70, 25);

    // set up the various panels (settings, mapping, etc)
    setupPanels();

    // start timer to update play positions, slider values etc.
    startTimer(50);

    // This tells the GUI to use a custom "theme"
    LookAndFeel::setDefaultLookAndFeel(&myLookAndFeel);
}

mlrVSTAudioProcessorEditor::~mlrVSTAudioProcessorEditor()
{
    sampleStripControlArray.clear(true);

    DBG("GUI destructor finished.");
}

void mlrVSTAudioProcessorEditor::buildSampleStripControls()
{
    // make sure we start from scratch
    sampleStripControlArray.clear(true);

    int numStrips = parent->getNumSampleStrips();

    // Add SampleStripControls, loading settings from the corresponding SampleStrip
    for(int i = 0; i < numStrips; ++i)
    {
        // this is passed to the SampleStripControl to allow data to be stored
        SampleStrip * currentStrip = parent->getSampleStrip(i);

        sampleStripControlArray.add(new SampleStripControl(i, waveformControlWidth,
            waveformControlHeight, numChannels, currentStrip, parent));

        int stripX = getWidth() - waveformControlWidth - PAD_AMOUNT;
        int stripY = PAD_AMOUNT + i * (waveformControlHeight + PAD_AMOUNT);
        sampleStripControlArray[i]->setBounds(stripX, stripY, waveformControlWidth, waveformControlHeight);
        addAndMakeVisible( sampleStripControlArray[i] );

        // Programmatically load parameters
        for(int p = SampleStrip::FirstParam; p < SampleStrip::NumGUIParams; ++p)
        {
            const void *newValue = parent->getSampleStripParameter(p, i);
            sampleStripControlArray[i]->recallParam(p, newValue, true);
        }
        DBG("params loaded for strip #" << i);
    }
}

void mlrVSTAudioProcessorEditor::buildSliders()
{
    xPosition = yPosition = PAD_AMOUNT;

    // work out the correct height width
    const int sliderWidth = (int)(270 / (float) (numChannels + 1));
    const int sliderHeight = waveformControlHeight - 16;

    // clear any existing sliders
    slidersArray.clear();
    slidersMuteBtnArray.clear();

    // Set master volume first
    addAndMakeVisible(&masterGainSlider);
    masterGainSlider.setSliderStyle(Slider::LinearVertical);

    masterGainSlider.setRange(0.0, 1.0, 0.01);
    masterGainSlider.setValue(0.0, false);
    masterGainSlider.setBounds(xPosition, yPosition, sliderWidth, sliderHeight);
    masterGainSlider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
    masterGainSlider.setColour(Slider::thumbColourId, Colours::darkgrey);
    masterGainSlider.setColour(Slider::backgroundColourId, (Colours::darkgrey).darker());
    masterGainSlider.setLookAndFeel(&menuLF);
    // and its label
    addAndMakeVisible(&masterSliderLabel);
    masterSliderLabel.setBounds(xPosition, yPosition + sliderHeight, sliderWidth, 16);
    masterSliderLabel.setColour(Label::backgroundColourId, Colours::black);
    masterSliderLabel.setColour(Label::textColourId, Colours::white);
    masterSliderLabel.setFont(fontSize);

    xPosition += 1;

    // then individual channel volumes
    for(int i = 0; i < numChannels; ++i)
    {
        xPosition += sliderWidth;

        slidersArray.add(new Slider("channel " + String(i) + " vol"));
        addAndMakeVisible(slidersArray[i]);
        slidersArray[i]->setSliderStyle(Slider::LinearVertical);
        slidersArray[i]->addListener(this);
        slidersArray[i]->setRange(0.0, 1.0, 0.01);
        slidersArray[i]->setBounds(xPosition, yPosition, sliderWidth - 1, sliderHeight);
        slidersArray[i]->setValue(parent->getChannelGain(i));

        Colour sliderColour = parent->getChannelColour(i);
        slidersArray[i]->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
        slidersArray[i]->setLookAndFeel(&menuLF);
        slidersArray[i]->setColour(Slider::thumbColourId, sliderColour);
        slidersArray[i]->setColour(Slider::backgroundColourId, sliderColour.darker());

        // add the labels
        slidersMuteBtnArray.add(new ToggleButton("ch " + String(i)));
        addAndMakeVisible(slidersMuteBtnArray[i]);
        slidersMuteBtnArray[i]->setBounds(xPosition, yPosition + sliderHeight, sliderWidth - 1, 16);
        slidersMuteBtnArray[i]->setColour(Label::backgroundColourId, Colours::black);
        slidersMuteBtnArray[i]->setColour(Label::textColourId, Colours::white);
        slidersMuteBtnArray[i]->addListener(this);
        slidersMuteBtnArray[i]->setToggleState(parent->getChannelMuteStatus(i), false);
        //slidersMuteBtnArray[i]->setFont(fontSilk, fontSize);
    }

    // increment yPosition for the next GUI item
    xPosition = PAD_AMOUNT;
    yPosition += waveformControlHeight + PAD_AMOUNT;
}

//==============================================================================
void mlrVSTAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colours::grey);      // fill with background colour
}


//==============================================================================
// This timer periodically checks whether any of the filter's
// parameters have changed. In pratical terms, this is usually
// to see if the host has modified the parameters.
void mlrVSTAudioProcessorEditor::timerCallback()
{

    if (useExternalTempo)
    {
        AudioPlayHead::CurrentPositionInfo newPos(parent->lastPosInfo);

        if (lastDisplayedPosition != newPos) bpmSlider.setValue(newPos.bpm);
    }

    recordBtn.setPercentDone(parent->getRecordingPrecountPercent(),
                             parent->getRecordingPercent());
    resampleBtn.setPercentDone(parent->getResamplingPrecountPercent(),
                               parent->getResamplingPercent());

    // see if the host has changed the master gain
    masterGainSlider.setValue(parent->getParameter(mlrVSTAudioProcessor::pMasterGainParam));

    // update the playback position
    for(int i = 0; i < sampleStripControlArray.size(); ++i)
    {
        sampleStripControlArray[i]->updatePlaybackStatus();
        sampleStripControlArray[i]->updateParamsIfChanged();
    }
}

// This is the callback for when the user drags a slider.
void mlrVSTAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    if (slider == &masterGainSlider)
    {
        // It's vital to use setParameterNotifyingHost to change any parameters that are automatable
        // by the host, rather than just modifying them directly, otherwise the host won't know
        // that they've changed.
        parent->setParameterNotifyingHost(mlrVSTAudioProcessor::pMasterGainParam,
                                                   (float) masterGainSlider.getValue());
    }
    else if (slider == &bpmSlider)
    {
        double newBPM = bpmSlider.getValue();
        parent->updateGlobalSetting(mlrVSTAudioProcessor::sCurrentBPM, &newBPM);
    }

    else if (slider == &resampleLengthSldr)
    {
        const int resampleLength = (int) resampleLengthSldr.getValue();
        parent->updateGlobalSetting(mlrVSTAudioProcessor::sResampleLength, &resampleLength);
    }
    else if (slider == &resamplePrecountSldr)
    {
        const int resamplePrecount = (int) resamplePrecountSldr.getValue();
        parent->updateGlobalSetting(mlrVSTAudioProcessor::sResamplePrecount, &resamplePrecount);
    }
    else if (slider == &resampleBankSldr)
    {
        const int resampleBank = (int) resampleBankSldr.getValue();
        parent->updateGlobalSetting(mlrVSTAudioProcessor::sResampleBank, &resampleBank);
    }
    else if (slider == &recordLengthSldr)
    {
        const int recordLength = (int) recordLengthSldr.getValue();
        parent->updateGlobalSetting(mlrVSTAudioProcessor::sRecordLength, &recordLength);
    }
    else if (slider == &recordPrecountSldr)
    {
        const int recordPrecount = (int) recordPrecountSldr.getValue();
        parent->updateGlobalSetting(mlrVSTAudioProcessor::sRecordPrecount, &recordPrecount);
    }
    else if (slider == &recordBankSldr)
    {
        const int recordBank = (int) recordBankSldr.getValue();
        parent->updateGlobalSetting(mlrVSTAudioProcessor::sRecordBank, &recordBank);
    }


    else
    {
        // check the channel volume notifications
        for(int i = 0; i < slidersArray.size(); ++i)
        {
            if (slider == slidersArray[i])
            {
                jassert(i < 8);     // we should not have more than 8 channels
                float newChannelGainValue = (float) slidersArray[i]->getValue();

                // let host know about the new value
                // channel0GainParam is the first channel id, so +i to access the rest
                parent->setChannelGain(i, newChannelGainValue);
            }
        }
    }

}

void mlrVSTAudioProcessorEditor::comboBoxChanged(ComboBox* comboBoxChanged)
{
    if (comboBoxChanged == &quantiseSettingsCbox)
    {
        const int choice = quantiseSettingsCbox.getSelectedId();

        if (choice > 0)
            parent->updateGlobalSetting(mlrVSTAudioProcessor::sQuantiseMenuSelection, &choice);
    }
}

void mlrVSTAudioProcessorEditor::buttonClicked(Button* btn)
{
    if(btn == &toggleSetlistBtn)
    {
        const bool currentlyVisible = presetPanel.isVisible();
        // close any existing panels
        closePanels();
        // Toggle the visibility of the setlist manager panel
        presetPanel.setVisible(!currentlyVisible);
        // and update the button
        toggleSetlistBtn.setToggleState(!currentlyVisible, false);
    }

    else if(btn == &toggleSettingsBtn)
    {
        const bool currentlyVisible = settingsPanel.isVisible();
        // close any existing panels
        closePanels();
        // Toggle the visibility of the settings panel
        settingsPanel.setVisible(!currentlyVisible);
        // and update the button
        toggleSettingsBtn.setToggleState(!currentlyVisible, false);
    }

    else if(btn == &toggleMappingBtn)
    {
        const bool currentlyVisible = mappingPanel.isVisible();
        // close any existing panels
        closePanels();
        // Toggle the settings panel
        mappingPanel.setVisible(!currentlyVisible);
        // and update the button
        toggleMappingBtn.setToggleState(!currentlyVisible, false);
    }

    else if(btn == &addPresetBtn)
    {

    #if JUCE_MODAL_LOOPS_PERMITTED

        AlertWindow w("Save Preset",
                      "Please enter a name for this preset",
                      AlertWindow::NoIcon);

        w.setColour(AlertWindow::textColourId, Colours::black);
        w.setColour(AlertWindow::outlineColourId, Colours::black);

        w.addTextEditor("text", "preset name", "preset name:");

        w.addButton("ok", 1, KeyPress (KeyPress::returnKey, 0, 0));
        w.addButton("cancel", 0, KeyPress (KeyPress::escapeKey, 0, 0));


        if (w.runModalLoop() != 0) // is they picked 'ok'
        {
            // this is the text they entered..
            String newPresetName(w.getTextEditorContents("text"));

            parent->savePreset(newPresetName);
        }
    #endif
    }

    else if(btn == &resampleBtn)
    {
        parent->startResampling();
    }

    else if(btn == &recordBtn)
    {
        parent->startRecording();
    }

    // load files manually using file dialog
	else if(btn == &loadFilesBtn)
    {
        FileChooser myChooser ("Please choose a file:",
                               File::getSpecialLocation(File::userDesktopDirectory),
                               "*.wav;*.flac;*.ogg;*.aif;*.aiff;*.caf");

        // ask user to load at least one file
        if(myChooser.browseForMultipleFilesToOpen())
        {	
            // if sucessful, try to add these to the sample pool
            Array<File> newFiles = myChooser.getResults();
            for (int i = 0; i < newFiles.size(); ++i)
            {
                File currentFile = newFiles[i];
                parent->addNewSample(currentFile);
            }
		}
	}

    else
    {
        for (int s = 0; s < slidersMuteBtnArray.size(); ++s)
        {
            if (btn == slidersMuteBtnArray[s])
                parent->setChannelMute(s, btn->getToggleState());
        }
    }

}


// Setting handling stuff
void mlrVSTAudioProcessorEditor::updateGlobalSetting(const int &parameterID, const void *newValue)
{
    /* First let the processor store the setting (as
       mlrVSTAudioProcessorEditor will lose these on
       closing.
    */
    parent->updateGlobalSetting(parameterID, newValue);

    // a few settings are of interest to the GUI
    switch (parameterID)
    {
    case mlrVSTAudioProcessor::sUseExternalTempo :
        {
            useExternalTempo = *static_cast<const bool*>(newValue);

            if (useExternalTempo)
            {
                bpmSlider.setEnabled(false);
                bpmLabel.setText("BPM (EXTERNAL)", false);
            }
            else
            {
                bpmSlider.setEnabled(true);
                bpmLabel.setText("BPM (INTERNAL)", false);
            }

            break;
        }

    case mlrVSTAudioProcessor::sNumChannels :
        {
            numChannels = *static_cast<const int*>(newValue);
            buildSliders();
            // let the SampleStrips add the right number of buttons
            for(int i = 0; i < sampleStripControlArray.size(); ++i)
                sampleStripControlArray[i]->setNumChannels(numChannels);
        }
    }
}

void mlrVSTAudioProcessorEditor::setUpRecordResampleUI()
{
    addAndMakeVisible(&resampleBtn);
	resampleBtn.addListener(this);
	resampleBtn.setBounds(PAD_AMOUNT, 450, 70, 25);
    resampleBtn.setFont(fontSilk, fontSize);

    addAndMakeVisible(&precountLbl);
    precountLbl.setBounds(70 + 2*PAD_AMOUNT, 430, 50, 20);
    precountLbl.setFont(fontSize);

    addAndMakeVisible(&resamplePrecountSldr);
    resamplePrecountSldr.setBounds(70 + 2*PAD_AMOUNT, 450, 50, 25);
    resamplePrecountSldr.setRange(0.0, 8.0, 1.0);
    resamplePrecountSldr.setLookAndFeel(&menuLF);
    resamplePrecountSldr.setSliderStyle(Slider::LinearBar);
    resamplePrecountSldr.addListener(this);
    const int resamplePrecount = *static_cast<const int*>(parent->getGlobalSetting(mlrVSTAudioProcessor::sResamplePrecount));
    resamplePrecountSldr.setValue(resamplePrecount);

    addAndMakeVisible(&recordLengthLbl);
    recordLengthLbl.setBounds(120 + 3*PAD_AMOUNT, 430, 50, 20);
    recordLengthLbl.setFont(fontSize);

    addAndMakeVisible(&resampleLengthSldr);
    resampleLengthSldr.setBounds(120 + 3*PAD_AMOUNT, 450, 50, 25);
    resampleLengthSldr.setRange(1.0, 32.0, 1.0);
    resampleLengthSldr.setLookAndFeel(&menuLF);
    resampleLengthSldr.setSliderStyle(Slider::LinearBar);
    resampleLengthSldr.addListener(this);
    const int resampleLength = *static_cast<const int*>(parent->getGlobalSetting(mlrVSTAudioProcessor::sResampleLength));
    resampleLengthSldr.setValue(resampleLength);

    addAndMakeVisible(&bankLbl);
    bankLbl.setBounds(170 + 4*PAD_AMOUNT, 430, 50, 20);
    bankLbl.setFont(fontSize);

    addAndMakeVisible(&resampleBankSldr);
    resampleBankSldr.setBounds(170 + 4*PAD_AMOUNT, 450, 50, 25);
    resampleBankSldr.setRange(0.0, 7.0, 1.0);
    resampleBankSldr.setLookAndFeel(&menuLF);
    resampleBankSldr.setSliderStyle(Slider::LinearBar);
    resampleBankSldr.addListener(this);
    const int resampleBank = *static_cast<const int*>(parent->getGlobalSetting(mlrVSTAudioProcessor::sResampleBank));
    resampleBankSldr.setValue(resampleBank);



    addAndMakeVisible(&recordBtn);
	recordBtn.addListener(this);
    recordBtn.setBounds(PAD_AMOUNT, 480, 70, 25);
    recordBtn.setFont(fontSilk, fontSize);

    addAndMakeVisible(&recordPrecountSldr);
    recordPrecountSldr.setBounds(70 + 2*PAD_AMOUNT, 480, 50, 25);
    recordPrecountSldr.setRange(0.0, 8.0, 1.0);
    recordPrecountSldr.setLookAndFeel(&menuLF);
    recordPrecountSldr.setSliderStyle(Slider::LinearBar);
    recordPrecountSldr.addListener(this);
    const int recordPrecount = *static_cast<const int*>(parent->getGlobalSetting(mlrVSTAudioProcessor::sRecordPrecount));
    recordPrecountSldr.setValue(recordPrecount);

    addAndMakeVisible(&recordLengthSldr);
    recordLengthSldr.setBounds(120 + 3*PAD_AMOUNT, 480, 50, 25);
    recordLengthSldr.setRange(1.0, 32.0, 1.0);
    recordLengthSldr.setLookAndFeel(&menuLF);
    recordLengthSldr.setSliderStyle(Slider::LinearBar);
    recordLengthSldr.addListener(this);
    const int recordLength = *static_cast<const int*>(parent->getGlobalSetting(mlrVSTAudioProcessor::sRecordLength));
    recordLengthSldr.setValue(recordLength);

    addAndMakeVisible(&recordBankSldr);
    recordBankSldr.setBounds(170 + 4*PAD_AMOUNT, 480, 50, 25);
    recordBankSldr.setRange(0.0, 7.0, 1.0);
    recordBankSldr.setLookAndFeel(&menuLF);
    recordBankSldr.setSliderStyle(Slider::LinearBar);
    recordBankSldr.addListener(this);
    const int recordBank = *static_cast<const int*>(parent->getGlobalSetting(mlrVSTAudioProcessor::sRecordBank));
    resampleBankSldr.setValue(recordBank);

}

void mlrVSTAudioProcessorEditor::setupTempoUI()
{
    int bpmSliderWidth = 180, bpmSliderHeight = 32;
    int bpmLabelWidth = bpmSliderWidth, bpmLabelHeight = 16;

    int quantiseBoxWidth = 80, quantiseBoxHeight = 32;
    int quantiseLabelWidth = quantiseBoxWidth, quantiseLabelHeight = 16;

    xPosition = PAD_AMOUNT;

    ///////////////////////
    // First add the labels
    addAndMakeVisible(&bpmLabel);
    bpmLabel.setBounds(xPosition, yPosition, bpmLabelWidth, bpmLabelHeight);
    bpmLabel.setFont(fontSize);
    bpmLabel.setColour(Label::textColourId, Colours::white);
    bpmLabel.setColour(Label::backgroundColourId, Colours::black);
    xPosition += bpmLabelWidth + PAD_AMOUNT;

    addAndMakeVisible(&quantiseLabel);
    quantiseLabel.setBounds(xPosition, yPosition, quantiseLabelWidth, quantiseLabelHeight);
    quantiseLabel.setFont(fontSize);
    quantiseLabel.setColour(Label::textColourId, Colours::white);
    quantiseLabel.setColour(Label::backgroundColourId, Colours::black);
    quantiseLabel.setText("QUANTISATION", false);

    xPosition = PAD_AMOUNT;
    yPosition += quantiseLabelHeight;



    ///////////////////////////////
    // Then add sliders and boxes
    addAndMakeVisible(&bpmSlider);
    bpmSlider.setBounds(xPosition, yPosition, bpmSliderWidth, bpmSliderHeight);
    bpmSlider.setColour(Slider::backgroundColourId, Colours::black.withAlpha(0.3f));
    bpmSlider.setSliderStyle(Slider::LinearBar);
    bpmSlider.setRange(20.0, 300.0, 0.01);
    bpmSlider.setTextBoxIsEditable(true);
    bpmSlider.addListener(this);

    useExternalTempo = *static_cast<const bool*>(getGlobalSetting(mlrVSTAudioProcessor::sUseExternalTempo));
    if (useExternalTempo)
    {
        bpmSlider.setEnabled(false);
        bpmLabel.setText("BPM (EXTERNAL)", false);
    }
    else
    {
        double newBPM = *static_cast<const double*>(getGlobalSetting(mlrVSTAudioProcessor::sCurrentBPM));
        bpmSlider.setValue(newBPM);
        bpmLabel.setText("BPM (INTERNAL)", false);
    }

    xPosition += bpmSliderWidth + PAD_AMOUNT;

    // add dropdown box for quantisation settings
    addAndMakeVisible(&quantiseSettingsCbox);
    quantiseSettingsCbox.setBounds(xPosition, yPosition, quantiseBoxWidth, quantiseBoxHeight);
    quantiseSettingsCbox.addListener(this);

    // add items to it
    quantiseSettingsCbox.addItem("None", 1);
    for (int i = 2, denom = 1; i < 8; ++i, denom *= 2)
        quantiseSettingsCbox.addItem(String(denom) + "n", i);

    // and load the stored selection if suitable
    const int menuSelection = *static_cast<const int*>(parent->getGlobalSetting(mlrVSTAudioProcessor::sQuantiseMenuSelection));
    if (menuSelection >= 0 && menuSelection < 8) quantiseSettingsCbox.setSelectedId(menuSelection);
    else quantiseSettingsCbox.setSelectedId(1);

    xPosition = PAD_AMOUNT;
    yPosition += bpmSliderHeight + PAD_AMOUNT;
}

void mlrVSTAudioProcessorEditor::setupPanels()
{
    const int buttonHeight = 25;
    const int buttonWidth = 60;

    // button to toggle the settings panel
    addAndMakeVisible(&toggleSettingsBtn);
    toggleSettingsBtn.addListener(this);
    toggleSettingsBtn.setBounds(xPosition, yPosition, buttonWidth, buttonHeight);
    // add the actual panel
    addChildComponent(&settingsPanel);
    settingsPanel.setBounds(settingsPanelBounds);
    xPosition += PAD_AMOUNT + buttonWidth;


    // button to toggle the setlist panel
    addAndMakeVisible(&toggleSetlistBtn);
    toggleSetlistBtn.addListener(this);
    toggleSetlistBtn.setBounds(xPosition, yPosition, buttonWidth, buttonHeight);
    // add the actual button
    addChildComponent(&presetPanel);
    presetPanel.setBounds(presetPanelBounds);
    xPosition += PAD_AMOUNT + buttonWidth;

    // button to toggle mappings panel
    addAndMakeVisible(&toggleMappingBtn);
    toggleMappingBtn.addListener(this);
    toggleMappingBtn.setBounds(xPosition, yPosition, buttonWidth, buttonHeight);
    // add the actual panel
    addChildComponent(&mappingPanel);
    mappingPanel.setBounds(mappingPanelBounds);

    xPosition = PAD_AMOUNT;
    yPosition += buttonHeight + PAD_AMOUNT;
}

// Force any visible panels to close
void mlrVSTAudioProcessorEditor::closePanels()
{
    presetPanel.setVisible(false);
    toggleSetlistBtn.setToggleState(false, false);

    mappingPanel.setVisible(false);
    toggleMappingBtn.setToggleState(false, false);

    settingsPanel.setVisible(false);
    toggleSettingsBtn.setToggleState(false, false);
}