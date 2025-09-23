#pragma once // ��׼ͷ�ļ�����

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
// --- ǰ������ ---

class DrawingArea;
class ToolPanel;
class MenuBar;
class Shape;
class ConnectionLine;
class TextBox; 
class FormatPanel;

// --- DiagramEditor �� ---
// �������࣬Э���������
class DiagramEditor : public QWidget {
    Q_OBJECT // �����źźͲۻ���

public:
    // ���캯��
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
    // --- ��Ӧ MenuBar �źŵĲۺ��� ---
    void createNewWindow(); 
    void onOpen();
    void onSave();
    void switchAera();
    void retranslateUi(); 

    // --- ��Ӧ MenuBar ��ʽ�˵��źŵĲۺ��� ---
    void onFormatChangeBorderColor(); // 
    
    void onFormatChangeFillColor();   // �޸������ɫ
    void onFormatToggleFill();        // �л����״̬
    void onFormatChangeBorderWidth(); // �޸ı߿���  
    void onFormatChangeLineColor();   // �޸�������ɫ
    void onFormatChangeLineWidth();   // �޸��������
    void onFormatChangeLineStyle();   // �޸�������ʽ
    void onFormatChangeTextFont();    // �޸��ı�����
    void onFormatChangeTextColor();   // �޸��ı���ɫ

    void onSelectedShapesBorderWidth(qreal width);
    void onSelectedShapesBorderStyle(Qt::PenStyle style);
    void onSelectedLineWidth(qreal width);
    void onSelectedlineStyleChanged(Qt::PenStyle style);
    void onSelectedLineArrowStateChanged(bool hasArrow); 
    void onSelectedTextFontChanged(const QFont& font);
    void onFormatToggleNoFill();
    void onSelectedShapeWidthChanged(int width);
    void onSelectedShapeHeightChanged(int height);
    // --- ��Ӧ DrawingArea �źŵĲۺ��� ---
    void handleClearSelection(); 
    void onPanelCanvasWidthChanged(int width);   
    void onPanelCanvasHeightChanged(int height); 
    void onPanelCanvasZoomChanged(double zoomPercentage); 

    void onDrawingAreaZoomChanged(double newZoomFactor);   
    void onDrawingAreaSizeChanged(const QSize& newSize);     

    // --- ���������� FormatPanel �Ĳ� ---
    void updateFormatPanelAll(); 
    QUndoStack* getUndoStack() const;

protected:
    void changeEvent(QEvent* event) override;

private:
    // --- UI ���ָ�� ---
    QSplitter* splitter;        
    QScrollArea* scrollArea;    
    QScrollArea* toolscrollArea;
    DrawingArea* drawingArea;   
    ToolPanel* toolPanel;      
    MenuBar* menuBar;        
    FormatPanel* formatPanel;
    QScrollArea* formatScrollArea;

    QUndoStack* undoStack; 
    // --- ״̬���� ---
    bool clickLineMode = false; 

    // --- ��ʼ���������� ---
    void initToolPanel(); 
    void initFormatPanel();
    void setDiagramEditorSize(int size_w, int size_h); 
};


// --- DrawingArea ������ ---
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

    // �����ʽ�˵�����ķ���
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

    // --- ��ȡѡ����� Getter ---
    const QList<Shape*>& getSelectedShapes() const { return selectedShapes; }
    const QList<ConnectionLine*>& getSelectedLines() const { return selectedLines; }

   

    // --- �����ĳ�Ա���� (����ʹ�ã����ͨ����������) ---
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
    // �Ҽ����ʱ�������ź�
    void clearSelection();
    void shapeFormat(const QList<Shape*>& selectedShapes);
    void lineFormat(const QList<ConnectionLine*>& selectedLines);
    void onSelectedShapesBorderColor(QColor color);
    void zoomChanged(double newZoomFactor);     
    void canvasSizeChanged(const QSize& newSize); 

protected:
    // --- �¼������� ---
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override; // ����˫���༭
    void keyPressEvent(QKeyEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void wheelEvent(QWheelEvent* event) override; // <-- ��д�����¼�
    void contextMenuEvent(QContextMenuEvent* event) override;


private:
    // --- ˽�г�Ա���� ---
    QScrollArea* scrollAreaPtr; 
    QUndoStack* undoStack;

    // --- ״̬��־ ---
    bool isDrawing = false;          
    bool isResizing = false;         
    bool isSelecting = false;        
    bool isMovingSelection = false;  
    bool isCreatingConnection = false;
    bool isStretchingLine = false;   
    bool m_isEditingText = false;    
    
    // --- ���������е���ʱ���� ---
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

    // --- ��ͣ������ ---
    Shape* hoveredShape = nullptr;         
    int hoveredConnectionPointIndex = -1; 

    QList<QLineF> m_alignmentGuides;     
    QPointF m_currentSnapOffset;        
    const qreal SNAP_THRESHOLD = 5.0;    


    // --- ����ͻ��� ---
    int gridSize = 20;        
    QColor gridColor = Qt::lightGray; 
    int minShapeSize = 10;      
    double m_scaleFactor=1.0; 
    // --- �ı��༭ ---
    TextBox* m_editingTextBox = nullptr; 
    QLineEdit* m_textEditor = nullptr;     
    QMap<Shape*, QRectF> m_originalShapeRectsOnMove;
    QMap<ConnectionLine*, QPair<QPointF, QPointF>> m_originalLinePointsOnMove;

    // --- ˽�и������� ---
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