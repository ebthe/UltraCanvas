#include "UltraCanvasDemo.h"
#include "Plugins/Charts/UltraCanvasPieChartV2.h"
#include "Plugins/Charts/UltraCanvasPieChart.h"
#include "UltraCanvasLabel.h"
#include "UltraCanvasSlider.h"
#include "UltraCanvasContainer.h"
#include <sstream>

namespace UltraCanvas {

static std::shared_ptr<ChartDataVector> GenData1() {
    auto d = std::make_shared<ChartDataVector>();
    d->LoadFromArray({
        ChartDataPoint(1, 350, 0, "Product A", 350),
        ChartDataPoint(2, 280, 0, "Product B", 280),
        ChartDataPoint(3, 210, 0, "Product C", 210),
        ChartDataPoint(4, 160, 0, "Product D", 160),
        ChartDataPoint(5, 120, 0, "Others", 120),
    });
    return d;
}

static std::shared_ptr<ChartDataVector> GenData2() {
    auto d = std::make_shared<ChartDataVector>();
    d->LoadFromArray({
        ChartDataPoint(1, 4500, 0, "Salaries", 4500),
        ChartDataPoint(2, 2300, 0, "Marketing", 2300),
        ChartDataPoint(3, 1800, 0, "R&D", 1800),
        ChartDataPoint(4, 1200, 0, "Operations", 1200),
        ChartDataPoint(5, 800, 0, "Infrastructure", 800),
        ChartDataPoint(6, 400, 0, "Other", 400),
    });
    return d;
}

static std::shared_ptr<ChartDataVector> GenData3() {
    auto d = std::make_shared<ChartDataVector>();
    d->LoadFromArray({
        ChartDataPoint(1, 2800, 0, "North America", 2800),
        ChartDataPoint(2, 2200, 0, "Europe", 2200),
        ChartDataPoint(3, 1900, 0, "Asia Pacific", 1900),
        ChartDataPoint(4, 850, 0, "Latin America", 850),
        ChartDataPoint(5, 450, 0, "Middle East", 450),
        ChartDataPoint(6, 300, 0, "Africa", 300),
    });
    return d;
}

std::shared_ptr<UltraCanvasUIElement> UltraCanvasDemoApplication::CreatePieChartV2Examples() {
    auto root = std::make_shared<UltraCanvasContainer>("PieV2Root", 0, 0, 1000, 900);

    auto title = std::make_shared<UltraCanvasLabel>("PieV2Title", 0, 20, 960, 30);
    title->SetText("Pie Chart: V1 (left) vs V2 (right) — Same data, side by side");
    title->SetFontSize(16);
    title->SetFontWeight(FontWeight::Bold);
    title->SetAlignment(TextAlignment::Center);
    root->AddChild(title);

    int row = 0;
    auto makeRow = [&](const std::string& label, std::shared_ptr<ChartDataVector> data,
                       std::function<void(std::shared_ptr<UltraCanvasPieChartElement>)> setupV1,
                       std::function<void(std::shared_ptr<UltraCanvasPieChartElementV2>)> setupV2)
    {
        int y = 50 + row * 280;
        auto lbl = std::make_shared<UltraCanvasLabel>("L" + std::to_string(row), 0, y, 960, 20);
        lbl->SetText(label);
        lbl->SetFontSize(12);
        lbl->SetFontWeight(FontWeight::Bold);
        root->AddChild(lbl);

        // V1
        auto v1 = CreatePieChartElement("v1_" + std::to_string(row), 20, y + 25, 460, 240);
        v1->SetDataSource(data);
        setupV1(v1);
        root->AddChild(v1);

        // V2
        auto v2 = CreatePieChartElementV2("v2_" + std::to_string(row), 520, y + 25, 460, 240);
        v2->SetDataSource(data);
        setupV2(v2);
        root->AddChild(v2);

        auto sep = std::make_shared<UltraCanvasLabel>("Sep" + std::to_string(row), 500, y + 25, 20, 240);
        sep->SetText("||");
        sep->SetFontSize(18);
        sep->SetTextColor(Color(200, 200, 200, 255));
        sep->SetAlignment(TextAlignment::Center);
        root->AddChild(sep);

        ++row;
    };

    makeRow("Basic Pie — inside labels",
            GenData1(),
            [](auto c) {
                c->SetLabelContent(LabelContent::NamePercentage);
                c->SetLabelPosition(LabelPosition::Auto);
                c->SetBorderColor(Colors::White);
                c->SetBorderWidth(2.0f);
            },
            [](auto c) {
                c->SetLabelContent(UltraCanvasPieChartElementV2::LabelContent::NamePercentage);
                c->SetLabelPosition(UltraCanvasPieChartElementV2::LabelPosition::Auto);
                c->SetBorderColor(Colors::White);
                c->SetBorderWidth(2.0f);
            });

    makeRow("Donut — outside labels + leader lines",
            GenData2(),
            [](auto c) {
                c->SetDonutMode(true);
                c->SetInnerRadius(0.45f);
                c->SetLabelContent(LabelContent::NameValue);
                c->SetLabelPosition(LabelPosition::Outside);
                c->SetLeaderLinesEnabled(true);
                c->SetBorderColor(Colors::White);
                c->SetBorderWidth(2.0f);
            },
            [](auto c) {
                c->SetDonutMode(true);
                c->SetInnerRadius(0.45f);
                c->SetLabelContent(UltraCanvasPieChartElementV2::LabelContent::NameValue);
                c->SetLabelPosition(UltraCanvasPieChartElementV2::LabelPosition::Outside);
                c->SetLeaderLinesEnabled(true);
                c->SetBorderColor(Colors::White);
                c->SetBorderWidth(2.0f);
            });

    makeRow("Pie — radial gradients, no border",
            GenData3(),
            [](auto c) {
                c->SetAutoRadialGradients();
                c->SetLabelContent(LabelContent::NamePercentage);
                c->SetLabelPosition(LabelPosition::Auto);
                c->SetBorderWidth(0.0f);
            },
            [](auto c) {
                c->SetGradientFillsEnabled(true);
                c->SetAutoRadialGradients();
                c->SetLabelContent(UltraCanvasPieChartElementV2::LabelContent::NamePercentage);
                c->SetLabelPosition(UltraCanvasPieChartElementV2::LabelPosition::Auto);
                c->SetBorderWidth(0.0f);
            });

    auto note = std::make_shared<UltraCanvasLabel>("PieV2Note", 20, row * 280 + 60, 960, 20);
    note->SetText("V1 = UltraCanvasPieChartElement (comprehensive), V2 = UltraCanvasPieChartElementV2 (simplified rewrite)");
    note->SetFontSize(10);
    note->SetTextColor(Color(120, 120, 120, 255));
    note->SetAlignment(TextAlignment::Center);
    root->AddChild(note);

    return root;
}

} // namespace UltraCanvas