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


// MIME 类型常量的外部声明
const QString DIAGRAM_EDITOR_MIME_TYPE = "application/x-diagram-editor-items";
// --- DiagramEditor 实现 ---

DiagramEditor::DiagramEditor(QWidget* parent) : QWidget(parent) {

	undoStack = new QUndoStack(this); // 创建 UndoStack
	// 1. 初始化滚动区域和绘图区域
	scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(false);
	//scrollArea->setBackgroundRole(QPalette::Dark);
	scrollArea->setStyleSheet("QScrollArea { background-color: #0F172A; border: none; }");


	drawingArea = new DrawingArea(scrollArea, this->undoStack, this);
	drawingArea->setObjectName("DrawingArea");
	setDiagramEditorSize(1600, 1050);
	scrollArea->setWidget(drawingArea);
	scrollArea->setAlignment(Qt::AlignCenter);

	// 2. 初始化工具面板和属性栏面板
	initToolPanel();
	initFormatPanel();


	// 3. 创建并设置分割器
	splitter = new QSplitter(Qt::Horizontal, this);
	splitter->addWidget(toolscrollArea);
	splitter->addWidget(scrollArea);
	splitter->addWidget(formatScrollArea); // 新增右边栏
	splitter->setSizes({ 80, width() - 80 - 250, 250 }); // 调整初始宽度 (工具栏变窄)
	splitter->setCollapsible(0, false);
	splitter->setCollapsible(2, false);
	splitter->setStretchFactor(0, 0);
	splitter->setStretchFactor(1, 1);
	splitter->setStretchFactor(2, 0); // 属性栏
	// 添加 Splitter 样式
	splitter->setStyleSheet(
		"QSplitter::handle {"
		"    background-color: #1E293B; /* 面板背景色 */"
		"    border: 1px solid #334155; /* 分割线颜色 */"
		"    width: 3px; /* 分割线宽度 */"
		"    margin: 1px 0;"
		"}"
		"QSplitter::handle:horizontal {"
		"    width: 3px;"
		"}"
		"QSplitter::handle:vertical {"
		"    height: 3px;"
		"}"
		"QSplitter::handle:hover {"
		"    background-color: #3B82F6; /* 悬停时蓝色 */"
		"}"
	);
	// 4. 设置主布局
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	// --- 修改：创建 MenuBar 时传递 undoStack ---
	menuBar = new MenuBar(this->undoStack, this); // <--- 传递 undoStack
	menuBar->setStyleSheet(
		"QMenuBar {"
		"    background-color: #1E293B; /* 面板背景色 */"
		"    color: #E2E8F0; /* 浅灰色文字 */"
		"    spacing: 5px; /* 菜单项之间的间距 */"
		"    border-bottom: 1px solid #334155; /* 深灰色边框 */"
		"}"
		"QMenuBar::item {"
		"    background-color: transparent;" // 正常状态透明
		"    padding: 5px 10px;"
		"    border-radius: 4px;"
		"}"
		"QMenuBar::item:selected {" /* 悬停或键盘选中 */
		"    background-color: #28354C; /* 稍亮背景 */"
		"    color: #FFFFFF; /* 白色文字 */"
		"}"
		"QMenuBar::item:pressed {" /* 按下时 */
		"    background-color: #3B82F6; /* 蓝色 */"
		"    color: #FFFFFF;"
		"}"
		// --- QMenu (下拉菜单) 样式 ---
		"QMenu {"
		"    background-color: #1E293B;"
		"    color: #E2E8F0;"
		"    border: 1px solid #334155;"
		"    padding: 5px;"
		"    margin: 2px;" // 与 MenuBar 之间留点空隙
		"}"
		"QMenu::item {"
		"    padding: 5px 20px 5px 20px;" // 调整内边距
		"    border-radius: 4px;"
		"}"
		"QMenu::item:selected {"
		"    background-color: #3B82F6; /* 选中时蓝色 */"
		"    color: #FFFFFF;"
		"}"
		"QMenu::separator {"
		"    height: 1px;"
		"    background-color: #334155; /* 分隔线颜色 */"
		"    margin: 4px 0;"
		"}"
	);
	mainLayout->setMenuBar(menuBar);
	mainLayout->addWidget(splitter);
	setLayout(mainLayout);

	// 5. 连接菜单栏信号
	connect(menuBar, &MenuBar::newWindowTriggered, this, &DiagramEditor::createNewWindow);
	connect(menuBar, &MenuBar::openTriggered, this, &DiagramEditor::onOpen);
	connect(menuBar, &MenuBar::saveTriggered, this, &DiagramEditor::onSave);
	connect(menuBar, &MenuBar::swichDrawingArea, this, &DiagramEditor::switchAera);
	connect(menuBar, &MenuBar::deleteTriggered, this, &DiagramEditor::onDelete);
	connect(menuBar, &MenuBar::cutTriggered, this, &DiagramEditor::onCut); // <--- 连接剪切信号
	connect(menuBar, &MenuBar::alignTopTriggered, this, &DiagramEditor::onAlignTop);
	connect(menuBar, &MenuBar::alignVCenterTriggered, this, &DiagramEditor::onAlignVCenter);
	connect(menuBar, &MenuBar::alignBottomTriggered, this, &DiagramEditor::onAlignBottom);

	// 连接图层操作信号
	connect(menuBar, &MenuBar::moveToTopLayer, drawingArea, &DrawingArea::onMoveToTopLayer);
	connect(menuBar, &MenuBar::moveToBottomLayer, drawingArea, &DrawingArea::onMoveToBottomLayer);
	connect(menuBar, &MenuBar::moveUpOneLayer, drawingArea, &DrawingArea::onMoveUpOneLayer);
	connect(menuBar, &MenuBar::moveDownOneLayer, drawingArea, &DrawingArea::onMoveDownOneLayer);

	// --- 新增：连接格式菜单信号到 DiagramEditor 的槽 ---
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
	connect(formatPanel, &FormatPanel::borderStyleChanged, this, &DiagramEditor::onSelectedShapesBorderStyle); // <--- 连接新信号
	connect(formatPanel, &FormatPanel::lineWidthChanged, this, &DiagramEditor::onSelectedLineWidth);
	connect(formatPanel, &FormatPanel::changeFillColorTriggered, this, &DiagramEditor::onFormatChangeFillColor);
	connect(formatPanel, &FormatPanel::shapeWidthChanged, this, &DiagramEditor::onSelectedShapeWidthChanged);
	connect(formatPanel, &FormatPanel::shapeHeightChanged, this, &DiagramEditor::onSelectedShapeHeightChanged);
	connect(formatPanel, &FormatPanel::changeLineColorTriggered, this, &DiagramEditor::onFormatChangeLineColor);
	connect(formatPanel, &FormatPanel::lineStyleChanged, this, &DiagramEditor::onSelectedlineStyleChanged);
	connect(formatPanel, &FormatPanel::arrowStateChanged, this, &DiagramEditor::onSelectedLineArrowStateChanged); // <<<--- 新增连接

	connect(formatPanel, &FormatPanel::changeNofillStateTriggered, this, &DiagramEditor::onFormatToggleNoFill);
	connect(formatPanel, &FormatPanel::fontChanged, this, &DiagramEditor::onSelectedTextFontChanged);
	connect(formatPanel, &FormatPanel::changeTextColorTriggered, this, &DiagramEditor::onFormatChangeTextColor);
	// --- 新增画布属性连接 ---
	connect(formatPanel, &FormatPanel::canvasWidthChanged, this, &DiagramEditor::onPanelCanvasWidthChanged);
	connect(formatPanel, &FormatPanel::canvasHeightChanged, this, &DiagramEditor::onPanelCanvasHeightChanged);
	connect(formatPanel, &FormatPanel::canvasZoomChanged, this, &DiagramEditor::onPanelCanvasZoomChanged);

	connect(drawingArea, &DrawingArea::shapeFormat, formatPanel, &FormatPanel::updateSelectedShapes);
	connect(drawingArea, &DrawingArea::lineFormat, formatPanel, &FormatPanel::updateSelectedLines);
	// --- 新增画布属性连接 ---
	connect(drawingArea, &DrawingArea::zoomChanged, this, &DiagramEditor::onDrawingAreaZoomChanged);
	connect(drawingArea, &DrawingArea::canvasSizeChanged, this, &DiagramEditor::onDrawingAreaSizeChanged);
	
	// 6. 连接工具面板信号到 DrawingArea (通过 DiagramEditor 中转设置模式)
	connect(toolPanel, &ToolPanel::shapeToolSelected, this, [this](int shapeType) {
		qDebug() << "DiagramEditor: shapeToolSelected signal, type:" << shapeType;
		if (drawingArea) {
			clickLineMode = false; // 取消点击连接线模式

			if (shapeType == -1 || (drawingArea->currentToolMode == DrawingArea::ShapeDrawingTool && drawingArea->currentShapeType == shapeType)) {
				// 如果是取消(-1) 或者 点击当前已选中的图形工具 -> 返回选择模式
				drawingArea->setToolMode(DrawingArea::SelectionTool);
				toolPanel->uncheckAllTools(); // 手动取消工具栏按钮状态
			}
			else {
				// 设置新形状类型并进入形状绘制模式
				drawingArea->currentShapeType = shapeType;
				drawingArea->setToolMode(DrawingArea::ShapeDrawingTool);
			}
		}
		});

	connect(toolPanel, &ToolPanel::connectionToolActivated, this, [this]() {
		qDebug() << "DiagramEditor: connectionToolActivated signal";
		if (drawingArea) {
			// 点击连接线工具切换模式
			if (drawingArea->currentToolMode == DrawingArea::ConnectionTool) {
				// 如果当前是连接模式，则切换回选择模式
				drawingArea->setToolMode(DrawingArea::SelectionTool);
				clickLineMode = false; // 确保状态同步
				// toolPanel->uncheckAllTools(); // 取消工具栏选中
			}
			else {
				// 否则，进入连接模式
				drawingArea->currentShapeType = -1; // 清除非连接模式的图形类型
				drawingArea->setToolMode(DrawingArea::ConnectionTool);
				clickLineMode = true; // 启用点击连接模式 (如果需要)
				// toolPanel->uncheckAllToolsExcept(-2); // -2 代表连接线工具？需要约定
			}
		}
		});

	connect(toolPanel, &ToolPanel::toSelectModel, this, [this]() {
		drawingArea->setToolMode(DrawingArea::SelectionTool);
		drawingArea->selectedLines.clear();
		drawingArea->selectedShapes.clear();

		});

	// 7. 连接 DrawingArea 的右键信号 (用于取消工具栏状态)
	connect(drawingArea, &DrawingArea::clearSelection, this, &DiagramEditor::handleClearSelection);
	// --- 初始化 FormatPanel 显示 ---
	updateFormatPanelAll(); // 在最后调用一次，确保初始状态正确显示
	//retranslateUi();
}


void DiagramEditor::changeEvent(QEvent* event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(); // **调用 retranslateUi 更新文本**
	}
	QWidget::changeEvent(event); // **调用基类的实现**
}

void DiagramEditor::retranslateUi() {

	QWidget* topLevelWindow = window(); // 获取此 DiagramEditor 所在的顶级窗口
	if (topLevelWindow) {
		topLevelWindow->setWindowTitle(tr("Qt Diagramming Software")); // 使用 tr()
	}

}
// --- 初始化工具面板  ---
void DiagramEditor::initToolPanel() {
	toolscrollArea = new QScrollArea(this);
	toolscrollArea->setWidgetResizable(true);
	// --- **修改:** 设置 ToolPanel 滚动区样式 ---
	toolscrollArea->setStyleSheet(
		"QScrollArea {"
		"    background-color: #1E293B;" // 面板背景色
		"    border: none;" // 通常不需要边框
		"}"
		"QScrollBar:vertical {" /* 垂直滚动条 */
		"    border: none;"
		"    background: #1E293B;"
		"    width: 8px;"
		"    margin: 0px 0px 0px 0px;"
		"}"
		"QScrollBar::handle:vertical {"
		"    background: #475569;" /* 滑块颜色 */
		"    min-height: 20px;"
		"    border-radius: 4px;"
		"}"
		"QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
		"    height: 0px;" // 隐藏两端箭头
		"}"
		"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
		"    background: none;"
		"}"
		// 水平滚动条类似，如果需要的话
	);
	toolscrollArea->setFixedWidth(80); // 变窄，主要放图标
	//toolscrollArea->setMaximumWidth(80); // 限制最大宽度
	toolPanel = new ToolPanel(this);
	toolscrollArea->setWidget(toolPanel);
}

void DiagramEditor::initFormatPanel() {
	formatScrollArea = new QScrollArea(this);
	formatScrollArea->setWidgetResizable(true);
	//formatScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 禁用水平滚动条
	formatScrollArea->setStyleSheet(
		"QScrollArea {"
		"    background-color: #1E293B;" /* 面板背景色 */
		"    border: none;"
		"}"
		/* --- 垂直滚动条 --- */
		"QScrollBar:vertical {"
		"    border: none;"
		"    background: #1E293B;" /* 滚动条背景色 */
		"    width: 8px;"          /* 滚动条宽度 */
		"    margin: 0px 0px 0px 0px;" /* 无边距 */
		"}"
		"QScrollBar::handle:vertical {"
		"    background: #475569;" /* 滑块颜色 */
		"    min-height: 20px;"     /* 滑块最小高度 */
		"    border-radius: 4px;"   /* 滑块圆角 */
		"}"
		"QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
		"    height: 0px;"          /* 隐藏两端箭头区域 */
		"    background: none;"
		"}"
		"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
		"    background: none;"     /* 滚动条背景（滑块之外的区域）*/
		"}"
		/* --- 水平滚动条 --- */
		"QScrollBar:horizontal {"
		"    border: none;"
		"    background: #1E293B;" /* 滚动条背景色 */
		"    height: 8px;"         /* 滚动条高度 */
		"    margin: 0px 0px 0px 0px;" /* 无边距 */
		"}"
		"QScrollBar::handle:horizontal {"
		"    background: #475569;" /* 滑块颜色 */
		"    min-width: 20px;"      /* 滑块最小宽度 */
		"    border-radius: 4px;"   /* 滑块圆角 */
		"}"
		"QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
		"    width: 0px;"           /* 隐藏两端箭头区域 */
		"    background: none;"
		"}"
		"QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
		"    background: none;"     /* 滚动条背景（滑块之外的区域）*/
		"}"
	);
	formatScrollArea->setFixedWidth(260);
	formatPanel = new FormatPanel(this);
	formatScrollArea->setWidget(formatPanel);
	//formatScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


}

// --- 处理右键信号  ---
void DiagramEditor::handleClearSelection() {
	qDebug() << "DiagramEditor: Handling right button press signal.";
	if (toolPanel) {
		//toolPanel->uncheckAllTools(); // 取消工具栏按钮选中
		toolPanel->selectSelectionTool(); // 这会取消其他按钮并选中选择按钮
	}
	clickLineMode = false; // 确保取消点击模式
	// DrawingArea 内部的右键处理已经切换回了 SelectionTool 模式
}

// --- 设置绘图区大小  ---
void DiagramEditor::setDiagramEditorSize(int size_w, int size_h) {
	if (drawingArea) {
		drawingArea->setFixedSize(size_w, size_h);
	}
}



Shape* DiagramEditor::getShape(int currentShapeType, QRect rect) {
	Shape* newShape = nullptr;
	QRect normalizedRect = rect.normalized();

	// --- 定义一些默认颜色和样式 ---
	const QColor BorderBlue(59, 130, 246);
	const QColor BorderRed(239, 68, 68);
	const QColor BorderGreen(16, 185, 129);
	const QColor BorderPurple(147, 51, 234);
	const QColor BorderOrange(217, 119, 6);
	const QColor BorderLime(74, 222, 128);
	const QColor BorderGray(Qt::darkGray);
	const QColor TextBlack(Qt::black);
	const QColor FillWhite(Qt::white);     // 默认填充色
	const qreal DefaultBorderWidth = 1.5;  // 默认边框宽度
	const bool DefaultIsFilled = false;    // 默认不填充
	const Qt::PenStyle DefaultBorderStyle = Qt::SolidLine; // <--- 默认边框样式

	// --- 根据类型创建，使用新的构造函数签名，包含 borderStyle ---
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
		// 注意：borderStyle 参数在 fillColor 之后，radius 参数之前
		newShape = new RoundedRectangle(normalizedRect, BorderOrange, DefaultBorderWidth,
			DefaultIsFilled, FillWhite, DefaultBorderStyle, 15); // 15 是圆角半径
		break;
	case 5: // Parallelogram
		newShape = new Parallelogram(normalizedRect, BorderLime, DefaultBorderWidth,
			DefaultIsFilled, FillWhite, DefaultBorderStyle, 20); // 20 是偏移量
		break;
	case 6:
		newShape = new TextBox(normalizedRect,           // Rect
			BorderGray,             // Border Color (TextBox 默认 darkGray)
			0.0,                    // Border Width (TextBox 默认 1.0)
			false,                  // Is Filled (TextBox 默认 false)
			FillWhite,              // Fill Color
			DefaultBorderStyle,     // Border Style (TextBox 默认 Solid)
			QObject::tr("Double-click to edit"), // Text - 已国际化
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
		// ID 在 Shape 构造函数中已生成
		qDebug() << QObject::tr("Created shape type") << currentShapeType << QObject::tr(", ID:") << newShape->getId();
	}
	return newShape;
}

// --- 菜单槽函数实现 ---


void DiagramEditor::createNewWindow()
{
	qDebug() << QObject::tr("Creating a new Diagram Editor window..."); // 调试输出
	DiagramEditor* newEditor = new DiagramEditor(); // 创建一个新的 DiagramEditor 实例

	// 可选：为新窗口设置属性
	newEditor->setWindowTitle(QObject::tr("Diagram Editor - New")); // 设置窗口标题
	newEditor->resize(this->size()); // 让新窗口和当前窗口一样大
	// 或者设置一个默认大小: newEditor->resize(1024, 768);

	// 关键：设置窗口关闭时自动删除关联的 DiagramEditor 对象，防止内存泄漏。
	// 这使得新窗口是独立的。
	newEditor->setAttribute(Qt::WA_DeleteOnClose);

	newEditor->show(); // 显示新窗口
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

	// 清理绘图区
	drawingArea->selectedShapes.clear();
	drawingArea->selectedLines.clear();
	drawingArea->showControlPoints = false; // 隐藏控制点
	qDeleteAll(drawingArea->allLines);
	drawingArea->allLines.clear();
	qDeleteAll(drawingArea->shapes);
	drawingArea->shapes.clear();
	drawingArea->updateSelectionStates(); // 确保状态同步

	QJsonObject rootObj = doc.object();
	QMap<QString, Shape*> shapeIdMap;
	int maxX = 0, maxY = 0;

	// --- **新增：加载画布尺寸** ---
	int loadedCanvasWidth = 2000;  // 提供默认值
	int loadedCanvasHeight = 1500; // 提供默认值
	if (rootObj.contains("canvasWidth") && rootObj["canvasWidth"].isDouble()) {
		loadedCanvasWidth = rootObj["canvasWidth"].toInt(loadedCanvasWidth); // 如果存在则读取
	}
	if (rootObj.contains("canvasHeight") && rootObj["canvasHeight"].isDouble()) {
		loadedCanvasHeight = rootObj["canvasHeight"].toInt(loadedCanvasHeight); // 如果存在则读取
	}

	// 加载图形 (使用 Shape::loadFromJson)
	if (rootObj.contains("shapes") && rootObj["shapes"].isArray()) {
		QJsonArray shapesArray = rootObj["shapes"].toArray();
		for (const QJsonValue& value : shapesArray) {
			if (!value.isObject()) continue;
			QJsonObject shapeObj = value.toObject();
			QString type = shapeObj.value("type").toString("unknown");
			Shape* loadedShape = nullptr;

			// --- 根据类型创建具体对象 ---
			// (这里仍然需要类型判断来 new 正确的子类)
			if (type == "rectangle") loadedShape = new Rectangle(QRect()); // 临时Rect
			else if (type == "circle") loadedShape = new Circle(QRect());
			else if (type == "triangle") loadedShape = new Triangle(QRect());
			else if (type == "diamond") loadedShape = new Diamond(QRect());
			else if (type == "rounded_rectangle") loadedShape = new RoundedRectangle(QRect());
			else if (type == "parallelogram") loadedShape = new Parallelogram(QRect());
			else if (type == "textbox") loadedShape = new TextBox(QRect());
			else {
				qWarning() << "onOpen: Unknown shape type found in JSON:" << type;
				continue; // 跳过未知类型
			}

			// --- 使用 loadFromJson 加载属性 ---
			if (loadedShape) {
				loadedShape->loadFromJson(shapeObj); // 调用虚函数加载属性

				// 检查加载后的ID是否有效
				QString shapeId = loadedShape->getId();
				if (shapeId.isEmpty()) { // 如果 loadFromJson 没有设置ID（理论上不应该）
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

	// 加载连接线 (使用 ConnectionLine::fromJson，这部分不变)
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
		QSize canvasSize = drawingArea->size(); // 获取 drawingArea 的当前尺寸
		rootObj["canvasWidth"] = canvasSize.width();
		rootObj["canvasHeight"] = canvasSize.height();
	}
	else {
		// 可以选择保存默认值或跳过
		rootObj["canvasWidth"] = 1500; 
		rootObj["canvasHeight"] = 1200; 
	}
	// --- **结束新增部分** ---
	// 保存图形 (使用 Shape::toJson)
	QJsonArray shapesArray;
	for (Shape* shape : drawingArea->shapes) {
		if (!shape) continue;
		QJsonObject shapeObj = shape->toJson(); 

		// 添加类型信息 (如果基类 toJson 不包含)
		// 可以优化：让每个派生类的 toJson 添加自己的类型
		QString typeStr = "unknown";
		if (dynamic_cast<Rectangle*>(shape)) typeStr = "rectangle";
		else if (dynamic_cast<Circle*>(shape)) typeStr = "circle";
		else if (dynamic_cast<Triangle*>(shape)) typeStr = "triangle";
		else if (dynamic_cast<Diamond*>(shape)) typeStr = "diamond";
		else if (dynamic_cast<RoundedRectangle*>(shape)) typeStr = "rounded_rectangle"; // toJson 已包含
		else if (dynamic_cast<Parallelogram*>(shape)) typeStr = "parallelogram"; // toJson 已包含
		else if (dynamic_cast<TextBox*>(shape)) typeStr = "textbox"; // toJson 已包含
		else { qWarning() << "onSave: Unknown shape type for ID:" << shape->getId(); continue; }

		// 如果 toJson 没有包含 type，在这里添加
		if (!shapeObj.contains("type")) {
			shapeObj["type"] = typeStr;
		}

		shapesArray.append(shapeObj);
	}
	rootObj["shapes"] = shapesArray;

	// 保存连接线 (使用 ConnectionLine::toJson，不变)
	QJsonArray allLinesArray;
	for (ConnectionLine* line : drawingArea->allLines) {
		if (!line) continue;
		// 安全检查：确保连接线指向的图形仍在列表中
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

	// 写入 JSON 文件 (不变)
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
		if (!drawingArea) { // 再次检查 drawingArea
			QMessageBox::warning(this, tr("Error"), tr("Drawing area is not available to save image."));
			
			QMessageBox::information(this, tr("Success"), tr("Diagram data has been successfully saved to %1 (image not saved).").arg(QFileInfo(jsonPath).fileName()));
			return;
		}

		QFileInfo info(jsonPath); // 获取 JSON 文件的信息
		QString baseName = info.baseName(); // 文件名（不含扩展名）
		QString defaultPath = info.absolutePath(); // 文件所在目录

		// **修改：设置文件对话框的文件过滤器，包含 SVG 和 PNG**
		QString selectedFilter; // 用于接收用户选择的过滤器
		QString imagePath = QFileDialog::getSaveFileName(this,
			tr("Save Image As"),
			QDir(defaultPath).filePath(baseName), // 默认文件名 (无扩展名)
			tr("SVG Image (*.svg);;PNG Image (*.png)"),
			&selectedFilter // 接收用户选择的过滤器
		);

		if (!imagePath.isEmpty()) {
			bool saveSuccess = false;
			QString errorMsg;

			// **根据选择的过滤器或文件扩展名判断保存格式**
			QString chosenSuffix = QFileInfo(imagePath).suffix().toLower();
			bool saveAsSvg = false;

			if (!selectedFilter.isEmpty()) { // 优先根据选择的过滤器判断
				if (selectedFilter.contains("*.svg", Qt::CaseInsensitive)) {
					saveAsSvg = true;
					if (chosenSuffix != "svg") imagePath += ".svg"; // 确保扩展名正确
				}
				else if (selectedFilter.contains("*.png", Qt::CaseInsensitive)) {
					if (chosenSuffix != "png") imagePath += ".png"; // 确保扩展名正确
				}
				else {
					
					if (chosenSuffix == "svg") saveAsSvg = true;
					else if (chosenSuffix != "png") imagePath += ".png"; // 默认 PNG
				}
			}
			else { 
				if (chosenSuffix == "svg") {
					saveAsSvg = true;
				}
				else if (chosenSuffix != "png") {
					// 如果后缀不是 svg 也不是 png，默认添加 .png
					imagePath += ".png";
				}
				
			}


			if (saveAsSvg) {
				// --- 保存为 SVG ---
				QSvgGenerator generator;
				generator.setFileName(imagePath);
				generator.setSize(drawingArea->size()); // 设置 SVG 文档大小为画布大小
				generator.setViewBox(drawingArea->rect()); // 设置视图框
				generator.setTitle(tr("Diagram Export"));
				generator.setDescription(tr("Generated from Diagram Editor"));

				QPainter svgPainter;
				if (svgPainter.begin(&generator)) {
					svgPainter.setRenderHint(QPainter::Antialiasing);
					// 渲染绘图区内容到 SVG 画笔
					if (auto* daWidget = qobject_cast<QWidget*>(drawingArea)) {
						// 渲染前设置背景（SVG默认透明，如果需要白色背景）
						// svgPainter.fillRect(drawingArea->rect(), Qt::white); // 取消注释以添加白色背景
						daWidget->render(&svgPainter);
						saveSuccess = true;
					}
					else {
						errorMsg = tr("Cannot cast drawing area to QWidget for SVG rendering.");
						qWarning() << errorMsg;
					}
					svgPainter.end(); // 结束绘制并写入文件
				}
				else {
					errorMsg = tr("Could not begin painting on SVG generator.");
					qWarning() << errorMsg;
				}

			}
			else {
				// --- 保存为 PNG (与之前类似) ---
				QImage image(drawingArea->size(), QImage::Format_ARGB32_Premultiplied);
				image.fill(Qt::white); // PNG 通常需要背景色
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

			// --- 根据保存结果显示消息 ---
			if (saveSuccess) {
				QMessageBox::information(this, tr("Success"), tr("Diagram and image (%1) have been successfully saved.").arg(QFileInfo(imagePath).suffix().toUpper()));
				return; // 成功保存图片后结束
			}
			else {
				QMessageBox::warning(this, tr("Error"), tr("Failed to save image: %1").arg(errorMsg));
				// 即使图片保存失败，仍然提示 JSON 已保存
				QMessageBox::information(this, tr("Success"), tr("Diagram data has been successfully saved to %1 (image saving failed).").arg(QFileInfo(jsonPath).fileName()));
				return;
			}
		}
		else {
			// 用户取消了图片保存对话框
			qDebug() << "Image save cancelled by user.";
		}
	}

	QMessageBox::information(this, tr("Success"), tr("Diagram data has been successfully saved to %1").arg(QFileInfo(jsonPath).fileName()));
}


// 实现新的槽函数
// 实现 onCut 槽函数：
void DiagramEditor::onCut() {
	if (drawingArea) {
		// 1. 先执行复制操作
		drawingArea->cutActionTriggered(); // <--- 需要在 DrawingArea 中实现这个方法
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
		// 调用 DrawingArea 的方法来执行删除，该方法应使用 Undo 命令
		drawingArea->deleteSelection(); // <--- 需要在 DrawingArea 中实现这个方法
	}
}

// 实现对齐槽函数：
void DiagramEditor::onAlignTop() {
	if (drawingArea) {
		drawingArea->alignSelectedShapes(Qt::AlignTop); // 调用 DrawingArea 方法
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

// switchAera (与原始一致)
void DiagramEditor::switchAera() { // 切换网格
	if (drawingArea) {
		drawingArea->isGridMode = !drawingArea->isGridMode;
		drawingArea->update();
	}
}

// --- 新增：格式菜单槽函数的实现 ---

void DiagramEditor::onFormatChangeBorderColor() {
	if (drawingArea) {
		drawingArea->requestChangeBorderColor(); // 调用 DrawingArea 的方法
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
		// 直接调用 DrawingArea 的方法，该方法内部会处理 Undo 命令
		drawingArea->changeSelectedShapeWidth(width);
		// 注意：DrawingArea 的方法在 Push 命令后，命令的 redo 会触发 update 和信号
		// 所以这里通常不需要再手动更新 formatPanel
		// formatPanel->updateSelectedShapes(drawingArea->selectedShapes); // 理论上命令执行后会被调用
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
		if (!drawingArea->selectedLines.isEmpty()) { // 只有在确实有线条被选中时才更新
			formatPanel->updateSelectedLines(drawingArea->selectedLines); // <--- 添加这一行
		}
	}
}

void DiagramEditor::onFormatChangeLineStyle() {
	if (drawingArea) {
		drawingArea->requestChangeLineStyle();
		if (!drawingArea->selectedLines.isEmpty()) { // 只有在确实有线条被选中时才更新
			formatPanel->updateSelectedLines(drawingArea->selectedLines); // <--- 添加这一行
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

// 实现新的槽函数
void DiagramEditor::onSelectedShapesBorderStyle(Qt::PenStyle style) {
	if (drawingArea) {
		drawingArea->changeSelectedShapesBorderStyle(style);
		// 命令执行后，redo 会触发 drawingArea->shapeFormatChanged() 来更新 FormatPanel
	}
}

void DiagramEditor::onSelectedlineStyleChanged(Qt::PenStyle style) {
	if (drawingArea) {
		drawingArea->changeSelectedlineStyle(style);
		formatPanel->updateSelectedLines(drawingArea->selectedLines);
	}

}
// --- 实现新增的槽函数 ---
void DiagramEditor::onSelectedLineArrowStateChanged(bool hasArrow) {
	if (drawingArea) {
		// 调用 DrawingArea 的方法来实际修改选中的线条
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
// --- 新增：响应 FormatPanel 画布属性改变的槽 ---
void DiagramEditor::onPanelCanvasWidthChanged(int width) {
	if (drawingArea) {
		QSize currentSize = drawingArea->canvasSize();
		drawingArea->setCanvasSize(QSize(width, currentSize.height()));
		// setCanvasSize 内部会发射 sizeChanged 信号，然后会触发 onDrawingAreaSizeChanged 更新面板
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
		drawingArea->setZoomFactor(zoomPercentage / 100.0); // 将百分比转为因子
		// setZoomFactor 内部会发射 zoomChanged 信号，然后触发 onDrawingAreaZoomChanged 更新面板
	}
}


// --- 新增：响应 DrawingArea 属性改变的槽 ---
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

// --- 新增：统一更新 FormatPanel 的方法 ---
void DiagramEditor::updateFormatPanelAll() {
	if (!drawingArea || !formatPanel) return;

	// 1. 更新选中项信息
	formatPanel->updateSelectedShapes(drawingArea->selectedShapes); // 传递选中的图形
	formatPanel->updateSelectedLines(drawingArea->selectedLines);   // 传递选中的线条

	// 2. 更新画布属性信息
	formatPanel->updateCanvasProperties(drawingArea->canvasSize(), drawingArea->getZoomFactor());
}

QUndoStack* DiagramEditor::getUndoStack() const {
	return undoStack;
}


// --- DrawingArea 实现 ---

// 构造函数 (基本不变, 初始化 lineBeingStretched 等)
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
	m_editingTextBox(nullptr), // <--- 新增
	m_textEditor(nullptr),     // <--- 新增
	m_isEditingText(false),    // <--- 新增
	m_currentSnapOffset(0, 0) // <<<--- 初始化吸附偏移量为零
{
	setGridSize(20);
	setAcceptDrops(true);
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);
	currentToolMode = SelectionTool; // 默认选择模式
	currentShapeType = -1;
	tempShape = nullptr;
	tempLine = nullptr;
	connectionStartShape = nullptr; // 用于连接模式
	connectionStartPointIndex = -1;
	m_alignmentGuides.clear(); // 确保开始时参考线列表为空
	// lineBeingStretched 初始化为 nullptr
	// currentHandle 初始化为 -1
	// hoveredShape 初始化为 nullptr
	// hoveredConnectionPointIndex 初始化为 -1

	// 清理旧尺寸策略 (如果需要)
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); // 使用固定尺寸
	if (scrollAreaPtr) {
		connect(scrollAreaPtr->horizontalScrollBar(), &QScrollBar::valueChanged,
			this, &DrawingArea::updateTextEditorGeometry);
		connect(scrollAreaPtr->verticalScrollBar(), &QScrollBar::valueChanged,
			this, &DrawingArea::updateTextEditorGeometry);
	}
}


// 析构函数 (不变)
DrawingArea::~DrawingArea() {
	qDeleteAll(allLines);
	allLines.clear();
	qDeleteAll(shapes);
	shapes.clear();
	delete tempShape;
	delete tempLine;
}

// setGridSize (不变)
void DrawingArea::setGridSize(int size) {
	gridSize = qMax(5, size);
	update();
}

// setToolMode (不变)
void DrawingArea::setToolMode(ToolMode mode) {
	resetActionState(); // 切换模式前重置
	currentToolMode = mode;

	switch (mode) {
	case SelectionTool:
		setCursor(Qt::ArrowCursor);
		currentShapeType = -1;
		// 如果有选中项，显示控制点
		showControlPoints = !selectedShapes.isEmpty() || !selectedLines.isEmpty();
		break;
	case ShapeDrawingTool:
		if (currentShapeType != -1) {
			setCursor(Qt::CrossCursor);
			showControlPoints = false; // 绘图时不显示旧的控制点
		}
		else {
			currentToolMode = SelectionTool; // 回退
			setCursor(Qt::ArrowCursor);
		}
		break;
	case ConnectionTool:
		setCursor(Qt::CrossCursor);
		currentShapeType = -1;
		showControlPoints = false; // 连接时不显示旧的控制点
		break;
	}
	update();
}

qreal DrawingArea::getZoomFactor() const {
	return m_scaleFactor;
}
QSize DrawingArea::canvasSize() const {
	return size(); // 直接返回当前部件尺寸
}

// --- Setter Slots ---
void DrawingArea::setZoomFactor(double factor) {
	factor = std::max(0.1, std::min(factor, 5.0)); // 限制范围 10% - 500%
	if (qAbs(m_scaleFactor - factor) > 1e-4) { // 避免浮点数误差导致不必要的更新
		m_scaleFactor = factor;
		emit zoomChanged(m_scaleFactor);
		update(); // 触发重绘以应用缩放
	}
}

void DrawingArea::setCanvasSize(const QSize& newSize) {
	QSize clampedSize(std::max(100, newSize.width()), std::max(100, newSize.height())); // 限制最小尺寸
	if (size() != clampedSize) {
		setFixedSize(clampedSize); // 设置固定尺寸
		emit canvasSizeChanged(clampedSize); // 发射尺寸变化信号
		update();
		// 注意：如果 setFixedSize 内部不自动更新 scrollarea 的 viewport，可能需要手动触发
		if (scrollAreaPtr && scrollAreaPtr->widget() == this) {
			// scrollAreaPtr->updateGeometry(); // 或者其他方式通知 scrollarea 尺寸变了
		}
	}
}


int DrawingArea::findHandleForFreeEnd(const QPointF& pos, ConnectionLine* line) {
	if (!line || !line->isSelected()) { // 必须是选中的线
		return -1;
	}

	const qreal handleTolerance = ConnectionLine::handleSize * 0.75; // 手柄半径的容差倍数
	const qreal handleTolSq = handleTolerance * handleTolerance;

	// 检查起点 (仅当起点自由时)
	if (!line->isStartAttached()) {
		QPointF startP = line->getStartPointPos();
		qreal dxStart = pos.x() - startP.x();
		qreal dyStart = pos.y() - startP.y();
		if ((dxStart * dxStart + dyStart * dyStart) <= handleTolSq) {
			return 0; // 0 代表起点
		}
	}

	// 检查终点 (仅当终点自由时)
	if (!line->isEndAttached()) {
		QPointF endP = line->getEndPointPos();
		qreal dxEnd = pos.x() - endP.x();
		qreal dyEnd = pos.y() - endP.y();
		if ((dxEnd * dxEnd + dyEnd * dyEnd) <= handleTolSq) {
			return 1; // 1 代表终点
		}
	}

	return -1; // 未点中自由端点的手柄
}

void DrawingArea::cutActionTriggered() {
	copySelectionToClipboard();

	// 2. 然后执行删除操作 (deleteSelection 内部会处理 Undo)
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
		// 可选：检查纯文本 JSON 作为后备？
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
	QList<Shape*> newShapes;                 // 存储新创建的图形
	QList<ConnectionLine*> newLines;         // 存储新创建的线条
	QMap<QString, Shape*> oldIdToNewShapeMap; // 映射：旧 ID -> 新创建的 Shape 指针

	// --- 定义粘贴偏移量 ---
	const QPointF pasteOffset(20, 20); // 粘贴时向下向右移动 20 像素

	// --- 1. 反序列化并创建新图形 ---
	if (rootObj.contains("shapes") && rootObj["shapes"].isArray()) {
		QJsonArray shapesArray = rootObj["shapes"].toArray();
		for (const QJsonValue& value : shapesArray) {
			if (!value.isObject()) continue;
			QJsonObject shapeObj = value.toObject();
			QString type = shapeObj.value("type").toString("unknown");
			QString oldId = shapeObj.value("id").toString(); // 获取旧 ID
			Shape* loadedShape = nullptr;

			// 创建正确的图形类型 (与 onOpen 逻辑相同)
			if (type == "rectangle") loadedShape = new Rectangle(QRect());
			else if (type == "circle") loadedShape = new Circle(QRect());
			else if (type == "triangle") loadedShape = new Triangle(QRect());
			else if (type == "diamond") loadedShape = new Diamond(QRect());
			else if (type == "rounded_rectangle") loadedShape = new RoundedRectangle(QRect());
			else if (type == "parallelogram") loadedShape = new Parallelogram(QRect());
			else if (type == "textbox") loadedShape = new TextBox(QRect());
			else { /* 警告由 loadFromJson 处理? */ }

			if (loadedShape) {
				loadedShape->loadFromJson(shapeObj); // 加载属性

				// *** 关键: 分配一个新的 ID ***
				QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
				loadedShape->setId(newId);

				// 应用粘贴偏移量
				QRect originalRect = loadedShape->getRect();
				loadedShape->setRect(originalRect.translated(pasteOffset.toPoint()));

				newShapes.append(loadedShape); // 添加到新图形列表
				if (!oldId.isEmpty()) {
					oldIdToNewShapeMap[oldId] = loadedShape; // 存储映射关系
				}
			}
		}
	}

	// --- 2. 反序列化并创建新线条 ---
	if (rootObj.contains("lines") && rootObj["lines"].isArray()) {
		QJsonArray linesArray = rootObj["lines"].toArray();
		for (const QJsonValue& value : linesArray) {
			if (!value.isObject()) continue;
			QJsonObject lineObj = value.toObject();

			// 使用修改后的 fromJson，它会利用 ID 映射
			ConnectionLine* loadedLine = ConnectionLine::fromJson(lineObj, oldIdToNewShapeMap);

			if (loadedLine) {
				// *** 关键: 分配一个新的 ID ***
				QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
				// 假设 ConnectionLine 有 setId 方法
				// 如果没有，你需要在 ConnectionLine 中添加 setId(const QString& id)
				// 或者在构造函数中处理ID（但不推荐用于粘贴场景）
				loadedLine->setId(newId);

				// 对自由端应用粘贴偏移量
				if (!loadedLine->isStartAttached()) {
					loadedLine->setFreeStartPoint(loadedLine->getStartPointPos() + pasteOffset);
				}
				if (!loadedLine->isEndAttached()) {
					loadedLine->setFreeEndPoint(loadedLine->getEndPointPos() + pasteOffset);
				}
				newLines.append(loadedLine); // 添加到新线条列表
			}
		}
	}

	// --- 3. 创建并推送 Paste 命令 ---
	if (!newShapes.isEmpty() || !newLines.isEmpty()) {
		if (undoStack) {
			// 创建粘贴命令，传入 this (DrawingArea), 新图形列表, 新线条列表
			undoStack->push(new PasteCommand(this, newShapes, newLines));
			// 命令的 redo() 将处理添加到列表、选择和更新的操作。
		}
		else {
			qWarning() << QObject::tr("Undo stack is null. Cannot perform paste with undo support.");
			// 后备方案：直接添加（但失去撤销功能）- 通常不推荐
			// for (Shape* s : newShapes) shapes.append(s);
			// for (ConnectionLine* l : newLines) allLines.append(l);
			// update();
			// 如果不使用命令，需要清理内存
			// qDeleteAll(newShapes);
			// qDeleteAll(newLines);
		}
	}
	else {
		// 清理已创建但未使用的对象的内存
		qDeleteAll(newShapes);
		qDeleteAll(newLines);
	}
}

// 实现 alignSelectedShapes 方法：
void DrawingArea::alignSelectedShapes(Qt::Alignment alignment) {
	// 对齐操作至少需要选中两个图形
	if (selectedShapes.size() < 2) {
		QMessageBox::information(this, tr("Tip"), tr("Please select at least two shapes to align."));
		return;
	}

	if (!undoStack) {
		return;
	}

	// 创建并推送对齐命令
	undoStack->push(new AlignShapesCommand(this, selectedShapes, alignment));
	// 命令的 redo 会自动执行，并更新 UI 和属性面板
}

// 实现 deleteSelection 方法：
void DrawingArea::deleteSelection() {
	// 检查是否有选中的项目以及是否有撤销栈
	if ((selectedShapes.isEmpty() && selectedLines.isEmpty()) || !undoStack) {
		return;
	}
	QList<Shape*> shapesToDelete = selectedShapes;
	QList<ConnectionLine*> linesToDelete = selectedLines;

	// 推送删除命令
	undoStack->push(new RemoveItemsCommand(this, shapesToDelete, linesToDelete));

}

// resetActionState (确保清理新状态)
void DrawingArea::resetActionState() {
	// --- 新增：如果正在编辑文本，先结束编辑 ---
	if (m_isEditingText) {
		onTextEditFinished(); // 尝试提交编辑并清理
	}
	// --- 结束新增 --
	isDrawing = false;           // 绘制图形或自由线
	isResizing = false;          // 缩放选中项
	isSelecting = false;         // 框选
	isMovingSelection = false;   // 移动选中项
	isCreatingConnection = false; // 从图形开始连接
	isStretchingLine = false;    // 拉伸线的自由端点
	delete tempShape; 
	tempShape = nullptr;
	delete tempLine; 
	tempLine = nullptr; // 重置临时线

	connectionStartShape = nullptr;      // 清理连接起点信息
	connectionStartPointIndex = -1;

	lineBeingStretched = nullptr;       // 清理被拉伸的线
	stretchingStartHandle = false;      // 清理拉伸句柄状态

	currentHandle = -1;                 // 清理缩放句柄索引

	// 清理缩放缓存
	shapeOriginalRelativeRects.clear();
	lineOriginalRelativePoints.clear(); // 清空旧缓存

	// 通常不重置悬停状态，由 mouseMove 更新
	// 选择状态通常也不在此处重置，除非特定操作（如右键）

	// 恢复默认光标，除非特定工具模式需要不同光标
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
			updateTextEditorGeometry(); // <--- 调用更新
			
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
	Q_UNUSED(event); // 标记 event 未使用，避免编译器警告
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing); // 开启抗锯齿
	painter.fillRect(rect(), Qt::white); // 用白色填充背景

	// --- 应用全局缩放 ---
	painter.save(); // 保存当前的绘制状态 (变换矩阵, 画笔, 画刷等)
	painter.scale(m_scaleFactor, m_scaleFactor); // 应用当前的缩放因子

	// --- 在缩放后的坐标系下绘制 ---

	// 1. 绘制网格 (如果启用，并补偿笔宽)
	if (isGridMode) {
		qreal gridPenWidth = 1.0 / m_scaleFactor; // 计算笔宽，使视觉上接近 1px
		painter.setPen(QPen(gridColor, gridPenWidth)); // 设置网格线画笔

		// 计算需要绘制网格的范围 (基于当前视口在缩放坐标系下的区域)
		QRectF visibleRect = painter.transform().inverted().mapRect(rect());
		int startX = qFloor(visibleRect.left() / gridSize) * gridSize;
		int startY = qFloor(visibleRect.top() / gridSize) * gridSize;
		int endX = qCeil(visibleRect.right());
		int endY = qCeil(visibleRect.bottom());

		// 绘制水平线
		for (int y = startY; y < endY; y += gridSize) {
			painter.drawLine(QPointF(visibleRect.left(), y), QPointF(visibleRect.right(), y));
		}
		// 绘制垂直线
		for (int x = startX; x < endX; x += gridSize) {
			painter.drawLine(QPointF(x, visibleRect.top()), QPointF(x, visibleRect.bottom()));
		}
	}

	// 2. 绘制所有线 (直接调用 draw，不修改 Line 类，线条视觉粗细会变化)
	for (ConnectionLine* line : allLines) {
		if (line) {
			line->draw(painter); // 使用 ConnectionLine 自己的绘制方法
		}
	}

	// 3. 绘制所有图形 (直接调用 draw，不修改 Shape 类，边框和文字视觉大小会变化)
	for (Shape* shape : shapes) {
		if (shape) {
			// 特殊处理正在编辑的 TextBox：只让它绘制边框
			if (m_isEditingText && shape == m_editingTextBox) {
				painter.save(); // 保存画笔状态
				// 假设 Shape::draw 能处理边框绘制，这里不需要手动绘制
				// 如果 Shape::draw 不画边框，则需要在这里画
				// painter.setPen(QPen(shape->getBorderColor(), shape->getBorderWidth())); // 使用原始宽度
				// painter.setBrush(Qt::NoBrush);
				// painter.drawRect(shape->getRect()); // 绘制矩形边框示例
				shape->draw(painter); // 让 TextBox 自己绘制（可能只画边框，或内部处理）
				painter.restore(); // 恢复画笔状态
			}
			else {
				// 正常绘制其他图形或非编辑状态的 TextBox
				shape->draw(painter); // 使用 Shape 子类自己的绘制方法
			}
		}
	}

	// ******** 新增：绘制对齐参考线 ********
	if (!m_alignmentGuides.isEmpty()) {
		painter.save();
		QPen guidePen;
		guidePen.setColor(QColor(0, 180, 0, 200)); // 绿色，稍微透明
		guidePen.setStyle(Qt::DashLine);          // 虚线
		guidePen.setWidthF(2.0 / m_scaleFactor);  // 补偿缩放，保持视觉宽度接近 1px
		guidePen.setCosmetic(true);              // 尝试让线宽更稳定
		painter.setPen(guidePen);
		for (const QLineF& guide : m_alignmentGuides) {
			painter.drawLine(guide);
		}
		painter.restore();
	}
	// ******** 结束新增 ********


	// 4. 绘制临时图形 (视觉效果同上)
	if (isDrawing && tempShape && currentToolMode == ShapeDrawingTool) {
		tempShape->draw(painter); // 使用临时图形自己的绘制方法
	}

	// 5. 绘制临时线 (视觉效果同上)
	if (tempLine && currentToolMode == ConnectionTool && (isDrawing || isCreatingConnection)) {
		tempLine->draw(painter); // 使用临时线自己的绘制方法
	}

	// 6. 绘制选择框和缩放控制点 (进行补偿以保持视觉大小/粗细)
	if (currentToolMode == SelectionTool && showControlPoints && (!selectedShapes.isEmpty() || !selectedLines.isEmpty()))
	{
		// 计算所有选中元素的总边界框
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

		// 仅在边界有效时绘制
		if (!totalSelectedBounds.isNull() && totalSelectedBounds.isValid()) {
			bool canResize = true; // 默认可以缩放
			// 特例：如果只选中一个部分连接的线，则不能整体缩放
			if (selectedShapes.isEmpty() && selectedLines.size() == 1 && selectedLines.first()->isPartiallyAttached()) {
				canResize = false;
			}

			if (canResize) {
				painter.save(); // 保存状态，以便局部设置画笔

				// a. 绘制虚线外框 (补偿笔宽和边距)
				qreal selBoxPenWidth = 1.0 / m_scaleFactor; // 目标视觉宽度 1px
				qreal margin = 2.0 / m_scaleFactor;        // 目标视觉边距 2px
				painter.setPen(QPen(Qt::darkGray, selBoxPenWidth, Qt::DashLine));
				painter.setBrush(Qt::NoBrush); // 不填充
				painter.drawRect(totalSelectedBounds.adjusted(-margin, -margin, margin, margin)); // 绘制带边距的外框

				// b. 绘制缩放控制手柄 (补偿笔宽和绘制大小)
				qreal handlePenWidth = 1.0 / m_scaleFactor;         // 目标视觉边框宽度 1px
				qreal handleVisualSize = ConnectionLine::handleSize; // 手柄的 *目标视觉* 尺寸 (来自 ConnectionLine 定义)
				qreal handleDrawSize = handleVisualSize / m_scaleFactor; // 计算在当前缩放下需要绘制的尺寸

				painter.setPen(QPen(Qt::black, handlePenWidth)); // 设置手柄边框画笔
				painter.setBrush(Qt::white);                   // 设置手柄填充颜色

				// 计算 8 个手柄的位置
				QPointF handles[8];
				handles[0] = totalSelectedBounds.topLeft();
				handles[1] = QPointF(totalSelectedBounds.center().x(), totalSelectedBounds.top());
				handles[2] = totalSelectedBounds.topRight();
				handles[3] = QPointF(totalSelectedBounds.left(), totalSelectedBounds.center().y());
				handles[4] = QPointF(totalSelectedBounds.right(), totalSelectedBounds.center().y());
				handles[5] = totalSelectedBounds.bottomLeft();
				handles[6] = QPointF(totalSelectedBounds.center().x(), totalSelectedBounds.bottom());
				handles[7] = totalSelectedBounds.bottomRight();

				// 绘制 8 个手柄矩形
				for (int i = 0; i < 8; ++i) {
					painter.drawRect(QRectF(handles[i].x() - handleDrawSize / 2.0, handles[i].y() - handleDrawSize / 2.0, handleDrawSize, handleDrawSize));
				}
				painter.restore(); // 恢复画笔状态
			}
			// 注意：ConnectionLine 内部绘制其自由端点手柄时，因为我们不修改它的 draw 方法，
			// 所以那些手柄的视觉大小会随着缩放变化。
		}
	}


	// 7. 绘制连接点高亮 (补偿笔宽和半径)
	qreal pointPenWidth = 1.0 / m_scaleFactor;           // 目标视觉笔宽 1px
	qreal normalVisualRadius = 4.0;                    // 普通连接点的目标视觉半径
	qreal highlightVisualRadius = 5.0;                 // 高亮连接点的目标视觉半径
	qreal normalDrawRadius = normalVisualRadius / m_scaleFactor; // 计算普通点需要绘制的半径
	qreal highlightDrawRadius = highlightVisualRadius / m_scaleFactor; // 计算高亮电需要绘制的半径

	// 7a. 绘制正在连接的 *起始* 图形的连接点
	if (currentToolMode == ConnectionTool && isCreatingConnection && connectionStartShape != nullptr && connectionStartPointIndex != -1)
	{
		painter.save(); // 保存状态
		int numPoints = connectionStartShape->getConnectionPointCount();
		for (int i = 0; i < numPoints; ++i) {
			QPoint hp = connectionStartShape->getConnectionPoint(i); // 获取连接点 (QPoint)
			if (i == connectionStartPointIndex) { // 高亮选中的起始点
				painter.setBrush(Qt::yellow);
				painter.setPen(QPen(Qt::darkYellow, pointPenWidth));
				painter.drawEllipse(QPointF(hp), highlightDrawRadius, highlightDrawRadius); // 使用 QPointF 重载
			}
			else { // 绘制其他普通点
				painter.setBrush(Qt::NoBrush);
				painter.setPen(QPen(Qt::gray, pointPenWidth));
				painter.drawEllipse(QPointF(hp), normalDrawRadius, normalDrawRadius); // 使用 QPointF 重载
			}
		}
		painter.restore(); // 恢复状态
	}
	// 7b. 绘制 *悬停目标* 图形的连接点 (当连接或拉伸时)
	bool shouldDrawTargetPoints = hoveredShape != nullptr &&
		((currentToolMode == ConnectionTool) ||
			(currentToolMode == SelectionTool && isStretchingLine));
	if (shouldDrawTargetPoints)
	{
		painter.save(); // 保存状态
		int numPoints = hoveredShape->getConnectionPointCount();
		for (int i = 0; i < numPoints; ++i) {
			QPoint hp = hoveredShape->getConnectionPoint(i); // 获取连接点 (QPoint)
			if (i == hoveredConnectionPointIndex) { // 高亮悬停/吸附的目标点
				painter.setBrush(Qt::yellow);
				painter.setPen(QPen(Qt::darkYellow, pointPenWidth));
				painter.drawEllipse(QPointF(hp), highlightDrawRadius, highlightDrawRadius); // 使用 QPointF 重载
			}
			else { // 绘制其他普通点
				painter.setBrush(Qt::NoBrush);
				painter.setPen(QPen(Qt::gray, pointPenWidth));
				painter.drawEllipse(QPointF(hp), normalDrawRadius, normalDrawRadius); // 使用 QPointF 重载
			}
		}
		painter.restore(); // 恢复状态
	}


	// 8. 绘制框选矩形 (补偿笔宽)
	if (isSelecting && currentToolMode == SelectionTool) {
		QRectF selectionRect(selectionStart, selectionEnd); // 框选范围
		painter.save(); // 保存状态
		qreal selRectPenWidth = 1.0 / m_scaleFactor; // 目标视觉笔宽 1px
		painter.setPen(QPen(Qt::blue, selRectPenWidth, Qt::DashLine)); // 设置画笔
		painter.setBrush(QColor(0, 100, 255, 30)); // 设置半透明填充
		painter.drawRect(selectionRect.normalized()); // 绘制矩形
		painter.restore(); // 恢复状态
	}

	// --- 恢复缩放前的状态 ---
	painter.restore(); // 恢复调用 painter.save() 时保存的状态 (变换矩阵等)

	// --- 在未缩放的坐标系下绘制 ---
	// 这里可以绘制不希望被缩放的元素，例如调试信息或固定的UI覆盖层
	// 例如，显示当前的缩放比例:
	// painter.setPen(Qt::red);
	// painter.drawText(10, 20, QString("缩放: %1%").arg(m_scaleFactor * 100, 0, 'f', 0));

	// 注意: QLineEdit 文本编辑器由 Qt 的窗口系统绘制，它不会受 painter.scale() 影响。
	// 它的位置和大小是在 mouseDoubleClickEvent/mouseMoveEvent 中基于 TextBox 未缩放的 rect 设置的。
	// 在非 100% 缩放时，编辑器和背景 TextBox 视觉上可能不完全重合。
}


// --- mousePressEvent (修改 ConnectionTool 和 SelectionTool 逻辑) ---
void DrawingArea::mousePressEvent(QMouseEvent* event) {
	QPointF viewportPos = event->localPos(); // 获取视口坐标
	QPointF pos = mapToScene(viewportPos); // 获取场景逻辑坐标
	qDebug() << "Mouse Press - Viewport:" << viewportPos << "Scene:" << pos;
	qDebug() << "Mouse Press - Viewport:" << viewportPos << "Scene:" << pos;
	qDebug() << "Mouse Press - Viewport:" << viewportPos << "Scene:" << pos.toPoint();

	// --- 修改: lastMousePos 存储场景坐标 ---
	lastMousePos = pos; // <--- 记录本次按下的【场景坐标】
	// --- 新增：如果正在编辑文本，则忽略鼠标按下事件 (除了在编辑器内部) ---
	if (m_isEditingText) {
		// 检查点击是否在编辑器控件外部
		if (m_textEditor && !m_textEditor->geometry().contains(pos.toPoint())) {
			// 点击外部，结束编辑
			onTextEditFinished();
			
		}
		else {
			
			return;
		}
		
	}
	// --- 结束新增 ---
	bool actionTaken = false; // 本次事件是否执行了具体操作
	setFocus(); // 获取焦点

	// --- 处理左键按下 ---
	if (event->button() == Qt::LeftButton) {
		switch (currentToolMode) {
			// --- 绘图工具 (不变) ---
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

			// --- 连接工具 (修改) ---
		case ConnectionTool: {
			resetActionState(); // 重置状态
			selectedShapes.clear(); // 清除选择
			selectedLines.clear();
			updateSelectionStates();
			showControlPoints = false;

			Shape* startShape = nullptr;
			int startIndex = -1;
			
			if (findClosestPointOnShape(pos, startShape, startIndex, 12.0)) {
			
				isCreatingConnection = true; // 标记为从图形开始
				connectionStartShape = startShape;
				connectionStartPointIndex = startIndex;
				delete tempLine; // 清理可能存在的旧临时线
				
				tempLine = new ConnectionLine(startShape->getConnectionPoint(startIndex), pos, QColor(0, 0, 255, 150));
				tempLine->attachStart(startShape, startIndex); // 将起点连接到图形
				tempLine->setSelected(true); // 高亮显示
				actionTaken = true;
				
			}
			else {
				
				isDrawing = true; // 标记为绘制自由线
				startPoint = pos; // 记录起点
				delete tempLine;
				tempLine = new ConnectionLine(startPoint, startPoint, QColor(0, 0, 255, 150));
				tempLine->setSelected(true);
				actionTaken = true;
				
			}
			setCursor(Qt::CrossCursor);
		} break;

			// --- 选择工具 (修改对线和手柄的处理) ---
		case SelectionTool: {
			// 重置本次操作的状态
			isResizing = false;
			isStretchingLine = false;
			isMovingSelection = false;
			isSelecting = false;
			currentHandle = -1;
			lineBeingStretched = nullptr;
			actionTaken = false;

			bool selectionExists = !selectedShapes.isEmpty() || !selectedLines.isEmpty();
			QRectF totalSelectedBounds; // 总边界框


			// --- 按优先级顺序检查点击位置 ---

			// --- 1. 检查是否点击了缩放手柄 (仅当有多选或选中图形/完全自由线时) ---
			bool canResize = false; // 是否可以显示和使用缩放手柄
			if (selectionExists) {
				// 只有在选中多个元素，或只选中图形，或只选中完全自由线时才计算缩放框
				if (selectedShapes.size() > 1 || selectedLines.size() > 1 ||
					(selectedShapes.size() == 1 && selectedLines.isEmpty()) ||
					(selectedLines.size() == 1 && selectedShapes.isEmpty() && selectedLines.first()->isFullyFree()))
				{
					canResize = true;
					// 计算总边界框 (代码与之前类似)
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
						if (line && line->isFullyFree()) { // 只考虑完全自由线的边界用于缩放框计算
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
					// 如果只选中一个部分连接线，则不进行缩放
					canResize = false;
				}
				else {
					// 其他单选情况（如图形+线）也可以考虑是否允许缩放
					// 为简化，这里也允许缩放
					canResize = true;
					// 重新计算边界，包括所有类型
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
					//... (计算 handles 的代码不变)...
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
							originalResizeBounds = totalSelectedBounds; // 记录原始边界
							actionTaken = true;
							setCursorBasedOnHandle(i);

							// --- 清空并填充缩放缓存 (修改：包含自由点信息) ---
							shapeOriginalRelativeRects.clear();
							lineOriginalRelativePoints.clear(); // 使用这个缓存存储自由端点的相对位置
							if (originalResizeBounds.width() > 1e-6 && originalResizeBounds.height() > 1e-6) {
								// 缓存选中图形的相对矩形 (不变)
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
								// 缓存选中线的 *自由* 端点的相对坐标
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
									// 只有当至少一个端点是自由时才缓存
									if (startIsFree || endIsFree) {
										lineOriginalRelativePoints[line] = relPoints;
									}
								}
								
							}
							else {
								
								isResizing = false;
								actionTaken = false;
							}
							break; // 找到手柄，跳出循环
						} // end if isPointNear
					} // end for handles
				} // end if canResize and totalSelectedBounds valid
			} // 结束检查缩放手柄

			/// +++ 2. 检查是否点击选中线的 *已连接* 端点以开始分离 +++
			ConnectionLine* lineToDetach = nullptr;
			int attachedEndIndex = -1;
			if (!actionTaken && !selectedLines.isEmpty()) { // 只有选中线才检查
				attachedEndIndex = findAttachedEndNearPoint(pos, lineToDetach);
				if (attachedEndIndex != -1 && lineToDetach) {
					isStretchingLine = true; // 复用状态
					lineBeingStretched = lineToDetach;
					stretchingStartHandle = (attachedEndIndex == 0);
					// --- 立即分离 ---
					if (stretchingStartHandle) {
						lineBeingStretched->detachStart();
						lineBeingStretched->setFreeStartPoint(pos); // 移动到鼠标位置
					}
					else {
						lineBeingStretched->detachEnd();
						lineBeingStretched->setFreeEndPoint(pos); // 移动到鼠标位置
					}
					actionTaken = true;
					setCursor(Qt::ClosedHandCursor); // 设置拖拽光标
					
				}
			} // +++ 结束检查已连接端点 +++

		   // --- 2. 检查是否点击了线的自由端点手柄 (仅当有选中线且未进入缩放) ---
			if (selectionExists && !actionTaken) {
				for (ConnectionLine* line : selectedLines) { // 只检查选中的线
					if (line && line->isSelected()) { // 双重检查选中状态
						int handleIdx = findHandleForFreeEnd(pos, line); // 使用新函数
						if (handleIdx != -1) { // 0 for start handle, 1 for end handle
							isStretchingLine = true;
							lineBeingStretched = line;
							stretchingStartHandle = (handleIdx == 0); // true if start handle, false if end handle
							actionTaken = true;
							setCursor(Qt::CrossCursor); // 拉伸时用十字光标
							
							break; // 找到一个可拉伸的手柄就够了
						}
					}
				}
			} // 结束检查自由端点拉伸手柄

			// --- 3. 如果未操作手柄，检查是否点击了图形或线本身 ---
			if (!actionTaken) {
				Shape* topShape = findTopShapeAt(pos);
				ConnectionLine* topLine = findTopLineAt(pos);

				Shape* clickedShapePtr = nullptr;
				ConnectionLine* clickedLinePtr = nullptr;
				bool elementIsSelected = false;

				// 优先判断图形
				if (topLine) { // 没有点中图形，再判断线
					clickedLinePtr = topLine;
					elementIsSelected = selectedLines.contains(topLine);
				}
				else if (topShape) {
					clickedShapePtr = topShape;
					elementIsSelected = selectedShapes.contains(topShape);
				}
			

				bool clickedOnElement = (clickedShapePtr != nullptr || clickedLinePtr != nullptr);
				bool ctrl = event->modifiers() & Qt::ControlModifier;

				if (clickedOnElement) { // --- 点击在了某个元素上 ---
					if (!ctrl) { // --- 单击 (无 Ctrl) ---
						if (!elementIsSelected) { // 单击了 *未选中* 的元素
							selectedShapes.clear();
							selectedLines.clear();
							if (clickedShapePtr) {
								selectedShapes.append(clickedShapePtr);
							}
							else {
								selectedLines.append(clickedLinePtr);
							}
							updateSelectionStates();
							showControlPoints = true; // 显示控制点
							isMovingSelection = true; // 选中后立即准备移动
							actionTaken = true;
							setCursor(Qt::SizeAllCursor);
							
						}
						else { 
							isMovingSelection = true;
							actionTaken = true;
							setCursor(Qt::SizeAllCursor);
							
						}

					}
					else { // --- Ctrl+单击 ---
						if (elementIsSelected) { // Ctrl+单击 *已选中* -> 取消选中该项
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
						else { // Ctrl+单击 *未选中* -> 添加到选择集
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
				} // 结束 if (clickedOnElement)

				// --- 4. 如果未点击元素，检查是否在选中区域空白处 ---
				
				else if (selectionExists && !actionTaken) {
					totalSelectedBounds = QRectF(); // 清空并重新开始计算
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

				if (isMovingSelection) { // 当确定要开始移动时
					
					setCursor(Qt::SizeAllCursor);
					selectionStart = pos; 

					// *** 记录移动前状态 ***
					m_originalShapeRectsOnMove.clear();
					m_originalLinePointsOnMove.clear();
					for (Shape* shape : selectedShapes) {
						if (shape) {
							m_originalShapeRectsOnMove[shape] = QRectF(shape->getRect());
						}
					}
					for (ConnectionLine* line : selectedLines) {
						if (line) {
							// 只记录自由端点，因为连接端点由 Shape 决定
							QPointF startP = line->isStartAttached() ? QPointF() : line->getStartPointPos();
							QPointF endP = line->isEndAttached() ? QPointF() : line->getEndPointPos();
							m_originalLinePointsOnMove[line] = { startP, endP };
						}
					}
					actionTaken = true;
				}


				// --- 5. 如果以上均不满足，则点击了画布的真正空白处 ---
				if (!actionTaken) {
					if (!ctrl) { // 无 Ctrl -> 清除选择并开始框选
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
						setCursor(Qt::ArrowCursor); // 框选时用箭头
						
					}
					else { // Ctrl+单击空白，无操作
						actionTaken = true; // 阻止后续操作，但本身无效果
						
					}
				}

			} // 结束步骤 3, 4, 5

		} break; // 结束 case SelectionTool
		} // 结束 switch(currentToolMode)

		if (actionTaken) {
			update(); // 如果执行了操作，请求重绘
		}

	} // 结束 if (event->button() == Qt::LeftButton)

	
} // 结束 mousePressEvent


// --- 辅助函数：设置光标 (不变) ---
void DrawingArea::setCursorBasedOnHandle(int handleIndex) {
	switch (handleIndex) {
	case 0: case 7: setCursor(Qt::SizeFDiagCursor); break;
	case 2: case 5: setCursor(Qt::SizeBDiagCursor); break;
	case 1: case 6: setCursor(Qt::SizeVerCursor); break;
	case 3: case 4: setCursor(Qt::SizeHorCursor); break;
	default:        setCursor(Qt::ArrowCursor); break;
	}
}

// --- mouseMoveEvent (修改 ConnectionTool, SelectionTool-Resize, SelectionTool-Stretch 逻辑) ---
void DrawingArea::mouseMoveEvent(QMouseEvent* event)
{
	if (m_isEditingText) {
		
		return; 
	}
	
	
	QPointF viewportPos = event->localPos();
	QPointF pos = mapToScene(viewportPos);
	QPointF offset = pos - lastMousePos; // 计算自上次移动以来的位移
	
	updateHoveredShapeAndPoint(pos);

	// --- 仅当鼠标左键按下时处理拖动 ---
	if (event->buttons() & Qt::LeftButton)
	{
		// --- A. 连接工具模式 ---
		if (currentToolMode == ConnectionTool)
		{
			if ((isCreatingConnection || isDrawing) && tempLine)
			{
				
				if (isCreatingConnection)
				{
					// 从图形开始，更新终点
					tempLine->setFreeEndPoint(pos);
				}
				else
				{
					// 从空白开始，更新终点 (起点已固定)
					tempLine->setFreeEndPoint(pos);
				}
				// 检查画布扩展 (基于临时线的包围盒)
				expandCanvasIfNeeded(tempLine->getBoundingRect().toAlignedRect());
				// 请求重绘临时线
				update();
			}
		}
		// --- B. 图形绘制工具模式 (不变) ---
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
			tempShape = DiagramEditor::getShape(currentShapeType, rect); // 使用工厂
			if (tempShape)
			{
				expandCanvasIfNeeded(tempShape->getRect());
			}
			update();
		}
		// --- C. 选择工具模式 ---
		else if (currentToolMode == SelectionTool)
		{
			// --- C.1 处理缩放 (isResizing, 修改线的处理) ---
			if (isResizing && currentHandle != -1)
			{
				// 安全检查 (不变)
				if (originalResizeBounds.isNull()
					|| !originalResizeBounds.isValid()
					|| originalResizeBounds.width() <= 1e-6
					|| originalResizeBounds.height() <= 1e-6
					|| (shapeOriginalRelativeRects.isEmpty() && lineOriginalRelativePoints.isEmpty() && selectedLines.isEmpty()))
				{
					// 如果没有缓存且没有选中线，也退出
					
					resetActionState();
					update();
					return;
				}

				// 计算新的总边界框 newBounds (根据拖动的手柄，不变)
				QRectF newBounds = originalResizeBounds;
				qreal minAllowedSize = 5.0;

				//... (switch(currentHandle) 来更新 newBounds 的代码不变) ...
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
				newBounds = newBounds.normalized(); // 确保 topLeft <= bottomRight

				// --- 应用缩放 (修改部分：处理线的自由端点) ---
				// 应用到图形 (不变)
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

				// 应用到线的 *自由* 端点 (使用 lineOriginalRelativePoints 缓存)
				for (auto it = lineOriginalRelativePoints.constBegin(); it != lineOriginalRelativePoints.constEnd(); ++it)
				{
					ConnectionLine* line = it.key();
					QPair<QPointF, QPointF> relPoints = it.value(); // {relStart, relEnd}
					if (!line)
					{
						continue;
					}

					// 检查起点是否自由并更新
					if (!line->isStartAttached() && !relPoints.first.isNull())
					{
						// 检查缓存是否有效
						QPointF relP1 = relPoints.first;
						qreal newP1X = newBounds.x() + relP1.x() * newBounds.width();
						qreal newP1Y = newBounds.y() + relP1.y() * newBounds.height();
						line->setFreeStartPoint(QPointF(newP1X, newP1Y));
					}
					// 检查终点是否自由并更新
					if (!line->isEndAttached() && !relPoints.second.isNull())
					{
						// 检查缓存是否有效
						QPointF relP2 = relPoints.second;
						qreal newP2X = newBounds.x() + relP2.x() * newBounds.width();
						qreal newP2Y = newBounds.y() + relP2.y() * newBounds.height();
						line->setFreeEndPoint(QPointF(newP2X, newP2Y));
					}
				}
				
				if (m_isEditingText && m_editingTextBox && selectedShapes.contains(m_editingTextBox)) {
					if (m_textEditor) {
						// 使用更新后的 TextBox 矩形来设置编辑器的几何形状
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
			} // 结束 if (isResizing)

			// --- C.2 处理自由端点拉伸 (isStretchingLine) ---
			else if (isStretchingLine && lineBeingStretched)
			{
				// 根据 stretchingStartHandle 判断是拉伸起点还是终点
				if (stretchingStartHandle)
				{
					// 拉伸起点
					lineBeingStretched->setFreeStartPoint(pos);
				}
				else
				{
					// 拉伸终点
					lineBeingStretched->setFreeEndPoint(pos);
				}
				// 扩展画布并更新
				expandCanvasIfNeeded(lineBeingStretched->getBoundingRect().toAlignedRect());
				update();
			}
			// --- C.3 处理移动选中项 (isMovingSelection) ---
			else if (isMovingSelection && offset.manhattanLength() > 0.1)
			{
			// ********对齐和吸附逻辑开始 * *******
			m_alignmentGuides.clear();          // 清除上一帧的参考线
			m_currentSnapOffset = QPointF(0, 0); // 重置当前帧的吸附偏移
			bool snappedX = false;              // 标记X轴是否已吸附
			bool snappedY = false;              // 标记Y轴是否已吸附

			// 1. 获取所有【非选中】图形的边界矩形
			QList<QRectF> staticRects;
			for (Shape* s : shapes) {
				if (s && !selectedShapes.contains(s)) {
					staticRects.append(QRectF(s->getRect()));
				}
			}

			// 如果没有静态图形，则无需对齐
			if (!staticRects.isEmpty()) {

				// 2. 遍历每个【选中】的图形，计算预测位置并检查对齐
				qreal closestSnapDistX = SNAP_THRESHOLD; // 记录当前找到的最近X吸附距离
				qreal closestSnapDistY = SNAP_THRESHOLD; // 记录当前找到的最近Y吸附距离
				QLineF bestGuideX; // 最佳垂直参考线
				QLineF bestGuideY; // 最佳水平参考线

				for (Shape* movingShape : selectedShapes) {
					if (!movingShape) continue;

					QRectF originalMovingRect = QRectF(movingShape->getRect());
					// 计算没有吸附时的预测位置
					QRectF predictedMovingRect = originalMovingRect.translated(offset);

					// 定义移动图形的关键X, Y坐标
					qreal movingLeft = predictedMovingRect.left();
					qreal movingCenterX = predictedMovingRect.center().x();
					qreal movingRight = predictedMovingRect.right();
					qreal movingTop = predictedMovingRect.top();
					qreal movingCenterY = predictedMovingRect.center().y();
					qreal movingBottom = predictedMovingRect.bottom();

					// 与每个静态图形进行比较
					for (const QRectF& staticRect : staticRects) {
						// 定义静态图形的关键X, Y坐标
						qreal staticLeft = staticRect.left();
						qreal staticCenterX = staticRect.center().x();
						qreal staticRight = staticRect.right();
						qreal staticTop = staticRect.top();
						qreal staticCenterY = staticRect.center().y();
						qreal staticBottom = staticRect.bottom();

						// --- 检查 X 轴对齐 ---
						qreal diffsX[] = {
							movingLeft - staticLeft, movingLeft - staticCenterX, movingLeft - staticRight,
							movingCenterX - staticLeft, movingCenterX - staticCenterX, movingCenterX - staticRight,
							movingRight - staticLeft, movingRight - staticCenterX, movingRight - staticRight
						};
						qreal targetsX[] = { // 对应的静态目标X坐标
							staticLeft, staticCenterX, staticRight,
							staticLeft, staticCenterX, staticRight,
							staticLeft, staticCenterX, staticRight
						};

						for (int i = 0; i < 9; ++i) {
							if (qAbs(diffsX[i]) < closestSnapDistX) {
								closestSnapDistX = qAbs(diffsX[i]);
								m_currentSnapOffset.setX(-diffsX[i]); // X轴吸附偏移
								snappedX = true;
								// 计算垂直参考线 (Y 范围需要更智能地计算)
								qreal guideY1 = qMin(predictedMovingRect.top(), staticRect.top()) - 20;
								qreal guideY2 = qMax(predictedMovingRect.bottom(), staticRect.bottom()) + 20;
								bestGuideX = QLineF(targetsX[i], guideY1, targetsX[i], guideY2);
							}
						}

						// --- 检查 Y 轴对齐 ---
						qreal diffsY[] = {
						   movingTop - staticTop, movingTop - staticCenterY, movingTop - staticBottom,
						   movingCenterY - staticTop, movingCenterY - staticCenterY, movingCenterY - staticBottom,
						   movingBottom - staticTop, movingBottom - staticCenterY, movingBottom - staticBottom
						};
						qreal targetsY[] = { // 对应的静态目标Y坐标
						   staticTop, staticCenterY, staticBottom,
						   staticTop, staticCenterY, staticBottom,
						   staticTop, staticCenterY, staticBottom
						};
						for (int i = 0; i < 9; ++i) {
							if (qAbs(diffsY[i]) < closestSnapDistY) {
								closestSnapDistY = qAbs(diffsY[i]);
								m_currentSnapOffset.setY(-diffsY[i]); // Y轴吸附偏移
								snappedY = true;
								// 计算水平参考线 (X 范围需要更智能地计算)
								qreal guideX1 = qMin(predictedMovingRect.left(), staticRect.left()) - 20;
								qreal guideX2 = qMax(predictedMovingRect.right(), staticRect.right()) + 20;
								bestGuideY = QLineF(guideX1, targetsY[i], guideX2, targetsY[i]);
							}
						}
					} // end for staticRects
				} // end for movingShapes

				// 如果找到了吸附，添加最佳参考线
				if (snappedX) m_alignmentGuides.append(bestGuideX);
				if (snappedY) m_alignmentGuides.append(bestGuideY);

				// 如果X和Y都没吸附，确保吸附偏移为0
				if (!snappedX) m_currentSnapOffset.setX(0);
				if (!snappedY) m_currentSnapOffset.setY(0);

			} // end if !staticRects.isEmpty()
			// ******** 对齐和吸附逻辑结束 ********
			// --- 逃逸检查 (可选) ---
				// 如果用户进行了较大的、明确的移动，可能不希望吸附发生
			const qreal ESCAPE_THRESHOLD_FACTOR = 1.5; // 比如原始移动超过阈值的1.5倍
			if (offset.manhattanLength() > SNAP_THRESHOLD * ESCAPE_THRESHOLD_FACTOR) {
				// 强制取消当前帧的吸附
				if (!m_currentSnapOffset.isNull()) {
					m_currentSnapOffset = QPointF(0, 0);
					m_alignmentGuides.clear(); // 同时清除参考线
				}
			}
			// --- 结束逃逸检查 ---
			// --- 应用最终偏移量 (原始偏移 + 吸附偏移) ---
			QRectF currentMovingBounds;
			for (Shape* movingShape : selectedShapes) {
				if (movingShape) currentMovingBounds = currentMovingBounds.united(QRectF(movingShape->getRect()).translated(offset));
			}
			for (ConnectionLine* movingLine : selectedLines) {
				if (movingLine) {
					QPointF p1 = movingLine->getStartPointPos();
					QPointF p2 = movingLine->getEndPointPos();
					if (!movingLine->isStartAttached()) p1 += offset; // 只用原始 offset
					if (!movingLine->isEndAttached()) p2 += offset; // 只用原始 offset
					currentMovingBounds = currentMovingBounds.united(QRectF(p1, p2).normalized());
				}
			}
			expandCanvasIfNeeded(currentMovingBounds.toAlignedRect());

			// 实际移动图形和线条时，只应用原始鼠标偏移量 offset
			for (Shape* shape : selectedShapes) {
				if (shape) {
					shape->move(offset.toPoint()); // <<<--- 只用 offset
				}
			}
			for (ConnectionLine* line : selectedLines) {
				if (line) {
					line->move(offset); // <<<--- 只用 offset
				}
			}
			// ... (处理文本编辑器移动不变) ...
			if (m_isEditingText && m_editingTextBox && selectedShapes.contains(m_editingTextBox)) {
				if (m_textEditor) {
					updateTextEditorGeometry();
				}
			}

			update(); // 请求重绘以显示移动和参考线
			}
			// --- C.4 处理框选 (isSelecting, 不变) ---
			else if (isSelecting)
			{
				selectionEnd = pos;
				m_alignmentGuides.clear(); // 框选时不显示参考线
				update();
			}
		} // 结束 if (currentToolMode == SelectionTool)

		lastMousePos = pos; // 更新上次鼠标位置，用于下次计算 offset
	} // 结束 if (event->buttons() & Qt::LeftButton)
	else
	{
		// 鼠标未按下时的处理 (更新光标和悬停效果)
		bool needsRepaint = false;
		Qt::CursorShape currentCursorShape = cursor().shape();
		Qt::CursorShape newCursorShape = cursor().shape(); // 默认保持不变

		// --- 根据模式设置光标 ---
		switch (currentToolMode)
		{
		case ConnectionTool:
			// 如果悬停在连接点上，显示手型，否则十字
			newCursorShape = (hoveredShape && hoveredConnectionPointIndex != -1) ? Qt::PointingHandCursor : Qt::CrossCursor;
			break;

		case ShapeDrawingTool:
			newCursorShape = Qt::CrossCursor;
			break;

		case SelectionTool:
		{
			newCursorShape = Qt::ArrowCursor; // 默认箭头
			bool cursorSet = false;

			// 检查悬停在缩放手柄 (仅当可缩放时)
			bool canResize = false;
			QRectF totalBounds; // 需要计算边界
			if (!selectedShapes.isEmpty() || !selectedLines.isEmpty())
			{
				// 复制 press 事件里的 canResize 判断逻辑
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
					// 其他情况允许缩放
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
				//... (计算 handles 的代码不变) ...
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
							//... 设置 newCursorShape...
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

			// 检查悬停在线的自由端点手柄
			if (!cursorSet && !selectedLines.isEmpty() && showControlPoints)
			{
				for (ConnectionLine* line : selectedLines)
				{
					if (line && line->isSelected())
					{
						// 检查自由端点
						if (findHandleForFreeEnd(pos, line) != -1)
						{
							newCursorShape = Qt::CrossCursor; // 悬停在自由手柄上用十字
							cursorSet = true;
							break;
						}
						// +++ 检查已连接(可分离)端点 +++
						ConnectionLine* dummyLine = nullptr;
						if (findAttachedEndNearPoint(pos, dummyLine) != -1 && dummyLine == line)
						{
							newCursorShape = Qt::PointingHandCursor; // 悬停在可分离端点上用手型
							cursorSet = true;
							break;
						}
						// +++ 结束检查 +++
					}
				}
			}

			// 检查悬停在元素体上
			if (!cursorSet)
			{
				Shape* hShape = findTopShapeAt(pos);
				ConnectionLine* hLine = findTopLineAt(pos);
				if (hShape || hLine)
				{
					bool itemIsSelected = (hShape && selectedShapes.contains(hShape)) || (hLine && selectedLines.contains(hLine));
					// 如果悬停在已选项上，显示移动光标，否则显示手型
					newCursorShape = itemIsSelected ? Qt::SizeAllCursor : Qt::PointingHandCursor;
					cursorSet = true;
				}
			}
			// 如果都没悬停，则保持默认箭头
		}
		break; // End SelectionTool Case
		} // End Switch

		// --- 统一设置光标 (仅当需要改变时) ---
		if (currentCursorShape != newCursorShape)
		{
			setCursor(newCursorShape);
		}
		// 确保在非移动状态下清除参考线
		if (!isMovingSelection && !m_alignmentGuides.isEmpty()) {
			m_alignmentGuides.clear();
			update(); // 清除后需要重绘
		}

	} // 结束鼠标未按下时的处理
} // 结束 mouseMoveEvent


// --- mouseReleaseEvent (修改 ConnectionTool, SelectionTool-Stretch, SelectionTool-Select 逻辑) ---
void DrawingArea::mouseReleaseEvent(QMouseEvent* event) {
	if (m_isEditingText) {
		// 可以选择性地更新光标，例如一直显示箭头
		// setCursor(Qt::ArrowCursor);
		return; // 不处理任何移动、拖拽、缩放等逻辑
	}
	if (event->button() == Qt::LeftButton) { // 只处理左键释放
		//QPointF pos = event->pos();
		QPointF viewportPos = event->localPos();
		QPointF pos = mapToScene(viewportPos);
		bool needsUpdate = false; // 是否需要调用 update()
		qreal dragDistance = (pos - lastMousePos).manhattanLength(); // 计算拖动距离 (近似)
		bool commandPushed = false; // 标记是否有命令被添加到堆栈

		// --- A. 连接工具模式 ---
		if (currentToolMode == ConnectionTool) {
			if (isCreatingConnection && tempLine) { // 完成从图形开始的连接尝试
				Shape* startShapeRef = tempLine->getStartShape(); // 获取起点信息
				int startIndexRef = tempLine->getStartPointIndex();
				QPointF endPos = tempLine->getEndPointPos(); // 获取终点位置

				Shape* endShape = nullptr;
				int endIndex = -1;
				// 查找释放位置是否有连接点 (排除起点图形)
				bool foundEnd = findClosestPointOnShape(pos, endShape, endIndex, 12.0) && endShape != startShapeRef;

				if (foundEnd && startShapeRef) { // 成功连接到另一个图形
					tempLine->attachEnd(endShape, endIndex); // 连接终点
					//allLines.append(tempLine); // 添加到正式列表
					//tempLine = nullptr; // 清除临时指针，所有权转移
					undoStack->push(new AddItemCommand(this, tempLine));
					tempLine = nullptr;
					
				}
				else if (startShapeRef && QLineF(tempLine->getStartPointPos(), endPos).length() > QApplication::startDragDistance()) {
					// 未连接到图形，但拖动距离足够 -> 创建部分连接线
					// tempLine 已经是起点连接，终点自由的状态
					/*allLines.append(tempLine);
					tempLine = nullptr;*/
					undoStack->push(new AddItemCommand(this, tempLine));
					tempLine = nullptr;
					

				}
				else { // 连接失败或距离太短
				
					delete tempLine;
					tempLine = nullptr; // 删除临时线
				}
				needsUpdate = true;
			}
			else if (isDrawing && tempLine) { // 完成绘制自由线尝试
				QPointF startPos = tempLine->getStartPointPos();
				QPointF endPos = tempLine->getEndPointPos();

				Shape* endShape = nullptr;
				int endIndex = -1;
				// 查找释放位置是否有连接点
				bool foundEnd = findClosestPointOnShape(pos, endShape, endIndex, 12.0);

				if (foundEnd && QLineF(startPos, endPos).length() > QApplication::startDragDistance()) { // 吸附到终点
					tempLine->attachEnd(endShape, endIndex);
					/*allLines.append(tempLine);
					tempLine = nullptr;*/
					undoStack->push(new AddItemCommand(this, tempLine));
					tempLine = nullptr;
					
				}
				else if (QLineF(startPos, endPos).length() > QApplication::startDragDistance()) { // 创建完全自由线
					/*allLines.append(tempLine);
					tempLine = nullptr;*/
					undoStack->push(new AddItemCommand(this, tempLine));
					tempLine = nullptr;
					
				}
				else { // 距离太短
					
					delete tempLine;
					tempLine = nullptr;
				}
				needsUpdate = true;
			}
			// 清理连接/绘图状态
			resetActionState(); // 使用 reset 清理状态
			// 不自动切换工具模式，保持连接模式
		}

		// --- B. 图形绘制工具模式 (切换回选择模式) ---
		else if (currentToolMode == ShapeDrawingTool && isDrawing) {
			QRect finalRect;
			Shape* shapeToCommit = nullptr;

			if (tempShape) {
				finalRect = tempShape->getRect();
				if (finalRect.isValid() && finalRect.width() >= minShapeSize && finalRect.height() >= minShapeSize) {
					shapeToCommit = tempShape;
					tempShape = nullptr; // 转移所有权
					
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
			// 绘制完图形后自动切换回选择模式
			setToolMode(SelectionTool);
			emit clearSelection(); // 通知取消工具栏按钮选中状态
			resetActionState(); // 确保清理绘图状态
		}

		// --- C. 选择工具模式 ---
		else if (currentToolMode == SelectionTool) {
			// --- C.1 结束框选 ---
			if (isSelecting) {
				QRectF selectionRectF(selectionStart, pos);
				selectionRectF = selectionRectF.normalized();
				// 只有拖动距离足够才执行选择逻辑
				if ((selectionStart - pos).manhattanLength() >= QApplication::startDragDistance()) {
					bool ctrl = event->modifiers() & Qt::ControlModifier;
					if (!ctrl) {
						selectedShapes.clear();
						selectedLines.clear();
					} // 非Ctrl替换选择
					// 添加相交的图形和线 (不变)
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
				else { // 拖动距离太短，视为单击空白 (Press事件已处理，这里是保险)
					
					bool ctrl = event->modifiers() & Qt::ControlModifier;
					if (!ctrl && !findTopShapeAt(pos) && !findTopLineAt(pos)) { // 确认是单击空白
						if (!selectedShapes.isEmpty() || !selectedLines.isEmpty()) {
							selectedShapes.clear();
							selectedLines.clear();
							updateSelectionStates();
							showControlPoints = false;
							
						}
					}
				}
				needsUpdate = true; // 框选结束需要更新
				isSelecting = false; // 清理状态
			}

			// --- C.2 结束缩放 ---
			if (isResizing) {
				
				needsUpdate = true;
				isResizing = false; // 清理状态
				currentHandle = -1;
				shapeOriginalRelativeRects.clear(); // 清理缓存
				lineOriginalRelativePoints.clear();
			}

			// --- C.3 结束拉伸 (检查吸附) ---
			if (isStretchingLine && lineBeingStretched) {
				Shape* targetShape = nullptr;
				int targetIndex = -1;
				// 检查释放点是否可以吸附到图形连接点
				// 需要排除线另一端连接的图形（如果存在）
				Shape* excludeShape = nullptr;
				if (stretchingStartHandle && lineBeingStretched->isEndAttached()) {
					excludeShape = lineBeingStretched->getEndShape();
				}
				else if (!stretchingStartHandle && lineBeingStretched->isStartAttached()) {
					excludeShape = lineBeingStretched->getStartShape();
				}

				bool snapped = false;
				if (findClosestPointOnShape(pos, targetShape, targetIndex, 12.0) && targetShape != excludeShape) {
					// 吸附成功
					if (stretchingStartHandle) { // 拉伸的是起点
						lineBeingStretched->attachStart(targetShape, targetIndex);
					}
					else { // 拉伸的是终点
						lineBeingStretched->attachEnd(targetShape, targetIndex);
					}
					snapped = true;
					
				}
				else {
					// 没有吸附，自由端点位置在 move 事件中已更新
					qDebug() << QObject::tr("SelectionTool: Stretching ended, no snapping.");
				}
				needsUpdate = true;
				isStretchingLine = false; // 清理状态
				lineBeingStretched = nullptr;
			}

			// --- C.4 结束移动 (使用基于状态的 MoveCommand) ---
			if (isMovingSelection) {
				
				// --- 应用最终的吸附偏移 ---
			   // m_currentSnapOffset 是最后一次 mouseMove 计算的值
			   // 如果吸附偏移不为零，需要再次移动选中的元素
				if (!m_currentSnapOffset.isNull()) {
		
					for (Shape* shape : selectedShapes) {
						if (shape) {
							shape->move(m_currentSnapOffset.toPoint()); // 在当前位置基础上再移动吸附偏移
						}
					}
					for (ConnectionLine* line : selectedLines) {
						if (line) {
							line->move(m_currentSnapOffset); // 移动自由端点
						}
					}
					// 如果有文本编辑器，也需要更新其最终位置
					if (m_isEditingText && m_editingTextBox && selectedShapes.contains(m_editingTextBox)) {
						if (m_textEditor) {
							updateTextEditorGeometry();
						}
					}
					needsUpdate = true; // 因为应用了吸附，需要更新
				}
				// 记录移动后的最终状态
				QMap<Shape*, QRectF> finalShapeRects;
				QMap<ConnectionLine*, QPair<QPointF, QPointF>> finalLinePoints;
				for (Shape* shape : selectedShapes) { // 使用当前的 selectedShapes
					if (shape) finalShapeRects[shape] = QRectF(shape->getRect());
				}
				for (ConnectionLine* line : selectedLines) {
					if (line) {
						// 始终记录当前端点位置，无论是否连接
						finalLinePoints[line] = { line->getStartPointPos(), line->getEndPointPos() };
					}
				}

				// --- **修改:** 比较初始状态和最终状态 ---
				bool hasMoved = false;
				// 检查数量是否变化 (理论上不应发生，但作为安全检查)
				if (m_originalShapeRectsOnMove.count() != finalShapeRects.count() ||
					m_originalLinePointsOnMove.count() != finalLinePoints.count()) {
					hasMoved = true;
					
				}
				else {
					// 比较图形矩形
					for (auto it = m_originalShapeRectsOnMove.constBegin(); it != m_originalShapeRectsOnMove.constEnd() && !hasMoved; ++it) { // 一旦移动就停止比较
						Shape* shape = it.key();
						const QRectF& oldRect = it.value();
						if (!finalShapeRects.contains(shape)) { // 对象不见了？
							hasMoved = true; 
						}
						const QRectF& newRect = finalShapeRects.value(shape);
						// *** 修正: 分别比较 QRectF 的四个浮点数分量 ***
						if (!qFuzzyCompare(oldRect.x(), newRect.x()) ||
							!qFuzzyCompare(oldRect.y(), newRect.y()) ||
							!qFuzzyCompare(oldRect.width(), newRect.width()) ||
							!qFuzzyCompare(oldRect.height(), newRect.height()))
						{
							hasMoved = true;
							
							// break; // 可以移除 break，完成所有比较的调试输出
						}
					}

					// 比较线的端点 (只有在图形没移动时才继续检查线)
					if (!hasMoved) {
						for (auto it = m_originalLinePointsOnMove.constBegin(); it != m_originalLinePointsOnMove.constEnd() && !hasMoved; ++it) {
							ConnectionLine* line = it.key();
							const QPointF& oldStart = it.value().first; // 这是原始的自由点位置(或空)
							const QPointF& oldEnd = it.value().second;  // 这是原始的自由点位置(或空)

							if (!finalLinePoints.contains(line)) { // 线不见了？
								hasMoved = true; 
							}
							const QPointF& newStart = finalLinePoints.value(line).first; // 当前的起点位置
							const QPointF& newEnd = finalLinePoints.value(line).second;   // 当前的终点位置

							// *** 修正: 只比较自由端点的位置 ***
							bool startMoved = false;
							if (!line->isStartAttached()) { // 如果起点是自由的
								// 比较其原始记录的位置 (oldStart) 和当前位置 (newStart)
								if (!qFuzzyCompare(oldStart.x(), newStart.x()) || !qFuzzyCompare(oldStart.y(), newStart.y())) {
									startMoved = true;
								}
							}

							bool endMoved = false;
							if (!line->isEndAttached()) { // 如果终点是自由的
								// 比较其原始记录的位置 (oldEnd) 和当前位置 (newEnd)
								if (!qFuzzyCompare(oldEnd.x(), newEnd.x()) || !qFuzzyCompare(oldEnd.y(), newEnd.y())) {
									endMoved = true;
								}
							}

							if (startMoved || endMoved) {
								hasMoved = true;
								
							}
						}
					}
				} // --- 结束状态比较 ---

				if (hasMoved) { // 只有真的移动了才 push 命令
					if (undoStack) {
						
						// 创建 MoveCommand，传入移动前和移动后的状态 Map
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
					needsUpdate = true; // 可能需要更新光标等
				}

				
				 // <<<--- 确认移动状态结束，清除参考线和吸附偏移 ---
				bool guidesNeedClearing = !m_alignmentGuides.isEmpty();
				m_alignmentGuides.clear();
				m_currentSnapOffset = QPointF(0, 0);
				if (guidesNeedClearing) {
					needsUpdate = true; // 如果清除了参考线，标记需要重绘
				}
				// 清理本次移动记录的原始状态
				m_originalShapeRectsOnMove.clear();
				m_originalLinePointsOnMove.clear();
				//isMovingSelection = false; // 结束移动状态

			} // 结束 isMovingSelection 处理

			// --- C.5 单击空白清除选择 (如果在 Press 中未处理，且当前无活动状态) ---
			if (!isResizing && !isStretchingLine && !isMovingSelection && !isSelecting && !needsUpdate) {
				// 检查是否真的点击在空白处
				bool ctrl = event->modifiers() & Qt::ControlModifier;
				Shape* clickedShape = findTopShapeAt(pos);
				ConnectionLine* clickedLine = findTopLineAt(pos);
				// 如果是单击空白（非Ctrl），并且没有选中任何东西，或者有选中但没点中元素
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
		} // 结束 if (currentToolMode == SelectionTool)

		// --- 统一更新光标和请求重绘 ---
		resetActionState(); // 确保所有状态标志复位
		mouseMoveEvent(event); // 调用 move 事件来设置释放后的光标

		if (needsUpdate) {
			update(); // 如果有任何需要重绘的情况
		}
		lineFormatChanged();
		shapeFormatChanged();
		/*if (!selectedLines.isEmpty() && selectedShapes.isEmpty()) {
			lineFormatChanged();
		}
		if (selectedLines.isEmpty() && !selectedShapes.isEmpty()) {
			shapeFormatChanged();
		}*/
	} // 结束 if (event->button() == Qt::LeftButton)
	else if (event->button() == Qt::RightButton) {
		// 右键释放通常不做任何事情，逻辑在 Press 中处理
		resetActionState(); // 确保状态干净
		mouseMoveEvent(event); // 更新光标
		lineFormatChanged();
		shapeFormatChanged();
	}
}



int DrawingArea::findAttachedEndNearPoint(const QPointF& pos, ConnectionLine*& foundLine) {
	foundLine = nullptr;
	const qreal handleCheckSize = ConnectionLine::handleSize * 1.5; // 使用稍大的检测范围
	const qreal handleCheckRadiusSq = (handleCheckSize / 2.0) * (handleCheckSize / 2.0);

	// 只检查选中的线
	for (ConnectionLine* line : selectedLines) {
		if (!line) continue;

		// 检查连接的起点
		if (line->isStartAttached()) {
			QPointF startP = line->getStartPointPos();
			if (isPointNear(pos, startP, handleCheckRadiusSq)) {
				foundLine = line;
				return 0; // 找到起点
			}
		}

		// 检查连接的终点
		if (line->isEndAttached()) {
			QPointF endP = line->getEndPointPos();
			if (isPointNear(pos, endP, handleCheckRadiusSq)) {
				foundLine = line;
				return 1; // 找到终点
			}
		}
	}
	return -1; // 未找到
}

// keyPressEvent (修改 Delete 逻辑)
void DrawingArea::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Escape) {

		// 执行原来右键的操作：
		resetActionState(); // 重置所有进行中的操作（如绘制、移动等）
		selectedShapes.clear(); // 清除图形选择
		selectedLines.clear();  // 清除线条选择
		updateSelectionStates(); // 更新内部选中状态
		showControlPoints = false; // 隐藏控制点
		setToolMode(SelectionTool); // 强制返回选择模式
		emit clearSelection();  // 通知外部（例如取消工具栏选中），保持信号名但理解其新含义
		update(); // 请求重绘

		// --- 新增：更新 FormatPanel ---
		shapeFormatChanged(); // 发射信号以更新面板
		lineFormatChanged();  // 发射信号以更新面板
		// --- 结束新增 ---

		
	}
	// --- 结束修改 Esc ---
	else {
		QWidget::keyPressEvent(event); // 其他键交给父类
	}
}

// --- 实现 contextMenuEvent ---
void DrawingArea::contextMenuEvent(QContextMenuEvent* event) {
	QMenu contextMenu(this);

	// --- 获取 Undo/Redo Action ---
	// 直接从 undoStack 创建，它们会自动管理 enabled 状态
	if (undoStack) {
		QAction* undoAction = undoStack->createUndoAction(&contextMenu, tr("Undo"));
		undoAction->setShortcut(QKeySequence::Undo); // 可选：显示快捷键提示
		contextMenu.addAction(undoAction);

		QAction* redoAction = undoStack->createRedoAction(&contextMenu, tr("Redo"));
		redoAction->setShortcut(QKeySequence::Redo); // 可选：显示快捷键提示
		contextMenu.addAction(redoAction);
	}

	contextMenu.addSeparator();

	// --- 添加剪切动作 ---
	QAction* cutAction = contextMenu.addAction(tr("Cut"));
	cutAction->setShortcut(QKeySequence::Cut);
	// 剪切操作需要有选中项
	cutAction->setEnabled(!selectedShapes.isEmpty() || !selectedLines.isEmpty());
	connect(cutAction, &QAction::triggered, this, &DrawingArea::cutActionTriggered);

	// --- 创建 Copy Action ---
	QAction* copyAction = contextMenu.addAction(tr("Copy"));
	copyAction->setShortcut(QKeySequence::Copy);
	// 根据是否有选中项来启用/禁用复制
	copyAction->setEnabled(!selectedShapes.isEmpty() || !selectedLines.isEmpty());
	// 连接到 DrawingArea 的复制方法
	connect(copyAction, &QAction::triggered, this, &DrawingArea::copySelectionToClipboard);

	// --- 创建 Paste Action ---
	QAction* pasteAction = contextMenu.addAction(tr("Paste"));
	pasteAction->setShortcut(QKeySequence::Paste);
	// 根据剪贴板内容来启用/禁用粘贴
	const QClipboard* clipboard = QApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();
	pasteAction->setEnabled(mimeData && mimeData->hasFormat(DIAGRAM_EDITOR_MIME_TYPE));
	// 连接到 DrawingArea 的粘贴方法
	connect(pasteAction, &QAction::triggered, this, &DrawingArea::pasteFromClipboard);

	// --- 添加删除动作 ---
	QAction* deleteAction = contextMenu.addAction(tr("Delete")); // 中文已替换为英文
	deleteAction->setShortcut(QKeySequence::Delete); // 显示快捷键提示
	// 启用/禁用删除动作基于是否有选中项
	deleteAction->setEnabled(!selectedShapes.isEmpty() || !selectedLines.isEmpty());
	// 连接 triggered 信号直接调用 deleteSelection 方法
	connect(deleteAction, &QAction::triggered, this, &DrawingArea::deleteSelection); // <--- 连接到删除方法

	// --- 显示菜单 ---
	// event->pos() 是相对于 DrawingArea 的坐标
	// event->globalPos() 是全局屏幕坐标
	contextMenu.exec(event->globalPos());

	// 不需要调用 event->accept()，因为 exec() 会处理
}

// --- 查找最上层的线 (不变) ---
ConnectionLine* DrawingArea::findTopLineAt(const QPointF& pos) {
	// 从后往前遍历 (绘制顺序的逆序)
	for (auto it = allLines.rbegin(); it != allLines.rend(); ++it) {
		// 使用 ConnectionLine 的 contains 方法
		if (*it && (*it)->contains(pos)) {
			return *it;
		}
	}
	return nullptr; // 没有找到
}

// --- 更新选中状态 (不变) ---
void DrawingArea::updateSelectionStates() {
	 //更新图形选中状态 (如果 Shape 类有 setSelected 方法)
	 /*for (Shape* shape : shapes) {
	     if (shape) shape->setSelected(selectedShapes.contains(shape));
	 }*/


	// 更新所有线的选中状态
	for (ConnectionLine* line : allLines) {
		if (line) line->setSelected(selectedLines.contains(line));
	}
}

// --- *修改*：只断开连接，不删除线 ---
void DrawingArea::detachConnectionsAttachedTo(Shape* shape) {
	if (!shape) return;

	int count = 0;
	for (ConnectionLine* line : allLines) {
		if (line) {
			// 检查是否连接到该图形
			bool wasAttached = (line->getStartShape() == shape || line->getEndShape() == shape);
			line->detachShape(shape); // 调用新的 detachShape 方法

			if (wasAttached && !line->isStartAttached() && !line->isEndAttached()) {
				// 可选：如果 detach 后变成完全自由线，可以做些什么，例如记录日志
				qDebug() << QObject::tr("Line became fully free after detaching from shape %1").arg(shape->getId());
			}

			if (wasAttached) count++;
		}
	}

	if (count > 0)
		qDebug() << QObject::tr("Detached %1 connection ends from shape %2").arg(count).arg(shape->getId());
}


// --- findTopShapeAt (不变) ---
Shape* DrawingArea::findTopShapeAt(const QPointF& pos) {
	for (auto it = shapes.rbegin(); it != shapes.rend(); ++it) {
		if (*it && (*it)->isSelected(pos.toPoint())) { // Shape::isSelected 用 QPoint
			return *it;
		}
	}
	return nullptr;
}

// --- findClosestPointOnShape (不变) ---
bool DrawingArea::findClosestPointOnShape(const QPointF& pos, Shape*& targetShape, int& targetIndex, double maxDist) {
	targetShape = nullptr;
	targetIndex = -1;
	double minDistSq = maxDist * maxDist + 1e-6; // 加一点点防止等于

	for (auto it = shapes.rbegin(); it != shapes.rend(); ++it) { // 从上层图形开始找
		Shape* currentShape = *it;
		if (!currentShape) continue;
		int currentBestIndex = -1;
		double currentBestDistSq = minDistSq; // 使用当前最小距离平方

		// 调用 Shape 的方法查找最近连接点
		// Shape::findClosestConnectionPoint 返回 true 如果找到点且距离平方小于传入的 currentBestDistSq
		if (currentShape->findClosestConnectionPoint(pos.toPoint(), currentBestIndex, currentBestDistSq, std::sqrt(minDistSq))) {
			// 如果找到了更近的点
			if (currentBestDistSq < minDistSq) {
				minDistSq = currentBestDistSq;
				targetShape = currentShape;
				targetIndex = currentBestIndex;
			}
		}
	}
	return targetShape != nullptr; // 如果找到了目标图形则返回 true
}


// --- 画布扩展 (修改：移除 line->move) ---
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

		// --- 平移所有元素 ---
		if (shift != QPoint(0, 0)) {
			qDebug() << QObject::tr("Expanding canvas and shifting elements by (%1, %2)").arg(shift.x()).arg(shift.y());

			// 平移图形
			for (Shape* s : shapes) {
				if (s) {
					s->move(shift);
				}
			}

			// 平移 *完全自由* 的线 和 部分连接线的 *自由端点*
			for (ConnectionLine* l : allLines) {
				if (l) {
					l->move(shift); // ConnectionLine::move 只移动自由端点
				}
			}

			// 平移临时图形和临时线 (如果存在)
			if (tempShape) {
				tempShape->move(shift);
			}
			if (tempLine) {
				tempLine->move(shift); // 假设临时线总是可以通过 move 移动其自由端
			}

			// 平移内部状态坐标
			startPoint += shift;
			endPoint += shift;
			lastMousePos += shift;
			selectionStart += shift;
			selectionEnd += shift;

			// 平移缩放缓存中的自由点坐标（如果正在缩放）
			if (isResizing && !originalResizeBounds.isNull()) {
				originalResizeBounds.translate(shift); // 平移原始边界记录
				// lineOriginalRelativePoints 是相对坐标，不需要平移
				// shapeOriginalRelativeRects 也是相对坐标，不需要平移
			}
		}

		// 设置新尺寸并调整滚动条
		int newWidth = currentWidth + expandLeftBy + expandRightBy;
		int newHeight = currentHeight + expandTopBy + expandBottomBy;
		QSize newCanvasSize(newWidth, newHeight); // <-- 创建 QSize 对象

		int oldScrollX = scrollAreaPtr->horizontalScrollBar()->value();
		int oldScrollY = scrollAreaPtr->verticalScrollBar()->value();

		// 使用 invokeMethod 确保安全地调整大小
		QMetaObject::invokeMethod(this, [this, newCanvasSize]() {
			this->setCanvasSize(newCanvasSize); // <-- 调用 setCanvasSize
			}, Qt::QueuedConnection);

		int newScrollX = oldScrollX + expandLeftBy;
		int newScrollY = oldScrollY + expandTopBy;
		QMetaObject::invokeMethod(scrollAreaPtr->horizontalScrollBar(), "setValue", Qt::QueuedConnection, Q_ARG(int, newScrollX));
		QMetaObject::invokeMethod(scrollAreaPtr->verticalScrollBar(), "setValue", Qt::QueuedConnection, Q_ARG(int, newScrollY));

		// 请求重绘以反映移动和尺寸变化
		update();
	}
}


// --- 更新悬停信息 (修改以处理拉伸排除) ---
void DrawingArea::updateHoveredShapeAndPoint(const QPointF& pos) {
	Shape* prevHoverShape = hoveredShape;
	int prevHoverIndex = hoveredConnectionPointIndex;

	Shape* newHoveredShape = nullptr;
	int newHoveredIndex = -1;

	// 条件：正在创建连接/自由线，或者正在拉伸线的自由端
	bool shouldCheckHover = (currentToolMode == ConnectionTool) ||
		(currentToolMode == SelectionTool && isStretchingLine && lineBeingStretched);

	if (shouldCheckHover) {
		Shape* excludeShape = nullptr; // 要排除的图形

		// 如果是从图形连接，则排除起始图形
		if (currentToolMode == ConnectionTool && isCreatingConnection) {
			//excludeShape = connectionStartShape;
		}
		// 如果是拉伸自由线，则排除该线 *另一端* 可能连接的图形
		else if (currentToolMode == SelectionTool && isStretchingLine && lineBeingStretched) {
			if (stretchingStartHandle && lineBeingStretched->isEndAttached()) { // 拉伸起点，排除终点图形
				excludeShape = lineBeingStretched->getEndShape();
			}
			else if (!stretchingStartHandle && lineBeingStretched->isStartAttached()) { // 拉伸终点，排除起点图形
				excludeShape = lineBeingStretched->getStartShape();
			}
		}

		// 查找鼠标下的最顶层图形 (排除 excludeShape)
		for (auto it = shapes.rbegin(); it != shapes.rend(); ++it) {
			Shape* currentShape = *it;
			if (!currentShape || currentShape == excludeShape) continue; // 跳过无效或排除的图形

			double closestDistSq = std::numeric_limits<double>::max();
			const double maxSnapDistance = 30.0; // 吸附探测半径
			bool findColseShapePoint = currentShape->findClosestConnectionPoint(pos.toPoint(), newHoveredIndex, closestDistSq, maxSnapDistance);
			if (currentShape->isSelected(pos.toPoint()) || findColseShapePoint) { // 检查鼠标是否在图形内

				// 在此图形上查找最近连接点
				if (currentShape->findClosestConnectionPoint(pos.toPoint(), newHoveredIndex, closestDistSq, maxSnapDistance)) {
					newHoveredShape = currentShape; // 找到了可以吸附的点
					// qDebug() << "Hovering near point" << newHoveredIndex << "on shape" << newHoveredShape->getId();
				}
				else {
					// 鼠标在图形上，但不在任何连接点附近
					newHoveredShape = currentShape;
					newHoveredIndex = -1; // 没有吸附点
					// qDebug() << "Hovering over shape" << newHoveredShape->getId() << "but not near points.";
				}
				break; // 找到最上层的图形就停止
			}
		}
	} // 结束 shouldCheckHover

	// --- 更新内部状态 ---
	bool changed = false;
	if (hoveredShape != newHoveredShape) {
		hoveredShape = newHoveredShape;
		changed = true;
		// qDebug() << "Hovered shape changed to:" << (hoveredShape ? hoveredShape->getId() : "None");
	}
	// 只有在拖动/拉伸过程中才更新高亮索引
	if (shouldCheckHover) {
		if (hoveredConnectionPointIndex != newHoveredIndex) {
			hoveredConnectionPointIndex = newHoveredIndex;
			changed = true;
			// qDebug() << "Hovered point index changed to:" << hoveredConnectionPointIndex;
		}
	}
	else { // 如果不是拖动/拉伸状态（即纯悬停），确保索引是 -1
		if (hoveredConnectionPointIndex != -1) {
			hoveredConnectionPointIndex = -1;
			changed = true;
			// qDebug() << "Reset hovered point index.";
		}
	}

	// 如果状态改变则请求重绘
	if (changed) {
		update();
	}
}

// --- 拖放事件 (不变) ---
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
		// --- 新增：处理 TEXT_BOX ---
		if (type == 6) { // 如果是文本框类型
			//QPoint pos = event->pos();
			QPointF pos = mapToScene(event->posF());
			QSize size = setIniShapeSize(type);; // 文本框默认大小
			QRect rect(pos.x() - size.width() / 2, pos.y() - size.height() / 2, size.width(), size.height());
			Shape* newShape = DiagramEditor::getShape(type, rect);
			if (newShape) {
				expandCanvasIfNeeded(newShape->getRect());
				undoStack->push(new AddItemCommand(this, newShape));
				showControlPoints = true; // 允许调整大小
				//setToolMode(SelectionTool); // 返回选择模式
				event->acceptProposedAction(); this->setFocus(); 
				update();
			}
			else {
				event->ignore();
			}
			return; // 处理完 TextBox，直接返回
		}
		// --- 结束新增 ---
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
			expandCanvasIfNeeded(newShape->getRect()); // 先扩展画布
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

// --- 设置拖放初始尺寸 (不变) ---
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

// --- 点接近判断 (不变) ---
bool DrawingArea::isPointNear(const QPointF& p1, const QPointF& p2, qreal thresholdSquared) {
	qreal dx = p1.x() - p2.x();
	qreal dy = p1.y() - p2.y();
	return (dx * dx + dy * dy) <= thresholdSquared;
}

// --- DrawingArea::mouseDoubleClickEvent (启动文本编辑) ---
void DrawingArea::mouseDoubleClickEvent(QMouseEvent* event) {
	// 仅在选择工具模式下响应双击
	if (currentToolMode == SelectionTool && event->button() == Qt::LeftButton) {
		// 如果当前正在编辑，则先结束之前的编辑
		if (m_isEditingText) {
			onTextEditFinished(); // 尝试提交之前的编辑
		}
		QPointF pos = mapToScene(event->localPos()); // <--- 转换坐标
		// 查找双击位置的最顶层图形
		//Shape* topShape = findTopShapeAt(event->pos());
		Shape* topShape = findTopShapeAt(pos);
		TextBox* textBox = dynamic_cast<TextBox*>(topShape);

		// 如果找到的是一个 TextBox
		if (textBox) {
			
			m_editingTextBox = textBox; // 记录正在编辑的文本框
			m_isEditingText = true;    // 设置编辑状态

			// --- 创建 QLineEdit 作为编辑器 ---
			QLineEdit* lineEdit = new QLineEdit(this); // 以 DrawingArea 为父控件
			m_textEditor = lineEdit; // 记录编辑器指针

			// 设置编辑器的位置和大小与 TextBox 一致
			lineEdit->setGeometry(textBox->getRect().adjusted(1, 1, -1, -1)); // 稍微内缩一点
			lineEdit->setText(textBox->getText());
			lineEdit->setFont(textBox->getFont()); // 使用 TextBox 的字体
			// lineEdit->setAlignment(textBox->getTextAlignment()); // QLineEdit 的对齐方式不同

			// 连接编辑完成信号 (例如按 Enter 键)
			connect(lineEdit, &QLineEdit::editingFinished, this, &DrawingArea::onTextEditFinished);
			// 可选：连接文本变化信号实时更新？(通常不需要)
			// connect(lineEdit, &QLineEdit::textChanged, ...);
			updateTextEditorGeometry(); // <--- 设置初始位置

			// 可选：处理焦点丢失来结束编辑
			// lineEdit->installEventFilter(this); // 需要重写 eventFilter 来捕获 FocusOut

			lineEdit->show();      // 显示编辑器
			lineEdit->setFocus();    // 设置键盘焦点
			lineEdit->selectAll(); // 全选文本，方便修改

			// 编辑时隐藏 TextBox 本身的文字 (通过 paintEvent 实现)
			update(); // 触发重绘

			event->accept(); // 事件已处理
			return; // 退出，避免传递给基类
		}
	}
	// 如果不是双击文本框或模式不对，则调用基类处理
	QWidget::mouseDoubleClickEvent(event);
}


// --- DrawingArea::onTextEditFinished (处理编辑完成，使用 Undo 命令) ---
void DrawingArea::onTextEditFinished() {

	// 1. 状态检查：确保我们确实处于文本编辑状态，并且相关指针有效
	if (!m_isEditingText || !m_editingTextBox || !m_textEditor) {
		// 尝试清理可能残留的编辑器，防止内存泄漏
		if (m_textEditor) {
			m_textEditor->deleteLater();
			m_textEditor = nullptr;
		}
		m_editingTextBox = nullptr; // 清除指针
		m_isEditingText = false;    // 重置状态标志
		// 最好还是将焦点还给绘图区
		this->setFocus();
		return; // 直接返回，不做任何操作
	}

	// --- 2. 获取编辑前后的文本 ---
	QString oldText = m_editingTextBox->getText(); // 获取编辑开始时的文本
	QString newText;
	// 确认编辑器是 QLineEdit 类型并获取文本
	if (QLineEdit* le = qobject_cast<QLineEdit*>(m_textEditor)) {
		newText = le->text(); // 获取编辑器中当前的文本
	}
	else {
		// 异常情况：无法获取新文本，直接清理状态，不改变原文本，不 push 命令
		m_textEditor->deleteLater(); m_textEditor = nullptr;
		m_editingTextBox = nullptr; m_isEditingText = false;
		this->setFocus();
		return;
	}

	// --- 3. 保存指针并清理编辑器、重置状态 ---
	// 在编辑器被 deleteLater 之前保存指向 TextBox 的指针，命令需要它
	QLineEdit* editorToDelete = m_textEditor;
	TextBox* textBoxEdited = m_editingTextBox; // 这个是我们要修改的目标 TextBox

	// 立刻重置编辑状态，防止用户在命令处理前再次双击等操作导致混乱
	m_textEditor = nullptr;
	m_editingTextBox = nullptr;
	m_isEditingText = false;
	this->setFocus(); // 将键盘焦点还给绘图区，以便响应键盘事件（如 Delete）

	// 安全地删除编辑器控件
	editorToDelete->deleteLater();

	// --- 4. 创建并 Push 命令 (仅当文本实际改变时) ---
	if (oldText != newText) {
		// 检查撤销栈是否有效
		if (undoStack) {
			// 创建 ChangeTextCommand，传入 DrawingArea、目标 TextBox、旧文本、新文本
			undoStack->push(new ChangeTextCommand(this, textBoxEdited, oldText, newText));
		}
		else {
			// --- 无 UndoStack 的回退方案 ---
			textBoxEdited->setText(newText); // 直接应用新文本
			update(); // 手动触发重绘
			shapeFormatChanged(); // 手动通知 FormatPanel 更新
			// --- 结束回退方案 ---
		}
	}
	else {
		update();
		// 可能仍然需要通知 FormatPanel，以防字体等其他属性在编辑期间通过其他方式改变
		shapeFormatChanged();
	}
	// 不需要在这里调用 update() 或 shapeFormatChanged()，因为命令的 redo 会处理
}


// --- 图层操作方法 (不变) ---
void DrawingArea::onMoveToTopLayer() {
	if (!selectedShapes.isEmpty()) {
		QList<Shape*> shapesToMove = selectedShapes; // 处理副本
		for (Shape* s : shapesToMove) {
			int index = shapes.indexOf(s);
			if (index != -1) shapes.move(index, shapes.size() - 1);
		}
		update();
	} // 线的图层暂不处理
}
void DrawingArea::onMoveToBottomLayer() {
	if (!selectedShapes.isEmpty()) {
		QList<Shape*> shapesToMove = selectedShapes;
		int targetIndex = 0;
		for (Shape* shape : shapesToMove) {
			int index = shapes.indexOf(shape);
			if (index != -1) shapes.move(index, targetIndex++); // 移到前面
		}
		update();
	}
}
void DrawingArea::onMoveUpOneLayer() {
	if (!selectedShapes.isEmpty()) {
		QList<Shape*> shapesToMove = selectedShapes;
		// 从后往前处理，防止索引变化影响
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
		// 从前往后处理
		for (int i = 0; i < shapes.size(); ++i) {
			Shape* shape = shapes.at(i);
			if (shapesToMove.contains(shape)) {
				if (i > 0) shapes.move(i, i - 1);
			}
		}
		update();
	}
}

// --- DrawingArea 实现结束 ---

void DrawingArea::requestChangeBorderColor() {
	// 检查是否有选中的图形或文本框
	if (selectedShapes.isEmpty()) {
		QMessageBox::information(this, tr("Tip"), tr("Please select one or more shapes or text boxes first."));
		return;
	}

	// 获取第一个选中项的当前边框颜色作为默认值
	QColor currentColor = selectedShapes.first()->getBorderColor();
	QColor newColor = QColorDialog::getColor(currentColor, this, tr("Select Border Color"));

	undoStack->push(new ChangeBorderColorCommand(this, selectedShapes, newColor));
}

void DrawingArea::requestChangeFillColor() {
	if (selectedShapes.isEmpty()) {
		QMessageBox::information(this, tr("Tip"), tr("Please select one or more shapes or text boxes first."));
		return;
	}

	// 获取第一个选中项的当前填充颜色
	QColor currentColor = selectedShapes.first()->getFillColor();
	QColor newColor = QColorDialog::getColor(currentColor, this, tr("Select Fill Color"));

	undoStack->push(new ChangeFillColorCommand(this, selectedShapes, newColor));
}

void DrawingArea::requestToggleFill() {
	if (selectedShapes.isEmpty()) {
		QMessageBox::information(this, tr("Tip"), tr("Please select one or more shapes or text boxes first."));
		return;
	}

	// 以第一个选中项的状态为准进行切换
	bool newState = !selectedShapes.first()->isFilled(); // 获取第一个的反状态

	for (Shape* shape : selectedShapes) {
		shape->setFilled(newState);
	}
	update();
}

void DrawingArea::requestNoFill() {
	if (selectedShapes.isEmpty()) {
		//QMessageBox::information(this, u8"提示", u8"请4先选择一个或多个图形或文本框。");
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

	// 获取第一个选中项的当前宽度
	qreal currentWidth = selectedShapes.first()->getBorderWidth();

	// 找到当前宽度在列表中的索引
	int currentIndex = 0;
	if (currentWidth == 0) {
		currentIndex = 0; // 无边框对应的索引
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
		currentWidth, 0.5, 20.0, 1, &ok, // 最小值0.5
		Qt::WindowCloseButtonHint);

	undoStack->push(new ChangeLineWidthCommand(this, selectedLines, newWidth));
}

void DrawingArea::requestChangeLineStyle() {
	if (selectedLines.isEmpty()) {
		QMessageBox::information(this, tr("Tip"), tr("Please select one or more connection lines first."));
		return;
	}

	// 定义线型选项
	QStringList items;
	QMap<QString, Qt::PenStyle> styleMap;
	items << tr("Solid Line"); styleMap[tr("Solid Line")] = Qt::SolidLine;
	items << tr("Dash Line"); styleMap[tr("Dash Line")] = Qt::DashLine;
	items << tr("Dot Line"); styleMap[tr("Dot Line")] = Qt::DotLine;
	items << tr("Dash-Dot Line"); styleMap[tr("Dash-Dot Line")] = Qt::DashDotLine;
	items << tr("Dash-Dot-Dot Line"); styleMap[tr("Dash-Dot-Dot Line")] = Qt::DashDotDotLine;

	// 获取当前样式名称
	Qt::PenStyle currentStyle = selectedLines.first()->getLineStyle();
	QString currentItem = tr("Solid Line"); // 默认
	for (auto it = styleMap.constBegin(); it != styleMap.constEnd(); ++it) {
		if (it.value() == currentStyle) {
			currentItem = it.key();
			break;
		}
	}

	bool ok;
	QString selectedItem = QInputDialog::getItem(this, tr("Select Line Style"), tr("Style:"), items, items.indexOf(currentItem), false, &ok); // 不可编辑
	Qt::PenStyle newStyle = styleMap.value(selectedItem, Qt::SolidLine);

	undoStack->push(new ChangeLineStyleCommand(this, selectedLines, newStyle));
}

void DrawingArea::requestChangeTextFont() {
	TextBox* firstTextBox = nullptr;
	// 找到第一个选中的 TextBox
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

	// 将整个 selectedShapes 列表传递给命令，命令内部会过滤出 TextBox
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

	//// 使用浮点数比较来判断目标宽度是否为 "无边框" (即 0)
	//bool isTargetNoBorder = qFuzzyCompare(newWidth, 0);
	//for (Shape* shape : selectedShapes) {
	//	if (!shape) continue;

	//	// 获取图形当前的边框宽度，判断修改前的状态
	//	qreal currentWidth = shape->getBorderWidth();
	//	bool wasNoBorder = qFuzzyCompare(currentWidth, 0);

	//	// 1. 设置新的边框宽度
	//	//shape->setBorderWidth(newWidth);
	//	undoStack->push(new ChangeBorderWidthCommand(this, shapes, newWidth));
	//	// 2. 根据新旧状态调整颜色
	//	if (isTargetNoBorder) {
	//		// 如果目标是无边框，将颜色设置为透明
	//		if (shape->getBorderColor() != Qt::transparent) {
	//			shape->setBorderColor(Qt::transparent);
	//		}
	//	}
	//	else {
	//		if (wasNoBorder) {
	//			// 并且之前是无边框，则将颜色重置为黑色
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
// 实现新的方法

void DrawingArea::changeSelectedShapesBorderStyle(Qt::PenStyle style) {
	if (selectedShapes.isEmpty() || !undoStack) {
		qDebug() << "DrawingArea: Cannot change border style - no selection or no undo stack.";
		return;
	}

	qDebug() << "DrawingArea: Requesting to change border style for selected shapes to" << static_cast<int>(style);
	// 创建并推送命令
	undoStack->push(new ChangeBorderStyleCommand(this, selectedShapes, style));
	// 命令的 redo 会自动调用，处理实际的修改和界面更新
}

void DrawingArea::changeSelectedLineWidth(qreal width) {
	if (selectedLines.isEmpty() || !undoStack) return;

	/*for (ConnectionLine* line : selectedLines) {
		line->setLineWidth(width);
	}*/
	undoStack->push(new ChangeLineWidthCommand(this, selectedLines, width));
	update(); // 刷新界面
}


// ... 其他 DrawingArea 方法 ...

void DrawingArea::changeSelectedShapeWidth(int width)
{
	if (selectedShapes.size() != 1 || !undoStack) {
		return;
	}

	Shape* shape = selectedShapes.first();
	if (!shape) return;

	// --- 获取当前矩形 (QRect)，然后转换为 QRectF ---
	QRect currentIntRect = shape->getRect();
	QRectF oldRect = QRectF(currentIntRect); // <--- 从 QRect 构造 QRectF

	// 创建只改变宽度的新浮点矩形
	QRectF newRect = oldRect;
	newRect.setWidth(width); // QRectF 的 setWidth 接受 qreal (int 可以隐式转换)

	// 检查宽度是否真的改变 (使用浮点比较) 且新宽度有效
	// 注意：minShapeSize 通常是 int，需要比较 qreal >= int
	if (!qFuzzyCompare(oldRect.width(), newRect.width()) && newRect.width() >= static_cast<qreal>(minShapeSize))
	{
		// --- 传递 QRectF 给命令 ---
		undoStack->push(new ResizeShapeCommand(this, shape, newRect)); // <--- 传递的是 newRect (QRectF)
	}
}

void DrawingArea::changeSelectedShapeHeight(int height)
{
	if (selectedShapes.size() != 1 || !undoStack) {
		return;
	}

	Shape* shape = selectedShapes.first();
	if (!shape) return;

	// --- 获取当前矩形 (QRect)，然后转换为 QRectF ---
	QRect currentIntRect = shape->getRect();
	QRectF oldRect = QRectF(currentIntRect); // <--- 从 QRect 构造 QRectF

	// 创建只改变高度的新浮点矩形
	QRectF newRect = oldRect;
	newRect.setHeight(height); // QRectF 的 setHeight 接受 qreal

	// 检查高度是否真的改变 (使用浮点比较) 且新高度有效
	if (!qFuzzyCompare(oldRect.height(), newRect.height()) && newRect.height() >= static_cast<qreal>(minShapeSize))
	{
		// --- 传递 QRectF 给命令 ---
		undoStack->push(new ResizeShapeCommand(this, shape, newRect)); // <--- 传递的是 newRect (QRectF)
	}
}

void DrawingArea::changeSelectedlineStyle(Qt::PenStyle style) {
	if (selectedLines.isEmpty()) {
		QMessageBox::information(this, tr("Tip"), tr("Please select one or more connection lines first."));
		return;
	}
	undoStack->push(new ChangeLineStyleCommand(this, selectedLines, style));
}
// --- 实现新增的公共方法 ---
void DrawingArea::changeSelectedLineArrowState(bool hasArrow) {
	if (selectedLines.isEmpty()) {
		return; // 没有选中的线条
	}

	bool changed = false;
	for (ConnectionLine* line : selectedLines) {
		if (line && line->hasArrowHead() != hasArrow) { // 检查状态是否真的改变
			line->setHasArrowHead(hasArrow);
			changed = true;
		}
	}

	if (changed) {
		update(); // 触发重绘以显示或隐藏箭头
		lineFormatChanged(); // 通知 FormatPanel 更新（可能状态不一致时点击了复选框）
		// 注意：这里暂时不添加 Undo/Redo 命令，如果需要，应该在这里创建并 push 命令
	}
}


QPointF DrawingArea::mapToScene(const QPointF& viewportPos) const {
	if (!scrollAreaPtr) {
		// 如果没有滚动区域，简单地进行缩放反算
		return viewportPos / m_scaleFactor;
	}
	// 结合滚动条位置和缩放因子进行转换
	/*QPointF widgetPos = viewportPos -QPointF(scrollAreaPtr->horizontalScrollBar()->value(),
		scrollAreaPtr->verticalScrollBar()->value());
	return widgetPos / m_scaleFactor;*/
	return viewportPos / m_scaleFactor;
}


void DrawingArea::updateTextEditorGeometry() {
	// 仅当处于编辑状态且所有指针有效时才执行
	if (!m_isEditingText || !m_editingTextBox || !m_textEditor || !scrollAreaPtr) {
		return;
	}

	// 1. 获取 TextBox 的逻辑 (场景) 矩形
	QRect sceneRect = m_editingTextBox->getRect();

	// 2. 应用当前的缩放因子，得到在完整 DrawingArea 画布坐标系下的矩形
	QTransform transform;
	transform.scale(m_scaleFactor, m_scaleFactor);
	QRectF scaledWidgetRect = transform.mapRect(QRectF(sceneRect));

	// 3. 减去当前的滚动条偏移量，得到在视口 (Viewport) 中的矩形
	/*QPointF scrollOffset(scrollAreaPtr->horizontalScrollBar()->value(),
		scrollAreaPtr->verticalScrollBar()->value());
	QRectF viewportRect = scaledWidgetRect.translated(-scrollOffset);*/
	QRectF viewportRect = scaledWidgetRect;
	// 4. 设置 QLineEdit 的几何位置 (转换为整数像素并微调)
	//    使用 toAlignedRect() 进行像素对齐，避免模糊
	m_textEditor->setGeometry(viewportRect.toAlignedRect().adjusted(1, 1, -1, -1));

	// 5. 确保编辑器在最上层
	m_textEditor->raise();

}