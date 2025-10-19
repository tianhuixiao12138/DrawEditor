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
        "    background-color: #1E293B;" // ��屳��ɫ
        "    color: #E2E8F0;"            // Ĭ��������ɫ (ǳ��)
        "    border: none;"
        "}"
        // --- QGroupBox ��ʽ ---
        "QGroupBox {"
        "    color: #E2E8F0;" // GroupBox ������ɫ
        "    font-weight: bold;"
        "    border: 1px solid #334155;" // GroupBox �߿���ɫ
        "    border-radius: 6px;"
        "    margin-top: 10px;" // Ϊ���������ռ�
        "    padding: 15px 5px 5px 5px;" // �������ڱ߾�
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    subcontrol-position: top left;" // ����λ��
        "    left: 10px;"
        "    padding: 0 3px 0 3px;"
        "    background-color: #1E293B;" // ���ⱳ���������ͬ
        "}"
        // --- QLabel ��ʽ (�����Ҫ����Ĭ��) ---
        "QLabel {"
        "    color: #94A3B8;" // ��ǩʹ���԰��Ļ�ɫ
        "    background-color: transparent;" // ȷ������͸��
        "    padding-top: 4px;" // ��ֱ���ж���
        "}"
        // --- ����ؼ�ͨ����ʽ ---
        "QAbstractSpinBox, QLineEdit, QComboBox {" // QDoubleSpinBox �̳� QAbstractSpinBox
        "    background-color: #28354C;" // �ؼ�����
        "    color: #E2E8F0;" // �ؼ�������ɫ
        "    border: 1px solid #475569;" // �ؼ��߿�
        "    border-radius: 4px;"
        "    padding: 4px 8px;" // �ڱ߾�
        "    min-height: 24px;" // ͳһ��С�߶�
        "}"
        "QAbstractSpinBox:focus, QLineEdit:focus, QComboBox:focus {"
        "    border-color: #3B82F6;" // �۽�ʱ��ɫ�߿�
        "}"
        // --- QSpinBox/QDoubleSpinBox ��ͷ��ʽ ---
        "QSpinBox::up-button, QDoubleSpinBox::up-button {"
        "    subcontrol-origin: border;"
        "    subcontrol-position: top right;"
        "    width: 16px;"
        "    image: %1;" // ��Ҫǳɫ���ϼ�ͷͼ��
        "    background-color: #334155;"
        "    border-left: 1px solid #475569;"
        "    border-top-right-radius: 4px;"
        "    border-bottom-right-radius: 0px;"
        "}"
        "QSpinBox::down-button, QDoubleSpinBox::down-button {"
        "    subcontrol-origin: border;"
        "    subcontrol-position: bottom right;"
        "    width: 16px;"
        "    image: %2;" // ��Ҫǳɫ���¼�ͷͼ��
        "    background-color: #334155;"
        "    border-left: 1px solid #475569;"
        "    border-top-right-radius: 0px;"
        "    border-bottom-right-radius: 4px;"
        "}"
        "QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover,"
        "QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover {"
        "    background-color: #475569;"
        "}"
        // --- QComboBox ������ͷ ---
        "QComboBox::drop-down {"
        "    subcontrol-origin: padding;"
        "    subcontrol-position: top right;"
        "    width: 20px;"
        "    border-left: 1px solid #475569;"
        "    border-top-right-radius: 4px;"
        "    border-bottom-right-radius: 4px;"
        "}"
        "QComboBox::down-arrow {"
        "    image: %2;" // ʹ���� SpinBox ��ͬ��ͼ��
        "    width: 12px;"
        "    height: 12px;"
        "}"
        "QComboBox QAbstractItemView {" /* �����б� */
        "    background-color: #28354C;"
        "    color: #E2E8F0;"
        "    border: 1px solid #475569;"
        "    selection-background-color: #3B82F6;" // ѡ�����
        "    selection-color: #FFFFFF;" // ѡ��������
        "    padding: 4px;"
        "}"
        // --- QCheckBox ��ʽ ---
        "QCheckBox {"
        "    color: #E2E8F0;" // ��ѡ��������ɫ
        "    spacing: 8px;" // ���ֺ�ָʾ�����
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
        "    background-color: #3B82F6;" // ѡ��ʱ��ɫ
        "    border-color: #3B82F6;"
        //"    image: url(:/icons/check_light.svg);" // ��Ҫ��ɫ����ͼ��
        "}"
        "QCheckBox::indicator:unchecked:hover {"
        "    border-color: #60A5FA;" // ��ͣ�߿�
        "}"
        "QCheckBox::indicator:checked:hover {"
        "    background-color: #60A5FA;" // ѡ��ʱ��ͣ��ɫ
        "    border-color: #60A5FA;"
        "}"
        "QCheckBox::indicator:indeterminate {" // ����ѡ��״̬
        "    background-color: #475569;"
        "    border-color: #60A5FA;"
        //"     image: url(:/icons/minus_light.svg);" // ��Ҫ��ɫ����ͼ��
        "}"
        // --- QPushButton (������ɫ��) ---
        // ��ʽ���� updateButtonColor ������
    ).arg(upArrowUrlCorrected, downArrowUrlCorrected /*, ... ����ͼ��� URL */);
    setupUI();
    this->setStyleSheet(styleSheetString);
    // ��ʼ״̬���������飬ֱ���ж�����ѡ�� �� ��ʾ��������
    // setEnabled(false); // �Ȳ����ã��û�������һֱ�ɼ�
    // �Ժ�����Ƿ���ѡ���������÷ǻ�����
    if (borderGroup_ptr) borderGroup_ptr->setEnabled(false);
    if (fillGroup_ptr) fillGroup_ptr->setEnabled(false);
    if (lineGroup_ptr) lineGroup_ptr->setEnabled(false);
    if (textGroup_ptr) textGroup_ptr->setEnabled(false);
    if (canvasGroup_ptr) canvasGroup_ptr->setEnabled(true);
    retranslateUi();
}
void FormatPanel::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi(); // <--- ���� retranslateUi �����ı�
    }
    QWidget::changeEvent(event); // ���û����ʵ��
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

// --- ���� UI �ĺ��� ---

void FormatPanel::updateSelectedShapes(const QList<Shape*>& shapes) {
    bool hasSelection = !shapes.isEmpty();          // �Ƿ���ѡ����
    bool singleShapeSelected = shapes.size() == 1; // �Ƿ�ֻѡ����һ��ͼ��

    // === 1. �����Ƿ���ѡ�������/���ÿؼ��� ===
    if (borderGroup_ptr) borderGroup_ptr->setEnabled(hasSelection);
    if (fillGroup_ptr) fillGroup_ptr->setEnabled(hasSelection);
    // �ı��� (textGroup_ptr) ������״̬���ں�������Ƿ�ȫΪ�ı���������
    if (shapeWidthSpin_ptr) shapeWidthSpin_ptr->setEnabled(singleShapeSelected);  // ͼ�ο�ȿؼ����ڵ�ѡʱ����
    if (shapeHeightSpin_ptr) shapeHeightSpin_ptr->setEnabled(singleShapeSelected); // ͼ�θ߶ȿؼ����ڵ�ѡʱ����

    // === 2. ���û��ѡ����������пؼ�ΪĬ��/����״̬ ===
    if (!hasSelection) {
        // ���ñ߿���
        if (borderButton_ptr) updateButtonColor(borderButton_ptr, Qt::black);
        if (borderWidthCombo_ptr) borderWidthCombo_ptr->setCurrentIndex(-1); // ���ѡ��
        if (borderStyleCombo_ptr) borderStyleCombo_ptr->setCurrentIndex(-1); // ���ѡ��
        // ���������
        if (fillCheckBox_ptr) fillCheckBox_ptr->setCheckState(Qt::Unchecked);
        if (fillButton_ptr) updateButtonColor(fillButton_ptr, Qt::white);
        // ���óߴ��� (����ֹ�źŴ���)
        if (shapeWidthSpin_ptr) {
            shapeWidthSpin_ptr->blockSignals(true);
            shapeWidthSpin_ptr->setValue(shapeWidthSpin_ptr->minimum()); // ��Ϊ��Сֵ
            shapeWidthSpin_ptr->blockSignals(false);
        }
        if (shapeHeightSpin_ptr) {
            shapeHeightSpin_ptr->blockSignals(true);
            shapeHeightSpin_ptr->setValue(shapeHeightSpin_ptr->minimum()); // ��Ϊ��Сֵ
            shapeHeightSpin_ptr->blockSignals(false);
        }
        // �����ı���
        if (textGroup_ptr) textGroup_ptr->setEnabled(false); // �����ı���
        if (textButton_ptr) updateButtonColor(textButton_ptr, Qt::black);
        if (fontCombo_ptr) fontCombo_ptr->setCurrentIndex(-1); // �������ѡ��
        if (fontSizeSpin_ptr) fontSizeSpin_ptr->setValue(fontSizeSpin_ptr->minimum()); // �ֺ���Ϊ��Сֵ (��ʾΪ��)

        return; // û��ѡ������������������
    }

    // === 3. ����ѡ��������� ===
    bool multiSelection = shapes.size() > 1; // �Ƿ�Ϊ��ѡ
    Shape* firstShape = shapes.first();     // ��ȡ��һ��ѡ�е�ͼ����Ϊ��׼
    if (!firstShape) {
        // �����һ��ͼ����Ч�������ϲ�Ӧ�����������������鲢����
        if (borderGroup_ptr) borderGroup_ptr->setEnabled(false);
        if (fillGroup_ptr) fillGroup_ptr->setEnabled(false);
        if (lineGroup_ptr) lineGroup_ptr->setEnabled(false); // Ҳ���������飨��Ȼ�˺���������
        if (textGroup_ptr) textGroup_ptr->setEnabled(false);
        // ������ͨ����������
        return;
    }

    // --- 3.1 ��ȡ��һ��ͼ�ε����� ---
    QColor firstBorderColor = firstShape->getBorderColor();
    Qt::PenStyle firstBorderStyle = firstShape->getBorderStyle();
    qreal firstBorderWidth = firstShape->getBorderWidth();
    bool firstFillState = firstShape->isFilled();
    QColor firstFillColor = firstShape->getFillColor();

    // --- 3.2 ����ѡʱ�����Ƿ�һ�� ---
    bool borderColorConsistent = true;
    bool borderStyleConsistent = true;
    bool borderWidthConsistent = true;
    bool fillStateConsistent = true;
    bool fillColorConsistent = true;

    if (multiSelection) {
        for (int i = 1; i < shapes.size(); ++i) {
            Shape* currentShape = shapes.at(i);
            if (!currentShape) continue; // ������Чͼ��
            if (currentShape->getBorderColor() != firstBorderColor) borderColorConsistent = false;
            if (currentShape->getBorderStyle() != firstBorderStyle) borderStyleConsistent = false; // ���߿���ʽ
            if (!qFuzzyCompare(currentShape->getBorderWidth(), firstBorderWidth)) borderWidthConsistent = false; // ʹ�ø���Ƚ�
            if (currentShape->isFilled() != firstFillState) fillStateConsistent = false;
            if (currentShape->getFillColor() != firstFillColor) fillColorConsistent = false;
            // ����������Զ��Ѿ���һ�£�������ǰ�˳�ѭ������΢�Ż���
            // if (!borderColorConsistent && !borderStyleConsistent && !borderWidthConsistent && !fillStateConsistent && !fillColorConsistent) break;
        }
    }

    // === 4. ���� UI �ؼ�״̬ ===

    // --- 4.1 ���±߿��� UI ---
    // �߿���ɫ��ť
    if (borderButton_ptr) {
        borderButton_ptr->setEnabled(true); // ֻҪ��ѡ�о�����
        if (borderColorConsistent) {
            updateButtonColor(borderButton_ptr, firstBorderColor);
            borderButton_ptr->setToolTip(firstBorderColor.name());
        }
        else {
            updateButtonColor(borderButton_ptr, QColor()); // ��Ч��ɫ��ʾ���̸�
            borderButton_ptr->setToolTip(tr("Border colors are inconsistent", "�߿���ɫ��һ��")); // ʹ�� tr()
        }
    }

    // �߿���������
    bool isEffectivelyNoBorder = false; // ����Ƿ�Ӧ��Ϊ�ޱ߿�
    if (borderWidthCombo_ptr) {
        borderWidthCombo_ptr->blockSignals(true);
        if (borderWidthConsistent) {
            if (qFuzzyCompare(firstBorderWidth, 0)) {
                // ���Ҳ�ѡ�� "�ޱ߿�" ѡ��
                int noBorderIndex = borderWidthCombo_ptr->findText(tr("No Border", "�ޱ߿�"));
                borderWidthCombo_ptr->setCurrentIndex(noBorderIndex != -1 ? noBorderIndex : 0);
                isEffectivelyNoBorder = true;
            }
            else {
                QString widthStr = QString::number(firstBorderWidth, 'f', 1);
                int index = borderWidthCombo_ptr->findText(widthStr);
                borderWidthCombo_ptr->setCurrentIndex(index); // ѡ�ж�Ӧ��ȣ��Ҳ�����Ϊ -1 (��ʾ�հ�)
                isEffectivelyNoBorder = false;
            }
        }
        else {
            borderWidthCombo_ptr->setCurrentIndex(-1); // ��Ȳ�һ�£���ʾ�հ�
            isEffectivelyNoBorder = false; // ����ȷ���Ƿ��ޱ߿�
        }
        borderWidthCombo_ptr->blockSignals(false);
    }

    // �߿���ʽ������ (��״̬�����ڱ߿���)
    if (borderStyleCombo_ptr) {
        borderStyleCombo_ptr->blockSignals(true);
        if (isEffectivelyNoBorder) {
            // ���ȷ�����ޱ߿�ѡ�� "����ʽ" ������
            int noStyleIndex = borderStyleCombo_ptr->findText(tr("No Style", "����ʽ"));
            borderStyleCombo_ptr->setCurrentIndex(noStyleIndex != -1 ? noStyleIndex : 0);
            borderStyleCombo_ptr->setEnabled(false);
        }
        else {
            // ����߿��ȷ� 0 ��һ�£�������ʽѡ��
            borderStyleCombo_ptr->setEnabled(true);
            if (borderStyleConsistent) {
                // ��ʽһ�£����Ҳ�ѡ�ж�Ӧ��ʽ����
                QString styleName;
                // ע�⣺����ʹ�� borderStyleMap_data ��Ա����
                for (auto it = borderStyleMap_data.constBegin(); it != borderStyleMap_data.constEnd(); ++it) {
                    if (it.value() == firstBorderStyle) {
                        styleName = it.key(); // key Ӧ�����Ѿ�������� (�� retranslateUi ������)
                        break;
                    }
                }
                int index = borderStyleCombo_ptr->findText(styleName);
                borderStyleCombo_ptr->setCurrentIndex(index); // �Ҳ�������ʾ�հ�
            }
            else {
                borderStyleCombo_ptr->setCurrentIndex(-1); // ��ʽ��һ�£���ʾ�հ�
            }
        }
        borderStyleCombo_ptr->blockSignals(false);
    }

    // ͼ�γߴ� SpinBox (����ѡʱ����ֵ)
    if (shapeWidthSpin_ptr && shapeHeightSpin_ptr) {
        shapeWidthSpin_ptr->blockSignals(true);
        shapeHeightSpin_ptr->blockSignals(true);
        if (singleShapeSelected) {
            QRect currentRect = firstShape->getRect();
            shapeWidthSpin_ptr->setValue(currentRect.width());
            shapeHeightSpin_ptr->setValue(currentRect.height());
        }
        else {
            // ��ѡʱ����Ϊ��Сֵ���򱣳ֽ���ʱ��ֵ��
            shapeWidthSpin_ptr->setValue(shapeWidthSpin_ptr->minimum());
            shapeHeightSpin_ptr->setValue(shapeHeightSpin_ptr->minimum());
        }
        shapeWidthSpin_ptr->blockSignals(false);
        shapeHeightSpin_ptr->blockSignals(false);
    }

    // --- 4.2 ��������� UI ---
    // "�����" ��ѡ��
    if (fillCheckBox_ptr) {
        fillCheckBox_ptr->blockSignals(true);
        if (fillStateConsistent) {
            // ״̬һ�£����ݵ�һ��ͼ�ε�״̬���� (ע���߼���ת)
            fillCheckBox_ptr->setCheckState(!firstFillState ? Qt::Checked : Qt::Unchecked);
        }
        else {
            fillCheckBox_ptr->setCheckState(Qt::PartiallyChecked); // ״̬��һ�£�����ѡ��
        }
        fillCheckBox_ptr->setEnabled(true); // ��ѡ��ʼ�����ã�ֻҪ�����ã�
        fillCheckBox_ptr->blockSignals(false);
    }

    // �����ɫ��ť
    if (fillButton_ptr) {
        fillButton_ptr->setEnabled(true); // ��ɫ��ťʼ�����ã�ֻҪ�����ã�
        if (fillStateConsistent && firstFillState) { // ״̬һ���ҡ�����䡿
            if (fillColorConsistent) {
                updateButtonColor(fillButton_ptr, firstFillColor);
                fillButton_ptr->setToolTip(firstFillColor.name());
            }
            else {
                updateButtonColor(fillButton_ptr, QColor()); // ��Ч��ɫ
                fillButton_ptr->setToolTip(tr("Fill colors are inconsistent", "�����ɫ��һ��")); // tr()
            }
        }
        else if (fillStateConsistent && !firstFillState) { // ״̬һ���ҡ�����䡿
            updateButtonColor(fillButton_ptr, QColor(230, 230, 230)); // ��ʾһ����ʾ���ޡ��Ļ�ɫ
            fillButton_ptr->setToolTip(tr("No fill", "�����")); // tr()
        }
        else { // ���״̬��һ��
            updateButtonColor(fillButton_ptr, QColor()); // ��Ч��ɫ
            fillButton_ptr->setToolTip(tr("Fill states are inconsistent", "���״̬��һ��")); // tr()
        }
    }

    // --- 4.3 �����ı��� UI ---
    // �����ж��Ƿ�����ѡ�еĶ����ı���
    bool containsTextBox = false;
    bool allAreTextBoxes = hasSelection; // ��ʼ����Ϊ�棨��� hasSelection Ϊ false ���ֵΪ false��
    QFont firstFont;
    QColor firstTextColor;
    TextBox* firstTextBox = dynamic_cast<TextBox*>(firstShape); // ����ת����һ��

    if (firstTextBox) {
        containsTextBox = true;
        firstFont = firstTextBox->getFont();
        firstTextColor = firstTextBox->getTextColor();
    }
    else {
        allAreTextBoxes = false; // ��һ���Ͳ����ı���
    }

    // ����Ƕ�ѡ�����ʣ����Ƿ�Ҳ�����ı��򣬲��������һ����
    bool fontConsistent = true;
    bool textColorConsistent = true;
    if (multiSelection && allAreTextBoxes) { // ֻ���ڵ�һ�����ı������Ƕ�ѡʱ����Ҫ�������
        for (int i = 1; i < shapes.size(); ++i) {
            TextBox* currentTB = dynamic_cast<TextBox*>(shapes.at(i));
            if (!currentTB) {
                allAreTextBoxes = false; // ����һ�������ı���
                fontConsistent = false;
                textColorConsistent = false;
                break; // �����ټ��
            }
            // ��������ı��򣬼������һ����
            if (fontConsistent && currentTB->getFont() != firstFont) fontConsistent = false;
            if (textColorConsistent && currentTB->getTextColor() != firstTextColor) textColorConsistent = false;
        }
    }
    else if (!firstTextBox && hasSelection) {
        // �����һ���Ͳ����ı�����϶����ǡ�ȫ���ı���
        allAreTextBoxes = false;
    }

    // ���ݼ��������/�����ı��鲢���ÿؼ�״̬
    bool enableTextGroup = containsTextBox && allAreTextBoxes;
    if (textGroup_ptr) textGroup_ptr->setEnabled(enableTextGroup);

    if (enableTextGroup) {
        // ������������ֺ� SpinBox
        if (fontCombo_ptr && fontSizeSpin_ptr) {
            fontCombo_ptr->blockSignals(true);
            fontSizeSpin_ptr->blockSignals(true);
            if (fontConsistent) {
                fontCombo_ptr->setCurrentFont(firstFont);
                int currentSize = firstFont.pointSize() > 0 ? firstFont.pointSize() : firstFont.pixelSize();
                // ȷ���ֺ��� SpinBox ��Χ��
                currentSize = qBound(fontSizeSpin_ptr->minimum(), currentSize, fontSizeSpin_ptr->maximum());
                fontSizeSpin_ptr->setValue(currentSize > 0 ? currentSize : 10); // ��ֵ��ȷ����Ĭ��
            }
            else {
                // ���岻һ�£������ʾ
                fontCombo_ptr->setCurrentIndex(-1); // ������� lineEdit
                if (fontCombo_ptr->lineEdit()) fontCombo_ptr->lineEdit()->clear();
                fontSizeSpin_ptr->setValue(fontSizeSpin_ptr->minimum()); // �ֺ���Ϊ��Сֵ (��ʾΪ��)
            }
            fontCombo_ptr->blockSignals(false);
            fontSizeSpin_ptr->blockSignals(false);
        }
        // �ı���ɫ��ť
        if (textButton_ptr) {
            textButton_ptr->setEnabled(true); // ������ɫ��ť
            if (textColorConsistent) {
                updateButtonColor(textButton_ptr, firstTextColor);
                textButton_ptr->setToolTip(firstTextColor.name());
            }
            else {
                updateButtonColor(textButton_ptr, QColor()); // ��Ч��ɫ
                textButton_ptr->setToolTip(tr("Text colors are inconsistent", "�ı���ɫ��һ��")); // tr()
            }
        }
    }
    else {
        // ����ı��鱻���ã����ÿؼ�״̬
        if (fontCombo_ptr) fontCombo_ptr->setCurrentIndex(-1);
        if (fontSizeSpin_ptr) fontSizeSpin_ptr->setValue(fontSizeSpin_ptr->minimum());
        if (textButton_ptr) updateButtonColor(textButton_ptr, Qt::black);
    }

    update(); 
}

void FormatPanel::updateSelectedLines(const QList<ConnectionLine*>& lines) {
    bool hasSelection = !lines.isEmpty(); // �Ƿ���ѡ�е�����

    // === 1. �����Ƿ���ѡ������������/�������������� ===
    if (lineGroup_ptr) lineGroup_ptr->setEnabled(hasSelection);

    // === 2. ���û��ѡ�������������ؼ�ΪĬ��/����״̬ ===
    if (!hasSelection) {
        if (lineColorButton_ptr) updateButtonColor(lineColorButton_ptr, Qt::black);
        if (lineWidthCombo_ptr) lineWidthCombo_ptr->setCurrentIndex(-1); // ���ѡ��
        if (lineStyleCombo_ptr) lineStyleCombo_ptr->setCurrentIndex(-1); // ���ѡ��
        if (arrowStyleCombo_ptr) {
            arrowStyleCombo_ptr->setCurrentIndex(-1); // ���ѡ��
            arrowStyleCombo_ptr->setEnabled(false); // ���ü�ͷ��ʽѡ��
        }
        // ע�⣺���ﲻ��Ҫ���� lineGroup_ptr ������Ϊ�Ѿ��ڲ��� 1 ������
        return; // û��ѡ����������
    }

    // === 3. ����ѡ������������ ===
    bool multiSelection = lines.size() > 1;         // �Ƿ�Ϊ��ѡ
    ConnectionLine* firstLine = lines.first();      // ��ȡ��һ��ѡ�е�������Ϊ��׼
    if (!firstLine) {
        // �����һ��������Ч�����������鲢����
        if (lineGroup_ptr) lineGroup_ptr->setEnabled(false);
        return;
    }

    // --- 3.1 ��ȡ��һ������������ ---
    QColor firstLineColor = firstLine->getColor();
    qreal firstLineWidth = firstLine->getLineWidth();
    Qt::PenStyle firstLineStyle = firstLine->getLineStyle();
    bool firstArrowState = firstLine->hasArrowHead(); // ��ȡ��ͷ״̬

    // --- 3.2 ����ѡʱ�����Ƿ�һ�� ---
    bool colorConsistent = true;
    bool widthConsistent = true;
    bool styleConsistent = true;
    bool arrowConsistent = true; // ����ͷ״̬һ����

    if (multiSelection) {
        for (int i = 1; i < lines.size(); ++i) {
            ConnectionLine* currentLine = lines.at(i);
            if (!currentLine) continue; // ������Ч����
            if (currentLine->getColor() != firstLineColor) colorConsistent = false;
            if (!qFuzzyCompare(currentLine->getLineWidth(), firstLineWidth)) widthConsistent = false; // ����Ƚ�
            if (currentLine->getLineStyle() != firstLineStyle) styleConsistent = false;
            if (currentLine->hasArrowHead() != firstArrowState) arrowConsistent = false; // ����ͷ
            // ��ѡ�Ż�: if (!colorConsistent && !widthConsistent && ...) break;
        }
    }

    // === 4. ���� UI �ؼ�״̬ ===

    // --- 4.1 ����������ɫ��ť ---
    if (lineColorButton_ptr) {
        lineColorButton_ptr->setEnabled(true); // ֻҪ��ѡ�о�����
        if (colorConsistent) {
            updateButtonColor(lineColorButton_ptr, firstLineColor);
            lineColorButton_ptr->setToolTip(firstLineColor.name());
        }
        else {
            updateButtonColor(lineColorButton_ptr, QColor()); // ��Ч��ɫ
            lineColorButton_ptr->setToolTip(tr("Line colors are inconsistent", "������ɫ��һ��")); // tr()
        }
    }

    // --- 4.2 ����������������� ---
    if (lineWidthCombo_ptr) {
        lineWidthCombo_ptr->blockSignals(true);
        if (widthConsistent) {
            QString widthStr = QString::number(firstLineWidth, 'f', 1);
            int index = lineWidthCombo_ptr->findText(widthStr);
            lineWidthCombo_ptr->setCurrentIndex(index); // �Ҳ�����Ϊ -1 (��ʾ�հ�)
        }
        else {
            lineWidthCombo_ptr->setCurrentIndex(-1); // ��Ȳ�һ�£���ʾ�հ�
        }
        lineWidthCombo_ptr->blockSignals(false);
    }

    // --- 4.3 ����������ʽ������ ---
    if (lineStyleCombo_ptr) {
        lineStyleCombo_ptr->blockSignals(true);
        if (styleConsistent) {
            QString styleName;
            // ע�⣺ʹ�� lineStyleMap_data ��Ա����
            for (auto it = lineStyleMap_data.constBegin(); it != lineStyleMap_data.constEnd(); ++it) {
                if (it.value() == firstLineStyle) {
                    styleName = it.key(); // key Ӧ�����ѷ���� (���� retranslateUi)
                    break;
                }
            }
            int index = lineStyleCombo_ptr->findText(styleName);
            lineStyleCombo_ptr->setCurrentIndex(index); // �Ҳ�����Ϊ -1 (��ʾ�հ�)
        }
        else {
            lineStyleCombo_ptr->setCurrentIndex(-1); // ��ʽ��һ�£���ʾ�հ�
        }
        lineStyleCombo_ptr->blockSignals(false);
    }

    // --- 4.4 ���¼�ͷ��ʽ������ ---
    if (arrowStyleCombo_ptr) {
        arrowStyleCombo_ptr->blockSignals(true);
        arrowStyleCombo_ptr->setEnabled(true); // ֻҪ��ѡ�о�����
        if (arrowConsistent) {
            // ״̬һ�£����ݵ�һ��������״̬����ѡ����
            if (firstArrowState) {
                // ���Ҳ�ѡ�� "��ͷ" ѡ��
                int arrowIndex = arrowStyleCombo_ptr->findText(tr("Arrow", "��ͷ"));
                arrowStyleCombo_ptr->setCurrentIndex(arrowIndex != -1 ? arrowIndex : 0);
            }
            else {
                // ���Ҳ�ѡ�� "ֱ��" ѡ��
                int lineIndex = arrowStyleCombo_ptr->findText(tr("Straight Line", "ֱ��"));
                arrowStyleCombo_ptr->setCurrentIndex(lineIndex != -1 ? lineIndex : 0);
            }
        }
        else {
            // ��ͷ״̬��һ�£����ѡ����
            arrowStyleCombo_ptr->setCurrentIndex(-1);
        }
        arrowStyleCombo_ptr->blockSignals(false);
    }

    // update(); // ͨ������Ҫ��ʽ���� update()
}

// --- �ۺ���ʵ�� (���� UI �仯�������ź�) ---

void FormatPanel::onBorderWidthChanged(const QString& text) {
    if (text.isEmpty()) return;

    qreal width = 0; 
    if (text != tr("No Border")) {
        bool ok;
        width = text.toDouble(&ok);
        if (!ok || width < 0) return; // ת��ʧ�ܻ���Чֵ�����
    }
    // *** �����߼������ݿ�ȸ�����ʽ������״̬ ***
    borderStyleCombo_ptr->blockSignals(true); // ��ʱ��ֹ�ź�
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
    borderStyleCombo_ptr->blockSignals(false); // �ָ��ź�
    emit borderWidthChanged(width);
}

// --- ʵ���µĲۺ��� ---
void FormatPanel::onBorderStyleChanged(const QString& text) {
    // ���ָ���Ƿ���Ч
    if (!borderStyleCombo_ptr || !borderGroup_ptr || !borderGroup_ptr->isEnabled()) {
        // ����ؼ������ڣ����߱߿��鱻���ã��򲻴���
        return;
    }
    if (text.isEmpty() || text == tr("No Style", "����ʽ") || !borderStyleMap_data.contains(text)) {
        return;
    }
    Qt::PenStyle style = borderStyleMap_data.value(text);

    // �����źţ�֪ͨ�ⲿ�߿���ʽ�Ѹı�
    emit borderStyleChanged(style);
}

void FormatPanel::onFillToggled(bool checked) {
    // �����ؿؼ�ָ���Ƿ���Ч
    if (!fillButton_ptr || !fillGroup_ptr || !fillGroup_ptr->isEnabled()) {
        // ����ؼ������ڻ�����鱻���ã��򲻴���
        return;
    }
    bool isFilled = checked;

    emit fillStateChanged(isFilled);

 
}

void FormatPanel::onLineWidthChanged(const QString& text) {
    if (text.isEmpty()) return;
    bool ok;
    qreal width = text.toDouble(&ok);
    if (ok && width >= 0.1) { // ȷ�������Ч
        emit lineWidthChanged(width);
    }
}

void FormatPanel::onLineStyleChanged(const QString& text) {
    if (text.isEmpty() || !lineStyleMap_data.contains(text)) return;
    Qt::PenStyle style = lineStyleMap_data.value(text);

    emit lineStyleChanged(style);
}

// --- �����ۺ���ʵ�� ---
void FormatPanel::onArrowStyleChanged(const QString& text) {
    if (text.isEmpty()) return; // ������ setCurrentIndex(-1) ���µĿ��ı�

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

    // �����źţ�����ת����Ĳ���ֵ
    emit arrowStateChanged(hasArrow);
}

void FormatPanel::onFontChanged(const QFont& font) {
    if (!textGroup_ptr->isEnabled()) return; // ����ı���δ���ã������ź�

    // �����ֺ� SpinBox ��ƥ��
    int newSize = font.pointSize();
    if (newSize <= 0) newSize = font.pixelSize();
    if (newSize > 0 && fontSizeSpin_ptr->value() != newSize) {
        fontSizeSpin_ptr->blockSignals(true);
        if (newSize >= fontSizeSpin_ptr->minimum() && newSize <= fontSizeSpin_ptr->maximum()) {
            fontSizeSpin_ptr->setValue(newSize);
        }
        else {
            fontSizeSpin_ptr->setValue(0); // ������Χ��Ϊ0������ֵ
        }
        fontSizeSpin_ptr->blockSignals(false);
    }
    emit fontChanged(font); // �����������������Ϣ���ź�
}

void FormatPanel::onFontSizeChanged(int size) {
    if (!textGroup_ptr->isEnabled() || size <= 0) return; // ����ı���δ���û��С��Ч������

    QFont currentFont = fontCombo_ptr->currentFont();
    bool changed = false;

    // ����ǵ��С�������ش�С�ı�
    if (currentFont.pointSize() > 0) {
        if (currentFont.pointSize() != size) {
            currentFont.setPointSize(size);
            changed = true;
        }
    }
    else { // ʹ�����ش�С
        if (currentFont.pixelSize() != size) {
            currentFont.setPixelSize(size);
            changed = true;
        }
    }

    if (changed) {
        // ���� FontComboBox ����ֹ���ٴδ��� onFontChanged
        fontCombo_ptr->blockSignals(true);
        fontCombo_ptr->setCurrentFont(currentFont);
        fontCombo_ptr->blockSignals(false);
        // �����������������Ϣ���ź�
        emit fontChanged(currentFont);
    }
}

// --- �������� ---
// --- **�޸�:** �������� updateButtonColor ---
void FormatPanel::updateButtonColor(QPushButton* button, const QColor& color) {
    if (!button) return;

    QString styleSheet =
        "QPushButton {"
        "   background-color: %1;" // ��ɫ�����̸�
        "   border: 1px solid #475569;" // ͳһ�߿�
        "   border-radius: 4px;"
        "   min-height: 24px;" // ͳһ�߶�
        "   padding: 4px;"
        "}"
        "QPushButton:hover {"
        "    border-color: #60A5FA;" // ��ͣʱ�߿��ǳ��    
        "}"
        "QPushButton:pressed {"
        "    background-color: #334155;" // ����ʱ�䰵
        "}";

    if (!color.isValid()) {
        // ʹ�����̸񱳾���ʾ��Ч����ɫ
        styleSheet = styleSheet.arg("qlineargradient(spread:repeat, x1:0, y1:0, x2:10, y2:10,"
            "stop:0 #CCCCCC, stop:0.5 #CCCCCC,"
            "stop:0.5 #E0E0E0, stop:1 #E0E0E0)");
        button->setToolTip(tr("Color is inconsistent"));
    }
    else if (color == Qt::transparent || color.alpha() == 0) {
        // ʹ�����̸񱳾���ʾ͸��ɫ
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
    button->setText(""); // ʼ�����ı�
}

void FormatPanel::onFillCheckboxToggled(bool checked) {
    // checked Ϊ true ��ζ�� "�����" ����ѡ��
    bool isNowFilled = !checked; // ʵ�ʵ����״̬�븴ѡ��״̬�෴

    //qDebug() << "FormatPanel: '�����' checkbox toggled. Checked (No Fill):" << checked << ", Is Now Filled:" << isNowFilled;

    if (fillButton_ptr) {
        fillButton_ptr->setEnabled(true); // ������������״̬����������ɫ��ť���������
    }

    // ���� fillStateChanged �źţ����ݡ�ʵ�ʡ������״̬
    emit fillStateChanged(isNowFilled);
    emit changeNofillStateTriggered();
}

// --- ʵ�ֻ���������صĲۺ��� ---
// �ڲ��ۣ����û��༭��������ʱ����
void FormatPanel::onCanvasWidthEdited(int value) {
    qDebug() << tr("FormatPanel: Canvas width edited in panel:") << value;
    emit canvasWidthChanged(value); // �����ź�֪ͨ�ⲿ
}

// �ڲ��ۣ����û��༭�߶������ʱ����
void FormatPanel::onCanvasHeightEdited(int value) {
    qDebug() << tr("FormatPanel: Canvas height edited in panel:") << value;
    emit canvasHeightChanged(value); // �����ź�֪ͨ�ⲿ
}

// �ڲ��ۣ����û��༭���������ʱ����
void FormatPanel::onCanvasZoomEdited(double value) {
    qDebug() << tr("FormatPanel: Canvas zoom edited in panel:") << value << "%";
    emit canvasZoomChanged(value); // �����ź�֪ͨ�ⲿ (���Ͱٷֱ�)
}

// �����ۣ������ⲿ�Ļ������ԣ�����UI��ʾ
void FormatPanel::updateCanvasProperties(const QSize& size, double currentZoomFactor) {
    qDebug() << tr("FormatPanel: Updating canvas properties UI. Size:") << size << tr("Zoom Factor:") << currentZoomFactor;

    // ��ֹ�źţ���ֹ����ֵʱ���� valueChanged �ٴη����ź�
    canvasWidthSpin_ptr->blockSignals(true);
    canvasHeightSpin_ptr->blockSignals(true);
    canvasZoomSpin_ptr->blockSignals(true);

    canvasWidthSpin_ptr->setValue(size.width());
    canvasHeightSpin_ptr->setValue(size.height());
    canvasZoomSpin_ptr->setValue(currentZoomFactor * 100.0); // ������תΪ�ٷֱ�

    // �ָ��ź�
    canvasWidthSpin_ptr->blockSignals(false);
    canvasHeightSpin_ptr->blockSignals(false);
    canvasZoomSpin_ptr->blockSignals(false);
}

void FormatPanel::onShapeWidthChanged(int value) {
    qDebug() << tr("FormatPanel: Shape width edited in panel:") << value;
    // ���������������֤������ȷ�� value ����ĳ����Сֵ
    if (value >= 10) { // ������С���Ϊ 10
        emit shapeWidthChanged(value); // �����ź�֪ͨ DiagramEditor
    }
    else {
        // ���ֵ��Ч�����Իָ���ֵ�����κβ���
        // QTimer::singleShot(0, [this, oldValue](){ shapeWidthSpin->setValue(oldValue); }); // �ӳٻָ�
    }
}

void FormatPanel::onShapeHeightChanged(int value) {
    qDebug() << tr("FormatPanel: Shape height edited in panel:") << value;
    if (value >= 10) { // ������С�߶�Ϊ 10
        emit shapeHeightChanged(value); // �����ź�֪ͨ DiagramEditor
    }
    else {
        // ������Чֵ
    }
}