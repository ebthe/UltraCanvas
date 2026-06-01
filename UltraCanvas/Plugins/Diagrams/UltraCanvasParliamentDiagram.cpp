// src/Plugins/Diagrams/UltraCanvasParliamentDiagram.cpp
// Parliament / hemicycle seat distribution diagram — complete implementation
// Version: 2.0.17
// Last Modified: 2026-05-15
// Author: UltraCanvas Framework
//
// =============================================================================
// CHANGELOG
// =============================================================================
// v2.0.17 (2026-05-15) — Auto-fit bounds + full-block centering
//   * Auto-fit constants retuned for reliable fit across all dome sizes:
//       kAutoFitFactor      0.55 → 0.45  (block fills ~75% of hole height)
//       kAutoFitMinFontSize 12   →  8
//       kAutoFitMaxFontSize 28   → 22
//       sub ratio           0.375→ 0.33  (subtitle visually subordinate)
//       sub floor            6   →  6.0  (readable "Total Seats")
//       subGap range      [4,14]→[3,10]  (tighter, proportional)
//   * NEW: proportional scaling when candidate block exceeds 85% of rInner
//     — both main and sub font sizes are scaled down so the block always
//     fits the donut hole. Handles small-rInner presets (US Congress at
//     rInner≈18px) automatically without per-preset special cases.
//     Normal Bundestag (rInner≈37) is unaffected by the scaling.
//   * Semicircle: full-block centering with DYNAMIC bias proportional to
//     available headroom (bias = 0.50 + headroom / (2*rInner), capped at
//     0.70) — small holes stay geometrically centred while large domes get
//     the optical upward nudge, always leaving ≥4px breathing room vs the
//     inner arc. PLUS a fixed 3px optical lift (kOpticalLift) to compensate
//     for the Bold number's visual weight pulling the composition down.
//   * Circle: same full-block centering at cyArc (blockTop = cyArc -
//     totalBlockH * 0.5), so the label pair doesn't drift below centre.
//   * Horseshoe: unchanged (main-line-only centering preserved).
// v2.0.16 (2026-05-11) — Auto-fit center label
//   * New autoFitCenterLabel flag (default true). DrawCenterLabel computes
//     dynamic font sizes from rInner; user override via
//     SetCenterLabelFontSize() disables auto-fit.
//   * SetCenterLabelAutoFit(bool) / IsCenterLabelAutoFit() accessors.
// v2.0.15 (2026-05-11) — Final size + subtitle alignment polish
//   * Default centerLabelFontSize 24 -> 20 (header).
//   * kTextWidthRatio 0.38 -> 0.43 (measured: "Total Seats" at 9pt is
//     ~45px wide, real ratio = 45/(11*9) = 0.45; 0.43 conservative).
// v2.0.14 (2026-05-11) — Center label proportions
//   * Default centerLabelFontSize 28 -> 24 (header).
//   * Semicircle label vertical position: rInner * 0.55 -> rInner * 0.65
//     (label sits higher in the donut hole, not crowded against seats).
// v2.0.13 (2026-05-11) — Empirical Bold digit ratio + DIAG reverted
//   * Default centerLabelFontSize 28 -> 24 (header).
//   * Semicircle label vertical position: rInner * 0.55 -> rInner * 0.65
//     (label sits higher in the donut hole, not crowded against seats).
// v2.0.13 (2026-05-11) — Empirical Bold digit ratio + DIAG reverted
//   * kDigitBoldWidthRatio: 0.62 -> 0.78 (measured directly from rendered
//     pixels: "365" at 28pt Bold renders 65-66px wide → ratio 65/84 ≈ 0.78).
//   * std::cerr logs from v2.0.13-DIAG removed.
//   * <iostream> include removed.
// v2.0.13-DIAG (2026-05-11) — DIAGNOSTIC BUILD (now reverted)
// v2.0.12 (2026-05-11) — Bold digit width ratio
//   * New kDigitBoldWidthRatio = 0.62 for Bold-weight digits.
//   * Applied to DrawCenterLabel main and DrawPartySeatLabels.
//   * DrawMajorityLine threshold remains on Regular kDigitWidthRatio (0.52).
// v2.0.11 (2026-05-11) — Calibrated centering + uniform legend slots
//   * kTextWidthRatio recalibrated 0.46 -> 0.38 (centering subtitles).
//   * New kLegendTextWidthRatio = 0.55 (conservative for legend layout).
//   * DrawLegend reverts to uniform slot width for column alignment
//     across rows. Last incomplete row remains centered as a unit.
// v2.0.10 (2026-05-11) — Breathing room via dome fill factor
//   * Added kDomeFillFactor = 0.92f applied to maxR in ComputeArcGeometry.
//   * Removed kCenterLabelLift (no longer needed — smaller dome gives
//     natural space).
//   * kLegendTopPadding 10 -> 8 (smaller dome gives natural breathing room).
//   * Net result: ~8% breathing room on every side, replacing the
//     v2.0.7-v2.0.9 series of micro-offsets.
// v2.0.9 (2026-05-11) — Vertical breathing room + relative offsets
//   * Added kLegendTopPadding (10px) before the first legend row.
//   * Added kCenterLabelLift (6px) — center label block sits slightly
//     higher inside the donut opening.
//   * DrawMajorityLine threshold label positioned at cy - rOuter - 10px
//     instead of via fixed offsets from the line tip — scales with the
//     actual dome size.
// v2.0.8 (2026-05-11) — Semicircle vertical balance fix
//   * Semicircle cy derivation: two-pass approach. First compute maxR
//     from dw/2 only (no cy dependency). Then derive cy =
//     dy + dh*0.55 + rOuter*0.45 with a clamp to keep the dome inside
//     the drawing area when dh is the binding constraint. Fixes the
//     bottom-anchored dome that left ~176px of empty space above on
//     narrow cards (regression from v2.0.7).
// v2.0.7 (2026-05-11) — Semicircle space utilization
//   * ComputeArcGeometry: Semicircle anchor decoupled from max radius.
//     cy now uses a fixed bottom margin (kSemicircleBottomMargin = 4px)
//     and maxR is independently capped by min(dw/2, dh - margin - 2px).
//     Eliminates the double-0.88 application that wasted ~30% of vertical
//     space on Bottom-legend cards.
//   * Default legendRowHeight 18.0f -> 16.0f and bottom legend padding
//     12 -> 8 in ComputeDrawingArea. Recovers ~6px for the diagram.
// v2.0.6 (2026-05-11) — Centering precision + legend slot layout
//   * Split kCharWidthRatio into kDigitWidthRatio (0.52f) for numeric text
//     and kTextWidthRatio (0.46f) for alphabetic text. Eliminates the
//     ~3px leftward drift observed on subtitles like "Sitze" / "Seats".
//   * DrawLegend rewritten: per-slot measured widths instead of uniform
//     column slots. Each (swatch + label) pair occupies only its own
//     width; slots are separated by a fixed gap (legendSlotGap = 18px).
//     Within a row, all slots are concatenated; the full row width is
//     used to center the block within the legend area. For multi-row
//     legends, each row is centered independently so that incomplete
//     rows (e.g. "SSW 1" alone) sit visually centered too.
// v2.0.5 (2026-05-11) — Centering + repaint trigger
//   * OnEvent: RequestRedraw() called after hover/selection state changes
//     so the framework repaints immediately. Fixes click-to-select lag.
//   * Manual X centering for all texts that previously relied on
//     TextAlignment::Center: width is estimated as chars * fontSize *
//     ratio, leftX = centerX - width / 2. Applies to:
//     - DrawCenterLabel main number ("365", "630", "720")
//     - DrawCenterLabel subtitle ("Total Seats", "Sitze", "Seats")
//     - DrawPartySeatLabels (outline + main: 64, 120, 85, 208, 152)
//     - DrawMajorityLine threshold label ("316")
// v2.0.4 (2026-05-11) — Selection performance + visual polish
//   * SelectParty() and hover state changes no longer call InvalidateCache().
//     Selection only affects rendering colors; geometry is unchanged. This
//     eliminates the noticeable lag on click-to-select.
//   * DrawPartySeatLabels: per-sector numbers now respect selection dim
//     (alpha reduced for non-selected sectors), matching dots behaviour.
//   * DrawCenterLabel: subtitle ("Total Seats", "Sitze") gap from main
//     label increased to 10px to prevent overlap. Default font 9pt.
//   * DrawMajorityLine: threshold label sized 9pt, positioned with top-edge
//     offset above the line end; no longer overlaps the dome interior.
//   * DrawLegend: chord-width estimator 0.58 -> 0.52 for tighter centering.
// v2.0.3 (2026-05-11) — Visual refinements
//   * DrawPartySeatLabels: adaptive font size per sector based on the chord
//     length at the label radius. fontSize = clamp(chord * 0.35f, 6.0f,
//     partyLabelFontSize). Wide sectors get the configured size; narrow
//     sectors shrink down to 6pt minimum. Sectors too narrow even for 6pt
//     are skipped (chord < ~17px).
//   * DrawLegend: column width now sized from the widest party label (text
//     width estimated as chars * fontSize * 0.58f, a conservative fallback
//     for backends without MeasureText). The legend block is horizontally
//     centered within the legend area.
// v2.0.2 (2026-05-11) — CRITICAL: Semicircle angle range fix
//   * GetArcAngles() for Semicircle was returning [pi, 2*pi] (LOWER half of
//     the circle, where sin <= 0) when it should return the UPPER half
//     [0, pi] where sin >= 0. With Y-down screen mapping and
//     seat.Y = cy - r * sin(angle), negative sin placed dots BELOW the
//     anchor — which is exactly what we saw in v2.0.0/v2.0.1: dots
//     squashed against the bottom of the card.
//   * Fixed by returning [pi, 0] (reversed) — preserves left-to-right party
//     ordering convention while using sin >= 0 over the entire range.
//   * Updated DrawPieDonut, DrawPartySeatLabels, HitTestSector to handle
//     signed/reversed sector spans (sectorSpan can now be negative for
//     Semicircle).
// v2.0.1 (2026-05-11) — Geometry & layout fixes
//   * Fix title/subtitle overlap (DrawText Y = top-edge convention).
//   * Move semicircle dome anchor 0.92f -> 0.88f to keep arc within card.
//   * Reposition center label inside the donut opening (was at anchor).
//   * Extract ComputeArcGeometry() helper, eliminating 5x duplication.
// v2.0.0 (2026-05-11) — Major geometric refactor
//   * Replaced row-first party assignment with hybrid d3-parliament algorithm.
//   * Added ComputeSectors() (phase 1) reused by Dots and PieDonut.
//   * Added density compensation across rows.
//   * Real configurable arcSpanDegrees (180/270/360/intermediate).
//   * Added ParliamentRenderMode::PieDonut (scanline fan rendering).
//   * Added title bar, legend (4 positions), per-party seat labels.
//   * Added highlight/select visual (dim non-selected, brighten hovered).
//   * Added ApplyPreset() for Bundestag, EuropeanParliament, Westminster,
//     USCongress.
//   * Added onPartyHover callback alongside existing onSeatHover.
// v1.0.0 (2026-04-12) — Initial release
// =============================================================================

#include "Plugins/Diagrams/UltraCanvasParliamentDiagram.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace UltraCanvas {

    static constexpr float kPi      = 3.14159265358979323846f;
    static constexpr float kTwoPi   = 6.28318530717958647692f;
    static constexpr float kHalfPi  = 1.57079632679489661923f;

    // v2.0.6: split text-width estimation into two ratios, calibrated for
    // the sans-serif faces used by UltraCanvas backends (Cairo/Pango on
    // Linux, DirectWrite on Windows, Core Text on macOS).
    //   - kDigitWidthRatio: digits 0-9 in REGULAR weight. Sans-serif
    //     digits at regular weight have uniform width close to fontSize
    //     * 0.55; 0.52 is slightly conservative.
    //   - kDigitBoldWidthRatio: digits in BOLD weight. v2.0.13 calibrated
    //     directly from rendered pixels at 28pt: "365" renders 65px wide,
    //     so ratio = 65 / (3 * 28) ≈ 0.78. Cairo/Pango Bold sans-serif at
    //     large sizes is notably wider than the theoretical 0.62 estimate
    //     used in v2.0.12.
    //   - kTextWidthRatio: alphabetic text width estimate, used for X
    //     centering of subtitles. v2.0.15 recalibrated 0.38 -> 0.43 based
    //     on pixel measurement: "Total Seats" at 9pt renders ~45px wide
    //     (real ratio = 45/(11*9) ≈ 0.45); 0.43 is conservative to keep
    //     drift under 1px for shorter strings ("Sitze", "Seats").
    //   - kLegendTextWidthRatio: SEPARATE ratio for legend layout. The
    //     legend needs a CONSERVATIVE (slightly larger than real) width
    //     estimate so items don't visually touch. 0.55 leaves ~15% padding
    //     beyond the rendered text.
    static constexpr float kDigitWidthRatio      = 0.52f;
    static constexpr float kDigitBoldWidthRatio  = 0.78f;
    static constexpr float kTextWidthRatio       = 0.43f;
    static constexpr float kTextBoldWidthRatio   = 0.65f;
    static constexpr float kLegendTextWidthRatio = 0.55f;

    // v2.0.6: configurable horizontal gap between legend slots when each
    // slot is sized to its own (swatch + label) width.
    static constexpr float kLegendSlotGap   = 18.0f;

    // v2.0.7: introduced as a fixed bottom margin for Semicircle anchor.
    // v2.0.8: no longer used by the active algorithm (which derives cy
    // from dh and rOuter and clamps independently); kept defined for
    // documentation and potential future tuning of the bottom safety
    // distance if the clamp logic is changed.
    static constexpr float kSemicircleBottomMargin = 4.0f;

    // v2.0.10: vertical padding ABOVE the first row of a Bottom/Top legend.
    // Provides a small breathing gap between the diagram and the legend.
    // v2.0.9 used 10px; v2.0.10 reduces to 8 because the smaller dome
    // (see kDomeFillFactor) already provides natural visual separation.
    static constexpr float kLegendTopPadding = 8.0f;

    // v2.0.10: uniform shrink factor applied to the geometric maximum
    // radius computed from width/height constraints. The dome now occupies
    // 92% of the available space, giving 8% breathing room on every side.
    // This is a SYSTEMIC fix replacing the v2.0.7-v2.0.9 series of
    // micro-offsets (kCenterLabelLift, larger kLegendTopPadding, etc.)
    // that tried to patch the visual "bottom-heavy" feel pixel by pixel.
    // Applies uniformly across all presets and the default — users who
    // need a tighter or looser dome can override via SetOuterRadiusFraction.
    static constexpr float kDomeFillFactor = 0.97f;

// =============================================================================
// CONSTRUCTOR
// =============================================================================

    UltraCanvasParliamentDiagram::UltraCanvasParliamentDiagram(
        const std::string& id,
        long x, long y, long width, long height)
        : UltraCanvasUIElement(id, static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), static_cast<int>(height)) {
    }

// =============================================================================
// CACHE INVALIDATION
// =============================================================================

    void UltraCanvasParliamentDiagram::InvalidateCache() {
        cacheValid = false;
    }

// =============================================================================
// DATA API
// =============================================================================

    void UltraCanvasParliamentDiagram::AddParty(const ParliamentParty& party) {
        parties.push_back(party);
        InvalidateCache();
    }

    void UltraCanvasParliamentDiagram::SetParties(
        const std::vector<ParliamentParty>& partyList) {
        parties = partyList;
        InvalidateCache();
    }

    void UltraCanvasParliamentDiagram::ClearParties() {
        parties.clear();
        InvalidateCache();
    }

    int UltraCanvasParliamentDiagram::GetTotalSeats() const {
        int total = 0;
        for (const auto& p : parties) total += p.Seats;
        return total;
    }

// =============================================================================
// LAYOUT API
// =============================================================================

    void UltraCanvasParliamentDiagram::SetLayout(ParliamentLayout newLayout) {
        layout = newLayout;
        InvalidateCache();
    }

    void UltraCanvasParliamentDiagram::SetRenderMode(ParliamentRenderMode mode) {
        renderMode = mode;
        // PieDonut doesn't need seat positions but Dots does — keep cache
        // invalidation simple: any render mode switch invalidates.
        InvalidateCache();
    }

    void UltraCanvasParliamentDiagram::SetRows(int rowCount) {
        rows = (rowCount < 0) ? 0 : rowCount;
        InvalidateCache();
    }

    void UltraCanvasParliamentDiagram::SetArcSpanDegrees(float deg) {
        if (deg < 30.0f)  deg = 30.0f;
        if (deg > 360.0f) deg = 360.0f;
        arcSpanDegrees = deg;
        InvalidateCache();
    }

    void UltraCanvasParliamentDiagram::SetInnerRadiusFraction(float f) {
        if (f < 0.0f)  f = 0.0f;
        if (f > 0.95f) f = 0.95f;
        innerRadiusFrac = f;
        InvalidateCache();
    }

    void UltraCanvasParliamentDiagram::SetOuterRadiusFraction(float f) {
        if (f < 0.1f) f = 0.1f;
        if (f > 1.0f) f = 1.0f;
        outerRadiusFrac = f;
        InvalidateCache();
    }

    void UltraCanvasParliamentDiagram::SetDotRadius(float r) {
        if (r < 0.5f) r = 0.5f;
        dotRadius = r;
        InvalidateCache();
    }

    void UltraCanvasParliamentDiagram::SetDotSpacing(float s) {
        if (s < 0.0f) s = 0.0f;
        dotSpacing = s;
        InvalidateCache();
    }

// =============================================================================
// TITLE / SUBTITLE API
// =============================================================================

    void UltraCanvasParliamentDiagram::SetTitle(const std::string& text) {
        titleText = text;
        InvalidateCache();  // title height affects drawing area
    }

    void UltraCanvasParliamentDiagram::SetSubtitle(const std::string& text) {
        subtitleText = text;
        InvalidateCache();
    }

    void UltraCanvasParliamentDiagram::SetTitleColor(const Color& c) {
        titleColor = c;
    }

    void UltraCanvasParliamentDiagram::SetSubtitleColor(const Color& c) {
        subtitleColor = c;
    }

    void UltraCanvasParliamentDiagram::SetTitleFontSize(float size) {
        if (size < 8.0f) size = 8.0f;
        titleFontSize = size;
        InvalidateCache();
    }

    void UltraCanvasParliamentDiagram::SetSubtitleFontSize(float size) {
        if (size < 6.0f) size = 6.0f;
        subtitleFontSize = size;
        InvalidateCache();
    }

// =============================================================================
// MAJORITY LINE API
// =============================================================================

    void UltraCanvasParliamentDiagram::SetShowMajorityLine(bool show) {
        showMajorityLine = show;
    }

    void UltraCanvasParliamentDiagram::SetMajorityThreshold(int seats) {
        majorityThreshold = seats;
    }

    void UltraCanvasParliamentDiagram::SetMajorityLineColor(const Color& c) {
        majorityLineColor = c;
    }

    void UltraCanvasParliamentDiagram::SetMajorityLineWidth(float w) {
        if (w < 0.5f) w = 0.5f;
        majorityLineWidth = w;
    }

// =============================================================================
// CENTER LABEL API
// =============================================================================

    void UltraCanvasParliamentDiagram::SetShowCenterLabel(bool show) {
        showCenterLabel = show;
    }

    void UltraCanvasParliamentDiagram::SetCenterLabelText(
        const std::string& text) { centerLabelText = text; }

    void UltraCanvasParliamentDiagram::SetCenterLabelSubtext(
        const std::string& subtext) { centerLabelSubtext = subtext; }

    void UltraCanvasParliamentDiagram::SetCenterLabelColor(const Color& c) {
        centerLabelColor = c;
    }

    void UltraCanvasParliamentDiagram::SetCenterLabelSubColor(const Color& c) {
        centerLabelSubColor = c;
    }

    void UltraCanvasParliamentDiagram::SetCenterLabelFontSize(float size) {
        if (size < 6.0f) size = 6.0f;
        centerLabelFontSize = size;
        // v2.0.16: user explicitly setting font size opts out of auto-fit.
        autoFitCenterLabel = false;
    }

    void UltraCanvasParliamentDiagram::SetCenterLabelSubFontSize(float size) {
        if (size < 6.0f) size = 6.0f;
        centerLabelSubFontSize = size;
    }

    // v2.0.16: auto-fit accessors
    void UltraCanvasParliamentDiagram::SetCenterLabelAutoFit(bool enable) {
        autoFitCenterLabel = enable;
    }

    bool UltraCanvasParliamentDiagram::IsCenterLabelAutoFit() const {
        return autoFitCenterLabel;
    }

// =============================================================================
// PER-PARTY SEAT LABEL API
// =============================================================================

    void UltraCanvasParliamentDiagram::SetShowPartySeatLabels(bool show) {
        showPartySeatLabels = show;
    }

    void UltraCanvasParliamentDiagram::SetPartyLabelColor(const Color& c) {
        partyLabelColor = c;
    }

    void UltraCanvasParliamentDiagram::SetPartyLabelOutlineColor(const Color& c) {
        partyLabelOutlineColor = c;
    }

    void UltraCanvasParliamentDiagram::SetPartyLabelFontSize(float size) {
        if (size < 6.0f) size = 6.0f;
        partyLabelFontSize = size;
    }

// =============================================================================
// DOT STYLING API
// =============================================================================

    void UltraCanvasParliamentDiagram::SetShowDotBorder(bool show) {
        showDotBorder = show;
    }

    void UltraCanvasParliamentDiagram::SetDotBorderColor(const Color& c) {
        dotBorderColor = c;
    }

    void UltraCanvasParliamentDiagram::SetDotBorderWidth(float w) {
        if (w < 0.1f) w = 0.1f;
        dotBorderWidth = w;
    }

// =============================================================================
// HIGHLIGHT / SELECT VISUAL API
// =============================================================================

    void UltraCanvasParliamentDiagram::SetDimmedAlphaFactor(float factor) {
        if (factor < 0.0f) factor = 0.0f;
        if (factor > 1.0f) factor = 1.0f;
        dimmedAlphaFactor = factor;
    }

    void UltraCanvasParliamentDiagram::SetHoverBrightenFactor(float factor) {
        if (factor < 0.5f) factor = 0.5f;
        if (factor > 2.0f) factor = 2.0f;
        hoverBrightenFactor = factor;
    }

// =============================================================================
// LEGEND API
// =============================================================================

    void UltraCanvasParliamentDiagram::SetLegendPosition(
        ParliamentLegendPosition pos) {
        legendPosition = pos;
        InvalidateCache();  // legend position affects drawing area
    }

    void UltraCanvasParliamentDiagram::SetLegendColumns(int cols) {
        if (cols < 0) cols = 0;
        legendColumns = cols;
        InvalidateCache();
    }

    void UltraCanvasParliamentDiagram::SetLegendFontSize(float size) {
        if (size < 6.0f) size = 6.0f;
        legendFontSize = size;
        InvalidateCache();
    }

    void UltraCanvasParliamentDiagram::SetLegendTextColor(const Color& c) {
        legendTextColor = c;
    }

    void UltraCanvasParliamentDiagram::SetLegendSwatchSize(float size) {
        if (size < 4.0f) size = 4.0f;
        legendSwatchSize = size;
        InvalidateCache();
    }

    void UltraCanvasParliamentDiagram::SetLegendRowHeight(float h) {
        if (h < 10.0f) h = 10.0f;
        legendRowHeight = h;
        InvalidateCache();
    }

// =============================================================================
// OPPOSING BENCHES / CLASSROOM TUNING
// =============================================================================

    void UltraCanvasParliamentDiagram::SetOpposingBenchCols(int cols) {
        opposingBenchCols = (cols < 0) ? 0 : cols;
        InvalidateCache();
    }

    void UltraCanvasParliamentDiagram::SetClassroomCols(int cols) {
        classroomCols = (cols < 0) ? 0 : cols;
        InvalidateCache();
    }

// =============================================================================
// PRESETS
// =============================================================================

    void UltraCanvasParliamentDiagram::ApplyPreset(ParliamentPreset preset) {
        switch (preset) {
            case ParliamentPreset::Bundestag:
                layout          = ParliamentLayout::Semicircle;
                renderMode      = ParliamentRenderMode::Dots;
                arcSpanDegrees  = 180.0f;
                innerRadiusFrac = 0.32f;
                outerRadiusFrac = 0.92f;
                dotRadius       = 4.5f;
                dotSpacing      = 1.4f;
                showCenterLabel = true;
                centerLabelSubtext = "Sitze";
                showMajorityLine = false;
                legendPosition   = ParliamentLegendPosition::Bottom;
                legendColumns    = 0;
                showPartySeatLabels = true;
                showDotBorder    = false;
                break;

            case ParliamentPreset::EuropeanParliament:
                layout          = ParliamentLayout::Semicircle;
                renderMode      = ParliamentRenderMode::Dots;
                arcSpanDegrees  = 180.0f;
                innerRadiusFrac = 0.38f;   // larger center hole for EU logo/label
                outerRadiusFrac = 0.95f;
                dotRadius       = 4.0f;
                dotSpacing      = 1.2f;
                showCenterLabel = true;
                centerLabelSubtext = "Seats";
                showMajorityLine = false;  // EU doesn't show a single majority line
                legendPosition   = ParliamentLegendPosition::Right;
                legendColumns    = 1;
                showPartySeatLabels = false;
                showDotBorder    = false;
                break;

            case ParliamentPreset::Westminster:
                layout          = ParliamentLayout::OpposingBenches;
                renderMode      = ParliamentRenderMode::Dots;
                arcSpanDegrees  = 180.0f;   // ignored for OpposingBenches
                dotRadius       = 4.5f;
                dotSpacing      = 1.6f;
                showCenterLabel = false;
                showMajorityLine = false;
                legendPosition   = ParliamentLegendPosition::Bottom;
                legendColumns    = 0;
                showPartySeatLabels = false;
                showDotBorder    = false;
                break;

            case ParliamentPreset::USCongress:
                layout          = ParliamentLayout::Semicircle;
                renderMode      = ParliamentRenderMode::Dots;
                arcSpanDegrees  = 180.0f;
                innerRadiusFrac = 0.18f;   // small center, big seat area
                outerRadiusFrac = 0.95f;
                dotRadius       = 5.0f;
                dotSpacing      = 1.3f;
                showCenterLabel = true;
                centerLabelSubtext = "Total Seats";
                showMajorityLine = false;
                legendPosition   = ParliamentLegendPosition::Bottom;
                legendColumns    = 0;
                showPartySeatLabels = false;
                showDotBorder    = true;
                dotBorderColor   = Color(255, 255, 255, 220);
                dotBorderWidth   = 0.6f;
                break;
        }
        InvalidateCache();
    }

// =============================================================================
// INTERACTION
// =============================================================================

    void UltraCanvasParliamentDiagram::HighlightParty(int partyIndex) {
        if (partyIndex < -1 ||
            partyIndex >= static_cast<int>(parties.size())) return;
        hoveredPartyIndex = partyIndex;
    }

    void UltraCanvasParliamentDiagram::SelectParty(int partyIndex) {
        if (partyIndex < -1 ||
            partyIndex >= static_cast<int>(parties.size())) return;
        selectedPartyIndex = partyIndex;
        if (partyIndex >= 0 && onPartySelect) {
            onPartySelect(parties[partyIndex]);
        } else if (partyIndex < 0 && onPartyDeselect) {
            onPartyDeselect();
        }
    }

// =============================================================================
// GEOMETRY HELPERS
// =============================================================================

    bool UltraCanvasParliamentDiagram::IsArcLayout() const {
        return layout == ParliamentLayout::Semicircle ||
               layout == ParliamentLayout::Horseshoe  ||
               layout == ParliamentLayout::Circle;
    }

    bool UltraCanvasParliamentDiagram::HasOpenCenter() const {
        // True when there is an open central region that can host a center
        // label / EU logo / total seats text.
        return IsArcLayout();
    }

    float UltraCanvasParliamentDiagram::GetArcSpanForLayout() const {
        switch (layout) {
            case ParliamentLayout::Semicircle: return 180.0f;
            case ParliamentLayout::Horseshoe:  return 270.0f;
            case ParliamentLayout::Circle:     return 360.0f;
            default:                           return arcSpanDegrees;
        }
    }

    // Convention: angle 0 = right (+X), angle increases counter-clockwise
    // in math space. We render with Y-down so caller adjusts when computing
    // sin/cos to map angles to pixel space.
    //
    // For Semicircle (180°): arc spans from 180° (left) to 360° (right),
    // i.e. the upper half — we use 180°..360° so sin(angle) is negative
    // (= up in screen space when we flip Y).
    //
    // For Horseshoe (270°): arc spans from 135° to 405° (= 45°), opening at
    // the bottom — the "missing" 90° wedge sits at the bottom-center, matching
    // the speaker's table opening in Australian/NZ parliaments.
    //
    // For Circle (360°): we start at 90° (top) and go full circle, so the
    // first party starts at the top and parties wrap clockwise in screen
    // space. This matches the EU/Slovenia visual convention.
    //
    void UltraCanvasParliamentDiagram::GetArcAngles(
        float& startAngle, float& endAngle) const {
        const float spanDeg = GetArcSpanForLayout();
        const float spanRad = spanDeg * (kPi / 180.0f);

        switch (layout) {
            case ParliamentLayout::Semicircle: {
                // Upper half of the circle in math convention (X right, Y up):
                // angle 0   = right, sin(0)   =  0
                // angle pi/2 = top,  sin(pi/2) = +1   <-- top
                // angle pi  = left,  sin(pi)  =  0
                // So the upper half is the range [0, pi]. The arc is drawn
                // from the right edge (0) counter-clockwise to the left edge
                // (pi). In screen space (Y-down), seat.Y = cy - radius * sin(a)
                // with sin >= 0 places dots ABOVE cy, which is what we want
                // for a dome opening downward.
                //
                // BUT: parties are conventionally drawn left-to-right (govt
                // on left, opposition on right). Walking [0, pi] places the
                // first party on the RIGHT side. To get the conventional
                // left-to-right order, we walk the arc backwards: start=pi,
                // end=0. The packing loop uses start + sectorSpan where
                // sectorSpan is positive, so we keep start < end and reverse
                // the visual mapping by using [0, pi] with parties appearing
                // right-to-left, or [pi, 0] which we encode as start=pi,
                // end=0 with a NEGATIVE total span.
                //
                // Cleanest solution: keep start < end with [0, pi] and let
                // callers know parties walk right-to-left. Better solution:
                // make start > end (start = pi, end = 0) so the sector
                // allocator naturally walks from left (pi) to right (0).
                // We choose the latter because it preserves left-to-right
                // party ordering matching real parliament conventions.
                startAngle = kPi;        // left edge
                endAngle   = 0.0f;       // right edge (walking clockwise visually)
                break;
            }
            case ParliamentLayout::Horseshoe: {
                // 270° open at the bottom — opening centered on bottom (270°
                // in math = down when Y is up; in our Y-down render space we
                // map sin negatively, but the arc-angle math stays clean).
                // The closed arc covers the top + left + right; opening is the
                // 90° wedge centered at angle 270° (math).
                //
                // Arc start = 270° + 45° = 315° (= -45°), end = 315° + 270° = 585°
                // (= 225°). We keep them as continuous radians.
                const float openCenter = 1.5f * kPi;     // 270° (bottom)
                const float halfGap    = (kTwoPi - spanRad) * 0.5f;
                startAngle = openCenter + halfGap;       // 315°
                endAngle   = startAngle + spanRad;       // 585°
                break;
            }
            case ParliamentLayout::Circle: {
                // Full 360°, starting at the top (90° in math space)
                startAngle = kHalfPi;
                endAngle   = startAngle + kTwoPi;
                break;
            }
            default: {
                // Custom arcSpanDegrees: centered on top
                const float openCenter = kHalfPi;        // top
                const float halfSpan   = spanRad * 0.5f;
                startAngle = openCenter - halfSpan;
                endAngle   = openCenter + halfSpan;
                break;
            }
        }
    }

    float UltraCanvasParliamentDiagram::GetCenterX() const {
        return static_cast<float>(GetX()) +
               static_cast<float>(GetWidth()) * 0.5f;
    }

    float UltraCanvasParliamentDiagram::GetCenterY() const {
        // Default — refined per-layout inside ComputeArcLayout, which uses
        // the drawing area instead of element bounds directly.
        return static_cast<float>(GetY()) +
               static_cast<float>(GetHeight()) * 0.5f;
    }

    int UltraCanvasParliamentDiagram::AutoRows() const {
        // Pick row count proportional to sqrt(totalSeats) — same heuristic as
        // d3-parliament. Clamped between 4 and 16 for visual sanity.
        const int total = GetTotalSeats();
        if (total <= 0) return 4;
        int r = static_cast<int>(std::ceil(std::sqrt(
                    static_cast<float>(total) / 3.5f)));
        if (r < 4)  r = 4;
        if (r > 16) r = 16;
        return r;
    }

// =============================================================================
// DRAWING AREA COMPUTATION
// =============================================================================
//
//   The element bounds [GetX..GetX+GetWidth] × [GetY..GetY+GetHeight] are
//   carved up as follows:
//
//      ┌─────────────────────────────────────────────┐
//      │  TITLE  /  SUBTITLE   (titleAreaHeight)     │
//      ├─────────────────────────────────────────────┤
//      │   ┌──────────────────────────────────┐      │
//      │   │                                  │      │
//      │   │       DIAGRAM DRAWING AREA       │ LEG  │
//      │   │       (returned by this fn)      │ END  │
//      │   │                                  │      │
//      │   └──────────────────────────────────┘      │
//      ├─────────────────────────────────────────────┤
//      │           LEGEND   (legendAreaSize)         │
//      └─────────────────────────────────────────────┘
//
//   Only one of {Bottom, Right, Left, Top} subtracts from the diagram area
//   at a time. The title bar always sits at top regardless.
//
    void UltraCanvasParliamentDiagram::ComputeDrawingArea(
        float& outX, float& outY, float& outW, float& outH) const {

        const float elemX = static_cast<float>(GetX());
        const float elemY = static_cast<float>(GetY());
        const float elemW = static_cast<float>(GetWidth());
        const float elemH = static_cast<float>(GetHeight());

        // Title bar height
        float titleH = 0.0f;
        if (!titleText.empty())    titleH += titleFontSize + 6.0f;
        if (!subtitleText.empty()) titleH += subtitleFontSize + 4.0f;
        if (titleH > 0.0f)         titleH += 8.0f;  // bottom padding
        titleAreaHeight = titleH;

        // Legend area size (height for top/bottom, width for left/right)
        float legendArea = 0.0f;
        if (legendPosition != ParliamentLegendPosition::None &&
            !parties.empty()) {

            const int partyCount = static_cast<int>(parties.size());
            int cols = legendColumns;

            if (legendPosition == ParliamentLegendPosition::Bottom ||
                legendPosition == ParliamentLegendPosition::Top) {
                if (cols <= 0) {
                    // Auto: try to fit on 1-2 rows
                    cols = (partyCount <= 5) ? partyCount
                         : (partyCount <= 10) ? (partyCount + 1) / 2
                                              : (partyCount + 2) / 3;
                }
                const int rowsNeeded = (partyCount + cols - 1) / cols;
                // v2.0.9: legendArea = topPadding + rows + bottomPadding.
                // The topPadding (10px) gives breathing room between the
                // diagram and the legend; rows occupy rowsNeeded * rowHeight;
                // the trailing 8px is bottom padding.
                legendArea = kLegendTopPadding +
                             rowsNeeded * legendRowHeight + 8.0f;
            } else {
                // Left or Right
                if (cols <= 0) cols = 1;
                // Rough width estimate: swatch + gap + 12 chars worth of text
                legendArea = legendSwatchSize + 8.0f +
                             12.0f * (legendFontSize * 0.55f) + 16.0f;
                legendArea = std::max(legendArea, 100.0f);
                legendArea = std::min(legendArea, elemW * 0.4f);
            }
        }
        legendAreaSize = legendArea;

        // Start with full element minus title bar
        float dx = elemX;
        float dy = elemY + titleH;
        float dw = elemW;
        float dh = elemH - titleH;

        // Subtract legend area
        switch (legendPosition) {
            case ParliamentLegendPosition::Bottom:
                dh -= legendArea;
                break;
            case ParliamentLegendPosition::Top:
                dy += legendArea;
                dh -= legendArea;
                break;
            case ParliamentLegendPosition::Right:
                dw -= legendArea;
                break;
            case ParliamentLegendPosition::Left:
                dx += legendArea;
                dw -= legendArea;
                break;
            case ParliamentLegendPosition::None:
            default:
                break;
        }

        // Defensive clamp
        if (dw < 10.0f) dw = 10.0f;
        if (dh < 10.0f) dh = 10.0f;

        outX = dx;
        outY = dy;
        outW = dw;
        outH = dh;
    }

// =============================================================================
// COMPUTE ARC GEOMETRY  (v2.0.1 — single source of truth)
// =============================================================================
//
//   Centralizes the cx / cy / rOuter / rInner computation that was previously
//   duplicated across ComputeSeats, DrawPieDonut, DrawPartySeatLabels,
//   DrawMajorityLine, DrawCenterLabel, and HitTestSector. All of those now
//   call this helper to ensure visual alignment.
//
//   Anchor positions:
//     Semicircle: two-pass derivation [v2.0.8]
//                 Pass 1: maxR = dw / 2 - safety  (width-bound only)
//                 Pass 2: cy   = dy + dh*0.55 + rOuter*0.45  (centered with
//                               slight upward bias), clamped so the dome
//                               stays inside the drawing area.
//                 History: v2.0.0 used 0.92, v2.0.1 used 0.88, v2.0.7 used
//                 a fixed 4px bottom margin. All those approaches assumed
//                 dh was the binding constraint, but for narrow cards the
//                 actual limit is dw/2 — the older formulas then either
//                 wasted vertical space or pinned the dome to the bottom.
//     Horseshoe:  cy = dy + dh * 0.55f   (unchanged — opening at bottom)
//     Circle:     cy = dy + dh * 0.5f    (unchanged — true center)
//
    void UltraCanvasParliamentDiagram::ComputeArcGeometry(
        float& outCx, float& outCy,
        float& outRInner, float& outROuter) const {

        float dx, dy, dw, dh;
        ComputeDrawingArea(dx, dy, dw, dh);

        outCx = dx + dw * 0.5f;

        float maxR;
        switch (layout) {
            case ParliamentLayout::Semicircle: {
                // v2.0.8: two-pass derivation to avoid the bottom-anchor
                // problem from v2.0.7. The bug: cy = dy + dh - margin pins
                // the dome to the bottom edge, but when dw/2 is the binding
                // constraint on radius (narrow cards), this leaves huge
                // empty space ABOVE the dome.
                //
                // PASS 1: compute maxR from horizontal constraint alone.
                // The dome's diameter never exceeds the card width, so
                // dw / 2 is always a hard upper bound for radius.
                // PASS 2: derive cy so the dome's vertical center sits at
                // dy + dh * 0.55 (slight upward bias for "floating" look),
                // adjusted by rOuter * 0.45 to convert center-of-arc to
                // anchor (cy in our model is the bottom of the dome).
                //
                //   center_of_arc = dy + dh * 0.55
                //   cy            = center_of_arc + rOuter * 0.45
                //                 = dy + dh * 0.55 + rOuter * 0.45
                //
                // CLAMP: if the derived cy would push the dome top above
                // dy (clipping), clamp cy up to (dy + rOuter). Conversely
                // if cy would let the dome bottom exceed dy+dh, clamp cy
                // down to (dy + dh - 2). The two clamps protect against
                // any edge case (very small dh, very wide card, etc.).
                //
                // v2.0.10: kDomeFillFactor applied to halfW HERE (before
                // predicting rOuter for the cy computation) so the cy
                // prediction matches the actual rOuter that DrawSeats will
                // see. Applying it post-switch would cause a ~3-4px cy
                // drift because the prediction would be for an 8%-larger
                // dome than what actually renders.
                const float halfW = (dw * 0.5f - 2.0f) * kDomeFillFactor;
                maxR = (halfW > 0.0f) ? halfW : 0.0f;

                // Predict rOuter for cy derivation
                const float predictedROuter = maxR * outerRadiusFrac;
                float cy = dy + dh * 0.55f + predictedROuter * 0.45f;

                // Clamp: dome top must stay >= dy + 1 (don't clip into title)
                const float topMin   = dy + predictedROuter + 1.0f;
                if (cy < topMin) cy = topMin;
                // Clamp: dome bottom must stay <= dy + dh - 2 (small margin)
                const float bottomMax = dy + dh - 2.0f;
                if (cy > bottomMax) cy = bottomMax;

                // If vertical clamping reduced cy below topMin (i.e., the
                // drawing area is so short that the predicted radius does
                // not fit), shrink the radius to what fits.
                const float availV = cy - dy - 1.0f;  // dome top = cy - rOuter
                if (predictedROuter > availV) {
                    // Reduce maxR so that maxR * outerRadiusFrac <= availV
                    if (outerRadiusFrac > 0.0f) {
                        maxR = availV / outerRadiusFrac;
                    }
                    if (maxR < 0.0f) maxR = 0.0f;
                }

                outCy = cy;
                break;
            }
            case ParliamentLayout::Horseshoe:
                outCy = dy + dh * 0.55f;
                maxR  = std::min(dw, dh) * 0.5f * kDomeFillFactor;
                break;
            case ParliamentLayout::Circle:
            default:
                outCy = dy + dh * 0.5f;
                maxR  = std::min(dw, dh) * 0.5f * kDomeFillFactor;
                break;
        }

        outROuter = maxR * outerRadiusFrac;
        outRInner = maxR * innerRadiusFrac;
    }

// =============================================================================
// COMPUTE SECTORS  (PHASE 1 of hybrid algorithm)
// =============================================================================
//
//   For each party, allocate a contiguous angular range proportional to its
//   seat share. Sectors are placed in the order parties appear in the parties
//   vector — typical convention is left-to-right in semicircle: government
//   first (left), then coalition partners, then opposition.
//
    void UltraCanvasParliamentDiagram::ComputeSectors() const {
        computedSectors.clear();
        if (parties.empty()) return;

        const int total = GetTotalSeats();
        if (total <= 0) return;

        float startAngle, endAngle;
        GetArcAngles(startAngle, endAngle);
        const float totalSpan = endAngle - startAngle;

        float cursor = startAngle;
        for (size_t i = 0; i < parties.size(); ++i) {
            const float share = static_cast<float>(parties[i].Seats) /
                                static_cast<float>(total);
            const float sectorSpan = totalSpan * share;

            ParliamentSector sec;
            sec.PartyIndex    = static_cast<int>(i);
            sec.StartAngle    = cursor;
            sec.EndAngle      = cursor + sectorSpan;
            sec.AssignedSeats = parties[i].Seats;
            computedSectors.push_back(sec);

            cursor += sectorSpan;
        }
    }

// =============================================================================
// COMPUTE ARC LAYOUT  (PHASE 2 of hybrid algorithm)
// =============================================================================
//
//   For each row r in [0, rows), place dots along the arc at radius
//   rRow = rInner + (rOuter - rInner) * (r / (rows-1)). Across all parties
//   and rows, walk the angular range of each sector and place dots at evenly
//   spaced angular steps.
//
//   Density compensation: each row's angular step is computed from the row's
//   own circumference. Without compensation, inner rows (smaller radius) would
//   have dots much further apart in pixels than outer rows. We compute a
//   global "dots per pixel along arc" target from the outer row and divide
//   each party's seats across rows proportionally to each row's available arc
//   length within that party's sector.
//
//   This matches the d3-parliament "row-first with density compensation"
//   behaviour and produces the look of EU Parliament / Bundestag.
//
    void UltraCanvasParliamentDiagram::ComputeArcLayout(
        float cx, float cy, float rInner, float rOuter,
        float startAngle, float endAngle,
        int   resolvedRows) const {

        computedSeats.clear();
        if (parties.empty() || resolvedRows <= 0) return;

        ComputeSectors();
        if (computedSectors.empty()) return;

        // Pre-compute each row's radius
        std::vector<float> rowRadius(resolvedRows);
        if (resolvedRows == 1) {
            rowRadius[0] = (rInner + rOuter) * 0.5f;
        } else {
            for (int r = 0; r < resolvedRows; ++r) {
                const float t = static_cast<float>(r) /
                                static_cast<float>(resolvedRows - 1);
                rowRadius[r] = rInner + (rOuter - rInner) * t;
            }
        }

        // Pre-compute each row's arc-length weight (= radius). The total arc
        // length per party-sector across all rows is sum(radius[r] * sectorSpan).
        // We allocate seats per row proportionally to that row's weight.
        float totalRowWeight = 0.0f;
        for (int r = 0; r < resolvedRows; ++r) totalRowWeight += rowRadius[r];

        // Distribute each party's seats across rows.
        // We iterate parties × rows and place dots evenly within the row's
        // slice of the party's angular sector.
        for (const auto& sec : computedSectors) {
            if (sec.AssignedSeats <= 0) continue;
            const int   partyIdx   = sec.PartyIndex;
            const float sectorSpan = sec.EndAngle - sec.StartAngle;

            // Allocate seats per row proportional to row weight
            std::vector<int> seatsPerRow(resolvedRows, 0);
            int allocated = 0;
            for (int r = 0; r < resolvedRows; ++r) {
                const float share = rowRadius[r] / totalRowWeight;
                seatsPerRow[r] = static_cast<int>(
                    std::floor(sec.AssignedSeats * share));
                allocated += seatsPerRow[r];
            }

            // Distribute the remainder (from floor rounding) to rows with the
            // largest fractional remainders, starting from the outer row
            // (visually more space). We keep it simple: spread by largest
            // weights first.
            int remainder = sec.AssignedSeats - allocated;
            for (int r = resolvedRows - 1; r >= 0 && remainder > 0; --r) {
                seatsPerRow[r] += 1;
                --remainder;
            }
            // If somehow we over-allocated (shouldn't happen with floor), trim
            for (int r = 0; r < resolvedRows && remainder < 0; ++r) {
                if (seatsPerRow[r] > 0) {
                    seatsPerRow[r] -= 1;
                    ++remainder;
                }
            }

            // Place dots within each row's slice of the sector
            int seatCounter = 0;
            for (int r = 0; r < resolvedRows; ++r) {
                const int n = seatsPerRow[r];
                if (n <= 0) continue;

                const float radius = rowRadius[r];

                // Angular step: distribute n dots evenly within [start, end].
                // For n == 1 we center it in the sector slice for the row.
                // For n > 1 we use n+1 intervals so dots don't touch sector
                // edges (cleaner visual separation between adjacent parties).
                float a0, da;
                if (n == 1) {
                    a0 = sec.StartAngle + sectorSpan * 0.5f;
                    da = 0.0f;
                } else {
                    da = sectorSpan / static_cast<float>(n);
                    a0 = sec.StartAngle + da * 0.5f;
                }

                for (int k = 0; k < n; ++k) {
                    const float angle = a0 + da * k;
                    // Y-down screen space: y = cy - radius * sin(angle)
                    // (mirror sin so positive sin = upward visually for top
                    // arcs in Semicircle/Horseshoe).
                    ParliamentSeat seat;
                    seat.X = cx + radius * std::cos(angle);
                    seat.Y = cy - radius * std::sin(angle);
                    seat.PartyIndex  = partyIdx;
                    seat.SeatInParty = seatCounter++;
                    computedSeats.push_back(seat);
                }
            }
        }
    }

// =============================================================================
// COMPUTE OPPOSING LAYOUT  (Westminster)
// =============================================================================

    void UltraCanvasParliamentDiagram::ComputeOpposingLayout(
        float areaW, float areaH) const {

        computedSeats.clear();
        if (parties.empty()) return;

        const int total = GetTotalSeats();
        if (total <= 0) return;

        // Split parties into two benches by IsGoverning: governing on the
        // left, opposition on the right. Independents (neither) join the
        // smaller side to balance.
        std::vector<int> leftBench, rightBench;
        int leftSeats = 0, rightSeats = 0;
        for (size_t i = 0; i < parties.size(); ++i) {
            if (parties[i].IsGoverning) {
                leftBench.push_back(static_cast<int>(i));
                leftSeats += parties[i].Seats;
            } else {
                rightBench.push_back(static_cast<int>(i));
                rightSeats += parties[i].Seats;
            }
        }

        // Layout parameters
        const float kPad  = 8.0f;
        const float aW    = areaW - kPad * 2.0f;
        const float aH    = areaH - kPad * 2.0f;
        const float centerGap = std::max(40.0f, aW * 0.08f);
        const float benchW    = (aW - centerGap) * 0.5f;
        const float benchH    = aH;
        auto benchCols = [&](int benchSeatCount) {
            if (opposingBenchCols > 0) return opposingBenchCols;
            // Auto: aim for roughly square layout
            int cols = static_cast<int>(
                std::ceil(std::sqrt(static_cast<float>(benchSeatCount) *
                                    (benchW / benchH))));
            if (cols < 4)  cols = 4;
            if (cols > 30) cols = 30;
            return cols;
        };

        auto placeBench = [&](const std::vector<int>& bench, int seatCount,
                              float x0, float y0) {
            if (seatCount <= 0 || bench.empty()) return;

            const int   cols = benchCols(seatCount);
            const int   rowsNeeded = (seatCount + cols - 1) / cols;
            const float cellW = benchW / static_cast<float>(cols);
            const float cellH = benchH / static_cast<float>(rowsNeeded);

            int globalIndex = 0;
            // Walk parties in order, placing seats contiguously row-by-row
            for (int partyIdx : bench) {
                const int n = parties[partyIdx].Seats;
                for (int s = 0; s < n; ++s) {
                    const int col = globalIndex % cols;
                    const int row = globalIndex / cols;
                    ParliamentSeat seat;
                    seat.X = x0 + (col + 0.5f) * cellW;
                    seat.Y = y0 + (row + 0.5f) * cellH;
                    seat.PartyIndex  = partyIdx;
                    seat.SeatInParty = s;
                    computedSeats.push_back(seat);
                    ++globalIndex;
                }
            }
        };

        // Origin from drawing area
        float dx, dy, dw, dh;
        ComputeDrawingArea(dx, dy, dw, dh);

        const float ox = dx + kPad;
        const float oy = dy + kPad;
        placeBench(leftBench,  leftSeats,  ox,                       oy);
        placeBench(rightBench, rightSeats, ox + benchW + centerGap,  oy);
    }

// =============================================================================
// COMPUTE CLASSROOM LAYOUT
// =============================================================================

    void UltraCanvasParliamentDiagram::ComputeClassroomLayout(
        float areaW, float areaH) const {

        computedSeats.clear();
        if (parties.empty()) return;

        const int total = GetTotalSeats();
        if (total <= 0) return;

        const float kPad  = 8.0f;
        const float aW    = areaW - kPad * 2.0f;
        const float aH    = areaH - kPad * 2.0f;

        int cols = classroomCols;
        if (cols <= 0) {
            cols = static_cast<int>(
                std::ceil(std::sqrt(static_cast<float>(total) *
                                    (aW / aH))));
            if (cols < 6)  cols = 6;
            if (cols > 40) cols = 40;
        }
        const int rowsNeeded = (total + cols - 1) / cols;
        const float cellW = aW / static_cast<float>(cols);
        const float cellH = aH / static_cast<float>(rowsNeeded);

        float dx, dy, dw, dh;
        ComputeDrawingArea(dx, dy, dw, dh);
        const float ox = dx + kPad;
        const float oy = dy + kPad;

        int globalIndex = 0;
        for (size_t p = 0; p < parties.size(); ++p) {
            const int n = parties[p].Seats;
            for (int s = 0; s < n; ++s) {
                const int col = globalIndex % cols;
                const int row = globalIndex / cols;
                ParliamentSeat seat;
                seat.X = ox + (col + 0.5f) * cellW;
                seat.Y = oy + (row + 0.5f) * cellH;
                seat.PartyIndex  = static_cast<int>(p);
                seat.SeatInParty = s;
                computedSeats.push_back(seat);
                ++globalIndex;
            }
        }
    }

// =============================================================================
// COMPUTE SEATS  (top-level dispatcher)
// =============================================================================

    void UltraCanvasParliamentDiagram::ComputeSeats() const {
        cachedTotalSeats = GetTotalSeats();

        // For arc layouts we still need sector data for PieDonut mode even
        // when not packing individual dots — ComputeSectors() is cheap.
        if (IsArcLayout()) {
            ComputeSectors();
        } else {
            computedSectors.clear();
        }

        float dx, dy, dw, dh;
        ComputeDrawingArea(dx, dy, dw, dh);

        if (IsArcLayout()) {
            // v2.0.1: geometry centralized in ComputeArcGeometry()
            float cx, cy, rInner, rOuter;
            ComputeArcGeometry(cx, cy, rInner, rOuter);

            int resolvedRows = (rows > 0) ? rows : AutoRows();

            float startAngle, endAngle;
            GetArcAngles(startAngle, endAngle);

            // PieDonut doesn't need per-dot positions; bail out after sectors.
            if (renderMode == ParliamentRenderMode::PieDonut) {
                computedSeats.clear();
            } else {
                ComputeArcLayout(cx, cy, rInner, rOuter,
                                 startAngle, endAngle, resolvedRows);
            }
        } else if (layout == ParliamentLayout::OpposingBenches) {
            ComputeOpposingLayout(dw, dh);
        } else if (layout == ParliamentLayout::Classroom) {
            ComputeClassroomLayout(dw, dh);
        }

        cacheValid = true;
    }



// =============================================================================
// COLOR HELPERS
// =============================================================================

    Color UltraCanvasParliamentDiagram::ApplyAlphaFactor(
        const Color& base, float factor) const {
        if (factor < 0.0f) factor = 0.0f;
        if (factor > 1.0f) factor = 1.0f;
        Color out = base;
        out.a = static_cast<unsigned char>(
            std::round(static_cast<float>(base.a) * factor));
        return out;
    }

    Color UltraCanvasParliamentDiagram::ApplyBrightenFactor(
        const Color& base, float factor) const {
        auto clamp255 = [](float v) -> unsigned char {
            if (v < 0.0f)   v = 0.0f;
            if (v > 255.0f) v = 255.0f;
            return static_cast<unsigned char>(std::round(v));
        };
        Color out = base;
        out.r = clamp255(static_cast<float>(base.r) * factor);
        out.g = clamp255(static_cast<float>(base.g) * factor);
        out.b = clamp255(static_cast<float>(base.b) * factor);
        return out;
    }

    // Returns the color a party's seats / sector should be rendered with,
    // accounting for selection (dim others) and hover (brighten hovered).
    Color UltraCanvasParliamentDiagram::GetEffectivePartyColor(
        int partyIndex) const {

        if (partyIndex < 0 ||
            partyIndex >= static_cast<int>(parties.size())) {
            return Color(160, 160, 160, 255);
        }

        Color c = parties[partyIndex].PartyColor;

        // Hover: brighten the hovered party slightly
        if (partyIndex == hoveredPartyIndex && selectedPartyIndex < 0) {
            c = ApplyBrightenFactor(c, hoverBrightenFactor);
        }

        // Selection: dim non-selected parties
        if (selectedPartyIndex >= 0 && partyIndex != selectedPartyIndex) {
            c = ApplyAlphaFactor(c, dimmedAlphaFactor);
        }

        return c;
    }

// =============================================================================
// RENDER  (top-level orchestrator)
// =============================================================================

    void UltraCanvasParliamentDiagram::Render(IRenderContext* ctx, const Rect2Di& dirtyRect) {
        if (!ctx) return;
        if (!cacheValid) ComputeSeats();

        // 1. Title bar (always at top, unaffected by legend position)
        if (!titleText.empty() || !subtitleText.empty()) {
            DrawTitleBar(ctx);
        }

        // 2. Main diagram
        if (parties.empty() || GetTotalSeats() <= 0) {
            // Nothing to render beyond title — exit early to avoid drawing
            // empty center labels.
            return;
        }

        if (IsArcLayout() && renderMode == ParliamentRenderMode::PieDonut) {
            DrawPieDonut(ctx);
        } else {
            DrawSeats(ctx);
        }

        // 3. Per-party seat labels (only meaningful for arc layouts)
        if (showPartySeatLabels && IsArcLayout()) {
            DrawPartySeatLabels(ctx);
        }

        // 4. Majority line (arc layouts only)
        if (showMajorityLine && IsArcLayout()) {
            DrawMajorityLine(ctx);
        }

        // 5. Center label (arc layouts with open center only)
        if (showCenterLabel && HasOpenCenter()) {
            DrawCenterLabel(ctx);
        }

        // 6. Legend
        if (legendPosition != ParliamentLegendPosition::None) {
            DrawLegend(ctx);
        }
    }

// =============================================================================
// DRAW TITLE BAR
// =============================================================================

    void UltraCanvasParliamentDiagram::DrawTitleBar(IRenderContext* ctx) const {
        const float elemX = static_cast<float>(GetX());
        const float elemY = static_cast<float>(GetY());
        const float elemW = static_cast<float>(GetWidth());

        // NOTE on Y convention: in this rendering backend, DrawText(text, Point2Df(x, y))
        // interprets y as the TOP-edge of the text bounding box (verified against v1.0.0
        // visual output where +fontSize offset caused 18px gap that visually overlapped
        // the subtitle below). We use y = top-edge here and advance the cursor by the
        // glyph height + small padding.
        float yCursor = elemY + 4.0f;

        if (!titleText.empty()) {
            ctx->SetFontSize(titleFontSize);
            ctx->SetFontWeight(FontWeight::Bold);
            ctx->SetTextPaint(titleColor);
            TextStyle style;
            style.alignment  = TextAlignment::Left;
            ctx->SetTextStyle(style);
            ctx->DrawText(titleText, Point2Df(elemX + 8.0f, yCursor));
            yCursor += titleFontSize + 4.0f;
        }

        if (!subtitleText.empty()) {
            ctx->SetFontSize(subtitleFontSize);
            ctx->SetFontWeight(FontWeight::Normal);
            ctx->SetTextPaint(subtitleColor);
            TextStyle style;
            style.alignment  = TextAlignment::Left;
            ctx->SetTextStyle(style);
            ctx->DrawText(subtitleText, Point2Df(elemX + 8.0f, yCursor));
        }

        (void)elemW;  // currently unused; reserved for centered title variants
    }

// =============================================================================
// DRAW SEATS  (Dots render mode)
// =============================================================================

    void UltraCanvasParliamentDiagram::DrawSeats(IRenderContext* ctx) const {
        for (const auto& seat : computedSeats) {
            const Color fill = GetEffectivePartyColor(seat.PartyIndex);

            ctx->SetFillPaint(fill);
            ctx->FillCircle(Point2Df(seat.X, seat.Y), dotRadius);

            if (showDotBorder) {
                // Match border alpha to fill alpha so dimmed dots get dimmed
                // borders too — avoids "ghost ring" artefacts on faded parties.
                Color border = dotBorderColor;
                border.a = static_cast<unsigned char>(
                    (static_cast<int>(border.a) *
                     static_cast<int>(fill.a)) / 255);

                ctx->SetStrokePaint(border);
                ctx->SetStrokeWidth(dotBorderWidth);
                ctx->DrawCircle(Point2Df(seat.X, seat.Y), dotRadius);
            }
        }
    }

// =============================================================================
// DRAW PIE-DONUT  (solid filled sectors via scanline fan)
// =============================================================================
//
//   IRenderContext exposes no FillPolygon, so we render each sector as a
//   triangle fan emitted as horizontal DrawLine scanlines (same approach used
//   in UltraCanvasBlockDiagram arrowheads). Cross-backend safe.
//
//   For each sector:
//     - Compute outer arc polyline (segmented)
//     - Compute inner arc polyline (reverse direction, segmented)
//     - Form a closed polygon: outer arc forward + inner arc backward
//     - Rasterize via scanline fill using min/max X per Y row
//
    static void RasterizeFilledPolygon(IRenderContext* ctx,
                                       const std::vector<Point2Df>& poly,
                                       const Color& fill) {
        if (poly.size() < 3) return;

        // Find Y bounds
        float minY = poly[0].y, maxY = poly[0].y;
        for (const auto& p : poly) {
            if (p.y < minY) minY = p.y;
            if (p.y > maxY) maxY = p.y;
        }
        const int yStart = static_cast<int>(std::floor(minY));
        const int yEnd   = static_cast<int>(std::ceil(maxY));

        ctx->SetStrokePaint(fill);
        ctx->SetStrokeWidth(1.0f);
        ctx->SetFillPaint(fill);

        // Scanline: for each integer Y, find all X intersections with polygon
        // edges, sort, draw horizontal segments between pairs.
        for (int y = yStart; y <= yEnd; ++y) {
            const float yf = static_cast<float>(y) + 0.5f;
            std::vector<float> xs;

            for (size_t i = 0; i < poly.size(); ++i) {
                const Point2Df& a = poly[i];
                const Point2Df& b = poly[(i + 1) % poly.size()];

                // Skip horizontal edges (don't contribute crossings)
                if ((a.y <= yf && b.y > yf) || (b.y <= yf && a.y > yf)) {
                    const float t = (yf - a.y) / (b.y - a.y);
                    xs.push_back(a.x + t * (b.x - a.x));
                }
            }

            if (xs.size() < 2) continue;
            std::sort(xs.begin(), xs.end());

            for (size_t k = 0; k + 1 < xs.size(); k += 2) {
                ctx->DrawLine(Point2Df(xs[k],     yf),
                              Point2Df(xs[k + 1], yf));
            }
        }
    }

    void UltraCanvasParliamentDiagram::DrawPieDonut(IRenderContext* ctx) const {
        if (computedSectors.empty()) return;

        // v2.0.1: single source of truth via ComputeArcGeometry()
        float cx, cy, rInner, rOuter;
        ComputeArcGeometry(cx, cy, rInner, rOuter);

        // Small angular gap between adjacent sectors for visual separation
        const float gapRad = 0.004f;  // ~0.25°

        for (const auto& sec : computedSectors) {
            if (sec.AssignedSeats <= 0) continue;
            const Color fill = GetEffectivePartyColor(sec.PartyIndex);

            // Handle signed sector span: Semicircle uses [pi, 0] so EndAngle <
            // StartAngle, giving sectorSpan < 0. PieDonut math is sign-agnostic
            // for cos/sin but we must shrink the gap on the correct side.
            const float fullSpan = sec.EndAngle - sec.StartAngle;
            if (std::fabs(fullSpan) <= 2.0f * gapRad) continue;
            const float gapSigned = (fullSpan >= 0.0f) ? gapRad : -gapRad;
            const float a0   = sec.StartAngle + gapSigned;
            const float a1   = sec.EndAngle   - gapSigned;
            const float span = a1 - a0;
            if (std::fabs(span) < 1e-4f) continue;

            // Segment count proportional to |sector span| - smooth curvature
            // without over-tessellating tiny sectors.
            int segs = static_cast<int>(std::ceil(std::fabs(span) * 24.0f));
            if (segs < 3)  segs = 3;
            if (segs > 96) segs = 96;

            std::vector<Point2Df> poly;
            poly.reserve((segs + 1) * 2);

            // Outer arc forward (signed span direction)
            for (int i = 0; i <= segs; ++i) {
                const float t = static_cast<float>(i) /
                                static_cast<float>(segs);
                const float a = a0 + t * span;
                poly.emplace_back(cx + rOuter * std::cos(a),
                                  cy - rOuter * std::sin(a));
            }
            // Inner arc backward
            for (int i = segs; i >= 0; --i) {
                const float t = static_cast<float>(i) /
                                static_cast<float>(segs);
                const float a = a0 + t * span;
                poly.emplace_back(cx + rInner * std::cos(a),
                                  cy - rInner * std::sin(a));
            }

            RasterizeFilledPolygon(ctx, poly, fill);
        }
    }

// =============================================================================
// DRAW PARTY SEAT LABELS  (Bundestag-style numbers on each sector)
// =============================================================================

    void UltraCanvasParliamentDiagram::DrawPartySeatLabels(
        IRenderContext* ctx) const {
        if (computedSectors.empty()) return;

        // v2.0.1: single source of truth via ComputeArcGeometry()
        float cx, cy, rInner, rOuter;
        ComputeArcGeometry(cx, cy, rInner, rOuter);
        const float rLabel = (rInner + rOuter) * 0.5f;  // mid-radius

        // v2.0.3: adaptive font sizing. The available horizontal space within
        // a sector at the label radius is approximately the chord length
        // between the sector's start and end at rLabel:
        //
        //     chord = 2 * rLabel * sin(|sectorSpan| / 2)
        //
        // We size the font so a 3-digit number ("208", "152") fits comfortably
        // within ~60% of the chord. Fall back to 6pt minimum for legibility;
        // skip the label entirely if even 6pt won't fit.
        //
        // The chord-to-fontSize multiplier 0.35 is calibrated so that the
        // estimated text width (digits * fontSize * 0.58) lands at ~0.6 * chord.
        // For a 3-digit number: width = 3 * fontSize * 0.58 = 1.74 * fontSize.
        // Setting 1.74 * fontSize = 0.6 * chord -> fontSize = 0.345 * chord.

        TextStyle style;
        style.alignment = TextAlignment::Left;   // v2.0.5: we center manually

        for (const auto& sec : computedSectors) {
            if (sec.AssignedSeats <= 0) continue;

            // |span| handles Semicircle's reversed sector convention
            const float spanAbs = std::fabs(sec.EndAngle - sec.StartAngle);
            const float chordPx = 2.0f * rLabel * std::sin(spanAbs * 0.5f);

            // Adaptive font size — clamp to [6pt, partyLabelFontSize]
            float fSize = chordPx * 0.35f;
            if (fSize > partyLabelFontSize) fSize = partyLabelFontSize;
            if (fSize < 6.0f) continue;  // sector too tiny — skip label

            // Number of digits in the label drives the width estimate. If
            // even at fSize the text wouldn't fit, skip it.
            char buf[24];
            const int len = std::snprintf(buf, sizeof(buf), "%d",
                                          sec.AssignedSeats);
            const float estTextWidth = static_cast<float>(len) * fSize * 0.58f;
            if (estTextWidth > chordPx * 0.85f) continue;  // wouldn't fit

            const float midA = (sec.StartAngle + sec.EndAngle) * 0.5f;
            const float lxCenter = cx + rLabel * std::cos(midA);
            const float lyCenter = cy - rLabel * std::sin(midA);

            // DrawText Y is top-edge in this backend; shift up by fSize/2 to
            // visually center the glyph around (lxCenter, lyCenter).
            // v2.0.5: also center X manually (TextAlignment::Center ignored).
            // v2.0.6: kDigitWidthRatio — labels are integer seat counts.
            // v2.0.12: switched to kDigitBoldWidthRatio since the labels
            // render with FontWeight::Bold (set a few lines below). Bold
            // digits are ~18% wider than Regular, so the old 0.52 ratio
            // under-estimated and pushed the text to the right of each
            // sector's geometric midpoint.
            const float lyTop = lyCenter - fSize * 0.5f;
            const float labelTextWidth =
                static_cast<float>(len) * fSize * kDigitBoldWidthRatio;
            const float lxLeft = lxCenter - labelTextWidth * 0.5f;

            // Compute selection dimming once; applies to BOTH outline and
            // main text so the label fully fades for non-selected sectors
            // instead of leaving a dark outline halo behind.
            const bool dimThisLabel =
                (selectedPartyIndex >= 0 &&
                 sec.PartyIndex != selectedPartyIndex);

            // Apply the adaptive size BEFORE outline pass
            ctx->SetFontSize(fSize);
            ctx->SetFontWeight(FontWeight::Bold);

            // Render outline behind text for legibility on any background:
            // 4-way offset trick (cheap text outline).
            Color outlineCol = partyLabelOutlineColor;
            if (dimThisLabel) {
                outlineCol = ApplyAlphaFactor(outlineCol, dimmedAlphaFactor);
            }
            ctx->SetTextPaint(outlineCol);
            ctx->SetTextStyle(style);
            for (float ox = -1.0f; ox <= 1.0f; ox += 1.0f) {
                for (float oy = -1.0f; oy <= 1.0f; oy += 1.0f) {
                    if (ox == 0.0f && oy == 0.0f) continue;
                    ctx->DrawText(buf, Point2Df(lxLeft + ox, lyTop + oy));
                }
            }

            // Main text
            Color labelCol = partyLabelColor;
            if (dimThisLabel) {
                labelCol = ApplyAlphaFactor(labelCol, dimmedAlphaFactor);
            }
            ctx->SetTextPaint(labelCol);
            ctx->SetTextStyle(style);
            ctx->DrawText(buf, Point2Df(lxLeft, lyTop));
        }
    }

// =============================================================================
// DRAW MAJORITY LINE
// =============================================================================

    void UltraCanvasParliamentDiagram::DrawMajorityLine(
        IRenderContext* ctx) const {
        if (computedSectors.empty()) return;

        const int total = cachedTotalSeats;
        const int threshold = (majorityThreshold > 0)
            ? majorityThreshold
            : (total / 2 + 1);
        if (threshold <= 0 || threshold > total) return;

        // Walk sectors accumulating seats; locate the angle at which the
        // running sum crosses the threshold.
        int running = 0;
        float majorityAngle = 0.0f;
        bool found = false;

        for (const auto& sec : computedSectors) {
            const int next = running + sec.AssignedSeats;
            if (next >= threshold) {
                const float t = (sec.AssignedSeats > 0)
                    ? static_cast<float>(threshold - running) /
                      static_cast<float>(sec.AssignedSeats)
                    : 0.0f;
                majorityAngle = sec.StartAngle +
                                t * (sec.EndAngle - sec.StartAngle);
                found = true;
                break;
            }
            running = next;
        }
        if (!found) return;

        // v2.0.1: single source of truth via ComputeArcGeometry()
        float cx, cy, rInner, rOuter;
        ComputeArcGeometry(cx, cy, rInner, rOuter);
        const float rEnd = rOuter + dotRadius + 4.0f;

        const float x1 = cx + rInner * std::cos(majorityAngle);
        const float y1 = cy - rInner * std::sin(majorityAngle);
        const float x2 = cx + rEnd   * std::cos(majorityAngle);
        const float y2 = cy - rEnd   * std::sin(majorityAngle);

        ctx->SetStrokePaint(majorityLineColor);
        ctx->SetStrokeWidth(majorityLineWidth);
        ctx->DrawLine(Point2Df(x1, y1), Point2Df(x2, y2));

        // v2.0.9: threshold label position is now relative to the dome's
        // outer radius geometry (cy - rOuter - margin), not to the line
        // tip (y2). Previously y2 = cy - rEnd*sin(angle) varied with the
        // majority angle: a near-vertical majority line put y2 ~rEnd above
        // cy, but a slanted line put y2 only ~rEnd*sin(angle) above, which
        // made the label position visually inconsistent across data sets.
        // Tying the label to (cy - rOuter - 10) keeps it at a stable
        // distance above the dome regardless of where the majority line
        // intersects the arc.
        const float labelFontSize = 9.0f;
        const float labelMargin   = 10.0f;
        // Top-edge convention: subtract fontSize so the text bbox sits
        // with its BOTTOM at (cy - rOuter - labelMargin).
        const float labelTop = cy - rOuter - labelMargin - labelFontSize;

        char buf[24];
        const int thLen = std::snprintf(buf, sizeof(buf), "%d", threshold);
        // v2.0.6: kDigitWidthRatio — threshold is an integer.
        const float thWidth =
            static_cast<float>(thLen) * labelFontSize * kDigitWidthRatio;
        // Center the label horizontally on the line's outer x coordinate
        // (which is where the visual majority indicator points to).
        const float thLeftX = x2 - thWidth * 0.5f;

        ctx->SetFontSize(labelFontSize);
        ctx->SetFontWeight(FontWeight::Normal);
        ctx->SetTextPaint(majorityLineColor);
        TextStyle style;
        style.alignment  = TextAlignment::Left;   // v2.0.5: we center manually
        ctx->SetTextStyle(style);
        ctx->DrawText(buf, Point2Df(thLeftX, labelTop));
    }

// =============================================================================
// DRAW CENTER LABEL
// =============================================================================

    void UltraCanvasParliamentDiagram::DrawCenterLabel(
        IRenderContext* ctx) const {

        float cxArc, cyArc, rInner, rOuter;
        ComputeArcGeometry(cxArc, cyArc, rInner, rOuter);

        // v2.0.17: auto-fit scales the entire label block to fit
        // comfortably inside the donut hole. Constants tuned so the
        // block occupies ~70-80% of the hole height across the full
        // range of dome sizes (rInner ≈ 18-70 px). When the candidate
        // block would exceed kMaxHoleFill (85%) of rInner, both main
        // and sub font sizes are scaled down proportionally — this
        // handles small-rInner presets like US Congress automatically
        // without per-preset special cases.
        float effMainFontSize = centerLabelFontSize;
        float effSubFontSize  = centerLabelSubFontSize;
        // v2.0.18: auto-fit gap constants, may be overridden by small-hole branch
        float gapRatio = 0.30f, gapMin = 3.0f, gapMax = 10.0f;
        if (autoFitCenterLabel) {
            constexpr float kMaxHoleFill = 0.85f;
            float candMain, candSub, candGap;

            if (rInner < 28.0f) {
                // Small donut hole (<28px): tuned for US Congress / tight
                constexpr float kFactor    = 0.48f;
                constexpr float kMinMain   = 10.0f;
                constexpr float kMaxMain   = 24.0f;
                constexpr float kSubRatio  = 0.34f;
                constexpr float kMinSub    = 6.0f;
                constexpr float kMaxSub    = 9.0f;
                constexpr float kGapRatio  = 0.22f;
                constexpr float kMinGap    = 3.0f;
                constexpr float kMaxGap    = 8.0f;

                candMain = std::clamp(rInner * kFactor, kMinMain, kMaxMain);
                candSub  = std::clamp(candMain * kSubRatio,
                                      kMinSub, kMaxSub);
                candGap  = std::clamp(candMain * kGapRatio,
                                      kMinGap, kMaxGap);
                gapRatio = kGapRatio; gapMin = kMinGap; gapMax = kMaxGap;
            } else {
                // Medium/large hole (>=28px): original auto-fit constants
                constexpr float kFactor    = 0.45f;
                constexpr float kMinMain   = 8.0f;
                constexpr float kMaxMain   = 36.0f;
                constexpr float kSubRatio  = 0.33f;
                constexpr float kMinSub    = 6.0f;
                constexpr float kGapRatio  = 0.30f;
                constexpr float kMinGap    = 3.0f;
                constexpr float kMaxGap    = 10.0f;

                candMain = std::clamp(rInner * kFactor, kMinMain, kMaxMain);
                candSub  = std::max(kMinSub, candMain * kSubRatio);
                candGap  = std::clamp(candMain * kGapRatio,
                                      kMinGap, kMaxGap);
                gapRatio = kGapRatio; gapMin = kMinGap; gapMax = kMaxGap;
            }

            const float candTotal = candMain + candGap + candSub;
            const float maxBlockH = rInner * kMaxHoleFill;
            if (candTotal > maxBlockH) {
                const float s = maxBlockH / candTotal;
                effMainFontSize = std::max(candMain * 0.75f,
                                           candMain * s);
                effSubFontSize  = std::max(candSub * 0.75f,
                                           candSub * s);
            } else {
                effMainFontSize = candMain;
                effSubFontSize  = candSub;
            }
        }

        // Resolve text: explicit override OR auto-derived from total seats
        std::string mainText = centerLabelText;
        if (mainText.empty()) {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%d", cachedTotalSeats);
            mainText = buf;
        }

        // v2.0.17: subtitle gap proportional to main font but soft-clamped.
        const bool hasSub = !centerLabelSubtext.empty();
        const float subGap = hasSub
            ? std::clamp(effMainFontSize * gapRatio, gapMin, gapMax)
            : 0.0f;
        const float totalBlockH = effMainFontSize
            + subGap
            + (hasSub ? effSubFontSize : 0.0f);

        float labelCx = cxArc;
        float blockTop;
        switch (layout) {
            case ParliamentLayout::Semicircle: {
                // v2.0.17: centre the entire label block within the donut
                // opening [cyArc - rInner, cyArc]. A dynamic bias factor
                // shifts the block centre above the geometric midpoint
                // (0.50) to compensate for the Bold number's visual weight
                // and the asymmetric dome shape — proportional to available
                // headroom so small holes (US Congress rInner≈18) stay
                // centred while large domes get a stronger upward nudge.
                // The block always maintains at least `topPadding` px of
                // breathing room against the inner arc.
                {
                    // v2.0.20: fixed slight upward bias (54 %) so the block
                    // sits a touch above the geometric centre in all cases.
                    // Keeps the text inside the hole without crowding the
                    // inner arc, regardless of hole size or legend.
                    constexpr float kBias = 0.45f;
                    const float holeCenterY = cyArc - rInner * kBias;
                    blockTop = holeCenterY - totalBlockH * 0.5f;
                    // Clamp: never push the block above the inner arc
                    const float holeTopY = cyArc - rInner;
                    if (blockTop < holeTopY + 1.0f)
                        blockTop = holeTopY + 1.0f;
                }
                }
                break;
            case ParliamentLayout::Horseshoe:
                // Keep existing behaviour: centre main text line at
                // cyArc - rInner * 0.20.
                blockTop = (cyArc - rInner * 0.20f)
                         - effMainFontSize * 0.5f;
                break;
            case ParliamentLayout::Circle:
            default:
                // v2.0.17: centre the full block at the arc centre, not
                // just the main text line, so subtitle doesn't dip below
                // the visual centre.
                blockTop = cyArc - totalBlockH * 0.5f;
                break;
        }
        const float mainTop = blockTop;

        // NOTE: DrawText Y is top-edge of text bbox in this backend.
        // v2.0.5: centre X manually — TextAlignment::Center is ignored
        // by Cairo/Pango when DrawText receives a single point, so we
        // estimate text width and shift X by -width/2.
        // v2.0.6: pick the right ratio based on whether the main text is
        // pure digits (typical: "365", "630") or contains letters (custom
        // override). All-digits -> kDigitBoldWidthRatio (the main label is
        // always rendered Bold), otherwise text ratio.
        // v2.0.12: switched digit branch from kDigitWidthRatio (0.52,
        // calibrated for Regular weight) to kDigitBoldWidthRatio (0.62)
        // because the main label uses FontWeight::Bold below. Bold glyphs
        // are ~18% wider than regular; the old ratio under-estimated the
        // width and shifted the text ~6-8px to the right of center.
        bool mainAllDigits = !mainText.empty();
        for (char c : mainText) {
            if (c < '0' || c > '9') { mainAllDigits = false; break; }
        }
        const float mainRatio = mainAllDigits
            ? kDigitBoldWidthRatio
            : kTextWidthRatio;
        const float mainWidth =
            static_cast<float>(mainText.size()) *
            effMainFontSize * mainRatio;
        const float mainLeftX = labelCx - mainWidth * 0.5f;

        ctx->SetFontSize(effMainFontSize);
        ctx->SetFontWeight(FontWeight::Bold);
        ctx->SetTextPaint(centerLabelColor);
        TextStyle mainStyle;
        mainStyle.alignment = TextAlignment::Left;
        ctx->SetTextStyle(mainStyle);

        ctx->DrawText(mainText, Point2Df(mainLeftX, mainTop));

        if (hasSub) {
            const float subTop = mainTop + effMainFontSize + subGap;

            // v2.0.20: ExtraBold weight with kTextBoldWidthRatio (0.65).
            // The ratio compensates for wider ExtraBold glyphs at small
            // sizes; 0.55 (Bold) still showed a ~2-3px rightward shift.
            const float subWidth =
                static_cast<float>(centerLabelSubtext.size()) *
                effSubFontSize * kTextBoldWidthRatio;
            const float subLeftX = labelCx - subWidth * 0.5f;

            ctx->SetFontSize(effSubFontSize);
            ctx->SetFontWeight(FontWeight::ExtraBold);
            ctx->SetTextPaint(centerLabelSubColor);
            TextStyle subStyle;
            subStyle.alignment = TextAlignment::Left;
            ctx->SetTextStyle(subStyle);
            ctx->DrawText(centerLabelSubtext, Point2Df(subLeftX, subTop));
        }
    }

// =============================================================================
// DRAW LEGEND
// =============================================================================

    void UltraCanvasParliamentDiagram::DrawLegend(IRenderContext* ctx) const {
        if (parties.empty()) return;

        const float elemX = static_cast<float>(GetX());
        const float elemY = static_cast<float>(GetY());
        const float elemW = static_cast<float>(GetWidth());
        const float elemH = static_cast<float>(GetHeight());

        const int partyCount = static_cast<int>(parties.size());

        float legendX = elemX, legendY = elemY, legendW = elemW, legendH = elemH;
        int cols = legendColumns;
        bool isHorizontal = false;  // true for Bottom/Top, false for Left/Right

        switch (legendPosition) {
            case ParliamentLegendPosition::Bottom:
                legendX = elemX + 8.0f;
                // v2.0.9: top padding inside the legend area, before the
                // first row, replaces the previous fixed +6px offset.
                legendY = elemY + elemH - legendAreaSize + kLegendTopPadding;
                legendW = elemW - 16.0f;
                legendH = legendAreaSize - kLegendTopPadding - 8.0f;
                isHorizontal = true;
                if (cols <= 0) {
                    cols = (partyCount <= 5) ? partyCount
                         : (partyCount <= 10) ? (partyCount + 1) / 2
                                              : (partyCount + 2) / 3;
                }
                break;
            case ParliamentLegendPosition::Top:
                legendX = elemX + 8.0f;
                // v2.0.9: same padding for Top position. The legend sits
                // below the title bar with kLegendTopPadding of breathing
                // room above the first row.
                legendY = elemY + titleAreaHeight + kLegendTopPadding;
                legendW = elemW - 16.0f;
                legendH = legendAreaSize - kLegendTopPadding - 8.0f;
                isHorizontal = true;
                if (cols <= 0) {
                    cols = (partyCount <= 5) ? partyCount
                         : (partyCount <= 10) ? (partyCount + 1) / 2
                                              : (partyCount + 2) / 3;
                }
                break;
            case ParliamentLegendPosition::Right:
                legendX = elemX + elemW - legendAreaSize + 8.0f;
                legendY = elemY + titleAreaHeight + 8.0f;
                legendW = legendAreaSize - 16.0f;
                legendH = elemH - titleAreaHeight - 16.0f;
                if (cols <= 0) cols = 1;
                break;
            case ParliamentLegendPosition::Left:
                legendX = elemX + 8.0f;
                legendY = elemY + titleAreaHeight + 8.0f;
                legendW = legendAreaSize - 16.0f;
                legendH = elemH - titleAreaHeight - 16.0f;
                if (cols <= 0) cols = 1;
                break;
            case ParliamentLegendPosition::None:
            default:
                return;
        }

        if (cols < 1) cols = 1;
        (void)legendH;  // not used for layout in v2.0.6 — kept for future bounds checks

        // v2.0.11: uniform slot width + kLegendTextWidthRatio.
        //
        // Step 1: pre-compute each item's text width using the conservative
        // kLegendTextWidthRatio (0.55), which leaves padding beyond the
        // actual render. This prevents items from visually touching.
        //
        // Step 2: determine UNIFORM slot width = max(item widths). All
        // slots in the grid share this same width, which makes swatches
        // align across rows on identical X positions.
        //
        // Step 3: compute the block width = uniformSlotW * itemsInRow +
        // gaps, and center the block within the legend area (for
        // Bottom/Top placements). The last row may have fewer items, but
        // because each slot still has the same width, the first slot of
        // row N+1 sits at exactly the same X as the first slot of row N
        // (when both rows are full-width), or centered as a group of
        // smaller count when row N+1 is incomplete.

        ctx->SetFontSize(legendFontSize);
        ctx->SetFontWeight(FontWeight::Normal);
        TextStyle style;
        style.alignment = TextAlignment::Left;

        // Pre-compute each item's label string and its conservative width.
        struct LegendItem {
            std::string text;       // "Linke  64"
            float       textWidth;  // conservative estimated width
        };
        std::vector<LegendItem> items;
        items.reserve(partyCount);
        float maxItemTextWidth = 0.0f;
        for (int i = 0; i < partyCount; ++i) {
            const std::string& label = parties[i].ShortName.empty()
                ? parties[i].Name
                : parties[i].ShortName;
            char buf[160];
            std::snprintf(buf, sizeof(buf), "%s  %d",
                          label.c_str(), parties[i].Seats);
            LegendItem it;
            it.text = buf;
            it.textWidth =
                static_cast<float>(it.text.size()) *
                legendFontSize * kLegendTextWidthRatio;
            if (it.textWidth > maxItemTextWidth) {
                maxItemTextWidth = it.textWidth;
            }
            items.push_back(it);
        }

        // Uniform slot width = swatch + 6px gap + max text width
        const float uniformSlotW = legendSwatchSize + 6.0f + maxItemTextWidth;

        // Render
        for (int i = 0; i < partyCount; i += cols) {
            const int rowStart = i;
            const int rowEnd   = std::min(i + cols, partyCount);
            const int itemsInRow = rowEnd - rowStart;

            // Row width: uniform slots + gaps between them
            const float rowWidth =
                uniformSlotW * static_cast<float>(itemsInRow) +
                static_cast<float>(itemsInRow - 1) * kLegendSlotGap;

            // Center the row in legendW (for Bottom/Top); pin-left otherwise
            float xCursor;
            if (isHorizontal) {
                const float offset = (legendW - rowWidth) * 0.5f;
                xCursor = legendX + (offset > 0.0f ? offset : 0.0f);
            } else {
                xCursor = legendX;
            }

            const int rowIdx = i / cols;
            const float ey = legendY + rowIdx * legendRowHeight;
            const float swR = legendSwatchSize * 0.5f;
            const float swCy = ey + legendRowHeight * 0.5f;
            // v2.0.20: 0.85 lifts text so caps abbreviations sit level
            // with the swatch centre.
            const float textY = swCy - legendFontSize * 0.85f;

            for (int k = rowStart; k < rowEnd; ++k) {
                // Swatch
                Color swCol = parties[k].PartyColor;
                if (selectedPartyIndex >= 0 && k != selectedPartyIndex) {
                    swCol = ApplyAlphaFactor(swCol, dimmedAlphaFactor);
                }
                ctx->SetFillPaint(swCol);
                ctx->FillCircle(Point2Df(xCursor + swR, swCy), swR);

                // Label text
                Color textCol = legendTextColor;
                if (selectedPartyIndex >= 0 && k != selectedPartyIndex) {
                    textCol = ApplyAlphaFactor(textCol, 0.45f);
                }
                ctx->SetTextPaint(textCol);
                ctx->SetTextStyle(style);
                ctx->DrawText(items[k].text,
                    Point2Df(xCursor + legendSwatchSize + 6.0f, textY));

                // Advance cursor by uniform slot width + gap
                xCursor += uniformSlotW;
                if (k < rowEnd - 1) xCursor += kLegendSlotGap;
            }
        }
    }

// =============================================================================
// HIT TESTING — Dots
// =============================================================================

    int UltraCanvasParliamentDiagram::HitTestSeat(float mx, float my) const {
        // v2.0.4: With Bundestag (630 dots) or EU Parliament (720 dots),
        // naive O(n) hit testing on every MouseMove event causes visible
        // hover lag. Quick reject: a dot can only be hit if mx, my fall
        // within the element bounds extended by dotRadius. Within the inner
        // loop, also do cheap rejects on dx and dy before the squared
        // distance.
        const float elemX0 = static_cast<float>(GetX());
        const float elemY0 = static_cast<float>(GetY());
        const float elemX1 = elemX0 + static_cast<float>(GetWidth());
        const float elemY1 = elemY0 + static_cast<float>(GetHeight());
        const float pad = 1.5f;
        const float reach = dotRadius + pad;
        if (mx < elemX0 - reach || mx > elemX1 + reach ||
            my < elemY0 - reach || my > elemY1 + reach) {
            return -1;
        }

        const float rSq = reach * reach;
        // Reverse iteration so the visually-on-top dot wins on overlap
        for (int i = static_cast<int>(computedSeats.size()) - 1; i >= 0; --i) {
            const float dx = computedSeats[i].X - mx;
            if (dx > reach || dx < -reach) continue;     // cheap x reject
            const float dy = computedSeats[i].Y - my;
            if (dy > reach || dy < -reach) continue;     // cheap y reject
            if (dx * dx + dy * dy <= rSq) return i;
        }
        return -1;
    }

// =============================================================================
// HIT TESTING — Sectors (PieDonut)
// =============================================================================

    int UltraCanvasParliamentDiagram::HitTestSector(float mx, float my) const {
        if (computedSectors.empty()) return -1;

        // v2.0.1: single source of truth via ComputeArcGeometry()
        float cx, cy, rInner, rOuter;
        ComputeArcGeometry(cx, cy, rInner, rOuter);

        // Radial test
        const float ddx = mx - cx;
        const float ddy = -(my - cy);   // flip Y back to math space
        const float dist = std::sqrt(ddx * ddx + ddy * ddy);
        if (dist < rInner || dist > rOuter) return -1;

        // Angle in math space [0, 2π)
        float ang = std::atan2(ddy, ddx);
        if (ang < 0.0f) ang += kTwoPi;

        for (size_t i = 0; i < computedSectors.size(); ++i) {
            const auto& sec = computedSectors[i];

            // Normalize sector angles: Semicircle uses signed/reversed span
            // (start=pi, end=0), other layouts use start < end and may wrap
            // past 2*pi (Horseshoe, Circle). Normalize to [lo, hi] in math
            // convention with lo < hi.
            float lo = std::min(sec.StartAngle, sec.EndAngle);
            float hi = std::max(sec.StartAngle, sec.EndAngle);

            // Reduce both into [0, 2*pi) preserving the relationship
            while (lo >= kTwoPi) { lo -= kTwoPi; hi -= kTwoPi; }
            while (lo < 0.0f)    { lo += kTwoPi; hi += kTwoPi; }

            // If hi wraps past 2*pi, allow testA in (lo..2*pi) OR (0..hi-2*pi)
            if (hi > kTwoPi) {
                if (ang >= lo || ang <= hi - kTwoPi) {
                    return static_cast<int>(i);
                }
            } else {
                if (ang >= lo && ang <= hi) {
                    return static_cast<int>(i);
                }
            }
        }
        return -1;
    }

// =============================================================================
// EVENT HANDLING
// =============================================================================

    bool UltraCanvasParliamentDiagram::OnEvent(const UCEvent& event) {
        if (!cacheValid) ComputeSeats();

        switch (event.type) {
            case UCEventType::MouseMove: {
                const float mx = static_cast<float>(event.pointer.x);
                const float my = static_cast<float>(event.pointer.y);

                int newHoveredParty = -1;
                int newHoveredSeat  = -1;

                if (IsArcLayout() &&
                    renderMode == ParliamentRenderMode::PieDonut) {
                    const int secIdx = HitTestSector(mx, my);
                    if (secIdx >= 0) {
                        newHoveredParty =
                            computedSectors[secIdx].PartyIndex;
                    }
                } else {
                    newHoveredSeat = HitTestSeat(mx, my);
                    if (newHoveredSeat >= 0) {
                        newHoveredParty =
                            computedSeats[newHoveredSeat].PartyIndex;
                    }
                }

                if (newHoveredParty != hoveredPartyIndex ||
                    newHoveredSeat  != hoveredSeatIndex) {

                    hoveredPartyIndex = newHoveredParty;
                    hoveredSeatIndex  = newHoveredSeat;

                    if (newHoveredParty >= 0) {
                        if (onPartyHover) {
                            onPartyHover(parties[newHoveredParty]);
                        }
                        if (onSeatHover && newHoveredSeat >= 0) {
                            onSeatHover(parties[newHoveredParty],
                                        newHoveredSeat);
                        }
                    }

                    // v2.0.5: trigger an immediate repaint so the hover
                    // highlight (or its absence when crossing into a gap)
                    // is reflected on screen without waiting for the next
                    // unrelated event to drive a paint cycle.
                    RequestRedraw();
                }
                return true;
            }

            case UCEventType::MouseLeave: {
                if (hoveredPartyIndex != -1 || hoveredSeatIndex != -1) {
                    hoveredPartyIndex = -1;
                    hoveredSeatIndex  = -1;
                    RequestRedraw();  // v2.0.5
                }
                return true;
            }

            case UCEventType::MouseDown: {
                const float mx = static_cast<float>(event.pointer.x);
                const float my = static_cast<float>(event.pointer.y);

                int clickedParty = -1;
                if (IsArcLayout() &&
                    renderMode == ParliamentRenderMode::PieDonut) {
                    const int secIdx = HitTestSector(mx, my);
                    if (secIdx >= 0) {
                        clickedParty = computedSectors[secIdx].PartyIndex;
                    }
                } else {
                    const int seatIdx = HitTestSeat(mx, my);
                    if (seatIdx >= 0) {
                        clickedParty = computedSeats[seatIdx].PartyIndex;
                    }
                }

                // v2.0.5: track whether selection actually changed so we
                // only request a redraw when something visual differs.
                const int prevSelected = selectedPartyIndex;

                if (clickedParty >= 0) {
                    // Toggle: clicking the already-selected party deselects
                    if (clickedParty == selectedPartyIndex) {
                        SelectParty(-1);
                    } else {
                        SelectParty(clickedParty);
                    }
                } else {
                    // Click on empty space: deselect
                    if (selectedPartyIndex >= 0) SelectParty(-1);
                }

                if (selectedPartyIndex != prevSelected) {
                    RequestRedraw();
                }
                return true;
            }

            default:
                break;
        }
        return false;
    }

} // namespace UltraCanvas