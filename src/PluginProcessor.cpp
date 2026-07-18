#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

namespace
{
    // Rate choices in beats per envelope cycle (4/4: "1/4" = one duck per beat).
    constexpr double rateBeats[] = { 4.0, 2.0, 1.0, 0.5, 0.25 };
}

PumpedUpKickProcessor::PumpedUpKickProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    mixParam    = apvts.getRawParameterValue ("mix");
    rateParam   = apvts.getRawParameterValue ("rate");
    modeParam   = apvts.getRawParameterValue ("mode");
    smoothParam = apvts.getRawParameterValue ("smooth");

    curve.setPoints (FactoryPresets::getAll()[0].points);
    lut.render (curve);
}

juce::AudioProcessorValueTreeState::ParameterLayout PumpedUpKickProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "mix", 1 }, "Mix",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction (
            [] (float v, int) { return String (roundToInt (v * 100.0f)) + " %"; })));

    layout.add (std::make_unique<AudioParameterChoice> (
        ParameterID { "rate", 1 }, "Rate",
        StringArray { "1/1", "1/2", "1/4", "1/8", "1/16" }, 2));

    layout.add (std::make_unique<AudioParameterChoice> (
        ParameterID { "mode", 1 }, "Trigger",
        StringArray { "Sync", "MIDI" }, 0));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "smooth", 1 }, "Smooth",
        NormalisableRange<float> (0.1f, 20.0f, 0.1f, 0.5f), 2.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction (
            [] (float v, int) { return String (v, 1) + " ms"; })));

    return layout;
}

double PumpedUpKickProcessor::getBeatsPerCycle() const noexcept
{
    const int idx = juce::jlimit (0, 4, (int) rateParam->load());
    return rateBeats[idx];
}

void PumpedUpKickProcessor::prepareToPlay (double sampleRate, int)
{
    sampleRateHz  = sampleRate;
    smoothedGain  = 1.0f;
    phase         = 0.0;
    oneShotActive = false;
}

bool PumpedUpKickProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();
    return in == out && (out == juce::AudioChannelSet::mono()
                      || out == juce::AudioChannelSet::stereo());
}

void PumpedUpKickProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numSamples == 0)
        return;

    const bool  midiMode = modeParam->load() > 0.5f;
    const float mix      = mixParam->load();
    const double beats   = getBeatsPerCycle();

    double bpm = fallbackBpm;
    bool hostPlaying = false;
    double hostPpq = 0.0;
    bool hasPpq = false;

    if (auto* playHead = getPlayHead())
    {
        if (auto info = playHead->getPosition())
        {
            if (auto b = info->getBpm())
                bpm = *b > 1.0 ? *b : fallbackBpm;
            hostPlaying = info->getIsPlaying();
            if (auto ppq = info->getPpqPosition())
            {
                hostPpq = *ppq;
                hasPpq = true;
            }
        }
    }

    const double phaseInc = (bpm / 60.0) / (beats * sampleRateHz);

    // One-pole gain smoothing (time constant from the Smooth parameter).
    const float smoothMs   = smoothParam->load();
    const float smoothCoef = std::exp (-1.0f / ((smoothMs / 1000.0f) * (float) sampleRateHz));

    // Sync mode: lock phase to the host timeline at block start when playing;
    // free-run at host BPM otherwise so you hear the pump while auditioning.
    if (! midiMode && hostPlaying && hasPpq)
    {
        double p = std::fmod (hostPpq / beats, 1.0);
        if (p < 0.0)
            p += 1.0;
        phase = p;
    }

    auto midiIt = midi.begin();

    for (int i = 0; i < numSamples; ++i)
    {
        if (midiMode)
        {
            while (midiIt != midi.end() && (*midiIt).samplePosition <= i)
            {
                const auto msg = (*midiIt).getMessage();
                if (msg.isNoteOn())
                {
                    phase = 0.0;
                    oneShotActive = true;
                }
                ++midiIt;
            }
        }

        float shape = 1.0f;

        if (midiMode)
        {
            if (oneShotActive)
            {
                shape = lut.get ((float) phase);
                phase += phaseInc;
                if (phase >= 1.0)
                {
                    phase = 0.0;
                    oneShotActive = false;
                }
            }
        }
        else
        {
            shape = lut.get ((float) phase);
            phase += phaseInc;
            if (phase >= 1.0)
                phase -= 1.0;
        }

        const float target = 1.0f + mix * (shape - 1.0f);
        smoothedGain = target + (smoothedGain - target) * smoothCoef;

        for (int ch = 0; ch < numChannels; ++ch)
            buffer.getWritePointer (ch)[i] *= smoothedGain;
    }

    uiPhase.store ((float) phase, std::memory_order_relaxed);
    uiGain.store (smoothedGain, std::memory_order_relaxed);
    uiActive.store (! midiMode || oneShotActive, std::memory_order_relaxed);

    midi.clear();
}

//==============================================================================
void PumpedUpKickProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ValueTree state ("PUMPEDUPKICK");
    state.appendChild (apvts.copyState(), nullptr);
    state.appendChild (curve.toValueTree(), nullptr);
    state.setProperty ("presetName", currentPresetName, nullptr);

    juce::MemoryOutputStream stream (destData, false);
    state.writeToStream (stream);
}

void PumpedUpKickProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto state = juce::ValueTree::readFromData (data, (size_t) sizeInBytes);
    if (! state.isValid() || ! state.hasType ("PUMPEDUPKICK"))
        return;

    auto params = state.getChildWithName ("PARAMS");
    if (params.isValid())
        apvts.replaceState (params);

    auto curveTree = state.getChildWithName ("CURVE");
    if (curveTree.isValid())
    {
        curve.fromValueTree (curveTree);
        lut.render (curve);
    }

    currentPresetName = state.getProperty ("presetName", "Custom").toString();
}

juce::AudioProcessorEditor* PumpedUpKickProcessor::createEditor()
{
    return new PumpedUpKickEditor (*this);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PumpedUpKickProcessor();
}
