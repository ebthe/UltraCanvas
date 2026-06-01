#include "Plugins/Charts/UltraCanvasPieChartV2.h"
#include <algorithm>
#include <cstdio>
#include <sstream>

namespace UltraCanvas {

UltraCanvasPieChartElementV2::UltraCanvasPieChartElementV2(
    const std::string& id, int x, int y, int w, int h)
    : UltraCanvasChartElementBase(id, x, y, w, h)
{
    showGrid = false;
    showAxes = false;
    showBackground = false;
}

void UltraCanvasPieChartElementV2::SetColorPalette(const std::vector<Color>& colors) {
    colorPalette = colors;
    RequestRedraw();
}

void UltraCanvasPieChartElementV2::SetBorderColor(const Color& color) {
    borderColor = color;
    RequestRedraw();
}

void UltraCanvasPieChartElementV2::SetBorderWidth(float width) {
    borderWidth = width;
    RequestRedraw();
}

void UltraCanvasPieChartElementV2::SetInnerRadius(float radius) {
    innerRadius = std::max(0.0f, std::min(0.9f, radius));
    donutEnabled = (innerRadius > 0.0f);
    RequestRedraw();
}

void UltraCanvasPieChartElementV2::SetDonutMode(bool enabled) {
    donutEnabled = enabled;
    if (enabled && innerRadius <= 0.0f) innerRadius = 0.4f;
    RequestRedraw();
}

void UltraCanvasPieChartElementV2::SetSliceExplosion(size_t index, float distance) {
    if (sliceExplosions.size() <= index) sliceExplosions.resize(index + 1, 0.0f);
    sliceExplosions[index] = std::max(0.0f, std::min(0.5f, distance));
    RequestRedraw();
}

float UltraCanvasPieChartElementV2::GetSliceExplosion(size_t index) const {
    if (index < sliceExplosions.size()) return sliceExplosions[index];
    return 0.0f;
}

void UltraCanvasPieChartElementV2::SetGlobalExplosion(float distance) {
    globalExplosion = std::max(0.0f, std::min(0.5f, distance));
    RequestRedraw();
}

void UltraCanvasPieChartElementV2::ResetExplosion() {
    sliceExplosions.clear();
    globalExplosion = 0.0f;
    RequestRedraw();
}

void UltraCanvasPieChartElementV2::SetGradientFillsEnabled(bool enabled) {
    gradientEnabled = enabled;
    RequestRedraw();
}

void UltraCanvasPieChartElementV2::SetAutoRadialGradients() {
    sliceGradients.clear();
    if (!dataSource || dataSource->GetPointCount() == 0) return;
    sliceGradients.resize(dataSource->GetPointCount());
    for (size_t i = 0; i < dataSource->GetPointCount(); ++i) {
        Color base = GetColor(i);
        Color light = base;
        light.r = (uint8_t)std::min(255, (int)(light.r * 1.4f));
        light.g = (uint8_t)std::min(255, (int)(light.g * 1.4f));
        light.b = (uint8_t)std::min(255, (int)(light.b * 1.4f));
        Color dark = base;
        dark.r = (uint8_t)(dark.r * 0.65f);
        dark.g = (uint8_t)(dark.g * 0.65f);
        dark.b = (uint8_t)(dark.b * 0.65f);
        sliceGradients[i] = {
            GradientStop(0.0f, light),
            GradientStop(0.3f, base),
            GradientStop(1.0f, dark)
        };
    }
    gradientEnabled = true;
    RequestRedraw();
}

void UltraCanvasPieChartElementV2::ClearSliceGradients() {
    sliceGradients.clear();
    gradientEnabled = false;
    RequestRedraw();
}

void UltraCanvasPieChartElementV2::SetLabelFont(const std::string& family, float size, FontWeight weight) {
    labelFontFamily = family;
    labelFontSize = size;
    labelFontWeight = weight;
    RequestRedraw();
}

void UltraCanvasPieChartElementV2::SetLeaderLineStyle(const Color& c, float w) {
    leaderLineColor = c;
    leaderLineWidth = w;
    RequestRedraw();
}

void UltraCanvasPieChartElementV2::SetLabelBackground(bool enabled, const Color& bg, float pad) {
    labelBgEnabled = enabled;
    labelBgColor = bg;
    labelBgPadding = pad;
    RequestRedraw();
}

// ===== COLOR =====

Color UltraCanvasPieChartElementV2::GetColor(size_t index) {
    if (colorPalette.empty()) return Color(128, 128, 128, 255);
    return colorPalette[index % colorPalette.size()];
}

// ===== SLICE BUILDING =====

void UltraCanvasPieChartElementV2::RebuildSlices() {
    slices.clear();
    if (!dataSource) return;
    size_t count = dataSource->GetPointCount();
    if (count == 0) return;

    double total = 0.0;
    struct Raw { size_t idx; ChartDataPoint pt; double val; };
    std::vector<Raw> raw;
    raw.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        auto p = dataSource->GetPoint(i);
        double v = (p.value != 0.0) ? p.value : p.y;
        if (v > 0.0) { raw.push_back({i, p, v}); total += v; }
    }
    if (total <= 0.0) return;

    double angle = -M_PI / 2.0;
    for (auto& r : raw) {
        double pct = r.val / total;
        double sweep = pct * 2.0 * M_PI;
        SliceData s;
        s.index = r.idx;
        s.name = r.pt.label;
        s.value = r.val;
        s.percentage = pct;
        s.startAngle = angle;
        s.endAngle = angle + sweep;
        s.midAngle = angle + sweep / 2.0;
        s.color = GetColor(s.index);
        s.explosion = (s.index < sliceExplosions.size()) ? sliceExplosions[s.index] : globalExplosion;
        slices.push_back(s);
        angle = s.endAngle;
    }
}

// ===== RENDER =====

void UltraCanvasPieChartElementV2::RenderChart(IRenderContext* ctx) {
    if (!ctx) return;
    UpdateRenderingCache();
    RebuildSlices();
    if (slices.empty()) { DrawEmptyState(ctx); return; }

    int w = GetWidth(), h = GetHeight();
    Point2Df center(w / 2.0f, h / 2.0f);
    float radius = std::min(w, h) * 0.4f;

    float maxExplosion = globalExplosion;
    for (auto& e : sliceExplosions) maxExplosion = std::max(maxExplosion, e);
    if (maxExplosion > 0.0f) radius /= (1.0f + std::min(maxExplosion, 0.5f));
    radius = std::max(20.0f, radius);

    if (chartTitle.empty()) {
        Render2D(ctx, center, radius);
    } else {
        ctx->PushState();
        ctx->SetFontFamily(labelFontFamily);
        ctx->SetFontSize(labelFontSize + 2.0f);
        ctx->SetFontWeight(FontWeight::Bold);
        ctx->SetTextPaint(labelColor);
        Size2Di ts = ctx->GetTextLineDimensions(chartTitle);
        ctx->DrawText(chartTitle, Point2Df(center.x - ts.width / 2.0f, 8.0f));
        ctx->PopState();
        Render2D(ctx, center, radius);
    }

    if (labelsEnabled && labelPosition != LabelPosition::None) {
        RenderLabels(ctx, center, radius);
    }
}

void UltraCanvasPieChartElementV2::Render2D(IRenderContext* ctx, const Point2Df& center, float radius) {
    float innerR = donutEnabled ? radius * innerRadius : 0.0f;

    for (auto& s : slices) {
        double ex = std::cos(s.midAngle) * s.explosion * radius;
        double ey = std::sin(s.midAngle) * s.explosion * radius;
        Point2Df sc(center.x + ex, center.y + ey);

        ctx->PushState();
        ctx->Translate(sc.x, sc.y);

        bool useGrad = gradientEnabled && s.index < sliceGradients.size() && !sliceGradients[s.index].empty();
        if (useGrad) {
            auto grad = ctx->CreateRadialGradientPattern(0, 0, 0, 0, 0, radius,
                                                         sliceGradients[s.index]);
            if (grad) ctx->SetFillPaint(grad);
            else { ctx->SetFillPaint(s.color); useGrad = false; }
        }
        if (!useGrad) ctx->SetFillPaint(s.color);

        if (donutEnabled && innerR > 0.0f) {
            double sweep = s.endAngle - s.startAngle;
            int segs = std::max(4, (int)std::ceil(std::fabs(sweep) / (M_PI / 36.0)));
            std::vector<Point2Df> pts;
            pts.reserve(2 * (segs + 1));
            for (int i = 0; i <= segs; ++i) {
                double a = s.startAngle + sweep * i / segs;
                pts.emplace_back(std::cos(a) * radius, std::sin(a) * radius);
            }
            for (int i = segs; i >= 0; --i) {
                double a = s.startAngle + sweep * i / segs;
                pts.emplace_back(std::cos(a) * innerR, std::sin(a) * innerR);
            }
            ctx->FillLinePath(pts);
            if (borderWidth > 0.0f && borderColor.a > 0) {
                ctx->SetStrokePaint(borderColor);
                ctx->SetStrokeWidth(borderWidth);
                ctx->DrawLinePath(pts, true);
            }
        } else {
            ctx->ClearPath();
            ctx->MoveTo(0, 0);
            ctx->Arc(0, 0, radius, s.startAngle, s.endAngle);
            ctx->LineTo(0, 0);
            ctx->ClosePath();
            ctx->FillPathPreserve();
            if (borderWidth > 0.0f && borderColor.a > 0) {
                ctx->SetStrokePaint(borderColor);
                ctx->SetStrokeWidth(borderWidth);
                ctx->StrokePathPreserve();
            }
            ctx->ClearPath();
        }
        ctx->PopState();
    }
}

void UltraCanvasPieChartElementV2::Render3D(IRenderContext* ctx, const Point2Df& center, float radius) {
    (void)ctx; (void)center; (void)radius;
    // 3D not implemented in v2 yet
}

void UltraCanvasPieChartElementV2::DrawSliceArcPath(IRenderContext* ctx, const Point2Df& center,
                                                     float radius, const SliceData& s) {
    ctx->ClearPath();
    ctx->MoveTo(center.x, center.y);
    ctx->Arc(center.x, center.y, radius, s.startAngle, s.endAngle);
    ctx->LineTo(center.x, center.y);
    ctx->ClosePath();
}

// ===== LABELS =====

std::string UltraCanvasPieChartElementV2::FormatValue(double v) {
    char buf[32];
    if (std::floor(v) == v && std::fabs(v) < 1e12)
        std::snprintf(buf, sizeof(buf), "%.0f", v);
    else
        std::snprintf(buf, sizeof(buf), "%g", v);
    return buf;
}

std::string UltraCanvasPieChartElementV2::FormatPercent(double pct) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.1f%%", pct * 100.0);
    return buf;
}

std::string UltraCanvasPieChartElementV2::FormatLabel(const SliceData& s) {
    switch (labelContent) {
        case LabelContent::Name:           return s.name;
        case LabelContent::Value:          return FormatValue(s.value);
        case LabelContent::Percentage:     return FormatPercent(s.percentage);
        case LabelContent::NameValue:      return s.name + ": " + FormatValue(s.value);
        case LabelContent::NamePercentage: return s.name + ": " + FormatPercent(s.percentage);
        case LabelContent::ValuePercentage:return FormatValue(s.value) + " (" + FormatPercent(s.percentage) + ")";
        case LabelContent::All:
            return s.name + "\n" + FormatValue(s.value) + " (" + FormatPercent(s.percentage) + ")";
        case LabelContent::None: break;
    }
    return "";
}

void UltraCanvasPieChartElementV2::RenderLabels(IRenderContext* ctx, const Point2Df& center, float radius) {
    if (slices.empty()) return;

    ctx->PushState();
    ctx->SetFontFamily(labelFontFamily);
    ctx->SetFontSize(labelFontSize);
    ctx->SetFontWeight(labelFontWeight);
    ctx->SetTextPaint(labelColor);

    struct Item {
        const SliceData* slice;
        std::string text;
        LabelPosition resolved;
        Point2Df anchor;
        Point2Df textPos;
        Size2Di textSize;
        bool rightSide;
    };
    std::vector<Item> items;

    for (auto& s : slices) {
        Item it;
        it.slice = &s;
        it.text = FormatLabel(s);
        if (it.text.empty()) continue;

        it.textSize = ctx->GetTextLineDimensions(it.text);

        LabelPosition pos = labelPosition;
        double sweep = s.endAngle - s.startAngle;
        if (pos == LabelPosition::Auto)
            pos = (sweep > 25.0 * M_PI / 180.0) ? LabelPosition::Inside : LabelPosition::Outside;

        it.resolved = pos;
        it.rightSide = std::cos(s.midAngle) >= 0.0;

        double ex = std::cos(s.midAngle) * s.explosion * radius;
        double ey = std::sin(s.midAngle) * s.explosion * radius;

        if (pos == LabelPosition::Inside) {
            float labelR = donutEnabled ? (radius * innerRadius + radius) * 0.5f : radius * 0.5f;
            it.anchor = Point2Df(center.x + ex + labelR * std::cos(s.midAngle),
                                 center.y + ey + labelR * std::sin(s.midAngle));
        } else {
            float rf = (pos == LabelPosition::Edge) ? 1.0f : 1.18f;
            it.anchor = Point2Df(center.x + ex + radius * rf * std::cos(s.midAngle),
                                 center.y + ey + radius * rf * std::sin(s.midAngle));
        }

        if (pos == LabelPosition::Inside) {
            it.textPos = Point2Df(it.anchor.x - it.textSize.width / 2.0f,
                                  it.anchor.y - it.textSize.height / 2.0f);
        } else if (pos == LabelPosition::Edge) {
            if (it.rightSide)
                it.textPos = Point2Df(it.anchor.x + 2.0f, it.anchor.y - it.textSize.height / 2.0f);
            else
                it.textPos = Point2Df(it.anchor.x - 2.0f - it.textSize.width, it.anchor.y - it.textSize.height / 2.0f);
        } else {
            float elbowX = it.rightSide ? it.anchor.x + 6.0f : it.anchor.x - 6.0f - it.textSize.width;
            it.textPos = Point2Df(elbowX, it.anchor.y - it.textSize.height / 2.0f);
        }

        items.push_back(it);
    }

    // Resolve overlaps for outside labels
    auto resolveOverlaps = [&](bool right) {
        std::vector<size_t> idxs;
        for (size_t i = 0; i < items.size(); ++i)
            if (items[i].resolved == LabelPosition::Outside && items[i].rightSide == right)
                idxs.push_back(i);
        std::sort(idxs.begin(), idxs.end(), [&](size_t a, size_t b) {
            return items[a].textPos.y < items[b].textPos.y;
        });
        const float gap = 2.0f;
        for (size_t k = 1; k < idxs.size(); ++k) {
            auto& prev = items[idxs[k-1]];
            auto& cur  = items[idxs[k]];
            float prevBottom = prev.textPos.y + prev.textSize.height;
            if (cur.textPos.y < prevBottom + gap)
                cur.textPos.y = prevBottom + gap;
        }
        for (size_t k = idxs.size(); k-- > 1;) {
            auto& cur  = items[idxs[k-1]];
            auto& next = items[idxs[k]];
            float curBottom = cur.textPos.y + cur.textSize.height;
            if (curBottom + gap > next.textPos.y)
                cur.textPos.y = next.textPos.y - gap - cur.textSize.height;
        }
    };
    resolveOverlaps(true);
    resolveOverlaps(false);

    // Draw leader lines + labels
    for (auto& it : items) {
        if (it.resolved == LabelPosition::Outside && leaderLines) {
            float lx = it.rightSide ? it.anchor.x + 6.0f : it.anchor.x - 6.0f;
            ctx->SetStrokePaint(leaderLineColor);
            ctx->SetStrokeWidth(leaderLineWidth);
            ctx->DrawLine(it.anchor, Point2Df(lx, it.anchor.y));
            ctx->DrawLine(Point2Df(lx, it.anchor.y),
                          Point2Df(lx, it.textPos.y + it.textSize.height / 2.0f));
            ctx->DrawLine(Point2Df(lx, it.textPos.y + it.textSize.height / 2.0f),
                          Point2Df(it.textPos.x, it.textPos.y + it.textSize.height / 2.0f));
        }

        if (labelBgEnabled) {
            ctx->SetFillPaint(labelBgColor);
            ctx->FillRectangle(Rect2Df(it.textPos.x - labelBgPadding, it.textPos.y - labelBgPadding,
                                       it.textSize.width + labelBgPadding * 2,
                                       it.textSize.height + labelBgPadding * 2));
        }

        ctx->SetTextPaint(labelColor);
        ctx->DrawText(it.text, it.textPos);
    }
    ctx->PopState();
}

// ===== HIT TEST =====

size_t UltraCanvasPieChartElementV2::HitTestSlice(const Point2Df& center, float radius, const Point2Di& mouse) {
    float dx = mouse.x - center.x;
    float dy = mouse.y - center.y;
    float dist = std::sqrt(dx * dx + dy * dy);
    float innerR = donutEnabled ? radius * innerRadius : 0.0f;
    if (dist > radius || dist < innerR) return SIZE_MAX;

    double angle = std::atan2(dy, dx);
    if (angle < -M_PI / 2.0) angle += 2.0 * M_PI;

    for (size_t i = 0; i < slices.size(); ++i) {
        double s = slices[i].startAngle - 0.01;
        double e = slices[i].endAngle + 0.01;
        if (angle >= s && angle <= e) return i;
    }
    return SIZE_MAX;
}

bool UltraCanvasPieChartElementV2::HandleChartMouseMove(const Point2Di& mousePos) {
    if (!dataSource || !enableTooltips) return false;

    int w = GetWidth(), h = GetHeight();
    Point2Df center(w / 2.0f, h / 2.0f);
    float radius = std::min(w, h) * 0.4f;

    size_t idx = HitTestSlice(center, radius, mousePos);
    if (idx != SIZE_MAX) {
        auto& s = slices[idx];
        auto pt = dataSource->GetPoint(s.index);
        ShowChartPointTooltip(mousePos, pt, s.index);
        return true;
    }
    if (isTooltipActive) HideTooltip();
    return false;
}

// ===== FACTORY =====

std::shared_ptr<UltraCanvasPieChartElementV2> CreatePieChartElementV2(
    const std::string& id, int x, int y, int w, int h)
{
    return std::make_shared<UltraCanvasPieChartElementV2>(id, x, y, w, h);
}

} // namespace UltraCanvas