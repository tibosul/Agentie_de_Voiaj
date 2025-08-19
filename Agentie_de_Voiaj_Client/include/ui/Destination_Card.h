#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QEnterEvent>
#include <memory>

#include "models/Destination_Model.h"

class Destination_Card : public QWidget
{
    Q_OBJECT

public:
    explicit Destination_Card(QWidget* parent = nullptr);
    explicit Destination_Card(const Destination_Model::Destination& destination, QWidget* parent = nullptr);
    ~Destination_Card();

    void set_destination(const Destination_Model::Destination& destination);
    const Destination_Model::Destination& get_destination() const;

    void set_card_size(const QSize& size);
    void set_image_size(const QSize& size);
    void set_animation_enabled(bool enabled);

signals:
    void clicked();
    void view_offers_clicked(int destination_id);
    void image_clicked(int destination_id);
    void favourite_toggled(int destination_id, bool is_favourite);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private slots:
    void on_view_offers_clicked();
    void on_favourite_clicked();

private:
    void setup_ui();
    void setup_animations();
    void update_content();
    void load_destination_image();

    void animate_hover_enter();
    void animate_hover_leave();
    void animate_press();

    QVBoxLayout* m_main_layout;
    QWidget* m_image_container;
    QLabel* m_image_label;
    QLabel* m_name_label;
    QLabel* m_country_label;
    QLabel* m_description_label;
    QPushButton* m_view_offers_button;
    QPushButton* m_favorite_button;

    Destination_Model::Destination m_destination;

    std::unique_ptr<QGraphicsDropShadowEffect> m_shadow_effect;
    std::unique_ptr<QPropertyAnimation> m_hover_animation;
    std::unique_ptr<QPropertyAnimation> m_press_animation;

    QSize m_card_size;
    QSize m_image_size;
    bool m_animation_enabled;
    bool m_is_favorite;
    bool m_is_hovered;
};