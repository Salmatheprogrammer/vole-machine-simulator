#pragma once

// NOTE: Qt headers MUST come before Machine.h (Machine.h -> CPU.h has "using namespace std")
#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QString>
#include <QColor>
#include <QTableWidgetItem>

#include "Machine.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private Q_SLOTS:
    void browseFile();
    void loadProgram();
    void runAll();
    void stepOnce();
    void resetAll();
    void writeMemory();
    void writeRegister();
    void showHelp();
    void enterPressed();
    void onMemCellEdited(QTableWidgetItem* item);
    void onRegCellEdited(QTableWidgetItem* item);
    void toggleTheme();
    void validateInstructions();
    void loadTypedInstructions();

private:
    void buildUi();
    void applyStyle();
    void refreshAll();
    void setStatus(const QString& text, bool isError = false);
    QString hex2(int value) const;
    static bool isValidInstruction(const QString& text);   // same rules as CPU::isValid

    // the simulator itself (your existing classes, untouched)
    Machine machine;

    // load program
    QLineEdit*   fileEdit;
    QLineEdit*   startEdit;
    QPushButton* browseBtn;
    QPushButton* loadBtn;

    // execution
    QPushButton* runBtn;
    QPushButton* stepBtn;
    QPushButton* resetBtn;

    // modify memory / register
    QLineEdit*   memAddrEdit;
    QLineEdit*   memValEdit;
    QPushButton* writeMemBtn;
    QLineEdit*   regNumEdit;
    QLineEdit*   regValEdit;
    QPushButton* writeRegBtn;

    // displays
    QTableWidget*   memTable;
    QTableWidget*   regTable;
    QLabel*         pcLabel;
    QLabel*         irLabel;
    QPlainTextEdit* screenView;
    QLabel*         statusLabel;
    QPushButton*    helpBtn;
    QPushButton*    themeBtn;

    // direct instruction input
    QPlainTextEdit* instrEdit;
    QLineEdit*      instrStartEdit;
    QPushButton*    instrLoadBtn;
    QLabel*         instrInfo;

    // theme
    bool   darkMode = true;
    QColor accentColor, onAccentColor, dimColor, textColor, errColor;
};
