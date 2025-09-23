#ifndef FORMATPANEL_H
#define FORMATPANEL_H

#include <QWidget>
#include <QColor>
#include <QFont>
#include <QMap>
#include <Qt>

#define NO_BORDER_STYLE_TEXT QT_TRANSLATE_NOOP("FormatPanel", "No Style")

class QComboBox;
class QPushButton;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QFontComboBox;
class QGroupBox;
class QLabel;
class Shape;
class ConnectionLine;

class FormatPanel : public QWidget {
    Q_OBJECT

public:
    explicit FormatPanel(QWidget* parent = nullptr);

signals:
    void changeBorderColorTriggered();
    void changeFillColorTriggered();
    void changeLineColorTriggered();
    void changeTextColorTriggered();
    void changeNofillStateTriggered();
    void shapeWidthChanged(int width);
    void shapeHeightChanged(int height);

    void borderWidthChanged(qreal width);
    void borderStyleChanged(Qt::PenStyle style);
    void fillStateChanged(bool isFilled);
    void lineWidthChanged(qreal width);
    void lineStyleChanged(Qt::PenStyle style);
    void arrowStateChanged(bool hasArrow);
    void fontChanged(const QFont& font);

    void canvasWidthChanged(int width);
    void canvasHeightChanged(int height);
    void canvasZoomChanged(double zoomPercentage);

public slots:
    void updateSelectedShapes(const QList<Shape*>& shapes);
    void updateSelectedLines(const QList<ConnectionLine*>& lines);
    void updateCanvasProperties(const QSize& size, double currentZoomFactor);

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void onBorderWidthChanged(const QString& text);
    void onBorderStyleChanged(const QString& text);
    void onFillCheckboxToggled(bool checked);
    void onFillToggled(bool checked);
    void onShapeWidthChanged(int value);
    void onShapeHeightChanged(int value);
    void onLineWidthChanged(const QString& text);
    void onLineStyleChanged(const QString& text);
    void onArrowStyleChanged(const QString& text);
    void onFontChanged(const QFont& font);
    void onFontSizeChanged(int size);

    void onCanvasWidthEdited(int value);
    void onCanvasHeightEdited(int value);
    void onCanvasZoomEdited(double value);

private:
    void setupUI();
    void retranslateUi();
    void updateButtonColor(QPushButton* button, const QColor& color);

    QLabel* titleLabel_ptr;

    QGroupBox* borderGroup_ptr, * fillGroup_ptr, * lineGroup_ptr, * textGroup_ptr, * canvasGroup_ptr;

    QLabel* labelBorderColor_ptr, * labelBorderWidth_ptr, * labelBorderStyle_ptr, * labelShapeWidth_ptr, * labelShapeHeight_ptr;
    QPushButton* borderButton_ptr;
    QComboBox* borderWidthCombo_ptr;
    QComboBox* borderStyleCombo_ptr;
    QSpinBox* shapeWidthSpin_ptr;
    QSpinBox* shapeHeightSpin_ptr;

    QLabel* labelFillColor_ptr;
    QCheckBox* fillCheckBox_ptr;
    QPushButton* fillButton_ptr;

    QLabel* labelLineColor_ptr, * labelLineThickness_ptr, * labelLineStyleUI_ptr, * labelLineEndpointStyle_ptr;
    QPushButton* lineColorButton_ptr;
    QComboBox* lineWidthCombo_ptr;
    QComboBox* lineStyleCombo_ptr;
    QComboBox* arrowStyleCombo_ptr;

    QLabel* labelTextCorlor_ptr, * labelTextFont_ptr, * labelTextSize_ptr;
    QPushButton* textButton_ptr;
    QFontComboBox* fontCombo_ptr;
    QSpinBox* fontSizeSpin_ptr;

    QLabel* labelCanvasWidth_ptr, * labelCanvasHeight_ptr, * labelCanvasZoom_ptr;
    QSpinBox* canvasWidthSpin_ptr;
    QSpinBox* canvasHeightSpin_ptr;
    QDoubleSpinBox* canvasZoomSpin_ptr;

    QMap<QString, Qt::PenStyle> borderStyleMap_data;
    QMap<QString, Qt::PenStyle> lineStyleMap_data;
};

#endif // FORMATPANEL_H