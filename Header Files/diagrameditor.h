#pragma once // 标准头文件保护

#include <QWidget>          
#include <QList>            
#include <QRect>            
#include <QPointF>          
#include <QRectF>           
#include <QMap>             
#include <QPair>            
#include <QSet>             
#include <QPen>             
#include <QFont>            
#include <QColor>           
#include <QObject>          
#include <QScrollArea>      
#include <QSplitter>        
#include <QVBoxLayout>      
#include <QMenuBar>         
#include <QMouseEvent>      
#include <QKeyEvent>        
#include <QPaintEvent>      
#include <QDragEnterEvent>  
#include <QDropEvent>       
#include <QAction>         
#include <QMenu>            
#include <QColorDialog>     
#include <QInputDialog>     
#include <QFontDialog>      
#include <QLineEdit>        
#include <QApplication>     
#include <QDebug>           
#include <QtMath>          
#include <limits>          
#include <QWheelEvent>    
#include <QUndoStack>
// --- 前向声明 ---

class DrawingArea;
class ToolPanel;
class MenuBar;
class Shape;
class ConnectionLine;
class TextBox; 
class FormatPanel;

// --- DiagramEditor 类 ---
// 主窗口类，协调各个组件
class DiagramEditor : public QWidget {
    Q_OBJECT // 启用信号和槽机制

public:
    // 构造函数
    explicit DiagramEditor(QWidget* parent = nullptr);
    static Shape* getShape(int currentShapeType, QRect rect);

public slots:
    void onCopy();  
    void onPaste(); 
    void onDelete(); 
    void onCut(); 
    void onAlignTop();
    void onAlignVCenter();
    void onAlignBottom();

private slots:
    // --- 响应 MenuBar 信号的槽函数 ---
    void createNewWindow(); 
    void onOpen();
    void onSave();
    void switchAera();
    void retranslateUi(); 

    // --- 响应 MenuBar 格式菜单信号的槽函数 ---
    void onFormatChangeBorderColor(); // 
    
    void onFormatChangeFillColor();   // 修改填充颜色
    void onFormatToggleFill();        // 切换填充状态
    void onFormatChangeBorderWidth(); // 修改边框宽度  
    void onFormatChangeLineColor();   // 修改线条颜色
    void onFormatChangeLineWidth();   // 修改线条宽度
    void onFormatChangeLineStyle();   // 修改线条样式
    void onFormatChangeTextFont();    // 修改文本字体
    void onFormatChangeTextColor();   // 修改文本颜色

    void onSelectedShapesBorderWidth(qreal width);
    void onSelectedShapesBorderStyle(Qt::PenStyle style);
    void onSelectedLineWidth(qreal width);
    void onSelectedlineStyleChanged(Qt::PenStyle style);
    void onSelectedLineArrowStateChanged(bool hasArrow); 
    void onSelectedTextFontChanged(const QFont& font);
    void onFormatToggleNoFill();
    void onSelectedShapeWidthChanged(int width);
    void onSelectedShapeHeightChanged(int height);
    // --- 响应 DrawingArea 信号的槽函数 ---
    void handleClearSelection(); 
    void onPanelCanvasWidthChanged(int width);   
    void onPanelCanvasHeightChanged(int height); 
    void onPanelCanvasZoomChanged(double zoomPercentage); 

    void onDrawingAreaZoomChanged(double newZoomFactor);   
    void onDrawingAreaSizeChanged(const QSize& newSize);     

    // --- 新增：更新 FormatPanel 的槽 ---
    void updateFormatPanelAll(); 
    QUndoStack* getUndoStack() const;

protected:
    void changeEvent(QEvent* event) override;

private:
    // --- UI 组件指针 ---
    QSplitter* splitter;        
    QScrollArea* scrollArea;    
    QScrollArea* toolscrollArea;
    DrawingArea* drawingArea;   
    ToolPanel* toolPanel;      
    MenuBar* menuBar;        
    FormatPanel* formatPanel;
    QScrollArea* formatScrollArea;

    QUndoStack* undoStack; 
    // --- 状态变量 ---
    bool clickLineMode = false; 

    // --- 初始化辅助函数 ---
    void initToolPanel(); 
    void initFormatPanel();
    void setDiagramEditorSize(int size_w, int size_h); 
};


// --- DrawingArea 类声明 ---
class DrawingArea : public QWidget {
    Q_OBJECT 

public:
    enum ToolMode { 
        SelectionTool, 
        ShapeDrawingTool, 
        ConnectionTool 
    };

    DrawingArea(QScrollArea* scrollAreaParent, QUndoStack* stack,QWidget* parent = nullptr );
    ~DrawingArea();

    void setToolMode(ToolMode mode);
    void setGridSize(int size);

    void onMoveToTopLayer();
    void onMoveToBottomLayer();
    void onMoveUpOneLayer();
    void onMoveDownOneLayer();

    // 处理格式菜单请求的方法
    void requestChangeBorderColor();
    void requestChangeFillColor();
    void requestToggleFill();
    void requestChangeBorderWidth();
    void requestChangeLineColor();
    void requestChangeLineWidth();
    void requestChangeLineStyle();

    void requestChangeTextFont();
    void requestChangeTextColor();
    void requestNoFill();


    void changeSelectedShapesBorderWidth(qreal width);
    void changeSelectedShapesBorderStyle(Qt::PenStyle style); 

    void changeSelectedShapeWidth(int width);
    void changeSelectedShapeHeight(int height);

    void changeSelectedLineWidth(qreal width);
    void changeSelectedlineStyle(Qt::PenStyle style);
    void changeSelectedLineArrowState(bool hasArrow); 
    void changeSelectedTextFont(const QFont& font);

    void setZoomFactor(double factor);    
    void setCanvasSize(const QSize& size); 

    // --- 获取选中项的 Getter ---
    const QList<Shape*>& getSelectedShapes() const { return selectedShapes; }
    const QList<ConnectionLine*>& getSelectedLines() const { return selectedLines; }

   

    // --- 公开的成员变量 (谨慎使用，最好通过方法访问) ---
    ToolMode currentToolMode = SelectionTool; 
    int currentShapeType = -1;               
    bool isGridMode = false;                
    int expansionMargin = 50;              
    QList<Shape*> shapes;                 
    QList<ConnectionLine*> allLines;       
    QList<Shape*> selectedShapes;          
    QList<ConnectionLine*> selectedLines;  
    bool showControlPoints = false;          
    void updateSelectionStates(); 

    qreal getZoomFactor() const;      
    QSize canvasSize() const;         

    void cutActionTriggered();      
    void copySelectionToClipboard(); 
    void pasteFromClipboard();      
    void deleteSelection(); 
    void alignSelectedShapes(Qt::Alignment alignment); 
    void detachConnectionsAttachedTo(Shape* shape);
    void expandCanvasIfNeeded(const QRect& elementBounds); 

signals:
    // 右键点击时发出的信号
    void clearSelection();
    void shapeFormat(const QList<Shape*>& selectedShapes);
    void lineFormat(const QList<ConnectionLine*>& selectedLines);
    void onSelectedShapesBorderColor(QColor color);
    void zoomChanged(double newZoomFactor);     
    void canvasSizeChanged(const QSize& newSize); 

protected:
    // --- 事件处理函数 ---
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override; // 处理双击编辑
    void keyPressEvent(QKeyEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void wheelEvent(QWheelEvent* event) override; // <-- 重写滚轮事件
    void contextMenuEvent(QContextMenuEvent* event) override;


private:
    // --- 私有成员变量 ---
    QScrollArea* scrollAreaPtr; 
    QUndoStack* undoStack;

    // --- 状态标志 ---
    bool isDrawing = false;          
    bool isResizing = false;         
    bool isSelecting = false;        
    bool isMovingSelection = false;  
    bool isCreatingConnection = false;
    bool isStretchingLine = false;   
    bool m_isEditingText = false;    
    
    // --- 交互过程中的临时数据 ---
    QPointF startPoint;              
    QPointF endPoint;                 
    QPointF lastMousePos;             
    QPointF selectionStart;           
    QPointF selectionEnd;             
    Shape* tempShape = nullptr;       
    ConnectionLine* tempLine = nullptr;
    Shape* connectionStartShape = nullptr; 
    int connectionStartPointIndex = -1;   
    ConnectionLine* lineBeingStretched = nullptr; 
    bool stretchingStartHandle = false; 
    int currentHandle = -1;           
    QRectF originalResizeBounds;     
    QMap<Shape*, QRectF> shapeOriginalRelativeRects; 
    QMap<ConnectionLine*, QPair<QPointF, QPointF>> lineOriginalRelativePoints; 

    // --- 悬停和吸附 ---
    Shape* hoveredShape = nullptr;         
    int hoveredConnectionPointIndex = -1; 

    QList<QLineF> m_alignmentGuides;     
    QPointF m_currentSnapOffset;        
    const qreal SNAP_THRESHOLD = 5.0;    


    // --- 网格和画布 ---
    int gridSize = 20;        
    QColor gridColor = Qt::lightGray; 
    int minShapeSize = 10;      
    double m_scaleFactor=1.0; 
    // --- 文本编辑 ---
    TextBox* m_editingTextBox = nullptr; 
    QLineEdit* m_textEditor = nullptr;     
    QMap<Shape*, QRectF> m_originalShapeRectsOnMove;
    QMap<ConnectionLine*, QPair<QPointF, QPointF>> m_originalLinePointsOnMove;

    // --- 私有辅助方法 ---
    void resetActionState(); 
    Shape* findTopShapeAt(const QPointF& pos); 
    ConnectionLine* findTopLineAt(const QPointF& pos); 
    bool findClosestPointOnShape(const QPointF& pos, Shape*& targetShape, int& targetIndex, double maxDist);
    int findHandleForFreeEnd(const QPointF& pos, ConnectionLine* line);
    int findAttachedEndNearPoint(const QPointF& pos, ConnectionLine*& foundLine);
    void setCursorBasedOnHandle(int handleIndex); 
    void updateHoveredShapeAndPoint(const QPointF& pos); 
    QSize setIniShapeSize(int iniType);
    bool isPointNear(const QPointF& p1, const QPointF& p2, qreal thresholdSquared);
    QPointF mapToScene(const QPointF& viewportPos) const; 
    void updateTextEditorGeometry();

private slots:
    void onTextEditFinished();

    
public slots:
    void shapeFormatChanged();
    void lineFormatChanged();


}; 