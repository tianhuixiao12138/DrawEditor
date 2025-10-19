#include "formatpanel.h"
#include "shape.h"           
#include "connectionline.h" 
#include <QVBoxLayout>
#include <QFormLayout>
#include <QColorDialog>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QFontComboBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QDebug>
#include <QLineEdit> 
#include <QTimer>
#include <QEvent> 
#include <QApplication>

FormatPanel::FormatPanel(QWidget* parent)
    : QWidget(parent)
    // Initialize all pointer members to nullptr for safety
    , titleLabel_ptr(nullptr)
    , borderGroup_ptr(nullptr), fillGroup_ptr(nullptr), lineGroup_ptr(nullptr), textGroup_ptr(nullptr), canvasGroup_ptr(nullptr)
    , labelBorderColor_ptr(nullptr), labelBorderWidth_ptr(nullptr), labelBorderStyle_ptr(nullptr), labelShapeWidth_ptr(nullptr), labelShapeHeight_ptr(nullptr)
    , borderButton_ptr(nullptr), borderWidthCombo_ptr(nullptr), borderStyleCombo_ptr(nullptr), shapeWidthSpin_ptr(nullptr), shapeHeightSpin_ptr(nullptr)
    , labelFillColor_ptr(nullptr), fillCheckBox_ptr(nullptr), fillButton_ptr(nullptr)
    , labelLineColor_ptr(nullptr), labelLineThickness_ptr(nullptr), labelLineStyleUI_ptr(nullptr), labelLineEndpointStyle_ptr(nullptr)
    , lineColorButton_ptr(nullptr), lineWidthCombo_ptr(nullptr), lineStyleCombo_ptr(nullptr), arrowStyleCombo_ptr(nullptr)
    , labelTextCorlor_ptr(nullptr), labelTextFont_ptr(nullptr), labelTextSize_ptr(nullptr)
    , textButton_ptr(nullptr), fontCombo_ptr(nullptr), fontSizeSpin_ptr(nullptr)
    , labelCanvasWidth_ptr(nullptr), labelCanvasHeight_ptr(nullptr), labelCanvasZoom_ptr(nullptr)
    , canvasWidthSpin_ptr(nullptr), canvasHeightSpin_ptr(nullptr), canvasZoomSpin_ptr(nullptr)
{
    QString path = QCoreApplication::applicationDirPath()+"/icons";
    QString upArrowUrl = " url("+path + "arrow_up_light.svg"+")";
    QString upArrowUrlCorrected = "url(" + path + "/arrow_up_light.svg" + ")";
    QString downArrowUrlCorrected = "url(" + path + "/arrow_down_light.svg" + ")";
    QString checkLightCorrected = "url(" + path + "/check_light.svg" + ")"; 
    QString styleSheetString= QString(
        "QWidget {"
        "    background-color: #1E293B;" // 面板背景色
        "    color: #E2E8F0;"            // 默认文字颜色 (浅灰)
        "    border: none;"
        "}"
        // --- QGroupBox 样式 ---
        "QGroupBox {"
        "    color: #E2E8F0;" // GroupBox 标题颜色
        "    font-weight: bold;"
        "    border: 1px solid #334155;" // GroupBox 边框颜色
        "    border-radius: 6px;"
        "    margin-top: 10px;" // 为标题留出空间
        "    padding: 15px 5px 5px 5px;" // 增加上内边距
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    subcontrol-position: top left;" // 标题位置
        "    left: 10px;"
        "    padding: 0 3px 0 3px;"
        "    background-color: #1E293B;" // 标题背景与面板相同
        "}"
        // --- QLabel 样式 (如果需要覆盖默认) ---
        "QLabel {"
        "    color: #94A3B8;" // 标签使用稍暗的灰色
        "    background-color: transparent;" // 确保背景透明
        "    padding-top: 4px;" // 垂直居中对齐
        "}"
        // --- 输入控件通用样式 ---
        "QAbstractSpinBox, QLineEdit, QComboBox {" // QDoubleSpinBox 继承 QAbstractSpinBox
        "    background-color: #28354C;" // 控件背景
        "    color: #E2E8F0;" // 控件文字颜色
        "    border: 1px solid #475569;" // 控件边框
        "    border-radius: 4px;"
        "    padding: 4px 8px;" // 内边距
        "    min-height: 24px;" // 统一最小高度
        "}"
        "QAbstractSpinBox:focus, QLineEdit:focus, QComboBox:focus {"
        "    border-color: #3B82F6;" // 聚焦时蓝色边框
        "}"
        // --- QSpinBox/QDoubleSpinBox 箭头样式 ---
        "QSpinBox::up-button, QDoubleSpinBox::up-button {"
        "    subcontrol-origin: border;"
        "    subcontrol-position: top right;"
        "    width: 16px;"
        "    image: %1;" // 需要浅色向上箭头图标
        "    background-color: #334155;"
        "    border-left: 1px solid #475569;"
        "    border-top-right-radius: 4px;"
        "    border-bottom-right-radius: 0px;"
        "}"
        "QSpinBox::down-button, QDoubleSpinBox::down-button {"
        "    subcontrol-origin: border;"
        "    subcontrol-position: bottom right;"
        "    width: 16px;"
        "    image: %2;" // 需要浅色向下箭头图标
        "    background-color: #334155;"
        "    border-left: 1px solid #475569;"
        "    border-top-right-radius: 0px;"
        "    border-bottom-right-radius: 4px;"
        "}"
        "QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover,"
        "QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover {"
        "    background-color: #475569;"
        "}"
        // --- QComboBox 下拉箭头 ---
        "QComboBox::drop-down {"
        "    subcontrol-origin: padding;"
        "    subcontrol-position: top right;"
        "    width: 20px;"
        "    border-left: 1px solid #475569;"
        "    border-top-right-radius: 4px;"
        "    border-bottom-right-radius: 4px;"
        "}"
        "QComboBox::down-arrow {"
        "    image: %2;" // 使用与 SpinBox 相同的图标
        "    width: 12px;"
        "    height: 12px;"
        "}"
        "QComboBox QAbstractItemView {" /* 下拉列表 */
        "    background-color: #28354C;"
        "    color: #E2E8F0;"
        "    border: 1px solid #475569;"
        "    selection-background-color: #3B82F6;" // 选中项背景
        "    selection-color: #FFFFFF;" // 选中项文字
        "    padding: 4px;"
        "}"
        // --- QCheckBox 样式 ---
        "QCheckBox {"
        "    color: #E2E8F0;" // 复选框文字颜色
        "    spacing: 8px;" // 文字和指示器间距
        "    background: transparent;"
        "}"
        "QCheckBox::indicator {"
        "    width: 16px;"
        "    height: 16px;"
        "    border: 1px solid #475569;"
        "    border-radius: 3px;"
        "    background-color: #28354C;"
        "}"
        "QCheckBox::indicator:checked {"
        "    background-color: #3B82F6;" // 选中时蓝色
        "    border-color: #3B82F6;"
        //"    image: url(:/icons/check_light.svg);" // 需要白色勾号图标
        "}"
        "QCheckBox::indicator:unchecked:hover {"
        "    border-color: #60A5FA;" // 悬停边框
        "}"
        "QCheckBox::indicator:checked:hover {"
        "    background-color: #60A5FA;" // 选中时悬停颜色
        "    border-color: #60A5FA;"
        "}"
        "QCheckBox::indicator:indeterminate {" // 部分选中状态
        "    background-color: #475569;"
        "    border-color: #60A5FA;"
        //"     image: url(:/icons/minus_light.svg);" // 需要白色减号图标
        "}"
        // --- QPushButton (用于颜色块) ---
        // 样式将在 updateButtonColor 中设置
    ).arg(upArrowUrlCorrected, downArrowUrlCorrected /*, ... 其他图标的 URL */);
    setupUI();
    this->setStyleSheet(styleSheetString);
    // 初始状态禁用所有组，直到有东西被选中 或 显示画布属性
    // setEnabled(false); // 先不禁用，让画布属性一直可见
    // 稍后根据是否有选中项来禁用非画布组
    if (borderGroup_ptr) borderGroup_ptr->setEnabled(false);
    if (fillGroup_ptr) fillGroup_ptr->setEnabled(false);
    if (lineGroup_ptr) lineGroup_ptr->setEnabled(false);
    if (textGroup_ptr) textGroup_ptr->setEnabled(false);
    if (canvasGroup_ptr) canvasGroup_ptr->setEnabled(true);
    retranslateUi();
}
void FormatPanel::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi(); // <--- 调用 retranslateUi 更新文本
    }
    QWidget::changeEvent(event); // 调用基类的实现
}

void FormatPanel::retranslateUi() {
    qDebug() << "FormatPanel: Retranslating UI...";

    if (titleLabel_ptr) titleLabel_ptr->setText(tr("Format Properties"));

    if (borderGroup_ptr) borderGroup_ptr->setTitle(tr("Border"));
    if (labelBorderColor_ptr) labelBorderColor_ptr->setText(tr("Color:"));
    if (labelBorderWidth_ptr) labelBorderWidth_ptr->setText(tr("Width:"));
    if (labelBorderStyle_ptr) labelBorderStyle_ptr->setText(tr("Style:"));
    if (labelShapeWidth_ptr) labelShapeWidth_ptr->setText(tr("Shape Width:"));
    if (labelShapeHeight_ptr) labelShapeHeight_ptr->setText(tr("Shape Height:"));

    if (fillGroup_ptr) fillGroup_ptr->setTitle(tr("Fill"));
    if (fillCheckBox_ptr) fillCheckBox_ptr->setText(tr("No Fill"));
    if (labelFillColor_ptr) labelFillColor_ptr->setText(tr("Color:"));

    if (lineGroup_ptr) lineGroup_ptr->setTitle(tr("Line"));
    if (labelLineColor_ptr) labelLineColor_ptr->setText(tr("Color:"));
    if (labelLineThickness_ptr) labelLineThickness_ptr->setText(tr("Thickness:"));
    if (labelLineStyleUI_ptr) labelLineStyleUI_ptr->setText(tr("Style:"));
    if (labelLineEndpointStyle_ptr) labelLineEndpointStyle_ptr->setText(tr("Endpoint Style:"));

    if (textGroup_ptr) textGroup_ptr->setTitle(tr("Text"));
    if (labelTextCorlor_ptr) labelTextCorlor_ptr->setText(tr("Color:"));
    if (labelTextFont_ptr) labelTextFont_ptr->setText(tr("Font:"));
    if (labelTextSize_ptr) labelTextSize_ptr->setText(tr("Size:"));

    if (canvasGroup_ptr) canvasGroup_ptr->setTitle(tr("Canvas"));
    if (labelCanvasWidth_ptr) labelCanvasWidth_ptr->setText(tr("Width:"));
    if (labelCanvasHeight_ptr) labelCanvasHeight_ptr->setText(tr("Height:"));
    if (labelCanvasZoom_ptr) labelCanvasZoom_ptr->setText(tr("Zoom:"));

    // Retranslate ComboBox items
    if (borderWidthCombo_ptr) {
        QString current = borderWidthCombo_ptr->currentText();
        borderWidthCombo_ptr->blockSignals(true);
        borderWidthCombo_ptr->clear();
        borderWidthCombo_ptr->addItem(tr("No Border"));
        borderWidthCombo_ptr->addItem("0.5");
        for (double i = 1.0; i <= 10; i += 0.5) { borderWidthCombo_ptr->addItem(QString::number(i, 'f', 1)); }
        int idx = borderWidthCombo_ptr->findText(current);
        borderWidthCombo_ptr->setCurrentIndex(idx != -1 ? idx : 0);
        borderWidthCombo_ptr->blockSignals(false);
    }

    if (borderStyleCombo_ptr) {
        QString current = borderStyleCombo_ptr->currentText();
        borderStyleMap_data.clear();
        borderStyleCombo_ptr->blockSignals(true);
        borderStyleCombo_ptr->clear();
        borderStyleMap_data[tr("Solid Line")] = Qt::SolidLine;
        borderStyleMap_data[tr("Dashed Line")] = Qt::DashLine;
        borderStyleMap_data[tr("Dotted Line")] = Qt::DotLine;
        borderStyleMap_data[tr("Dash-Dot Line")] = Qt::DashDotLine;
        borderStyleMap_data[tr("Dash-Dot-Dot Line")] = Qt::DashDotDotLine;
        borderStyleCombo_ptr->addItem(tr(NO_BORDER_STYLE_TEXT));
        borderStyleCombo_ptr->addItems(borderStyleMap_data.keys());
        int idx = borderStyleCombo_ptr->findText(current);
        borderStyleCombo_ptr->setCurrentIndex(idx != -1 ? idx : 0);
        borderStyleCombo_ptr->blockSignals(false);
    }

    if (lineStyleCombo_ptr) {
        QString current = lineStyleCombo_ptr->currentText();
        lineStyleMap_data.clear();
        lineStyleCombo_ptr->blockSignals(true);
        lineStyleCombo_ptr->clear();
        lineStyleMap_data[tr("Solid Line")] = Qt::SolidLine;
        lineStyleMap_data[tr("Dashed Line")] = Qt::DashLine;
        lineStyleMap_data[tr("Dotted Line")] = Qt::DotLine;
        lineStyleMap_data[tr("Dash-Dot Line")] = Qt::DashDotLine;
        lineStyleMap_data[tr("Dash-Dot-Dot Line")] = Qt::DashDotDotLine;
        lineStyleCombo_ptr->addItems(lineStyleMap_data.keys());
        int idx = lineStyleCombo_ptr->findText(current);
        lineStyleCombo_ptr->setCurrentIndex(idx != -1 ? idx : 0);
        lineStyleCombo_ptr->blockSignals(false);
    }

    if (arrowStyleCombo_ptr) {
        QString current = arrowStyleCombo_ptr->currentText();
        arrowStyleCombo_ptr->blockSignals(true);
        arrowStyleCombo_ptr->clear();
        arrowStyleCombo_ptr->addItem(tr("Arrow"));
        arrowStyleCombo_ptr->addItem(tr("Straight Line"));
        int idx = arrowStyleCombo_ptr->findText(current);
        arrowStyleCombo_ptr->setCurrentIndex(idx != -1 ? idx : 0);
        arrowStyleCombo_ptr->blockSignals(false);
    }

    // Update tooltips that might need retranslation (if they are static)
    // Dynamic tooltips (like "Color is inconsistent") are handled in updateSelected... methods.
}


void FormatPanel::setupUI() {
    // This QVBoxLayout will be set as the main layout for the FormatPanel widget
    QVBoxLayout* mainLayout = new QVBoxLayout(this); // 'this' sets FormatPanel as parent of layout
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(15);

    titleLabel_ptr = new QLabel(tr("Format Properties"));
    titleLabel_ptr->setAlignment(Qt::AlignCenter);
    titleLabel_ptr->setStyleSheet(
        "border: none; font-size: 18px; font-weight: bold; color: #FFFFFF; "
        "background-color: #475569; padding: 8px; border-radius: 4px;"
    );
    mainLayout->addWidget(titleLabel_ptr);

    // Border Group
    borderGroup_ptr = new QGroupBox(tr("Border"));
    QFormLayout* borderLayout = new QFormLayout();
    borderLayout->setSpacing(8);
    borderLayout->setContentsMargins(10, 5, 10, 5);

    labelBorderColor_ptr = new QLabel(tr("Color:"));
    borderButton_ptr = new QPushButton();
    borderButton_ptr->setFixedWidth(120);
    updateButtonColor(borderButton_ptr, Qt::black);
    connect(borderButton_ptr, &QPushButton::clicked, this, &FormatPanel::changeBorderColorTriggered);
    borderLayout->addRow(labelBorderColor_ptr, borderButton_ptr);

    labelBorderWidth_ptr = new QLabel(tr("Width:"));
    borderWidthCombo_ptr = new QComboBox();
    borderWidthCombo_ptr->setFixedWidth(120);
    borderWidthCombo_ptr->addItem(tr("No Border"));
    borderWidthCombo_ptr->addItem("0.5");
    for (double i = 1.0; i <= 10; i += 0.5) { borderWidthCombo_ptr->addItem(QString::number(i, 'f', 1)); }
    borderLayout->addRow(labelBorderWidth_ptr, borderWidthCombo_ptr);

    labelBorderStyle_ptr = new QLabel(tr("Style:"));
    borderStyleCombo_ptr = new QComboBox();
    borderStyleCombo_ptr->setFixedWidth(120);
    borderStyleCombo_ptr->setFixedHeight(28);
    borderStyleMap_data[tr("Solid Line")] = Qt::SolidLine;
    borderStyleMap_data[tr("Dashed Line")] = Qt::DashLine;
    borderStyleMap_data[tr("Dotted Line")] = Qt::DotLine;
    borderStyleMap_data[tr("Dash-Dot Line")] = Qt::DashDotLine;
    borderStyleMap_data[tr("Dash-Dot-Dot Line")] = Qt::DashDotDotLine;
    borderStyleCombo_ptr->addItem(tr(NO_BORDER_STYLE_TEXT));
    borderStyleCombo_ptr->addItems(borderStyleMap_data.keys());
    borderLayout->addRow(labelBorderStyle_ptr, borderStyleCombo_ptr);

    labelShapeWidth_ptr = new QLabel(tr("Shape Width:"));
    shapeWidthSpin_ptr = new QSpinBox(this);
    shapeWidthSpin_ptr->setFixedWidth(120);
    shapeWidthSpin_ptr->setRange(10, 5000);
    shapeWidthSpin_ptr->setSuffix(" px");
    shapeWidthSpin_ptr->setButtonSymbols(QAbstractSpinBox::NoButtons);
    borderLayout->addRow(labelShapeWidth_ptr, shapeWidthSpin_ptr);

    labelShapeHeight_ptr = new QLabel(tr("Shape Height:"));
    shapeHeightSpin_ptr = new QSpinBox(this);
    shapeHeightSpin_ptr->setFixedWidth(120);
    shapeHeightSpin_ptr->setRange(10, 5000);
    shapeHeightSpin_ptr->setSuffix(" px");
    shapeHeightSpin_ptr->setButtonSymbols(QAbstractSpinBox::NoButtons);
    borderLayout->addRow(labelShapeHeight_ptr, shapeHeightSpin_ptr);

    borderGroup_ptr->setLayout(borderLayout);
    mainLayout->addWidget(borderGroup_ptr);

    // Fill Group
    fillGroup_ptr = new QGroupBox(tr("Fill"));
    QFormLayout* fillLayout = new QFormLayout();
    fillLayout->setSpacing(8);
    fillLayout->setContentsMargins(10, 5, 10, 5);
    fillCheckBox_ptr = new QCheckBox(tr("No Fill"));
    connect(fillCheckBox_ptr, &QCheckBox::toggled, this, &FormatPanel::onFillCheckboxToggled);
    fillLayout->addRow(fillCheckBox_ptr);
    labelFillColor_ptr = new QLabel(tr("Color:"));
    fillButton_ptr = new QPushButton();
    fillButton_ptr->setFixedWidth(160);
    fillButton_ptr->setFlat(true);
    updateButtonColor(fillButton_ptr, Qt::white);
    connect(fillButton_ptr, &QPushButton::clicked, this, &FormatPanel::changeFillColorTriggered);
    fillLayout->addRow(labelFillColor_ptr, fillButton_ptr);
    fillGroup_ptr->setLayout(fillLayout);
    mainLayout->addWidget(fillGroup_ptr);

    // Line Group
    lineGroup_ptr = new QGroupBox(tr("Line"));
    QFormLayout* lineLayout = new QFormLayout();
    lineLayout->setSpacing(8);
    lineLayout->setContentsMargins(10, 5, 10, 5);
    labelLineColor_ptr = new QLabel(tr("Color:"));
    lineColorButton_ptr = new QPushButton();
    lineColorButton_ptr->setFixedWidth(120);
    lineColorButton_ptr->setFlat(true);
    updateButtonColor(lineColorButton_ptr, Qt::black);
    connect(lineColorButton_ptr, &QPushButton::clicked, this, &FormatPanel::changeLineColorTriggered);
    lineLayout->addRow(labelLineColor_ptr, lineColorButton_ptr);
    labelLineThickness_ptr = new QLabel(tr("Thickness:"));
    lineWidthCombo_ptr = new QComboBox();
    lineWidthCombo_ptr->setFixedWidth(120);
    lineWidthCombo_ptr->addItem("0.5");
    for (double i = 1.0; i <= 10; i += 0.5) { lineWidthCombo_ptr->addItem(QString::number(i, 'f', 1)); }
    lineLayout->addRow(labelLineThickness_ptr, lineWidthCombo_ptr);
    labelLineStyleUI_ptr = new QLabel(tr("Style:"));
    lineStyleCombo_ptr = new QComboBox();
    lineStyleCombo_ptr->setFixedWidth(120);
    lineStyleMap_data[tr("Solid Line")] = Qt::SolidLine;
    lineStyleMap_data[tr("Dashed Line")] = Qt::DashLine;
    lineStyleMap_data[tr("Dotted Line")] = Qt::DotLine;
    lineStyleMap_data[tr("Dash-Dot Line")] = Qt::DashDotLine;
    lineStyleMap_data[tr("Dash-Dot-Dot Line")] = Qt::DashDotDotLine;
    lineStyleCombo_ptr->addItems(lineStyleMap_data.keys());
    lineLayout->addRow(labelLineStyleUI_ptr, lineStyleCombo_ptr);
    labelLineEndpointStyle_ptr = new QLabel(tr("Endpoint Style:"));
    arrowStyleCombo_ptr = new QComboBox();
    arrowStyleCombo_ptr->setFixedWidth(120);
    arrowStyleCombo_ptr->addItem(tr("Arrow"));
    arrowStyleCombo_ptr->addItem(tr("Straight Line"));
    connect(arrowStyleCombo_ptr, &QComboBox::currentTextChanged, this, &FormatPanel::onArrowStyleChanged);
    lineLayout->addRow(labelLineEndpointStyle_ptr, arrowStyleCombo_ptr);
    lineGroup_ptr->setLayout(lineLayout);
    mainLayout->addWidget(lineGroup_ptr);

    // Text Group
    textGroup_ptr = new QGroupBox(tr("Text"));
    QFormLayout* textLayout = new QFormLayout();
    textLayout->setSpacing(8);
    textLayout->setContentsMargins(10, 5, 10, 5);
    labelTextCorlor_ptr = new QLabel(tr("Color:"));
    textButton_ptr = new QPushButton();
    textButton_ptr->setFixedWidth(150);
    updateButtonColor(textButton_ptr, Qt::black);
    connect(textButton_ptr, &QPushButton::clicked, this, &FormatPanel::changeTextColorTriggered);
    textLayout->addRow(labelTextCorlor_ptr, textButton_ptr);
    labelTextFont_ptr = new QLabel(tr("Font:"));
    fontCombo_ptr = new QFontComboBox();
    fontCombo_ptr->setFixedWidth(150);
    textLayout->addRow(labelTextFont_ptr, fontCombo_ptr);
    labelTextSize_ptr = new QLabel(tr("Size:"));
    fontSizeSpin_ptr = new QSpinBox();
    fontSizeSpin_ptr->setFixedWidth(150);
    fontSizeSpin_ptr->setRange(6, 36);
    fontSizeSpin_ptr->setValue(10);
    fontSizeSpin_ptr->setSpecialValueText("");
    fontSizeSpin_ptr->setValue(fontSizeSpin_ptr->minimum());
    textLayout->addRow(labelTextSize_ptr, fontSizeSpin_ptr);
    textGroup_ptr->setLayout(textLayout);
    mainLayout->addWidget(textGroup_ptr);

    // Canvas Group
    canvasGroup_ptr = new QGroupBox(tr("Canvas"));
    QFormLayout* canvasLayout = new QFormLayout();
    canvasLayout->setSpacing(8);
    canvasLayout->setContentsMargins(10, 5, 10, 5);
    labelCanvasWidth_ptr = new QLabel(tr("Width:"));
    canvasWidthSpin_ptr = new QSpinBox();
    canvasWidthSpin_ptr->setFixedWidth(120);
    canvasWidthSpin_ptr->setRange(100, 10000);
    canvasWidthSpin_ptr->setSuffix(" px");
    canvasWidthSpin_ptr->setButtonSymbols(QAbstractSpinBox::NoButtons);
    connect(canvasWidthSpin_ptr, QOverload<int>::of(&QSpinBox::valueChanged), this, &FormatPanel::onCanvasWidthEdited);
    canvasLayout->addRow(labelCanvasWidth_ptr, canvasWidthSpin_ptr);
    labelCanvasHeight_ptr = new QLabel(tr("Height:"));
    canvasHeightSpin_ptr = new QSpinBox();
    canvasHeightSpin_ptr->setFixedWidth(120);
    canvasHeightSpin_ptr->setRange(100, 10000);
    canvasHeightSpin_ptr->setSuffix(" px");
    // canvasHeightSpin_ptr->setFixedHeight(28); // Removed for consistency or add to all
    connect(canvasHeightSpin_ptr, QOverload<int>::of(&QSpinBox::valueChanged), this, &FormatPanel::onCanvasHeightEdited);
    canvasLayout->addRow(labelCanvasHeight_ptr, canvasHeightSpin_ptr);
    labelCanvasZoom_ptr = new QLabel(tr("Zoom:"));
    canvasZoomSpin_ptr = new QDoubleSpinBox();
    canvasZoomSpin_ptr->setFixedWidth(120);
    canvasZoomSpin_ptr->setRange(10.0, 500.0);
    canvasZoomSpin_ptr->setSingleStep(10.0);
    canvasZoomSpin_ptr->setDecimals(1);
    canvasZoomSpin_ptr->setSuffix(" %");
    // canvasZoomSpin_ptr->setFixedHeight(28); // Removed for consistency
    connect(canvasZoomSpin_ptr, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FormatPanel::onCanvasZoomEdited);
    canvasLayout->addRow(labelCanvasZoom_ptr, canvasZoomSpin_ptr);
    canvasGroup_ptr->setLayout(canvasLayout);
    mainLayout->addWidget(canvasGroup_ptr);

    mainLayout->addStretch(); // Pushes all group boxes to the top

    // Connect signals
    connect(borderWidthCombo_ptr, &QComboBox::currentTextChanged, this, &FormatPanel::onBorderWidthChanged);
    connect(shapeWidthSpin_ptr, QOverload<int>::of(&QSpinBox::valueChanged), this, &FormatPanel::onShapeWidthChanged);
    connect(shapeHeightSpin_ptr, QOverload<int>::of(&QSpinBox::valueChanged), this, &FormatPanel::onShapeHeightChanged);
    connect(borderStyleCombo_ptr, &QComboBox::currentTextChanged, this, &FormatPanel::onBorderStyleChanged);
    connect(lineWidthCombo_ptr, &QComboBox::currentTextChanged, this, &FormatPanel::onLineWidthChanged);
    connect(lineStyleCombo_ptr, &QComboBox::currentTextChanged, this, &FormatPanel::onLineStyleChanged);
    connect(fontCombo_ptr, &QFontComboBox::currentFontChanged, this, &FormatPanel::onFontChanged);
    connect(fontSizeSpin_ptr, QOverload<int>::of(&QSpinBox::valueChanged), this, &FormatPanel::onFontSizeChanged);
}

// --- 更新 UI 的函数 ---

void FormatPanel::updateSelectedShapes(const QList<Shape*>& shapes) {
    bool hasSelection = !shapes.isEmpty();          // 是否有选中项
    bool singleShapeSelected = shapes.size() == 1; // 是否只选中了一个图形

    // === 1. 根据是否有选中项，启用/禁用控件组 ===
    if (borderGroup_ptr) borderGroup_ptr->setEnabled(hasSelection);
    if (fillGroup_ptr) fillGroup_ptr->setEnabled(hasSelection);
    // 文本组 (textGroup_ptr) 的启用状态将在后面根据是否全为文本框来决定
    if (shapeWidthSpin_ptr) shapeWidthSpin_ptr->setEnabled(singleShapeSelected);  // 图形宽度控件仅在单选时启用
    if (shapeHeightSpin_ptr) shapeHeightSpin_ptr->setEnabled(singleShapeSelected); // 图形高度控件仅在单选时启用

    // === 2. 如果没有选中项，重置所有控件为默认/禁用状态 ===
    if (!hasSelection) {
        // 重置边框组
        if (borderButton_ptr) updateButtonColor(borderButton_ptr, Qt::black);
        if (borderWidthCombo_ptr) borderWidthCombo_ptr->setCurrentIndex(-1); // 清空选择
        if (borderStyleCombo_ptr) borderStyleCombo_ptr->setCurrentIndex(-1); // 清空选择
        // 重置填充组
        if (fillCheckBox_ptr) fillCheckBox_ptr->setCheckState(Qt::Unchecked);
        if (fillButton_ptr) updateButtonColor(fillButton_ptr, Qt::white);
        // 重置尺寸组 (并阻止信号触发)
        if (shapeWidthSpin_ptr) {
            shapeWidthSpin_ptr->blockSignals(true);
            shapeWidthSpin_ptr->setValue(shapeWidthSpin_ptr->minimum()); // 设为最小值
            shapeWidthSpin_ptr->blockSignals(false);
        }
        if (shapeHeightSpin_ptr) {
            shapeHeightSpin_ptr->blockSignals(true);
            shapeHeightSpin_ptr->setValue(shapeHeightSpin_ptr->minimum()); // 设为最小值
            shapeHeightSpin_ptr->blockSignals(false);
        }
        // 重置文本组
        if (textGroup_ptr) textGroup_ptr->setEnabled(false); // 禁用文本组
        if (textButton_ptr) updateButtonColor(textButton_ptr, Qt::black);
        if (fontCombo_ptr) fontCombo_ptr->setCurrentIndex(-1); // 清空字体选择
        if (fontSizeSpin_ptr) fontSizeSpin_ptr->setValue(fontSizeSpin_ptr->minimum()); // 字号设为最小值 (显示为空)

        return; // 没有选中项，后续处理无需进行
    }

    // === 3. 处理选中项的属性 ===
    bool multiSelection = shapes.size() > 1; // 是否为多选
    Shape* firstShape = shapes.first();     // 获取第一个选中的图形作为基准
    if (!firstShape) {
        // 如果第一个图形无效（理论上不应发生），禁用所有组并返回
        if (borderGroup_ptr) borderGroup_ptr->setEnabled(false);
        if (fillGroup_ptr) fillGroup_ptr->setEnabled(false);
        if (lineGroup_ptr) lineGroup_ptr->setEnabled(false); // 也禁用线条组（虽然此函数不处理）
        if (textGroup_ptr) textGroup_ptr->setEnabled(false);
        // 画布组通常保持启用
        return;
    }

    // --- 3.1 获取第一个图形的属性 ---
    QColor firstBorderColor = firstShape->getBorderColor();
    Qt::PenStyle firstBorderStyle = firstShape->getBorderStyle();
    qreal firstBorderWidth = firstShape->getBorderWidth();
    bool firstFillState = firstShape->isFilled();
    QColor firstFillColor = firstShape->getFillColor();

    // --- 3.2 检查多选时属性是否一致 ---
    bool borderColorConsistent = true;
    bool borderStyleConsistent = true;
    bool borderWidthConsistent = true;
    bool fillStateConsistent = true;
    bool fillColorConsistent = true;

    if (multiSelection) {
        for (int i = 1; i < shapes.size(); ++i) {
            Shape* currentShape = shapes.at(i);
            if (!currentShape) continue; // 跳过无效图形
            if (currentShape->getBorderColor() != firstBorderColor) borderColorConsistent = false;
            if (currentShape->getBorderStyle() != firstBorderStyle) borderStyleConsistent = false; // 检查边框样式
            if (!qFuzzyCompare(currentShape->getBorderWidth(), firstBorderWidth)) borderWidthConsistent = false; // 使用浮点比较
            if (currentShape->isFilled() != firstFillState) fillStateConsistent = false;
            if (currentShape->getFillColor() != firstFillColor) fillColorConsistent = false;
            // 如果所有属性都已经不一致，可以提前退出循环（轻微优化）
            // if (!borderColorConsistent && !borderStyleConsistent && !borderWidthConsistent && !fillStateConsistent && !fillColorConsistent) break;
        }
    }

    // === 4. 更新 UI 控件状态 ===

    // --- 4.1 更新边框组 UI ---
    // 边框颜色按钮
    if (borderButton_ptr) {
        borderButton_ptr->setEnabled(true); // 只要有选中就启用
        if (borderColorConsistent) {
            updateButtonColor(borderButton_ptr, firstBorderColor);
            borderButton_ptr->setToolTip(firstBorderColor.name());
        }
        else {
            updateButtonColor(borderButton_ptr, QColor()); // 无效颜色显示棋盘格
            borderButton_ptr->setToolTip(tr("Border colors are inconsistent", "边框颜色不一致")); // 使用 tr()
        }
    }

    // 边框宽度下拉框
    bool isEffectivelyNoBorder = false; // 标记是否应视为无边框
    if (borderWidthCombo_ptr) {
        borderWidthCombo_ptr->blockSignals(true);
        if (borderWidthConsistent) {
            if (qFuzzyCompare(firstBorderWidth, 0)) {
                // 查找并选中 "无边框" 选项
                int noBorderIndex = borderWidthCombo_ptr->findText(tr("No Border", "无边框"));
                borderWidthCombo_ptr->setCurrentIndex(noBorderIndex != -1 ? noBorderIndex : 0);
                isEffectivelyNoBorder = true;
            }
            else {
                QString widthStr = QString::number(firstBorderWidth, 'f', 1);
                int index = borderWidthCombo_ptr->findText(widthStr);
                borderWidthCombo_ptr->setCurrentIndex(index); // 选中对应宽度，找不到则为 -1 (显示空白)
                isEffectivelyNoBorder = false;
            }
        }
        else {
            borderWidthCombo_ptr->setCurrentIndex(-1); // 宽度不一致，显示空白
            isEffectivelyNoBorder = false; // 不能确定是否无边框
        }
        borderWidthCombo_ptr->blockSignals(false);
    }

    // 边框样式下拉框 (其状态依赖于边框宽度)
    if (borderStyleCombo_ptr) {
        borderStyleCombo_ptr->blockSignals(true);
        if (isEffectivelyNoBorder) {
            // 如果确定是无边框，选中 "无样式" 并禁用
            int noStyleIndex = borderStyleCombo_ptr->findText(tr("No Style", "无样式"));
            borderStyleCombo_ptr->setCurrentIndex(noStyleIndex != -1 ? noStyleIndex : 0);
            borderStyleCombo_ptr->setEnabled(false);
        }
        else {
            // 如果边框宽度非 0 或不一致，启用样式选择
            borderStyleCombo_ptr->setEnabled(true);
            if (borderStyleConsistent) {
                // 样式一致，查找并选中对应样式名称
                QString styleName;
                // 注意：这里使用 borderStyleMap_data 成员变量
                for (auto it = borderStyleMap_data.constBegin(); it != borderStyleMap_data.constEnd(); ++it) {
                    if (it.value() == firstBorderStyle) {
                        styleName = it.key(); // key 应该是已经翻译过的 (在 retranslateUi 中填充的)
                        break;
                    }
                }
                int index = borderStyleCombo_ptr->findText(styleName);
                borderStyleCombo_ptr->setCurrentIndex(index); // 找不到则显示空白
            }
            else {
                borderStyleCombo_ptr->setCurrentIndex(-1); // 样式不一致，显示空白
            }
        }
        borderStyleCombo_ptr->blockSignals(false);
    }

    // 图形尺寸 SpinBox (仅单选时更新值)
    if (shapeWidthSpin_ptr && shapeHeightSpin_ptr) {
        shapeWidthSpin_ptr->blockSignals(true);
        shapeHeightSpin_ptr->blockSignals(true);
        if (singleShapeSelected) {
            QRect currentRect = firstShape->getRect();
            shapeWidthSpin_ptr->setValue(currentRect.width());
            shapeHeightSpin_ptr->setValue(currentRect.height());
        }
        else {
            // 多选时，设为最小值（或保持禁用时的值）
            shapeWidthSpin_ptr->setValue(shapeWidthSpin_ptr->minimum());
            shapeHeightSpin_ptr->setValue(shapeHeightSpin_ptr->minimum());
        }
        shapeWidthSpin_ptr->blockSignals(false);
        shapeHeightSpin_ptr->blockSignals(false);
    }

    // --- 4.2 更新填充组 UI ---
    // "无填充" 复选框
    if (fillCheckBox_ptr) {
        fillCheckBox_ptr->blockSignals(true);
        if (fillStateConsistent) {
            // 状态一致，根据第一个图形的状态设置 (注意逻辑反转)
            fillCheckBox_ptr->setCheckState(!firstFillState ? Qt::Checked : Qt::Unchecked);
        }
        else {
            fillCheckBox_ptr->setCheckState(Qt::PartiallyChecked); // 状态不一致，部分选中
        }
        fillCheckBox_ptr->setEnabled(true); // 复选框始终启用（只要组启用）
        fillCheckBox_ptr->blockSignals(false);
    }

    // 填充颜色按钮
    if (fillButton_ptr) {
        fillButton_ptr->setEnabled(true); // 颜色按钮始终启用（只要组启用）
        if (fillStateConsistent && firstFillState) { // 状态一致且【已填充】
            if (fillColorConsistent) {
                updateButtonColor(fillButton_ptr, firstFillColor);
                fillButton_ptr->setToolTip(firstFillColor.name());
            }
            else {
                updateButtonColor(fillButton_ptr, QColor()); // 无效颜色
                fillButton_ptr->setToolTip(tr("Fill colors are inconsistent", "填充颜色不一致")); // tr()
            }
        }
        else if (fillStateConsistent && !firstFillState) { // 状态一致且【无填充】
            updateButtonColor(fillButton_ptr, QColor(230, 230, 230)); // 显示一个表示“无”的灰色
            fillButton_ptr->setToolTip(tr("No fill", "无填充")); // tr()
        }
        else { // 填充状态不一致
            updateButtonColor(fillButton_ptr, QColor()); // 无效颜色
            fillButton_ptr->setToolTip(tr("Fill states are inconsistent", "填充状态不一致")); // tr()
        }
    }

    // --- 4.3 更新文本组 UI ---
    // 首先判断是否所有选中的都是文本框
    bool containsTextBox = false;
    bool allAreTextBoxes = hasSelection; // 初始假设为真（如果 hasSelection 为 false 则此值为 false）
    QFont firstFont;
    QColor firstTextColor;
    TextBox* firstTextBox = dynamic_cast<TextBox*>(firstShape); // 尝试转换第一个

    if (firstTextBox) {
        containsTextBox = true;
        firstFont = firstTextBox->getFont();
        firstTextColor = firstTextBox->getTextColor();
    }
    else {
        allAreTextBoxes = false; // 第一个就不是文本框
    }

    // 如果是多选，检查剩余的是否也都是文本框，并检查属性一致性
    bool fontConsistent = true;
    bool textColorConsistent = true;
    if (multiSelection && allAreTextBoxes) { // 只有在第一个是文本框且是多选时才需要继续检查
        for (int i = 1; i < shapes.size(); ++i) {
            TextBox* currentTB = dynamic_cast<TextBox*>(shapes.at(i));
            if (!currentTB) {
                allAreTextBoxes = false; // 发现一个不是文本框
                fontConsistent = false;
                textColorConsistent = false;
                break; // 无需再检查
            }
            // 如果都是文本框，检查属性一致性
            if (fontConsistent && currentTB->getFont() != firstFont) fontConsistent = false;
            if (textColorConsistent && currentTB->getTextColor() != firstTextColor) textColorConsistent = false;
        }
    }
    else if (!firstTextBox && hasSelection) {
        // 如果第一个就不是文本框，则肯定不是“全是文本框”
        allAreTextBoxes = false;
    }

    // 根据检查结果启用/禁用文本组并设置控件状态
    bool enableTextGroup = containsTextBox && allAreTextBoxes;
    if (textGroup_ptr) textGroup_ptr->setEnabled(enableTextGroup);

    if (enableTextGroup) {
        // 字体下拉框和字号 SpinBox
        if (fontCombo_ptr && fontSizeSpin_ptr) {
            fontCombo_ptr->blockSignals(true);
            fontSizeSpin_ptr->blockSignals(true);
            if (fontConsistent) {
                fontCombo_ptr->setCurrentFont(firstFont);
                int currentSize = firstFont.pointSize() > 0 ? firstFont.pointSize() : firstFont.pixelSize();
                // 确保字号在 SpinBox 范围内
                currentSize = qBound(fontSizeSpin_ptr->minimum(), currentSize, fontSizeSpin_ptr->maximum());
                fontSizeSpin_ptr->setValue(currentSize > 0 ? currentSize : 10); // 设值，确保有默认
            }
            else {
                // 字体不一致，清空显示
                fontCombo_ptr->setCurrentIndex(-1); // 或尝试清空 lineEdit
                if (fontCombo_ptr->lineEdit()) fontCombo_ptr->lineEdit()->clear();
                fontSizeSpin_ptr->setValue(fontSizeSpin_ptr->minimum()); // 字号设为最小值 (显示为空)
            }
            fontCombo_ptr->blockSignals(false);
            fontSizeSpin_ptr->blockSignals(false);
        }
        // 文本颜色按钮
        if (textButton_ptr) {
            textButton_ptr->setEnabled(true); // 启用颜色按钮
            if (textColorConsistent) {
                updateButtonColor(textButton_ptr, firstTextColor);
                textButton_ptr->setToolTip(firstTextColor.name());
            }
            else {
                updateButtonColor(textButton_ptr, QColor()); // 无效颜色
                textButton_ptr->setToolTip(tr("Text colors are inconsistent", "文本颜色不一致")); // tr()
            }
        }
    }
    else {
        // 如果文本组被禁用，重置控件状态
        if (fontCombo_ptr) fontCombo_ptr->setCurrentIndex(-1);
        if (fontSizeSpin_ptr) fontSizeSpin_ptr->setValue(fontSizeSpin_ptr->minimum());
        if (textButton_ptr) updateButtonColor(textButton_ptr, Qt::black);
    }

    update(); 
}

void FormatPanel::updateSelectedLines(const QList<ConnectionLine*>& lines) {
    bool hasSelection = !lines.isEmpty(); // 是否有选中的线条

    // === 1. 根据是否有选中线条，启用/禁用线条属性组 ===
    if (lineGroup_ptr) lineGroup_ptr->setEnabled(hasSelection);

    // === 2. 如果没有选中项，重置线条组控件为默认/禁用状态 ===
    if (!hasSelection) {
        if (lineColorButton_ptr) updateButtonColor(lineColorButton_ptr, Qt::black);
        if (lineWidthCombo_ptr) lineWidthCombo_ptr->setCurrentIndex(-1); // 清空选择
        if (lineStyleCombo_ptr) lineStyleCombo_ptr->setCurrentIndex(-1); // 清空选择
        if (arrowStyleCombo_ptr) {
            arrowStyleCombo_ptr->setCurrentIndex(-1); // 清空选择
            arrowStyleCombo_ptr->setEnabled(false); // 禁用箭头样式选择
        }
        // 注意：这里不需要禁用 lineGroup_ptr 本身，因为已经在步骤 1 处理了
        return; // 没有选中项，处理完毕
    }

    // === 3. 处理选中线条的属性 ===
    bool multiSelection = lines.size() > 1;         // 是否为多选
    ConnectionLine* firstLine = lines.first();      // 获取第一个选中的线条作为基准
    if (!firstLine) {
        // 如果第一个线条无效，禁用线条组并返回
        if (lineGroup_ptr) lineGroup_ptr->setEnabled(false);
        return;
    }

    // --- 3.1 获取第一个线条的属性 ---
    QColor firstLineColor = firstLine->getColor();
    qreal firstLineWidth = firstLine->getLineWidth();
    Qt::PenStyle firstLineStyle = firstLine->getLineStyle();
    bool firstArrowState = firstLine->hasArrowHead(); // 获取箭头状态

    // --- 3.2 检查多选时属性是否一致 ---
    bool colorConsistent = true;
    bool widthConsistent = true;
    bool styleConsistent = true;
    bool arrowConsistent = true; // 检查箭头状态一致性

    if (multiSelection) {
        for (int i = 1; i < lines.size(); ++i) {
            ConnectionLine* currentLine = lines.at(i);
            if (!currentLine) continue; // 跳过无效线条
            if (currentLine->getColor() != firstLineColor) colorConsistent = false;
            if (!qFuzzyCompare(currentLine->getLineWidth(), firstLineWidth)) widthConsistent = false; // 浮点比较
            if (currentLine->getLineStyle() != firstLineStyle) styleConsistent = false;
            if (currentLine->hasArrowHead() != firstArrowState) arrowConsistent = false; // 检查箭头
            // 可选优化: if (!colorConsistent && !widthConsistent && ...) break;
        }
    }

    // === 4. 更新 UI 控件状态 ===

    // --- 4.1 更新线条颜色按钮 ---
    if (lineColorButton_ptr) {
        lineColorButton_ptr->setEnabled(true); // 只要有选中就启用
        if (colorConsistent) {
            updateButtonColor(lineColorButton_ptr, firstLineColor);
            lineColorButton_ptr->setToolTip(firstLineColor.name());
        }
        else {
            updateButtonColor(lineColorButton_ptr, QColor()); // 无效颜色
            lineColorButton_ptr->setToolTip(tr("Line colors are inconsistent", "线条颜色不一致")); // tr()
        }
    }

    // --- 4.2 更新线条宽度下拉框 ---
    if (lineWidthCombo_ptr) {
        lineWidthCombo_ptr->blockSignals(true);
        if (widthConsistent) {
            QString widthStr = QString::number(firstLineWidth, 'f', 1);
            int index = lineWidthCombo_ptr->findText(widthStr);
            lineWidthCombo_ptr->setCurrentIndex(index); // 找不到则为 -1 (显示空白)
        }
        else {
            lineWidthCombo_ptr->setCurrentIndex(-1); // 宽度不一致，显示空白
        }
        lineWidthCombo_ptr->blockSignals(false);
    }

    // --- 4.3 更新线条样式下拉框 ---
    if (lineStyleCombo_ptr) {
        lineStyleCombo_ptr->blockSignals(true);
        if (styleConsistent) {
            QString styleName;
            // 注意：使用 lineStyleMap_data 成员变量
            for (auto it = lineStyleMap_data.constBegin(); it != lineStyleMap_data.constEnd(); ++it) {
                if (it.value() == firstLineStyle) {
                    styleName = it.key(); // key 应该是已翻译的 (来自 retranslateUi)
                    break;
                }
            }
            int index = lineStyleCombo_ptr->findText(styleName);
            lineStyleCombo_ptr->setCurrentIndex(index); // 找不到则为 -1 (显示空白)
        }
        else {
            lineStyleCombo_ptr->setCurrentIndex(-1); // 样式不一致，显示空白
        }
        lineStyleCombo_ptr->blockSignals(false);
    }

    // --- 4.4 更新箭头样式下拉框 ---
    if (arrowStyleCombo_ptr) {
        arrowStyleCombo_ptr->blockSignals(true);
        arrowStyleCombo_ptr->setEnabled(true); // 只要有选中就启用
        if (arrowConsistent) {
            // 状态一致，根据第一个线条的状态设置选中项
            if (firstArrowState) {
                // 查找并选中 "箭头" 选项
                int arrowIndex = arrowStyleCombo_ptr->findText(tr("Arrow", "箭头"));
                arrowStyleCombo_ptr->setCurrentIndex(arrowIndex != -1 ? arrowIndex : 0);
            }
            else {
                // 查找并选中 "直线" 选项
                int lineIndex = arrowStyleCombo_ptr->findText(tr("Straight Line", "直线"));
                arrowStyleCombo_ptr->setCurrentIndex(lineIndex != -1 ? lineIndex : 0);
            }
        }
        else {
            // 箭头状态不一致，清空选中项
            arrowStyleCombo_ptr->setCurrentIndex(-1);
        }
        arrowStyleCombo_ptr->blockSignals(false);
    }

    // update(); // 通常不需要显式调用 update()
}

// --- 槽函数实现 (处理 UI 变化并发出信号) ---

void FormatPanel::onBorderWidthChanged(const QString& text) {
    if (text.isEmpty()) return;

    qreal width = 0; 
    if (text != tr("No Border")) {
        bool ok;
        width = text.toDouble(&ok);
        if (!ok || width < 0) return; // 转换失败或无效值则忽略
    }
    // *** 新增逻辑：根据宽度更新样式下拉框状态 ***
    borderStyleCombo_ptr->blockSignals(true); // 临时阻止信号
    if (qFuzzyCompare(width, 0)) {
        borderStyleCombo_ptr->setCurrentIndex(borderStyleCombo_ptr->findText(NO_BORDER_STYLE_TEXT));
        borderStyleCombo_ptr->setEnabled(false);
    }
    else {
        if (borderStyleCombo_ptr->currentText() == NO_BORDER_STYLE_TEXT) {
            borderStyleCombo_ptr->setCurrentIndex(borderStyleCombo_ptr->findText(tr("Solid Line")));
        }
        borderStyleCombo_ptr->setEnabled(true);
    }
    borderStyleCombo_ptr->blockSignals(false); // 恢复信号
    emit borderWidthChanged(width);
}

// --- 实现新的槽函数 ---
void FormatPanel::onBorderStyleChanged(const QString& text) {
    // 检查指针是否有效
    if (!borderStyleCombo_ptr || !borderGroup_ptr || !borderGroup_ptr->isEnabled()) {
        // 如果控件不存在，或者边框组被禁用，则不处理
        return;
    }
    if (text.isEmpty() || text == tr("No Style", "无样式") || !borderStyleMap_data.contains(text)) {
        return;
    }
    Qt::PenStyle style = borderStyleMap_data.value(text);

    // 发射信号，通知外部边框样式已改变
    emit borderStyleChanged(style);
}

void FormatPanel::onFillToggled(bool checked) {
    // 检查相关控件指针是否有效
    if (!fillButton_ptr || !fillGroup_ptr || !fillGroup_ptr->isEnabled()) {
        // 如果控件不存在或填充组被禁用，则不处理
        return;
    }
    bool isFilled = checked;

    emit fillStateChanged(isFilled);

 
}

void FormatPanel::onLineWidthChanged(const QString& text) {
    if (text.isEmpty()) return;
    bool ok;
    qreal width = text.toDouble(&ok);
    if (ok && width >= 0.1) { // 确保宽度有效
        emit lineWidthChanged(width);
    }
}

void FormatPanel::onLineStyleChanged(const QString& text) {
    if (text.isEmpty() || !lineStyleMap_data.contains(text)) return;
    Qt::PenStyle style = lineStyleMap_data.value(text);

    emit lineStyleChanged(style);
}

// --- 新增槽函数实现 ---
void FormatPanel::onArrowStyleChanged(const QString& text) {
    if (text.isEmpty()) return; // 忽略因 setCurrentIndex(-1) 导致的空文本

    bool hasArrow = false;
    if (text == tr("Arrow")) {
        hasArrow = true;
    }
    else if (text == tr("Straight Line")) {
        hasArrow = false;
    }
    else {

        return;
    }

    // 发射信号，传递转换后的布尔值
    emit arrowStateChanged(hasArrow);
}

void FormatPanel::onFontChanged(const QFont& font) {
    if (!textGroup_ptr->isEnabled()) return; // 如果文本组未启用，忽略信号

    // 更新字号 SpinBox 以匹配
    int newSize = font.pointSize();
    if (newSize <= 0) newSize = font.pixelSize();
    if (newSize > 0 && fontSizeSpin_ptr->value() != newSize) {
        fontSizeSpin_ptr->blockSignals(true);
        if (newSize >= fontSizeSpin_ptr->minimum() && newSize <= fontSizeSpin_ptr->maximum()) {
            fontSizeSpin_ptr->setValue(newSize);
        }
        else {
            fontSizeSpin_ptr->setValue(0); // 超出范围设为0或特殊值
        }
        fontSizeSpin_ptr->blockSignals(false);
    }
    emit fontChanged(font); // 发射包含完整字体信息的信号
}

void FormatPanel::onFontSizeChanged(int size) {
    if (!textGroup_ptr->isEnabled() || size <= 0) return; // 如果文本组未启用或大小无效，忽略

    QFont currentFont = fontCombo_ptr->currentFont();
    bool changed = false;

    // 检查是点大小还是像素大小改变
    if (currentFont.pointSize() > 0) {
        if (currentFont.pointSize() != size) {
            currentFont.setPointSize(size);
            changed = true;
        }
    }
    else { // 使用像素大小
        if (currentFont.pixelSize() != size) {
            currentFont.setPixelSize(size);
            changed = true;
        }
    }

    if (changed) {
        // 更新 FontComboBox 但阻止其再次触发 onFontChanged
        fontCombo_ptr->blockSignals(true);
        fontCombo_ptr->setCurrentFont(currentFont);
        fontCombo_ptr->blockSignals(false);
        // 发射包含完整字体信息的信号
        emit fontChanged(currentFont);
    }
}

// --- 辅助函数 ---
// --- **修改:** 辅助函数 updateButtonColor ---
void FormatPanel::updateButtonColor(QPushButton* button, const QColor& color) {
    if (!button) return;

    QString styleSheet =
        "QPushButton {"
        "   background-color: %1;" // 颜色或棋盘格
        "   border: 1px solid #475569;" // 统一边框
        "   border-radius: 4px;"
        "   min-height: 24px;" // 统一高度
        "   padding: 4px;"
        "}"
        "QPushButton:hover {"
        "    border-color: #60A5FA;" // 悬停时边框变浅蓝    
        "}"
        "QPushButton:pressed {"
        "    background-color: #334155;" // 按下时变暗
        "}";

    if (!color.isValid()) {
        // 使用棋盘格背景表示无效或混合色
        styleSheet = styleSheet.arg("qlineargradient(spread:repeat, x1:0, y1:0, x2:10, y2:10,"
            "stop:0 #CCCCCC, stop:0.5 #CCCCCC,"
            "stop:0.5 #E0E0E0, stop:1 #E0E0E0)");
        button->setToolTip(tr("Color is inconsistent"));
    }
    else if (color == Qt::transparent || color.alpha() == 0) {
        // 使用棋盘格背景表示透明色
        styleSheet = styleSheet.arg("qlineargradient(spread:repeat, x1:0, y1:0, x2:10, y2:10,"
            "stop:0 #CCCCCC, stop:0.5 #CCCCCC,"
            "stop:0.5 #FFFFFF, stop:1 #FFFFFF)");
        button->setToolTip(tr("Transparent"));
    }
    else {
        styleSheet = styleSheet.arg(color.name());
        button->setToolTip(color.name());
    }
    button->setStyleSheet(styleSheet);
    button->setText(""); // 始终无文本
}

void FormatPanel::onFillCheckboxToggled(bool checked) {
    // checked 为 true 意味着 "无填充" 被勾选了
    bool isNowFilled = !checked; // 实际的填充状态与复选框状态相反

    //qDebug() << "FormatPanel: '无填充' checkbox toggled. Checked (No Fill):" << checked << ", Is Now Filled:" << isNowFilled;

    if (fillButton_ptr) {
        fillButton_ptr->setEnabled(true); // 如果现在是填充状态，则启用颜色按钮；否则禁用
    }

    // 发出 fillStateChanged 信号，传递【实际】的填充状态
    emit fillStateChanged(isNowFilled);
    emit changeNofillStateTriggered();
}

// --- 实现画布属性相关的槽函数 ---
// 内部槽：当用户编辑宽度输入框时触发
void FormatPanel::onCanvasWidthEdited(int value) {
    qDebug() << tr("FormatPanel: Canvas width edited in panel:") << value;
    emit canvasWidthChanged(value); // 发射信号通知外部
}

// 内部槽：当用户编辑高度输入框时触发
void FormatPanel::onCanvasHeightEdited(int value) {
    qDebug() << tr("FormatPanel: Canvas height edited in panel:") << value;
    emit canvasHeightChanged(value); // 发射信号通知外部
}

// 内部槽：当用户编辑缩放输入框时触发
void FormatPanel::onCanvasZoomEdited(double value) {
    qDebug() << tr("FormatPanel: Canvas zoom edited in panel:") << value << "%";
    emit canvasZoomChanged(value); // 发射信号通知外部 (发送百分比)
}

// 公共槽：接收外部的画布属性，更新UI显示
void FormatPanel::updateCanvasProperties(const QSize& size, double currentZoomFactor) {
    qDebug() << tr("FormatPanel: Updating canvas properties UI. Size:") << size << tr("Zoom Factor:") << currentZoomFactor;

    // 阻止信号，防止设置值时触发 valueChanged 再次发射信号
    canvasWidthSpin_ptr->blockSignals(true);
    canvasHeightSpin_ptr->blockSignals(true);
    canvasZoomSpin_ptr->blockSignals(true);

    canvasWidthSpin_ptr->setValue(size.width());
    canvasHeightSpin_ptr->setValue(size.height());
    canvasZoomSpin_ptr->setValue(currentZoomFactor * 100.0); // 将因子转为百分比

    // 恢复信号
    canvasWidthSpin_ptr->blockSignals(false);
    canvasHeightSpin_ptr->blockSignals(false);
    canvasZoomSpin_ptr->blockSignals(false);
}

void FormatPanel::onShapeWidthChanged(int value) {
    qDebug() << tr("FormatPanel: Shape width edited in panel:") << value;
    // 可以在这里添加验证，比如确保 value 大于某个最小值
    if (value >= 10) { // 假设最小宽度为 10
        emit shapeWidthChanged(value); // 发射信号通知 DiagramEditor
    }
    else {
        // 如果值无效，可以恢复旧值或不做任何操作
        // QTimer::singleShot(0, [this, oldValue](){ shapeWidthSpin->setValue(oldValue); }); // 延迟恢复
    }
}

void FormatPanel::onShapeHeightChanged(int value) {
    qDebug() << tr("FormatPanel: Shape height edited in panel:") << value;
    if (value >= 10) { // 假设最小高度为 10
        emit shapeHeightChanged(value); // 发射信号通知 DiagramEditor
    }
    else {
        // 处理无效值
    }
}