#include <QRect>
#include <QColor>
#include <QPainter>
#include <QPoint>
#include <QPolygon>
#include <QVector>
#include <cmath>
#include <QDebug>
#include <QUuid>
#include <QJsonObject>
#include <QFont>        
#include <QTextOption> 

// --- ���ӵ������ռ� ---
namespace ConnectionPoint {
    const int Top = 0;
    const int Right = 1;
    const int Bottom = 2;
    const int Left = 3;
    const int Count = 4;
}

// --- ͼ�λ��� ---
class Shape {
protected:
    QRect m_rect;            
    QColor m_borderColor;    
    QColor m_fillColor;      
    qreal m_borderWidth;     
    bool m_isFilled;         
    Qt::PenStyle m_borderStyle; 
    QString m_id;            

public:
    // --- ���캯�� (����) ---
    Shape() : m_borderColor(Qt::black), m_fillColor(Qt::white), m_borderWidth(1.5), m_isFilled(false), m_borderStyle(Qt::SolidLine) { // Ĭ�ϲ���䣬��ɫ���ɫ��1.5�߿��
        m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    // ��Ҫ���캯���������������л�������
    Shape(const QRect& r,
        const QColor& borderColor = Qt::black,
        qreal borderWidth = 1.5,
        bool isFilled = false,
        const QColor& fillColor = Qt::white,
        Qt::PenStyle borderStyle = Qt::SolidLine) // <--- ��� borderStyle ����
        : m_rect(r.normalized()), m_borderColor(borderColor), m_fillColor(fillColor),
        m_borderWidth(borderWidth), m_isFilled(isFilled), m_borderStyle(borderStyle) // <--- ��ʼ�� borderStyle
    {
        m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }

    virtual ~Shape() {} // ����������

    // --- ���麯�� (������������ʵ��) ---
    virtual void draw(QPainter& painter) const = 0; // �����߼�
    virtual bool isSelected(const QPoint& point) const = 0; // ѡ�е�����߼�

    // --- Getters (����) ---
    QRect getRect() const { return m_rect; }
    QColor getBorderColor() const { return m_borderColor; } // ������
    
    Qt::PenStyle getBorderStyle() const { return m_borderStyle; }// <--- ���� Getter
    QColor getFillColor() const { return m_fillColor; }     // <--- ����
    qreal getBorderWidth() const { return m_borderWidth; }   // <--- ����
    bool isFilled() const { return m_isFilled; }            // <--- ����
    QString getId() const { return m_id; }

    // --- Setters (����) ---
    virtual void setRect(const QRect& newRect) { m_rect = newRect.normalized(); }
    void setBorderColor(const QColor& newColor) { m_borderColor = newColor; } // ������
    void setFillColor(const QColor& newColor) { m_fillColor = newColor; }     // <--- ����
    void setBorderWidth(qreal newWidth) { m_borderWidth = qMax(0.1, newWidth); } // <--- ���� (����С�������)
    void setBorderStyle(Qt::PenStyle style) {m_borderStyle = style;}  // <--- ���� Setter
    void setFilled(bool filled) { m_isFilled = filled; }                      // <--- ����
    void setId(const QString& newId) { m_id = newId; }

    // --- �������� ---
    virtual void move(const QPoint& offset) {
        m_rect.translate(offset);
    }

    // --- ���ӵ���ط��� (�������䣬��ʹ�� m_rect) ---
    virtual QPoint getConnectionPoint(int index) const {
        switch (index) {
        case ConnectionPoint::Top:    return QPoint(m_rect.center().x(), m_rect.top());
        case ConnectionPoint::Right:  return QPoint(m_rect.right(), m_rect.center().y());
        case ConnectionPoint::Bottom: return QPoint(m_rect.center().x(), m_rect.bottom());
        case ConnectionPoint::Left:   return QPoint(m_rect.left(), m_rect.center().y());
        default:                      qWarning() << "Shape::getConnectionPoint - ��Ч����:" << index; return m_rect.center();
        }
    }

    virtual int getConnectionPointCount() const {
        return ConnectionPoint::Count;
    }

    // findClosestConnectionPoint (���䣬�ڲ���ʹ�� getConnectionPoint)
    virtual bool findClosestConnectionPoint(const QPoint& globalPos, int& closestIndex, double& minDistanceSq, double maxDistanceThreshold = 10.0) const {
        closestIndex = -1;
        minDistanceSq = maxDistanceThreshold * maxDistanceThreshold + 1.0;
        bool found = false;
        for (int i = 0; i < getConnectionPointCount(); ++i) {
            QPoint cp = getConnectionPoint(i);
            double dx = globalPos.x() - cp.x();
            double dy = globalPos.y() - cp.y();
            double distSq = dx * dx + dy * dy;
            if (distSq < minDistanceSq && distSq <= maxDistanceThreshold * maxDistanceThreshold) {
                minDistanceSq = distSq;
                closestIndex = i;
                found = true;
            }
        }
        return found;
    }

    // drawConnectionPoints (���䣬�ڲ���ʹ�� getConnectionPoint)
    virtual void drawConnectionPoints(QPainter& painter) const {
        painter.save();
        painter.setBrush(QColor(0, 200, 0, 180));
        painter.setPen(QPen(Qt::darkGreen, 1));
        const int radius = 4; // ��Сһ��뾶
        for (int i = 0; i < getConnectionPointCount(); ++i) {
            painter.drawEllipse(getConnectionPoint(i), radius, radius);
        }
        painter.restore();
    }

    // --- ���л� (���£����������) ---
    virtual QJsonObject toJson() const {
        QJsonObject obj;
        // �������ⲿ��ӻ��������� toJson �����
        obj["id"] = m_id;
        obj["x"] = m_rect.x();
        obj["y"] = m_rect.y();
        obj["width"] = m_rect.width();
        obj["height"] = m_rect.height();
        obj["borderColor"] = m_borderColor.name(QColor::HexRgb); // ����߿���ɫ (������key)
        obj["borderStyle"] = static_cast<int>(m_borderStyle); // <--- ���� borderStyle Ϊ����
        obj["borderWidth"] = m_borderWidth;                      // <--- ����
        obj["isFilled"] = m_isFilled;                            // <--- ����
        obj["fillColor"] = m_fillColor.name(QColor::HexRgb);     // <--- ����
        return obj;
    }

    // --- �������л��������� ---
    // ͨ���� DiagramEditor::onOpen �д����������������ṩһ����������
    virtual void loadFromJson(const QJsonObject& obj) {
        // �������ͨ������
        m_id = obj.value("id").toString(QUuid::createUuid().toString(QUuid::WithoutBraces)); 
        int x = obj.value("x").toInt(0);
        int y = obj.value("y").toInt(0);
        int w = obj.value("width").toInt(10);
        int h = obj.value("height").toInt(10);
        m_rect = QRect(x, y, w, h).normalized();

        QColor loadedBorderColor(obj.value("borderColor").toString("black"));
        m_borderColor = loadedBorderColor.isValid() ? loadedBorderColor : Qt::black;

        m_borderWidth = obj.value("borderWidth").toDouble(1.5);
        if (m_borderWidth < 0.1) m_borderWidth = 0.1; // �����Сֵ
        m_borderStyle = static_cast<Qt::PenStyle>(obj.value("borderStyle").toInt(Qt::SolidLine));

        m_isFilled = obj.value("isFilled").toBool(false);

        QColor loadedFillColor(obj.value("fillColor").toString("white"));
        m_fillColor = loadedFillColor.isValid() ? loadedFillColor : Qt::white;
    }
};

// --- �����ࣺ���� (Rectangle) ---
class Rectangle : public Shape {
public:
    // ���캯�����û����¹��캯��
    Rectangle(const QRect& r,
        const QColor& borderColor = Qt::black,
        qreal borderWidth = 1.5,
        bool isFilled = false,
        const QColor& fillColor = Qt::white,
        Qt::PenStyle borderStyle = Qt::SolidLine)
        : Shape(r, borderColor, borderWidth, isFilled, fillColor) {}

    // draw ʵ�ָ��£�ʹ��������
    void draw(QPainter& painter) const override {
        painter.save();
        // ���û��� (�߿�)
        QPen pen(m_borderColor);
       // pen.setWidthF(m_borderWidth);
        if (qFuzzyCompare(m_borderWidth, 0)) {
            pen.setStyle(Qt::NoPen); // �ޱ߿�
        }
        else {
            pen.setWidthF(m_borderWidth);
            pen.setStyle(m_borderStyle); // <--- Ӧ�ñ߿���ʽ
        }


        painter.setPen(pen);

        // ���û�ˢ (���)
        if (m_isFilled) {
            painter.setBrush(m_fillColor);
        }
        else {
            painter.setBrush(Qt::NoBrush);
        }

        // ���ƾ���
        painter.drawRect(m_rect);
        painter.restore();
    }

    // isSelected (����)
    bool isSelected(const QPoint& point) const override {
        // ��΢����ѡ�з�Χ�������߿�
        qreal expand = qMax(3.0, m_borderWidth / 2.0 + 1.0);
        return m_rect.adjusted(-expand, -expand, expand, expand).contains(point);
    }
};

// --- �����ࣺԲ�� (Circle / Ellipse) ---
class Circle : public Shape {
public:
    // ���캯�����û����¹��캯��
    Circle(const QRect& r,
        const QColor& borderColor = Qt::black,
        qreal borderWidth = 1.5,
        bool isFilled = false,
        const QColor& fillColor = Qt::white,Qt::PenStyle borderStyle = Qt::SolidLine)
        : Shape(r, borderColor, borderWidth, isFilled, fillColor) {}

    // draw ʵ�ָ��£�ʹ��������
    void draw(QPainter& painter) const override {
        painter.save();
        QPen pen(m_borderColor);
        //pen.setWidthF(m_borderWidth);
        if (qFuzzyCompare(m_borderWidth, 0)) {
            pen.setStyle(Qt::NoPen); // �ޱ߿�
        }
        else {
            pen.setWidthF(m_borderWidth);
            pen.setStyle(m_borderStyle); // <--- Ӧ�ñ߿���ʽ
        }


        painter.setPen(pen);
        
        if (m_isFilled) {
            painter.setBrush(m_fillColor);
        }
        else {
            painter.setBrush(Qt::NoBrush);
        }

        painter.drawEllipse(m_rect); // ������Բ
        painter.restore();
    }

    // isSelected (����)
    bool isSelected(const QPoint& point) const override {
        QPoint center = m_rect.center();
        double a = m_rect.width() / 2.0;
        double b = m_rect.height() / 2.0;

        if (a < 1e-6 || b < 1e-6) {
            return m_rect.contains(point);
        }

        double dx = point.x() - center.x();
        double dy = point.y() - center.y();
        // ����һ���ݲ�����߿�
        double borderFactor = 1.0 + qMax(0.05, m_borderWidth / qMin(a, b));
        return (dx * dx / (a * a) + dy * dy / (b * b)) <= borderFactor * borderFactor;
    }
};

// --- �����ࣺ������ (Triangle) ---
class Triangle : public Shape {
public:
    // ���캯�����û����¹��캯��
    Triangle(const QRect& r,
        const QColor& borderColor = Qt::black,
        qreal borderWidth = 1.5,
        bool isFilled = false,
        const QColor& fillColor = Qt::white,
        Qt::PenStyle borderStyle = Qt::SolidLine)
        : Shape(r, borderColor, borderWidth, isFilled, fillColor) {}

    // getPolygon (����, ʹ�� m_rect)
    QPolygon getPolygon() const {
        QPolygon trianglePoly;
        trianglePoly << QPoint(m_rect.center().x(), m_rect.top())
            << m_rect.bottomLeft()
            << m_rect.bottomRight();
        return trianglePoly;
    }

    // draw ʵ�ָ���
    void draw(QPainter& painter) const override {
        painter.save();
        QPen pen(m_borderColor);
        //pen.setWidthF(m_borderWidth);
        if (qFuzzyCompare(m_borderWidth, 0)) {
            pen.setStyle(Qt::NoPen); // �ޱ߿�
        }
        else {
            pen.setWidthF(m_borderWidth);
            pen.setStyle(m_borderStyle); // <--- Ӧ�ñ߿���ʽ
        }


        painter.setPen(pen);
       

        if (m_isFilled) {
            painter.setBrush(m_fillColor);
        }
        else {
            painter.setBrush(Qt::NoBrush);
        }

        painter.drawPolygon(getPolygon()); // ���ƶ����
        painter.restore();
    }

    // isSelected (����)
    bool isSelected(const QPoint& point) const override {
        // ������΢���� Polygon �����жϣ�����ʹ�õ��߽�ľ���
        return getPolygon().containsPoint(point, Qt::OddEvenFill);
        // ����ȷ�Ĵ��߿��жϻḴ��
    }

    // getConnectionPoint (����, ʹ�� getPolygon)
    QPoint getConnectionPoint(int index) const override {
        QPolygon poly = getPolygon();
        if (poly.size() < 3) return m_rect.center();
        switch (index) {
        case ConnectionPoint::Top:    return poly.at(0);
        case ConnectionPoint::Right:  return QPoint((poly.at(0).x() + poly.at(2).x()) / 2, (poly.at(0).y() + poly.at(2).y()) / 2);
        case ConnectionPoint::Bottom: return QPoint((poly.at(1).x() + poly.at(2).x()) / 2, poly.at(1).y());
        case ConnectionPoint::Left:   return QPoint((poly.at(0).x() + poly.at(1).x()) / 2, (poly.at(0).y() + poly.at(1).y()) / 2);
        default: return m_rect.center();
        }
    }
};

// --- �����ࣺ���� (Diamond) ---
class Diamond : public Shape {
public:
    // ���캯�����û����¹��캯��
    Diamond(const QRect& r,
        const QColor& borderColor = Qt::black,
        qreal borderWidth = 1.5,
        bool isFilled = false,
        const QColor& fillColor = Qt::white,
        Qt::PenStyle borderStyle = Qt::SolidLine)
        : Shape(r, borderColor, borderWidth, isFilled, fillColor) {}

    // getPolygon (����, ʹ�� m_rect)
    QPolygon getPolygon() const {
        QPolygon diamondPoly;
        diamondPoly << QPoint(m_rect.center().x(), m_rect.top())
            << QPoint(m_rect.right(), m_rect.center().y())
            << QPoint(m_rect.center().x(), m_rect.bottom())
            << QPoint(m_rect.left(), m_rect.center().y());
        return diamondPoly;
    }

    // draw ʵ�ָ���
    void draw(QPainter& painter) const override {
        painter.save();
        QPen pen(m_borderColor);
        //pen.setWidthF(m_borderWidth);
        if (qFuzzyCompare(m_borderWidth, 0)) {
            pen.setStyle(Qt::NoPen); // �ޱ߿�
        }
        else {
            pen.setWidthF(m_borderWidth);
            pen.setStyle(m_borderStyle); // <--- Ӧ�ñ߿���ʽ
        }


        painter.setPen(pen);

        if (m_isFilled) {
            painter.setBrush(m_fillColor);
        }
        else {
            painter.setBrush(Qt::NoBrush);
        }

        painter.drawPolygon(getPolygon()); // �������ζ����
        painter.restore();
    }

    // isSelected (����)
    bool isSelected(const QPoint& point) const override {
        return getPolygon().containsPoint(point, Qt::OddEvenFill);
    }

    // getConnectionPoint (ʹ�û���Ĭ�ϻ���дΪ����)
    // ����������дΪ����
    QPoint getConnectionPoint(int index) const override {
        QPolygon poly = getPolygon();
        if (poly.size() < 4) return m_rect.center();
        switch (index) {
        case ConnectionPoint::Top:    return poly.at(0); // �϶���
        case ConnectionPoint::Right:  return poly.at(1); // �Ҷ���
        case ConnectionPoint::Bottom: return poly.at(2); // �¶���
        case ConnectionPoint::Left:   return poly.at(3); // �󶥵�
        default: return m_rect.center();
        }
    }
};

// --- �����ࣺԲ�Ǿ��� (RoundedRectangle) ---
class RoundedRectangle : public Shape {
private:
    int cornerRadiusX;
    int cornerRadiusY;

public:
    // ���캯������
    RoundedRectangle(const QRect& r,
        const QColor& borderColor = Qt::black,
        qreal borderWidth = 1.5,
        bool isFilled = false,
        const QColor& fillColor = Qt::white,
        Qt::PenStyle borderStyle = Qt::SolidLine, // <--- �����ʽ
        int radius = 15) // Բ�ǰ뾶��������
        : Shape(r, borderColor, borderWidth, isFilled, fillColor)
    {
        setCornerRadius(radius); // ��ʼ��Բ��
    }

    // setCornerRadius (����, ʹ�� m_rect)
    void setCornerRadius(int radius) {
        cornerRadiusX = qMin(radius, m_rect.width() / 2);
        cornerRadiusY = qMin(radius, m_rect.height() / 2);
        if (cornerRadiusX < 0) cornerRadiusX = 0;
        if (cornerRadiusY < 0) cornerRadiusY = 0;
    }
    int getCornerRadius() const { return qMax(cornerRadiusX, cornerRadiusY); } // Getter for radius

    // ��д setRect ����Բ������ (����)
    void setRect(const QRect& newRect) override {
        Shape::setRect(newRect);
        setCornerRadius(qMax(cornerRadiusX, cornerRadiusY));
    }



    // draw ʵ�ָ���
    void draw(QPainter& painter) const override {
        painter.save();
        QPen pen(m_borderColor);
        //pen.setWidthF(m_borderWidth);
        if (qFuzzyCompare(m_borderWidth, 0)) {
            pen.setStyle(Qt::NoPen); // �ޱ߿�
        }
        else {
            pen.setWidthF(m_borderWidth);
            pen.setStyle(m_borderStyle); // <--- Ӧ�ñ߿���ʽ
        }
        painter.setPen(pen);


        if (m_isFilled) {
            painter.setBrush(m_fillColor);
        }
        else {
            painter.setBrush(Qt::NoBrush);
        }

        painter.drawRoundedRect(m_rect, cornerRadiusX, cornerRadiusY);
        painter.restore();
    }

    // isSelected (����)
    bool isSelected(const QPoint& point) const override {
        qreal expand = qMax(3.0, m_borderWidth / 2.0 + 1.0);
        return m_rect.adjusted(-expand, -expand, expand, expand).contains(point);
        // ����ȷ���жϻḴ��
    }

    // --- ���л�/�����л� (��ѡ��ǿ������ radius) ---
    QJsonObject toJson() const override {
        QJsonObject obj = Shape::toJson(); // ���û��෽��
        obj["type"] = "rounded_rectangle"; // �������
        obj["cornerRadius"] = qMax(cornerRadiusX, cornerRadiusY); // ����Բ�ǰ뾶
        return obj;
    }
    void loadFromJson(const QJsonObject& obj) override {
        Shape::loadFromJson(obj); // ���ػ�������
        setCornerRadius(obj.value("cornerRadius").toInt(15)); // ����Բ�ǰ뾶
    }
};

// --- �����ࣺƽ���ı��� (Parallelogram) ---
class Parallelogram : public Shape {
private:
    int xOffset;

public:
    // ���캯������
    Parallelogram(const QRect& r,
        const QColor& borderColor = Qt::black,
        qreal borderWidth = 1.5,
        bool isFilled = false,
        const QColor& fillColor = Qt::white,
        Qt::PenStyle borderStyle = Qt::SolidLine, // <--- �����ʽ
        int offset = 20) // ƫ������������
        : Shape(r, borderColor, borderWidth, isFilled, fillColor)
    {
        setXOffset(offset);
    }

    // setXOffset (����, ʹ�� m_rect)
    void setXOffset(int offset) {
        xOffset = qBound(-m_rect.width() / 2 + 1, offset, m_rect.width() / 2 - 1);
    }
    int getXOffset() const { return xOffset; } // Getter for offset

    // ��д setRect ����ƫ�������� (����)
    void setRect(const QRect& newRect) override {
        Shape::setRect(newRect);
        setXOffset(xOffset);
    }

    // getPolygon (����, ʹ�� m_rect �� xOffset)
    QPolygon getPolygon() const {
        QPolygon parallelogramPoly;
        parallelogramPoly << QPoint(m_rect.left() + xOffset, m_rect.top())
            << QPoint(m_rect.right() + xOffset, m_rect.top()) // ���������Ͻ�ҲӦƫ��
            << QPoint(m_rect.right() - xOffset, m_rect.bottom())
            << QPoint(m_rect.left() - xOffset, m_rect.bottom()); // ���������½�ҲӦƫ��
        // �����������getPolygonʵ�ָ������Σ�ƽ���ı���Ӧ���ǶԱ�ƽ��
        // ����ʵ��ƽ���ı��εĶ������
        parallelogramPoly.clear();
        int effectiveOffset = xOffset; // ���Ի��� m_rect.height() �����㣿 ���� xOffset
        parallelogramPoly << m_rect.topLeft() + QPoint(effectiveOffset, 0) // ���Ͻ�����ƫ
            << m_rect.topRight() // ���Ͻǲ��� (����Ҳƫ�ƣ�ȡ���ڶ���)
            << m_rect.bottomRight() - QPoint(effectiveOffset, 0) // ���½�����ƫ
            << m_rect.bottomLeft(); // ���½ǲ���
// ��һ�ֶ��壺��������ƫ���ײ�����ƫ
// parallelogramPoly.clear();
// parallelogramPoly << m_rect.topLeft() + QPoint(xOffset, 0)
//                   << m_rect.topRight() + QPoint(xOffset, 0)
//                   << m_rect.bottomRight()
//                   << m_rect.bottomLeft();

        return parallelogramPoly;
    }

    // draw ʵ�ָ���
    void draw(QPainter& painter) const override {
        painter.save();
        QPen pen(m_borderColor);
        //pen.setWidthF(m_borderWidth);
        if (qFuzzyCompare(m_borderWidth, 0)) {
            pen.setStyle(Qt::NoPen); // �ޱ߿�
        }
        else {
            pen.setWidthF(m_borderWidth);
            pen.setStyle(m_borderStyle); // <--- Ӧ�ñ߿���ʽ
        }
        painter.setPen(pen);

        if (m_isFilled) {
            painter.setBrush(m_fillColor);
        }
        else {
            painter.setBrush(Qt::NoBrush);
        }

        painter.drawPolygon(getPolygon()); // ����ƽ���ı���
        painter.restore();
    }

    // isSelected (����)
    bool isSelected(const QPoint& point) const override {
        return getPolygon().containsPoint(point, Qt::OddEvenFill);
    }

    // getConnectionPoint (ʹ��������� getPolygon)
    QPoint getConnectionPoint(int index) const override {
        QPolygon poly = getPolygon();
        if (poly.size() < 4) return m_rect.center();
        switch (index) {
        case ConnectionPoint::Top:    return QPoint((poly.at(0).x() + poly.at(1).x()) / 2, poly.at(0).y()); // �ϱ��е�
        case ConnectionPoint::Right:  return QPoint((poly.at(1).x() + poly.at(2).x()) / 2, (poly.at(1).y() + poly.at(2).y()) / 2); // �ұ��е�
        case ConnectionPoint::Bottom: return QPoint((poly.at(2).x() + poly.at(3).x()) / 2, poly.at(2).y()); // �±��е�
        case ConnectionPoint::Left:   return QPoint((poly.at(3).x() + poly.at(0).x()) / 2, (poly.at(3).y() + poly.at(0).y()) / 2); // ����е�
        default: return m_rect.center();
        }
    }
    // --- ���л�/�����л� (��ѡ��ǿ������ xOffset) ---
    QJsonObject toJson() const override {
        QJsonObject obj = Shape::toJson();
        obj["type"] = "parallelogram";
        obj["xOffset"] = xOffset; // ����ƫ����
        return obj;
    }
    void loadFromJson(const QJsonObject& obj) override {
        Shape::loadFromJson(obj);
        setXOffset(obj.value("xOffset").toInt(20)); // ����ƫ����
    }
};


// --- �����ࣺ�ı��� (TextBox) ---
class TextBox : public Shape {
private:
    QString m_text;
    QFont   m_font;
    QColor  m_textColor;
    QTextOption m_textOption; // �������롢����
public:
    // ���캯�����£�������������
    TextBox(const QRect& r,
        const QColor& borderColor = Qt::transparent, // Ĭ�ϱ߿�ɫ
        qreal borderWidth = 0.0,                 // Ĭ�ϱ߿���
        bool isFilled = false,                   // Ĭ�ϲ����
        const QColor& fillColor = Qt::white,    // Ĭ�����ɫ
        Qt::PenStyle borderStyle = Qt::SolidLine,
        const QString& text = QObject::tr("Double-click to edit"),
        const QFont& font = QFont("Arial", 10), // Ĭ������
        const QColor& textColor = Qt::black,     // Ĭ���ı���ɫ
        Qt::Alignment alignment = Qt::AlignCenter) // Ĭ�϶���
        : Shape(r, borderColor, borderWidth, isFilled, fillColor), // ���û��๹��
        m_text(text), m_font(font), m_textColor(textColor)
    {
        m_textOption.setAlignment(alignment);
        m_textOption.setWrapMode(QTextOption::WordWrap); // Ĭ���Զ�����
    }

    // --- �ı���� Getters/Setters (���/ȷ�ϴ���) ---
    void setText(const QString& text) { m_text = text; }
    QString getText() const { return m_text; }
    void setFont(const QFont& font) { m_font = font; }
    QFont getFont() const { return m_font; }
    void setTextColor(const QColor& color) { m_textColor = color; }
    QColor getTextColor() const { return m_textColor; }
    void setTextAlignment(Qt::Alignment alignment) { m_textOption.setAlignment(alignment); }
    Qt::Alignment getTextAlignment() const { return m_textOption.alignment(); }

    // --- ��д draw (ʹ�������Ի��Ʊ߿�ͱ���) ---
    void draw(QPainter& painter) const override {
        painter.save();
        // 1. ���Ʊ߿�ͱ��� (ʹ�û�������)
        QPen pen(m_borderColor);
        //pen.setWidthF(m_borderWidth);
        if (qFuzzyCompare(m_borderWidth, 0)) {
            pen.setStyle(Qt::NoPen); // �ޱ߿�
        }
        else {
            pen.setWidthF(m_borderWidth);
            pen.setStyle(m_borderStyle); // <--- Ӧ�ñ߿���ʽ
        }
        painter.setPen(pen);
        if (m_isFilled) {
            painter.setBrush(m_fillColor); // ʹ�����ɫ
        }
        else {
            painter.setBrush(Qt::NoBrush);  // ͸������
        }
        // ���ƾ�����Ϊ�����ͱ߿�
        painter.drawRect(m_rect);
        // 2. �����ı� (ʹ���ı�����)
        painter.setPen(m_textColor);   // �����ı���ɫ
        painter.setFont(m_font);       // ��������
        // �ھ����ڲ������ı��������߾ࣨ�߾���Կ����߿�
        int margin = qMax(2, int(m_borderWidth / 2.0) + 1);
        QRect textRect = m_rect.adjusted(margin, margin, -margin, -margin);
        if (textRect.isValid()) {
            painter.drawText(textRect, m_text, m_textOption);
        }
        painter.restore();
    }

    // isSelected (����)
    bool isSelected(const QPoint& point) const override {
        qreal expand = qMax(2.0, m_borderWidth / 2.0); // ����ѡ�з�Χ
        return m_rect.adjusted(-expand, -expand, expand, expand).contains(point);
    }

    // --- ���л� (���û��� toJson ������ı�����) ---
    QJsonObject toJson() const override {
        QJsonObject obj = Shape::toJson(); // ��ȡ��������(ID, rect, border, fill)
        obj["type"] = "textbox";           // ���ͱ�ʶ
        obj["text"] = m_text;
        obj["textColor"] = m_textColor.name(QColor::HexRgb);
        obj["fontFamily"] = m_font.family();
        obj["fontSize"] = m_font.pointSize();
        obj["fontBold"] = m_font.bold();
        obj["fontItalic"] = m_font.italic();
        obj["textAlignment"] = static_cast<int>(m_textOption.alignment()); // ������뷽ʽ
        // ������Ӹ�����������...
        return obj;
    }

    // --- �����л� (���û��� loadFromJson �������ı�����) ---
    void loadFromJson(const QJsonObject& obj) override {
        Shape::loadFromJson(obj); // ���ػ�������
        m_text = obj.value("text").toString(QObject::tr("Text"));
        QColor loadedTextColor(obj.value("textColor").toString("black"));
        m_textColor = loadedTextColor.isValid() ? loadedTextColor : Qt::black;
        // ��������
        QString family = obj.value("fontFamily").toString("Arial");
        int size = obj.value("fontSize").toInt(10);
        bool bold = obj.value("fontBold").toBool(false);
        bool italic = obj.value("fontItalic").toBool(false);
        m_font = QFont(family, size, bold ? QFont::Bold : QFont::Normal, italic);
        // ���ض���
        int alignInt = obj.value("textAlignment").toInt(static_cast<int>(Qt::AlignCenter));
        m_textOption.setAlignment(static_cast<Qt::Alignment>(alignInt));
    }
};