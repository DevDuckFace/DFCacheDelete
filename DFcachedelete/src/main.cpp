#include "MainWindow.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    app.setApplicationName("DFCacheDelete");
    app.setWindowIcon(QIcon(":/duck_icon.png"));
    
    MainWindow window;
    window.resize(1000, 600);
    window.setWindowTitle("DFCacheDelete - Cache Folder Cleaner");
    window.show();
    
    return app.exec();
}
