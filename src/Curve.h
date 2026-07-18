#pragma once

#include <juce_core/juce_core.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <vector>

//==============================================================================
// Envelope curve model: sorted points in [0,1]x[0,1]. Each point carries the
// tension of the segment that starts at it (-1 .. +1, 0 = linear).
// y == 1 means unity gain, y == 0 means silence.
//==============================================================================
struct CurvePoint
{
    float x = 0.0f, y = 1.0f, tension = 0.0f;
};

class Curve
{
public:
    Curve() { resetToDefault(); }

    void resetToDefault()
    {
        points = { { 0.0f, 0.0f, 0.55f }, { 0.7f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } };
    }

    const std::vector<CurvePoint>& getPoints() const noexcept { return points; }
    int getNumPoints() const noexcept { return (int) points.size(); }

    // Shapes a normalised 0..1 ramp. tension > 0 stays low then rises late
    // (classic sidechain recovery), tension < 0 rises early then flattens.
    static float bend (float t, float tension) noexcept
    {
        if (std::abs (tension) < 1.0e-4f)
            return t;
        const float e = std::pow (2.0f, std::abs (tension) * 4.0f);
        return tension > 0.0f ? std::pow (t, e)
                              : 1.0f - std::pow (1.0f - t, e);
    }

    float evaluate (float x) const noexcept
    {
        x = juce::jlimit (0.0f, 1.0f, x);

        for (size_t i = 1; i < points.size(); ++i)
        {
            if (x <= points[i].x)
            {
                const auto& a = points[i - 1];
                const auto& b = points[i];
                const float span = b.x - a.x;
                if (span <= 1.0e-6f)
                    return b.y;
                const float t = bend ((x - a.x) / span, a.tension);
                return a.y + (b.y - a.y) * t;
            }
        }
        return points.back().y;
    }

    //==========================================================================
    // Editing. Endpoints (x = 0 and x = 1) can move only vertically.
    //==========================================================================
    int addPoint (float x, float y)
    {
        x = juce::jlimit (0.001f, 0.999f, x);
        y = juce::jlimit (0.0f, 1.0f, y);
        auto it = std::upper_bound (points.begin(), points.end(), x,
                                    [] (float v, const CurvePoint& p) { return v < p.x; });
        const int index = (int) std::distance (points.begin(), it);
        points.insert (it, { x, y, 0.0f });
        return index;
    }

    void removePoint (int index)
    {
        if (index > 0 && index < (int) points.size() - 1)
            points.erase (points.begin() + index);
    }

    void movePoint (int index, float x, float y)
    {
        if (index < 0 || index >= (int) points.size())
            return;

        auto& p = points[(size_t) index];
        p.y = juce::jlimit (0.0f, 1.0f, y);

        if (index == 0)                             p.x = 0.0f;
        else if (index == (int) points.size() - 1)  p.x = 1.0f;
        else
        {
            const float lo = points[(size_t) index - 1].x + 0.002f;
            const float hi = points[(size_t) index + 1].x - 0.002f;
            p.x = juce::jlimit (lo, hi, x);
        }
    }

    void setTension (int segmentIndex, float tension)
    {
        if (segmentIndex >= 0 && segmentIndex < (int) points.size() - 1)
            points[(size_t) segmentIndex].tension = juce::jlimit (-1.0f, 1.0f, tension);
    }

    void setPoints (std::vector<CurvePoint> newPoints)
    {
        if (newPoints.size() < 2)
            return;
        std::sort (newPoints.begin(), newPoints.end(),
                   [] (const CurvePoint& a, const CurvePoint& b) { return a.x < b.x; });
        newPoints.front().x = 0.0f;
        newPoints.back().x  = 1.0f;
        points = std::move (newPoints);
    }

    //==========================================================================
    // Serialisation
    //==========================================================================
    juce::ValueTree toValueTree() const
    {
        juce::ValueTree tree ("CURVE");
        for (const auto& p : points)
        {
            juce::ValueTree pt ("PT");
            pt.setProperty ("x", p.x, nullptr);
            pt.setProperty ("y", p.y, nullptr);
            pt.setProperty ("t", p.tension, nullptr);
            tree.appendChild (pt, nullptr);
        }
        return tree;
    }

    void fromValueTree (const juce::ValueTree& tree)
    {
        if (! tree.hasType ("CURVE") || tree.getNumChildren() < 2)
            return;
        std::vector<CurvePoint> loaded;
        for (const auto& pt : tree)
            loaded.push_back ({ (float) pt.getProperty ("x", 0.0),
                                (float) pt.getProperty ("y", 1.0),
                                (float) pt.getProperty ("t", 0.0) });
        setPoints (std::move (loaded));
    }

private:
    std::vector<CurvePoint> points;
};

//==============================================================================
// Lock-free double-buffered lookup table. The message thread renders the curve
// into the inactive table and swaps; the audio thread only ever reads.
//==============================================================================
class CurveLUT
{
public:
    static constexpr int size = 2048;

    void render (const Curve& curve)
    {
        auto& table = tables[(size_t) (1 - active.load (std::memory_order_acquire))];
        for (int i = 0; i <= size; ++i)
            table[(size_t) i] = curve.evaluate ((float) i / (float) size);
        active.store (1 - active.load (std::memory_order_acquire), std::memory_order_release);
    }

    float get (float phase) const noexcept
    {
        const auto& table = tables[(size_t) active.load (std::memory_order_acquire)];
        const float pos = juce::jlimit (0.0f, 1.0f, phase) * (float) size;
        const int i = juce::jmin ((int) pos, size - 1);
        const float frac = pos - (float) i;
        return table[(size_t) i] + (table[(size_t) i + 1] - table[(size_t) i]) * frac;
    }

private:
    std::array<std::array<float, size + 1>, 2> tables { };
    std::atomic<int> active { 0 };
};
