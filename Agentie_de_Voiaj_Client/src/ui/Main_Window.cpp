#include "ui/Main_Window.h"
#include "ui/Login_Window.h"
#include "models/User_Model.h"
#include "models/Destination_Model.h"
#include "config.h"

#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QGroupBox>
#include <QScrollArea>

Main_Window::Main_Window(QWidget *parent)
    : QMainWindow(parent)
    , m_central_widget(nullptr)
    , m_tab_widget(nullptr)
    , m_destinations_tab(nullptr)
    , m_offers_tab(nullptr)
    , m_reservations_tab(nullptr)
    , m_profile_tab(nullptr)
    , m_menu_bar(nullptr)
    , m_file_menu(nullptr)
    , m_view_menu(nullptr)
    , m_help_menu(nullptr)
    , m_login_action(nullptr)
    , m_logout_action(nullptr)
    , m_exit_action(nullptr)
    , m_settings_action(nullptr)
    , m_toggle_theme_action(nullptr)
    , m_about_action(nullptr)
    , m_status_bar(nullptr)
    , m_connection_status_label(nullptr)
    , m_user_status_label(nullptr)
    , m_progress_bar(nullptr)
    , m_header_widget(nullptr)
    , m_title_label(nullptr)
    , m_user_info_label(nullptr)
    , m_user_menu_button(nullptr)
    , m_theme_toggle_button(nullptr)
    , m_user_model(std::make_unique<User_Model>(this))
    , m_destination_model(std::make_unique<Destination_Model>(this))
    , m_is_authenticated(false)
    , m_current_theme("light")
{
    setup_ui();
    setup_menu_bar();
    setup_status_bar();
    setup_connections();
    
    // Initialize with login prompt
    show_login_prompt();
    
    // Load destinations
    m_destination_model->refresh_destinations();
    
    setWindowTitle("Agentie de Voiaj");
    setMinimumSize(800, 600);
    resize(1200, 800);
}

Main_Window::~Main_Window() = default;

void Main_Window::setup_ui()
{
    // Central widget
    m_central_widget = new QWidget();
    setCentralWidget(m_central_widget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(m_central_widget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Header
    m_header_widget = new QWidget();
    m_header_widget->setFixedHeight(80);
    m_header_widget->setObjectName("headerWidget");
    m_header_widget->setStyleSheet(
        "#headerWidget { "
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 #4a90e2, stop:1 #357abd); "
        "border-bottom: 2px solid #2968a3; "
        "}"
    );
    
    QHBoxLayout* headerLayout = new QHBoxLayout(m_header_widget);
    headerLayout->setContentsMargins(20, 10, 20, 10);
    
    // Title
    m_title_label = new QLabel("Agentie de Voiaj");
    m_title_label->setStyleSheet(
        "font-size: 24px; font-weight: bold; color: white;"
    );
    headerLayout->addWidget(m_title_label);
    
    headerLayout->addStretch();
    
    // User info
    m_user_info_label = new QLabel("Neconectat");
    m_user_info_label->setStyleSheet("color: white; font-size: 14px;");
    headerLayout->addWidget(m_user_info_label);
    
    // Theme toggle button
    m_theme_toggle_button = new QPushButton("🌙");
    m_theme_toggle_button->setFixedSize(40, 40);
    m_theme_toggle_button->setStyleSheet(
        "QPushButton { "
        "background-color: rgba(255,255,255,0.2); "
        "border: 2px solid rgba(255,255,255,0.3); "
        "border-radius: 20px; "
        "font-size: 16px; "
        "} "
        "QPushButton:hover { "
        "background-color: rgba(255,255,255,0.3); "
        "}"
    );
    headerLayout->addWidget(m_theme_toggle_button);
    
    // User menu button
    m_user_menu_button = new QPushButton("👤");
    m_user_menu_button->setFixedSize(40, 40);
    m_user_menu_button->setStyleSheet(
        "QPushButton { "
        "background-color: rgba(255,255,255,0.2); "
        "border: 2px solid rgba(255,255,255,0.3); "
        "border-radius: 20px; "
        "font-size: 16px; "
        "} "
        "QPushButton:hover { "
        "background-color: rgba(255,255,255,0.3); "
        "}"
    );
    headerLayout->addWidget(m_user_menu_button);
    
    mainLayout->addWidget(m_header_widget);
    
    // Tab widget
    setup_tabs();
    mainLayout->addWidget(m_tab_widget);
}

void Main_Window::setup_tabs()
{
    m_tab_widget = new QTabWidget();
    m_tab_widget->setTabPosition(QTabWidget::North);
    m_tab_widget->setMovable(false);
    
    create_destinations_tab();
    create_offers_tab();
    create_reservations_tab();
    create_profile_tab();
    
    // Set icons (emoji for now, can be replaced with actual icons)
    m_tab_widget->setTabText(0, "🏖️ Destinații");
    m_tab_widget->setTabText(1, "🎫 Oferte");
    m_tab_widget->setTabText(2, "📋 Rezervările Mele");
    m_tab_widget->setTabText(3, "👤 Profilul Meu");
    
    connect(m_tab_widget, &QTabWidget::currentChanged, 
            this, &Main_Window::on_tab_changed);
}

void Main_Window::create_destinations_tab()
{
    m_destinations_tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_destinations_tab);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);
    
    // Search section
    QHBoxLayout* searchLayout = new QHBoxLayout();
    
    QLabel* searchLabel = new QLabel("Căutare destinații:");
    searchLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    searchLayout->addWidget(searchLabel);
    
    QLineEdit* searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Introduceți numele destinației sau țării...");
    searchEdit->setMinimumHeight(35);
    searchLayout->addWidget(searchEdit);
    
    QPushButton* searchButton = new QPushButton("🔍 Caută");
    searchButton->setMinimumHeight(35);
    searchLayout->addWidget(searchButton);
    
    layout->addLayout(searchLayout);
    
    // Destinations grid (placeholder)
    QScrollArea* scrollArea = new QScrollArea();
    QWidget* scrollContent = new QWidget();
    QGridLayout* gridLayout = new QGridLayout(scrollContent);
    
    // Add some placeholder destination cards
    for (int i = 0; i < 6; ++i) {
        QWidget* card = new QWidget();
        card->setFixedSize(300, 200);
        card->setStyleSheet(
            "QWidget { "
            "background-color: white; "
            "border: 1px solid #e0e0e0; "
            "border-radius: 8px; "
            "} "
            "QWidget:hover { "
            "border-color: #4a90e2; "
            "}"
        );
        
        QVBoxLayout* cardLayout = new QVBoxLayout(card);
        
        QLabel* imageLabel = new QLabel("📷");
        imageLabel->setAlignment(Qt::AlignCenter);
        imageLabel->setStyleSheet("font-size: 48px; background-color: #f8f9fa; border-radius: 4px;");
        imageLabel->setFixedHeight(120);
        cardLayout->addWidget(imageLabel);
        
        QLabel* nameLabel = new QLabel(QString("Destinația %1").arg(i + 1));
        nameLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
        nameLabel->setAlignment(Qt::AlignCenter);
        cardLayout->addWidget(nameLabel);
        
        QLabel* countryLabel = new QLabel("Țara Exemplu");
        countryLabel->setAlignment(Qt::AlignCenter);
        countryLabel->setStyleSheet("color: #666;");
        cardLayout->addWidget(countryLabel);
        
        QPushButton* viewButton = new QPushButton("Vezi Oferte");
        cardLayout->addWidget(viewButton);
        
        gridLayout->addWidget(card, i / 3, i % 3);
    }
    
    scrollArea->setWidget(scrollContent);
    scrollArea->setWidgetResizable(true);
    layout->addWidget(scrollArea);
    
    m_tab_widget->addTab(m_destinations_tab, "Destinații");
}

void Main_Window::create_offers_tab()
{
    m_offers_tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_offers_tab);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);
    
    // Filters section
    QHBoxLayout* filtersLayout = new QHBoxLayout();
    
    QLabel* filtersLabel = new QLabel("Filtrează ofertele:");
    filtersLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    filtersLayout->addWidget(filtersLabel);
    
    QLineEdit* priceMinEdit = new QLineEdit();
    priceMinEdit->setPlaceholderText("Preț min");
    priceMinEdit->setMaximumWidth(100);
    filtersLayout->addWidget(priceMinEdit);
    
    QLineEdit* priceMaxEdit = new QLineEdit();
    priceMaxEdit->setPlaceholderText("Preț max");
    priceMaxEdit->setMaximumWidth(100);
    filtersLayout->addWidget(priceMaxEdit);
    
    QPushButton* filterButton = new QPushButton("Aplică Filtre");
    filtersLayout->addWidget(filterButton);
    
    filtersLayout->addStretch();
    layout->addLayout(filtersLayout);
    
    // Offers list
    QScrollArea* scrollArea = new QScrollArea();
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* offersLayout = new QVBoxLayout(scrollContent);
    
    // Placeholder offers
    for (int i = 0; i < 5; ++i) {
        QWidget* offerCard = new QWidget();
        offerCard->setFixedHeight(150);
        offerCard->setStyleSheet(
            "QWidget { "
            "background-color: white; "
            "border: 1px solid #e0e0e0; "
            "border-radius: 8px; "
            "margin: 5px; "
            "} "
            "QWidget:hover { "
            "border-color: #4a90e2; "
            "}"
        );
        
        QHBoxLayout* offerLayout = new QHBoxLayout(offerCard);
        
        // Image placeholder
        QLabel* imageLabel = new QLabel("🏖️");
        imageLabel->setFixedSize(120, 120);
        imageLabel->setAlignment(Qt::AlignCenter);
        imageLabel->setStyleSheet("background-color: #f8f9fa; border-radius: 4px; font-size: 32px;");
        offerLayout->addWidget(imageLabel);
        
        // Offer details
        QVBoxLayout* detailsLayout = new QVBoxLayout();
        
        QLabel* titleLabel = new QLabel(QString("Oferta Paris %1 zile").arg(3 + i));
        titleLabel->setStyleSheet("font-weight: bold; font-size: 18px;");
        detailsLayout->addWidget(titleLabel);
        
        QLabel* descLabel = new QLabel("Descriere scurtă a ofertei cu toate detaliile importante...");
        descLabel->setWordWrap(true);
        detailsLayout->addWidget(descLabel);
        
        QLabel* priceLabel = new QLabel(QString("€%1/persoană").arg(299 + i * 50));
        priceLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #e74c3c;");
        detailsLayout->addWidget(priceLabel);
        
        detailsLayout->addStretch();
        offerLayout->addLayout(detailsLayout);
        
        // Actions
        QVBoxLayout* actionsLayout = new QVBoxLayout();
        actionsLayout->addStretch();
        
        QPushButton* bookButton = new QPushButton("Rezervă Acum");
        bookButton->setStyleSheet("background-color: #27ae60; color: white; font-weight: bold;");
        actionsLayout->addWidget(bookButton);
        
        QPushButton* detailsButton = new QPushButton("Detalii");
        actionsLayout->addWidget(detailsButton);
        
        offerLayout->addLayout(actionsLayout);
        
        offersLayout->addWidget(offerCard);
    }
    
    scrollArea->setWidget(scrollContent);
    scrollArea->setWidgetResizable(true);
    layout->addWidget(scrollArea);
    
    m_tab_widget->addTab(m_offers_tab, "Oferte");
}

void Main_Window::create_reservations_tab()
{
    m_reservations_tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_reservations_tab);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);
    
    QLabel* titleLabel = new QLabel("Rezervările Mele");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 20px;");
    layout->addWidget(titleLabel);
    
    // Authentication required message
    QWidget* authRequiredWidget = new QWidget();
    QVBoxLayout* authLayout = new QVBoxLayout(authRequiredWidget);
    authLayout->setAlignment(Qt::AlignCenter);
    
    QLabel* authLabel = new QLabel("Pentru a vedea rezervările, vă rugăm să vă conectați.");
    authLabel->setAlignment(Qt::AlignCenter);
    authLabel->setStyleSheet("font-size: 16px; color: #666;");
    authLayout->addWidget(authLabel);
    
    QPushButton* loginButton = new QPushButton("Conectează-te");
    loginButton->setMaximumWidth(200);
    connect(loginButton, &QPushButton::clicked, this, &Main_Window::on_login_action);
    authLayout->addWidget(loginButton);
    
    layout->addWidget(authRequiredWidget);
    
    m_tab_widget->addTab(m_reservations_tab, "Rezervări");
}

void Main_Window::create_profile_tab()
{
    m_profile_tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_profile_tab);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);
    
    QLabel* titleLabel = new QLabel("Profilul Meu");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 20px;");
    layout->addWidget(titleLabel);
    
    // Profile form (disabled initially)
    QGroupBox* profileGroup = new QGroupBox("Informații Personale");
    QGridLayout* profileLayout = new QGridLayout(profileGroup);
    
    profileLayout->addWidget(new QLabel("Nume utilizator:"), 0, 0);
    QLineEdit* usernameEdit = new QLineEdit();
    usernameEdit->setEnabled(false);
    profileLayout->addWidget(usernameEdit, 0, 1);
    
    profileLayout->addWidget(new QLabel("Email:"), 1, 0);
    QLineEdit* emailEdit = new QLineEdit();
    emailEdit->setEnabled(false);
    profileLayout->addWidget(emailEdit, 1, 1);
    
    profileLayout->addWidget(new QLabel("Prenume:"), 2, 0);
    QLineEdit* firstNameEdit = new QLineEdit();
    firstNameEdit->setEnabled(false);
    profileLayout->addWidget(firstNameEdit, 2, 1);
    
    profileLayout->addWidget(new QLabel("Nume:"), 3, 0);
    QLineEdit* lastNameEdit = new QLineEdit();
    lastNameEdit->setEnabled(false);
    profileLayout->addWidget(lastNameEdit, 3, 1);
    
    QPushButton* editButton = new QPushButton("Editează Profil");
    editButton->setEnabled(false);
    profileLayout->addWidget(editButton, 4, 0, 1, 2);
    
    layout->addWidget(profileGroup);
    layout->addStretch();
    
    m_tab_widget->addTab(m_profile_tab, "Profil");
}

void Main_Window::setup_menu_bar()
{
    m_menu_bar = menuBar();
    
    // File Menu
    m_file_menu = m_menu_bar->addMenu("&Fișier");
    
    m_login_action = new QAction("&Conectare...", this);
    m_login_action->setShortcut(QKeySequence("Ctrl+L"));
    m_file_menu->addAction(m_login_action);
    
    m_logout_action = new QAction("&Deconectare", this);
    m_logout_action->setEnabled(false);
    m_file_menu->addAction(m_logout_action);
    
    m_file_menu->addSeparator();
    
    m_exit_action = new QAction("&Ieșire", this);
    m_exit_action->setShortcut(QKeySequence::Quit);
    m_file_menu->addAction(m_exit_action);
    
    // View Menu
    m_view_menu = m_menu_bar->addMenu("&Vizualizare");
    
    m_toggle_theme_action = new QAction("Comută &Tema", this);
    m_toggle_theme_action->setShortcut(QKeySequence("Ctrl+T"));
    m_view_menu->addAction(m_toggle_theme_action);
    
    m_settings_action = new QAction("&Setări...", this);
    m_view_menu->addAction(m_settings_action);
    
    // Help Menu
    m_help_menu = m_menu_bar->addMenu("&Ajutor");
    
    m_about_action = new QAction("&Despre...", this);
    m_help_menu->addAction(m_about_action);
}

void Main_Window::setup_status_bar()
{
    m_status_bar = statusBar();
    
    // Connection status
    m_connection_status_label = new QLabel("🔴 Deconectat");
    m_status_bar->addWidget(m_connection_status_label);
    
    m_status_bar->addPermanentWidget(new QLabel("|"));
    
    // User status
    m_user_status_label = new QLabel("Neautentificat");
    m_status_bar->addPermanentWidget(m_user_status_label);
    
    // Progress bar
    m_progress_bar = new QProgressBar();
    m_progress_bar->setVisible(false);
    m_progress_bar->setMaximumWidth(200);
    m_status_bar->addPermanentWidget(m_progress_bar);
}

void Main_Window::setup_connections()
{
    // Menu actions
    connect(m_login_action, &QAction::triggered, this, &Main_Window::on_login_action);
    connect(m_logout_action, &QAction::triggered, this, &Main_Window::on_logout_action);
    connect(m_exit_action, &QAction::triggered, this, &Main_Window::on_exit_action);
    connect(m_about_action, &QAction::triggered, this, &Main_Window::on_about_action);
    connect(m_settings_action, &QAction::triggered, this, &Main_Window::on_settings_action);
    connect(m_toggle_theme_action, &QAction::triggered, this, &Main_Window::on_toggle_theme_action);
    
    // Header buttons
    connect(m_user_menu_button, &QPushButton::clicked, this, &Main_Window::on_login_action);
    connect(m_theme_toggle_button, &QPushButton::clicked, this, &Main_Window::on_toggle_theme_action);
    
    // User model connections
    connect(m_user_model.get(), &User_Model::authentication_status_changed,
            this, &Main_Window::on_authentification_status_changed);
    connect(m_user_model.get(), &User_Model::login_succes,
            this, &Main_Window::on_user_logged_in);
    connect(m_user_model.get(), &User_Model::logged_out,
            this, &Main_Window::on_user_logged_out);
    
    // Destination model connections
    connect(m_destination_model.get(), &Destination_Model::destinations_loaded,
            this, &Main_Window::on_destinations_loaded);
}

void Main_Window::on_login_action()
{
    if (m_is_authenticated) {
        return;
    }
    
    if (Login_Window::show_login_dialog(this)) {
        // Login successful - UI will be updated via signals
    }
}

void Main_Window::on_logout_action()
{
    if (!m_is_authenticated) {
        return;
    }
    
    m_user_model->logout();
}

void Main_Window::on_exit_action()
{
    close();
}

void Main_Window::on_about_action()
{
    QMessageBox::about(this, "Despre Agentie de Voiaj",
                      QString("Agentie de Voiaj v1.0\n\n"
                              "Aplicație client pentru agenția de voiaj.\n"
                              "Dezvoltat cu Qt și C++."));
}

void Main_Window::on_settings_action()
{
    QMessageBox::information(this, "Setări", "Fereastra de setări va fi implementată în curând.");
}

void Main_Window::on_toggle_theme_action()
{
    // Toggle between light and dark theme
    m_current_theme = (m_current_theme == "light") ? "dark" : "light";
    m_theme_toggle_button->setText((m_current_theme == "light") ? "🌙" : "☀️");
    
    // Apply theme (StyleManager will be used here in future)
    QMessageBox::information(this, "Temă", QString("Comutare la tema: %1").arg(m_current_theme));
}

void Main_Window::on_tab_changed(int index)
{
    Q_UNUSED(index)
    animate_tab_change();
}

void Main_Window::on_user_logged_in()
{
    show_welcome_message();
}

void Main_Window::on_user_logged_out()
{
    show_login_prompt();
}

void Main_Window::on_authentification_status_changed(bool is_authenticated)
{
    m_is_authenticated = is_authenticated;
    update_ui_for_authentification_state();
}

void Main_Window::on_destinations_loaded()
{
    m_status_bar->showMessage("Destinații încărcate cu succes", 3000);
}

void Main_Window::update_ui_for_authentification_state()
{
    // Update menu actions
    m_login_action->setEnabled(!m_is_authenticated);
    m_logout_action->setEnabled(m_is_authenticated);
    
    // Update user info
    if (m_is_authenticated) {
        QString displayName = m_user_model->get_display_name();
        m_user_info_label->setText("Bună ziua, " + displayName);
        m_user_status_label->setText("Conectat: " + displayName);
        m_connection_status_label->setText("🟢 Conectat");
    } else {
        m_user_info_label->setText("Neconectat");
        m_user_status_label->setText("Neautentificat");
        m_connection_status_label->setText("🔴 Deconectat");
    }
    
    // Update tabs availability
    m_tab_widget->setTabEnabled(2, m_is_authenticated); // Reservations
    m_tab_widget->setTabEnabled(3, m_is_authenticated); // Profile
}

void Main_Window::show_welcome_message()
{
    QString name = m_user_model->get_display_name();
    m_status_bar->showMessage(QString("Bună ziua, %1! Conectare reușită.").arg(name), 5000);
}

void Main_Window::show_login_prompt()
{
    m_status_bar->showMessage("Pentru a accesa toate funcționalitățile, vă rugăm să vă conectați.", 5000);
}

void Main_Window::animate_tab_change()
{
    // Simple fade animation (can be enhanced)
    // Animation implementation would go here
}

void Main_Window::show_loading_indicator(bool show)
{
    m_progress_bar->setVisible(show);
    if (show) {
        m_progress_bar->setRange(0, 0); // Indeterminate progress
    }
}

