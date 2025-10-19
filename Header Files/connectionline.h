
#include <QObject> 
#include <QColor>
#include <QPointF>
#include <QPen>
#include <QJsonObject> 
#include <QMap>      



class Shape;
class ConnectionLine { 
public:
    static const qreal handleSize;       
    static const qreal selectionPadding;  
    

    ConnectionLine(Shape* startShape, int startPointIndex, Shape* endShape, int endPointIndex,
        QColor color = Qt::darkGray, qreal lineWidth = 1.5, Qt::PenStyle lineStyle = Qt::SolidLine, bool hasArrow = true); // ���Ĭ��ֵ
    ConnectionLine(const QPointF& p1, const QPointF& p2,
        QColor color = Qt::darkGray, qreal lineWidth = 1.5, Qt::PenStyle lineStyle = Qt::SolidLine, bool hasArrow = true); // ���Ĭ��ֵ
    ConnectionLine(); 

    // --- ������ѡ�� ---
    void draw(QPainter& painter) const;
    bool contains(const QPointF& point, qreal tolerance = 3.0) const;
    void setSelected(bool selected);
    bool isSelected() const { return m_selected; }
    QRectF getBoundingRect() const;

    // --- ���� Getters ---
    QColor getColor() const { return m_color; }
    qreal getLineWidth() const { return m_lineWidth; }      
    Qt::PenStyle getLineStyle() const { return m_lineStyle; } 
    Shape* getStartShape() const { return m_startShape; }
    int getStartPointIndex() const { return m_startPointIndex; }
    Shape* getEndShape() const { return m_endShape; }
    int getEndPointIndex() const { return m_endPointIndex; }
    QPointF getStartPointPos() const; 
    QPointF getEndPointPos() const;   
    QString getId() const { return m_id; } 
    bool hasArrowHead() const; 

    // --- ���� Setters ---
    void setColor(const QColor& color);
    void setLineWidth(qreal width);       
    void setLineStyle(Qt::PenStyle style); 
    void setId(const QString& id) { m_id = id; } 
    void setHasArrowHead(bool hasArrow); 

    // --- ���ӹ��� ---
    bool isStartAttached() const { return m_startShape != nullptr && m_startPointIndex >= 0; }
    bool isEndAttached() const { return m_endShape != nullptr && m_endPointIndex >= 0; }
    bool isFullyConnected() const { return isStartAttached() && isEndAttached(); }
    bool isFullyFree() const { return !isStartAttached() && !isEndAttached(); }
    bool isPartiallyAttached() const { return (isStartAttached() != isEndAttached()); } 

    void attachStart(Shape* shape, int pointIndex);
    void attachEnd(Shape* shape, int pointIndex);
    void detachStart();
    void detachEnd();
    void detachShape(const Shape* shape); 

    void setFreeStartPoint(const QPointF& p);
    void setFreeEndPoint(const QPointF& p);

    // --- �������� ---
    void move(const QPointF& offset); 

    // --- ���л� / �����л� ---
    QJsonObject toJson() const;
    static ConnectionLine* fromJson(const QJsonObject& obj, const QMap<QString, Shape*>& shapeIdMap);

private:
    // --- �������� ---
    QString m_id; 
    QColor m_color;             
    qreal m_lineWidth;          
    Qt::PenStyle m_lineStyle;   
    bool m_selected;            

    // --- ������Ϣ ---
    Shape* m_startShape;        
    int m_startPointIndex;      
    QPointF m_freeStartPoint;   

    Shape* m_endShape;          
    int m_endPointIndex;        
    QPointF m_freeEndPoint;     

    // --- �ڲ�״̬ ---
    QPen m_pen;                 
    bool m_hasArrowHead; 

    // --- ˽�и������� ---
    void updatePen();           
    void drawArrowHead(QPainter& painter, const QPointF& p1, const QPointF& p2) const;
    void drawHandle(QPainter& painter, const QPointF& pos) const; 
};