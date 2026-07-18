#pragma once

#include "Curve.h"

//==============================================================================
// Factory presets — classic four-on-the-floor ducking shapes.
//==============================================================================
namespace FactoryPresets
{
    struct Preset
    {
        const char* name;
        std::vector<CurvePoint> points;
    };

    inline const std::vector<Preset>& getAll()
    {
        static const std::vector<Preset> presets = {
            { "Classic",
              { { 0.0f,  0.0f,  0.55f }, { 0.70f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } } },

            { "Soft",
              { { 0.0f,  0.42f, 0.35f }, { 0.60f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } } },

            { "Tight",
              { { 0.0f,  0.0f,  0.45f }, { 0.32f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } } },

            { "Long Pump",
              { { 0.0f,  0.0f,  0.70f }, { 0.96f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } } },

            { "Wobble",
              { { 0.0f,  0.0f,  0.50f }, { 0.45f, 1.0f, 0.0f }, { 0.50f, 0.30f, 0.50f },
                { 0.92f, 1.0f,  0.0f  }, { 1.0f,  1.0f, 0.0f } } },
        };
        return presets;
    }

    inline int count() { return (int) getAll().size(); }
}
