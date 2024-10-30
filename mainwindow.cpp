#include "mainwindow.h"
#include <QQuickView>
#include <QVBoxLayout>
#include <QWidget>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Créer un QWidget central
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Créer un layout pour le widget central
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    // Créer un QQuickView pour charger le fichier QML de la carte
    QQuickView *quickView = new QQuickView();
    quickView->setSource(QUrl(QStringLiteral("qrc:/maCarte/MapView.qml"))); // Charge le fichier QML

    // Vérifiez si le chargement a réussi
    if (quickView->status() == QQuickView::Error) {
        qWarning() << "Erreur de chargement du fichier QML:" << quickView->errors();
    }

    // Ajouter le QQuickView au layout
    layout->addWidget(QWidget::createWindowContainer(quickView, this)); // Créer un conteneur pour le QQuickView
}

MainWindow::~MainWindow() {}
