#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "BinaryData.h"

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
        // Special Gothic (SIL OFL 1.1) is compiled in via BinaryData, so the
        // UI renders identically regardless of what the host system has
        // installed. Medium is used for every control; the Condensed display
        // cut is reserved for the logo.
        //
        // Every font hook below attaches the typeface explicitly: fonts with
        // no explicit typeface resolve through the process-wide default
        // LookAndFeel, not the one set on this editor, so relying on
        // setDefaultSansSerifTypeface() here would silently fall back to the
        // system font (and mutating the global default is unsafe with
        // multiple plugin instances in one host).
        medium    = juce::Typeface::createSystemTypefaceFor (
                        BinaryData::SpecialGothicMedium_ttf,
                        BinaryData::SpecialGothicMedium_ttfSize);
        condensed = juce::Typeface::createSystemTypefaceFor (
                        BinaryData::SpecialGothicCondensedOneRegular_ttf,
                        BinaryData::SpecialGothicCondensedOneRegular_ttfSize);

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

    // Medium with the typeface attached explicitly — see the constructor
    // comment for why implicit resolution can't be trusted.
    juce::Font mediumFont (float height) const
    {
        return juce::Font (juce::FontOptions().withTypeface (medium)
                                              .withHeight (height));
    }

    // Condensed all-caps display cut; a touch of tracking opens it up.
    juce::Font getLogoFont (float height) const
    {
        return juce::Font (juce::FontOptions().withTypeface (condensed)
                                              .withHeight (height))
                   .withExtraKerningFactor (0.03f);
    }

    // Small all-caps section captions (MIX / RATE / …).
    juce::Font getCaptionFont (float height) const
    {
        return mediumFont (height).withExtraKerningFactor (0.08f);
    }

    //==========================================================================
    // Font hooks: every piece of text the UI draws goes through one of these.
    juce::Font getComboBoxFont (juce::ComboBox&) override        { return mediumFont (15.0f); }
    juce::Font getPopupMenuFont() override                       { return mediumFont (15.0f); }
    // Medium already carries its weight — no synthetic bold on top of it.
    juce::Font getTextButtonFont (juce::TextButton&, int) override { return mediumFont (14.0f); }
    juce::Font getAlertWindowTitleFont() override                { return mediumFont (17.0f); }
    juce::Font getAlertWindowMessageFont() override              { return mediumFont (15.0f); }
    juce::Font getAlertWindowFont() override                     { return mediumFont (14.0f); }

    // Covers every Label that didn't opt into a specific font — most notably
    // the slider value boxes ("80 %", "2.0 ms").
    juce::Font getLabelFont (juce::Label& label) override
    {
        const auto current = label.getFont();
        if (current.getTypefaceName() == juce::Font::getDefaultSansSerifFontName())
            return mediumFont (current.getHeight());
        return current;   // keeps fonts set explicitly (e.g. the captions)
    }

private:
    juce::Typeface::Ptr medium, condensed;
};
