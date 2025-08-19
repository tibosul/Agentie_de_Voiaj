#include "ui/Booking_Dialog.h"
#include "ui/Offer_Card.h"
#include "models/User_Model.h"
#include "config.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDateEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QTableWidget>
#include <QComboBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>
#include <QRegularExpressionValidator>
#include <QTimer>
#include <QFrame>

Booking_Dialog::Booking_Dialog(QWidget* parent)
    : QDialog(parent)
    , m_main_layout(nullptr)
    , m_offer_group(nullptr)
    , m_offer_name_label(nullptr)
    , m_destination_label(nullptr)
    , m_dates_label(nullptr)
    , m_price_per_person_label(nullptr)
    , m_booking_group(nullptr)
    , m_person_count_spin(nullptr)
    , m_persons_table(nullptr)
    , m_special_requests_edit(nullptr)
    , m_price_group(nullptr)
    , m_subtotal_label(nullptr)
    , m_taxes_label(nullptr)
    , m_total_price_label(nullptr)
    , m_confirm_button(nullptr)
    , m_cancel_button(nullptr)
    , m_progress_bar(nullptr)
    , m_status_label(nullptr)
    , m_user_model(std::make_unique<User_Model>(this))
    , m_is_loading(false)
{
    setup_ui();
    setup_connections();
    
    setModal(true);
    setWindowTitle("Rezervare Ofertă");
    setFixedSize(700, 600);
}

Booking_Dialog::Booking_Dialog(const Offer_Data& offer, QWidget* parent)
    : Booking_Dialog(parent)
{
    m_offer = offer;
    m_booking_data.offer_id = offer.id;
    m_booking_data.offer_name = offer.name;
    m_booking_data.destination = offer.destination;
    m_booking_data.price_per_person = offer.price_per_person;
    m_booking_data.departure_date = offer.departure_date;
    m_booking_data.return_date = offer.return_date;
    
    // Update UI with offer data
    if (m_offer_name_label) m_offer_name_label->setText(offer.name);
    if (m_destination_label) m_destination_label->setText("📍 " + offer.destination);
    if (m_dates_label) m_dates_label->setText(QString("📅 %1 → %2").arg(offer.departure_date, offer.return_date));
    if (m_price_per_person_label) m_price_per_person_label->setText(QString("💰 €%1 per persoană").arg(offer.price_per_person, 0, 'f', 0));
    
    update_price_calculation();
}

Booking_Dialog::~Booking_Dialog() = default;

bool Booking_Dialog::show_booking_dialog(const Offer_Data& offer, QWidget* parent)
{
    Booking_Dialog dialog(offer, parent);
    return dialog.exec() == QDialog::Accepted;
}

Booking_Data Booking_Dialog::get_booking_data() const
{
    return m_booking_data;
}

void Booking_Dialog::setup_ui()
{
    m_main_layout = new QVBoxLayout(this);
    m_main_layout->setSpacing(20);
    m_main_layout->setContentsMargins(20, 20, 20, 20);
    
    // Title
    QLabel* titleLabel = new QLabel("Rezervare Nouă");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    m_main_layout->addWidget(titleLabel);
    
    // Offer info section
    m_offer_group = new QGroupBox("Detalii Ofertă");
    QGridLayout* offerLayout = new QGridLayout(m_offer_group);
    
    m_offer_name_label = new QLabel("Nume ofertă");
    m_offer_name_label->setStyleSheet("font-weight: bold; font-size: 16px;");
    offerLayout->addWidget(m_offer_name_label, 0, 0, 1, 2);
    
    m_destination_label = new QLabel("Destinație");
    offerLayout->addWidget(m_destination_label, 1, 0);
    
    m_dates_label = new QLabel("Perioada");
    offerLayout->addWidget(m_dates_label, 1, 1);
    
    m_price_per_person_label = new QLabel("Preț");
    m_price_per_person_label->setStyleSheet("color: #e74c3c; font-weight: bold;");
    offerLayout->addWidget(m_price_per_person_label, 2, 0, 1, 2);
    
    m_main_layout->addWidget(m_offer_group);
    
    // Booking details section
    m_booking_group = new QGroupBox("Detalii Rezervare");
    QVBoxLayout* bookingLayout = new QVBoxLayout(m_booking_group);
    
    // Person count
    QHBoxLayout* personCountLayout = new QHBoxLayout();
    personCountLayout->addWidget(new QLabel("Numărul de persoane:"));
    
    m_person_count_spin = new QSpinBox();
    m_person_count_spin->setRange(1, Config::Business::MAX_PERSONS_PER_RESERVATION);
    m_person_count_spin->setValue(1);
    m_person_count_spin->setStyleSheet("font-size: 14px; padding: 5px;");
    personCountLayout->addWidget(m_person_count_spin);
    
    personCountLayout->addStretch();
    bookingLayout->addLayout(personCountLayout);
    
    // Persons table
    setup_persons_table();
    bookingLayout->addWidget(m_persons_table);
    
    // Special requests
    QLabel* requestsLabel = new QLabel("Cerințe speciale (opțional):");
    bookingLayout->addWidget(requestsLabel);
    
    m_special_requests_edit = new QTextEdit();
    m_special_requests_edit->setMaximumHeight(80);
    m_special_requests_edit->setPlaceholderText("Ex: cameră cu vedere la mare, mâncare vegetariană, etc.");
    bookingLayout->addWidget(m_special_requests_edit);
    
    m_main_layout->addWidget(m_booking_group);
    
    // Price section
    m_price_group = new QGroupBox("Calculul Prețului");
    QGridLayout* priceLayout = new QGridLayout(m_price_group);
    
    priceLayout->addWidget(new QLabel("Subtotal:"), 0, 0);
    m_subtotal_label = new QLabel("€0");
    m_subtotal_label->setAlignment(Qt::AlignRight);
    priceLayout->addWidget(m_subtotal_label, 0, 1);
    
    priceLayout->addWidget(new QLabel("Taxe (19%):"), 1, 0);
    m_taxes_label = new QLabel("€0");
    m_taxes_label->setAlignment(Qt::AlignRight);
    priceLayout->addWidget(m_taxes_label, 1, 1);
    
    // Separator line
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    priceLayout->addWidget(line, 2, 0, 1, 2);
    
    QLabel* totalLabel = new QLabel("TOTAL:");
    totalLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    priceLayout->addWidget(totalLabel, 3, 0);
    
    m_total_price_label = new QLabel("€0");
    m_total_price_label->setAlignment(Qt::AlignRight);
    m_total_price_label->setStyleSheet("font-weight: bold; font-size: 18px; color: #e74c3c;");
    priceLayout->addWidget(m_total_price_label, 3, 1);
    
    m_main_layout->addWidget(m_price_group);
    
    // Status and progress
    m_status_label = new QLabel();
    m_status_label->setAlignment(Qt::AlignCenter);
    m_status_label->setWordWrap(true);
    m_status_label->hide();
    m_main_layout->addWidget(m_status_label);
    
    m_progress_bar = new QProgressBar();
    m_progress_bar->setRange(0, 0);
    m_progress_bar->hide();
    m_main_layout->addWidget(m_progress_bar);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_cancel_button = new QPushButton("Anulează");
    m_cancel_button->setStyleSheet(
        "QPushButton { padding: 10px 20px; font-size: 14px; }"
    );
    buttonLayout->addWidget(m_cancel_button);
    
    m_confirm_button = new QPushButton("Confirmă Rezervarea");
    m_confirm_button->setStyleSheet(
        "QPushButton { "
        "background-color: #28a745; "
        "color: white; "
        "padding: 10px 20px; "
        "font-size: 14px; "
        "font-weight: bold; "
        "border: none; "
        "border-radius: 6px; "
        "} "
        "QPushButton:hover { background-color: #218838; } "
        "QPushButton:disabled { background-color: #6c757d; }"
    );
    m_confirm_button->setDefault(true);
    buttonLayout->addWidget(m_confirm_button);
    
    m_main_layout->addLayout(buttonLayout);
}

void Booking_Dialog::setup_persons_table()
{
    m_persons_table = new QTableWidget(1, 4);
    
    QStringList headers = {"Nume Complet*", "CNP*", "Data Nașterii*", "Tip Persoană"};
    m_persons_table->setHorizontalHeaderLabels(headers);
    
    // Set column widths
    m_persons_table->setColumnWidth(0, 180);
    m_persons_table->setColumnWidth(1, 120);
    m_persons_table->setColumnWidth(2, 100);
    m_persons_table->setColumnWidth(3, 100);
    
    m_persons_table->horizontalHeader()->setStretchLastSection(true);
    m_persons_table->setAlternatingRowColors(true);
    m_persons_table->setMaximumHeight(200);
    
    update_persons_table();
}

void Booking_Dialog::setup_connections()
{
    // Person count changes
    connect(m_person_count_spin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &Booking_Dialog::on_person_count_changed);
    
    // Price calculation triggers
    connect(m_person_count_spin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &Booking_Dialog::on_calculate_total_price);
    
    // Buttons
    connect(m_confirm_button, &QPushButton::clicked, this, &Booking_Dialog::on_confirm_booking);
    connect(m_cancel_button, &QPushButton::clicked, this, &Booking_Dialog::on_cancel_booking);
    
    // Model connections
    connect(m_user_model.get(), &User_Model::login_success, 
            this, &Booking_Dialog::on_booking_success);
    // Note: In real implementation, connect to booking-specific signals
}

void Booking_Dialog::on_person_count_changed(int count)
{
    m_booking_data.number_of_persons = count;
    update_persons_table();
    update_price_calculation();
}

void Booking_Dialog::update_persons_table()
{
    int personCount = m_person_count_spin->value();
    m_persons_table->setRowCount(personCount);
    
    for (int row = 0; row < personCount; ++row)
    {
        // Name field
        QLineEdit* nameEdit = new QLineEdit();
        nameEdit->setPlaceholderText("Nume și prenume");
        m_persons_table->setCellWidget(row, 0, nameEdit);
        
        // CNP field
        QLineEdit* cnpEdit = new QLineEdit();
        cnpEdit->setPlaceholderText("1234567890123");
        cnpEdit->setMaxLength(13);
        // Add CNP validator
        QRegularExpression cnpRegex("\\d{13}");
        cnpEdit->setValidator(new QRegularExpressionValidator(cnpRegex, this));
        m_persons_table->setCellWidget(row, 1, cnpEdit);
        
        // Birth date field
        QDateEdit* birthEdit = new QDateEdit();
        birthEdit->setDate(QDate(1990, 1, 1));
        birthEdit->setMaximumDate(QDate::currentDate());
        birthEdit->setCalendarPopup(true);
        m_persons_table->setCellWidget(row, 2, birthEdit);
        
        // Person type combo
        QComboBox* typeCombo = new QComboBox();
        typeCombo->addItems({"Adult", "Copil (2-12 ani)", "Infant (0-2 ani)"});
        m_persons_table->setCellWidget(row, 3, typeCombo);
        
        // Connect change signals for validation
        connect(nameEdit, &QLineEdit::textChanged, this, &Booking_Dialog::validate_and_enable_booking);
        connect(cnpEdit, &QLineEdit::textChanged, this, &Booking_Dialog::validate_and_enable_booking);
    }
    
    validate_and_enable_booking();
}

void Booking_Dialog::on_calculate_total_price()
{
    update_price_calculation();
}

void Booking_Dialog::update_price_calculation()
{
    double subtotal = m_offer.price_per_person * m_person_count_spin->value();
    double taxes = subtotal * TAX_RATE;
    double total = subtotal + taxes;
    
    m_booking_data.total_price = total;
    
    if (m_subtotal_label) m_subtotal_label->setText(QString("€%1").arg(subtotal, 0, 'f', 0));
    if (m_taxes_label) m_taxes_label->setText(QString("€%1").arg(taxes, 0, 'f', 0));
    if (m_total_price_label) m_total_price_label->setText(QString("€%1").arg(total, 0, 'f', 0));
}

void Booking_Dialog::on_confirm_booking()
{
    if (!validate_persons_data() || !validate_required_fields())
    {
        return;
    }
    
    // Collect persons data
    m_booking_data.persons.clear();
    for (int row = 0; row < m_persons_table->rowCount(); ++row)
    {
        Booking_Data::Person_Info person;
        
        QLineEdit* nameEdit = qobject_cast<QLineEdit*>(m_persons_table->cellWidget(row, 0));
        QLineEdit* cnpEdit = qobject_cast<QLineEdit*>(m_persons_table->cellWidget(row, 1));
        QDateEdit* birthEdit = qobject_cast<QDateEdit*>(m_persons_table->cellWidget(row, 2));
        QComboBox* typeCombo = qobject_cast<QComboBox*>(m_persons_table->cellWidget(row, 3));
        
        if (nameEdit && cnpEdit && birthEdit && typeCombo)
        {
            person.full_name = nameEdit->text().trimmed();
            person.cnp = cnpEdit->text().trimmed();
            person.birth_date = birthEdit->date().toString("yyyy-MM-dd");
            person.person_type = typeCombo->currentText();
            
            m_booking_data.persons.append(person);
        }
    }
    
    // Note: special_requests field is not in struct, need to add if needed
    
    show_loading_state(true);
    
    // Simulate booking process (replace with real API call)
    QTimer::singleShot(2000, [this]()
        {
        show_loading_state(false);
        
        // Simulate success
        show_success("Rezervarea a fost confirmată cu succes!");
        emit booking_confirmed(m_booking_data);
        
        QTimer::singleShot(1500, [this]() {
            accept();
        });
    });
}

void Booking_Dialog::on_cancel_booking()
{
    emit booking_canceled();
    reject();
}

void Booking_Dialog::on_booking_success(const QString& message)
{
    show_loading_state(false);
    show_success(message);
    
    QTimer::singleShot(1500, [this]() {
        accept();
    });
}

void Booking_Dialog::on_booking_failed(const QString& error)
{
    show_loading_state(false);
    show_error("Rezervarea a eșuat: " + error);
}

bool Booking_Dialog::validate_persons_data()
{
    for (int row = 0; row < m_persons_table->rowCount(); ++row)
    {
        QLineEdit* nameEdit = qobject_cast<QLineEdit*>(m_persons_table->cellWidget(row, 0));
        QLineEdit* cnpEdit = qobject_cast<QLineEdit*>(m_persons_table->cellWidget(row, 1));
        
        if (!nameEdit || !cnpEdit) continue;
        
        if (nameEdit->text().trimmed().length() < 3)
        {
            show_error(QString("Numele pentru persoana %1 este invalid.").arg(row + 1));
            nameEdit->setFocus();
            return false;
        }
        
        if (cnpEdit->text().length() != 13)
        {
            show_error(QString("CNP-ul pentru persoana %1 trebuie să aibă 13 cifre.").arg(row + 1));
            cnpEdit->setFocus();
            return false;
        }
    }
    
    return true;
}

bool Booking_Dialog::validate_required_fields()
{
    if (m_person_count_spin->value() < 1)
    {
        show_error("Trebuie să selectați cel puțin o persoană.");
        return false;
    }
    
    return true;
}

void Booking_Dialog::validate_and_enable_booking()
{
    bool isValid = true;
    
    // Check if all required fields are filled
    for (int row = 0; row < m_persons_table->rowCount(); ++row)
    {
        QLineEdit* nameEdit = qobject_cast<QLineEdit*>(m_persons_table->cellWidget(row, 0));
        QLineEdit* cnpEdit = qobject_cast<QLineEdit*>(m_persons_table->cellWidget(row, 1));
        
        if (!nameEdit || !cnpEdit) continue;
        
        if (nameEdit->text().trimmed().isEmpty() || cnpEdit->text().trimmed().isEmpty())
        {
            isValid = false;
            break;
        }
    }
    
    m_confirm_button->setEnabled(isValid && !m_is_loading);
}

void Booking_Dialog::show_loading_state(bool loading)
{
    m_is_loading = loading;
    
    m_confirm_button->setEnabled(!loading);
    m_cancel_button->setEnabled(!loading);
    m_person_count_spin->setEnabled(!loading);
    m_persons_table->setEnabled(!loading);
    m_special_requests_edit->setEnabled(!loading);
    
    if (loading)
    {
        m_progress_bar->show();
        m_status_label->setText("Se procesează rezervarea...");
        m_status_label->setStyleSheet("color: #4a90e2; font-weight: bold;");
        m_status_label->show();
    }
    else
    {
        m_progress_bar->hide();
    }
}

void Booking_Dialog::show_error(const QString& message)
{
    m_status_label->setText(message);
    m_status_label->setStyleSheet("color: #e74c3c; font-weight: bold;");
    m_status_label->show();
}

void Booking_Dialog::show_success(const QString& message)
{
    m_status_label->setText(message);
    m_status_label->setStyleSheet("color: #27ae60; font-weight: bold;");
    m_status_label->show();
}
