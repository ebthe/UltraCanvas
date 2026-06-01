// include/Plugins/Diagrams/UltraCanvasParliamentDiagram.h
// Parliament / hemicycle seat distribution diagram
// Version: 2.0.17
// Last Modified: 2026-05-15
// Author: UltraCanvas Framework
//
// =============================================================================
// CHANGELOG
// =============================================================================
// v2.0.17 (2026-05-15) — Auto-fit bounds + full-block centering
//   * Auto-fit constants retuned: factor 0.55→0.45, min 12→8, max 28→22,
//     sub ratio 0.375→0.33, sub floor 6→6.
//   * NEW: proportional scaling when block exceeds 85% of rInner — small-
//     rInner presets (US Congress) scale down automatically.
//   * Semicircle: dynamic bias (0.50 + headroom/2rInner, capped 0.70) +
//     3px optical lift for visual weight of Bold number.
//   * Circle: full-block centering at cyArc.
//   * Horseshoe: unchanged.
// v2.0.16 (2026-05-11) — Auto-fit center label
//   * Added bool autoFitCenterLabel (default: true). When enabled,
//     centerLabelFontSize is computed dynamically each render as
//     rInner * kAutoFitFactor (0.55), so the label scales naturally
//     with dome size: large domes -> large fonts, small domes -> small.
//     Eliminates the need to manually tune fontSize per layout/preset.
//   * SetCenterLabelFontSize() now also disables auto-fit (user opt-out
//     when they want explicit control).
//   * SetCenterLabelAutoFit(bool) and IsCenterLabelAutoFit() accessors
//     added.
//   * Subtitle fontSize also scales when auto-fit is active, preserving
//     the visual ratio (subFontSize = mainFontSize * 0.375).
//   * NOTE: requires 'make clean && make' (header changes inline defaults).
// v2.0.15 (2026-05-11) — Final size + subtitle alignment polish
//   * Default centerLabelFontSize 24 -> 20. For small dome cards (~310px
//     wide), 24pt was still visually dominant relative to the dome size
//     (~45% of dome width). 20pt brings the main label to ~37% of dome
//     width, better infographic proportion.
//   * kTextWidthRatio recalibrated 0.38 -> 0.43. Pixel-precise measurement
//     of "Total Seats" rendered at 9pt showed real width ~45px while
//     v2.0.14's estimate was 37.6px → ~3.5px drift to the right. The
//     new 0.43 brings drift to <1px for all subtitle strings ("Sitze",
//     "Seats", "Total Seats", "Total").
//   * IMPORTANT: requires 'make clean && make' (header changes inline
//     defaults).
// v2.0.14 (2026-05-11) — Center label proportions
//   * Default centerLabelFontSize reduced 28.0 -> 24.0. The 28pt size was
//     calibrated when domes were larger; with v2.0.10's kDomeFillFactor
//     shrinking the dome by 8%, the 28pt label was visually too dominant.
//     24pt restores balance between the seat count and the dome itself.
//   * Semicircle label position factor: cyArc - rInner * 0.55f ->
//     cyArc - rInner * 0.65f. Moves the center label block ~10% higher
//     within the donut hole, away from the bottom edge where it was
//     visually crowded against the seat rows.
//   * NOTE: changing the inline default in the header requires
//     'make clean && make' to take effect — incremental builds may keep
//     the old value baked into already-compiled translation units.
// v2.0.13 (2026-05-11) — Empirically-calibrated Bold digit ratio
//   * Default centerLabelFontSize reduced 28.0 -> 24.0. The 28pt size was
//     calibrated when domes were larger; with v2.0.10's kDomeFillFactor
//     shrinking the dome by 8%, the 28pt label was visually too dominant.
//     24pt restores balance between the seat count and the dome itself.
//   * Semicircle label position factor: cyArc - rInner * 0.55f ->
//     cyArc - rInner * 0.65f. Moves the center label block ~10% higher
//     within the donut hole, away from the bottom edge where it was
//     visually crowded against the seat rows.
// v2.0.13 (2026-05-11) — Empirically-calibrated Bold digit ratio
//   * kDigitBoldWidthRatio recalibrated 0.62 -> 0.78 based on pixel-level
//     measurement of the actual rendered text. Previous 0.62 was
//     theoretical estimate; the real Cairo/Pango Bold sans-serif glyphs
//     render at ratio ~0.78 of fontSize at 28pt. Now "365"/"630"/"720"/
//     "435" sit visually centered on the dome geometric center.
//   * Diagnostic std::cerr logs from v2.0.13-DIAG removed.
// v2.0.13-DIAG (2026-05-11) — DIAGNOSTIC BUILD (now reverted)
// v2.0.12 (2026-05-11) — Bold digit width ratio
//   * Introduced kDigitBoldWidthRatio = 0.62f, separate from
//     kDigitWidthRatio (0.52f) for Regular weight. Bold sans-serif
//     glyphs are ~18% wider than Regular, so using a single ratio for
//     both weights caused the Bold center label ("630", "720") and Bold
//     party seat labels ("208", "152", "85") to drift ~6-8px to the
//     right of the geometric center.
//   * Applied to: DrawCenterLabel main (Bold), DrawPartySeatLabels
//     (Bold). DrawMajorityLine threshold ("316") stays on the Regular
//     ratio since it renders with FontWeight::Normal.
// v2.0.11 (2026-05-11) — Calibrated centering + uniform legend slots
//   * kTextWidthRatio recalibrated 0.46 -> 0.38. Empirical measurement
//     showed the previous value overestimated alphabetic text widths by
//     ~20% in the active backends (Cairo/Pango on Linux, where the demo
//     is running). The drift was proportional to text length: "Sitze"
//     showed ~3px drift, "Total Seats" showed ~12px drift. The new 0.38
//     places text within ~1px of true center across short and long
//     captions.
//   * Introduced kLegendTextWidthRatio = 0.55f as a separate ratio for
//     legend slot width reservation. Legend layout needs CONSERVATIVE
//     width estimates (slightly larger than real) so items don't visually
//     touch each other. The previous 0.46 was too tight, causing items
//     like "Linke 64" and "SPD 120" to appear glued together.
//   * Legend reverts to uniform slot width (max of all items) instead of
//     per-item measured widths. Result: swatches across all rows align
//     to the same X column positions — the professional infographic look
//     used by EU Parliament / Bundestag publications. The whole block
//     is still centered as a unit within the legend area.
// v2.0.10 (2026-05-11) — Breathing room via dome fill factor
//   * Introduced kDomeFillFactor = 0.92f, applied uniformly to maxR in
//     ComputeArcGeometry. The dome now occupies 92% of the geometric
//     maximum, giving 8% breathing room on every side. This is a
//     systemic fix replacing the v2.0.7-v2.0.9 series of micro-offsets
//     that tried to patch the visual "bottom-heavy" feel pixel by pixel.
//   * kCenterLabelLift removed (was 6px in v2.0.9). With the dome
//     slightly smaller, the center label naturally sits with adequate
//     space above the legend.
//   * kLegendTopPadding reduced from 10 to 8 px. The visual breathing
//     room now comes from the smaller dome, not from injected padding.
//   * Net effect on Bundestag card (470x360): dome shrinks from 60% to
//     ~52% of card height, with proportional space above the title bar
//     and below the legend.
// v2.0.9 (2026-05-11) — Vertical breathing room + relative offsets
//   * DrawLegend: added kLegendTopPadding (10px) before the first legend
//     row in Bottom/Top positions. Previously the legend hugged the dome
//     too tightly. ComputeDrawingArea reserves this extra padding inside
//     legendAreaSize so geometry stays consistent.
//   * DrawMajorityLine threshold label ("316") is now positioned at
//     cy - rOuter - 10px (relative to actual dome geometry), not via
//     fixed offsets from x2/y2. Scales correctly with any dome size.
//   * DrawCenterLabel: nudges the entire label block up by 6px so the
//     "630" / "720" sits a touch higher inside the donut opening,
//     creating clearer visual separation from the legend below.
//   * The 6px nudge is encoded as kCenterLabelLift constant for easy
//     tuning.
// v2.0.8 (2026-05-11) — Semicircle vertical balance fix
//   * ComputeArcGeometry Semicircle: replaced bottom-anchored cy with a
//     two-pass derivation. Pass 1 computes maxR from the width constraint
//     alone (dw / 2), independent of cy. Pass 2 derives cy as:
//
//         cy = dy + dh * 0.55f + rOuter * 0.45f
//
//     This places the dome's vertical center slightly above the drawing
//     area's midpoint (the 0.55 / 0.45 blend gives a subtle upward bias
//     so the dome "floats" naturally), instead of pinning the dome to
//     the bottom edge. When dh would otherwise clip the dome, a fallback
//     clamps cy down to (dy + dh - safety_margin).
//   * Eliminates the v2.0.7 regression where narrow cards
//     (Two-party System, Semicircle in Layouts tab) showed ~176px of
//     wasted vertical space above the dome.
//   * Horseshoe and Circle layouts unchanged.
// v2.0.7 (2026-05-11) — Semicircle space utilization
//   * ComputeArcGeometry for Semicircle now decouples the dome anchor from
//     the maximum radius. Previous v2.0.4 used cy = dy + dh × 0.88 AND
//     maxR = dh × 0.88, which double-applied the 0.88 factor and capped
//     the dome height at ~54% of the drawing area. Now:
//       cy   = dy + dh - kSemicircleBottomMargin
//       maxR = min(dw × 0.5, dh - kSemicircleBottomMargin - 2)
//     The anchor sits a fixed 4px above the drawing-area bottom, and the
//     radius is limited only by the actual usable space. Result: ~30%
//     larger dome on cards with bottom legend (Bundestag, US House, demo
//     Render Modes tab).
//   * Default legendRowHeight reduced from 18.0 to 16.0. The 2px savings
//     per legend row gives back 4px to the diagram on Bottom-legend cards
//     and tightens the legend look without compromising readability.
//   * Legend bottom padding inside ComputeDrawingArea trimmed from 12 to
//     8px (each row already includes vertical centering padding through
//     legendRowHeight).
// v2.0.6 (2026-05-11) — Centering precision + legend slot layout
//   * Split kCharWidthRatio into two constants:
//     - kDigitWidthRatio = 0.52f  (uniform digit glyphs: "365", "630")
//     - kTextWidthRatio  = 0.46f  (mixed-width letters: "Sitze", "Total Seats")
//     Subtitle drift to the left observed in v2.0.5 was caused by the 0.52
//     ratio overestimating alphabetic widths (i, l, t are much narrower than
//     digit average). The 0.46 ratio matches sans-serif text widths much
//     more closely.
//   * Legend layout: each slot is now sized to the EXACT width of its own
//     (swatch + label) pair, with a configurable fixed gap between slots.
//     Previously all slots shared the width of the widest label, which
//     caused short labels like "SSW 1" to leave large gaps and the block
//     to appear visually off-center. The block is still centered as a
//     whole within the legend area, but now hugs its content.
// v2.0.5 (2026-05-11) — Centering + repaint trigger
//   * OnEvent now calls RequestRedraw() after every hover/selection state
//     change so the visual update happens immediately instead of waiting
//     for the next paint cycle. Eliminates the noticeable lag when
//     clicking parties or hovering across them.
//   * Replaced reliance on TextAlignment::Center (ignored by Cairo/Pango
//     and Direct2D when DrawText receives a single Point2Df) with
//     explicit X centering: text width is estimated and the X position
//     shifted by -width/2. Affects DrawCenterLabel (main number +
//     subtitle), DrawPartySeatLabels (per-sector numbers), and
//     DrawMajorityLine (threshold label). All centered texts now align
//     vertically beneath the center of the dome.
// v2.0.4 (2026-05-11) — Selection performance + visual polish
//   * Performance: SelectParty() and hover state changes no longer
//     invalidate the geometry cache. Geometry depends on parties/layout/size
//     only, so selection just triggers a redraw via colors. Click-to-select
//     latency now matches the framework's redraw cost, not full re-layout.
//   * Selection dim now also applies to per-party seat labels (numbers like
//     208, 152 over each sector). Previously stayed bold/dark over dimmed
//     dots, breaking the visual hierarchy. Consistent uniform dim now.
//   * Center label subtitle ("Total Seats", "Sitze", "Seats") gets more
//     breathing room: 4px gap -> 10px gap, and default size 11pt -> 9pt.
//     Matches EU Parliament / Bundestag professional typography.
//   * Majority threshold label (the red "316") repositioned and sized down
//     to 9pt with proper top-edge offset, no longer overlapping the dome.
//   * Legend block centering: character-width estimator tightened from 0.58
//     to 0.52, which produced visually-centered legend blocks in
//     Bundestag, EU Parliament, US House.
// v2.0.3 (2026-05-11) — Visual refinements
//   * Per-party seat labels now adapt their font size to the sector's chord
//     length. Tiny sectors (e.g. SSW=1 in Bundestag) shrink to a minimum
//     legible size; wide sectors (e.g. CDU=208) use the full configured
//     size. Eliminates the "208 spilling out of the dome" artefact seen
//     in v2.0.2.
//   * Legend block is now horizontally centered within the legend area.
//     Column width measured against the widest party label, so all swatches
//     align on the same vertical axis. Matches Bundestag/EU Parliament
//     professional legend look.
// v2.0.2 (2026-05-11) — CRITICAL: Semicircle angle range fix
//   * Fixed root cause of "dots squashed at card bottom" in Semicircle and
//     Two-party System layouts: GetArcAngles() was returning [pi, 2*pi]
//     (lower half of the circle where sin <= 0), which with Y-down screen
//     mapping placed all dots BELOW the anchor instead of above. Corrected
//     to [pi, 0] (upper half, preserving left-to-right party ordering via
//     reversed iteration).
// v2.0.1 (2026-05-11) — Geometry & layout fixes
//   * Fixed title/subtitle overlap caused by DrawText interpreting Y as
//     top-edge: removed the `+ fontSize` offset in y position calculation.
//   * Fixed semicircle arc being clipped at the bottom of small cards: moved
//     dome anchor from 0.92f to 0.88f, giving the dome more vertical room.
//   * Fixed center label appearing outside the donut opening: position now
//     computed as cy_anchor - rInner * 0.5f for Semicircle (centered inside
//     the donut hole) and similar offsets for Horseshoe/Circle.
//   * Extracted ComputeArcGeometry() private helper to eliminate the 5x
//     duplicated geometry reconstruction in DrawPieDonut /
//     DrawPartySeatLabels / DrawMajorityLine / DrawCenterLabel /
//     HitTestSector. Single source of truth for cx, cy, rInner, rOuter.
//   * ComputeSeats() now also uses ComputeArcGeometry() rather than its own
//     inline copy of the same logic.
// v2.0.0 (2026-04-12) — Major geometric refactor
//   * Replaced incorrect row-first party assignment with correct angular-sector
//     distribution (hybrid d3-parliament algorithm). Each party now occupies a
//     contiguous angular wedge spanning all rows, matching real-world
//     visualizations (Bundestag, European Parliament, Westminster).
//   * Added ParliamentRenderMode { Dots, PieDonut } for infographic-style
//     solid-sector rendering alongside the classic dots mode.
//   * Added ParliamentLegendPosition + configurable legend rendering with
//     column control.
//   * Added ParliamentPreset { Bundestag, EuropeanParliament, Westminster,
//     USCongress } with ApplyPreset() one-call configuration.
//   * Added title/subtitle rendering above the diagram with professional
//     typography.
//   * Added per-party seat labels (SetShowPartySeatLabels) for inline labels
//     over each sector (Bundestag-style).
//   * Added visual highlight/select: non-selected parties dim to a configurable
//     alpha when a selection is active.
//   * Added onPartyHover callback (previously only onSeatHover existed).
//   * Added density compensation so inner-row dots aren't more cramped than
//     outer-row dots in arc layouts.
//   * Real configurable arcSpanDegrees (180, 270, 360, intermediate values)
//     with correct gap/symmetry handling for non-180 spans.
//   * V1 public API preserved — no breaking signature changes on existing
//     methods.
// v1.0.0 (2026-04-12) — Initial release
// =============================================================================

#pragma once

#include "UltraCanvasUIElement.h"
#include "UltraCanvasRenderContext.h"
#include "UltraCanvasCommonTypes.h"
#include "UltraCanvasEvent.h"
#include <vector>
#include <string>
#include <functional>
#include <cmath>

namespace UltraCanvas {

// =============================================================================
// PARLIAMENT LAYOUT TYPE
// =============================================================================

    enum class ParliamentLayout {
        Semicircle,      // 180° arc  — USA, EU, France, Germany
        Horseshoe,       // 270° arc  — Australia, New Zealand, Ireland
        Circle,          // 360° arc  — Slovenia, Lesotho
        OpposingBenches, // Two facing grids — UK Westminster
        Classroom        // Rectangular rows — China, Cuba
    };

// =============================================================================
// PARLIAMENT RENDER MODE  (v2.0.0)
// =============================================================================
//
//   Dots:     classic seat-as-dot rendering, one circle per representative
//   PieDonut: solid filled sectors (pie/donut chart style — see EU Parliament
//             projection visualizations). Only meaningful for arc-based
//             layouts; ignored for OpposingBenches and Classroom.
//
    enum class ParliamentRenderMode {
        Dots,
        PieDonut
    };

// =============================================================================
// PARLIAMENT LEGEND POSITION  (v2.0.0)
// =============================================================================

    enum class ParliamentLegendPosition {
        None,
        Bottom,
        Right,
        Left,
        Top
    };

// =============================================================================
// PARLIAMENT PRESET  (v2.0.0)
// =============================================================================
//
//   One-call configuration of layout + render mode + arc span + styling to
//   match well-known parliament visualizations. Sets defaults; can be
//   overridden by subsequent setter calls.
//
    enum class ParliamentPreset {
        Bundestag,           // Semicircle 180°, dots, center label
        EuropeanParliament,  // Semicircle 180°, dots, large center hole, legend right
        Westminster,         // OpposingBenches, no center label, legend bottom
        USCongress           // Semicircle 180°, dots, two-party styling
    };

// =============================================================================
// PARTY DATA STRUCTURE
// =============================================================================

    struct ParliamentParty {
        std::string Name;
        std::string ShortName;
        int         Seats = 0;
        Color       PartyColor;
        bool        IsGoverning = false;

        ParliamentParty() = default;
        ParliamentParty(const std::string& name, const std::string& shortName,
                        int seats, const Color& color, bool governing = false)
            : Name(name), ShortName(shortName), Seats(seats),
              PartyColor(color), IsGoverning(governing) {}
    };

// =============================================================================
// INTERNAL SEAT RECORD
// =============================================================================

    struct ParliamentSeat {
        float           X = 0.0f;
        float           Y = 0.0f;
        int             PartyIndex = -1;
        int             SeatInParty = 0;
    };

// =============================================================================
// INTERNAL ANGULAR SECTOR RECORD  (v2.0.0)
// =============================================================================
//
//   Result of phase 1 of the hybrid algorithm — angular range allocated to a
//   single party. Phase 2 packs that party's dots within [StartAngle, EndAngle]
//   across all rows. Also reused directly by PieDonut render mode.
//
    struct ParliamentSector {
        int   PartyIndex     = -1;
        float StartAngle     = 0.0f;   // radians
        float EndAngle       = 0.0f;   // radians
        int   AssignedSeats  = 0;
    };

// =============================================================================
// PARLIAMENT DIAGRAM ELEMENT
// =============================================================================

    class UltraCanvasParliamentDiagram : public UltraCanvasUIElement {
    private:
        // ── Data ──────────────────────────────────────────────────────────────
        std::vector<ParliamentParty>          parties;
        mutable std::vector<ParliamentSeat>   computedSeats;
        mutable std::vector<ParliamentSector> computedSectors;
        mutable int                           cachedTotalSeats = 0;
        mutable bool                          cacheValid = false;

        // ── Layout config ─────────────────────────────────────────────────────
        ParliamentLayout     layout          = ParliamentLayout::Semicircle;
        ParliamentRenderMode renderMode      = ParliamentRenderMode::Dots;
        int                  rows            = 0;
        float                arcSpanDegrees  = 180.0f;
        float                innerRadiusFrac = 0.30f;
        float                outerRadiusFrac = 0.85f;
        float                dotRadius       = 4.0f;
        float                dotSpacing      = 1.2f;

        // ── Title / subtitle  (v2.0.0) ────────────────────────────────────────
        std::string titleText        = "";
        std::string subtitleText     = "";
        Color       titleColor       = Color(20, 20, 20, 255);
        Color       subtitleColor    = Color(110, 110, 110, 255);
        float       titleFontSize    = 18.0f;
        float       subtitleFontSize = 12.0f;
        mutable float       titleAreaHeight  = 0.0f;  // computed at render time

        // ── Majority line ─────────────────────────────────────────────────────
        bool  showMajorityLine       = false;
        int   majorityThreshold      = 0;
        Color majorityLineColor      = Color(220, 50, 50, 220);
        float majorityLineWidth      = 1.5f;

        // ── Center label ──────────────────────────────────────────────────────
        bool        showCenterLabel        = false;
        std::string centerLabelText        = "";
        std::string centerLabelSubtext     = "Total Seats";
        Color       centerLabelColor       = Color(30,  30,  30,  255);
        Color       centerLabelSubColor    = Color(100, 100, 100, 255);
        float       centerLabelFontSize    = 20.0f;   // v2.0.15: was 24.0f; v2.0.17: auto-fit clamps+scales
        float       centerLabelSubFontSize = 9.0f;   // v2.0.4: was 11.0f
        bool        autoFitCenterLabel     = true;  // v2.0.16: scale font to rInner

        // ── Per-party seat labels  (v2.0.0) ───────────────────────────────────
        bool  showPartySeatLabels   = false;
        Color partyLabelColor       = Color(255, 255, 255, 255);
        Color partyLabelOutlineColor= Color(0,   0,   0,   180);
        float partyLabelFontSize    = 12.0f;

        // ── Dot styling ───────────────────────────────────────────────────────
        bool  showDotBorder        = false;
        Color dotBorderColor       = Color(255, 255, 255, 180);
        float dotBorderWidth       = 0.8f;

        // ── Highlight / select visual  (v2.0.0) ───────────────────────────────
        float dimmedAlphaFactor    = 0.18f;  // alpha multiplier for non-selected
        float hoverBrightenFactor  = 1.10f;  // multiplier on RGB for hovered

        // ── Legend  (v2.0.0) ──────────────────────────────────────────────────
        ParliamentLegendPosition legendPosition  = ParliamentLegendPosition::None;
        int                      legendColumns   = 0;     // 0 = auto
        float                    legendFontSize  = 11.0f;
        Color                    legendTextColor = Color(40, 40, 40, 255);
        float                    legendSwatchSize= 10.0f;
        float                    legendRowHeight = 16.0f;  // v2.0.7: was 18.0f
        mutable float                    legendAreaSize  = 0.0f;  // computed at render

        // ── Interaction state ─────────────────────────────────────────────────
        int   hoveredPartyIndex    = -1;
        int   hoveredSeatIndex     = -1;
        int   selectedPartyIndex   = -1;

        // ── Opposing benches grid config ──────────────────────────────────────
        int   opposingBenchCols    = 0;

        // ── Classroom config ──────────────────────────────────────────────────
        int   classroomCols        = 0;

    public:
        // ── Callbacks ─────────────────────────────────────────────────────────
        std::function<void(const ParliamentParty&, int seatIndex)> onSeatHover;
        std::function<void(const ParliamentParty&)>                onPartyHover;   // v2.0.0
        std::function<void(const ParliamentParty&)>                onPartySelect;
        std::function<void()>                                      onPartyDeselect;

    public:
        UltraCanvasParliamentDiagram(const std::string& id,
                                     long x, long y, long width, long height);
        ~UltraCanvasParliamentDiagram() override = default;
        bool AcceptsFocus() const override { return true; }

        // ── Data API ──────────────────────────────────────────────────────────
        void AddParty(const ParliamentParty& party);
        void SetParties(const std::vector<ParliamentParty>& partyList);
        void ClearParties();
        int  GetTotalSeats() const;
        const std::vector<ParliamentParty>& GetParties() const { return parties; }

        // ── Layout API ────────────────────────────────────────────────────────
        void SetLayout(ParliamentLayout newLayout);
        void SetRenderMode(ParliamentRenderMode mode);                  // v2.0.0
        ParliamentRenderMode GetRenderMode() const { return renderMode; } // v2.0.0
        void SetRows(int rowCount);
        void SetArcSpanDegrees(float deg);
        void SetInnerRadiusFraction(float f);
        void SetOuterRadiusFraction(float f);
        void SetDotRadius(float r);
        void SetDotSpacing(float s);

        // ── Title / subtitle API  (v2.0.0) ────────────────────────────────────
        void SetTitle(const std::string& text);
        void SetSubtitle(const std::string& text);
        void SetTitleColor(const Color& c);
        void SetSubtitleColor(const Color& c);
        void SetTitleFontSize(float size);
        void SetSubtitleFontSize(float size);

        // ── Majority line API ─────────────────────────────────────────────────
        void SetShowMajorityLine(bool show);
        void SetMajorityThreshold(int seats);
        void SetMajorityLineColor(const Color& c);
        void SetMajorityLineWidth(float w);

        // ── Center label API ──────────────────────────────────────────────────
        void SetShowCenterLabel(bool show);
        void SetCenterLabelText(const std::string& text);
        void SetCenterLabelSubtext(const std::string& subtext);
        void SetCenterLabelColor(const Color& c);
        void SetCenterLabelSubColor(const Color& c);
        void SetCenterLabelFontSize(float size);
        void SetCenterLabelSubFontSize(float size);
        void SetCenterLabelAutoFit(bool enable);     // v2.0.16
        bool IsCenterLabelAutoFit() const;           // v2.0.16

        // ── Per-party seat label API  (v2.0.0) ────────────────────────────────
        void SetShowPartySeatLabels(bool show);
        void SetPartyLabelColor(const Color& c);
        void SetPartyLabelOutlineColor(const Color& c);
        void SetPartyLabelFontSize(float size);

        // ── Dot styling API ───────────────────────────────────────────────────
        void SetShowDotBorder(bool show);
        void SetDotBorderColor(const Color& c);
        void SetDotBorderWidth(float w);

        // ── Highlight / select visual  (v2.0.0) ───────────────────────────────
        void SetDimmedAlphaFactor(float factor);
        void SetHoverBrightenFactor(float factor);

        // ── Legend API  (v2.0.0) ──────────────────────────────────────────────
        void SetLegendPosition(ParliamentLegendPosition pos);
        void SetLegendColumns(int cols);
        void SetLegendFontSize(float size);
        void SetLegendTextColor(const Color& c);
        void SetLegendSwatchSize(float size);
        void SetLegendRowHeight(float h);

        // ── Opposing benches / classroom tuning ───────────────────────────────
        void SetOpposingBenchCols(int cols);
        void SetClassroomCols(int cols);

        // ── Presets  (v2.0.0) ─────────────────────────────────────────────────
        //   Applies a coordinated set of layout + render mode + arc span +
        //   styling to match a well-known parliament visualization.
        void ApplyPreset(ParliamentPreset preset);

        // ── Interaction ───────────────────────────────────────────────────────
        void HighlightParty(int partyIndex);
        void SelectParty(int partyIndex);
        int  GetSelectedPartyIndex() const { return selectedPartyIndex; }
        int  GetHoveredPartyIndex()  const { return hoveredPartyIndex; }

        // ── UltraCanvasUIElement overrides ────────────────────────────────────
        void Render(IRenderContext* ctx, const Rect2Di& dirtyRect) override;
        bool OnEvent(const UCEvent& event) override;

    private:
        // ── Cache invalidation helper ─────────────────────────────────────────
        void  InvalidateCache();

        // ── Layout computation ────────────────────────────────────────────────
        void  ComputeSectors() const;
        void  ComputeSeats() const;
        void  ComputeArcLayout(float cx, float cy, float rInner, float rOuter,
                               float startAngle, float endAngle,
                               int   resolvedRows) const;
        void  ComputeOpposingLayout(float areaW, float areaH) const;
        void  ComputeClassroomLayout(float areaW, float areaH) const;

        int   AutoRows() const;
        float GetArcSpanForLayout() const;
        void  GetArcAngles(float& startAngle, float& endAngle) const;
        float GetCenterX() const;
        float GetCenterY() const;

        // ── Drawing area computation  (v2.0.0) ────────────────────────────────
        // Account for title bar at top and legend area at chosen side.
        void  ComputeDrawingArea(float& outX, float& outY,
                                 float& outW, float& outH) const;

        // ── Arc geometry computation  (v2.0.1) ────────────────────────────────
        // Single source of truth for arc center / radii used by all rendering
        // passes (DrawSeats, DrawPieDonut, DrawPartySeatLabels,
        // DrawMajorityLine, DrawCenterLabel, HitTestSector). Previously
        // duplicated 5+ times; consolidated here.
        void  ComputeArcGeometry(float& outCx, float& outCy,
                                 float& outRInner, float& outROuter) const;

        // ── Rendering passes ──────────────────────────────────────────────────
        void  DrawTitleBar(IRenderContext* ctx) const;       // v2.0.0
        void  DrawSeats(IRenderContext* ctx) const;
        void  DrawPieDonut(IRenderContext* ctx) const;       // v2.0.0
        void  DrawPartySeatLabels(IRenderContext* ctx) const;// v2.0.0
        void  DrawMajorityLine(IRenderContext* ctx) const;
        void  DrawCenterLabel(IRenderContext* ctx) const;
        void  DrawLegend(IRenderContext* ctx) const;         // v2.0.0

        // ── Color helpers  (v2.0.0) ───────────────────────────────────────────
        Color GetEffectivePartyColor(int partyIndex) const;
        Color ApplyAlphaFactor(const Color& base, float factor) const;
        Color ApplyBrightenFactor(const Color& base, float factor) const;

        // ── Hit testing ───────────────────────────────────────────────────────
        int   HitTestSeat(float mx, float my) const;
        int   HitTestSector(float mx, float my) const;        // v2.0.0 (PieDonut)

        // ── Helpers ───────────────────────────────────────────────────────────
        bool  HasOpenCenter() const;
        bool  IsArcLayout()   const;                          // v2.0.0
    };

} // namespace UltraCanvas