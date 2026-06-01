// Apps/DemoApp/UltraCanvasGaugeExamples.cpp
// Comprehensive examples of gauge element modes using UltraCanvas layout managers
// Version: 2.3.0
// Last Modified: 2026-05-26
// Author: UltraCanvas Framework
// V2.3.0 changelog: Removed Speed sub-dial config (V2.6.1 dark renderer omits it).
// V2.2.0 changelog: Speedometer demo tuned for the V2.6.0 dark automotive look —
//   set value to 109, dropped colored range arcs (renderer now uses graduated
//   tick colors), sub-dial relabeled MPH and recentred.

#include "UltraCanvasDemo.h"
#include "Plugins/Diagrams/UltraCanvasGaugeDiagramElement.h"
#include "UltraCanvasContainer.h"
#include "UltraCanvasLabel.h"
#include "UltraCanvasSlider.h"
#include "UltraCanvasButton.h"
#include "UltraCanvasTabbedContainer.h"
#include "UltraCanvasBoxLayout.h"
#include "UltraCanvasGridLayout.h"
#include "UltraCanvasApplication.h"
#include <sstream>
#include <iomanip>

namespace UltraCanvas {

// =============================================================================
// LAYOUT CONSTANTS (V2.1)
// =============================================================================

namespace {
    constexpr long kCardW = 272;
    constexpr long kCardH = 374;
    constexpr long kSliderH = 22;
    constexpr long kValueLabelH = 20;
    constexpr long kCardPadding = 10;
}

// =============================================================================
// HELPER: Create gauge + slider + label card
// =============================================================================

static std::shared_ptr<UltraCanvasContainer> CreateGaugeCard(
    const std::string& id,
    long w, long h,
    std::shared_ptr<UltraCanvasGaugeDiagramElement> gauge,
    float sliderMin, float sliderMax, float sliderInit,
    const std::string& valueSuffix,
    int decimals = 0,
    float sliderStep = -1.0f) {

    auto card = std::make_shared<UltraCanvasContainer>(id, 0, 0, w, h);
    card->SetBackgroundColor(Color(255, 255, 255, 255));
    card->SetBorders(1.0f, Color(218, 219, 228, 255));
    card->SetPadding(kCardPadding);

    auto layout = CreateVBoxLayout(card.get());
    layout->SetSpacing(8);

    // Gauge takes the upper portion (most of the height)
    long gaugeW = w - 2 * kCardPadding;
    long gaugeH = h - 2 * kCardPadding - kSliderH - kValueLabelH - 16;  // 16 = spacing
    auto gaugeWrap = std::make_shared<UltraCanvasContainer>(id + "_GW", 0, 0, gaugeW, gaugeH);
    auto gaugeLayout = CreateVBoxLayout(gaugeWrap.get());
    gauge->SetBounds(Rect2Di(0, 0, static_cast<int>(gaugeW), static_cast<int>(gaugeH)));
    gaugeLayout->AddUIElement(gauge, 1)->SetCrossAlignment(LayoutAlignment::Fill);
    layout->AddUIElement(gaugeWrap, 1)->SetCrossAlignment(LayoutAlignment::Fill);

    // Slider for interactive control
    auto slider = std::make_shared<UltraCanvasSlider>(id + "_Sl", 0, 0, gaugeW, kSliderH);
    slider->SetOrientation(SliderOrientation::Horizontal);
    slider->SetRange(sliderMin, sliderMax);
    slider->SetValue(sliderInit);
    if (sliderStep >= 0.0f) slider->SetStep(sliderStep);
    layout->AddUIElement(slider, 0)->SetCrossAlignment(LayoutAlignment::Fill);

    // Value display label below slider
    auto valueLabel = std::make_shared<UltraCanvasLabel>(id + "_V", 0, 0, gaugeW, kValueLabelH);
    valueLabel->SetAlignment(TextAlignment::Center);
    valueLabel->SetFontSize(11);
    valueLabel->SetTextColor(Color(100, 100, 115, 255));
    std::ostringstream initVal;
    initVal << std::fixed << std::setprecision(decimals) << sliderInit << " " << valueSuffix;
    valueLabel->SetText(initVal.str());
    layout->AddUIElement(valueLabel, 0)->SetCrossAlignment(LayoutAlignment::Fill);

    auto gaugePtr = gauge.get();
    auto labelPtr = valueLabel.get();
    slider->onValueChanged = [gaugePtr, labelPtr, valueSuffix, decimals](float val) {
        gaugePtr->SetValue(static_cast<double>(val));
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(decimals) << val << " " << valueSuffix;
        labelPtr->SetText(oss.str());
    };

    gauge->SetValue(static_cast<double>(sliderInit));

    return card;
}

// =============================================================================
// TAB 1: ANALOG GAUGES
// =============================================================================

static std::shared_ptr<UltraCanvasContainer> BuildAnalogTab(long w, long h) {
    auto tab = std::make_shared<UltraCanvasContainer>("AnalogTab", 0, 0, w, h);
    tab->SetBackgroundColor(Color(240, 241, 248, 255));
    tab->SetPadding(12);

    auto layout = CreateVBoxLayout(tab.get());
    layout->SetSpacing(10);

    auto title = std::make_shared<UltraCanvasLabel>("AnaTitle", 0, 0, w - 24, 28);
    title->SetText("Analog Gauges - Speedometer, Semicircular, Compass, Clock, Stopwatch");
    title->SetFontSize(16);
    title->SetFontWeight(FontWeight::Bold);
    title->SetTextColor(Color(50, 50, 75, 255));
    layout->AddUIElement(title, 0)->SetCrossAlignment(LayoutAlignment::Fill);

    auto gridContainer = std::make_shared<UltraCanvasContainer>("AnaGrid", 0, 0, w - 24, h - 60);
    auto grid = CreateGridLayout(gridContainer.get(), 2, 3);
    grid->SetSpacing(12);

    // --- Speedometer with RPM sub-dial ---
    auto speedo = CreateGaugeDiagramElement("speedo", 0, 0, kCardW, kCardH);
    speedo->SetMode(GaugeMode::Speedometer);
    speedo->SetTitle("Speed");
    speedo->SetUnit("km/h");
    speedo->SetMaxValue(240.0);
    speedo->SetMajorTickCount(6);
    speedo->SetValue(109.0);  // V2.2: non-zero so the orange needle + LCD read like the photo
    // V2.3: sub-dial removed for the dark Speedometer (V2.6.1 renderer omits it).

    auto speedoCard = CreateGaugeCard("speedo_c", kCardW, kCardH, speedo, 0.0f, 240.0f, 0.0f, "km/h");
    grid->AddUIElement(speedoCard, 0, 0);

    // --- Semicircular Sales Assessment ---
    auto semi = CreateGaugeDiagramElement("semi", 0, 0, kCardW, kCardH);
    semi->SetMode(GaugeMode::Semicircular);
    semi->SetTitle("Sales Assessment");
    semi->SetUnit("k$");
    semi->SetMinValue(0.0);
    semi->SetMaxValue(100.0);
    semi->SetMajorTickCount(5);
    semi->AddRange(GaugeRangeSegment(0.0, 50.0, Color(100, 180, 255, 255), ""));
    semi->AddRange(GaugeRangeSegment(50.0, 100.0, Color(80, 200, 120, 255), ""));

    auto semiCard = CreateGaugeCard("semi_c", kCardW, kCardH, semi, 0.0f, 100.0f, 73.0f, "k$");
    grid->AddUIElement(semiCard, 0, 1);

    auto compass = CreateGaugeDiagramElement("compass", 0, 0, kCardW, kCardH);
    compass->SetMode(GaugeMode::Compass);
    compass->SetTitle("Wind Direction");
    compass->SetMaxValue(360.0);
    compass->SetNeedleStyle(GaugeNeedleStyle::Thin);

    GaugeSubDial wind;
    wind.enabled = true;
    wind.mode = GaugeMode::Semicircular;
    wind.title = "Wind";
    wind.minValue = 0.0;
    wind.maxValue = 25.0;
    wind.currentValue = 12.0;
    wind.unit = "m/s";
    compass->SetSubDial(wind);

    auto compassCard = CreateGaugeCard("compass_c", kCardW, kCardH, compass, 0.0f, 360.0f, 120.0f, "\xC2\xB0");
    grid->AddUIElement(compassCard, 0, 2);

    // --- Analog Clock ---
    auto clock = CreateGaugeDiagramElement("clock", 0, 0, kCardW, kCardH);
    clock->SetMode(GaugeMode::AnalogClock);
    clock->SetTitle("Clock");

    auto clockCard = std::make_shared<UltraCanvasContainer>("clock_c", 0, 0, kCardW, kCardH);
    clockCard->SetBackgroundColor(Color(255, 255, 255, 255));
    clockCard->SetBorders(1.0f, Color(218, 219, 228, 255));
    clockCard->SetPadding(kCardPadding);
    auto clockLayout = CreateVBoxLayout(clockCard.get());
    long clockGaugeH = kCardH - 2 * kCardPadding - kValueLabelH - 8;
    clock->SetBounds(Rect2Di(0, 0, static_cast<int>(kCardW - 2 * kCardPadding), static_cast<int>(clockGaugeH)));
    clockLayout->AddUIElement(clock, 1)->SetCrossAlignment(LayoutAlignment::Fill);
    auto clockInfo = std::make_shared<UltraCanvasLabel>("clockI", 0, 0, kCardW - 2 * kCardPadding, kValueLabelH);
    clockInfo->SetText("Live system time");
    clockInfo->SetAlignment(TextAlignment::Center);
    clockInfo->SetFontSize(11);
    clockInfo->SetTextColor(Color(100, 100, 115, 255));
    clockLayout->AddUIElement(clockInfo, 0)->SetCrossAlignment(LayoutAlignment::Fill);
    grid->AddUIElement(clockCard, 1, 0);

    // --- Stopwatch ---
    auto stopwatch = CreateGaugeDiagramElement("sw", 0, 0, kCardW, kCardH);
    stopwatch->SetMode(GaugeMode::Stopwatch);
    stopwatch->SetTitle("Stopwatch");
    stopwatch->SetMaxValue(60.0);
    stopwatch->SetMajorTickCount(12);

    GaugeSubDial mins;
    mins.enabled = true;
    mins.mode = GaugeMode::Semicircular;
    mins.title = "min";
    mins.minValue = 0.0;
    mins.maxValue = 30.0;
    mins.currentValue = 0.0;
    mins.unit = "m";
    stopwatch->SetSubDial(mins);

    auto swCard = std::make_shared<UltraCanvasContainer>("sw_c", 0, 0, kCardW, kCardH);
    swCard->SetBackgroundColor(Color(255, 255, 255, 255));
    swCard->SetBorders(1.0f, Color(218, 219, 228, 255));
    swCard->SetPadding(kCardPadding);
    {
        auto swLayout = CreateVBoxLayout(swCard.get());
        swLayout->SetSpacing(6);
        long swGaugeW = kCardW - 2 * kCardPadding;
        long swGaugeH = kCardH - 2 * kCardPadding - kValueLabelH - 6 - 28 - 6;
        auto swGaugeWrap = std::make_shared<UltraCanvasContainer>("sw_GW", 0, 0, swGaugeW, swGaugeH);
        auto swGaugeLayout = CreateVBoxLayout(swGaugeWrap.get());
        stopwatch->SetBounds(Rect2Di(0, 0, static_cast<int>(swGaugeW), static_cast<int>(swGaugeH)));
        swGaugeLayout->AddUIElement(stopwatch, 1)->SetCrossAlignment(LayoutAlignment::Fill);
        swLayout->AddUIElement(swGaugeWrap, 1)->SetCrossAlignment(LayoutAlignment::Fill);

        // Start/Stop/Reset buttons
        auto swBtnContainer = std::make_shared<UltraCanvasContainer>("sw_Btns", 0, 0, swGaugeW, 28l);
        auto swBtnLayout = CreateHBoxLayout(swBtnContainer.get());
        swBtnLayout->SetSpacing(6);
        auto swStart = std::make_shared<UltraCanvasButton>("sw_Start", 0, 0, 80, 26, "Start");
        auto swStop = std::make_shared<UltraCanvasButton>("sw_Stop", 0, 0, 80, 26, "Stop");
        auto swReset = std::make_shared<UltraCanvasButton>("sw_Reset", 0, 0, 80, 26, "Reset");
        swBtnLayout->AddUIElement(swStart, 1);
        swBtnLayout->AddUIElement(swStop, 1);
        swBtnLayout->AddUIElement(swReset, 1);
        swLayout->AddUIElement(swBtnContainer, 0)->SetCrossAlignment(LayoutAlignment::Fill);

        // Value label
        auto swValue = std::make_shared<UltraCanvasLabel>("sw_V", 0, 0, swGaugeW, kValueLabelH);
        swValue->SetAlignment(TextAlignment::Center);
        swValue->SetFontSize(11);
        swValue->SetTextColor(Color(100, 100, 115, 255));
        swValue->SetText("14.90 s");
        swLayout->AddUIElement(swValue, 0)->SetCrossAlignment(LayoutAlignment::Fill);

        auto swGaugePtr = stopwatch.get();
        auto swLabelPtr = swValue.get();
        swStart->SetOnClick([swGaugePtr] { swGaugePtr->StopwatchStart(); });
        swStop->SetOnClick([swGaugePtr] { swGaugePtr->StopwatchStop(); });
        swReset->SetOnClick([swGaugePtr] {
            swGaugePtr->StopwatchReset();
        });
        stopwatch->onGaugeValueChange = [swLabelPtr](double val) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << val << " s";
            swLabelPtr->SetText(oss.str());
        };
        stopwatch->SetValue(14.9);
    }
    grid->AddUIElement(swCard, 1, 1);

    // --- Quadrant Power ---
    auto quad = CreateGaugeDiagramElement("quad", 0, 0, kCardW, kCardH);
    quad->SetMode(GaugeMode::Quadrant);
    quad->SetTitle("Power");
    quad->SetUnit("kW");
    quad->SetMaxValue(50.0);
    quad->SetMajorTickCount(5);
    quad->AddRange(GaugeRangeSegment(0.0, 30.0, Color(0, 200, 140, 255)));
    quad->AddRange(GaugeRangeSegment(30.0, 50.0, Color(255, 160, 60, 255)));

    auto quadCard = CreateGaugeCard("quad_c", kCardW, kCardH, quad, 0.0f, 50.0f, 0.0f, "kW", 1);
    grid->AddUIElement(quadCard, 1, 2);

    layout->AddUIElement(gridContainer, 1)->SetCrossAlignment(LayoutAlignment::Fill);
    return tab;
}

// =============================================================================
// TAB 2: PROGRESS & LINEAR GAUGES
// =============================================================================

static std::shared_ptr<UltraCanvasContainer> BuildProgressTab(long w, long h) {
    auto tab = std::make_shared<UltraCanvasContainer>("ProgTab", 0, 0, w, h);
    tab->SetBackgroundColor(Color(240, 241, 248, 255));
    tab->SetPadding(12);

    auto layout = CreateVBoxLayout(tab.get());
    layout->SetSpacing(10);

    auto title = std::make_shared<UltraCanvasLabel>("ProgTitle", 0, 0, w - 24, 28);
    title->SetText("Progress & Linear Gauges - Bar, LED, Segmented, Multi-Pointer, With Arrow");
    title->SetFontSize(16);
    title->SetFontWeight(FontWeight::Bold);
    title->SetTextColor(Color(50, 50, 75, 255));
    layout->AddUIElement(title, 0)->SetCrossAlignment(LayoutAlignment::Fill);

    auto gridContainer = std::make_shared<UltraCanvasContainer>("ProgGrid", 0, 0, w - 24, h - 60);
    auto grid = CreateGridLayout(gridContainer.get(), 2, 3);
    grid->SetSpacing(12);

    // --- Linear Bar download progress ---
    auto bar = CreateGaugeDiagramElement("bar", 0, 0, kCardW, kCardH);
    bar->SetMode(GaugeMode::LinearBar);
    bar->SetTitle("Download Progress");
    bar->SetUnit("%");
    bar->SetGaugeColor(Color(0, 140, 255, 255));
    auto barCard = CreateGaugeCard("bar_c", kCardW, kCardH, bar, 0.0f, 100.0f, 65.0f, "%");
    grid->AddUIElement(barCard, 0, 0);

    // --- LED Segmented VU Meter ---
    auto led = CreateGaugeDiagramElement("led", 0, 0, kCardW, kCardH);
    led->SetMode(GaugeMode::LinearLED);
    led->SetOrientation(GaugeOrientation::Vertical);
    led->SetTitle("VU Meter");
    led->SetSegmentCount(20);
    led->SetUnit("%");
    led->SetSegmentCount(16);
    led->SetGaugeColor(Color(0, 200, 255, 255));
    led->AddRange(GaugeRangeSegment(0.0, 80.0, Color(0, 200, 255, 255)));
    led->AddRange(GaugeRangeSegment(80.0, 100.0, Color(255, 100, 80, 255)));
    auto ledCard = CreateGaugeCard("led_c", kCardW, kCardH, led, 0.0f, 100.0f, 65.0f, "%");
    grid->AddUIElement(ledCard, 0, 1);

    // --- Segmented Brick Revenue ---
    auto brick = CreateGaugeDiagramElement("brick", 0, 0, kCardW, kCardH);
    brick->SetMode(GaugeMode::LinearSegmented);
    brick->SetTitle("Revenue");
    brick->SetUnit("M$");
    brick->SetMaxValue(20.0);
    brick->SetSegmentCount(20);
    brick->AddRange(GaugeRangeSegment(0.0, 10.0, Color(255, 140, 80, 255)));
    brick->AddRange(GaugeRangeSegment(10.0, 20.0, Color(0, 180, 220, 255)));
    brick->SetDecimalPlaces(2);
    auto brickCard = CreateGaugeCard("brick_c", kCardW, kCardH, brick, 0.0f, 20.0f, 12.45f, "M$", 2);
    grid->AddUIElement(brickCard, 0, 2);

    // --- Multi-pointer Recipe Layers ---
    auto multi = CreateGaugeDiagramElement("multi", 0, 0, kCardW, kCardH);
    multi->SetMode(GaugeMode::LinearMultiPointer);
    multi->SetOrientation(GaugeOrientation::Vertical);
    multi->SetTitle("Recipe Layers");
    multi->SetUnit("ml");
    multi->SetMaxValue(2000.0);
    multi->SetGaugeColor(Color(0, 160, 255, 255));
    multi->AddExternalPointer(GaugeExternalPointer(1500.0, Color(0, 160, 255, 255), "Base"));
    multi->AddExternalPointer(GaugeExternalPointer(1660.0, Color(255, 180, 60, 255), "Fill"));
    multi->AddExternalPointer(GaugeExternalPointer(2000.0, Color(180, 180, 190, 255), "Max"));
    auto multiCard = CreateGaugeCard("multi_c", kCardW, kCardH, multi, 0.0f, 2000.0f, 1500.0f, "ml");
    grid->AddUIElement(multiCard, 1, 0);

    // --- Linear With Arrow Glucose ---
    auto arrow = CreateGaugeDiagramElement("arrow", 0, 0, kCardW, kCardH);
    arrow->SetMode(GaugeMode::LinearWithArrow);
    arrow->SetOrientation(GaugeOrientation::Vertical);
    arrow->SetTitle("Glucose Level");
    arrow->SetUnit("mmol/l");
    arrow->SetMaxValue(15.0);
    arrow->SetDecimalPlaces(1);
    arrow->AddRange(GaugeRangeSegment(0.0, 5.6, Color(80, 200, 120, 255), "Normal"));
    arrow->AddRange(GaugeRangeSegment(5.6, 7.0, Color(255, 190, 60, 255), "Borderline"));
    arrow->AddRange(GaugeRangeSegment(7.0, 15.0, Color(255, 100, 80, 255), "Elevated"));
    auto arrowCard = CreateGaugeCard("arrow_c", kCardW, kCardH, arrow, 0.0f, 15.0f, 5.7f, "mmol/l", 1);
    grid->AddUIElement(arrowCard, 1, 1);

    // --- Linear Scale Radio Tuner ---
    auto scale = CreateGaugeDiagramElement("scale", 0, 0, kCardW, kCardH);
    scale->SetMode(GaugeMode::LinearScale);
    scale->SetTitle("Radio Tuner");
    scale->SetUnit("MHz");
    scale->SetMinValue(180.0);
    scale->SetMaxValue(970.0);
    scale->AddThreshold(GaugeThreshold(180.0, Color(180, 180, 190, 255), "Galaxy"));
    scale->AddThreshold(GaugeThreshold(571.0, Color(180, 180, 190, 255), "Dukes"));
    scale->AddThreshold(GaugeThreshold(780.0, Color(180, 180, 190, 255), "Frasier"));
    scale->AddThreshold(GaugeThreshold(970.0, Color(180, 180, 190, 255), "Simpsons"));
    auto scaleCard = CreateGaugeCard("scale_c", kCardW, kCardH, scale, 180.0f, 970.0f, 571.0f, "MHz");
    grid->AddUIElement(scaleCard, 1, 2);

    layout->AddUIElement(gridContainer, 1)->SetCrossAlignment(LayoutAlignment::Fill);
    return tab;
}

// =============================================================================
// TAB 3: SPECIALIZED GAUGES
// =============================================================================

static std::shared_ptr<UltraCanvasContainer> BuildSpecializedTab(long w, long h) {
    auto tab = std::make_shared<UltraCanvasContainer>("SpecTab", 0, 0, w, h);
    tab->SetBackgroundColor(Color(240, 241, 248, 255));
    tab->SetPadding(12);

    auto layout = CreateVBoxLayout(tab.get());
    layout->SetSpacing(10);

    auto title = std::make_shared<UltraCanvasLabel>("SpecTitle", 0, 0, w - 24, 28);
    title->SetText("Specialized Gauges - Battery, Thermometer, Cylinder, Ring, Digital");
    title->SetFontSize(16);
    title->SetFontWeight(FontWeight::Bold);
    title->SetTextColor(Color(50, 50, 75, 255));
    layout->AddUIElement(title, 0)->SetCrossAlignment(LayoutAlignment::Fill);

    auto gridContainer = std::make_shared<UltraCanvasContainer>("SpecGrid", 0, 0, w - 24, h - 60);
    auto grid = CreateGridLayout(gridContainer.get(), 2, 3);
    grid->SetSpacing(12);

    // --- Battery Bar pointer ---
    auto bat1 = CreateGaugeDiagramElement("bat1", 0, 0, kCardW, kCardH);
    bat1->SetMode(GaugeMode::Battery);
    bat1->SetTitle("Battery (Bar)");
    bat1->SetBatteryStyle(GaugeBatteryStyle::BarPointer);
    bat1->SetShowBolt(true);
    auto bat1Card = CreateGaugeCard("bat1_c", kCardW, kCardH, bat1, 0.0f, 100.0f, 75.0f, "%");
    grid->AddUIElement(bat1Card, 0, 0);

    // --- Battery LED ---
    auto bat2 = CreateGaugeDiagramElement("bat2", 0, 0, kCardW, kCardH);
    bat2->SetMode(GaugeMode::Battery);
    bat2->SetTitle("Battery (LED)");
    bat2->SetBatteryStyle(GaugeBatteryStyle::LedPointer);
    bat2->SetSegmentCount(10);
    auto bat2Card = CreateGaugeCard("bat2_c", kCardW, kCardH, bat2, 0.0f, 100.0f, 100.0f, "%");
    grid->AddUIElement(bat2Card, 0, 1);

    // --- Thermometer ---
    auto thermo = CreateGaugeDiagramElement("thermo", 0, 0, kCardW, kCardH);
    thermo->SetMode(GaugeMode::Thermometer);
    thermo->SetTitle("Temperature");
    thermo->SetUnit("C");
    thermo->SetMinValue(-25.0);
    thermo->SetMaxValue(25.0);
    thermo->SetGaugeColor(Color(0, 180, 255, 255));
    thermo->SetDecimalPlaces(0);
    auto thermoCard = CreateGaugeCard("thermo_c", kCardW, kCardH, thermo, -25.0f, 25.0f, 12.0f, "C");
    grid->AddUIElement(thermoCard, 0, 2);

    // --- Cylinder Water ---
    auto cyl = CreateGaugeDiagramElement("cyl", 0, 0, kCardW, kCardH);
    cyl->SetMode(GaugeMode::Cylinder);
    cyl->SetTitle("Water");
    cyl->SetUnit("ml");
    cyl->SetMaxValue(1000.0);
    cyl->SetGaugeColor(Color(0, 200, 200, 255));
    auto cylCard = CreateGaugeCard("cyl_c", kCardW, kCardH, cyl, 0.0f, 1000.0f, 1000.0f, "ml");
    grid->AddUIElement(cylCard, 1, 0);

    // --- Circular Ring ---
    auto ring = CreateGaugeDiagramElement("ring", 0, 0, kCardW, kCardH);
    ring->SetMode(GaugeMode::CircularRing);
    ring->SetTitle("Completion");
    ring->SetUnit("%");
    ring->SetGaugeColor(Color(0, 200, 140, 255));
    auto ringCard = CreateGaugeCard("ring_c", kCardW, kCardH, ring, 0.0f, 100.0f, 75.0f, "%");
    grid->AddUIElement(ringCard, 1, 1);

    // --- Digital LED ---
    auto digital = CreateGaugeDiagramElement("digital", 0, 0, kCardW, kCardH);
    digital->SetMode(GaugeMode::Digital);
    digital->SetTitle("LED Display");
    digital->SetUnit("Hz");
    digital->SetMaxValue(9999.0);
    digital->SetDecimalPlaces(1);
    digital->SetShowGlow(true);
    digital->AddRange(GaugeRangeSegment(0.0, 5000.0, Color(0, 180, 255, 255)));
    digital->AddRange(GaugeRangeSegment(5000.0, 8000.0, Color(255, 200, 60, 255)));
    digital->AddRange(GaugeRangeSegment(8000.0, 9999.0, Color(255, 80, 80, 255)));
    auto digCard = CreateGaugeCard("dig_c", kCardW, kCardH, digital, 0.0f, 9999.0f, 1234.5f, "Hz", 1);
    grid->AddUIElement(digCard, 1, 2);

    layout->AddUIElement(gridContainer, 1)->SetCrossAlignment(LayoutAlignment::Fill);
    return tab;
}

// =============================================================================
// MAIN ENTRY POINT
// =============================================================================

std::shared_ptr<UltraCanvasUIElement> UltraCanvasDemoApplication::CreateGaugeExamples() {
    // V2.1: Larger overall size to fit 3 columns of 320-wide cards comfortably
    auto tabs = std::make_shared<UltraCanvasTabbedContainer>(
        "GaugeTabs", 0, 0, 920, 860);
    tabs->SetTabStyle(TabStyle::Rounded);
    tabs->SetTabHeight(32);

    tabs->AddTab("Analog", BuildAnalogTab(920, 810));
    tabs->AddTab("Progress & Linear", BuildProgressTab(920, 810));
    tabs->AddTab("Specialized", BuildSpecializedTab(920, 810));
    tabs->SetActiveTab(0);

    return tabs;
}

} // namespace UltraCanvas