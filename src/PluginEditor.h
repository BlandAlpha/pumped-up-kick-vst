#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "EnvelopeEditor.h"
#include "LookAndFeel.h"

class PumpedUpKickEditor : public juce::AudioProcessorEditor,
                           private juce::Timer
{
public:
    explicit PumpedUpKickEditor (PumpedUpKickProcessor&);
    ~PumpedUpKickEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    void rebuildPresetMenu();
    void presetSelected();
    void loadFactoryPreset (int index);
    void loadUserPreset (const juce::File& file);
    void stepPreset (int direction);
    void promptSavePreset();
    void saveUserPreset (const juce::String& name);
    void markCustom();
    void syncPresetDisplay();

    static juce::File getUserPresetDirectory();

    PumpedUpKickProcessor& processor;
    PukLookAndFeel lookAndFeel;

    EnvelopeEditor envelope;

    juce::ComboBox presetBox;
    juce::TextButton prevButton { "<" }, nextButton { ">" }, saveButton { "SAVE" };

    juce::Slider mixKnob, smoothKnob;
    juce::Label mixLabel { {}, "MIX" }, smoothLabel { {}, "SMOOTH" };
    juce::Label rateLabel { {}, "RATE" }, modeLabel { {}, "TRIGGER" };

    std::vector<std::unique_ptr<juce::TextButton>> rateButtons, modeButtons;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment,
                                                                          smoothAttachment;
    std::unique_ptr<juce::ParameterAttachment> rateAttachment, modeAttachment;

    juce::Array<juce::File> userPresetFiles;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PumpedUpKickEditor)
};
