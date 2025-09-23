#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <qDebug>
#include <QApplication>
#include <QIcon>
#include <QButtonGroup> // ���ڰ�ť�������
#include <QLabel>

// --- �Զ������ק��ť�� (������״) ---
class DraggableButton : public QPushButton {
    Q_OBJECT
public:
    DraggableButton(int shapeType, const QIcon& icon, const QString& text, QWidget* parent = nullptr)
        : QPushButton(icon, text, parent), m_shapeType(shapeType) {}

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            m_dragStartPosition = event->pos();
        }
        QPushButton::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if (!(event->buttons() & Qt::LeftButton)) return;
        if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance()) return;

        QDrag* drag = new QDrag(this);
        QMimeData* mimeData = new QMimeData;
        mimeData->setData("application/x-shapetype", QByteArray::number(m_shapeType));
        drag->setMimeData(mimeData);
        drag->setPixmap(this->icon().pixmap(100, 70)); // ����ͼ���С���ɸ����������
        drag->setHotSpot(QPoint(32, 32));
        drag->exec(Qt::CopyAction);
    }

private:
    int m_shapeType;    
    QPoint m_dragStartPosition;
};


// --- ��������� ---
class ToolPanel : public QWidget {
    Q_OBJECT
public:
    ToolPanel(QWidget* parent = nullptr) : QWidget(parent), toolGroup(new QButtonGroup(this)) {
        initUI();
    }
    void uncheckAllTools() {
        foreach(QAbstractButton * button, toolGroup->buttons()) {
            button->setChecked(false);
        }
    }
    // <<<--- ������ѡ�С�ѡ�񡱹��ߵķ��� --->
    void selectSelectionTool() {
        if (btnSelection) { // ȷ����ť�Ѵ���
            uncheckAllTools(); // ������Ҫ��������� uncheckAllTools����Ϊ setChecked(true) �� exclusive ģʽ�»��Զ�ȡ��������
            qDebug() << "ToolPanel: Selecting Selection Tool";
            btnSelection->setChecked(true); // ����Ϊѡ�� (exclusive ģʽ�»��Զ�ȡ������)
        }
    }

signals:
    void shapeToolSelected(int shapeType);
    void connectionToolActivated();
    void toSelectModel();

private:
    QButtonGroup* toolGroup; 
    QPushButton* btnSelection;

    enum ShapeType {
        RECTANGLE = 0,
        CIRCLE = 1,
        TRIANGLE = 2,
        DIAMOND = 3,           
        ROUNDED_RECTANGLE = 4, 
        PARALLELOGRAM = 5,   
        TEXT_BOX = 6

    };
    enum ToolType {
        SELECTION, 
        SHAPE,
        CONNECTOR
    };
    void initUI() {
        setStyleSheet("QWidget { background-color: #1E293B; border: none; }"); // ������屳��
        QVBoxLayout* layout = new QVBoxLayout(this);
        QString path = QCoreApplication::applicationDirPath();

        layout->setContentsMargins(10, 10, 10, 10);
        layout->setSpacing(20);

        toolGroup->setExclusive(false); 
        // ------------------------- ��ͷѡ�񹤾߰�ť -------------------------
        btnSelection = createToolButton(path+"/icons/select_arrow2.svg", tr("Select"), tr("Select elements on the canvas"));
        btnSelection->setChecked(true);
        connect(btnSelection, &QPushButton::clicked, [this]() {
            handleButtonToggle(btnSelection, SELECTION);
            });
        layout->addWidget(btnSelection);
        toolGroup->addButton(btnSelection); // ��ӵ���ť��
        // ------------------------- �����߹��߰�ť -------------------------
        QPushButton* btnConnector = createToolButton(path + "/icons/line1.svg", tr("Connector"), tr("Drag on the canvas to create a connection line after clicking"));
        connect(btnConnector, &QPushButton::clicked, [this, btnConnector]() {
            handleButtonToggle(btnConnector, CONNECTOR);
            });
        layout->addWidget(btnConnector);
        toolGroup->addButton(btnConnector); 

        // ------------------------- ���ΰ�ť -------------------------
        DraggableButton* btnRectangle = createShapeButton(RECTANGLE, path + "/icons/rect.svg", tr("Rectangle"));
        connect(btnRectangle, &QPushButton::clicked, [this, btnRectangle]() {
            handleButtonToggle(btnRectangle, SHAPE, RECTANGLE);
            });
        layout->addWidget(btnRectangle);
        toolGroup->addButton(btnRectangle); 

        // ------------------------- Բ�ΰ�ť -------------------------
        DraggableButton* btnCircle = createShapeButton(CIRCLE, path + "/icons/circle2.svg", tr("Circle"));
        connect(btnCircle, &QPushButton::clicked, [this, btnCircle]() {
            handleButtonToggle(btnCircle, SHAPE, CIRCLE);
            });
        layout->addWidget(btnCircle);
        toolGroup->addButton(btnCircle); 

        // ------------------------- �����ΰ�ť -------------------------
        DraggableButton* btnTriangle = createShapeButton(TRIANGLE, path + "/icons/triangle.svg", tr("Triangle"));
        connect(btnTriangle, &QPushButton::clicked, [this, btnTriangle]() {
            handleButtonToggle(btnTriangle, SHAPE, TRIANGLE);
            });
        layout->addWidget(btnTriangle);
        toolGroup->addButton(btnTriangle); 
        // ------------------------ - ���ΰ�ť------------------------ -
        DraggableButton* btnDiamond = createShapeButton(DIAMOND, path + "/icons/diamond.svg", tr("Diamond"));
        connect(btnDiamond, &QPushButton::clicked, [this, btnDiamond]() {
            handleButtonToggle(btnDiamond, SHAPE, DIAMOND);
            });
        layout->addWidget(btnDiamond);
        toolGroup->addButton(btnDiamond); 

        // ------------------------- Բ�Ǿ��ΰ�ť -------------------------
        DraggableButton* btnRoundedRectangle = createShapeButton(ROUNDED_RECTANGLE, path + "/icons/rounded_rect.svg", tr("Rounded Rectangle"));
        connect(btnRoundedRectangle, &QPushButton::clicked, [this, btnRoundedRectangle]() {
            handleButtonToggle(btnRoundedRectangle, SHAPE, ROUNDED_RECTANGLE);
            });
        layout->addWidget(btnRoundedRectangle);
        toolGroup->addButton(btnRoundedRectangle); 

        // ------------------------- ƽ���ı��ΰ�ť -------------------------
        DraggableButton* btnParallelogram = createShapeButton(PARALLELOGRAM, path + "/icons/parallelogram.svg", tr("Parallelogram"));
        connect(btnParallelogram, &QPushButton::clicked, [this, btnParallelogram]() {
            handleButtonToggle(btnParallelogram, SHAPE, PARALLELOGRAM);
            });
        layout->addWidget(btnParallelogram);
        toolGroup->addButton(btnParallelogram); 

        // ------------------------- �ı���ť -------------------------
        DraggableButton* btnTextBox = createShapeButton(TEXT_BOX, path + "/icons/textbox.svg", tr("Text Box"));
        connect(btnTextBox, &QPushButton::clicked, [this, btnTextBox]() {
            handleButtonToggle(btnTextBox, SHAPE, TEXT_BOX);
            });
        layout->addWidget(btnTextBox);
        toolGroup->addButton(btnTextBox); 


        layout->addStretch();
        setLayout(layout);
    }


    void handleButtonToggle(QAbstractButton* clickedButton, ToolType type, int shapeType = -1) {
        // ����ǰ��ť��ѡ��
        if (clickedButton->isChecked()) {
            // ȡ��������ť��ѡ��״̬
            foreach(QAbstractButton * button, toolGroup->buttons()) {
                if (button != clickedButton) {
                    button->setChecked(false);
                }
            }
        }
        switch (type) {
        case SELECTION:
            emit toSelectModel();
            break;
        case SHAPE:
            emit shapeToolSelected(shapeType);
            break;
        case CONNECTOR:
            emit connectionToolActivated();
            break;
        default:
            break;
        }
    }
    // ����ͨ�ù��߰�ť�������߰�ť��
    QPushButton* createToolButton(const QString& iconPath, const QString& text, const QString& toolTip) {
        //QPushButton* button = new QPushButton(QIcon(iconPath), text, this);
        QPushButton* button = new QPushButton(QIcon(iconPath), "", this);
        button->setToolTip(toolTip);
        button->setCheckable(true); // ��ѡ��״̬
        buttShapeSet(button);
        return button;
    }

    // ������״��ť���̳��� DraggableButton��
    DraggableButton* createShapeButton(int shapeType, const QString& iconPath, const QString& text) {
        DraggableButton* button = new DraggableButton(shapeType, QIcon(iconPath), "", this);
        button->setToolTip(text);
        button->setCheckable(true); // ��ѡ��״̬
        buttShapeSet(button);
        return button;
    }

    // ���ð�ť��ʽ
    void buttShapeSet(QPushButton* button) {
        button->setIconSize(QSize(60, 60));
        button->setFixedSize(60, 60);
        button->setStyleSheet(
            "QPushButton {"
            "    background-color: transparent;"
            "    border: none;"
            "    border-radius: 4px;"
            "    padding: 6px;"
            "    color: #E2E8F0;"
            "}"
            "QPushButton:hover {"
            "    background-color: #28354C;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #334155;"
            "}"
            "QPushButton:checked {"
            "    background-color: #3B82F6;"
            "    color: #FFFFFF;"
            "}"
        );
    }
};