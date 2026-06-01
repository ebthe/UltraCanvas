#pragma once

#include "UltraCanvasChartElementBase.h"
#include "UltraCanvasCommonTypes.h"
#include "UltraCanvasRenderContext.h"
#include <vector>
#include <string>
#include <memory>
#include <cmath>

namespace UltraCanvas {

class UltraCanvasPieChartElementV2 : public UltraCanvasChartElementBase {
public:
    enum class LabelPosition {
        None, Inside, Outside, Edge, Auto
    };

    enum class LabelContent {
        None, Name, Value, Percentage, NameValue, NamePercentage, ValuePercentage, All
    };

    UltraCanvasPieChartElementV2(const std::string& id, int x, int y, int w, int h);

    void SetColorPalette(const std::vector<Color>& colors);
    void SetBorderColor(const Color& color);
    void SetBorderWidth(float width);

    void SetInnerRadius(float radius);
    float GetInnerRadius() const { return innerRadius; }
    void SetDonutMode(bool enabled);
    bool IsDonutMode() const { return donutEnabled; }

    void SetSliceExplosion(size_t index, float distance);
    float GetSliceExplosion(size_t index) const;
    void SetGlobalExplosion(float distance);
    float GetGlobalExplosion() const { return globalExplosion; }
    void ResetExplosion();

    void SetGradientFillsEnabled(bool enabled);
    bool GetGradientFillsEnabled() const { return gradientEnabled; }
    void SetAutoRadialGradients();
    void ClearSliceGradients();

    void SetLabelsEnabled(bool enabled) { labelsEnabled = enabled; RequestRedraw(); }
    bool GetLabelsEnabled() const { return labelsEnabled; }
    void SetLabelPosition(LabelPosition p) { labelPosition = p; RequestRedraw(); }
    LabelPosition GetLabelPosition() const { return labelPosition; }
    void SetLabelContent(LabelContent c) { labelContent = c; RequestRedraw(); }
    LabelContent GetLabelContent() const { return labelContent; }
    void SetLabelFont(const std::string& family, float size, FontWeight weight = FontWeight::Normal);
    void SetLabelTextColor(const Color& c) { labelColor = c; RequestRedraw(); }
    void SetLeaderLinesEnabled(bool on) { leaderLines = on; RequestRedraw(); }
    bool GetLeaderLinesEnabled() const { return leaderLines; }
    void SetLeaderLineStyle(const Color& c, float w);
    void SetLabelBackground(bool enabled, const Color& bg = Color(255,255,255,200), float pad = 2.0f);

    void RenderChart(IRenderContext* ctx) override;
    bool HandleChartMouseMove(const Point2Di& mousePos) override;

private:
    struct SliceData {
        size_t index;
        std::string name;
        double value;
        double percentage;
        double startAngle;
        double endAngle;
        double midAngle;
        Color color;
        float explosion;
    };

    std::vector<Color> colorPalette = {
        Color(54, 162, 235, 255), Color(255, 99, 132, 255),
        Color(255, 205, 86, 255), Color(75, 192, 192, 255),
        Color(153, 102, 255, 255), Color(255, 159, 64, 255),
        Color(199, 199, 199, 255), Color(83, 102, 255, 255)
    };
    Color borderColor{255, 255, 255, 255};
    float borderWidth = 2.0f;

    float innerRadius = 0.0f;
    bool donutEnabled = false;
    std::vector<float> sliceExplosions;
    float globalExplosion = 0.0f;

    bool gradientEnabled = false;
    std::vector<std::vector<GradientStop>> sliceGradients;

    bool labelsEnabled = true;
    LabelPosition labelPosition = LabelPosition::Auto;
    LabelContent labelContent = LabelContent::NamePercentage;
    std::string labelFontFamily = "Arial";
    float labelFontSize = 11.0f;
    FontWeight labelFontWeight = FontWeight::Normal;
    Color labelColor{0, 0, 0, 255};
    bool leaderLines = true;
    Color leaderLineColor{128, 128, 128, 255};
    float leaderLineWidth = 1.0f;
    bool labelBgEnabled = false;
    Color labelBgColor{255, 255, 255, 200};
    float labelBgPadding = 2.0f;

    std::vector<SliceData> slices;

    void RebuildSlices();
    Color GetColor(size_t index);
    void Render2D(IRenderContext* ctx, const Point2Df& center, float radius);
    void Render3D(IRenderContext* ctx, const Point2Df& center, float radius);
    void DrawSlice2D(IRenderContext* ctx, const Point2Df& center, float radius, const SliceData& s);
    void DrawSliceArcPath(IRenderContext* ctx, const Point2Df& center, float radius, const SliceData& s);
    void RenderLabels(IRenderContext* ctx, const Point2Df& center, float radius);
    std::string FormatLabel(const SliceData& s);
    std::string FormatValue(double v);
    std::string FormatPercent(double pct);
    size_t HitTestSlice(const Point2Df& center, float radius, const Point2Di& mouse);
};

std::shared_ptr<UltraCanvasPieChartElementV2> CreatePieChartElementV2(
    const std::string& id, int x, int y, int w, int h);

} // namespace UltraCanvas