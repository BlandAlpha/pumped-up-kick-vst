#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
// Palette + minimal flat look. Everything vector-drawn, no image resources.
//==============================================================================
namespace Palette
{
    const juce::Colour background   { 0xff14161a };
    const juce::Colour panel        { 0xff1b1e24 };
    const juce::Colour panelLight   { 0xff23272f };
    const juce::Colour grid         { 0xff2a2e37 };
    const juce::Colour gridStrong   { 0xff353a45 };
    const juce::Colour accent       { 0xffff6b52 };
    const juce::Colour accentDim    { 0xff8a4438 };
    const juce::Colour text         { 0xffe8e6e3 };
    const juce::Colour textDim      { 0xff8b909a };
    const juce::Colour playhead     { 0xffffc9bd };
}

class PukLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PukLookAndFeel()
    {
        setColour (juce::ResizableWindow::backgroundColourId, Palette::background);
        setColour (juce::ComboBox::backgroundColourId, Palette::panelLight);
        setColour (juce::ComboBox::textColourId, Palette::text);
        setColour (juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
        setColour (juce::ComboBox::arrowColourId, Palette::textDim);
        setColour (juce::PopupMenu::backgroundColourId, Palette::panelLight);
        setColour (juce::PopupMenu::textColourId, Palette::text);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, Palette::accent.withAlpha (0.25f));
        setColour (juce::PopupMenu::highlightedTextColourId, Palette::text);
        setColour (juce::TextButton::buttonColourId, Palette::panelLight);
        setColour (juce::TextButton::buttonOnColourId, Palette::accent);
        setColour (juce::TextButton::textColourOffId, Palette::textDim);
        setColour (juce::TextButton::textColourOnId, Palette::background);
        setColour (juce::Slider::textBoxTextColourId, Palette::text);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Label::textColourId, Palette::textDim);
        setColour (juce::TextEditor::backgroundColourId, Palette::panel);
        setColour (juce::TextEditor::textColourId, Palette::text);
        setColour (juce::TextEditor::highlightColourId, Palette::accent.withAlpha (0.4f));
        setColour (juce::TextEditor::focusedOutlineColourId, Palette::accent.withAlpha (0.6f));
        setColour (juce::AlertWindow::backgroundColourId, Palette::panel);
        setColour (juce::AlertWindow::textColourId, Palette::text);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (6.0f);
        const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre  = bounds.getCentre();
        const float angle  = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const float thickness = juce::jmax (3.0f, radius * 0.12f);
        const float arcRadius = radius - thickness * 0.5f;

        juce::Path track;
        track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                             rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (Palette::grid);
        g.strokePath (track, juce::PathStrokeType (thickness, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

        if (sliderPos > 0.001f)
        {
            juce::Path value;
            value.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                 rotaryStartAngle, angle, true);
            g.setColour (Palette::accent.withAlpha (slider.isEnabled() ? 1.0f : 0.4f));
            g.strokePath (value, juce::PathStrokeType (thickness, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));
        }

        // Needle
        const float inner = arcRadius * 0.45f;
        const juce::Point<float> tip   (centre.x + std::sin (angle) * (arcRadius - thickness),
                                        centre.y - std::cos (angle) * (arcRadius - thickness));
        const juce::Point<float> base  (centre.x + std::sin (angle) * inner,
                                        centre.y - std::cos (angle) * inner);
        g.setColour (Palette::text);
        g.drawLine ({ base, tip }, 2.4f);
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override
    {
        return juce::Font (juce::FontOptions (14.0f));
    }

    juce::Font getTextButtonFont (juce::TextButton&, int) override
    {
        return juce::Font (juce::FontOptions (13.0f, juce::Font::bold));
    }
};
