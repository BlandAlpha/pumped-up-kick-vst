#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Curve.h"
#include "LookAndFeel.h"

//==============================================================================
// Interactive envelope editor (LFOTool-style):
//   - drag points to move them (shift = snap to grid)
//   - double-click empty space to add a point
//   - double-click / alt-click / right-click a point to delete it
//   - drag a segment's midpoint handle vertically to bend the curve
// A playhead line + gain dot animate while audio runs (driven by the parent).
//==============================================================================
class EnvelopeEditor : public juce::Component
{
public:
    std::function<void()> onCurveChanged;   // called after any edit

    explicit EnvelopeEditor (Curve& curveToEdit) : curve (curveToEdit)
    {
        setOpaque (true);
    }

    void setPlayhead (float phase, float gain, bool active)
    {
        const bool changed = active != playheadActive
                             || std::abs (phase - playheadPhase) > 0.0005f;
        playheadPhase  = phase;
        playheadGain   = gain;
        playheadActive = active;
        if (changed)
            repaint();
    }

    //==========================================================================
    void paint (juce::Graphics& g) override
    {
        const auto area = getLocalBounds().toFloat();
        g.setColour (Palette::panel);
        g.fillRoundedRectangle (area, 8.0f);

        const auto plot = getPlotArea();

        // Grid: 16 columns (16th notes of the cycle), 4 rows
        g.setColour (Palette::grid);
        for (int i = 1; i < 16; ++i)
        {
            const float x = plot.getX() + plot.getWidth() * (float) i / 16.0f;
            g.setColour (i % 4 == 0 ? Palette::gridStrong : Palette::grid);
            g.drawVerticalLine ((int) x, plot.getY(), plot.getBottom());
        }
        g.setColour (Palette::grid);
        for (int i = 1; i < 4; ++i)
        {
            const float y = plot.getY() + plot.getHeight() * (float) i / 4.0f;
            g.drawHorizontalLine ((int) y, plot.getX(), plot.getRight());
        }

        // Curve path
        juce::Path path;
        constexpr int steps = 256;
        for (int i = 0; i <= steps; ++i)
        {
            const float t = (float) i / (float) steps;
            const auto p = toScreen (t, curve.evaluate (t), plot);
            if (i == 0) path.startNewSubPath (p);
            else        path.lineTo (p);
        }

        // Filled area under curve
        juce::Path fill (path);
        fill.lineTo (plot.getRight(), plot.getBottom());
        fill.lineTo (plot.getX(), plot.getBottom());
        fill.closeSubPath();
        g.setGradientFill (juce::ColourGradient (Palette::accent.withAlpha (0.28f),
                                                 plot.getX(), plot.getY(),
                                                 Palette::accent.withAlpha (0.04f),
                                                 plot.getX(), plot.getBottom(), false));
        g.fillPath (fill);

        g.setColour (Palette::accent);
        g.strokePath (path, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

        // Tension handles (segment midpoints)
        const auto& pts = curve.getPoints();
        for (int i = 0; i < (int) pts.size() - 1; ++i)
        {
            const float mx = (pts[(size_t) i].x + pts[(size_t) i + 1].x) * 0.5f;
            const auto hp = toScreen (mx, curve.evaluate (mx), plot);
            const bool hot = (hoverSegment == i && hoverPoint < 0) || dragSegment == i;
            g.setColour (hot ? Palette::text : Palette::accentDim);
            g.fillEllipse (juce::Rectangle<float> (hot ? 9.0f : 6.0f, hot ? 9.0f : 6.0f)
                               .withCentre (hp));
        }

        // Points
        for (int i = 0; i < (int) pts.size(); ++i)
        {
            const auto sp = toScreen (pts[(size_t) i].x, pts[(size_t) i].y, plot);
            const bool hot = hoverPoint == i || dragPoint == i;
            const float r = hot ? 7.0f : 5.0f;
            if (hot)
            {
                g.setColour (Palette::accent.withAlpha (0.30f));
                g.fillEllipse (juce::Rectangle<float> (r * 3.2f, r * 3.2f).withCentre (sp));
            }
            g.setColour (Palette::panel);
            g.fillEllipse (juce::Rectangle<float> (r * 2.0f, r * 2.0f).withCentre (sp));
            g.setColour (hot ? Palette::text : Palette::accent);
            g.drawEllipse (juce::Rectangle<float> (r * 2.0f, r * 2.0f).withCentre (sp), 2.0f);
        }

        // Playhead + gain dot
        if (playheadActive)
        {
            const float px = plot.getX() + playheadPhase * plot.getWidth();
            g.setColour (Palette::playhead.withAlpha (0.10f));
            g.fillRect (juce::Rectangle<float> (px - 3.0f, plot.getY(), 6.0f, plot.getHeight()));
            g.setColour (Palette::playhead.withAlpha (0.75f));
            g.fillRect (juce::Rectangle<float> (px - 0.75f, plot.getY(), 1.5f, plot.getHeight()));

            const auto dot = toScreen (playheadPhase, juce::jlimit (0.0f, 1.0f, playheadGain), plot);
            g.setColour (Palette::playhead.withAlpha (0.35f));
            g.fillEllipse (juce::Rectangle<float> (16.0f, 16.0f).withCentre (dot));
            g.setColour (juce::Colours::white);
            g.fillEllipse (juce::Rectangle<float> (6.0f, 6.0f).withCentre (dot));
        }
    }

    //==========================================================================
    void mouseMove (const juce::MouseEvent& e) override
    {
        const int oldPoint = hoverPoint, oldSeg = hoverSegment;
        hoverPoint   = findPointAt (e.position);
        hoverSegment = hoverPoint < 0 ? findHandleAt (e.position) : -1;
        setMouseCursor (hoverPoint >= 0 || hoverSegment >= 0
                            ? juce::MouseCursor::PointingHandCursor
                            : juce::MouseCursor::NormalCursor);
        if (oldPoint != hoverPoint || oldSeg != hoverSegment)
            repaint();
    }

    void mouseExit (const juce::MouseEvent&) override
    {
        hoverPoint = hoverSegment = -1;
        repaint();
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        dragPoint = findPointAt (e.position);
        dragSegment = -1;

        if (dragPoint >= 0)
        {
            // Delete on right-click or alt-click (not endpoints)
            if (e.mods.isPopupMenu() || e.mods.isAltDown())
            {
                deletePoint (dragPoint);
                dragPoint = -1;
            }
            return;
        }

        dragSegment = findHandleAt (e.position);
        if (dragSegment >= 0)
        {
            dragStartTension = curve.getPoints()[(size_t) dragSegment].tension;
            dragStartY = e.position.y;
        }
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        const auto plot = getPlotArea();

        if (dragPoint >= 0)
        {
            float nx = (e.position.x - plot.getX()) / plot.getWidth();
            float ny = 1.0f - (e.position.y - plot.getY()) / plot.getHeight();
            if (e.mods.isShiftDown())
            {
                nx = std::round (nx * 16.0f) / 16.0f;
                ny = std::round (ny * 8.0f)  / 8.0f;
            }
            curve.movePoint (dragPoint, nx, ny);
            notifyChange();
        }
        else if (dragSegment >= 0)
        {
            const auto& pts = curve.getPoints();
            const auto& a = pts[(size_t) dragSegment];
            const auto& b = pts[(size_t) dragSegment + 1];
            // Dragging down bends toward "stay low"; flip if segment descends.
            const float direction = (b.y >= a.y) ? 1.0f : -1.0f;
            const float delta = (e.position.y - dragStartY) / (plot.getHeight() * 0.5f);
            curve.setTension (dragSegment, dragStartTension + direction * delta);
            notifyChange();
        }
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        dragPoint = dragSegment = -1;
        repaint();
    }

    void mouseDoubleClick (const juce::MouseEvent& e) override
    {
        const int hit = findPointAt (e.position);
        if (hit >= 0)
        {
            deletePoint (hit);
            return;
        }

        const auto plot = getPlotArea();
        const float nx = (e.position.x - plot.getX()) / plot.getWidth();
        const float ny = 1.0f - (e.position.y - plot.getY()) / plot.getHeight();
        dragPoint = curve.addPoint (nx, ny);
        notifyChange();
    }

private:
    juce::Rectangle<float> getPlotArea() const
    {
        return getLocalBounds().toFloat().reduced (14.0f, 12.0f);
    }

    juce::Point<float> toScreen (float x, float y, const juce::Rectangle<float>& plot) const
    {
        return { plot.getX() + x * plot.getWidth(),
                 plot.getY() + (1.0f - y) * plot.getHeight() };
    }

    int findPointAt (juce::Point<float> pos) const
    {
        const auto plot = getPlotArea();
        const auto& pts = curve.getPoints();
        for (int i = 0; i < (int) pts.size(); ++i)
            if (toScreen (pts[(size_t) i].x, pts[(size_t) i].y, plot).getDistanceFrom (pos) < 11.0f)
                return i;
        return -1;
    }

    int findHandleAt (juce::Point<float> pos) const
    {
        const auto plot = getPlotArea();
        const auto& pts = curve.getPoints();
        for (int i = 0; i < (int) pts.size() - 1; ++i)
        {
            const float mx = (pts[(size_t) i].x + pts[(size_t) i + 1].x) * 0.5f;
            if (toScreen (mx, curve.evaluate (mx), plot).getDistanceFrom (pos) < 9.0f)
                return i;
        }
        return -1;
    }

    void deletePoint (int index)
    {
        if (index > 0 && index < curve.getNumPoints() - 1)
        {
            curve.removePoint (index);
            hoverPoint = -1;
            notifyChange();
        }
    }

    void notifyChange()
    {
        repaint();
        if (onCurveChanged)
            onCurveChanged();
    }

    Curve& curve;
    int hoverPoint = -1, hoverSegment = -1;
    int dragPoint = -1, dragSegment = -1;
    float dragStartTension = 0.0f, dragStartY = 0.0f;
    float playheadPhase = 0.0f, playheadGain = 1.0f;
    bool playheadActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopeEditor)
};
