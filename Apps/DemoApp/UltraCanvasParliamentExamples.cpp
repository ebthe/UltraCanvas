// Apps/DemoApp/UltraCanvasParliamentExamples.cpp
// Parliament diagram examples for UltraCanvas Demo App
// Version: 2.0.1
// Last Modified: 2026-05-11
//
// =============================================================================
// CHANGELOG
// =============================================================================
// v2.0.1 (2026-05-11) — Layout tuning to match v2.0.1 geometry fixes
//   * Increased Layouts tab cards from 310x310 to 310x360 to give the
//     semicircle arc enough vertical room after subtracting the title bar.
//   * Increased Presets / Render Modes tab cards from 470x320 to 470x360
//     for the same reason (those use legend Bottom which also subtracts).
//   * Increased root tab height from 700 to 760.
// v2.0.0 (2026-05-11) — Expanded demos for refactored parliament diagram
//   * Tabbed layout: "Layouts", "Presets", "Render Modes & Interaction".
//   * Layouts tab now shows the geometrically-correct angular-sector
//     distribution (parties as contiguous wedges, not dispersed).
//   * Presets tab demonstrates ApplyPreset() for Bundestag, European
//     Parliament, Westminster, and US Congress visualizations.
//   * Render Modes tab compares Dots vs PieDonut side-by-side and demonstrates
//     interactive party selection / highlight.
//   * All cards use SetTitle/SetSubtitle on the diagram itself instead of
//     external UltraCanvasLabel children for cleaner alignment.
// v1.0.0 (2026-04-12) — Initial release
// =============================================================================

#include "UltraCanvasDemo.h"
#include "UltraCanvasContainer.h"
#include "UltraCanvasTabbedContainer.h"
#include "Plugins/Diagrams/UltraCanvasParliamentDiagram.h"
#include <vector>
#include <memory>

namespace UltraCanvas {

// =============================================================================
// PARTY DATA HELPERS
// =============================================================================

    // Generic multi-party parliament (9 parties, ~365 seats)
    static std::vector<ParliamentParty> MakeDemoParties() {
        return {
            ParliamentParty("Social Democratic Party",  "SDP",   120, Color(200,  40,  40, 255), true),
            ParliamentParty("Green Alliance",           "Green",  45, Color( 60, 180,  60, 255), true),
            ParliamentParty("Liberal Party",            "Lib",    32, Color(240, 200,  20, 255), true),
            ParliamentParty("Conservative Union",       "CU",     98, Color( 40,  60, 180, 255), false),
            ParliamentParty("People's Front",           "PF",     28, Color(180,  40, 180, 255), false),
            ParliamentParty("Freedom Movement",         "FM",     15, Color( 20, 160, 200, 255), false),
            ParliamentParty("Agrarian League",          "AL",     12, Color( 80, 160,  40, 255), false),
            ParliamentParty("Justice Party",            "JP",      8, Color(200, 120,  40, 255), false),
            ParliamentParty("Independents",             "Ind",     7, Color(160, 160, 160, 255), false)
        };
    }

    // US-style two-party (House of Representatives shape, 435 seats)
    static std::vector<ParliamentParty> MakeUSCongressParties() {
        return {
            ParliamentParty("Democratic Party",  "Dem", 212, Color( 40,  80, 200, 255), false),
            ParliamentParty("Republican Party",  "Rep", 223, Color(200,  40,  40, 255), true)
        };
    }

    // German Bundestag-style (2025 composition, 630 seats)
    static std::vector<ParliamentParty> MakeBundestagParties() {
        return {
            ParliamentParty("Die Linke",        "Linke", 64,  Color(220, 130, 170, 255), false),
            ParliamentParty("SPD",              "SPD",   120, Color(200,  40,  40, 255), true),
            ParliamentParty("Buendnis 90/Gruene","Gruene",85,  Color( 60, 160,  60, 255), false),
            ParliamentParty("CDU/CSU",          "CDU",   208, Color( 30,  30,  30, 255), true),
            ParliamentParty("AfD",              "AfD",   152, Color(100, 160, 210, 255), false),
            ParliamentParty("SSW",              "SSW",   1,   Color( 60, 120, 200, 255), false)
        };
    }

    // European Parliament-style (720 seats, EP 2024-2029)
    static std::vector<ParliamentParty> MakeEUParliamentParties() {
        return {
            ParliamentParty("The Left",                    "Left",    46,  Color(140,  20,  20, 255), false),
            ParliamentParty("Progressive Alliance S&D",    "S&D",     136, Color(220,  50,  50, 255), false),
            ParliamentParty("Greens/EFA",                  "Greens",  53,  Color( 40, 140,  60, 255), false),
            ParliamentParty("Renew Europe",                "Renew",   77,  Color(255, 200,  40, 255), false),
            ParliamentParty("European People's Party",     "EPP",     188, Color( 80, 130, 200, 255), true),
            ParliamentParty("European Conservatives",      "ECR",     78,  Color( 30,  80, 150, 255), false),
            ParliamentParty("Patriots for Europe",         "PfE",     84,  Color( 20,  50, 110, 255), false),
            ParliamentParty("Europe of Sovereign Nations", "ESN",     25,  Color( 60,  70,  90, 255), false),
            ParliamentParty("Non-attached",                "NI",      33,  Color(160, 160, 160, 255), false)
        };
    }

    // UK House of Commons-style (650 seats, post-2024 election)
    static std::vector<ParliamentParty> MakeWestminsterParties() {
        return {
            ParliamentParty("Labour",               "LAB", 411, Color(220,  50,  50, 255), true),
            ParliamentParty("Liberal Democrats",    "LD",  72,  Color(250, 170,  40, 255), false),
            ParliamentParty("Scottish National",    "SNP", 9,   Color(255, 220,  60, 255), false),
            ParliamentParty("Green Party",          "GRN", 4,   Color( 80, 180,  80, 255), false),
            ParliamentParty("Others",               "Oth", 33,  Color(160, 160, 160, 255), false),
            ParliamentParty("Conservative",         "CON", 121, Color( 40,  90, 180, 255), false)
        };
    }

// =============================================================================
// CARD BUILDER
// =============================================================================

    // Builds a single card with a parliament diagram inside. The title and
    // subtitle live on the diagram itself (v2.0.0 API) — no external label
    // child required.
    static std::shared_ptr<UltraCanvasContainer> CreateParliamentCard(
        const std::string& id, int uidBase,
        int x, int y, int w, int h,
        const std::string& title,
        const std::string& subtitle,
        ParliamentLayout layout,
        const std::vector<ParliamentParty>& parties,
        bool showCenterLabel = true)
    {
        auto card = std::make_shared<UltraCanvasContainer>(
            id + "_card", x, y, w, h);
        card->SetBackgroundColor(Color(255, 255, 255, 240));
        card->SetBorders(1, Color(210, 210, 215, 255));

        auto diagram = std::make_shared<UltraCanvasParliamentDiagram>(
            id, 8, 8, w - 16, h - 16);

        diagram->SetParties(parties);
        diagram->SetLayout(layout);
        diagram->SetTitle(title);
        if (!subtitle.empty()) diagram->SetSubtitle(subtitle);
        diagram->SetDotRadius(4.0f);
        diagram->SetDotSpacing(1.2f);
        diagram->SetShowCenterLabel(showCenterLabel);
        diagram->SetShowMajorityLine(false);

        card->AddChild(diagram);
        return card;
    }

// =============================================================================
// TAB 1 — LAYOUTS
// =============================================================================
//
//   Shows the five layout types with the new correct angular-sector algorithm.
//   Compare against v1.0.0: same data, but parties now occupy contiguous
//   wedges instead of being dispersed across radii.
//
    static std::shared_ptr<UltraCanvasContainer> BuildLayoutsTab(int W, int H) {
        auto root = std::make_shared<UltraCanvasContainer>(
            "parliamentLayoutsTab", 0, 0, W, H);
        root->SetBackgroundColor(Color(238, 240, 244, 255));

        const int cardW = 360;
        const int cardH = 400;
        const int gap   = 14;
        int yPos = gap;

        // Row 1: Three arc-based layouts
        auto semicircle = CreateParliamentCard(
            "layoutSemi", 4401, gap, yPos, cardW, cardH,
            "Semicircle", "180\xC2\xB0 arc - France, Germany, USA",
            ParliamentLayout::Semicircle, MakeDemoParties());

        auto horseshoe = CreateParliamentCard(
            "layoutHorse", 4411, gap*2 + cardW, yPos, cardW, cardH,
            "Horseshoe", "270\xC2\xB0 arc - Australia, Ireland",
            ParliamentLayout::Horseshoe, MakeDemoParties());

        auto circle = CreateParliamentCard(
            "layoutCirc", 4421, gap*3 + cardW*2, yPos, cardW, cardH,
            "Circle", "360\xC2\xB0 - Slovenia, Lesotho",
            ParliamentLayout::Circle, MakeDemoParties());

        root->AddChild(semicircle);
        root->AddChild(horseshoe);
        root->AddChild(circle);

        // Row 2: Non-arc + US two-party
        yPos += cardH + gap;

        auto opposing = CreateParliamentCard(
            "layoutOppose", 4431, gap, yPos, cardW, cardH,
            "Opposing Benches", "Westminster-style",
            ParliamentLayout::OpposingBenches, MakeDemoParties(), false);

        auto classroom = CreateParliamentCard(
            "layoutClass", 4441, gap*2 + cardW, yPos, cardW, cardH,
            "Classroom", "Rectangular rows - China, Cuba",
            ParliamentLayout::Classroom, MakeDemoParties(), false);

        auto twoParty = CreateParliamentCard(
            "layoutUSA", 4451, gap*3 + cardW*2, yPos, cardW, cardH,
            "Two-party System", "USA House of Representatives",
            ParliamentLayout::Semicircle, MakeUSCongressParties());

        root->AddChild(opposing);
        root->AddChild(classroom);
        root->AddChild(twoParty);

        return root;
    }

// =============================================================================
// TAB 2 — PRESETS
// =============================================================================
//
//   Demonstrates ApplyPreset() with real-world parliament configurations.
//   Each card uses authentic party data + colors for that parliament.
//
    static std::shared_ptr<UltraCanvasContainer> BuildPresetsTab(int W, int H) {
        auto root = std::make_shared<UltraCanvasContainer>(
            "parliamentPresetsTab", 0, 0, W, H);
        root->SetBackgroundColor(Color(238, 240, 244, 255));

        const int cardW = 520;
        const int cardH = 400;
        const int gap   = 14;

        // Bundestag — top-left
        {
            auto card = std::make_shared<UltraCanvasContainer>(
                "presetBundestagCard",
                gap, gap, cardW, cardH);
            card->SetBackgroundColor(Color(255, 255, 255, 240));
            card->SetBorders(1, Color(210, 210, 215, 255));

            auto diagram = std::make_shared<UltraCanvasParliamentDiagram>(
                "presetBundestag", 8, 8, cardW - 16, cardH - 16);
            diagram->SetParties(MakeBundestagParties());
            diagram->ApplyPreset(ParliamentPreset::Bundestag);
            diagram->SetTitle("German Bundestag");
            diagram->SetSubtitle("Federal election 2025 - 630 seats");
            diagram->SetCenterLabelSubtext("Sitze");

            card->AddChild(diagram);
            root->AddChild(card);
        }

        // European Parliament — top-right
        {
            auto card = std::make_shared<UltraCanvasContainer>(
                "presetEUCard",
                gap*2 + cardW, gap, cardW, cardH);
            card->SetBackgroundColor(Color(255, 255, 255, 240));
            card->SetBorders(1, Color(210, 210, 215, 255));

            auto diagram = std::make_shared<UltraCanvasParliamentDiagram>(
                "presetEU", 8, 8, cardW - 16, cardH - 16);
            diagram->SetParties(MakeEUParliamentParties());
            diagram->ApplyPreset(ParliamentPreset::EuropeanParliament);
            diagram->SetTitle("European Parliament");
            diagram->SetSubtitle("2024-2029 - 720 seats");

            card->AddChild(diagram);
            root->AddChild(card);
        }

        // Westminster — bottom-left
        {
            auto card = std::make_shared<UltraCanvasContainer>(
                "presetWestminsterCard",
                gap, gap*2 + cardH, cardW, cardH);
            card->SetBackgroundColor(Color(255, 255, 255, 240));
            card->SetBorders(1, Color(210, 210, 215, 255));

            auto diagram = std::make_shared<UltraCanvasParliamentDiagram>(
                "presetWestminster", 8, 8, cardW - 16, cardH - 16);
            diagram->SetParties(MakeWestminsterParties());
            diagram->ApplyPreset(ParliamentPreset::Westminster);
            diagram->SetTitle("UK House of Commons");
            diagram->SetSubtitle("2024 General Election - 650 seats");

            card->AddChild(diagram);
            root->AddChild(card);
        }

        // US Congress — bottom-right
        {
            auto card = std::make_shared<UltraCanvasContainer>(
                "presetUSCard",
                gap*2 + cardW, gap*2 + cardH, cardW, cardH);
            card->SetBackgroundColor(Color(255, 255, 255, 240));
            card->SetBorders(1, Color(210, 210, 215, 255));

            auto diagram = std::make_shared<UltraCanvasParliamentDiagram>(
                "presetUS", 8, 8, cardW - 16, cardH - 16);
            diagram->SetParties(MakeUSCongressParties());
            diagram->ApplyPreset(ParliamentPreset::USCongress);
            diagram->SetTitle("US House of Representatives");
            diagram->SetSubtitle("119th Congress - 435 seats");

            card->AddChild(diagram);
            root->AddChild(card);
        }

        return root;
    }

// =============================================================================
// TAB 3 — RENDER MODES & INTERACTION
// =============================================================================
//
//   Three demos:
//     1. Dots vs PieDonut side-by-side (same Bundestag data, different mode)
//     2. Interactive parliament - click a party to highlight, others dim
//     3. Per-party seat labels enabled on the PieDonut card
//
    static std::shared_ptr<UltraCanvasContainer> BuildRenderModesTab(int W, int H) {
        auto root = std::make_shared<UltraCanvasContainer>(
            "parliamentRenderTab", 0, 0, W, H);
        root->SetBackgroundColor(Color(238, 240, 244, 255));

        const int gap = 14;

        // Row 1: Dots vs PieDonut (same data, different render mode)
        const int rowOneCardW = 520;
        const int rowOneCardH = 400;

        // Dots mode
        {
            auto card = std::make_shared<UltraCanvasContainer>(
                "renderDotsCard",
                gap, gap, rowOneCardW, rowOneCardH);
            card->SetBackgroundColor(Color(255, 255, 255, 240));
            card->SetBorders(1, Color(210, 210, 215, 255));

            auto diagram = std::make_shared<UltraCanvasParliamentDiagram>(
                "renderDots", 8, 8,
                rowOneCardW - 16, rowOneCardH - 16);
            diagram->SetParties(MakeBundestagParties());
            diagram->SetLayout(ParliamentLayout::Semicircle);
            diagram->SetRenderMode(ParliamentRenderMode::Dots);
            diagram->SetTitle("Dots mode");
            diagram->SetSubtitle("Classic seat-per-dot - high information density");
            diagram->SetShowCenterLabel(true);
            diagram->SetCenterLabelSubtext("Sitze");
            diagram->SetLegendPosition(ParliamentLegendPosition::Bottom);

            card->AddChild(diagram);
            root->AddChild(card);
        }

        // PieDonut mode
        {
            auto card = std::make_shared<UltraCanvasContainer>(
                "renderPieCard",
                gap*2 + rowOneCardW, gap, rowOneCardW, rowOneCardH);
            card->SetBackgroundColor(Color(255, 255, 255, 240));
            card->SetBorders(1, Color(210, 210, 215, 255));

            auto diagram = std::make_shared<UltraCanvasParliamentDiagram>(
                "renderPie", 8, 8,
                rowOneCardW - 16, rowOneCardH - 16);
            diagram->SetParties(MakeBundestagParties());
            diagram->SetLayout(ParliamentLayout::Semicircle);
            diagram->SetRenderMode(ParliamentRenderMode::PieDonut);
            diagram->SetInnerRadiusFraction(0.32f);
            diagram->SetOuterRadiusFraction(0.92f);
            diagram->SetTitle("Pie/Donut mode");
            diagram->SetSubtitle("Infographic style - solid filled sectors");
            diagram->SetShowCenterLabel(true);
            diagram->SetCenterLabelSubtext("Sitze");
            diagram->SetShowPartySeatLabels(true);
            diagram->SetLegendPosition(ParliamentLegendPosition::Bottom);

            card->AddChild(diagram);
            root->AddChild(card);
        }

        // Row 2: Interactive parliament with highlight, full-width
        const int interactiveW = rowOneCardW*2 + gap;
        const int interactiveH = 400;
        const int row2Y = gap*2 + rowOneCardH;

        {
            auto card = std::make_shared<UltraCanvasContainer>(
                "renderInteractiveCard",
                gap, row2Y, interactiveW, interactiveH);
            card->SetBackgroundColor(Color(255, 255, 255, 240));
            card->SetBorders(1, Color(210, 210, 215, 255));

            auto diagram = std::make_shared<UltraCanvasParliamentDiagram>(
                "renderInteractive", 8, 8,
                interactiveW - 16, interactiveH - 16);
            diagram->SetParties(MakeEUParliamentParties());
            diagram->SetLayout(ParliamentLayout::Semicircle);
            diagram->SetRenderMode(ParliamentRenderMode::Dots);
            diagram->SetInnerRadiusFraction(0.36f);
            diagram->SetOuterRadiusFraction(0.95f);
            diagram->SetDotRadius(3.5f);
            diagram->SetDotSpacing(1.1f);
            diagram->SetTitle("Interactive parliament - click a party to focus");
            diagram->SetSubtitle("European Parliament 2024-2029 - hover and click anywhere");
            diagram->SetShowCenterLabel(true);
            diagram->SetCenterLabelSubtext("Seats");
            diagram->SetShowMajorityLine(false);
            diagram->SetLegendPosition(ParliamentLegendPosition::Right);
            diagram->SetLegendColumns(1);
            diagram->SetDimmedAlphaFactor(0.15f);
            diagram->SetHoverBrightenFactor(1.12f);

            // Wire up hover / select callbacks for status feedback.
            // These hooks could route to a detail panel or status bar; left
            // as silent no-ops in the demo to avoid polluting the UI.
            diagram->onPartyHover = [](const ParliamentParty& p) {
                (void)p;
            };

            diagram->onPartySelect = [](const ParliamentParty& p) {
                (void)p;
            };

            diagram->onPartyDeselect = []() {
            };

            card->AddChild(diagram);
            root->AddChild(card);
        }

        return root;
    }

// =============================================================================
// MAIN ENTRY POINT
// =============================================================================
//
//   Returns a TabbedContainer wrapping the three demo tabs.
//
    std::shared_ptr<UltraCanvasUIElement>
    UltraCanvasDemoApplication::CreateParliamentDiagramExamples() {
        const int W = 980;
        const int H = 800;  // v2.0.1: was 700, fits 2 rows of 360-tall cards + tab bar

        auto rootTabs = std::make_shared<UltraCanvasTabbedContainer>(
            "parliamentTabs", 0, 0, W, H);

        auto layoutsTab = BuildLayoutsTab(W, H - 40);
        auto presetsTab = BuildPresetsTab(W, H - 40);
        auto renderTab  = BuildRenderModesTab(W, H - 40);

        rootTabs->AddTab("Layouts",                    layoutsTab);
        rootTabs->AddTab("Real-world Presets",         presetsTab);
        rootTabs->AddTab("Render Modes & Interaction", renderTab);

        return rootTabs;
    }

} // namespace UltraCanvas