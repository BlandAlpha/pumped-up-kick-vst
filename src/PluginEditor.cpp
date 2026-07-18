#include "PluginEditor.h"
#include "Presets.h"

namespace
{
    constexpr int factoryIdBase = 1;      // combo ids 1..N
    constexpr int userIdBase    = 1000;   // combo ids 1000..
}

//==============================================================================
PumpedUpKickEditor::PumpedUpKickEditor (PumpedUpKickProcessor& p)
    : AudioProcessorEditor (p), processor (p), envelope (p.getCurve())
{
    setLookAndFeel (&lookAndFeel);

    //==========================================================================
    addAndMakeVisible (envelope);
    envelope.onCurveChanged = [this]
    {
        processor.curveChanged();
        markCustom();
    };

    //==========================================================================
    addAndMakeVisible (presetBox);
    presetBox.setTextWhenNothingSelected ("Custom");
    presetBox.onChange = [this] { presetSelected(); };
    rebuildPresetMenu();

    for (auto* b : { &prevButton, &nextButton, &saveButton })
    {
        addAndMakeVisible (*b);
        b->setColour (juce::TextButton::textColourOffId, Palette::text);
    }
    prevButton.onClick = [this] { stepPreset (-1); };
    nextButton.onClick = [this] { stepPreset (+1); };
    saveButton.onClick = [this] { promptSavePreset(); };

    //==========================================================================
    auto setupKnob = [this] (juce::Slider& knob, juce::Label& label)
    {
        knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 68, 16);
        addAndMakeVisible (knob);

        label.setJustificationType (juce::Justification::centred);
        label.setFont (lookAndFeel.getCaptionFont (11.0f));
        addAndMakeVisible (label);
    };
    setupKnob (mixKnob, mixLabel);
    setupKnob (smoothKnob, smoothLabel);

    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, "mix", mixKnob);
    smoothAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, "smooth", smoothKnob);

    //==========================================================================
    // Segmented button groups for Rate and Trigger mode
    auto setupGroup = [this] (std::vector<std::unique_ptr<juce::TextButton>>& buttons,
                              const juce::StringArray& names, const juce::String& paramID,
                              std::unique_ptr<juce::ParameterAttachment>& attachment,
                              juce::Label& label)
    {
        auto* param = processor.apvts.getParameter (paramID);

        for (int i = 0; i < names.size(); ++i)
        {
            auto b = std::make_unique<juce::TextButton> (names[i]);
            b->setClickingTogglesState (false);
            b->onClick = [param, i, size = names.size()]
            {
                param->setValueNotifyingHost ((float) i / (float) (size - 1));
            };
            addAndMakeVisible (*b);
            buttons.push_back (std::move (b));
        }

        attachment = std::make_unique<juce::ParameterAttachment> (
            *param,
            [&buttons, param] (float newValue)
            {
                const int index = juce::roundToInt (
                    param->convertTo0to1 (newValue) * (float) (buttons.size() - 1));
                for (int i = 0; i < (int) buttons.size(); ++i)
                    buttons[(size_t) i]->setToggleState (i == index,
                                                         juce::dontSendNotification);
            });
        attachment->sendInitialUpdate();

        label.setJustificationType (juce::Justification::centred);
        label.setFont (lookAndFeel.getCaptionFont (11.0f));
        addAndMakeVisible (label);
    };

    setupGroup (rateButtons, { "1/1", "1/2", "1/4", "1/8", "1/16" }, "rate",
                rateAttachment, rateLabel);
    setupGroup (modeButtons, { "SYNC", "MIDI" }, "mode", modeAttachment, modeLabel);

    //==========================================================================
    syncPresetDisplay();

    setResizable (true, true);
    setResizeLimits (480, 350, 1120, 820);
    getConstrainer()->setFixedAspectRatio (560.0 / 410.0);
    setSize (560, 410);

    startTimerHz (60);
}

PumpedUpKickEditor::~PumpedUpKickEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void PumpedUpKickEditor::timerCallback()
{
    static_assert (PumpedUpKickProcessor::waveformBins == EnvelopeEditor::waveformBins,
                   "waveform bin counts must match");

    envelope.setWaveform (processor.waveIn.data(), processor.waveOut.data());
    envelope.setPlayhead (processor.uiPhase.load (std::memory_order_relaxed),
                          processor.uiGain.load (std::memory_order_relaxed),
                          processor.uiActive.load (std::memory_order_relaxed));
}

//==============================================================================
void PumpedUpKickEditor::paint (juce::Graphics& g)
{
    g.fillAll (Palette::background);

    auto header = getLocalBounds().removeFromTop (44).reduced (14, 0);
    g.setColour (Palette::text);
    g.setFont (lookAndFeel.getLogoFont (23.0f));
    g.drawText ("PUMPED UP KICK", header, juce::Justification::centredLeft);
}

void PumpedUpKickEditor::resized()
{
    auto area = getLocalBounds();

    // Header: title (painted) + preset controls on the right
    auto header = area.removeFromTop (44).reduced (14, 8);
    saveButton.setBounds (header.removeFromRight (52));
    header.removeFromRight (6);
    nextButton.setBounds (header.removeFromRight (26));
    presetBox.setBounds (header.removeFromRight (150));
    prevButton.setBounds (header.removeFromRight (26));

    // Bottom control bar
    auto controls = area.removeFromBottom (104).reduced (14, 8);

    auto knobArea = controls.removeFromLeft (100);
    mixLabel.setBounds (knobArea.removeFromBottom (14));
    mixKnob.setBounds (knobArea);

    knobArea = controls.removeFromLeft (84);
    smoothLabel.setBounds (knobArea.removeFromBottom (14));
    smoothKnob.setBounds (knobArea);

    controls.removeFromLeft (10);

    auto rateArea = controls.removeFromLeft (juce::jmax (170, controls.getWidth() - 130));
    rateLabel.setBounds (rateArea.removeFromBottom (14));
    rateArea = rateArea.withSizeKeepingCentre (rateArea.getWidth(), 30);
    const int rw = rateArea.getWidth() / (int) rateButtons.size();
    for (auto& b : rateButtons)
        b->setBounds (rateArea.removeFromLeft (rw).reduced (2, 0));

    controls.removeFromLeft (10);
    modeLabel.setBounds (controls.removeFromBottom (14));
    controls = controls.withSizeKeepingCentre (controls.getWidth(), 30);
    const int mw = controls.getWidth() / (int) modeButtons.size();
    for (auto& b : modeButtons)
        b->setBounds (controls.removeFromLeft (mw).reduced (2, 0));

    // Envelope fills the middle
    envelope.setBounds (area.reduced (14, 4));
}

//==============================================================================
juce::File PumpedUpKickEditor::getUserPresetDirectory()
{
    auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
   #if JUCE_MAC
    dir = dir.getChildFile ("Application Support");
   #endif
    return dir.getChildFile ("PumpedUpKick").getChildFile ("Presets");
}

void PumpedUpKickEditor::rebuildPresetMenu()
{
    presetBox.clear (juce::dontSendNotification);

    int id = factoryIdBase;
    for (const auto& preset : FactoryPresets::getAll())
        presetBox.addItem (preset.name, id++);

    userPresetFiles.clear();
    const auto dir = getUserPresetDirectory();
    if (dir.isDirectory())
        userPresetFiles = dir.findChildFiles (juce::File::findFiles, false, "*.pukpreset");
    userPresetFiles.sort();

    if (! userPresetFiles.isEmpty())
    {
        presetBox.addSeparator();
        for (int i = 0; i < userPresetFiles.size(); ++i)
            presetBox.addItem (userPresetFiles[i].getFileNameWithoutExtension(),
                               userIdBase + i);
    }
}

void PumpedUpKickEditor::presetSelected()
{
    const int id = presetBox.getSelectedId();
    if (id >= userIdBase)
    {
        const int index = id - userIdBase;
        if (index < userPresetFiles.size())
            loadUserPreset (userPresetFiles[index]);
    }
    else if (id >= factoryIdBase)
    {
        loadFactoryPreset (id - factoryIdBase);
    }
}

void PumpedUpKickEditor::loadFactoryPreset (int index)
{
    if (index < 0 || index >= FactoryPresets::count())
        return;
    const auto& preset = FactoryPresets::getAll()[(size_t) index];
    processor.getCurve().setPoints (preset.points);
    processor.curveChanged();
    processor.currentPresetName = preset.name;
    envelope.repaint();
}

void PumpedUpKickEditor::loadUserPreset (const juce::File& file)
{
    if (auto xml = juce::parseXML (file))
    {
        auto tree = juce::ValueTree::fromXml (*xml);
        auto curveTree = tree.hasType ("CURVE") ? tree : tree.getChildWithName ("CURVE");
        if (curveTree.isValid())
        {
            processor.getCurve().fromValueTree (curveTree);
            processor.curveChanged();
            processor.currentPresetName = file.getFileNameWithoutExtension();
            envelope.repaint();
        }
    }
}

void PumpedUpKickEditor::stepPreset (int direction)
{
    const int count = presetBox.getNumItems();
    if (count == 0)
        return;

    int index = presetBox.getSelectedItemIndex();
    index = index < 0 ? 0 : (index + direction + count) % count;
    presetBox.setSelectedItemIndex (index);   // triggers onChange -> load
}

void PumpedUpKickEditor::promptSavePreset()
{
    auto* window = new juce::AlertWindow ("Save Preset",
                                          "Name your preset:",
                                          juce::MessageBoxIconType::NoIcon, this);
    const auto initial = processor.currentPresetName == "Custom" ? juce::String()
                                                                 : processor.currentPresetName;
    window->addTextEditor ("name", initial);
    window->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey));
    window->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    window->enterModalState (true,
        juce::ModalCallbackFunction::create ([this, window] (int result)
        {
            if (result == 1)
            {
                const auto name = window->getTextEditorContents ("name").trim();
                if (name.isNotEmpty())
                    saveUserPreset (name);
            }
        }), true);
}

void PumpedUpKickEditor::saveUserPreset (const juce::String& name)
{
    const auto dir = getUserPresetDirectory();
    dir.createDirectory();

    const auto safeName = juce::File::createLegalFileName (name);
    const auto file = dir.getChildFile (safeName + ".pukpreset");

    if (auto xml = processor.getCurve().toValueTree().createXml())
        xml->writeTo (file);

    processor.currentPresetName = name;
    rebuildPresetMenu();
    syncPresetDisplay();
}

void PumpedUpKickEditor::markCustom()
{
    if (processor.currentPresetName != "Custom")
    {
        processor.currentPresetName = "Custom";
        presetBox.setSelectedId (0, juce::dontSendNotification);
        presetBox.setText ("Custom", juce::dontSendNotification);
    }
}

void PumpedUpKickEditor::syncPresetDisplay()
{
    const auto& name = processor.currentPresetName;

    for (int i = 0; i < presetBox.getNumItems(); ++i)
    {
        if (presetBox.getItemText (i) == name)
        {
            presetBox.setSelectedItemIndex (i, juce::dontSendNotification);
            return;
        }
    }
    presetBox.setSelectedId (0, juce::dontSendNotification);
    presetBox.setText (name, juce::dontSendNotification);
}
