#include "diagrameditor.h"    
#include "diagrameditortool.h" 
#include "diagrameditormeau.h"  
#include "ConnectionLine.h"   
#include "formatpanel.h"
#include "shape.h"
#include <QMessageBox>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QPainter>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include <QFileInfo>
#include <QScrollBar>
#include <QMetaObject>
#include <QDebug>
#include <algorithm> 
#include <QScrollArea>
#include <QApplication> 
#include <QCursor>      
#include <QButtonGroup> 
#include <QUuid>        
#include <limits>       
#include <QLineEdit>    
#include <QClipboard>
#include <QRectF> 
#include <QSvgGenerator> 
#include "additemcommand.h"
#include "movecommand.h"
#include "removeitemscommand.h"
#include "changebordercolorcommand.h"
#include "changefillcolorcommand.h"
#include "changeborderwidthcommand.h"
#include "changetextcommand.h"
#include "pastecommand.h" 
#include "changelinecolorcommand.h"
#include "changelinewidthcommand.h"
#include "resizeshapecommand.h" 
#include "changetextcolorcommand.h"
#include "changefontcommand.h"
#include "alignshapescommand.h" 
#include "changelinestylecommand.h"
#include "changeborderstylecommand.h" 


// MIME ���ͳ������ⲿ����
const QString DIAGRAM_EDITOR_MIME_TYPE = "application/x-diagram-editor-items";
// --- DiagramEditor ʵ�� ---

DiagramEditor::DiagramEditor(QWidget* parent) : QWidget(parent) {

	undoStack = new QUndoStack(this); // ���� UndoStack
	// 1. ��ʼ����������ͻ�ͼ����
	scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(false);
	//scrollArea->setBackgroundRole(QPalette::Dark);
	scrollArea->setStyleSheet("QScrollArea { background-color: #0F172A; border: none; }");


	drawingArea = new DrawingArea(scrollArea, this->undoStack, this);
	drawingArea->setObjectName("DrawingArea");
	setDiagramEditorSize(1600, 1050);
	scrollArea->setWidget(drawingArea);
	scrollArea->setAlignment(Qt::AlignCenter);

	// 2. ��ʼ�������������������
	initToolPanel();
	initFormatPanel();


	// 3. ���������÷ָ���
	splitter = new QSplitter(Qt::Horizontal, this);
	splitter->addWidget(toolscrollArea);
	splitter->addWidget(scrollArea);
	splitter->addWidget(formatScrollArea); // �����ұ���
	splitter->setSizes({ 80, width() - 80 - 250, 250 }); // ������ʼ��� (��������խ)
	splitter->setCollapsible(0, false);
	splitter->setCollapsible(2, false);
	splitter->setStretchFactor(0, 0);
	splitter->setStretchFactor(1, 1);
	splitter->setStretchFactor(2, 0); // ������
	// ��� Splitter ��ʽ
	splitter->setStyleSheet(
		"QSplitter::handle {"
		"    background-color: #1E293B; /* ��屳��ɫ */"
		"    border: 1px solid #334155; /* �ָ�����ɫ */"
		"    width: 3px; /* �ָ��߿�� */"
		"    margin: 1px 0;"
		"}"
		"QSplitter::handle:horizontal {"
		"    width: 3px;"
		"}"
		"QSplitter::handle:vertical {"
		"    height: 3px;"
		"}"
		"QSplitter::handle:hover {"
		"    background-color: #3B82F6; /* ��ͣʱ��ɫ */"
		"}"
	);
	// 4. ����������
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	// --- �޸ģ����� MenuBar ʱ���� undoStack ---
	menuBar = new MenuBar(this->undoStack, this); // <--- ���� undoStack
	menuBar->setStyleSheet(
		"QMenuBar {"
		"    background-color: #1E293B; /* ��屳��ɫ */"
		"    color: #E2E8F0; /* ǳ��ɫ���� */"
		"    spacing: 5px; /* �˵���֮��ļ�� */"
		"    border-bottom: 1px solid #334155; /* ���ɫ�߿� */"
		"}"
		"QMenuBar::item {"
		"    background-color: transparent;" // ����״̬͸��
		"    padding: 5px 10px;"
		"    border-radius: 4px;"
		"}"
		"QMenuBar::item:selected {" /* ��ͣ�����ѡ�� */
		"    background-color: #28354C; /* �������� */"
		"    color: #FFFFFF; /* ��ɫ���� */"
		"}"
		"QMenuBar::item:pressed {" /* ����ʱ */
		"    background-color: #3B82F6; /* ��ɫ */"
		"    color: #FFFFFF;"
		"}"
		// --- QMenu (�����˵�) ��ʽ ---
		"QMenu {"
		"    background-color: #1E293B;"
		"    color: #E2E8F0;"
		"    border: 1px solid #334155;"
		"    padding: 5px;"
		"    margin: 2px;" // �� MenuBar ֮�������϶
		"}"
		"QMenu::item {"
		"    padding: 5px 20px 5px 20px;" // �����ڱ߾�
		"    border-radius: 4px;"
		"}"
		"QMenu::item:selected {"
		"    background-color: #3B82F6; /* ѡ��ʱ��ɫ */"
		"    color: #FFFFFF;"
		"}"
		"QMenu::separator {"
		"    height: 1px;"
		"    background-color: #334155; /* �ָ�����ɫ */"
		"    margin: 4px 0;"
		"}"
	);
	mainLayout->setMenuBar(menuBar);
	mainLayout->addWidget(splitter);
	setLayout(mainLayout);

	// 5. ���Ӳ˵����ź�
	connect(menuBar, &MenuBar::newWindowTriggered, this, &DiagramEditor::createNewWindow);
	connect(menuBar, &MenuBar::openTriggered, this, &DiagramEditor::onOpen);
	connect(menuBar, &MenuBar::saveTriggered, this, &DiagramEditor::onSave);
	connect(menuBar, &MenuBar::swichDrawingArea, this, &DiagramEditor::switchAera);
	connect(menuBar, &MenuBar::deleteTriggered, this, &DiagramEditor::onDelete);
	connect(menuBar, &MenuBar::cutTriggered, this, &DiagramEditor::onCut); // <--- ���Ӽ����ź�
	connect(menuBar, &MenuBar::alignTopTriggered, this, &DiagramEditor::onAlignTop);
	connect(menuBar, &MenuBar::alignVCenterTriggered, this, &DiagramEditor::onAlignVCenter);
	connect(menuBar, &MenuBar::alignBottomTriggered, this, &DiagramEditor::onAlignBottom);

	// ����ͼ������ź�
	connect(menuBar, &MenuBar::moveToTopLayer, drawingArea, &DrawingArea::onMoveToTopLayer);
	connect(menuBar, &MenuBar::moveToBottomLayer, drawingArea, &DrawingArea::onMoveToBottomLayer);
	connect(menuBar, &MenuBar::moveUpOneLayer, drawingArea, &DrawingArea::onMoveUpOneLayer);
	connect(menuBar, &MenuBar::moveDownOneLayer, drawingArea, &DrawingArea::onMoveDownOneLayer);

	// --- ���������Ӹ�ʽ�˵��źŵ� DiagramEditor �Ĳ� ---
	connect(menuBar, &MenuBar::changeBorderColorTriggered, this, &DiagramEditor::onFormatChangeBorderColor);
	connect(menuBar, &MenuBar::changeFillColorTriggered, this, &DiagramEditor::onFormatChangeFillColor);
	connect(menuBar, &MenuBar::toggleFillTriggered, this, &DiagramEditor::onFormatToggleFill);
	connect(menuBar, &MenuBar::changeBorderWidthTriggered, this, &DiagramEditor::onFormatChangeBorderWidth);
	connect(menuBar, &MenuBar::changeLineColorTriggered, this, &DiagramEditor::onFormatChangeLineColor);
	connect(menuBar, &MenuBar::changeLineWidthTriggered, this, &DiagramEditor::onFormatChangeLineWidth);
	connect(menuBar, &MenuBar::changeLineStyleTriggered, this, &DiagramEditor::onFormatChangeLineStyle);
	connect(menuBar, &MenuBar::changeTextFontTriggered, this, &DiagramEditor::onFormatChangeTextFont);
	connect(menuBar, &MenuBar::changeTextColorTriggered, this, &DiagramEditor::onFormatChangeTextColor);
	connect(menuBar, &MenuBar::copyTriggered, this, &DiagramEditor::onCopy);
	connect(menuBar, &MenuBar::pasteTriggered, this, &DiagramEditor::onPaste);

	connect(formatPanel, &FormatPanel::changeBorderColorTriggered, this, &DiagramEditor::onFormatChangeBorderColor);
	connect(formatPanel, &FormatPanel::borderWidthChanged, this, &DiagramEditor::onSelectedShapesBorderWidth);
	connect(formatPanel, &FormatPanel::borderStyleChanged, this, &DiagramEditor::onSelectedShapesBorderStyle); // <--- �������ź�
	connect(formatPanel, &FormatPanel::lineWidthChanged, this, &DiagramEditor::onSelectedLineWidth);
	connect(formatPanel, &FormatPanel::changeFillColorTriggered, this, &DiagramEditor::onFormatChangeFillColor);
	connect(formatPanel, &FormatPanel::shapeWidthChanged, this, &DiagramEditor::onSelectedShapeWidthChanged);
	connect(formatPanel, &FormatPanel::shapeHeightChanged, this, &DiagramEditor::onSelectedShapeHeightChanged);
	connect(formatPanel, &FormatPanel::changeLineColorTriggered, this, &DiagramEditor::onFormatChangeLineColor);
	connect(formatPanel, &FormatPanel::lineStyleChanged, this, &DiagramEditor::onSelectedlineStyleChanged);
	connect(formatPanel, &FormatPanel::arrowStateChanged, this, &DiagramEditor::onSelectedLineArrowStateChanged); // <<<--- ��������

	connect(formatPanel, &FormatPanel::changeNofillStateTriggered, this, &DiagramEditor::onFormatToggleNoFill);
	connect(formatPanel, &FormatPanel::fontChanged, this, &DiagramEditor::onSelectedTextFontChanged);
	connect(formatPanel, &FormatPanel::changeTextColorTriggered, this, &DiagramEditor::onFormatChangeTextColor);
	// --- ���������������� ---
	connect(formatPanel, &FormatPanel::canvasWidthChanged, this, &DiagramEditor::onPanelCanvasWidthChanged);
	connect(formatPanel, &FormatPanel::canvasHeightChanged, this, &DiagramEditor::onPanelCanvasHeightChanged);
	connect(formatPanel, &FormatPanel::canvasZoomChanged, this, &DiagramEditor::onPanelCanvasZoomChanged);

	connect(drawingArea, &DrawingArea::shapeFormat, formatPanel, &FormatPanel::updateSelectedShapes);
	connect(drawingArea, &DrawingArea::lineFormat, formatPanel, &FormatPanel::updateSelectedLines);
	// --- ���������������� ---
	connect(drawingArea, &DrawingArea::zoomChanged, this, &DiagramEditor::onDrawingAreaZoomChanged);
	connect(drawingArea, &DrawingArea::canvasSizeChanged, this, &DiagramEditor::onDrawingAreaSizeChanged);
	
	// 6. ���ӹ�������źŵ� DrawingArea (ͨ�� DiagramEditor ��ת����ģʽ)
	connect(toolPanel, &ToolPanel::shapeToolSelected, this, [this](int shapeType) {
		qDebug() << "DiagramEditor: shapeToolSelected signal, type:" << shapeType;
		if (drawingArea) {
			clickLineMode = false; // ȡ�����������ģʽ

			if (shapeType == -1 || (drawingArea->currentToolMode == DrawingArea::ShapeDrawingTool && drawingArea->currentShapeType == shapeType)) {
				// �����ȡ��(-1) ���� �����ǰ��ѡ�е�ͼ�ι��� -> ����ѡ��ģʽ
				drawingArea->setToolMode(DrawingArea::SelectionTool);
				toolPanel->uncheckAllTools(); // �ֶ�ȡ����������ť״̬
			}
			else {
				// ��������״���Ͳ�������״����ģʽ
				drawingArea->currentShapeType = shapeType;
				drawingArea->setToolMode(DrawingArea::ShapeDrawingTool);
			}
		}
		});

	connect(toolPanel, &ToolPanel::connectionToolActivated, this, [this]() {
		qDebug() << "DiagramEditor: connectionToolActivated signal";
		if (drawingArea) {
			// ��������߹����л�ģʽ
			if (drawingArea->currentToolMode == DrawingArea::ConnectionTool) {
				// �����ǰ������ģʽ�����л���ѡ��ģʽ
				drawingArea->setToolMode(DrawingArea::SelectionTool);
				clickLineMode = false; // ȷ��״̬ͬ��
				// toolPanel->uncheckAllTools(); // ȡ��������ѡ��
			}
			else {
				// ���򣬽�������ģʽ
				drawingArea->currentShapeType = -1; // ���������ģʽ��ͼ������
				drawingArea->setToolMode(DrawingArea::ConnectionTool);
				clickLineMode = true; // ���õ������ģʽ (�����Ҫ)
				// toolPanel->uncheckAllToolsExcept(-2); // -2 ���������߹��ߣ���ҪԼ��
			}
		}
		});

	connect(toolPanel, &ToolPanel::toSelectModel, this, [this]() {
		drawingArea->setToolMode(DrawingArea::SelectionTool);
		drawingArea->selectedLines.clear();
		drawingArea->selectedShapes.clear();

		});

	// 7. ���� DrawingArea ���Ҽ��ź� (����ȡ��������״̬)
	connect(drawingArea, &DrawingArea::clearSelection, this, &DiagramEditor::handleClearSelection);
	// --- ��ʼ�� FormatPanel ��ʾ ---
	updateFormatPanelAll(); // ��������һ�Σ�ȷ����ʼ״̬��ȷ��ʾ
	//retranslateUi();
}


void DiagramEditor::changeEvent(QEvent* event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(); // **���� retranslateUi �����ı�**
	}
	QWidget::changeEvent(event); // **���û����ʵ��**
}

void DiagramEditor::retranslateUi() {

	QWidget* topLevelWindow = window(); // ��ȡ�� DiagramEditor ���ڵĶ�������
	if (topLevelWindow) {
		topLevelWindow->setWindowTitle(tr("Qt Diagramming Software")); // ʹ�� tr()
	}

}
// --- ��ʼ���������  ---
void DiagramEditor::initToolPanel() {
	toolscrollArea = new QScrollArea(this);
	toolscrollArea->setWidgetResizable(true);
	// --- **�޸�:** ���� ToolPanel ��������ʽ ---
	toolscrollArea->setStyleSheet(
		"QScrollArea {"
		"    background-color: #1E293B;" // ��屳��ɫ
		"    border: none;" // ͨ������Ҫ�߿�
		"}"
		"QScrollBar:vertical {" /* ��ֱ������ */
		"    border: none;"
		"    background: #1E293B;"
		"    width: 8px;"
		"    margin: 0px 0px 0px 0px;"
		"}"
		"QScrollBar::handle:vertical {"
		"    background: #475569;" /* ������ɫ */
		"    min-height: 20px;"
		"    border-radius: 4px;"
		"}"
		"QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
		"    height: 0px;" // �������˼�ͷ
		"}"
		"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
		"    background: none;"
		"}"
		// ˮƽ���������ƣ������Ҫ�Ļ�
	);
	toolscrollArea->setFixedWidth(80); // ��խ����Ҫ��ͼ��
	//toolscrollArea->setMaximumWidth(80); // ���������
	toolPanel = new ToolPanel(this);
	toolscrollArea->setWidget(toolPanel);
}

void DiagramEditor::initFormatPanel() {
	formatScrollArea = new QScrollArea(this);
	formatScrollArea->setWidgetResizable(true);
	//formatScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // ����ˮƽ������
	formatScrollArea->setStyleSheet(
		"QScrollArea {"
		"    background-color: #1E293B;" /* ��屳��ɫ */
		"    border: none;"
		"}"
		/* --- ��ֱ������ --- */
		"QScrollBar:vertical {"
		"    border: none;"
		"    background: #1E293B;" /* ����������ɫ */
		"    width: 8px;"          /* ��������� */
		"    margin: 0px 0px 0px 0px;" /* �ޱ߾� */
		"}"
		"QScrollBar::handle:vertical {"
		"    background: #475569;" /* ������ɫ */
		"    min-height: 20px;"     /* ������С�߶� */
		"    border-radius: 4px;"   /* ����Բ�� */
		"}"
		"QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
		"    height: 0px;"          /* �������˼�ͷ���� */
		"    background: none;"
		"}"
		"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
		"    background: none;"     /* ����������������֮�������*/
		"}"
		/* --- ˮƽ������ --- */
		"QScrollBar:horizontal {"
		"    border: none;"
		"    background: #1E293B;" /* ����������ɫ */
		"    height: 8px;"         /* �������߶� */
		"    margin: 0px 0px 0px 0px;" /* �ޱ߾� */
		"}"
		"QScrollBar::handle:horizontal {"
		"    background: #475569;" /* ������ɫ */
		"    min-width: 20px;"      /* ������С��� */
		"    border-radius: 4px;"   /* ����Բ�� */
		"}"
		"QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
		"    width: 0px;"           /* �������˼�ͷ���� */
		"    background: none;"
		"}"
		"QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
		"    background: none;"     /* ����������������֮�������*/
		"}"
	);
	formatScrollArea->setFixedWidth(260);
	formatPanel = new FormatPanel(this);
	formatScrollArea->setWidget(formatPanel);
	//formatScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


}

// --- �����Ҽ��ź�  ---
void DiagramEditor::handleClearSelection() {
	qDebug() << "DiagramEditor: Handling right button press signal.";
	if (toolPanel) {
		//toolPanel->uncheckAllTools(); // ȡ����������ťѡ��
		toolPanel->selectSelectionTool(); // ���ȡ��������ť��ѡ��ѡ��ť
	}
	clickLineMode = false; // ȷ��ȡ�����ģʽ
	// DrawingArea �ڲ����Ҽ������Ѿ��л����� SelectionTool ģʽ
}

// --- ���û�ͼ����С  ---
void DiagramEditor::setDiagramEditorSize(int size_w, int size_h) {
	if (drawingArea) {
		drawingArea->setFixedSize(size_w, size_h);
	}
}



Shape* DiagramEditor::getShape(int currentShapeType, QRect rect) {
	Shape* newShape = nullptr;
	QRect normalizedRect = rect.normalized();

	// --- ����һЩĬ����ɫ����ʽ ---
	const QColor BorderBlue(59, 130, 246);
	const QColor BorderRed(239, 68, 68);
	const QColor BorderGreen(16, 185, 129);
	const QColor BorderPurple(147, 51, 234);
	const QColor BorderOrange(217, 119, 6);
	const QColor BorderLime(74, 222, 128);
	const QColor BorderGray(Qt::darkGray);
	const QColor TextBlack(Qt::black);
	const QColor FillWhite(Qt::white);     // Ĭ�����ɫ
	const qreal DefaultBorderWidth = 1.5;  // Ĭ�ϱ߿���
	const bool DefaultIsFilled = false;    // Ĭ�ϲ����
	const Qt::PenStyle DefaultBorderStyle = Qt::SolidLine; // <--- Ĭ�ϱ߿���ʽ

	// --- �������ʹ�����ʹ���µĹ��캯��ǩ�������� borderStyle ---
	switch (currentShapeType) {
	case 0: // Rectangle
		newShape = new Rectangle(normalizedRect, BorderBlue, DefaultBorderWidth,
			DefaultIsFilled, FillWhite, DefaultBorderStyle);
		break;
	case 1: // Circle
		newShape = new Circle(normalizedRect, BorderRed, DefaultBorderWidth,
			DefaultIsFilled, FillWhite, DefaultBorderStyle);
		break;
	case 2: // Triangle
		newShape = new Triangle(normalizedRect, BorderGreen, DefaultBorderWidth,
			DefaultIsFilled, FillWhite, DefaultBorderStyle);
		break;
	case 3: // Diamond
		newShape = new Diamond(normalizedRect, BorderPurple, DefaultBorderWidth,
			DefaultIsFilled, FillWhite, DefaultBorderStyle);
		break;
	case 4: // RoundedRectangle
		// ע�⣺borderStyle ������ fillColor ֮��radius ����֮ǰ
		newShape = new RoundedRectangle(normalizedRect, BorderOrange, DefaultBorderWidth,
			DefaultIsFilled, FillWhite, DefaultBorderStyle, 15); // 15 ��Բ�ǰ뾶
		break;
	case 5: // Parallelogram
		newShape = new Parallelogram(normalizedRect, BorderLime, DefaultBorderWidth,
			DefaultIsFilled, FillWhite, DefaultBorderStyle, 20); // 20 ��ƫ����
		break;
	case 6:
		newShape = new TextBox(normalizedRect,           // Rect
			BorderGray,             // Border Color (TextBox Ĭ�� darkGray)
			0.0,                    // Border Width (TextBox Ĭ�� 1.0)
			false,                  // Is Filled (TextBox Ĭ�� false)
			FillWhite,              // Fill Color
			DefaultBorderStyle,     // Border Style (TextBox Ĭ�� Solid)
			QObject::tr("Double-click to edit"), // Text - �ѹ��ʻ�
			QFont("Arial", 10),     // Font
			TextBlack,              // Text Color
			Qt::AlignCenter);       // Alignment
		break;
	default:
		if (currentShapeType >= 0) {
			qWarning() << QObject::tr("DiagramEditor::getShape - Unknown shape type enum value:") << currentShapeType;
		}
		return nullptr;
	}

	if (newShape) {
		// ID �� Shape ���캯����������
		qDebug() << QObject::tr("Created shape type") << currentShapeType << QObject::tr(", ID:") << newShape->getId();
	}
	return newShape;
}

// --- �˵��ۺ���ʵ�� ---


void DiagramEditor::createNewWindow()
{
	qDebug() << QObject::tr("Creating a new Diagram Editor window..."); // �������
	DiagramEditor* newEditor = new DiagramEditor(); // ����һ���µ� DiagramEditor ʵ��

	// ��ѡ��Ϊ�´�����������
	newEditor->setWindowTitle(QObject::tr("Diagram Editor - New")); // ���ô��ڱ���
	newEditor->resize(this->size()); // ���´��ں͵�ǰ����һ����
	// ��������һ��Ĭ�ϴ�С: newEditor->resize(1024, 768);

	// �ؼ������ô��ڹر�ʱ�Զ�ɾ�������� DiagramEditor ���󣬷�ֹ�ڴ�й©��
	// ��ʹ���´����Ƕ����ġ�
	newEditor->setAttribute(Qt::WA_DeleteOnClose);

	newEditor->show(); // ��ʾ�´���
}

void DiagramEditor::onOpen() {
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open Diagram File"), "", tr("JSON Files (*.json);;All Files (*.*)"));
	if (filePath.isEmpty()) return;

	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::warning(this, tr("Error"), tr("Failed to open file: %1").arg(file.errorString()));
		return;
	}
	QByteArray data = file.readAll();
	file.close();

	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
	if (doc.isNull()) {
		QMessageBox::warning(this, tr("Error"), tr("Failed to parse JSON: %1").arg(parseError.errorString()));
		return;
	}

	// �����ͼ��
	drawingArea->selectedShapes.clear();
	drawingArea->selectedLines.clear();
	drawingArea->showControlPoints = false; // ���ؿ��Ƶ�
	qDeleteAll(drawingArea->allLines);
	drawingArea->allLines.clear();
	qDeleteAll(drawingArea->shapes);
	drawingArea->shapes.clear();
	drawingArea->updateSelectionStates(); // ȷ��״̬ͬ��

	QJsonObject rootObj = doc.object();
	QMap<QString, Shape*> shapeIdMap;
	int maxX = 0, maxY = 0;

	// --- **���������ػ����ߴ�** ---
	int loadedCanvasWidth = 2000;  // �ṩĬ��ֵ
	int loadedCanvasHeight = 1500; // �ṩĬ��ֵ
	if (rootObj.contains("canvasWidth") && rootObj["canvasWidth"].isDouble()) {
		loadedCanvasWidth = rootObj["canvasWidth"].toInt(loadedCanvasWidth); // ����������ȡ
	}
	if (rootObj.contains("canvasHeight") && rootObj["canvasHeight"].isDouble()) {
		loadedCanvasHeight = rootObj["canvasHeight"].toInt(loadedCanvasHeight); // ����������ȡ
	}

	// ����ͼ�� (ʹ�� Shape::loadFromJson)
	if (rootObj.contains("shapes") && rootObj["shapes"].isArray()) {
		QJsonArray shapesArray = rootObj["shapes"].toArray();
		for (const QJsonValue& value : shapesArray) {
			if (!value.isObject()) continue;
			QJsonObject shapeObj = value.toObject();
			QString type = shapeObj.value("type").toString("unknown");
			Shape* loadedShape = nullptr;

			// --- �������ʹ���������� ---
			// (������Ȼ��Ҫ�����ж��� new ��ȷ������)
			if (type == "rectangle") loadedShape = new Rectangle(QRect()); // ��ʱRect
			else if (type == "circle") loadedShape = new Circle(QRect());
			else if (type == "triangle") loadedShape = new Triangle(QRect());
			else if (type == "diamond") loadedShape = new Diamond(QRect());
			else if (type == "rounded_rectangle") loadedShape = new RoundedRectangle(QRect());
			else if (type == "parallelogram") loadedShape = new Parallelogram(QRect());
			else if (type == "textbox") loadedShape = new TextBox(QRect());
			else {
				qWarning() << "onOpen: Unknown shape type found in JSON:" << type;
				continue; // ����δ֪����
			}

			// --- ʹ�� loadFromJson �������� ---
			if (loadedShape) {
				loadedShape->loadFromJson(shapeObj); // �����麯����������

				// �����غ��ID�Ƿ���Ч
				QString shapeId = loadedShape->getId();
				if (shapeId.isEmpty()) { // ��� loadFromJson û������ID�������ϲ�Ӧ�ã�
					shapeId = QUuid::createUuid().toString(QUuid::WithoutBraces);
					loadedShape->setId(shapeId);
					qWarning() << "Shape loaded with empty ID, assigned new one:" << shapeId;
				}

				shapeIdMap[shapeId] = loadedShape;
				drawingArea->shapes.append(loadedShape);
				maxX = std::max(maxX, loadedShape->getRect().right());
				maxY = std::max(maxY, loadedShape->getRect().bottom());
			}
		}
	}

	// ���������� (ʹ�� ConnectionLine::fromJson���ⲿ�ֲ���)
	if (rootObj.contains("lines") && rootObj["lines"].isArray()) {
		QJsonArray linesArray = rootObj["lines"].toArray();
		for (const QJsonValue& value : linesArray) {
			if (!value.isObject()) continue;
			QJsonObject lineObj = value.toObject();
			ConnectionLine* loadedLine = ConnectionLine::fromJson(lineObj, shapeIdMap);
			if (loadedLine) {
				drawingArea->allLines.append(loadedLine);
				QRectF bounds = loadedLine->getBoundingRect();
				if (!bounds.isNull() && bounds.isValid()) {
					maxX = std::max(maxX, qRound(bounds.right()));
					maxY = std::max(maxY, qRound(bounds.bottom()));
				}
			}
			else {
				qWarning() << "Failed to load line from JSON object:" << lineObj;
			}
		}
	}

	drawingArea->setFixedSize(loadedCanvasWidth, loadedCanvasHeight);
	drawingArea->update();
	QMessageBox::information(this, tr("Success"), tr("File successfully loaded."));
}


void DiagramEditor::onSave() {
	QString jsonPath = QFileDialog::getSaveFileName(this, tr("Save Diagram Data"), "", tr("JSON Files (*.json)"));
	if (jsonPath.isEmpty()) return;

	QJsonObject rootObj;

	if (drawingArea) {
		QSize canvasSize = drawingArea->size(); // ��ȡ drawingArea �ĵ�ǰ�ߴ�
		rootObj["canvasWidth"] = canvasSize.width();
		rootObj["canvasHeight"] = canvasSize.height();
	}
	else {
		// ����ѡ�񱣴�Ĭ��ֵ������
		rootObj["canvasWidth"] = 1500; 
		rootObj["canvasHeight"] = 1200; 
	}
	// --- **������������** ---
	// ����ͼ�� (ʹ�� Shape::toJson)
	QJsonArray shapesArray;
	for (Shape* shape : drawingArea->shapes) {
		if (!shape) continue;
		QJsonObject shapeObj = shape->toJson(); 

		// ���������Ϣ (������� toJson ������)
		// �����Ż�����ÿ��������� toJson ����Լ�������
		QString typeStr = "unknown";
		if (dynamic_cast<Rectangle*>(shape)) typeStr = "rectangle";
		else if (dynamic_cast<Circle*>(shape)) typeStr = "circle";
		else if (dynamic_cast<Triangle*>(shape)) typeStr = "triangle";
		else if (dynamic_cast<Diamond*>(shape)) typeStr = "diamond";
		else if (dynamic_cast<RoundedRectangle*>(shape)) typeStr = "rounded_rectangle"; // toJson �Ѱ���
		else if (dynamic_cast<Parallelogram*>(shape)) typeStr = "parallelogram"; // toJson �Ѱ���
		else if (dynamic_cast<TextBox*>(shape)) typeStr = "textbox"; // toJson �Ѱ���
		else { qWarning() << "onSave: Unknown shape type for ID:" << shape->getId(); continue; }

		// ��� toJson û�а��� type�����������
		if (!shapeObj.contains("type")) {
			shapeObj["type"] = typeStr;
		}

		shapesArray.append(shapeObj);
	}
	rootObj["shapes"] = shapesArray;

	// ���������� (ʹ�� ConnectionLine::toJson������)
	QJsonArray allLinesArray;
	for (ConnectionLine* line : drawingArea->allLines) {
		if (!line) continue;
		// ��ȫ��飺ȷ��������ָ���ͼ�������б���
		bool lineValid = true;
		if (line->isStartAttached() && !drawingArea->shapes.contains(line->getStartShape())) {
			qWarning() << "Save: Line start shape is no longer in the main list. Skipping line.";
			lineValid = false;
		}
		if (line->isEndAttached() && !drawingArea->shapes.contains(line->getEndShape())) {
			qWarning() << "Save: Line end shape is no longer in the main list. Skipping line.";
			lineValid = false;
		}
		if (lineValid) {
			QJsonObject lineObj = line->toJson();
			if (!lineObj.isEmpty()) {
				allLinesArray.append(lineObj);
			}
			else {
				qWarning() << "Skipping saving line as toJson() returned empty object.";
			}
		}
	}
	rootObj["lines"] = allLinesArray;

	// д�� JSON �ļ� (����)
	QJsonDocument doc(rootObj);
	QFile file(jsonPath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::warning(this, tr("Error"), tr("Failed to save JSON file: %1").arg(file.errorString()));
		return;
	}
	file.write(doc.toJson(QJsonDocument::Indented));
	file.close();

	
	QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Save Image"),
		tr("Diagram data has been saved to %1.\nWould you like to also save the canvas as an image (PNG or SVG)?").arg(QFileInfo(jsonPath).fileName()),
		QMessageBox::Yes | QMessageBox::No);

	if (reply == QMessageBox::Yes) {
		if (!drawingArea) { // �ٴμ�� drawingArea
			QMessageBox::warning(this, tr("Error"), tr("Drawing area is not available to save image."));
			
			QMessageBox::information(this, tr("Success"), tr("Diagram data has been successfully saved to %1 (image not saved).").arg(QFileInfo(jsonPath).fileName()));
			return;
		}

		QFileInfo info(jsonPath); // ��ȡ JSON �ļ�����Ϣ
		QString baseName = info.baseName(); // �ļ�����������չ����
		QString defaultPath = info.absolutePath(); // �ļ�����Ŀ¼

		// **�޸ģ������ļ��Ի�����ļ������������� SVG �� PNG**
		QString selectedFilter; // ���ڽ����û�ѡ��Ĺ�����
		QString imagePath = QFileDialog::getSaveFileName(this,
			tr("Save Image As"),
			QDir(defaultPath).filePath(baseName), // Ĭ���ļ��� (����չ��)
			tr("SVG Image (*.svg);;PNG Image (*.png)"),
			&selectedFilter // �����û�ѡ��Ĺ�����
		);

		if (!imagePath.isEmpty()) {
			bool saveSuccess = false;
			QString errorMsg;

			// **����ѡ��Ĺ��������ļ���չ���жϱ����ʽ**
			QString chosenSuffix = QFileInfo(imagePath).suffix().toLower();
			bool saveAsSvg = false;

			if (!selectedFilter.isEmpty()) { // ���ȸ���ѡ��Ĺ������ж�
				if (selectedFilter.contains("*.svg", Qt::CaseInsensitive)) {
					saveAsSvg = true;
					if (chosenSuffix != "svg") imagePath += ".svg"; // ȷ����չ����ȷ
				}
				else if (selectedFilter.contains("*.png", Qt::CaseInsensitive)) {
					if (chosenSuffix != "png") imagePath += ".png"; // ȷ����չ����ȷ
				}
				else {
					
					if (chosenSuffix == "svg") saveAsSvg = true;
					else if (chosenSuffix != "png") imagePath += ".png"; // Ĭ�� PNG
				}
			}
			else { 
				if (chosenSuffix == "svg") {
					saveAsSvg = true;
				}
				else if (chosenSuffix != "png") {
					// �����׺���� svg Ҳ���� png��Ĭ����� .png
					imagePath += ".png";
				}
				
			}


			if (saveAsSvg) {
				// --- ����Ϊ SVG ---
				QSvgGenerator generator;
				generator.setFileName(imagePath);
				generator.setSize(drawingArea->size()); // ���� SVG �ĵ���СΪ������С
				generator.setViewBox(drawingArea->rect()); // ������ͼ��
				generator.setTitle(tr("Diagram Export"));
				generator.setDescription(tr("Generated from Diagram Editor"));

				QPainter svgPainter;
				if (svgPainter.begin(&generator)) {
					svgPainter.setRenderHint(QPainter::Antialiasing);
					// ��Ⱦ��ͼ�����ݵ� SVG ����
					if (auto* daWidget = qobject_cast<QWidget*>(drawingArea)) {
						// ��Ⱦǰ���ñ�����SVGĬ��͸���������Ҫ��ɫ������
						// svgPainter.fillRect(drawingArea->rect(), Qt::white); // ȡ��ע������Ӱ�ɫ����
						daWidget->render(&svgPainter);
						saveSuccess = true;
					}
					else {
						errorMsg = tr("Cannot cast drawing area to QWidget for SVG rendering.");
						qWarning() << errorMsg;
					}
					svgPainter.end(); // �������Ʋ�д���ļ�
				}
				else {
					errorMsg = tr("Could not begin painting on SVG generator.");
					qWarning() << errorMsg;
				}

			}
			else {
				// --- ����Ϊ PNG (��֮ǰ����) ---
				QImage image(drawingArea->size(), QImage::Format_ARGB32_Premultiplied);
				image.fill(Qt::white); // PNG ͨ����Ҫ����ɫ
				QPainter pngPainter(&image);
				pngPainter.setRenderHint(QPainter::Antialiasing);
				if (auto* daWidget = qobject_cast<QWidget*>(drawingArea)) {
					daWidget->render(&pngPainter);
				}

				pngPainter.end();

				if (image.save(imagePath, "PNG")) {
					saveSuccess = true;
				}
				else {
					errorMsg = tr("Failed to save PNG image to %1").arg(QFileInfo(imagePath).fileName());
					qWarning() << errorMsg;
				}
			}

			// --- ���ݱ�������ʾ��Ϣ ---
			if (saveSuccess) {
				QMessageBox::information(this, tr("Success"), tr("Diagram and image (%1) have been successfully saved.").arg(QFileInfo(imagePath).suffix().toUpper()));
				return; // �ɹ�����ͼƬ�����
			}
			else {
				QMessageBox::warning(this, tr("Error"), tr("Failed to save image: %1").arg(errorMsg));
				// ��ʹͼƬ����ʧ�ܣ���Ȼ��ʾ JSON �ѱ���
				QMessageBox::information(this, tr("Success"), tr("Diagram data has been successfully saved to %1 (image saving failed).").arg(QFileInfo(jsonPath).fileName()));
				return;
			}
		}
		else {
			// �û�ȡ����ͼƬ����Ի���
			qDebug() << "Image save cancelled by user.";
		}
	}

	QMessageBox::information(this, tr("Success"), tr("Diagram data has been successfully saved to %1").arg(QFileInfo(jsonPath).fileName()));
}


// ʵ���µĲۺ���
// ʵ�� onCut �ۺ�����
void DiagramEditor::onCut() {
	if (drawingArea) {
		// 1. ��ִ�и��Ʋ���
		drawingArea->cutActionTriggered(); // <--- ��Ҫ�� DrawingArea ��ʵ���������
	}
}
void DiagramEditor::onCopy() {
	if (drawingArea) {
		drawingArea->copySelectionToClipboard();
	}
}

void DiagramEditor::onPaste() {
	if (drawingArea) {
		drawingArea->pasteFromClipboard();
	}
}

void DiagramEditor::onDelete() {
	if (drawingArea) {
		// ���� DrawingArea �ķ�����ִ��ɾ�����÷���Ӧʹ�� Undo ����
		drawingArea->deleteSelection(); // <--- ��Ҫ�� DrawingArea ��ʵ���������
	}
}

// ʵ�ֶ���ۺ�����
void DiagramEditor::onAlignTop() {
	if (drawingArea) {
		drawingArea->alignSelectedShapes(Qt::AlignTop); // ���� DrawingArea ����
	}
}

void DiagramEditor::onAlignVCenter() {
	if (drawingArea) {
		drawingArea->alignSelectedShapes(Qt::AlignVCenter);
	}
}

void DiagramEditor::onAlignBottom() {
	if (drawingArea) {
		drawingArea->alignSelectedShapes(Qt::AlignBottom);
	}
}

// switchAera (��ԭʼһ��)
void DiagramEditor::switchAera() { // �л�����
	if (drawingArea) {
		drawingArea->isGridMode = !drawingArea->isGridMode;
		drawingArea->update();
	}
}

// --- ��������ʽ�˵��ۺ�����ʵ�� ---

void DiagramEditor::onFormatChangeBorderColor() {
	if (drawingArea) {
		drawingArea->requestChangeBorderColor(); // ���� DrawingArea �ķ���
		formatPanel->updateSelectedShapes(drawingArea->selectedShapes);
	}
}

void DiagramEditor::onFormatChangeFillColor() {
	if (drawingArea) {
		drawingArea->requestChangeFillColor();
		formatPanel->updateSelectedShapes(drawingArea->selectedShapes);
	}
}

void DiagramEditor::onSelectedShapeWidthChanged(int width) {
	if (drawingArea) {
		// ֱ�ӵ��� DrawingArea �ķ������÷����ڲ��ᴦ�� Undo ����
		drawingArea->changeSelectedShapeWidth(width);
		// ע�⣺DrawingArea �ķ����� Push ���������� redo �ᴥ�� update ���ź�
		// ��������ͨ������Ҫ���ֶ����� formatPanel
		// formatPanel->updateSelectedShapes(drawingArea->selectedShapes); // ����������ִ�к�ᱻ����
	}
}

void DiagramEditor::onSelectedShapeHeightChanged(int height) {

	if (drawingArea) {
		drawingArea->changeSelectedShapeHeight(height);
		
	}
}


void DiagramEditor::onFormatToggleFill() {
	if (drawingArea) {
		drawingArea->requestToggleFill();
	}
}

void DiagramEditor::onFormatChangeBorderWidth() {
	if (drawingArea) {
		drawingArea->requestChangeBorderWidth();
	}
}

void DiagramEditor::onFormatChangeLineColor() {
	if (drawingArea) {
		drawingArea->requestChangeLineColor();
		formatPanel->updateSelectedLines(drawingArea->selectedLines);
	}
}

void DiagramEditor::onFormatChangeLineWidth() {
	if (drawingArea) {
		drawingArea->requestChangeLineWidth();
		if (!drawingArea->selectedLines.isEmpty()) { // ֻ����ȷʵ��������ѡ��ʱ�Ÿ���
			formatPanel->updateSelectedLines(drawingArea->selectedLines); // <--- �����һ��
		}
	}
}

void DiagramEditor::onFormatChangeLineStyle() {
	if (drawingArea) {
		drawingArea->requestChangeLineStyle();
		if (!drawingArea->selectedLines.isEmpty()) { // ֻ����ȷʵ��������ѡ��ʱ�Ÿ���
			formatPanel->updateSelectedLines(drawingArea->selectedLines); // <--- �����һ��
		}
	}
}

void DiagramEditor::onFormatChangeTextFont() {
	if (drawingArea) {
		drawingArea->requestChangeTextFont();
	}
}

void DiagramEditor::onFormatChangeTextColor() {
	if (drawingArea) {
		drawingArea->requestChangeTextColor();
		formatPanel->updateSelectedShapes(drawingArea->selectedShapes);
	}
}


void DiagramEditor::onSelectedShapesBorderWidth(qreal width) {
	if (drawingArea) {
		drawingArea->changeSelectedShapesBorderWidth(width);
		formatPanel->updateSelectedShapes(drawingArea->selectedShapes);
	}
}

void DiagramEditor::onSelectedLineWidth(qreal width) {
	if (drawingArea) {
		drawingArea->changeSelectedLineWidth(width);
	}
}

// ʵ���µĲۺ���
void DiagramEditor::onSelectedShapesBorderStyle(Qt::PenStyle style) {
	if (drawingArea) {
		drawingArea->changeSelectedShapesBorderStyle(style);
		// ����ִ�к�redo �ᴥ�� drawingArea->shapeFormatChanged() ������ FormatPanel
	}
}

void DiagramEditor::onSelectedlineStyleChanged(Qt::PenStyle style) {
	if (drawingArea) {
		drawingArea->changeSelectedlineStyle(style);
		formatPanel->updateSelectedLines(drawingArea->selectedLines);
	}

}
// --- ʵ�������Ĳۺ��� ---
void DiagramEditor::onSelectedLineArrowStateChanged(bool hasArrow) {
	if (drawingArea) {
		// ���� DrawingArea �ķ�����ʵ���޸�ѡ�е�����
		drawingArea->changeSelectedLineArrowState(hasArrow);
		formatPanel->updateSelectedLines(drawingArea->selectedLines);
	}
}

void DiagramEditor::onFormatToggleNoFill() {
	if (drawingArea) {
		drawingArea->requestNoFill();
		formatPanel->updateSelectedShapes(drawingArea->selectedShapes);
	}
}
void DiagramEditor::onSelectedTextFontChanged(const QFont& font) {
	if (drawingArea) {
		drawingArea->changeSelectedTextFont(font);
		formatPanel->updateSelectedShapes(drawingArea->selectedShapes);
	}
}
// --- ��������Ӧ FormatPanel �������Ըı�Ĳ� ---
void DiagramEditor::onPanelCanvasWidthChanged(int width) {
	if (drawingArea) {
		QSize currentSize = drawingArea->canvasSize();
		drawingArea->setCanvasSize(QSize(width, currentSize.height()));
		// setCanvasSize �ڲ��ᷢ�� sizeChanged �źţ�Ȼ��ᴥ�� onDrawingAreaSizeChanged �������
	}
}

void DiagramEditor::onPanelCanvasHeightChanged(int height) {
	if (drawingArea) {
		QSize currentSize = drawingArea->canvasSize();
		drawingArea->setCanvasSize(QSize(currentSize.width(), height));
	}
}

void DiagramEditor::onPanelCanvasZoomChanged(double zoomPercentage) {
	if (drawingArea) {
		drawingArea->setZoomFactor(zoomPercentage / 100.0); // ���ٷֱ�תΪ����
		// setZoomFactor �ڲ��ᷢ�� zoomChanged �źţ�Ȼ�󴥷� onDrawingAreaZoomChanged �������
	}
}


// --- ��������Ӧ DrawingArea ���Ըı�Ĳ� ---
void DiagramEditor::onDrawingAreaZoomChanged(double newZoomFactor) {
	if (formatPanel && drawingArea) {
		formatPanel->updateCanvasProperties(drawingArea->canvasSize(), newZoomFactor);
	}
}

void DiagramEditor::onDrawingAreaSizeChanged(const QSize& newSize) {
	if (formatPanel && drawingArea) {
		formatPanel->updateCanvasProperties(newSize, drawingArea->getZoomFactor());
	}
}

// --- ������ͳһ���� FormatPanel �ķ��� ---
void DiagramEditor::updateFormatPanelAll() {
	if (!drawingArea || !formatPanel) return;

	// 1. ����ѡ������Ϣ
	formatPanel->updateSelectedShapes(drawingArea->selectedShapes); // ����ѡ�е�ͼ��
	formatPanel->updateSelectedLines(drawingArea->selectedLines);   // ����ѡ�е�����

	// 2. ���»���������Ϣ
	formatPanel->updateCanvasProperties(drawingArea->canvasSize(), drawingArea->getZoomFactor());
}

QUndoStack* DiagramEditor::getUndoStack() const {
	return undoStack;
}


// --- DrawingArea ʵ�� ---

// ���캯�� (��������, ��ʼ�� lineBeingStretched ��)
DrawingArea::DrawingArea(QScrollArea* scrollAreaParent, QUndoStack* stack ,QWidget* parent)
	: QWidget(parent), scrollAreaPtr(scrollAreaParent), undoStack(stack), isStretchingLine(false),
	lineBeingStretched(nullptr),
	stretchingStartHandle(false), // false = end handle
	currentHandle(-1),
	hoveredShape(nullptr),
	hoveredConnectionPointIndex(-1),
	isGridMode(false),
	gridColor(Qt::lightGray),
	expansionMargin(50),
	minShapeSize(10),
	m_editingTextBox(nullptr), // <--- ����
	m_textEditor(nullptr),     // <--- ����
	m_isEditingText(false),    // <--- ����
	m_currentSnapOffset(0, 0) // <<<--- ��ʼ������ƫ����Ϊ��
{
	setGridSize(20);
	setAcceptDrops(true);
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);
	currentToolMode = SelectionTool; // Ĭ��ѡ��ģʽ
	currentShapeType = -1;
	tempShape = nullptr;
	tempLine = nullptr;
	connectionStartShape = nullptr; // ��������ģʽ
	connectionStartPointIndex = -1;
	m_alignmentGuides.clear(); // ȷ����ʼʱ�ο����б�Ϊ��
	// lineBeingStretched ��ʼ��Ϊ nullptr
	// currentHandle ��ʼ��Ϊ -1
	// hoveredShape ��ʼ��Ϊ nullptr
	// hoveredConnectionPointIndex ��ʼ��Ϊ -1

	// ����ɳߴ���� (�����Ҫ)
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); // ʹ�ù̶��ߴ�
	if (scrollAreaPtr) {
		connect(scrollAreaPtr->horizontalScrollBar(), &QScrollBar::valueChanged,
			this, &DrawingArea::updateTextEditorGeometry);
		connect(scrollAreaPtr->verticalScrollBar(), &QScrollBar::valueChanged,
			this, &DrawingArea::updateTextEditorGeometry);
	}
}


// �������� (����)
DrawingArea::~DrawingArea() {
	qDeleteAll(allLines);
	allLines.clear();
	qDeleteAll(shapes);
	shapes.clear();
	delete tempShape;
	delete tempLine;
}

// setGridSize (����)
void DrawingArea::setGridSize(int size) {
	gridSize = qMax(5, size);
	update();
}

// setToolMode (����)
void DrawingArea::setToolMode(ToolMode mode) {
	resetActionState(); // �л�ģʽǰ����
	currentToolMode = mode;

	switch (mode) {
	case SelectionTool:
		setCursor(Qt::ArrowCursor);
		currentShapeType = -1;
		// �����ѡ�����ʾ���Ƶ�
		showControlPoints = !selectedShapes.isEmpty() || !selectedLines.isEmpty();
		break;
	case ShapeDrawingTool:
		if (currentShapeType != -1) {
			setCursor(Qt::CrossCursor);
			showControlPoints = false; // ��ͼʱ����ʾ�ɵĿ��Ƶ�
		}
		else {
			currentToolMode = SelectionTool; // ����
			setCursor(Qt::ArrowCursor);
		}
		break;
	case ConnectionTool:
		setCursor(Qt::CrossCursor);
		currentShapeType = -1;
		showControlPoints = false; // ����ʱ����ʾ�ɵĿ��Ƶ�
		break;
	}
	update();
}

qreal DrawingArea::getZoomFactor() const {
	return m_scaleFactor;
}
QSize DrawingArea::canvasSize() const {
	return size(); // ֱ�ӷ��ص�ǰ�����ߴ�
}

// --- Setter Slots ---
void DrawingArea::setZoomFactor(double factor) {
	factor = std::max(0.1, std::min(factor, 5.0)); // ���Ʒ�Χ 10% - 500%
	if (qAbs(m_scaleFactor - factor) > 1e-4) { // ���⸡�������²���Ҫ�ĸ���
		m_scaleFactor = factor;
		emit zoomChanged(m_scaleFactor);
		update(); // �����ػ���Ӧ������
	}
}

void DrawingArea::setCanvasSize(const QSize& newSize) {
	QSize clampedSize(std::max(100, newSize.width()), std::max(100, newSize.height())); // ������С�ߴ�
	if (size() != clampedSize) {
		setFixedSize(clampedSize); // ���ù̶��ߴ�
		emit canvasSizeChanged(clampedSize); // ����ߴ�仯�ź�
		update();
		// ע�⣺��� setFixedSize �ڲ����Զ����� scrollarea �� viewport��������Ҫ�ֶ�����
		if (scrollAreaPtr && scrollAreaPtr->widget() == this) {
			// scrollAreaPtr->updateGeometry(); // ����������ʽ֪ͨ scrollarea �ߴ����
		}
	}
}


int DrawingArea::findHandleForFreeEnd(const QPointF& pos, ConnectionLine* line) {
	if (!line || !line->isSelected()) { // ������ѡ�е���
		return -1;
	}

	const qreal handleTolerance = ConnectionLine::handleSize * 0.75; // �ֱ��뾶���ݲ��
	const qreal handleTolSq = handleTolerance * handleTolerance;

	// ������ (�����������ʱ)
	if (!line->isStartAttached()) {
		QPointF startP = line->getStartPointPos();
		qreal dxStart = pos.x() - startP.x();
		qreal dyStart = pos.y() - startP.y();
		if ((dxStart * dxStart + dyStart * dyStart) <= handleTolSq) {
			return 0; // 0 �������
		}
	}

	// ����յ� (�����յ�����ʱ)
	if (!line->isEndAttached()) {
		QPointF endP = line->getEndPointPos();
		qreal dxEnd = pos.x() - endP.x();
		qreal dyEnd = pos.y() - endP.y();
		if ((dxEnd * dxEnd + dyEnd * dyEnd) <= handleTolSq) {
			return 1; // 1 �����յ�
		}
	}

	return -1; // δ�������ɶ˵���ֱ�
}

void DrawingArea::cutActionTriggered() {
	copySelectionToClipboard();

	// 2. Ȼ��ִ��ɾ������ (deleteSelection �ڲ��ᴦ�� Undo)
	if (!selectedShapes.isEmpty() || !selectedLines.isEmpty()) {
		deleteSelection();
	}
}

void DrawingArea::copySelectionToClipboard() {
	if (selectedShapes.isEmpty() && selectedLines.isEmpty()) {
		return; // No items to copy
	}

	QJsonObject rootObj;
	QJsonArray shapesArray;
	QJsonArray linesArray;

	// 1. Serialize selected shapes
	for (Shape* shape : selectedShapes) {
		if (shape) {
			QJsonObject shapeObj = shape->toJson();
			// Ensure type is included (redundant if toJson already includes it)
			if (!shapeObj.contains("type")) {
				QString typeStr = "unknown";
				if (dynamic_cast<Rectangle*>(shape)) typeStr = "rectangle";
				else if (dynamic_cast<Circle*>(shape)) typeStr = "circle";
				else if (dynamic_cast<Triangle*>(shape)) typeStr = "triangle";
				else if (dynamic_cast<Diamond*>(shape)) typeStr = "diamond";
				else if (dynamic_cast<RoundedRectangle*>(shape)) typeStr = "rounded_rectangle";
				else if (dynamic_cast<Parallelogram*>(shape)) typeStr = "parallelogram";
				else if (dynamic_cast<TextBox*>(shape)) typeStr = "textbox";
				shapeObj["type"] = typeStr;
			}
			shapesArray.append(shapeObj);
		}
	}

	// 2. Serialize selected lines
	for (ConnectionLine* line : selectedLines) {
		if (line) {
			// Important: Only copy lines whose endpoints are either unattached or also being copied
			bool startOk = !line->isStartAttached() || selectedShapes.contains(line->getStartShape());
			bool endOk = !line->isEndAttached() || selectedShapes.contains(line->getEndShape());

			if (startOk && endOk) {
				QJsonObject lineObj = line->toJson();
				if (!lineObj.isEmpty()) {
					linesArray.append(lineObj);
				}
			}
		}
	}

	rootObj["shapes"] = shapesArray;
	rootObj["lines"] = linesArray;

	// 3. Put data into clipboard
	QClipboard* clipboard = QApplication::clipboard();
	if (!clipboard) {
		qWarning() << QObject::tr("Failed to access the clipboard.");
		return;
	}

	QMimeData* mimeData = new QMimeData();
	// Use custom MIME type to store compact JSON data
	mimeData->setData(DIAGRAM_EDITOR_MIME_TYPE, QJsonDocument(rootObj).toJson(QJsonDocument::Compact));

	// Optional: Add plain text representation for debugging or simple paste elsewhere
	// mimeData->setText(QString::fromUtf8(QJsonDocument(rootObj).toJson(QJsonDocument::Indented)));

	clipboard->setMimeData(mimeData); // Clipboard takes ownership of mimeData

	qDebug() << QObject::tr("Items copied to clipboard with MIME type:") << DIAGRAM_EDITOR_MIME_TYPE;
}


void DrawingArea::pasteFromClipboard() {
	QClipboard* clipboard = QApplication::clipboard();
	if (!clipboard) {
		qWarning() << QObject::tr("Failed to access the clipboard.");
		return;
	}

	const QMimeData* mimeData = clipboard->mimeData();
	if (!mimeData || !mimeData->hasFormat(DIAGRAM_EDITOR_MIME_TYPE)) {
		// ��ѡ����鴿�ı� JSON ��Ϊ�󱸣�
		qWarning() << QObject::tr("Clipboard does not contain diagram data in the expected format.");
		return;
	}

	QByteArray jsonData = mimeData->data(DIAGRAM_EDITOR_MIME_TYPE);
	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

	if (doc.isNull()) {
		qWarning() << QObject::tr("Failed to parse pasted JSON data: %1").arg(parseError.errorString());
		return;
	}
	if (!doc.isObject()) {
		qWarning() << QObject::tr("Pasted JSON data is not a valid object.");
		return;
	}

	QJsonObject rootObj = doc.object();
	QList<Shape*> newShapes;                 // �洢�´�����ͼ��
	QList<ConnectionLine*> newLines;         // �洢�´���������
	QMap<QString, Shape*> oldIdToNewShapeMap; // ӳ�䣺�� ID -> �´����� Shape ָ��

	// --- ����ճ��ƫ���� ---
	const QPointF pasteOffset(20, 20); // ճ��ʱ���������ƶ� 20 ����

	// --- 1. �����л���������ͼ�� ---
	if (rootObj.contains("shapes") && rootObj["shapes"].isArray()) {
		QJsonArray shapesArray = rootObj["shapes"].toArray();
		for (const QJsonValue& value : shapesArray) {
			if (!value.isObject()) continue;
			QJsonObject shapeObj = value.toObject();
			QString type = shapeObj.value("type").toString("unknown");
			QString oldId = shapeObj.value("id").toString(); // ��ȡ�� ID
			Shape* loadedShape = nullptr;

			// ������ȷ��ͼ������ (�� onOpen �߼���ͬ)
			if (type == "rectangle") loadedShape = new Rectangle(QRect());
			else if (type == "circle") loadedShape = new Circle(QRect());
			else if (type == "triangle") loadedShape = new Triangle(QRect());
			else if (type == "diamond") loadedShape = new Diamond(QRect());
			else if (type == "rounded_rectangle") loadedShape = new RoundedRectangle(QRect());
			else if (type == "parallelogram") loadedShape = new Parallelogram(QRect());
			else if (type == "textbox") loadedShape = new TextBox(QRect());
			else { /* ������ loadFromJson ����? */ }

			if (loadedShape) {
				loadedShape->loadFromJson(shapeObj); // ��������

				// *** �ؼ�: ����һ���µ� ID ***
				QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
				loadedShape->setId(newId);

				// Ӧ��ճ��ƫ����
				QRect originalRect = loadedShape->getRect();
				loadedShape->setRect(originalRect.translated(pasteOffset.toPoint()));

				newShapes.append(loadedShape); // ��ӵ���ͼ���б�
				if (!oldId.isEmpty()) {
					oldIdToNewShapeMap[oldId] = loadedShape; // �洢ӳ���ϵ
				}
			}
		}
	}

	// --- 2. �����л������������� ---
	if (rootObj.contains("lines") && rootObj["lines"].isArray()) {
		QJsonArray linesArray = rootObj["lines"].toArray();
		for (const QJsonValue& value : linesArray) {
			if (!value.isObject()) continue;
			QJsonObject lineObj = value.toObject();

			// ʹ���޸ĺ�� fromJson���������� ID ӳ��
			ConnectionLine* loadedLine = ConnectionLine::fromJson(lineObj, oldIdToNewShapeMap);

			if (loadedLine) {
				// *** �ؼ�: ����һ���µ� ID ***
				QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
				// ���� ConnectionLine �� setId ����
				// ���û�У�����Ҫ�� ConnectionLine ����� setId(const QString& id)
				// �����ڹ��캯���д���ID�������Ƽ�����ճ��������
				loadedLine->setId(newId);

				// �����ɶ�Ӧ��ճ��ƫ����
				if (!loadedLine->isStartAttached()) {
					loadedLine->setFreeStartPoint(loadedLine->getStartPointPos() + pasteOffset);
				}
				if (!loadedLine->isEndAttached()) {
					loadedLine->setFreeEndPoint(loadedLine->getEndPointPos() + pasteOffset);
				}
				newLines.append(loadedLine); // ��ӵ��������б�
			}
		}
	}

	// --- 3. ���������� Paste ���� ---
	if (!newShapes.isEmpty() || !newLines.isEmpty()) {
		if (undoStack) {
			// ����ճ��������� this (DrawingArea), ��ͼ���б�, �������б�
			undoStack->push(new PasteCommand(this, newShapes, newLines));
			// ����� redo() ��������ӵ��б�ѡ��͸��µĲ�����
		}
		else {
			qWarning() << QObject::tr("Undo stack is null. Cannot perform paste with undo support.");
			// �󱸷�����ֱ����ӣ���ʧȥ�������ܣ�- ͨ�����Ƽ�
			// for (Shape* s : newShapes) shapes.append(s);
			// for (ConnectionLine* l : newLines) allLines.append(l);
			// update();
			// �����ʹ�������Ҫ�����ڴ�
			// qDeleteAll(newShapes);
			// qDeleteAll(newLines);
		}
	}
	else {
		// �����Ѵ�����δʹ�õĶ�����ڴ�
		qDeleteAll(newShapes);
		qDeleteAll(newLines);
	}
}

// ʵ�� alignSelectedShapes ������
void DrawingArea::alignSelectedShapes(Qt::Alignment alignment) {
	// �������������Ҫѡ������ͼ��
	if (selectedShapes.size() < 2) {
		QMessageBox::information(this, tr("Tip"), tr("Please select at least two shapes to align."));
		return;
	}

	if (!undoStack) {
		return;
	}

	// ���������Ͷ�������
	undoStack->push(new AlignShapesCommand(this, selectedShapes, alignment));
	// ����� redo ���Զ�ִ�У������� UI ���������
}

// ʵ�� deleteSelection ������
void DrawingArea::deleteSelection() {
	// ����Ƿ���ѡ�е���Ŀ�Լ��Ƿ��г���ջ
	if ((selectedShapes.isEmpty() && selectedLines.isEmpty()) || !undoStack) {
		return;
	}
	QList<Shape*> shapesToDelete = selectedShapes;
	QList<ConnectionLine*> linesToDelete = selectedLines;

	// ����ɾ������
	undoStack->push(new RemoveItemsCommand(this, shapesToDelete, linesToDelete));

}

// resetActionState (ȷ��������״̬)
void DrawingArea::resetActionState() {
	// --- ������������ڱ༭�ı����Ƚ����༭ ---
	if (m_isEditingText) {
		onTextEditFinished(); // �����ύ�༭������
	}
	// --- �������� --
	isDrawing = false;           // ����ͼ�λ�������
	isResizing = false;          // ����ѡ����
	isSelecting = false;         // ��ѡ
	isMovingSelection = false;   // �ƶ�ѡ����
	isCreatingConnection = false; // ��ͼ�ο�ʼ����
	isStretchingLine = false;    // �����ߵ����ɶ˵�
	delete tempShape; 
	tempShape = nullptr;
	delete tempLine; 
	tempLine = nullptr; // ������ʱ��

	connectionStartShape = nullptr;      // �������������Ϣ
	connectionStartPointIndex = -1;

	lineBeingStretched = nullptr;       // �����������
	stretchingStartHandle = false;      // ����������״̬

	currentHandle = -1;                 // �������ž������

	// �������Ż���
	shapeOriginalRelativeRects.clear();
	lineOriginalRelativePoints.clear(); // ��վɻ���

	// ͨ����������ͣ״̬���� mouseMove ����
	// ѡ��״̬ͨ��Ҳ���ڴ˴����ã������ض����������Ҽ���

	// �ָ�Ĭ�Ϲ�꣬�����ض�����ģʽ��Ҫ��ͬ���
	if (currentToolMode == SelectionTool) {
		setCursor(Qt::ArrowCursor);
	}

	else if (currentToolMode == ShapeDrawingTool || currentToolMode == ConnectionTool) {
		setCursor(Qt::CrossCursor);
	}


}

// --- wheelEvent ---
void DrawingArea::wheelEvent(QWheelEvent* event) {
	if (event->modifiers() & Qt::ControlModifier) { // Check Ctrl key
		if (!scrollAreaPtr) {
			event->accept(); // Still accept the event
			return;
		}

		qreal oldScaleFactor = m_scaleFactor;
		qreal newScaleFactor;

		if (event->angleDelta().y() > 0) {
			newScaleFactor = m_scaleFactor * 1.15; 
		}
		else if (event->angleDelta().y() < 0) {
			newScaleFactor = m_scaleFactor / 1.15; 
		}
		else {
			event->ignore(); 
			return;
		}

		newScaleFactor = qBound(0.1, newScaleFactor, 5.0); // 10% to 500%

		if (qAbs(m_scaleFactor - newScaleFactor) > 1e-4) {


			QPointF mouseViewportPos = event->posF();

			QScrollBar* hBar = scrollAreaPtr->horizontalScrollBar();
			QScrollBar* vBar = scrollAreaPtr->verticalScrollBar();
			qreal scrollX_before = hBar->value();
			qreal scrollY_before = vBar->value();
			

			QPointF mouseWidgetPosBefore = mouseViewportPos + QPointF(scrollX_before, scrollY_before);
			QPointF scenePoint = mouseWidgetPosBefore / oldScaleFactor;

			m_scaleFactor = newScaleFactor;
			QPointF mouseWidgetPosAfter = scenePoint * m_scaleFactor;

			
			qreal newScrollX = mouseWidgetPosAfter.x() - mouseViewportPos.x();
			qreal newScrollY = mouseWidgetPosAfter.y() - mouseViewportPos.y();

			newScrollX = qBound((qreal)hBar->minimum(), newScrollX, (qreal)hBar->maximum());
			newScrollY = qBound((qreal)vBar->minimum(), newScrollY, (qreal)vBar->maximum());

			//hBar->setValue(qRound(newScrollX));
			//vBar->setValue(qRound(newScrollY));


			emit zoomChanged(m_scaleFactor);
			update(); 
			updateTextEditorGeometry(); // <--- ���ø���
			
		}
		event->accept(); // We handled the Ctrl+Wheel event

	}
	else {
		// No Ctrl key pressed, ignore the event so QScrollArea can handle normal scrolling
		event->ignore();
	}
}

// --- DrawingArea::paintEvent ---
void DrawingArea::paintEvent(QPaintEvent* event) {
	Q_UNUSED(event); // ��� event δʹ�ã��������������
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing); // ���������
	painter.fillRect(rect(), Qt::white); // �ð�ɫ��䱳��

	// --- Ӧ��ȫ������ ---
	painter.save(); // ���浱ǰ�Ļ���״̬ (�任����, ����, ��ˢ��)
	painter.scale(m_scaleFactor, m_scaleFactor); // Ӧ�õ�ǰ����������

	// --- �����ź������ϵ�»��� ---

	// 1. �������� (������ã��������ʿ�)
	if (isGridMode) {
		qreal gridPenWidth = 1.0 / m_scaleFactor; // ����ʿ�ʹ�Ӿ��Ͻӽ� 1px
		painter.setPen(QPen(gridColor, gridPenWidth)); // ���������߻���

		// ������Ҫ��������ķ�Χ (���ڵ�ǰ�ӿ�����������ϵ�µ�����)
		QRectF visibleRect = painter.transform().inverted().mapRect(rect());
		int startX = qFloor(visibleRect.left() / gridSize) * gridSize;
		int startY = qFloor(visibleRect.top() / gridSize) * gridSize;
		int endX = qCeil(visibleRect.right());
		int endY = qCeil(visibleRect.bottom());

		// ����ˮƽ��
		for (int y = startY; y < endY; y += gridSize) {
			painter.drawLine(QPointF(visibleRect.left(), y), QPointF(visibleRect.right(), y));
		}
		// ���ƴ�ֱ��
		for (int x = startX; x < endX; x += gridSize) {
			painter.drawLine(QPointF(x, visibleRect.top()), QPointF(x, visibleRect.bottom()));
		}
	}

	// 2. ���������� (ֱ�ӵ��� draw�����޸� Line �࣬�����Ӿ���ϸ��仯)
	for (ConnectionLine* line : allLines) {
		if (line) {
			line->draw(painter); // ʹ�� ConnectionLine �Լ��Ļ��Ʒ���
		}
	}

	// 3. ��������ͼ�� (ֱ�ӵ��� draw�����޸� Shape �࣬�߿�������Ӿ���С��仯)
	for (Shape* shape : shapes) {
		if (shape) {
			// ���⴦�����ڱ༭�� TextBox��ֻ�������Ʊ߿�
			if (m_isEditingText && shape == m_editingTextBox) {
				painter.save(); // ���滭��״̬
				// ���� Shape::draw �ܴ���߿���ƣ����ﲻ��Ҫ�ֶ�����
				// ��� Shape::draw �����߿�����Ҫ�����ﻭ
				// painter.setPen(QPen(shape->getBorderColor(), shape->getBorderWidth())); // ʹ��ԭʼ���
				// painter.setBrush(Qt::NoBrush);
				// painter.drawRect(shape->getRect()); // ���ƾ��α߿�ʾ��
				shape->draw(painter); // �� TextBox �Լ����ƣ�����ֻ���߿򣬻��ڲ�����
				painter.restore(); // �ָ�����״̬
			}
			else {
				// ������������ͼ�λ�Ǳ༭״̬�� TextBox
				shape->draw(painter); // ʹ�� Shape �����Լ��Ļ��Ʒ���
			}
		}
	}

	// ******** ���������ƶ���ο��� ********
	if (!m_alignmentGuides.isEmpty()) {
		painter.save();
		QPen guidePen;
		guidePen.setColor(QColor(0, 180, 0, 200)); // ��ɫ����΢͸��
		guidePen.setStyle(Qt::DashLine);          // ����
		guidePen.setWidthF(2.0 / m_scaleFactor);  // �������ţ������Ӿ���Ƚӽ� 1px
		guidePen.setCosmetic(true);              // �������߿���ȶ�
		painter.setPen(guidePen);
		for (const QLineF& guide : m_alignmentGuides) {
			painter.drawLine(guide);
		}
		painter.restore();
	}
	// ******** �������� ********


	// 4. ������ʱͼ�� (�Ӿ�Ч��ͬ��)
	if (isDrawing && tempShape && currentToolMode == ShapeDrawingTool) {
		tempShape->draw(painter); // ʹ����ʱͼ���Լ��Ļ��Ʒ���
	}

	// 5. ������ʱ�� (�Ӿ�Ч��ͬ��)
	if (tempLine && currentToolMode == ConnectionTool && (isDrawing || isCreatingConnection)) {
		tempLine->draw(painter); // ʹ����ʱ���Լ��Ļ��Ʒ���
	}

	// 6. ����ѡ�������ſ��Ƶ� (���в����Ա����Ӿ���С/��ϸ)
	if (currentToolMode == SelectionTool && showControlPoints && (!selectedShapes.isEmpty() || !selectedLines.isEmpty()))
	{
		// ��������ѡ��Ԫ�ص��ܱ߽��
		QRectF totalSelectedBounds;
		for (Shape* shape : selectedShapes) {
			if (shape) {
				totalSelectedBounds = totalSelectedBounds.united(QRectF(shape->getRect()));
			}
		}
		for (ConnectionLine* line : selectedLines) {
			if (line) {
				totalSelectedBounds = totalSelectedBounds.united(line->getBoundingRect());
			}
		}

		// ���ڱ߽���Чʱ����
		if (!totalSelectedBounds.isNull() && totalSelectedBounds.isValid()) {
			bool canResize = true; // Ĭ�Ͽ�������
			// ���������ֻѡ��һ���������ӵ��ߣ�������������
			if (selectedShapes.isEmpty() && selectedLines.size() == 1 && selectedLines.first()->isPartiallyAttached()) {
				canResize = false;
			}

			if (canResize) {
				painter.save(); // ����״̬���Ա�ֲ����û���

				// a. ����������� (�����ʿ�ͱ߾�)
				qreal selBoxPenWidth = 1.0 / m_scaleFactor; // Ŀ���Ӿ���� 1px
				qreal margin = 2.0 / m_scaleFactor;        // Ŀ���Ӿ��߾� 2px
				painter.setPen(QPen(Qt::darkGray, selBoxPenWidth, Qt::DashLine));
				painter.setBrush(Qt::NoBrush); // �����
				painter.drawRect(totalSelectedBounds.adjusted(-margin, -margin, margin, margin)); // ���ƴ��߾�����

				// b. �������ſ����ֱ� (�����ʿ�ͻ��ƴ�С)
				qreal handlePenWidth = 1.0 / m_scaleFactor;         // Ŀ���Ӿ��߿��� 1px
				qreal handleVisualSize = ConnectionLine::handleSize; // �ֱ��� *Ŀ���Ӿ�* �ߴ� (���� ConnectionLine ����)
				qreal handleDrawSize = handleVisualSize / m_scaleFactor; // �����ڵ�ǰ��������Ҫ���Ƶĳߴ�

				painter.setPen(QPen(Qt::black, handlePenWidth)); // �����ֱ��߿򻭱�
				painter.setBrush(Qt::white);                   // �����ֱ������ɫ

				// ���� 8 ���ֱ���λ��
				QPointF handles[8];
				handles[0] = totalSelectedBounds.topLeft();
				handles[1] = QPointF(totalSelectedBounds.center().x(), totalSelectedBounds.top());
				handles[2] = totalSelectedBounds.topRight();
				handles[3] = QPointF(totalSelectedBounds.left(), totalSelectedBounds.center().y());
				handles[4] = QPointF(totalSelectedBounds.right(), totalSelectedBounds.center().y());
				handles[5] = totalSelectedBounds.bottomLeft();
				handles[6] = QPointF(totalSelectedBounds.center().x(), totalSelectedBounds.bottom());
				handles[7] = totalSelectedBounds.bottomRight();

				// ���� 8 ���ֱ�����
				for (int i = 0; i < 8; ++i) {
					painter.drawRect(QRectF(handles[i].x() - handleDrawSize / 2.0, handles[i].y() - handleDrawSize / 2.0, handleDrawSize, handleDrawSize));
				}
				painter.restore(); // �ָ�����״̬
			}
			// ע�⣺ConnectionLine �ڲ����������ɶ˵��ֱ�ʱ����Ϊ���ǲ��޸����� draw ������
			// ������Щ�ֱ����Ӿ���С���������ű仯��
		}
	}


	// 7. �������ӵ���� (�����ʿ�Ͱ뾶)
	qreal pointPenWidth = 1.0 / m_scaleFactor;           // Ŀ���Ӿ��ʿ� 1px
	qreal normalVisualRadius = 4.0;                    // ��ͨ���ӵ��Ŀ���Ӿ��뾶
	qreal highlightVisualRadius = 5.0;                 // �������ӵ��Ŀ���Ӿ��뾶
	qreal normalDrawRadius = normalVisualRadius / m_scaleFactor; // ������ͨ����Ҫ���Ƶİ뾶
	qreal highlightDrawRadius = highlightVisualRadius / m_scaleFactor; // �����������Ҫ���Ƶİ뾶

	// 7a. �����������ӵ� *��ʼ* ͼ�ε����ӵ�
	if (currentToolMode == ConnectionTool && isCreatingConnection && connectionStartShape != nullptr && connectionStartPointIndex != -1)
	{
		painter.save(); // ����״̬
		int numPoints = connectionStartShape->getConnectionPointCount();
		for (int i = 0; i < numPoints; ++i) {
			QPoint hp = connectionStartShape->getConnectionPoint(i); // ��ȡ���ӵ� (QPoint)
			if (i == connectionStartPointIndex) { // ����ѡ�е���ʼ��
				painter.setBrush(Qt::yellow);
				painter.setPen(QPen(Qt::darkYellow, pointPenWidth));
				painter.drawEllipse(QPointF(hp), highlightDrawRadius, highlightDrawRadius); // ʹ�� QPointF ����
			}
			else { // ����������ͨ��
				painter.setBrush(Qt::NoBrush);
				painter.setPen(QPen(Qt::gray, pointPenWidth));
				painter.drawEllipse(QPointF(hp), normalDrawRadius, normalDrawRadius); // ʹ�� QPointF ����
			}
		}
		painter.restore(); // �ָ�״̬
	}
	// 7b. ���� *��ͣĿ��* ͼ�ε����ӵ� (�����ӻ�����ʱ)
	bool shouldDrawTargetPoints = hoveredShape != nullptr &&
		((currentToolMode == ConnectionTool) ||
			(currentToolMode == SelectionTool && isStretchingLine));
	if (shouldDrawTargetPoints)
	{
		painter.save(); // ����״̬
		int numPoints = hoveredShape->getConnectionPointCount();
		for (int i = 0; i < numPoints; ++i) {
			QPoint hp = hoveredShape->getConnectionPoint(i); // ��ȡ���ӵ� (QPoint)
			if (i == hoveredConnectionPointIndex) { // ������ͣ/������Ŀ���
				painter.setBrush(Qt::yellow);
				painter.setPen(QPen(Qt::darkYellow, pointPenWidth));
				painter.drawEllipse(QPointF(hp), highlightDrawRadius, highlightDrawRadius); // ʹ�� QPointF ����
			}
			else { // ����������ͨ��
				painter.setBrush(Qt::NoBrush);
				painter.setPen(QPen(Qt::gray, pointPenWidth));
				painter.drawEllipse(QPointF(hp), normalDrawRadius, normalDrawRadius); // ʹ�� QPointF ����
			}
		}
		painter.restore(); // �ָ�״̬
	}


	// 8. ���ƿ�ѡ���� (�����ʿ�)
	if (isSelecting && currentToolMode == SelectionTool) {
		QRectF selectionRect(selectionStart, selectionEnd); // ��ѡ��Χ
		painter.save(); // ����״̬
		qreal selRectPenWidth = 1.0 / m_scaleFactor; // Ŀ���Ӿ��ʿ� 1px
		painter.setPen(QPen(Qt::blue, selRectPenWidth, Qt::DashLine)); // ���û���
		painter.setBrush(QColor(0, 100, 255, 30)); // ���ð�͸�����
		painter.drawRect(selectionRect.normalized()); // ���ƾ���
		painter.restore(); // �ָ�״̬
	}

	// --- �ָ�����ǰ��״̬ ---
	painter.restore(); // �ָ����� painter.save() ʱ�����״̬ (�任�����)

	// --- ��δ���ŵ�����ϵ�»��� ---
	// ������Ի��Ʋ�ϣ�������ŵ�Ԫ�أ����������Ϣ��̶���UI���ǲ�
	// ���磬��ʾ��ǰ�����ű���:
	// painter.setPen(Qt::red);
	// painter.drawText(10, 20, QString("����: %1%").arg(m_scaleFactor * 100, 0, 'f', 0));

	// ע��: QLineEdit �ı��༭���� Qt �Ĵ���ϵͳ���ƣ��������� painter.scale() Ӱ�졣
	// ����λ�úʹ�С���� mouseDoubleClickEvent/mouseMoveEvent �л��� TextBox δ���ŵ� rect ���õġ�
	// �ڷ� 100% ����ʱ���༭���ͱ��� TextBox �Ӿ��Ͽ��ܲ���ȫ�غϡ�
}


// --- mousePressEvent (�޸� ConnectionTool �� SelectionTool �߼�) ---
void DrawingArea::mousePressEvent(QMouseEvent* event) {
	QPointF viewportPos = event->localPos(); // ��ȡ�ӿ�����
	QPointF pos = mapToScene(viewportPos); // ��ȡ�����߼�����
	qDebug() << "Mouse Press - Viewport:" << viewportPos << "Scene:" << pos;
	qDebug() << "Mouse Press - Viewport:" << viewportPos << "Scene:" << pos;
	qDebug() << "Mouse Press - Viewport:" << viewportPos << "Scene:" << pos.toPoint();

	// --- �޸�: lastMousePos �洢�������� ---
	lastMousePos = pos; // <--- ��¼���ΰ��µġ��������꡿
	// --- ������������ڱ༭�ı����������갴���¼� (�����ڱ༭���ڲ�) ---
	if (m_isEditingText) {
		// ������Ƿ��ڱ༭���ؼ��ⲿ
		if (m_textEditor && !m_textEditor->geometry().contains(pos.toPoint())) {
			// ����ⲿ�������༭
			onTextEditFinished();
			
		}
		else {
			
			return;
		}
		
	}
	// --- �������� ---
	bool actionTaken = false; // �����¼��Ƿ�ִ���˾������
	setFocus(); // ��ȡ����

	// --- ����������� ---
	if (event->button() == Qt::LeftButton) {
		switch (currentToolMode) {
			// --- ��ͼ���� (����) ---
		case ShapeDrawingTool: {
			if (currentShapeType != -1) {
				resetActionState();
				selectedShapes.clear();
				selectedLines.clear();
				updateSelectionStates();
				showControlPoints = false;
				isDrawing = true;
				startPoint = pos;
				endPoint = pos;
				actionTaken = true;
				setCursor(Qt::CrossCursor);
				
			}
			else {
				setToolMode(SelectionTool);
			}
		} break;

			// --- ���ӹ��� (�޸�) ---
		case ConnectionTool: {
			resetActionState(); // ����״̬
			selectedShapes.clear(); // ���ѡ��
			selectedLines.clear();
			updateSelectionStates();
			showControlPoints = false;

			Shape* startShape = nullptr;
			int startIndex = -1;
			
			if (findClosestPointOnShape(pos, startShape, startIndex, 12.0)) {
			
				isCreatingConnection = true; // ���Ϊ��ͼ�ο�ʼ
				connectionStartShape = startShape;
				connectionStartPointIndex = startIndex;
				delete tempLine; // ������ܴ��ڵľ���ʱ��
				
				tempLine = new ConnectionLine(startShape->getConnectionPoint(startIndex), pos, QColor(0, 0, 255, 150));
				tempLine->attachStart(startShape, startIndex); // ��������ӵ�ͼ��
				tempLine->setSelected(true); // ������ʾ
				actionTaken = true;
				
			}
			else {
				
				isDrawing = true; // ���Ϊ����������
				startPoint = pos; // ��¼���
				delete tempLine;
				tempLine = new ConnectionLine(startPoint, startPoint, QColor(0, 0, 255, 150));
				tempLine->setSelected(true);
				actionTaken = true;
				
			}
			setCursor(Qt::CrossCursor);
		} break;

			// --- ѡ�񹤾� (�޸Ķ��ߺ��ֱ��Ĵ���) ---
		case SelectionTool: {
			// ���ñ��β�����״̬
			isResizing = false;
			isStretchingLine = false;
			isMovingSelection = false;
			isSelecting = false;
			currentHandle = -1;
			lineBeingStretched = nullptr;
			actionTaken = false;

			bool selectionExists = !selectedShapes.isEmpty() || !selectedLines.isEmpty();
			QRectF totalSelectedBounds; // �ܱ߽��


			// --- �����ȼ�˳������λ�� ---

			// --- 1. ����Ƿ����������ֱ� (�����ж�ѡ��ѡ��ͼ��/��ȫ������ʱ) ---
			bool canResize = false; // �Ƿ������ʾ��ʹ�������ֱ�
			if (selectionExists) {
				// ֻ����ѡ�ж��Ԫ�أ���ֻѡ��ͼ�Σ���ֻѡ����ȫ������ʱ�ż������ſ�
				if (selectedShapes.size() > 1 || selectedLines.size() > 1 ||
					(selectedShapes.size() == 1 && selectedLines.isEmpty()) ||
					(selectedLines.size() == 1 && selectedShapes.isEmpty() && selectedLines.first()->isFullyFree()))
				{
					canResize = true;
					// �����ܱ߽�� (������֮ǰ����)
					for (Shape* shape : selectedShapes) {
						if (shape) {
							if (totalSelectedBounds.isNull()) {
								totalSelectedBounds = QRectF(shape->getRect());
							}
							else {
								totalSelectedBounds = totalSelectedBounds.united(QRectF(shape->getRect()));
							}
						}
					}
					for (ConnectionLine* line : selectedLines) {
						if (line && line->isFullyFree()) { // ֻ������ȫ�����ߵı߽��������ſ����
							QRectF lineBounds = line->getBoundingRect();
							if (!lineBounds.isNull() && lineBounds.isValid()) {
								if (totalSelectedBounds.isNull()) {
									totalSelectedBounds = lineBounds;
								}
								else {
									totalSelectedBounds = totalSelectedBounds.united(lineBounds);
								}
							}
						}
					}
				}
				else if (selectedLines.size() == 1 && selectedShapes.isEmpty() && selectedLines.first()->isPartiallyAttached()) {
					// ���ֻѡ��һ�����������ߣ��򲻽�������
					canResize = false;
				}
				else {
					// ������ѡ�������ͼ��+�ߣ�Ҳ���Կ����Ƿ���������
					// Ϊ�򻯣�����Ҳ��������
					canResize = true;
					// ���¼���߽磬������������
					for (Shape* shape : selectedShapes) {
						if (shape) {
							totalSelectedBounds = totalSelectedBounds.united(QRectF(shape->getRect()));
						}
					}
					for (ConnectionLine* line : selectedLines) {
						if (line) {
							totalSelectedBounds = totalSelectedBounds.united(line->getBoundingRect());
						}
					}
				}

				if (canResize && !totalSelectedBounds.isNull() && totalSelectedBounds.isValid()) {
					QPointF handles[8];
					//... (���� handles �Ĵ��벻��)...
					handles[0] = totalSelectedBounds.topLeft();
					handles[1] = QPointF(totalSelectedBounds.center().x(), totalSelectedBounds.top());
					handles[2] = totalSelectedBounds.topRight();
					handles[3] = QPointF(totalSelectedBounds.left(), totalSelectedBounds.center().y());
					handles[4] = QPointF(totalSelectedBounds.right(), totalSelectedBounds.center().y());
					handles[5] = totalSelectedBounds.bottomLeft();
					handles[6] = QPointF(totalSelectedBounds.center().x(), totalSelectedBounds.bottom());
					handles[7] = totalSelectedBounds.bottomRight();

					const qreal handleCheckSize = ConnectionLine::handleSize * 1.5;
					const qreal handleCheckRadiusSq = (handleCheckSize / 2.0) * (handleCheckSize / 2.0);

					for (int i = 0; i < 8; ++i) {
						if (isPointNear(pos, handles[i], handleCheckRadiusSq)) {
							isResizing = true;
							currentHandle = i;
							originalResizeBounds = totalSelectedBounds; // ��¼ԭʼ�߽�
							actionTaken = true;
							setCursorBasedOnHandle(i);

							// --- ��ղ�������Ż��� (�޸ģ��������ɵ���Ϣ) ---
							shapeOriginalRelativeRects.clear();
							lineOriginalRelativePoints.clear(); // ʹ���������洢���ɶ˵�����λ��
							if (originalResizeBounds.width() > 1e-6 && originalResizeBounds.height() > 1e-6) {
								// ����ѡ��ͼ�ε���Ծ��� (����)
								for (Shape* shape : selectedShapes) {
									if (!shape) {
										continue;
									}
									QRectF currentRect = QRectF(shape->getRect());
									qreal relX = (currentRect.x() - originalResizeBounds.x()) / originalResizeBounds.width();
									qreal relY = (currentRect.y() - originalResizeBounds.y()) / originalResizeBounds.height();
									qreal relW = currentRect.width() / originalResizeBounds.width();
									qreal relH = currentRect.height() / originalResizeBounds.height();
									shapeOriginalRelativeRects[shape] = QRectF(relX, relY, relW, relH);
								}
								// ����ѡ���ߵ� *����* �˵���������
								for (ConnectionLine* line : selectedLines) {
									if (!line) {
										continue;
									}
									QPair<QPointF, QPointF> relPoints = { QPointF(), QPointF() }; // {relStart, relEnd}
									bool startIsFree = false;
									bool endIsFree = false;
									if (!line->isStartAttached()) {
										QPointF p1 = line->getStartPointPos();
										relPoints.first = QPointF((p1.x() - originalResizeBounds.x()) / originalResizeBounds.width(),
											(p1.y() - originalResizeBounds.y()) / originalResizeBounds.height());
										startIsFree = true;
									}
									if (!line->isEndAttached()) {
										QPointF p2 = line->getEndPointPos();
										relPoints.second = QPointF((p2.x() - originalResizeBounds.x()) / originalResizeBounds.width(),
											(p2.y() - originalResizeBounds.y()) / originalResizeBounds.height());
										endIsFree = true;
									}
									// ֻ�е�����һ���˵�������ʱ�Ż���
									if (startIsFree || endIsFree) {
										lineOriginalRelativePoints[line] = relPoints;
									}
								}
								
							}
							else {
								
								isResizing = false;
								actionTaken = false;
							}
							break; // �ҵ��ֱ�������ѭ��
						} // end if isPointNear
					} // end for handles
				} // end if canResize and totalSelectedBounds valid
			} // ������������ֱ�

			/// +++ 2. ����Ƿ���ѡ���ߵ� *������* �˵��Կ�ʼ���� +++
			ConnectionLine* lineToDetach = nullptr;
			int attachedEndIndex = -1;
			if (!actionTaken && !selectedLines.isEmpty()) { // ֻ��ѡ���߲ż��
				attachedEndIndex = findAttachedEndNearPoint(pos, lineToDetach);
				if (attachedEndIndex != -1 && lineToDetach) {
					isStretchingLine = true; // ����״̬
					lineBeingStretched = lineToDetach;
					stretchingStartHandle = (attachedEndIndex == 0);
					// --- �������� ---
					if (stretchingStartHandle) {
						lineBeingStretched->detachStart();
						lineBeingStretched->setFreeStartPoint(pos); // �ƶ������λ��
					}
					else {
						lineBeingStretched->detachEnd();
						lineBeingStretched->setFreeEndPoint(pos); // �ƶ������λ��
					}
					actionTaken = true;
					setCursor(Qt::ClosedHandCursor); // ������ק���
					
				}
			} // +++ ������������Ӷ˵� +++

		   // --- 2. ����Ƿ������ߵ����ɶ˵��ֱ� (������ѡ������δ��������) ---
			if (selectionExists && !actionTaken) {
				for (ConnectionLine* line : selectedLines) { // ֻ���ѡ�е���
					if (line && line->isSelected()) { // ˫�ؼ��ѡ��״̬
						int handleIdx = findHandleForFreeEnd(pos, line); // ʹ���º���
						if (handleIdx != -1) { // 0 for start handle, 1 for end handle
							isStretchingLine = true;
							lineBeingStretched = line;
							stretchingStartHandle = (handleIdx == 0); // true if start handle, false if end handle
							actionTaken = true;
							setCursor(Qt::CrossCursor); // ����ʱ��ʮ�ֹ��
							
							break; // �ҵ�һ����������ֱ��͹���
						}
					}
				}
			} // ����������ɶ˵������ֱ�

			// --- 3. ���δ�����ֱ�������Ƿ�����ͼ�λ��߱��� ---
			if (!actionTaken) {
				Shape* topShape = findTopShapeAt(pos);
				ConnectionLine* topLine = findTopLineAt(pos);

				Shape* clickedShapePtr = nullptr;
				ConnectionLine* clickedLinePtr = nullptr;
				bool elementIsSelected = false;

				// �����ж�ͼ��
				if (topLine) { // û�е���ͼ�Σ����ж���
					clickedLinePtr = topLine;
					elementIsSelected = selectedLines.contains(topLine);
				}
				else if (topShape) {
					clickedShapePtr = topShape;
					elementIsSelected = selectedShapes.contains(topShape);
				}
			

				bool clickedOnElement = (clickedShapePtr != nullptr || clickedLinePtr != nullptr);
				bool ctrl = event->modifiers() & Qt::ControlModifier;

				if (clickedOnElement) { // --- �������ĳ��Ԫ���� ---
					if (!ctrl) { // --- ���� (�� Ctrl) ---
						if (!elementIsSelected) { // ������ *δѡ��* ��Ԫ��
							selectedShapes.clear();
							selectedLines.clear();
							if (clickedShapePtr) {
								selectedShapes.append(clickedShapePtr);
							}
							else {
								selectedLines.append(clickedLinePtr);
							}
							updateSelectionStates();
							showControlPoints = true; // ��ʾ���Ƶ�
							isMovingSelection = true; // ѡ�к�����׼���ƶ�
							actionTaken = true;
							setCursor(Qt::SizeAllCursor);
							
						}
						else { 
							isMovingSelection = true;
							actionTaken = true;
							setCursor(Qt::SizeAllCursor);
							
						}

					}
					else { // --- Ctrl+���� ---
						if (elementIsSelected) { // Ctrl+���� *��ѡ��* -> ȡ��ѡ�и���
							if (clickedShapePtr) {
								selectedShapes.removeOne(clickedShapePtr);
							}
							else {
								selectedLines.removeOne(clickedLinePtr);
							}
							updateSelectionStates();
							showControlPoints = !selectedShapes.isEmpty() || !selectedLines.isEmpty();
							actionTaken = true;
							
						}
						else { // Ctrl+���� *δѡ��* -> ��ӵ�ѡ��
							if (clickedShapePtr && !selectedShapes.contains(clickedShapePtr)) {
								selectedShapes.append(clickedShapePtr);
							}
							else if (clickedLinePtr && !selectedLines.contains(clickedLinePtr)) {
								selectedLines.append(clickedLinePtr);
							}
							updateSelectionStates();
							showControlPoints = !selectedShapes.isEmpty() || !selectedLines.isEmpty();
							actionTaken = true;
							
						}
					}
				} // ���� if (clickedOnElement)

				// --- 4. ���δ���Ԫ�أ�����Ƿ���ѡ������հ״� ---
				
				else if (selectionExists && !actionTaken) {
					totalSelectedBounds = QRectF(); // ��ղ����¿�ʼ����
					if (totalSelectedBounds.isNull()) {
						for (Shape* shape : selectedShapes) {
							if (shape) {
								totalSelectedBounds = totalSelectedBounds.united(QRectF(shape->getRect()));
							}
						}
						for (ConnectionLine* line : selectedLines) {
							if (line) {
								totalSelectedBounds = totalSelectedBounds.united(line->getBoundingRect());
							}
						}
					}
					
					if (!totalSelectedBounds.isNull() && totalSelectedBounds.contains(pos)) {
						isMovingSelection = true;
						actionTaken = true;
						setCursor(Qt::SizeAllCursor);
						
					}
				}

				if (isMovingSelection) { // ��ȷ��Ҫ��ʼ�ƶ�ʱ
					
					setCursor(Qt::SizeAllCursor);
					selectionStart = pos; 

					// *** ��¼�ƶ�ǰ״̬ ***
					m_originalShapeRectsOnMove.clear();
					m_originalLinePointsOnMove.clear();
					for (Shape* shape : selectedShapes) {
						if (shape) {
							m_originalShapeRectsOnMove[shape] = QRectF(shape->getRect());
						}
					}
					for (ConnectionLine* line : selectedLines) {
						if (line) {
							// ֻ��¼���ɶ˵㣬��Ϊ���Ӷ˵��� Shape ����
							QPointF startP = line->isStartAttached() ? QPointF() : line->getStartPointPos();
							QPointF endP = line->isEndAttached() ? QPointF() : line->getEndPointPos();
							m_originalLinePointsOnMove[line] = { startP, endP };
						}
					}
					actionTaken = true;
				}


				// --- 5. ������Ͼ������㣬�����˻����������հ״� ---
				if (!actionTaken) {
					if (!ctrl) { // �� Ctrl -> ���ѡ�񲢿�ʼ��ѡ
						if (selectionExists) {
							selectedShapes.clear();
							selectedLines.clear();
							updateSelectionStates();
							showControlPoints = false;
							
						}
						isSelecting = true;
						selectionStart = pos;
						selectionEnd = pos;
						actionTaken = true;
						setCursor(Qt::ArrowCursor); // ��ѡʱ�ü�ͷ
						
					}
					else { // Ctrl+�����հף��޲���
						actionTaken = true; // ��ֹ������������������Ч��
						
					}
				}

			} // �������� 3, 4, 5

		} break; // ���� case SelectionTool
		} // ���� switch(currentToolMode)

		if (actionTaken) {
			update(); // ���ִ���˲����������ػ�
		}

	} // ���� if (event->button() == Qt::LeftButton)

	
} // ���� mousePressEvent


// --- �������������ù�� (����) ---
void DrawingArea::setCursorBasedOnHandle(int handleIndex) {
	switch (handleIndex) {
	case 0: case 7: setCursor(Qt::SizeFDiagCursor); break;
	case 2: case 5: setCursor(Qt::SizeBDiagCursor); break;
	case 1: case 6: setCursor(Qt::SizeVerCursor); break;
	case 3: case 4: setCursor(Qt::SizeHorCursor); break;
	default:        setCursor(Qt::ArrowCursor); break;
	}
}

// --- mouseMoveEvent (�޸� ConnectionTool, SelectionTool-Resize, SelectionTool-Stretch �߼�) ---
void DrawingArea::mouseMoveEvent(QMouseEvent* event)
{
	if (m_isEditingText) {
		
		return; 
	}
	
	
	QPointF viewportPos = event->localPos();
	QPointF pos = mapToScene(viewportPos);
	QPointF offset = pos - lastMousePos; // �������ϴ��ƶ�������λ��
	
	updateHoveredShapeAndPoint(pos);

	// --- ��������������ʱ�����϶� ---
	if (event->buttons() & Qt::LeftButton)
	{
		// --- A. ���ӹ���ģʽ ---
		if (currentToolMode == ConnectionTool)
		{
			if ((isCreatingConnection || isDrawing) && tempLine)
			{
				
				if (isCreatingConnection)
				{
					// ��ͼ�ο�ʼ�������յ�
					tempLine->setFreeEndPoint(pos);
				}
				else
				{
					// �ӿհ׿�ʼ�������յ� (����ѹ̶�)
					tempLine->setFreeEndPoint(pos);
				}
				// ��黭����չ (������ʱ�ߵİ�Χ��)
				expandCanvasIfNeeded(tempLine->getBoundingRect().toAlignedRect());
				// �����ػ���ʱ��
				update();
			}
		}
		// --- B. ͼ�λ��ƹ���ģʽ (����) ---
		else if (currentToolMode == ShapeDrawingTool && isDrawing)
		{
			endPoint = pos;
			QRect rect(startPoint.toPoint(), endPoint.toPoint());
			rect = rect.normalized();
			if (rect.width() < minShapeSize)
			{
				rect.setWidth(minShapeSize);
			}
			if (rect.height() < minShapeSize)
			{
				rect.setHeight(minShapeSize);
			}
			delete tempShape;
			tempShape = DiagramEditor::getShape(currentShapeType, rect); // ʹ�ù���
			if (tempShape)
			{
				expandCanvasIfNeeded(tempShape->getRect());
			}
			update();
		}
		// --- C. ѡ�񹤾�ģʽ ---
		else if (currentToolMode == SelectionTool)
		{
			// --- C.1 �������� (isResizing, �޸��ߵĴ���) ---
			if (isResizing && currentHandle != -1)
			{
				// ��ȫ��� (����)
				if (originalResizeBounds.isNull()
					|| !originalResizeBounds.isValid()
					|| originalResizeBounds.width() <= 1e-6
					|| originalResizeBounds.height() <= 1e-6
					|| (shapeOriginalRelativeRects.isEmpty() && lineOriginalRelativePoints.isEmpty() && selectedLines.isEmpty()))
				{
					// ���û�л�����û��ѡ���ߣ�Ҳ�˳�
					
					resetActionState();
					update();
					return;
				}

				// �����µ��ܱ߽�� newBounds (�����϶����ֱ�������)
				QRectF newBounds = originalResizeBounds;
				qreal minAllowedSize = 5.0;

				//... (switch(currentHandle) ������ newBounds �Ĵ��벻��) ...
				switch (currentHandle) {
				case 0: newBounds.setTopLeft(QPointF(qMin(pos.x(), newBounds.right() - minAllowedSize), qMin(pos.y(), newBounds.bottom() - minAllowedSize))); break;
				case 1: newBounds.setTop(qMin(pos.y(), newBounds.bottom() - minAllowedSize)); break;
				case 2: newBounds.setTopRight(QPointF(qMax(pos.x(), newBounds.left() + minAllowedSize), qMin(pos.y(), newBounds.bottom() - minAllowedSize))); break;
				case 3: newBounds.setLeft(qMin(pos.x(), newBounds.right() - minAllowedSize)); break;
				case 4: newBounds.setRight(qMax(pos.x(), newBounds.left() + minAllowedSize)); break;
				case 5: newBounds.setBottomLeft(QPointF(qMin(pos.x(), newBounds.right() - minAllowedSize), qMax(pos.y(), newBounds.top() + minAllowedSize))); break;
				case 6: newBounds.setBottom(qMax(pos.y(), newBounds.top() + minAllowedSize)); break;
				case 7: newBounds.setBottomRight(QPointF(qMax(pos.x(), newBounds.left() + minAllowedSize), qMax(pos.y(), newBounds.top() + minAllowedSize))); break;
				}
				newBounds = newBounds.normalized(); // ȷ�� topLeft <= bottomRight

				// --- Ӧ������ (�޸Ĳ��֣������ߵ����ɶ˵�) ---
				// Ӧ�õ�ͼ�� (����)
				for (auto it = shapeOriginalRelativeRects.constBegin(); it != shapeOriginalRelativeRects.constEnd(); ++it)
				{
					Shape* shape = it.key();
					QRectF relRect = it.value();
					if (!shape)
					{
						continue;
					}
					qreal newShapeX = newBounds.x() + relRect.x() * newBounds.width();
					qreal newShapeY = newBounds.y() + relRect.y() * newBounds.height();
					qreal newShapeW = qMax((qreal)minShapeSize, relRect.width() * newBounds.width());
					qreal newShapeH = qMax((qreal)minShapeSize, relRect.height() * newBounds.height());
					shape->setRect(QRectF(newShapeX, newShapeY, newShapeW, newShapeH).toAlignedRect());
				}

				// Ӧ�õ��ߵ� *����* �˵� (ʹ�� lineOriginalRelativePoints ����)
				for (auto it = lineOriginalRelativePoints.constBegin(); it != lineOriginalRelativePoints.constEnd(); ++it)
				{
					ConnectionLine* line = it.key();
					QPair<QPointF, QPointF> relPoints = it.value(); // {relStart, relEnd}
					if (!line)
					{
						continue;
					}

					// �������Ƿ����ɲ�����
					if (!line->isStartAttached() && !relPoints.first.isNull())
					{
						// ��黺���Ƿ���Ч
						QPointF relP1 = relPoints.first;
						qreal newP1X = newBounds.x() + relP1.x() * newBounds.width();
						qreal newP1Y = newBounds.y() + relP1.y() * newBounds.height();
						line->setFreeStartPoint(QPointF(newP1X, newP1Y));
					}
					// ����յ��Ƿ����ɲ�����
					if (!line->isEndAttached() && !relPoints.second.isNull())
					{
						// ��黺���Ƿ���Ч
						QPointF relP2 = relPoints.second;
						qreal newP2X = newBounds.x() + relP2.x() * newBounds.width();
						qreal newP2Y = newBounds.y() + relP2.y() * newBounds.height();
						line->setFreeEndPoint(QPointF(newP2X, newP2Y));
					}
				}
				
				if (m_isEditingText && m_editingTextBox && selectedShapes.contains(m_editingTextBox)) {
					if (m_textEditor) {
						// ʹ�ø��º�� TextBox ���������ñ༭���ļ�����״
						m_textEditor->setGeometry(m_editingTextBox->getRect().adjusted(1, 1, -1, -1));
					}
				}
	
				QRectF finalBoundsAfterResize;
				for (Shape* shape : selectedShapes)
				{
					if (shape)
					{
						finalBoundsAfterResize = finalBoundsAfterResize.united(QRectF(shape->getRect()));
					}
				}
				for (ConnectionLine* line : selectedLines)
				{
					if (line)
					{
						finalBoundsAfterResize = finalBoundsAfterResize.united(line->getBoundingRect());
					}
				}
				if (!finalBoundsAfterResize.isNull())
				{
					expandCanvasIfNeeded(finalBoundsAfterResize.toAlignedRect());
				}
				update();
			} // ���� if (isResizing)

			// --- C.2 �������ɶ˵����� (isStretchingLine) ---
			else if (isStretchingLine && lineBeingStretched)
			{
				// ���� stretchingStartHandle �ж���������㻹���յ�
				if (stretchingStartHandle)
				{
					// �������
					lineBeingStretched->setFreeStartPoint(pos);
				}
				else
				{
					// �����յ�
					lineBeingStretched->setFreeEndPoint(pos);
				}
				// ��չ����������
				expandCanvasIfNeeded(lineBeingStretched->getBoundingRect().toAlignedRect());
				update();
			}
			// --- C.3 �����ƶ�ѡ���� (isMovingSelection) ---
			else if (isMovingSelection && offset.manhattanLength() > 0.1)
			{
			// ********����������߼���ʼ * *******
			m_alignmentGuides.clear();          // �����һ֡�Ĳο���
			m_currentSnapOffset = QPointF(0, 0); // ���õ�ǰ֡������ƫ��
			bool snappedX = false;              // ���X���Ƿ�������
			bool snappedY = false;              // ���Y���Ƿ�������

			// 1. ��ȡ���С���ѡ�С�ͼ�εı߽����
			QList<QRectF> staticRects;
			for (Shape* s : shapes) {
				if (s && !selectedShapes.contains(s)) {
					staticRects.append(QRectF(s->getRect()));
				}
			}

			// ���û�о�̬ͼ�Σ����������
			if (!staticRects.isEmpty()) {

				// 2. ����ÿ����ѡ�С���ͼ�Σ�����Ԥ��λ�ò�������
				qreal closestSnapDistX = SNAP_THRESHOLD; // ��¼��ǰ�ҵ������X��������
				qreal closestSnapDistY = SNAP_THRESHOLD; // ��¼��ǰ�ҵ������Y��������
				QLineF bestGuideX; // ��Ѵ�ֱ�ο���
				QLineF bestGuideY; // ���ˮƽ�ο���

				for (Shape* movingShape : selectedShapes) {
					if (!movingShape) continue;

					QRectF originalMovingRect = QRectF(movingShape->getRect());
					// ����û������ʱ��Ԥ��λ��
					QRectF predictedMovingRect = originalMovingRect.translated(offset);

					// �����ƶ�ͼ�εĹؼ�X, Y����
					qreal movingLeft = predictedMovingRect.left();
					qreal movingCenterX = predictedMovingRect.center().x();
					qreal movingRight = predictedMovingRect.right();
					qreal movingTop = predictedMovingRect.top();
					qreal movingCenterY = predictedMovingRect.center().y();
					qreal movingBottom = predictedMovingRect.bottom();

					// ��ÿ����̬ͼ�ν��бȽ�
					for (const QRectF& staticRect : staticRects) {
						// ���徲̬ͼ�εĹؼ�X, Y����
						qreal staticLeft = staticRect.left();
						qreal staticCenterX = staticRect.center().x();
						qreal staticRight = staticRect.right();
						qreal staticTop = staticRect.top();
						qreal staticCenterY = staticRect.center().y();
						qreal staticBottom = staticRect.bottom();

						// --- ��� X ����� ---
						qreal diffsX[] = {
							movingLeft - staticLeft, movingLeft - staticCenterX, movingLeft - staticRight,
							movingCenterX - staticLeft, movingCenterX - staticCenterX, movingCenterX - staticRight,
							movingRight - staticLeft, movingRight - staticCenterX, movingRight - staticRight
						};
						qreal targetsX[] = { // ��Ӧ�ľ�̬Ŀ��X����
							staticLeft, staticCenterX, staticRight,
							staticLeft, staticCenterX, staticRight,
							staticLeft, staticCenterX, staticRight
						};

						for (int i = 0; i < 9; ++i) {
							if (qAbs(diffsX[i]) < closestSnapDistX) {
								closestSnapDistX = qAbs(diffsX[i]);
								m_currentSnapOffset.setX(-diffsX[i]); // X������ƫ��
								snappedX = true;
								// ���㴹ֱ�ο��� (Y ��Χ��Ҫ�����ܵؼ���)
								qreal guideY1 = qMin(predictedMovingRect.top(), staticRect.top()) - 20;
								qreal guideY2 = qMax(predictedMovingRect.bottom(), staticRect.bottom()) + 20;
								bestGuideX = QLineF(targetsX[i], guideY1, targetsX[i], guideY2);
							}
						}

						// --- ��� Y ����� ---
						qreal diffsY[] = {
						   movingTop - staticTop, movingTop - staticCenterY, movingTop - staticBottom,
						   movingCenterY - staticTop, movingCenterY - staticCenterY, movingCenterY - staticBottom,
						   movingBottom - staticTop, movingBottom - staticCenterY, movingBottom - staticBottom
						};
						qreal targetsY[] = { // ��Ӧ�ľ�̬Ŀ��Y����
						   staticTop, staticCenterY, staticBottom,
						   staticTop, staticCenterY, staticBottom,
						   staticTop, staticCenterY, staticBottom
						};
						for (int i = 0; i < 9; ++i) {
							if (qAbs(diffsY[i]) < closestSnapDistY) {
								closestSnapDistY = qAbs(diffsY[i]);
								m_currentSnapOffset.setY(-diffsY[i]); // Y������ƫ��
								snappedY = true;
								// ����ˮƽ�ο��� (X ��Χ��Ҫ�����ܵؼ���)
								qreal guideX1 = qMin(predictedMovingRect.left(), staticRect.left()) - 20;
								qreal guideX2 = qMax(predictedMovingRect.right(), staticRect.right()) + 20;
								bestGuideY = QLineF(guideX1, targetsY[i], guideX2, targetsY[i]);
							}
						}
					} // end for staticRects
				} // end for movingShapes

				// ����ҵ��������������Ѳο���
				if (snappedX) m_alignmentGuides.append(bestGuideX);
				if (snappedY) m_alignmentGuides.append(bestGuideY);

				// ���X��Y��û������ȷ������ƫ��Ϊ0
				if (!snappedX) m_currentSnapOffset.setX(0);
				if (!snappedY) m_currentSnapOffset.setY(0);

			} // end if !staticRects.isEmpty()
			// ******** ����������߼����� ********
			// --- ���ݼ�� (��ѡ) ---
				// ����û������˽ϴ�ġ���ȷ���ƶ������ܲ�ϣ����������
			const qreal ESCAPE_THRESHOLD_FACTOR = 1.5; // ����ԭʼ�ƶ�������ֵ��1.5��
			if (offset.manhattanLength() > SNAP_THRESHOLD * ESCAPE_THRESHOLD_FACTOR) {
				// ǿ��ȡ����ǰ֡������
				if (!m_currentSnapOffset.isNull()) {
					m_currentSnapOffset = QPointF(0, 0);
					m_alignmentGuides.clear(); // ͬʱ����ο���
				}
			}
			// --- �������ݼ�� ---
			// --- Ӧ������ƫ���� (ԭʼƫ�� + ����ƫ��) ---
			QRectF currentMovingBounds;
			for (Shape* movingShape : selectedShapes) {
				if (movingShape) currentMovingBounds = currentMovingBounds.united(QRectF(movingShape->getRect()).translated(offset));
			}
			for (ConnectionLine* movingLine : selectedLines) {
				if (movingLine) {
					QPointF p1 = movingLine->getStartPointPos();
					QPointF p2 = movingLine->getEndPointPos();
					if (!movingLine->isStartAttached()) p1 += offset; // ֻ��ԭʼ offset
					if (!movingLine->isEndAttached()) p2 += offset; // ֻ��ԭʼ offset
					currentMovingBounds = currentMovingBounds.united(QRectF(p1, p2).normalized());
				}
			}
			expandCanvasIfNeeded(currentMovingBounds.toAlignedRect());

			// ʵ���ƶ�ͼ�κ�����ʱ��ֻӦ��ԭʼ���ƫ���� offset
			for (Shape* shape : selectedShapes) {
				if (shape) {
					shape->move(offset.toPoint()); // <<<--- ֻ�� offset
				}
			}
			for (ConnectionLine* line : selectedLines) {
				if (line) {
					line->move(offset); // <<<--- ֻ�� offset
				}
			}
			// ... (�����ı��༭���ƶ�����) ...
			if (m_isEditingText && m_editingTextBox && selectedShapes.contains(m_editingTextBox)) {
				if (m_textEditor) {
					updateTextEditorGeometry();
				}
			}

			update(); // �����ػ�����ʾ�ƶ��Ͳο���
			}
			// --- C.4 �����ѡ (isSelecting, ����) ---
			else if (isSelecting)
			{
				selectionEnd = pos;
				m_alignmentGuides.clear(); // ��ѡʱ����ʾ�ο���
				update();
			}
		} // ���� if (currentToolMode == SelectionTool)

		lastMousePos = pos; // �����ϴ����λ�ã������´μ��� offset
	} // ���� if (event->buttons() & Qt::LeftButton)
	else
	{
		// ���δ����ʱ�Ĵ��� (���¹�����ͣЧ��)
		bool needsRepaint = false;
		Qt::CursorShape currentCursorShape = cursor().shape();
		Qt::CursorShape newCursorShape = cursor().shape(); // Ĭ�ϱ��ֲ���

		// --- ����ģʽ���ù�� ---
		switch (currentToolMode)
		{
		case ConnectionTool:
			// �����ͣ�����ӵ��ϣ���ʾ���ͣ�����ʮ��
			newCursorShape = (hoveredShape && hoveredConnectionPointIndex != -1) ? Qt::PointingHandCursor : Qt::CrossCursor;
			break;

		case ShapeDrawingTool:
			newCursorShape = Qt::CrossCursor;
			break;

		case SelectionTool:
		{
			newCursorShape = Qt::ArrowCursor; // Ĭ�ϼ�ͷ
			bool cursorSet = false;

			// �����ͣ�������ֱ� (����������ʱ)
			bool canResize = false;
			QRectF totalBounds; // ��Ҫ����߽�
			if (!selectedShapes.isEmpty() || !selectedLines.isEmpty())
			{
				// ���� press �¼���� canResize �ж��߼�
				if (selectedShapes.size() > 1
					|| selectedLines.size() > 1
					|| (selectedShapes.size() == 1 && selectedLines.isEmpty())
					|| (selectedLines.size() == 1 && selectedShapes.isEmpty() && selectedLines.first()->isFullyFree()))
				{
					canResize = true;
					for (Shape* shape : selectedShapes)
					{
						if (shape)
						{
							totalBounds = totalBounds.united(QRectF(shape->getRect()));
						}
					}
					for (ConnectionLine* line : selectedLines)
					{
						if (line && line->isFullyFree())
						{
							totalBounds = totalBounds.united(line->getBoundingRect());
						}
					}
				}
				else if (selectedLines.size() == 1 && selectedShapes.isEmpty() && selectedLines.first()->isPartiallyAttached())
				{
					canResize = false;
				}
				else
				{
					// ���������������
					canResize = true;
					for (Shape* shape : selectedShapes)
					{
						if (shape)
						{
							totalBounds = totalBounds.united(QRectF(shape->getRect()));
						}
					}
					for (ConnectionLine* line : selectedLines)
					{
						if (line)
						{
							totalBounds = totalBounds.united(line->getBoundingRect());
						}
					}
				}
			}

			if (canResize && showControlPoints && !totalBounds.isNull() && totalBounds.isValid())
			{
				//... (���� handles �Ĵ��벻��) ...
				QPointF handles[8];
				handles[0] = totalBounds.topLeft();
				handles[1] = QPointF(totalBounds.center().x(), totalBounds.top());
				handles[2] = totalBounds.topRight();
				handles[3] = QPointF(totalBounds.left(), totalBounds.center().y());
				handles[4] = QPointF(totalBounds.right(), totalBounds.center().y());
				handles[5] = totalBounds.bottomLeft();
				handles[6] = QPointF(totalBounds.center().x(), totalBounds.bottom());
				handles[7] = totalBounds.bottomRight();
				const qreal handleCheckSize = ConnectionLine::handleSize * 1.5;
				const qreal handleCheckRadiusSq = (handleCheckSize / 2.0) * (handleCheckSize / 2.0);
				for (int i = 0; i < 8 && !cursorSet; ++i)
				{
					if (isPointNear(pos, handles[i], handleCheckRadiusSq))
					{
						switch (i)
						{
							//... ���� newCursorShape...
						case 0:
						case 7:
							newCursorShape = Qt::SizeFDiagCursor;
							break;
						case 2:
						case 5:
							newCursorShape = Qt::SizeBDiagCursor;
							break;
						case 1:
						case 6:
							newCursorShape = Qt::SizeVerCursor;
							break;
						case 3:
						case 4:
							newCursorShape = Qt::SizeHorCursor;
							break;
						}
						cursorSet = true;
					}
				}
			}

			// �����ͣ���ߵ����ɶ˵��ֱ�
			if (!cursorSet && !selectedLines.isEmpty() && showControlPoints)
			{
				for (ConnectionLine* line : selectedLines)
				{
					if (line && line->isSelected())
					{
						// ������ɶ˵�
						if (findHandleForFreeEnd(pos, line) != -1)
						{
							newCursorShape = Qt::CrossCursor; // ��ͣ�������ֱ�����ʮ��
							cursorSet = true;
							break;
						}
						// +++ ���������(�ɷ���)�˵� +++
						ConnectionLine* dummyLine = nullptr;
						if (findAttachedEndNearPoint(pos, dummyLine) != -1 && dummyLine == line)
						{
							newCursorShape = Qt::PointingHandCursor; // ��ͣ�ڿɷ���˵���������
							cursorSet = true;
							break;
						}
						// +++ ������� +++
					}
				}
			}

			// �����ͣ��Ԫ������
			if (!cursorSet)
			{
				Shape* hShape = findTopShapeAt(pos);
				ConnectionLine* hLine = findTopLineAt(pos);
				if (hShape || hLine)
				{
					bool itemIsSelected = (hShape && selectedShapes.contains(hShape)) || (hLine && selectedLines.contains(hLine));
					// �����ͣ����ѡ���ϣ���ʾ�ƶ���꣬������ʾ����
					newCursorShape = itemIsSelected ? Qt::SizeAllCursor : Qt::PointingHandCursor;
					cursorSet = true;
				}
			}
			// �����û��ͣ���򱣳�Ĭ�ϼ�ͷ
		}
		break; // End SelectionTool Case
		} // End Switch

		// --- ͳһ���ù�� (������Ҫ�ı�ʱ) ---
		if (currentCursorShape != newCursorShape)
		{
			setCursor(newCursorShape);
		}
		// ȷ���ڷ��ƶ�״̬������ο���
		if (!isMovingSelection && !m_alignmentGuides.isEmpty()) {
			m_alignmentGuides.clear();
			update(); // �������Ҫ�ػ�
		}

	} // �������δ����ʱ�Ĵ���
} // ���� mouseMoveEvent


// --- mouseReleaseEvent (�޸� ConnectionTool, SelectionTool-Stretch, SelectionTool-Select �߼�) ---
void DrawingArea::mouseReleaseEvent(QMouseEvent* event) {
	if (m_isEditingText) {
		// ����ѡ���Եظ��¹�꣬����һֱ��ʾ��ͷ
		// setCursor(Qt::ArrowCursor);
		return; // �������κ��ƶ�����ק�����ŵ��߼�
	}
	if (event->button() == Qt::LeftButton) { // ֻ��������ͷ�
		//QPointF pos = event->pos();
		QPointF viewportPos = event->localPos();
		QPointF pos = mapToScene(viewportPos);
		bool needsUpdate = false; // �Ƿ���Ҫ���� update()
		qreal dragDistance = (pos - lastMousePos).manhattanLength(); // �����϶����� (����)
		bool commandPushed = false; // ����Ƿ��������ӵ���ջ

		// --- A. ���ӹ���ģʽ ---
		if (currentToolMode == ConnectionTool) {
			if (isCreatingConnection && tempLine) { // ��ɴ�ͼ�ο�ʼ�����ӳ���
				Shape* startShapeRef = tempLine->getStartShape(); // ��ȡ�����Ϣ
				int startIndexRef = tempLine->getStartPointIndex();
				QPointF endPos = tempLine->getEndPointPos(); // ��ȡ�յ�λ��

				Shape* endShape = nullptr;
				int endIndex = -1;
				// �����ͷ�λ���Ƿ������ӵ� (�ų����ͼ��)
				bool foundEnd = findClosestPointOnShape(pos, endShape, endIndex, 12.0) && endShape != startShapeRef;

				if (foundEnd && startShapeRef) { // �ɹ����ӵ���һ��ͼ��
					tempLine->attachEnd(endShape, endIndex); // �����յ�
					//allLines.append(tempLine); // ��ӵ���ʽ�б�
					//tempLine = nullptr; // �����ʱָ�룬����Ȩת��
					undoStack->push(new AddItemCommand(this, tempLine));
					tempLine = nullptr;
					
				}
				else if (startShapeRef && QLineF(tempLine->getStartPointPos(), endPos).length() > QApplication::startDragDistance()) {
					// δ���ӵ�ͼ�Σ����϶������㹻 -> ��������������
					// tempLine �Ѿ���������ӣ��յ����ɵ�״̬
					/*allLines.append(tempLine);
					tempLine = nullptr;*/
					undoStack->push(new AddItemCommand(this, tempLine));
					tempLine = nullptr;
					

				}
				else { // ����ʧ�ܻ����̫��
				
					delete tempLine;
					tempLine = nullptr; // ɾ����ʱ��
				}
				needsUpdate = true;
			}
			else if (isDrawing && tempLine) { // ��ɻ��������߳���
				QPointF startPos = tempLine->getStartPointPos();
				QPointF endPos = tempLine->getEndPointPos();

				Shape* endShape = nullptr;
				int endIndex = -1;
				// �����ͷ�λ���Ƿ������ӵ�
				bool foundEnd = findClosestPointOnShape(pos, endShape, endIndex, 12.0);

				if (foundEnd && QLineF(startPos, endPos).length() > QApplication::startDragDistance()) { // �������յ�
					tempLine->attachEnd(endShape, endIndex);
					/*allLines.append(tempLine);
					tempLine = nullptr;*/
					undoStack->push(new AddItemCommand(this, tempLine));
					tempLine = nullptr;
					
				}
				else if (QLineF(startPos, endPos).length() > QApplication::startDragDistance()) { // ������ȫ������
					/*allLines.append(tempLine);
					tempLine = nullptr;*/
					undoStack->push(new AddItemCommand(this, tempLine));
					tempLine = nullptr;
					
				}
				else { // ����̫��
					
					delete tempLine;
					tempLine = nullptr;
				}
				needsUpdate = true;
			}
			// ��������/��ͼ״̬
			resetActionState(); // ʹ�� reset ����״̬
			// ���Զ��л�����ģʽ����������ģʽ
		}

		// --- B. ͼ�λ��ƹ���ģʽ (�л���ѡ��ģʽ) ---
		else if (currentToolMode == ShapeDrawingTool && isDrawing) {
			QRect finalRect;
			Shape* shapeToCommit = nullptr;

			if (tempShape) {
				finalRect = tempShape->getRect();
				if (finalRect.isValid() && finalRect.width() >= minShapeSize && finalRect.height() >= minShapeSize) {
					shapeToCommit = tempShape;
					tempShape = nullptr; // ת������Ȩ
					
				}
				else {
					
					delete tempShape; tempShape = nullptr;
				}
			}

			if (shapeToCommit) {
				if (undoStack) {
					
					undoStack->push(new AddItemCommand(this, shapeToCommit));
					commandPushed = true;
				}
				else {
					
					shapes.append(shapeToCommit); // Fallback
					selectedShapes.clear(); selectedLines.clear();
					selectedShapes.append(shapeToCommit);
					updateSelectionStates(); showControlPoints = true; update();
				}
			}
			needsUpdate = true;
			// ������ͼ�κ��Զ��л���ѡ��ģʽ
			setToolMode(SelectionTool);
			emit clearSelection(); // ֪ͨȡ����������ťѡ��״̬
			resetActionState(); // ȷ�������ͼ״̬
		}

		// --- C. ѡ�񹤾�ģʽ ---
		else if (currentToolMode == SelectionTool) {
			// --- C.1 ������ѡ ---
			if (isSelecting) {
				QRectF selectionRectF(selectionStart, pos);
				selectionRectF = selectionRectF.normalized();
				// ֻ���϶������㹻��ִ��ѡ���߼�
				if ((selectionStart - pos).manhattanLength() >= QApplication::startDragDistance()) {
					bool ctrl = event->modifiers() & Qt::ControlModifier;
					if (!ctrl) {
						selectedShapes.clear();
						selectedLines.clear();
					} // ��Ctrl�滻ѡ��
					// ����ཻ��ͼ�κ��� (����)
					for (Shape* s : shapes) {
						if (s && selectionRectF.intersects(QRectF(s->getRect())) && !selectedShapes.contains(s)) {
							selectedShapes.append(s);
						}
					}
					for (ConnectionLine* l : allLines) {
						if (l && selectionRectF.intersects(l->getBoundingRect()) && !selectedLines.contains(l)) {
							selectedLines.append(l);
						}
					}
					updateSelectionStates();
					showControlPoints = !selectedShapes.isEmpty() || !selectedLines.isEmpty();
					

				}
				else { // �϶�����̫�̣���Ϊ�����հ� (Press�¼��Ѵ��������Ǳ���)
					
					bool ctrl = event->modifiers() & Qt::ControlModifier;
					if (!ctrl && !findTopShapeAt(pos) && !findTopLineAt(pos)) { // ȷ���ǵ����հ�
						if (!selectedShapes.isEmpty() || !selectedLines.isEmpty()) {
							selectedShapes.clear();
							selectedLines.clear();
							updateSelectionStates();
							showControlPoints = false;
							
						}
					}
				}
				needsUpdate = true; // ��ѡ������Ҫ����
				isSelecting = false; // ����״̬
			}

			// --- C.2 �������� ---
			if (isResizing) {
				
				needsUpdate = true;
				isResizing = false; // ����״̬
				currentHandle = -1;
				shapeOriginalRelativeRects.clear(); // ������
				lineOriginalRelativePoints.clear();
			}

			// --- C.3 �������� (�������) ---
			if (isStretchingLine && lineBeingStretched) {
				Shape* targetShape = nullptr;
				int targetIndex = -1;
				// ����ͷŵ��Ƿ����������ͼ�����ӵ�
				// ��Ҫ�ų�����һ�����ӵ�ͼ�Σ�������ڣ�
				Shape* excludeShape = nullptr;
				if (stretchingStartHandle && lineBeingStretched->isEndAttached()) {
					excludeShape = lineBeingStretched->getEndShape();
				}
				else if (!stretchingStartHandle && lineBeingStretched->isStartAttached()) {
					excludeShape = lineBeingStretched->getStartShape();
				}

				bool snapped = false;
				if (findClosestPointOnShape(pos, targetShape, targetIndex, 12.0) && targetShape != excludeShape) {
					// �����ɹ�
					if (stretchingStartHandle) { // ����������
						lineBeingStretched->attachStart(targetShape, targetIndex);
					}
					else { // ��������յ�
						lineBeingStretched->attachEnd(targetShape, targetIndex);
					}
					snapped = true;
					
				}
				else {
					// û�����������ɶ˵�λ���� move �¼����Ѹ���
					qDebug() << QObject::tr("SelectionTool: Stretching ended, no snapping.");
				}
				needsUpdate = true;
				isStretchingLine = false; // ����״̬
				lineBeingStretched = nullptr;
			}

			// --- C.4 �����ƶ� (ʹ�û���״̬�� MoveCommand) ---
			if (isMovingSelection) {
				
				// --- Ӧ�����յ�����ƫ�� ---
			   // m_currentSnapOffset �����һ�� mouseMove �����ֵ
			   // �������ƫ�Ʋ�Ϊ�㣬��Ҫ�ٴ��ƶ�ѡ�е�Ԫ��
				if (!m_currentSnapOffset.isNull()) {
		
					for (Shape* shape : selectedShapes) {
						if (shape) {
							shape->move(m_currentSnapOffset.toPoint()); // �ڵ�ǰλ�û��������ƶ�����ƫ��
						}
					}
					for (ConnectionLine* line : selectedLines) {
						if (line) {
							line->move(m_currentSnapOffset); // �ƶ����ɶ˵�
						}
					}
					// ������ı��༭����Ҳ��Ҫ����������λ��
					if (m_isEditingText && m_editingTextBox && selectedShapes.contains(m_editingTextBox)) {
						if (m_textEditor) {
							updateTextEditorGeometry();
						}
					}
					needsUpdate = true; // ��ΪӦ������������Ҫ����
				}
				// ��¼�ƶ��������״̬
				QMap<Shape*, QRectF> finalShapeRects;
				QMap<ConnectionLine*, QPair<QPointF, QPointF>> finalLinePoints;
				for (Shape* shape : selectedShapes) { // ʹ�õ�ǰ�� selectedShapes
					if (shape) finalShapeRects[shape] = QRectF(shape->getRect());
				}
				for (ConnectionLine* line : selectedLines) {
					if (line) {
						// ʼ�ռ�¼��ǰ�˵�λ�ã������Ƿ�����
						finalLinePoints[line] = { line->getStartPointPos(), line->getEndPointPos() };
					}
				}

				// --- **�޸�:** �Ƚϳ�ʼ״̬������״̬ ---
				bool hasMoved = false;
				// ��������Ƿ�仯 (�����ϲ�Ӧ����������Ϊ��ȫ���)
				if (m_originalShapeRectsOnMove.count() != finalShapeRects.count() ||
					m_originalLinePointsOnMove.count() != finalLinePoints.count()) {
					hasMoved = true;
					
				}
				else {
					// �Ƚ�ͼ�ξ���
					for (auto it = m_originalShapeRectsOnMove.constBegin(); it != m_originalShapeRectsOnMove.constEnd() && !hasMoved; ++it) { // һ���ƶ���ֹͣ�Ƚ�
						Shape* shape = it.key();
						const QRectF& oldRect = it.value();
						if (!finalShapeRects.contains(shape)) { // ���󲻼��ˣ�
							hasMoved = true; 
						}
						const QRectF& newRect = finalShapeRects.value(shape);
						// *** ����: �ֱ�Ƚ� QRectF ���ĸ����������� ***
						if (!qFuzzyCompare(oldRect.x(), newRect.x()) ||
							!qFuzzyCompare(oldRect.y(), newRect.y()) ||
							!qFuzzyCompare(oldRect.width(), newRect.width()) ||
							!qFuzzyCompare(oldRect.height(), newRect.height()))
						{
							hasMoved = true;
							
							// break; // �����Ƴ� break��������бȽϵĵ������
						}
					}

					// �Ƚ��ߵĶ˵� (ֻ����ͼ��û�ƶ�ʱ�ż��������)
					if (!hasMoved) {
						for (auto it = m_originalLinePointsOnMove.constBegin(); it != m_originalLinePointsOnMove.constEnd() && !hasMoved; ++it) {
							ConnectionLine* line = it.key();
							const QPointF& oldStart = it.value().first; // ����ԭʼ�����ɵ�λ��(���)
							const QPointF& oldEnd = it.value().second;  // ����ԭʼ�����ɵ�λ��(���)

							if (!finalLinePoints.contains(line)) { // �߲����ˣ�
								hasMoved = true; 
							}
							const QPointF& newStart = finalLinePoints.value(line).first; // ��ǰ�����λ��
							const QPointF& newEnd = finalLinePoints.value(line).second;   // ��ǰ���յ�λ��

							// *** ����: ֻ�Ƚ����ɶ˵��λ�� ***
							bool startMoved = false;
							if (!line->isStartAttached()) { // �����������ɵ�
								// �Ƚ���ԭʼ��¼��λ�� (oldStart) �͵�ǰλ�� (newStart)
								if (!qFuzzyCompare(oldStart.x(), newStart.x()) || !qFuzzyCompare(oldStart.y(), newStart.y())) {
									startMoved = true;
								}
							}

							bool endMoved = false;
							if (!line->isEndAttached()) { // ����յ������ɵ�
								// �Ƚ���ԭʼ��¼��λ�� (oldEnd) �͵�ǰλ�� (newEnd)
								if (!qFuzzyCompare(oldEnd.x(), newEnd.x()) || !qFuzzyCompare(oldEnd.y(), newEnd.y())) {
									endMoved = true;
								}
							}

							if (startMoved || endMoved) {
								hasMoved = true;
								
							}
						}
					}
				} // --- ����״̬�Ƚ� ---

				if (hasMoved) { // ֻ������ƶ��˲� push ����
					if (undoStack) {
						
						// ���� MoveCommand�������ƶ�ǰ���ƶ����״̬ Map
						undoStack->push(new MoveCommand(this,
							m_originalShapeRectsOnMove,
							finalShapeRects,
							m_originalLinePointsOnMove,
							finalLinePoints));
						commandPushed = true;
					}
					else { /* fallback */ needsUpdate = true; }
				}
				else {
					needsUpdate = true; // ������Ҫ���¹���
				}

				
				 // <<<--- ȷ���ƶ�״̬����������ο��ߺ�����ƫ�� ---
				bool guidesNeedClearing = !m_alignmentGuides.isEmpty();
				m_alignmentGuides.clear();
				m_currentSnapOffset = QPointF(0, 0);
				if (guidesNeedClearing) {
					needsUpdate = true; // �������˲ο��ߣ������Ҫ�ػ�
				}
				// �������ƶ���¼��ԭʼ״̬
				m_originalShapeRectsOnMove.clear();
				m_originalLinePointsOnMove.clear();
				//isMovingSelection = false; // �����ƶ�״̬

			} // ���� isMovingSelection ����

			// --- C.5 �����հ����ѡ�� (����� Press ��δ�����ҵ�ǰ�޻״̬) ---
			if (!isResizing && !isStretchingLine && !isMovingSelection && !isSelecting && !needsUpdate) {
				// ����Ƿ���ĵ���ڿհ״�
				bool ctrl = event->modifiers() & Qt::ControlModifier;
				Shape* clickedShape = findTopShapeAt(pos);
				ConnectionLine* clickedLine = findTopLineAt(pos);
				// ����ǵ����հף���Ctrl��������û��ѡ���κζ�����������ѡ�е�û����Ԫ��
				if (!ctrl && !clickedShape && !clickedLine) {
					if (!selectedShapes.isEmpty() || !selectedLines.isEmpty()) {
						selectedShapes.clear();
						selectedLines.clear();
						updateSelectionStates();
						showControlPoints = false;
						needsUpdate = true;
						
					}
				}
			}
		} // ���� if (currentToolMode == SelectionTool)

		// --- ͳһ���¹��������ػ� ---
		resetActionState(); // ȷ������״̬��־��λ
		mouseMoveEvent(event); // ���� move �¼��������ͷź�Ĺ��

		if (needsUpdate) {
			update(); // ������κ���Ҫ�ػ�����
		}
		lineFormatChanged();
		shapeFormatChanged();
		/*if (!selectedLines.isEmpty() && selectedShapes.isEmpty()) {
			lineFormatChanged();
		}
		if (selectedLines.isEmpty() && !selectedShapes.isEmpty()) {
			shapeFormatChanged();
		}*/
	} // ���� if (event->button() == Qt::LeftButton)
	else if (event->button() == Qt::RightButton) {
		// �Ҽ��ͷ�ͨ�������κ����飬�߼��� Press �д���
		resetActionState(); // ȷ��״̬�ɾ�
		mouseMoveEvent(event); // ���¹��
		lineFormatChanged();
		shapeFormatChanged();
	}
}



int DrawingArea::findAttachedEndNearPoint(const QPointF& pos, ConnectionLine*& foundLine) {
	foundLine = nullptr;
	const qreal handleCheckSize = ConnectionLine::handleSize * 1.5; // ʹ���Դ�ļ�ⷶΧ
	const qreal handleCheckRadiusSq = (handleCheckSize / 2.0) * (handleCheckSize / 2.0);

	// ֻ���ѡ�е���
	for (ConnectionLine* line : selectedLines) {
		if (!line) continue;

		// ������ӵ����
		if (line->isStartAttached()) {
			QPointF startP = line->getStartPointPos();
			if (isPointNear(pos, startP, handleCheckRadiusSq)) {
				foundLine = line;
				return 0; // �ҵ����
			}
		}

		// ������ӵ��յ�
		if (line->isEndAttached()) {
			QPointF endP = line->getEndPointPos();
			if (isPointNear(pos, endP, handleCheckRadiusSq)) {
				foundLine = line;
				return 1; // �ҵ��յ�
			}
		}
	}
	return -1; // δ�ҵ�
}

// keyPressEvent (�޸� Delete �߼�)
void DrawingArea::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Escape) {

		// ִ��ԭ���Ҽ��Ĳ�����
		resetActionState(); // �������н����еĲ���������ơ��ƶ��ȣ�
		selectedShapes.clear(); // ���ͼ��ѡ��
		selectedLines.clear();  // �������ѡ��
		updateSelectionStates(); // �����ڲ�ѡ��״̬
		showControlPoints = false; // ���ؿ��Ƶ�
		setToolMode(SelectionTool); // ǿ�Ʒ���ѡ��ģʽ
		emit clearSelection();  // ֪ͨ�ⲿ������ȡ��������ѡ�У��������ź�����������º���
		update(); // �����ػ�

		// --- ���������� FormatPanel ---
		shapeFormatChanged(); // �����ź��Ը������
		lineFormatChanged();  // �����ź��Ը������
		// --- �������� ---

		
	}
	// --- �����޸� Esc ---
	else {
		QWidget::keyPressEvent(event); // ��������������
	}
}

// --- ʵ�� contextMenuEvent ---
void DrawingArea::contextMenuEvent(QContextMenuEvent* event) {
	QMenu contextMenu(this);

	// --- ��ȡ Undo/Redo Action ---
	// ֱ�Ӵ� undoStack ���������ǻ��Զ����� enabled ״̬
	if (undoStack) {
		QAction* undoAction = undoStack->createUndoAction(&contextMenu, tr("Undo"));
		undoAction->setShortcut(QKeySequence::Undo); // ��ѡ����ʾ��ݼ���ʾ
		contextMenu.addAction(undoAction);

		QAction* redoAction = undoStack->createRedoAction(&contextMenu, tr("Redo"));
		redoAction->setShortcut(QKeySequence::Redo); // ��ѡ����ʾ��ݼ���ʾ
		contextMenu.addAction(redoAction);
	}

	contextMenu.addSeparator();

	// --- ��Ӽ��ж��� ---
	QAction* cutAction = contextMenu.addAction(tr("Cut"));
	cutAction->setShortcut(QKeySequence::Cut);
	// ���в�����Ҫ��ѡ����
	cutAction->setEnabled(!selectedShapes.isEmpty() || !selectedLines.isEmpty());
	connect(cutAction, &QAction::triggered, this, &DrawingArea::cutActionTriggered);

	// --- ���� Copy Action ---
	QAction* copyAction = contextMenu.addAction(tr("Copy"));
	copyAction->setShortcut(QKeySequence::Copy);
	// �����Ƿ���ѡ����������/���ø���
	copyAction->setEnabled(!selectedShapes.isEmpty() || !selectedLines.isEmpty());
	// ���ӵ� DrawingArea �ĸ��Ʒ���
	connect(copyAction, &QAction::triggered, this, &DrawingArea::copySelectionToClipboard);

	// --- ���� Paste Action ---
	QAction* pasteAction = contextMenu.addAction(tr("Paste"));
	pasteAction->setShortcut(QKeySequence::Paste);
	// ���ݼ���������������/����ճ��
	const QClipboard* clipboard = QApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();
	pasteAction->setEnabled(mimeData && mimeData->hasFormat(DIAGRAM_EDITOR_MIME_TYPE));
	// ���ӵ� DrawingArea ��ճ������
	connect(pasteAction, &QAction::triggered, this, &DrawingArea::pasteFromClipboard);

	// --- ���ɾ������ ---
	QAction* deleteAction = contextMenu.addAction(tr("Delete")); // �������滻ΪӢ��
	deleteAction->setShortcut(QKeySequence::Delete); // ��ʾ��ݼ���ʾ
	// ����/����ɾ�����������Ƿ���ѡ����
	deleteAction->setEnabled(!selectedShapes.isEmpty() || !selectedLines.isEmpty());
	// ���� triggered �ź�ֱ�ӵ��� deleteSelection ����
	connect(deleteAction, &QAction::triggered, this, &DrawingArea::deleteSelection); // <--- ���ӵ�ɾ������

	// --- ��ʾ�˵� ---
	// event->pos() ������� DrawingArea ������
	// event->globalPos() ��ȫ����Ļ����
	contextMenu.exec(event->globalPos());

	// ����Ҫ���� event->accept()����Ϊ exec() �ᴦ��
}

// --- �������ϲ���� (����) ---
ConnectionLine* DrawingArea::findTopLineAt(const QPointF& pos) {
	// �Ӻ���ǰ���� (����˳�������)
	for (auto it = allLines.rbegin(); it != allLines.rend(); ++it) {
		// ʹ�� ConnectionLine �� contains ����
		if (*it && (*it)->contains(pos)) {
			return *it;
		}
	}
	return nullptr; // û���ҵ�
}

// --- ����ѡ��״̬ (����) ---
void DrawingArea::updateSelectionStates() {
	 //����ͼ��ѡ��״̬ (��� Shape ���� setSelected ����)
	 /*for (Shape* shape : shapes) {
	     if (shape) shape->setSelected(selectedShapes.contains(shape));
	 }*/


	// ���������ߵ�ѡ��״̬
	for (ConnectionLine* line : allLines) {
		if (line) line->setSelected(selectedLines.contains(line));
	}
}

// --- *�޸�*��ֻ�Ͽ����ӣ���ɾ���� ---
void DrawingArea::detachConnectionsAttachedTo(Shape* shape) {
	if (!shape) return;

	int count = 0;
	for (ConnectionLine* line : allLines) {
		if (line) {
			// ����Ƿ����ӵ���ͼ��
			bool wasAttached = (line->getStartShape() == shape || line->getEndShape() == shape);
			line->detachShape(shape); // �����µ� detachShape ����

			if (wasAttached && !line->isStartAttached() && !line->isEndAttached()) {
				// ��ѡ����� detach ������ȫ�����ߣ�������Щʲô�������¼��־
				qDebug() << QObject::tr("Line became fully free after detaching from shape %1").arg(shape->getId());
			}

			if (wasAttached) count++;
		}
	}

	if (count > 0)
		qDebug() << QObject::tr("Detached %1 connection ends from shape %2").arg(count).arg(shape->getId());
}


// --- findTopShapeAt (����) ---
Shape* DrawingArea::findTopShapeAt(const QPointF& pos) {
	for (auto it = shapes.rbegin(); it != shapes.rend(); ++it) {
		if (*it && (*it)->isSelected(pos.toPoint())) { // Shape::isSelected �� QPoint
			return *it;
		}
	}
	return nullptr;
}

// --- findClosestPointOnShape (����) ---
bool DrawingArea::findClosestPointOnShape(const QPointF& pos, Shape*& targetShape, int& targetIndex, double maxDist) {
	targetShape = nullptr;
	targetIndex = -1;
	double minDistSq = maxDist * maxDist + 1e-6; // ��һ����ֹ����

	for (auto it = shapes.rbegin(); it != shapes.rend(); ++it) { // ���ϲ�ͼ�ο�ʼ��
		Shape* currentShape = *it;
		if (!currentShape) continue;
		int currentBestIndex = -1;
		double currentBestDistSq = minDistSq; // ʹ�õ�ǰ��С����ƽ��

		// ���� Shape �ķ�������������ӵ�
		// Shape::findClosestConnectionPoint ���� true ����ҵ����Ҿ���ƽ��С�ڴ���� currentBestDistSq
		if (currentShape->findClosestConnectionPoint(pos.toPoint(), currentBestIndex, currentBestDistSq, std::sqrt(minDistSq))) {
			// ����ҵ��˸����ĵ�
			if (currentBestDistSq < minDistSq) {
				minDistSq = currentBestDistSq;
				targetShape = currentShape;
				targetIndex = currentBestIndex;
			}
		}
	}
	return targetShape != nullptr; // ����ҵ���Ŀ��ͼ���򷵻� true
}


// --- ������չ (�޸ģ��Ƴ� line->move) ---
void DrawingArea::expandCanvasIfNeeded(const QRect& elementBounds) {
	if (!scrollAreaPtr || elementBounds.isNull() || !elementBounds.isValid()) {
		return;
	}

	QSize currentSize = size();
	int currentWidth = currentSize.width();
	int currentHeight = currentSize.height();
	int expandLeftBy = 0, expandTopBy = 0, expandRightBy = 0, expandBottomBy = 0;

	if (elementBounds.left() < 0) {
		expandLeftBy = std::max(0, -elementBounds.left() + expansionMargin);
	}
	if (elementBounds.top() < 0) {
		expandTopBy = std::max(0, -elementBounds.top() + expansionMargin);
	}
	if (elementBounds.right() >= currentWidth) {
		expandRightBy = std::max(0, elementBounds.right() - currentWidth + 1 + expansionMargin);
	}
	if (elementBounds.bottom() >= currentHeight) {
		expandBottomBy = std::max(0, elementBounds.bottom() - currentHeight + 1 + expansionMargin);
	}

	if (expandLeftBy > 0 || expandTopBy > 0 || expandRightBy > 0 || expandBottomBy > 0) {
		QPoint shift(expandLeftBy, expandTopBy);

		// --- ƽ������Ԫ�� ---
		if (shift != QPoint(0, 0)) {
			qDebug() << QObject::tr("Expanding canvas and shifting elements by (%1, %2)").arg(shift.x()).arg(shift.y());

			// ƽ��ͼ��
			for (Shape* s : shapes) {
				if (s) {
					s->move(shift);
				}
			}

			// ƽ�� *��ȫ����* ���� �� ���������ߵ� *���ɶ˵�*
			for (ConnectionLine* l : allLines) {
				if (l) {
					l->move(shift); // ConnectionLine::move ֻ�ƶ����ɶ˵�
				}
			}

			// ƽ����ʱͼ�κ���ʱ�� (�������)
			if (tempShape) {
				tempShape->move(shift);
			}
			if (tempLine) {
				tempLine->move(shift); // ������ʱ�����ǿ���ͨ�� move �ƶ������ɶ�
			}

			// ƽ���ڲ�״̬����
			startPoint += shift;
			endPoint += shift;
			lastMousePos += shift;
			selectionStart += shift;
			selectionEnd += shift;

			// ƽ�����Ż����е����ɵ����꣨����������ţ�
			if (isResizing && !originalResizeBounds.isNull()) {
				originalResizeBounds.translate(shift); // ƽ��ԭʼ�߽��¼
				// lineOriginalRelativePoints ��������꣬����Ҫƽ��
				// shapeOriginalRelativeRects Ҳ��������꣬����Ҫƽ��
			}
		}

		// �����³ߴ粢����������
		int newWidth = currentWidth + expandLeftBy + expandRightBy;
		int newHeight = currentHeight + expandTopBy + expandBottomBy;
		QSize newCanvasSize(newWidth, newHeight); // <-- ���� QSize ����

		int oldScrollX = scrollAreaPtr->horizontalScrollBar()->value();
		int oldScrollY = scrollAreaPtr->verticalScrollBar()->value();

		// ʹ�� invokeMethod ȷ����ȫ�ص�����С
		QMetaObject::invokeMethod(this, [this, newCanvasSize]() {
			this->setCanvasSize(newCanvasSize); // <-- ���� setCanvasSize
			}, Qt::QueuedConnection);

		int newScrollX = oldScrollX + expandLeftBy;
		int newScrollY = oldScrollY + expandTopBy;
		QMetaObject::invokeMethod(scrollAreaPtr->horizontalScrollBar(), "setValue", Qt::QueuedConnection, Q_ARG(int, newScrollX));
		QMetaObject::invokeMethod(scrollAreaPtr->verticalScrollBar(), "setValue", Qt::QueuedConnection, Q_ARG(int, newScrollY));

		// �����ػ��Է�ӳ�ƶ��ͳߴ�仯
		update();
	}
}


// --- ������ͣ��Ϣ (�޸��Դ��������ų�) ---
void DrawingArea::updateHoveredShapeAndPoint(const QPointF& pos) {
	Shape* prevHoverShape = hoveredShape;
	int prevHoverIndex = hoveredConnectionPointIndex;

	Shape* newHoveredShape = nullptr;
	int newHoveredIndex = -1;

	// ���������ڴ�������/�����ߣ��������������ߵ����ɶ�
	bool shouldCheckHover = (currentToolMode == ConnectionTool) ||
		(currentToolMode == SelectionTool && isStretchingLine && lineBeingStretched);

	if (shouldCheckHover) {
		Shape* excludeShape = nullptr; // Ҫ�ų���ͼ��

		// ����Ǵ�ͼ�����ӣ����ų���ʼͼ��
		if (currentToolMode == ConnectionTool && isCreatingConnection) {
			//excludeShape = connectionStartShape;
		}
		// ��������������ߣ����ų����� *��һ��* �������ӵ�ͼ��
		else if (currentToolMode == SelectionTool && isStretchingLine && lineBeingStretched) {
			if (stretchingStartHandle && lineBeingStretched->isEndAttached()) { // ������㣬�ų��յ�ͼ��
				excludeShape = lineBeingStretched->getEndShape();
			}
			else if (!stretchingStartHandle && lineBeingStretched->isStartAttached()) { // �����յ㣬�ų����ͼ��
				excludeShape = lineBeingStretched->getStartShape();
			}
		}

		// ��������µ����ͼ�� (�ų� excludeShape)
		for (auto it = shapes.rbegin(); it != shapes.rend(); ++it) {
			Shape* currentShape = *it;
			if (!currentShape || currentShape == excludeShape) continue; // ������Ч���ų���ͼ��

			double closestDistSq = std::numeric_limits<double>::max();
			const double maxSnapDistance = 30.0; // ����̽��뾶
			bool findColseShapePoint = currentShape->findClosestConnectionPoint(pos.toPoint(), newHoveredIndex, closestDistSq, maxSnapDistance);
			if (currentShape->isSelected(pos.toPoint()) || findColseShapePoint) { // �������Ƿ���ͼ����

				// �ڴ�ͼ���ϲ���������ӵ�
				if (currentShape->findClosestConnectionPoint(pos.toPoint(), newHoveredIndex, closestDistSq, maxSnapDistance)) {
					newHoveredShape = currentShape; // �ҵ��˿��������ĵ�
					// qDebug() << "Hovering near point" << newHoveredIndex << "on shape" << newHoveredShape->getId();
				}
				else {
					// �����ͼ���ϣ��������κ����ӵ㸽��
					newHoveredShape = currentShape;
					newHoveredIndex = -1; // û��������
					// qDebug() << "Hovering over shape" << newHoveredShape->getId() << "but not near points.";
				}
				break; // �ҵ����ϲ��ͼ�ξ�ֹͣ
			}
		}
	} // ���� shouldCheckHover

	// --- �����ڲ�״̬ ---
	bool changed = false;
	if (hoveredShape != newHoveredShape) {
		hoveredShape = newHoveredShape;
		changed = true;
		// qDebug() << "Hovered shape changed to:" << (hoveredShape ? hoveredShape->getId() : "None");
	}
	// ֻ�����϶�/��������вŸ��¸�������
	if (shouldCheckHover) {
		if (hoveredConnectionPointIndex != newHoveredIndex) {
			hoveredConnectionPointIndex = newHoveredIndex;
			changed = true;
			// qDebug() << "Hovered point index changed to:" << hoveredConnectionPointIndex;
		}
	}
	else { // ��������϶�/����״̬��������ͣ����ȷ�������� -1
		if (hoveredConnectionPointIndex != -1) {
			hoveredConnectionPointIndex = -1;
			changed = true;
			// qDebug() << "Reset hovered point index.";
		}
	}

	// ���״̬�ı��������ػ�
	if (changed) {
		update();
	}
}

// --- �Ϸ��¼� (����) ---
void DrawingArea::dragEnterEvent(QDragEnterEvent* event) {
	if (event->mimeData()->hasFormat("application/x-shapetype")) event->acceptProposedAction();
	else event->ignore();
}


void DrawingArea::dropEvent(QDropEvent* event) {
	if (event->mimeData()->hasFormat("application/x-shapetype")) {
		QByteArray data = event->mimeData()->data("application/x-shapetype");
		bool ok;
		int type = data.toInt(&ok);
		if (!ok) {
			return;
		}
		// --- ���������� TEXT_BOX ---
		if (type == 6) { // ������ı�������
			//QPoint pos = event->pos();
			QPointF pos = mapToScene(event->posF());
			QSize size = setIniShapeSize(type);; // �ı���Ĭ�ϴ�С
			QRect rect(pos.x() - size.width() / 2, pos.y() - size.height() / 2, size.width(), size.height());
			Shape* newShape = DiagramEditor::getShape(type, rect);
			if (newShape) {
				expandCanvasIfNeeded(newShape->getRect());
				undoStack->push(new AddItemCommand(this, newShape));
				showControlPoints = true; // ���������С
				//setToolMode(SelectionTool); // ����ѡ��ģʽ
				event->acceptProposedAction(); this->setFocus(); 
				update();
			}
			else {
				event->ignore();
			}
			return; // ������ TextBox��ֱ�ӷ���
		}
		// --- �������� ---
		//QPoint pos = event->pos();
		QPointF pos = mapToScene(event->posF());
		QSize size = setIniShapeSize(type);
		if (size.isEmpty()) {
			return;
		}

		QRect rect(
			pos.x() - size.width() / 2,
			pos.y() - size.height() / 2,
			size.width(),
			size.height()
		);

		Shape* newShape = DiagramEditor::getShape(type, rect); // Use static factory
		if (newShape) {
			expandCanvasIfNeeded(newShape->getRect()); // ����չ����
			undoStack->push(new AddItemCommand(this, newShape));

			showControlPoints = true;
			if (currentToolMode == -1) {
				setToolMode(SelectionTool); // Return to selection mode
			}
			event->acceptProposedAction();
			this->setFocus();
			update();

		}
		else {
			event->ignore();
		}
	}
	else {
		event->ignore();
	}
}

// --- �����Ϸų�ʼ�ߴ� (����) ---
QSize DrawingArea::setIniShapeSize(int iniType) {
	switch (iniType) {
	case 0: return QSize(100, 70);
	case 1: return QSize(80, 80);
	case 2: return QSize(90, 100);
	case 3: return QSize(100, 80);
	case 4: return QSize(100, 70);
	case 5: return QSize(90, 70);
	case 6: return QSize(100, 100);
	default: return QSize();
	}
}

// --- ��ӽ��ж� (����) ---
bool DrawingArea::isPointNear(const QPointF& p1, const QPointF& p2, qreal thresholdSquared) {
	qreal dx = p1.x() - p2.x();
	qreal dy = p1.y() - p2.y();
	return (dx * dx + dy * dy) <= thresholdSquared;
}

// --- DrawingArea::mouseDoubleClickEvent (�����ı��༭) ---
void DrawingArea::mouseDoubleClickEvent(QMouseEvent* event) {
	// ����ѡ�񹤾�ģʽ����Ӧ˫��
	if (currentToolMode == SelectionTool && event->button() == Qt::LeftButton) {
		// �����ǰ���ڱ༭�����Ƚ���֮ǰ�ı༭
		if (m_isEditingText) {
			onTextEditFinished(); // �����ύ֮ǰ�ı༭
		}
		QPointF pos = mapToScene(event->localPos()); // <--- ת������
		// ����˫��λ�õ����ͼ��
		//Shape* topShape = findTopShapeAt(event->pos());
		Shape* topShape = findTopShapeAt(pos);
		TextBox* textBox = dynamic_cast<TextBox*>(topShape);

		// ����ҵ�����һ�� TextBox
		if (textBox) {
			
			m_editingTextBox = textBox; // ��¼���ڱ༭���ı���
			m_isEditingText = true;    // ���ñ༭״̬

			// --- ���� QLineEdit ��Ϊ�༭�� ---
			QLineEdit* lineEdit = new QLineEdit(this); // �� DrawingArea Ϊ���ؼ�
			m_textEditor = lineEdit; // ��¼�༭��ָ��

			// ���ñ༭����λ�úʹ�С�� TextBox һ��
			lineEdit->setGeometry(textBox->getRect().adjusted(1, 1, -1, -1)); // ��΢����һ��
			lineEdit->setText(textBox->getText());
			lineEdit->setFont(textBox->getFont()); // ʹ�� TextBox ������
			// lineEdit->setAlignment(textBox->getTextAlignment()); // QLineEdit �Ķ��뷽ʽ��ͬ

			// ���ӱ༭����ź� (���簴 Enter ��)
			connect(lineEdit, &QLineEdit::editingFinished, this, &DrawingArea::onTextEditFinished);
			// ��ѡ�������ı��仯�ź�ʵʱ���£�(ͨ������Ҫ)
			// connect(lineEdit, &QLineEdit::textChanged, ...);
			updateTextEditorGeometry(); // <--- ���ó�ʼλ��

			// ��ѡ�������㶪ʧ�������༭
			// lineEdit->installEventFilter(this); // ��Ҫ��д eventFilter ������ FocusOut

			lineEdit->show();      // ��ʾ�༭��
			lineEdit->setFocus();    // ���ü��̽���
			lineEdit->selectAll(); // ȫѡ�ı��������޸�

			// �༭ʱ���� TextBox ��������� (ͨ�� paintEvent ʵ��)
			update(); // �����ػ�

			event->accept(); // �¼��Ѵ���
			return; // �˳������⴫�ݸ�����
		}
	}
	// �������˫���ı����ģʽ���ԣ�����û��ദ��
	QWidget::mouseDoubleClickEvent(event);
}


// --- DrawingArea::onTextEditFinished (����༭��ɣ�ʹ�� Undo ����) ---
void DrawingArea::onTextEditFinished() {

	// 1. ״̬��飺ȷ������ȷʵ�����ı��༭״̬���������ָ����Ч
	if (!m_isEditingText || !m_editingTextBox || !m_textEditor) {
		// ����������ܲ����ı༭������ֹ�ڴ�й©
		if (m_textEditor) {
			m_textEditor->deleteLater();
			m_textEditor = nullptr;
		}
		m_editingTextBox = nullptr; // ���ָ��
		m_isEditingText = false;    // ����״̬��־
		// ��û��ǽ����㻹����ͼ��
		this->setFocus();
		return; // ֱ�ӷ��أ������κβ���
	}

	// --- 2. ��ȡ�༭ǰ����ı� ---
	QString oldText = m_editingTextBox->getText(); // ��ȡ�༭��ʼʱ���ı�
	QString newText;
	// ȷ�ϱ༭���� QLineEdit ���Ͳ���ȡ�ı�
	if (QLineEdit* le = qobject_cast<QLineEdit*>(m_textEditor)) {
		newText = le->text(); // ��ȡ�༭���е�ǰ���ı�
	}
	else {
		// �쳣������޷���ȡ���ı���ֱ������״̬�����ı�ԭ�ı����� push ����
		m_textEditor->deleteLater(); m_textEditor = nullptr;
		m_editingTextBox = nullptr; m_isEditingText = false;
		this->setFocus();
		return;
	}

	// --- 3. ����ָ�벢����༭��������״̬ ---
	// �ڱ༭���� deleteLater ֮ǰ����ָ�� TextBox ��ָ�룬������Ҫ��
	QLineEdit* editorToDelete = m_textEditor;
	TextBox* textBoxEdited = m_editingTextBox; // ���������Ҫ�޸ĵ�Ŀ�� TextBox

	// �������ñ༭״̬����ֹ�û��������ǰ�ٴ�˫���Ȳ������»���
	m_textEditor = nullptr;
	m_editingTextBox = nullptr;
	m_isEditingText = false;
	this->setFocus(); // �����̽��㻹����ͼ�����Ա���Ӧ�����¼����� Delete��

	// ��ȫ��ɾ���༭���ؼ�
	editorToDelete->deleteLater();

	// --- 4. ������ Push ���� (�����ı�ʵ�ʸı�ʱ) ---
	if (oldText != newText) {
		// ��鳷��ջ�Ƿ���Ч
		if (undoStack) {
			// ���� ChangeTextCommand������ DrawingArea��Ŀ�� TextBox�����ı������ı�
			undoStack->push(new ChangeTextCommand(this, textBoxEdited, oldText, newText));
		}
		else {
			// --- �� UndoStack �Ļ��˷��� ---
			textBoxEdited->setText(newText); // ֱ��Ӧ�����ı�
			update(); // �ֶ������ػ�
			shapeFormatChanged(); // �ֶ�֪ͨ FormatPanel ����
			// --- �������˷��� ---
		}
	}
	else {
		update();
		// ������Ȼ��Ҫ֪ͨ FormatPanel���Է���������������ڱ༭�ڼ�ͨ��������ʽ�ı�
		shapeFormatChanged();
	}
	// ����Ҫ��������� update() �� shapeFormatChanged()����Ϊ����� redo �ᴦ��
}


// --- ͼ��������� (����) ---
void DrawingArea::onMoveToTopLayer() {
	if (!selectedShapes.isEmpty()) {
		QList<Shape*> shapesToMove = selectedShapes; // ������
		for (Shape* s : shapesToMove) {
			int index = shapes.indexOf(s);
			if (index != -1) shapes.move(index, shapes.size() - 1);
		}
		update();
	} // �ߵ�ͼ���ݲ�����
}
void DrawingArea::onMoveToBottomLayer() {
	if (!selectedShapes.isEmpty()) {
		QList<Shape*> shapesToMove = selectedShapes;
		int targetIndex = 0;
		for (Shape* shape : shapesToMove) {
			int index = shapes.indexOf(shape);
			if (index != -1) shapes.move(index, targetIndex++); // �Ƶ�ǰ��
		}
		update();
	}
}
void DrawingArea::onMoveUpOneLayer() {
	if (!selectedShapes.isEmpty()) {
		QList<Shape*> shapesToMove = selectedShapes;
		// �Ӻ���ǰ������ֹ�����仯Ӱ��
		for (int i = shapes.size() - 1; i >= 0; --i) {
			Shape* shape = shapes.at(i);
			if (shapesToMove.contains(shape)) {
				if (i < shapes.size() - 1) shapes.move(i, i + 1);
			}
		}
		update();
	}
}
void DrawingArea::onMoveDownOneLayer() {
	if (!selectedShapes.isEmpty()) {
		QList<Shape*> shapesToMove = selectedShapes;
		// ��ǰ������
		for (int i = 0; i < shapes.size(); ++i) {
			Shape* shape = shapes.at(i);
			if (shapesToMove.contains(shape)) {
				if (i > 0) shapes.move(i, i - 1);
			}
		}
		update();
	}
}

// --- DrawingArea ʵ�ֽ��� ---

void DrawingArea::requestChangeBorderColor() {
	// ����Ƿ���ѡ�е�ͼ�λ��ı���
	if (selectedShapes.isEmpty()) {
		QMessageBox::information(this, tr("Tip"), tr("Please select one or more shapes or text boxes first."));
		return;
	}

	// ��ȡ��һ��ѡ����ĵ�ǰ�߿���ɫ��ΪĬ��ֵ
	QColor currentColor = selectedShapes.first()->getBorderColor();
	QColor newColor = QColorDialog::getColor(currentColor, this, tr("Select Border Color"));

	undoStack->push(new ChangeBorderColorCommand(this, selectedShapes, newColor));
}

void DrawingArea::requestChangeFillColor() {
	if (selectedShapes.isEmpty()) {
		QMessageBox::information(this, tr("Tip"), tr("Please select one or more shapes or text boxes first."));
		return;
	}

	// ��ȡ��һ��ѡ����ĵ�ǰ�����ɫ
	QColor currentColor = selectedShapes.first()->getFillColor();
	QColor newColor = QColorDialog::getColor(currentColor, this, tr("Select Fill Color"));

	undoStack->push(new ChangeFillColorCommand(this, selectedShapes, newColor));
}

void DrawingArea::requestToggleFill() {
	if (selectedShapes.isEmpty()) {
		QMessageBox::information(this, tr("Tip"), tr("Please select one or more shapes or text boxes first."));
		return;
	}

	// �Ե�һ��ѡ�����״̬Ϊ׼�����л�
	bool newState = !selectedShapes.first()->isFilled(); // ��ȡ��һ���ķ�״̬

	for (Shape* shape : selectedShapes) {
		shape->setFilled(newState);
	}
	update();
}

void DrawingArea::requestNoFill() {
	if (selectedShapes.isEmpty()) {
		//QMessageBox::information(this, u8"��ʾ", u8"��4��ѡ��һ������ͼ�λ��ı���");
		return;
	}
	for (Shape* shape : selectedShapes) {
		shape->setFilled(false);
	}
	update();
}


void DrawingArea::requestChangeBorderWidth() {
	if (selectedShapes.isEmpty()) {
		QMessageBox::information(this, tr("Tip"), tr("Please select one or more shapes or text boxes first."));
		return;
	}

	bool ok;
	QStringList items;
	items << tr("No Border") << "0.1" << "0.4";
	for (double i = 0.5; i <= 20; i += 0.5) {
		items << QString::number(i, 'f', 1);
	}

	// ��ȡ��һ��ѡ����ĵ�ǰ���
	qreal currentWidth = selectedShapes.first()->getBorderWidth();

	// �ҵ���ǰ������б��е�����
	int currentIndex = 0;
	if (currentWidth == 0) {
		currentIndex = 0; // �ޱ߿��Ӧ������
	}
	else {
		for (int i = 1; i < items.size(); ++i) {
			bool conversionOk;
			qreal itemValue = items[i].toDouble(&conversionOk);
			if (conversionOk && qFuzzyCompare(itemValue, currentWidth)) {
				currentIndex = i;
				break;
			}
		}
	}

	QString selected = QInputDialog::getItem(this, tr("Set Border Width"), tr("Width:"), items, currentIndex, true, &ok);
	if (ok) {
		qreal newWidth;
		if (selected == tr("No Border")) {
			newWidth = 0;
			for (Shape* shape : selectedShapes) {
				shape->setBorderColor(Qt::transparent);
			}
		}
		else {
			bool conversionOk;
			newWidth = selected.toDouble(&conversionOk);
			if (!conversionOk) {
				QMessageBox::critical(this, tr("Input Error"), tr("Please enter a valid numeric value."));
				return;
			}
		}

		undoStack->push(new ChangeBorderWidthCommand(this, selectedShapes, newWidth));
		update();
	}
}

void DrawingArea::requestChangeLineColor() {
	if (selectedLines.isEmpty() || !undoStack) {
		QMessageBox::information(this, tr("Tip"), tr("Please select one or more connection lines first."));
		return;
	}

	QColor currentColor = selectedLines.first()->getColor();
	QColor newColor = QColorDialog::getColor(currentColor, this, tr("Select Line Color"));

	undoStack->push(new ChangeLineColorCommand(this, selectedLines, newColor));
}

void DrawingArea::requestChangeLineWidth() {
	if (selectedLines.isEmpty()) {
		QMessageBox::information(this, tr("Tip"), tr("Please select one or more connection lines first."));
		return;
	}

	qreal currentWidth = selectedLines.first()->getLineWidth();
	bool ok;
	qreal newWidth = QInputDialog::getDouble(this, tr("Set Line Width"), tr("Width:"),
		currentWidth, 0.5, 20.0, 1, &ok, // ��Сֵ0.5
		Qt::WindowCloseButtonHint);

	undoStack->push(new ChangeLineWidthCommand(this, selectedLines, newWidth));
}

void DrawingArea::requestChangeLineStyle() {
	if (selectedLines.isEmpty()) {
		QMessageBox::information(this, tr("Tip"), tr("Please select one or more connection lines first."));
		return;
	}

	// ��������ѡ��
	QStringList items;
	QMap<QString, Qt::PenStyle> styleMap;
	items << tr("Solid Line"); styleMap[tr("Solid Line")] = Qt::SolidLine;
	items << tr("Dash Line"); styleMap[tr("Dash Line")] = Qt::DashLine;
	items << tr("Dot Line"); styleMap[tr("Dot Line")] = Qt::DotLine;
	items << tr("Dash-Dot Line"); styleMap[tr("Dash-Dot Line")] = Qt::DashDotLine;
	items << tr("Dash-Dot-Dot Line"); styleMap[tr("Dash-Dot-Dot Line")] = Qt::DashDotDotLine;

	// ��ȡ��ǰ��ʽ����
	Qt::PenStyle currentStyle = selectedLines.first()->getLineStyle();
	QString currentItem = tr("Solid Line"); // Ĭ��
	for (auto it = styleMap.constBegin(); it != styleMap.constEnd(); ++it) {
		if (it.value() == currentStyle) {
			currentItem = it.key();
			break;
		}
	}

	bool ok;
	QString selectedItem = QInputDialog::getItem(this, tr("Select Line Style"), tr("Style:"), items, items.indexOf(currentItem), false, &ok); // ���ɱ༭
	Qt::PenStyle newStyle = styleMap.value(selectedItem, Qt::SolidLine);

	undoStack->push(new ChangeLineStyleCommand(this, selectedLines, newStyle));
}

void DrawingArea::requestChangeTextFont() {
	TextBox* firstTextBox = nullptr;
	// �ҵ���һ��ѡ�е� TextBox
	for (Shape* shape : selectedShapes) {
		firstTextBox = dynamic_cast<TextBox*>(shape);
		if (firstTextBox) break;
	}

	if (!firstTextBox) {
		QMessageBox::information(this, tr("Tip"), tr("Please select one or more text boxes first."));
		return;
	}

	bool ok;
	QFont currentFont = firstTextBox->getFont();
	QFont newFont = QFontDialog::getFont(&ok, currentFont, this, tr("Select Font"));

	undoStack->push(new ChangeFontCommand(this, selectedShapes, newFont));
}
void DrawingArea::changeSelectedTextFont(const QFont& font) {
	/*for (Shape* shape : selectedShapes) {
		if (TextBox* tb = dynamic_cast<TextBox*>(shape)) {
			tb->setFont(font);
		}
	}*/
	undoStack->push(new ChangeFontCommand(this, selectedShapes, font));
	update();

}

void DrawingArea::requestChangeTextColor() {
	TextBox* firstTextBox = nullptr;
	for (Shape* shape : selectedShapes) {
		firstTextBox = dynamic_cast<TextBox*>(shape);
		if (firstTextBox) break;
	}

	if (!firstTextBox) {
		QMessageBox::information(this, tr("Tip"), tr("Please select one or more text boxes first."));
		return;
	}

	QColor currentColor = firstTextBox->getTextColor();
	QColor newColor = QColorDialog::getColor(currentColor, this, tr("Select Text Color"));

	// ������ selectedShapes �б��ݸ���������ڲ�����˳� TextBox
	undoStack->push(new ChangeTextColorCommand(this, selectedShapes, newColor));
}

void DrawingArea::shapeFormatChanged() {
	emit shapeFormat(selectedShapes);
};
void DrawingArea::lineFormatChanged() {
	emit lineFormat(selectedLines);
};

void DrawingArea::changeSelectedShapesBorderWidth(qreal newWidth) { 
	//if (selectedShapes.isEmpty()) return;

	//// ʹ�ø������Ƚ����ж�Ŀ�����Ƿ�Ϊ "�ޱ߿�" (�� 0)
	//bool isTargetNoBorder = qFuzzyCompare(newWidth, 0);
	//for (Shape* shape : selectedShapes) {
	//	if (!shape) continue;

	//	// ��ȡͼ�ε�ǰ�ı߿��ȣ��ж��޸�ǰ��״̬
	//	qreal currentWidth = shape->getBorderWidth();
	//	bool wasNoBorder = qFuzzyCompare(currentWidth, 0);

	//	// 1. �����µı߿���
	//	//shape->setBorderWidth(newWidth);
	//	undoStack->push(new ChangeBorderWidthCommand(this, shapes, newWidth));
	//	// 2. �����¾�״̬������ɫ
	//	if (isTargetNoBorder) {
	//		// ���Ŀ�����ޱ߿򣬽���ɫ����Ϊ͸��
	//		if (shape->getBorderColor() != Qt::transparent) {
	//			shape->setBorderColor(Qt::transparent);
	//		}
	//	}
	//	else {
	//		if (wasNoBorder) {
	//			// ����֮ǰ���ޱ߿�����ɫ����Ϊ��ɫ
	//			shape->setBorderColor(Qt::black);
	//		}
	//		else {
	//			if (shape->getBorderColor() == Qt::transparent) {
	//				shape->setBorderColor(Qt::black);
	//			}
	//		}
	//	}
	//} 

	//update(); 
	//shapeFormatChanged(); 
	if (selectedShapes.isEmpty()) return;
	undoStack->push(new ChangeBorderWidthCommand(this, selectedShapes, newWidth));
	shapeFormatChanged();
}
// ʵ���µķ���

void DrawingArea::changeSelectedShapesBorderStyle(Qt::PenStyle style) {
	if (selectedShapes.isEmpty() || !undoStack) {
		qDebug() << "DrawingArea: Cannot change border style - no selection or no undo stack.";
		return;
	}

	qDebug() << "DrawingArea: Requesting to change border style for selected shapes to" << static_cast<int>(style);
	// ��������������
	undoStack->push(new ChangeBorderStyleCommand(this, selectedShapes, style));
	// ����� redo ���Զ����ã�����ʵ�ʵ��޸ĺͽ������
}

void DrawingArea::changeSelectedLineWidth(qreal width) {
	if (selectedLines.isEmpty() || !undoStack) return;

	/*for (ConnectionLine* line : selectedLines) {
		line->setLineWidth(width);
	}*/
	undoStack->push(new ChangeLineWidthCommand(this, selectedLines, width));
	update(); // ˢ�½���
}


// ... ���� DrawingArea ���� ...

void DrawingArea::changeSelectedShapeWidth(int width)
{
	if (selectedShapes.size() != 1 || !undoStack) {
		return;
	}

	Shape* shape = selectedShapes.first();
	if (!shape) return;

	// --- ��ȡ��ǰ���� (QRect)��Ȼ��ת��Ϊ QRectF ---
	QRect currentIntRect = shape->getRect();
	QRectF oldRect = QRectF(currentIntRect); // <--- �� QRect ���� QRectF

	// ����ֻ�ı��ȵ��¸������
	QRectF newRect = oldRect;
	newRect.setWidth(width); // QRectF �� setWidth ���� qreal (int ������ʽת��)

	// ������Ƿ���ĸı� (ʹ�ø���Ƚ�) ���¿����Ч
	// ע�⣺minShapeSize ͨ���� int����Ҫ�Ƚ� qreal >= int
	if (!qFuzzyCompare(oldRect.width(), newRect.width()) && newRect.width() >= static_cast<qreal>(minShapeSize))
	{
		// --- ���� QRectF ������ ---
		undoStack->push(new ResizeShapeCommand(this, shape, newRect)); // <--- ���ݵ��� newRect (QRectF)
	}
}

void DrawingArea::changeSelectedShapeHeight(int height)
{
	if (selectedShapes.size() != 1 || !undoStack) {
		return;
	}

	Shape* shape = selectedShapes.first();
	if (!shape) return;

	// --- ��ȡ��ǰ���� (QRect)��Ȼ��ת��Ϊ QRectF ---
	QRect currentIntRect = shape->getRect();
	QRectF oldRect = QRectF(currentIntRect); // <--- �� QRect ���� QRectF

	// ����ֻ�ı�߶ȵ��¸������
	QRectF newRect = oldRect;
	newRect.setHeight(height); // QRectF �� setHeight ���� qreal

	// ���߶��Ƿ���ĸı� (ʹ�ø���Ƚ�) ���¸߶���Ч
	if (!qFuzzyCompare(oldRect.height(), newRect.height()) && newRect.height() >= static_cast<qreal>(minShapeSize))
	{
		// --- ���� QRectF ������ ---
		undoStack->push(new ResizeShapeCommand(this, shape, newRect)); // <--- ���ݵ��� newRect (QRectF)
	}
}

void DrawingArea::changeSelectedlineStyle(Qt::PenStyle style) {
	if (selectedLines.isEmpty()) {
		QMessageBox::information(this, tr("Tip"), tr("Please select one or more connection lines first."));
		return;
	}
	undoStack->push(new ChangeLineStyleCommand(this, selectedLines, style));
}
// --- ʵ�������Ĺ������� ---
void DrawingArea::changeSelectedLineArrowState(bool hasArrow) {
	if (selectedLines.isEmpty()) {
		return; // û��ѡ�е�����
	}

	bool changed = false;
	for (ConnectionLine* line : selectedLines) {
		if (line && line->hasArrowHead() != hasArrow) { // ���״̬�Ƿ���ĸı�
			line->setHasArrowHead(hasArrow);
			changed = true;
		}
	}

	if (changed) {
		update(); // �����ػ�����ʾ�����ؼ�ͷ
		lineFormatChanged(); // ֪ͨ FormatPanel ���£�����״̬��һ��ʱ����˸�ѡ��
		// ע�⣺������ʱ����� Undo/Redo ��������Ҫ��Ӧ�������ﴴ���� push ����
	}
}


QPointF DrawingArea::mapToScene(const QPointF& viewportPos) const {
	if (!scrollAreaPtr) {
		// ���û�й������򣬼򵥵ؽ������ŷ���
		return viewportPos / m_scaleFactor;
	}
	// ��Ϲ�����λ�ú��������ӽ���ת��
	/*QPointF widgetPos = viewportPos -QPointF(scrollAreaPtr->horizontalScrollBar()->value(),
		scrollAreaPtr->verticalScrollBar()->value());
	return widgetPos / m_scaleFactor;*/
	return viewportPos / m_scaleFactor;
}


void DrawingArea::updateTextEditorGeometry() {
	// �������ڱ༭״̬������ָ����Чʱ��ִ��
	if (!m_isEditingText || !m_editingTextBox || !m_textEditor || !scrollAreaPtr) {
		return;
	}

	// 1. ��ȡ TextBox ���߼� (����) ����
	QRect sceneRect = m_editingTextBox->getRect();

	// 2. Ӧ�õ�ǰ���������ӣ��õ������� DrawingArea ��������ϵ�µľ���
	QTransform transform;
	transform.scale(m_scaleFactor, m_scaleFactor);
	QRectF scaledWidgetRect = transform.mapRect(QRectF(sceneRect));

	// 3. ��ȥ��ǰ�Ĺ�����ƫ�������õ����ӿ� (Viewport) �еľ���
	/*QPointF scrollOffset(scrollAreaPtr->horizontalScrollBar()->value(),
		scrollAreaPtr->verticalScrollBar()->value());
	QRectF viewportRect = scaledWidgetRect.translated(-scrollOffset);*/
	QRectF viewportRect = scaledWidgetRect;
	// 4. ���� QLineEdit �ļ���λ�� (ת��Ϊ�������ز�΢��)
	//    ʹ�� toAlignedRect() �������ض��룬����ģ��
	m_textEditor->setGeometry(viewportRect.toAlignedRect().adjusted(1, 1, -1, -1));

	// 5. ȷ���༭�������ϲ�
	m_textEditor->raise();

}