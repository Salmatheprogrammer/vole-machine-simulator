#include "MainWindow.h"

#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDialog>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QShortcut>
#include <QStyle>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <exception>
#include <QStyledItemDelegate>
#include <QTemporaryFile>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextEdit>
#include <QSignalBlocker>
#include <QTextStream>

static const int  kMaxSteps = 100000;      // safety limit so an endless loop can't freeze the window

// editor used when a memory / register cell is double-clicked: hex digits only, text pre-selected
class HexDelegate : public QStyledItemDelegate {
public:
    explicit HexDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const override {
        auto* e = new QLineEdit(parent);
        e->setMaxLength(2);
        e->setAlignment(Qt::AlignCenter);
        e->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9A-Fa-f]{0,2}"), e));
        return e;
    }
    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        QStyledItemDelegate::setEditorData(editor, index);
        if (auto* e = qobject_cast<QLineEdit*>(editor)) e->selectAll();
    }
};

// ---------------------------------------------------------------- constructor
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Vole Machine Simulator");
    resize(1320, 920);
    setMinimumSize(1120, 800);

    buildUi();
    applyStyle();
    refreshAll();
    setStatus("Ready. Choose a program file, enter an even start address and press Load Program.");
}

// ---------------------------------------------------------------- helpers
QString MainWindow::hex2(int value) const {
    return QString("%1").arg(value & 0xFF, 2, 16, QChar('0')).toUpper();
}

void MainWindow::setStatus(const QString& text, bool isError) {
    statusLabel->setText(text);
    statusLabel->setProperty("error", isError);
    statusLabel->style()->unpolish(statusLabel);
    statusLabel->style()->polish(statusLabel);
}

// ---------------------------------------------------------------- build the window
void MainWindow::buildUi() {
    auto* central = new QWidget(this);
    central->setObjectName("central");
    setCentralWidget(central);

    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(14, 12, 14, 0);
    root->setSpacing(10);

    // ---------- header (title on the left, Help button on the right)
    auto* header   = new QHBoxLayout;
    auto* titleBox = new QVBoxLayout;
    auto* title    = new QLabel("VOLE MACHINE SIMULATOR");
    title->setObjectName("title");
    auto* sub = new QLabel("256 memory cells  |  16 registers  |  PC  |  IR  |  screen");
    sub->setObjectName("subtitle");
    titleBox->setSpacing(0);
    titleBox->addWidget(title);
    titleBox->addWidget(sub);

    helpBtn = new QPushButton("?  Help");
    helpBtn->setCursor(Qt::PointingHandCursor);
    helpBtn->setObjectName("ghost");
    themeBtn = new QPushButton("Light Mode");
    themeBtn->setCursor(Qt::PointingHandCursor);
    themeBtn->setObjectName("ghost");
    themeBtn->setToolTip("Switch between dark and light theme");

    header->addLayout(titleBox);
    header->addStretch();
    header->addWidget(themeBtn);
    header->addWidget(helpBtn);
    root->addLayout(header);

    // ---------- body = left controls | memory | right (cpu, registers, screen)
    auto* body = new QHBoxLayout;
    body->setSpacing(12);

    // ===== LEFT PANEL
    auto* left    = new QWidget;
    left->setFixedWidth(300);
    auto* leftLay = new QVBoxLayout(left);
    leftLay->setContentsMargins(0, 0, 0, 0);
    leftLay->setSpacing(8);

    // --- load program
    auto* progBox = new QGroupBox("Load Program");
    auto* progLay = new QGridLayout(progBox);
    fileEdit = new QLineEdit;
    fileEdit->setPlaceholderText("program file (.txt)");
    browseBtn = new QPushButton("Browse...");
    browseBtn->setCursor(Qt::PointingHandCursor);
    startEdit = new QLineEdit("00");
    startEdit->setMaxLength(2);
    startEdit->setValidator(new QRegularExpressionValidator(
        QRegularExpression("[0-9A-Fa-f]{0,2}"), this));
    loadBtn = new QPushButton("Load Program");
    loadBtn->setObjectName("primary");
    loadBtn->setCursor(Qt::PointingHandCursor);

    progLay->addWidget(new QLabel("File"), 0, 0);
    progLay->addWidget(fileEdit, 0, 1);
    progLay->addWidget(browseBtn, 1, 1);
    progLay->addWidget(new QLabel("Start (hex)"), 2, 0);
    progLay->addWidget(startEdit, 2, 1);
    progLay->addWidget(loadBtn, 3, 0, 1, 2);
    leftLay->addWidget(progBox);

    // --- execution
    auto* execBox = new QGroupBox("Execution");
    auto* execLay = new QVBoxLayout(execBox);
    runBtn   = new QPushButton("Run All");
    stepBtn  = new QPushButton("Step  (Enter)");
    resetBtn = new QPushButton("Reset All");
    runBtn->setObjectName("primary");
    resetBtn->setObjectName("danger");
    for (QPushButton* b : {runBtn, stepBtn, resetBtn}) {
        b->setCursor(Qt::PointingHandCursor);
        execLay->addWidget(b);
    }
    leftLay->addWidget(execBox);

    // --- modify memory
    auto* mBox = new QGroupBox("Modify Memory Cell");
    auto* mLay = new QGridLayout(mBox);
    memAddrEdit = new QLineEdit;
    memAddrEdit->setPlaceholderText("00 - FF");
    memAddrEdit->setMaxLength(2);
    memValEdit = new QLineEdit;
    memValEdit->setPlaceholderText("00 - FF");
    memValEdit->setMaxLength(2);
    writeMemBtn = new QPushButton("Write to Memory");
    writeMemBtn->setCursor(Qt::PointingHandCursor);
    mLay->addWidget(new QLabel("Address"), 0, 0);
    mLay->addWidget(memAddrEdit, 0, 1);
    mLay->addWidget(new QLabel("Value"), 1, 0);
    mLay->addWidget(memValEdit, 1, 1);
    mLay->addWidget(writeMemBtn, 2, 0, 1, 2);
    leftLay->addWidget(mBox);

    // --- modify register
    auto* rBox = new QGroupBox("Modify Register");
    auto* rLay = new QGridLayout(rBox);
    regNumEdit = new QLineEdit;
    regNumEdit->setPlaceholderText("0 - F");
    regNumEdit->setMaxLength(1);
    regValEdit = new QLineEdit;
    regValEdit->setPlaceholderText("00 - FF");
    regValEdit->setMaxLength(2);
    writeRegBtn = new QPushButton("Write to Register");
    writeRegBtn->setCursor(Qt::PointingHandCursor);
    rLay->addWidget(new QLabel("Register"), 0, 0);
    rLay->addWidget(regNumEdit, 0, 1);
    rLay->addWidget(new QLabel("Value"), 1, 0);
    rLay->addWidget(regValEdit, 1, 1);
    rLay->addWidget(writeRegBtn, 2, 0, 1, 2);
    leftLay->addWidget(rBox);

    leftLay->addStretch();

    // only hex digits in the modify boxes
    auto* hexVal = new QRegularExpressionValidator(QRegularExpression("[0-9A-Fa-f]{0,2}"), this);
    memAddrEdit->setValidator(hexVal);
    memValEdit->setValidator(hexVal);
    regValEdit->setValidator(hexVal);
    regNumEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9A-Fa-f]{0,1}"), this));

    // ===== CENTER: memory matrix
    auto* memBox = new QGroupBox("Main Memory  (256 cells, hex)");
    auto* memLay = new QVBoxLayout(memBox);
    memTable = new QTableWidget(16, 16);
    QStringList cols, rows;
    for (int i = 0; i < 16; ++i) {
        cols << QString::number(i, 16).toUpper();
        rows << QString::number(i, 16).toUpper();
    }
    memTable->setHorizontalHeaderLabels(cols);
    memTable->setVerticalHeaderLabels(rows);
    memTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    memTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    memTable->setEditTriggers(QAbstractItemView::DoubleClicked);   // double-click a cell to edit it
    memTable->setSelectionMode(QAbstractItemView::NoSelection);
    memTable->setFocusPolicy(Qt::ClickFocus);
    memTable->setItemDelegate(new HexDelegate(memTable));
    memTable->setToolTip("Double-click a cell to edit its hex value");
    memTable->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    for (int r = 0; r < 16; ++r) {
        for (int c = 0; c < 16; ++c) {
            auto* item = new QTableWidgetItem("00");
            item->setTextAlignment(Qt::AlignCenter);
            memTable->setItem(r, c, item);
        }
    }
    memLay->addWidget(memTable);

    // ===== Direct instruction input (under the memory matrix)
    auto* instrBox = new QGroupBox("Instruction Input  (hex, one 4-digit instruction per line)");
    auto* instrLay = new QGridLayout(instrBox);
    instrEdit = new QPlainTextEdit;
    instrEdit->setPlaceholderText("2105\n2203\n5312\n33FF\nC000");
    instrEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    instrEdit->setFixedHeight(100);
    instrEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    instrStartEdit = new QLineEdit("00");
    instrStartEdit->setMaxLength(2);
    instrStartEdit->setFixedWidth(56);
    instrStartEdit->setValidator(new QRegularExpressionValidator(
        QRegularExpression("[0-9A-Fa-f]{0,2}"), this));
    instrInfo = new QLabel("Lines starting with ; are comments. Invalid instructions turn red.");
    instrInfo->setObjectName("hint");
    instrLoadBtn = new QPushButton("Load Instructions");
    instrLoadBtn->setObjectName("primary");
    instrLoadBtn->setCursor(Qt::PointingHandCursor);
    instrLay->addWidget(instrEdit, 0, 0, 1, 4);
    instrLay->addWidget(new QLabel("Start (hex)"), 1, 0);
    instrLay->addWidget(instrStartEdit, 1, 1);
    instrLay->addWidget(instrInfo, 1, 2);
    instrLay->addWidget(instrLoadBtn, 1, 3);
    instrLay->setColumnStretch(2, 1);

    auto* center    = new QWidget;
    auto* centerLay = new QVBoxLayout(center);
    centerLay->setContentsMargins(0, 0, 0, 0);
    centerLay->setSpacing(8);
    centerLay->addWidget(memBox, 1);
    centerLay->addWidget(instrBox);

    // ===== RIGHT PANEL
    auto* right    = new QWidget;
    right->setFixedWidth(280);
    auto* rightLay = new QVBoxLayout(right);
    rightLay->setContentsMargins(0, 0, 0, 0);
    rightLay->setSpacing(8);

    // --- PC and IR
    auto* cpuBox = new QGroupBox("CPU");
    auto* cpuLay = new QGridLayout(cpuBox);
    pcLabel = new QLabel;
    irLabel = new QLabel;
    pcLabel->setObjectName("value");
    irLabel->setObjectName("value");
    pcLabel->setAlignment(Qt::AlignCenter);
    irLabel->setAlignment(Qt::AlignCenter);
    cpuLay->addWidget(new QLabel("PC"), 0, 0);
    cpuLay->addWidget(pcLabel, 0, 1);
    cpuLay->addWidget(new QLabel("IR"), 1, 0);
    cpuLay->addWidget(irLabel, 1, 1);
    rightLay->addWidget(cpuBox);

    // --- registers
    auto* regBox = new QGroupBox("Registers");
    auto* regLay = new QVBoxLayout(regBox);
    regTable = new QTableWidget(16, 2);
    regTable->setHorizontalHeaderLabels({"Hex", "Dec"});
    QStringList regNames;
    for (int i = 0; i < 16; ++i) regNames << ("R" + QString::number(i, 16).toUpper());
    regTable->setVerticalHeaderLabels(regNames);
    regTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    regTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    regTable->setEditTriggers(QAbstractItemView::DoubleClicked);   // double-click a Hex cell to edit it
    regTable->setSelectionMode(QAbstractItemView::NoSelection);
    regTable->setFocusPolicy(Qt::ClickFocus);
    regTable->setItemDelegate(new HexDelegate(regTable));
    regTable->setToolTip("Double-click a Hex cell to edit the register");
    regTable->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    for (int r = 0; r < 16; ++r) {
        for (int c = 0; c < 2; ++c) {
            auto* item = new QTableWidgetItem("0");
            item->setTextAlignment(Qt::AlignCenter);
            if (c == 1) item->setFlags(item->flags() & ~Qt::ItemIsEditable);   // Dec column is display only
            regTable->setItem(r, c, item);
        }
    }
    regLay->addWidget(regTable);
    rightLay->addWidget(regBox, 1);

    // --- screen
    auto* scrBox = new QGroupBox("Screen (writes to cell FF)");
    auto* scrLay = new QVBoxLayout(scrBox);
    screenView = new QPlainTextEdit;
    screenView->setReadOnly(true);
    screenView->setFixedHeight(90);
    screenView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    scrLay->addWidget(screenView);
    rightLay->addWidget(scrBox);

    body->addWidget(left);
    body->addWidget(center, 1);
    body->addWidget(right);
    root->addLayout(body, 1);

    // ---------- status bar
    statusLabel = new QLabel;
    statusLabel->setObjectName("status");
    statusLabel->setWordWrap(true);
    root->addWidget(statusLabel);

    // ---------- connections
    connect(browseBtn,   &QPushButton::clicked, this, &MainWindow::browseFile);
    connect(loadBtn,     &QPushButton::clicked, this, &MainWindow::loadProgram);
    connect(runBtn,      &QPushButton::clicked, this, &MainWindow::runAll);
    connect(stepBtn,     &QPushButton::clicked, this, &MainWindow::stepOnce);
    connect(resetBtn,    &QPushButton::clicked, this, &MainWindow::resetAll);
    connect(writeMemBtn, &QPushButton::clicked, this, &MainWindow::writeMemory);
    connect(writeRegBtn, &QPushButton::clicked, this, &MainWindow::writeRegister);
    connect(helpBtn,     &QPushButton::clicked, this, &MainWindow::showHelp);
    connect(themeBtn,    &QPushButton::clicked, this, &MainWindow::toggleTheme);
    connect(instrLoadBtn, &QPushButton::clicked, this, &MainWindow::loadTypedInstructions);
    connect(instrEdit,   &QPlainTextEdit::textChanged, this, &MainWindow::validateInstructions);
    connect(memTable,    &QTableWidget::itemChanged, this, &MainWindow::onMemCellEdited);
    connect(regTable,    &QTableWidget::itemChanged, this, &MainWindow::onRegCellEdited);

    // pressing Enter = next step (ignored while typing in a text box)
    auto* sc1 = new QShortcut(QKeySequence(Qt::Key_Return), this);
    auto* sc2 = new QShortcut(QKeySequence(Qt::Key_Enter), this);
    connect(sc1, &QShortcut::activated, this, &MainWindow::enterPressed);
    connect(sc2, &QShortcut::activated, this, &MainWindow::enterPressed);
}

// ---------------------------------------------------------------- dark / light style
void MainWindow::applyStyle() {
    QMap<QString, QString> t;
    if (darkMode) {
        t = {{"bg","#050807"},{"panel","#0d1311"},{"border","#1f3a30"},{"input","#070b0a"},
             {"inputBorder","#23463a"},{"accent","#3DDC97"},{"hover","#6cf0b8"},{"press","#2bb97c"},
             {"onAccent","#000000"},{"text","#d4f7e7"},{"muted","#6f9a89"},{"grid","#162b23"},
             {"btn","#10211a"},{"err","#ff8a8a"},{"statusText","#9fe8c7"},{"dim","#4a5f57"},
             {"disabled","#5d7a6d"}};
    } else {
        t = {{"bg","#eef3f1"},{"panel","#ffffff"},{"border","#b9d3c8"},{"input","#f7fbf9"},
             {"inputBorder","#9fc2b3"},{"accent","#0E9F6E"},{"hover","#12b97f"},{"press","#0b7f58"},
             {"onAccent","#ffffff"},{"text","#14281f"},{"muted","#5b7a6d"},{"grid","#d3e4dc"},
             {"btn","#e3f3ec"},{"err","#c62828"},{"statusText","#1f5c45"},{"dim","#9db5ab"},
             {"disabled","#9db5ab"}};
    }
    accentColor   = QColor(t["accent"]);
    onAccentColor = QColor(t["onAccent"]);
    dimColor      = QColor(t["dim"]);
    textColor     = QColor(t["text"]);
    errColor      = QColor(t["err"]);

    QString css = R"(
        QMainWindow, QWidget#central { background: @bg; }
        QWidget { color: @text; font-size: 13px; }

        QGroupBox {
            background: @panel;
            border: 1px solid @border;
            border-radius: 10px;
            margin-top: 14px;
            padding: 14px 10px 10px 10px;
            font-weight: bold;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 14px;
            padding: 0 6px;
            color: @accent;
        }

        QLineEdit, QPlainTextEdit, QTextBrowser {
            background: @input;
            border: 1px solid @inputBorder;
            border-radius: 6px;
            padding: 5px 8px;
            selection-background-color: @accent;
            selection-color: @onAccent;
        }
        QLineEdit:focus, QPlainTextEdit:focus { border: 1px solid @accent; }

        QPushButton {
            background: @btn;
            border: 1px solid @accent;
            border-radius: 8px;
            padding: 7px 16px;
            min-height: 20px;
            color: @accent;
            font-weight: bold;
        }
        QPushButton:hover   { background: @accent; color: @onAccent; }
        QPushButton:pressed { background: @press; color: @onAccent; }
        QPushButton:focus   { border: 2px solid @accent; }
        QPushButton:disabled { background: @panel; border: 1px solid @border; color: @disabled; }
        QPushButton#primary { background: @accent; color: @onAccent; }
        QPushButton#primary:hover   { background: @hover; }
        QPushButton#primary:pressed { background: @press; }
        QPushButton#danger  { background: @btn; border: 1px solid @err; color: @err; }
        QPushButton#danger:hover   { background: @err; color: @onAccent; }
        QPushButton#danger:pressed { background: @err; color: @onAccent; }
        QPushButton#ghost   { background: transparent; border: 1px solid @border; color: @text; padding: 6px 14px; }
        QPushButton#ghost:hover { border: 1px solid @accent; background: @btn; color: @accent; }

        QTableWidget {
            background: @input;
            gridline-color: @grid;
            border: 1px solid @inputBorder;
            border-radius: 6px;
        }
        QTableWidget QLineEdit {
            padding: 0px 2px;
            border: 2px solid @accent;
            border-radius: 0px;
        }
        QHeaderView::section {
            background: @panel;
            color: @accent;
            border: none;
            padding: 2px;
            font-weight: bold;
        }
        QTableCornerButton::section { background: @panel; border: none; }

        QLabel#title    { font-size: 24px; font-weight: bold; color: @accent; }
        QLabel#subtitle { color: @muted; }
        QLabel#hint     { color: @muted; }
        QLabel#hint[error="true"] { color: @err; font-weight: bold; }
        QLabel#value {
            font-family: Consolas, "Courier New", monospace;
            font-size: 20px;
            font-weight: bold;
            color: @accent;
            background: @input;
            border: 1px solid @inputBorder;
            border-radius: 6px;
            padding: 6px 10px;
        }
        QLabel#status {
            background: @panel;
            border-top: 1px solid @border;
            padding: 8px 12px;
            color: @statusText;
        }
        QLabel#status[error="true"] { color: @err; }

        QScrollBar:vertical { background: @input; width: 10px; }
        QScrollBar::handle:vertical { background: @inputBorder; border-radius: 5px; }
        QScrollBar:horizontal { background: @input; height: 10px; }
        QScrollBar::handle:horizontal { background: @inputBorder; border-radius: 5px; }
        QScrollBar::add-line, QScrollBar::sub-line { height: 0; width: 0; }
    )";
    for (auto it = t.constBegin(); it != t.constEnd(); ++it)
        css.replace("@" + it.key(), it.value());
    setStyleSheet(css);

    themeBtn->setText(darkMode ? "Light Mode" : "Dark Mode");
}

// ---------------------------------------------------------------- refresh every display
void MainWindow::refreshAll() {
    const Memory&       mem  = machine.getMemory();
    const RegisterFile& regs = machine.getRegisters();
    const int pc = machine.getPC();

    const QColor mint = accentColor;
    const QColor dim  = dimColor;
    const QSignalBlocker blockMem(memTable);   // programmatic updates must not look like user edits
    const QSignalBlocker blockReg(regTable);

    // memory matrix: mint = non-zero, dim = zero, mint background = cells the PC points at
    for (int r = 0; r < 16; ++r) {
        for (int c = 0; c < 16; ++c) {
            const int addr = r * 16 + c;
            const int v    = mem.read(addr);
            QTableWidgetItem* item = memTable->item(r, c);
            item->setText(hex2(v));

            const bool atPc = (addr == pc || addr == pc + 1);
            if (atPc) {
                item->setBackground(QBrush(mint));
                item->setForeground(QBrush(onAccentColor));
            } else {
                item->setBackground(QBrush());
                item->setForeground(QBrush(v == 0 ? dim : mint));
            }
        }
    }

    // registers
    for (int i = 0; i < 16; ++i) {
        const int v = regs.get(i);
        regTable->item(i, 0)->setText(hex2(v));
        regTable->item(i, 1)->setText(QString::number(v));
        regTable->item(i, 0)->setForeground(QBrush(v == 0 ? dim : mint));
        regTable->item(i, 1)->setForeground(QBrush(v == 0 ? dim : textColor));
    }

    // PC, IR, screen
    pcLabel->setText(QString::number(pc));   // PC shown in decimal
    irLabel->setText(QString::fromStdString(machine.getIR()));
    screenView->setPlainText(QString::fromStdString(mem.getScreenOutput()));
}

// ---------------------------------------------------------------- slots
void MainWindow::browseFile() {
    const QString path = QFileDialog::getOpenFileName(
        this, "Choose a program file", QString(), "Text files (*.txt);;All files (*)");
    if (!path.isEmpty())
        fileEdit->setText(path);
}

void MainWindow::loadProgram() {
    const QString path = fileEdit->text().trimmed();
    const QString addr = startEdit->text().trimmed();

    if (path.isEmpty() || !QFileInfo::exists(path)) {
        setStatus("Program file not found. Use Browse to choose a file.", true);
        return;
    }
    bool ok = false;
    const int a = addr.toInt(&ok, 16);
    if (addr.isEmpty() || !ok) {
        setStatus("Enter a valid hexadecimal start address (for example 00 or 10).", true);
        return;
    }
    if (a % 2 != 0 || a >= 0xFF) {
        setStatus("Start address must be an EVEN number below FF (00, 02, 04, ... FE).", true);
        return;
    }

    bool loaded = false;
    try {
        loaded = machine.loadProgram(path.toStdString(), addr.toStdString());
    } catch (const std::exception&) {
        loaded = false;
    }

    refreshAll();
    if (loaded) {
        setStatus(QString("Program loaded at address %1. PC = %1. Press Step or Run All.")
                      .arg(hex2(a)));
    } else {
        setStatus("Could not load the program. Every line must start with 4 hex digits "
                  "(or be a ';' comment) and the program must fit in memory.", true);
    }
}

void MainWindow::runAll() {
    if (machine.isHalted()) {
        setStatus("The machine is halted. Load a program or press Reset All first.", true);
        return;
    }

    int  steps = 0;
    bool ok = true, hitLimit = false;
    QString errorText;

    try {
        while (!machine.isHalted()) {
            if (steps >= kMaxSteps) { hitLimit = true; break; }
            if (!machine.step())    { ok = false;      break; }
            ++steps;
        }
    } catch (const std::exception& e) {
        ok = false;
        errorText = e.what();
    }

    refreshAll();   // memory, PC, IR, registers and screen shown after the run finishes

    if (!errorText.isEmpty())
        setStatus("Execution error: " + errorText, true);
    else if (hitLimit)
        setStatus(QString("Stopped after %1 steps (possible endless loop).").arg(kMaxSteps), true);
    else if (!ok)
        setStatus(QString("Invalid instruction %1 - execution stopped after %2 step(s).")
                      .arg(QString::fromStdString(machine.getIR())).arg(steps), true);
    else
        setStatus(QString("Program finished (halt reached) after %1 step(s).").arg(steps));
}

void MainWindow::stepOnce() {
    if (machine.isHalted()) {
        setStatus("The machine is halted. Load a program or press Reset All first.", true);
        return;
    }

    bool ok = false;
    QString errorText;
    try {
        ok = machine.step();
    } catch (const std::exception& e) {
        errorText = e.what();
    }

    refreshAll();

    const QString ir = QString::fromStdString(machine.getIR());
    if (!errorText.isEmpty())
        setStatus("Execution error: " + errorText, true);
    else if (!ok)
        setStatus(QString("Invalid instruction %1 - it was not executed.").arg(ir), true);
    else if (machine.isHalted())
        setStatus(QString("Executed %1 (HALT). Program finished.").arg(ir));
    else
        setStatus(QString("Executed %1. Next PC = %2. Press Enter for the next step.")
                      .arg(ir).arg(hex2(machine.getPC())));
}

void MainWindow::resetAll() {
    machine.resetAll();
    refreshAll();
    setStatus("Machine reset: memory, registers, PC, IR and screen were cleared. Load a program to continue.");
}

void MainWindow::writeMemory() {
    bool okA = false, okV = false;
    const int a = memAddrEdit->text().trimmed().toInt(&okA, 16);
    const int v = memValEdit->text().trimmed().toInt(&okV, 16);
    if (!okA || !okV || a < 0 || a > 0xFF || v < 0 || v > 0xFF) {
        setStatus("Enter a hex address (00-FF) and a hex value (00-FF).", true);
        return;
    }
    machine.setMemoryCell(static_cast<uint8_t>(a), static_cast<uint8_t>(v));
    refreshAll();
    if (a == 0xFF)
        setStatus(QString("Value %1 sent to the screen (cell FF).").arg(hex2(v)));
    else
        setStatus(QString("Memory cell %1 = %2").arg(hex2(a)).arg(hex2(v)));
}

void MainWindow::writeRegister() {
    bool okR = false, okV = false;
    const int r = regNumEdit->text().trimmed().toInt(&okR, 16);
    const int v = regValEdit->text().trimmed().toInt(&okV, 16);
    if (!okR || !okV || r < 0 || r > 0xF || v < 0 || v > 0xFF) {
        setStatus("Enter a register number (0-F) and a hex value (00-FF).", true);
        return;
    }
    machine.setRegister(static_cast<uint8_t>(r), static_cast<uint8_t>(v));
    refreshAll();
    setStatus(QString("Register R%1 = %2").arg(QString::number(r, 16).toUpper()).arg(hex2(v)));
}

void MainWindow::enterPressed() {
    QWidget* fw = focusWidget();
    if (auto* le = qobject_cast<QLineEdit*>(fw)) {
        // Enter inside a cell editor confirms the edit; in any other text box it is just typing
        QWidget* p = le->parentWidget();
        if (p == memTable->viewport() || p == regTable->viewport())
            le->clearFocus();
        return;
    }
    if (qobject_cast<QPlainTextEdit*>(fw) != nullptr)
        return;     // typing instructions, don't step
    stepOnce();
}

void MainWindow::showHelp() {
    QDialog dlg(this);
    dlg.setWindowTitle("Help - How to use the Vole Machine Simulator");
    dlg.resize(640, 640);
    dlg.setStyleSheet(styleSheet());

    auto* lay  = new QVBoxLayout(&dlg);
    auto* text = new QTextBrowser;
    text->setHtml(R"(
<h2 style="color:#3DDC97;">How to use</h2>
<ol>
<li><b>Load a program</b>: click <i>Browse</i> and pick your program file, type an
<b>even</b> start address in hex (00, 02, 04 ... FE) and press <i>Load Program</i>.
Each line of the file is one instruction of 4 hex digits (a line starting with ; is a comment).</li>
<li><b>Step by step</b>: press <i>Step</i> (or the <b>Enter</b> key). One instruction is fetched into IR,
validated, decoded and executed, then PC moves to the next instruction.</li>
<li><b>Run All</b>: runs the whole program at once until the halt instruction C000.
The memory, PC, IR, registers and screen update when it finishes.</li>
<li><b>Reset All</b>: clears memory, registers, PC, IR and the screen.</li>
<li><b>Modify Memory Cell</b>: type an address (00-FF) and a value (00-FF) in hex, then press the button.</li>
<li><b>Modify Register</b>: type a register number (0-F) and a value (00-FF) in hex, then press the button.</li>
<li>To run another program just press <i>Load Program</i> again (or <i>Reset All</i> first).</li>
</ol>

<h2 style="color:#3DDC97;">What you see</h2>
<ul>
<li><b>Main Memory</b>: all 256 cells as a 16x16 matrix. The row label is the address of the first cell of the row,
the column label is the offset. The two cells with a mint background are the instruction PC points to.</li>
<li><b>Registers</b>: R0 to RF in hex and decimal.</li>
<li><b>PC / IR</b>: program counter and instruction register.</li>
<li><b>Screen</b>: anything written (store) to memory cell <b>FF</b> is printed here instead of being stored.</li>
<li>The bar at the bottom tells you what just happened or what went wrong.</li>
</ul>

<h2 style="color:#3DDC97;">Instruction set</h2>
<table cellpadding="4">
<tr><td><b>1RXY</b></td><td>load register R from memory cell XY</td></tr>
<tr><td><b>2RXY</b></td><td>load register R with the value XY</td></tr>
<tr><td><b>3RXY</b></td><td>store register R into memory cell XY (XY = FF writes to the screen)</td></tr>
<tr><td><b>40RS</b></td><td>copy register R into register S</td></tr>
<tr><td><b>5RST</b></td><td>R = S + T (two's complement)</td></tr>
<tr><td><b>6RST</b></td><td>R = S + T (floating point)</td></tr>
<tr><td><b>7RST</b></td><td>R = S OR T</td></tr>
<tr><td><b>8RST</b></td><td>R = S AND T</td></tr>
<tr><td><b>9RST</b></td><td>R = S XOR T</td></tr>
<tr><td><b>AR0X</b></td><td>rotate register R right X times</td></tr>
<tr><td><b>BRXY</b></td><td>if R equals R0 then jump to address XY</td></tr>
<tr><td><b>C000</b></td><td>halt</td></tr>
</table>
)");
    lay->addWidget(text);

    auto* closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    lay->addWidget(closeBtn);

    dlg.exec();
}

// ---------------------------------------------------------------- double-click editing
void MainWindow::onMemCellEdited(QTableWidgetItem* item) {
    const int addr = item->row() * 16 + item->column();
    bool ok = false;
    const int v = item->text().trimmed().toInt(&ok, 16);
    if (!ok || v < 0 || v > 0xFF) {
        refreshAll();                                   // put the old value back
        setStatus("Invalid value. Enter 1 or 2 hex digits (00-FF).", true);
        return;
    }
    machine.setMemoryCell(static_cast<uint8_t>(addr), static_cast<uint8_t>(v));
    refreshAll();
    if (addr == 0xFF)
        setStatus(QString("Value %1 sent to the screen (cell FF).").arg(hex2(v)));
    else
        setStatus(QString("Memory cell %1 = %2").arg(hex2(addr)).arg(hex2(v)));
}

void MainWindow::onRegCellEdited(QTableWidgetItem* item) {
    if (item->column() != 0) return;
    const int r = item->row();
    bool ok = false;
    const int v = item->text().trimmed().toInt(&ok, 16);
    if (!ok || v < 0 || v > 0xFF) {
        refreshAll();
        setStatus("Invalid value. Enter 1 or 2 hex digits (00-FF).", true);
        return;
    }
    machine.setRegister(static_cast<uint8_t>(r), static_cast<uint8_t>(v));
    refreshAll();
    setStatus(QString("Register R%1 = %2").arg(QString::number(r, 16).toUpper()).arg(hex2(v)));
}

// ---------------------------------------------------------------- theme
void MainWindow::toggleTheme() {
    darkMode = !darkMode;
    applyStyle();
    refreshAll();
    validateInstructions();
}

// ---------------------------------------------------------------- direct instruction input
// same rules as CPU::isValid: 4 hex digits + opcode specific checks
bool MainWindow::isValidInstruction(const QString& text) {
    const QString s = text.trimmed().toUpper();
    if (s.length() != 4) return false;
    for (QChar c : s)
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) return false;
    switch (s[0].toLatin1()) {
        case '1': case '2': case '3': case '5': case '6':
        case '7': case '8': case '9': case 'B': return true;
        case '4': return s[1] == '0';
        case 'A': return s[2] == '0';
        case 'C': return s.mid(1) == "000";
        default:  return false;
    }
}

// strips a ';' comment and spaces from one editor line
static QString instructionPart(const QString& line) {
    QString t = line;
    const int c = t.indexOf(';');
    if (c >= 0) t = t.left(c);
    return t.trimmed();
}

void MainWindow::validateInstructions() {
    QList<QTextEdit::ExtraSelection> marks;
    int good = 0, bad = 0;

    QColor bg = errColor;
    bg.setAlpha(darkMode ? 70 : 50);

    for (QTextBlock b = instrEdit->document()->begin(); b.isValid(); b = b.next()) {
        const QString t = instructionPart(b.text());
        if (t.isEmpty()) continue;
        if (isValidInstruction(t)) { ++good; continue; }
        ++bad;
        QTextEdit::ExtraSelection sel;
        sel.format.setBackground(bg);
        sel.format.setForeground(errColor);
        sel.format.setFontWeight(QFont::Bold);
        sel.format.setProperty(QTextFormat::FullWidthSelection, true);
        sel.cursor = QTextCursor(b);
        sel.cursor.select(QTextCursor::LineUnderCursor);
        marks.append(sel);
    }
    instrEdit->setExtraSelections(marks);

    if (bad > 0)
        instrInfo->setText(QString("%1 invalid instruction(s) - marked in red.").arg(bad));
    else if (good > 0)
        instrInfo->setText(QString("%1 valid instruction(s).").arg(good));
    else
        instrInfo->setText("Lines starting with ; are comments. Invalid instructions turn red.");
    instrInfo->setProperty("error", bad > 0);
    instrInfo->style()->unpolish(instrInfo);
    instrInfo->style()->polish(instrInfo);
}

void MainWindow::loadTypedInstructions() {
    validateInstructions();

    const QString addr = instrStartEdit->text().trimmed();
    bool ok = false;
    const int a = addr.toInt(&ok, 16);
    if (addr.isEmpty() || !ok || a % 2 != 0 || a >= 0xFF) {
        setStatus("Start address must be an EVEN hex number below FF (00, 02, 04, ... FE).", true);
        return;
    }

    QStringList lines;
    for (QTextBlock b = instrEdit->document()->begin(); b.isValid(); b = b.next()) {
        const QString t = instructionPart(b.text());
        if (t.isEmpty()) continue;
        if (!isValidInstruction(t)) {
            setStatus(QString("Invalid instruction \"%1\" (line %2) - fix the lines marked in red.")
                          .arg(t).arg(b.blockNumber() + 1), true);
            return;
        }
        lines << t.toUpper();
    }
    if (lines.isEmpty()) {
        setStatus("Type at least one instruction first.", true);
        return;
    }

    // hand the instructions to the existing Machine::loadProgram through a temporary file
    QTemporaryFile tmp;
    if (!tmp.open()) {
        setStatus("Could not prepare the instructions for loading.", true);
        return;
    }
    QTextStream out(&tmp);
    for (const QString& l : lines) out << l << "\n";
    out.flush();
    tmp.close();

    bool loaded = false;
    try {
        loaded = machine.loadProgram(tmp.fileName().toStdString(), addr.toStdString());
    } catch (const std::exception&) {
        loaded = false;
    }

    refreshAll();
    if (loaded)
        setStatus(QString("%1 instruction(s) loaded at address %2. PC = %2. Press Step or Run All.")
                      .arg(lines.size()).arg(hex2(a)));
    else
        setStatus("Could not load the instructions - they do not fit in memory from that address.", true);
}
