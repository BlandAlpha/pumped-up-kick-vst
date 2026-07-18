#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Curve.h"

class PumpedUpKickProcessor : public juce::AudioProcessor
{
public:
    PumpedUpKickProcessor();

    //==========================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                     { return true; }

    const juce::String getName() const override        { return "PumpedUpKick"; }
    bool acceptsMidi() const override                   { return true; }
    bool producesMidi() const override                  { return false; }
    double getTailLengthSeconds() const override        { return 0.0; }

    int getNumPrograms() override                       { return 1; }
    int getCurrentProgram() override                    { return 0; }
    void setCurrentProgram (int) override               {}
    const juce::String getProgramName (int) override    { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==========================================================================
    // Curve access (message thread edits, then calls curveChanged()).
    Curve& getCurve() noexcept                          { return curve; }
    void curveChanged()                                 { lut.render (curve); }

    juce::AudioProcessorValueTreeState apvts;

    // For UI animation (written by audio thread, read by editor).
    std::atomic<float> uiPhase   { 0.0f };
    std::atomic<float> uiGain    { 1.0f };
    std::atomic<bool>  uiActive  { false };  // envelope currently running

    // Per-cycle waveform display: peak of the last cycle, binned by phase.
    static constexpr int waveformBins = 512;
    std::array<std::atomic<float>, waveformBins> waveIn { };   // pre-gain peaks
    std::array<std::atomic<float>, waveformBins> waveOut { };  // post-gain peaks

    // Current preset display name (message thread only).
    juce::String currentPresetName { "Classic" };

    static constexpr double fallbackBpm = 120.0;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    double getBeatsPerCycle() const noexcept;

    Curve    curve;
    CurveLUT lut;

    // Phase engine state (audio thread only).
    double phase        = 0.0;      // 0..1 within one cycle
    bool   oneShotActive = false;   // MIDI mode: envelope currently running
    float  smoothedGain = 1.0f;
    double sampleRateHz = 44100.0;

    // Waveform-binning accumulators (audio thread only).
    int   waveBin    = -1;
    float binPeakIn  = 0.0f;
    float binPeakOut = 0.0f;

    std::atomic<float>* mixParam    = nullptr;
    std::atomic<float>* rateParam   = nullptr;
    std::atomic<float>* modeParam   = nullptr;
    std::atomic<float>* smoothParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PumpedUpKickProcessor)
};
