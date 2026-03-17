//
// Created by Daniel White on 3/5/26.
//

#ifndef ASTGEN_CAMERA_H
#define ASTGEN_CAMERA_H

#include <SFML/Graphics.hpp>

class Camera {
public:
    Camera(sf::Vector2u windowSize,
           sf::Vector2f startPos)
        : m_windowSize(windowSize),
          m_zoomCenter(startPos)
    {
        m_zoomView = sf::View(
            startPos,
            {windowSize.x / 8.f, windowSize.y / 8.f}
            );
    }

    void HandleEvent(sf::RenderWindow& window, const sf::Event& event)
    {
        if (auto mouse = event.getIf<sf::Event::MouseButtonPressed>())
        {
            m_dragging = true;
            m_dragStart =
                window.mapPixelToCoords(
                    sf::Mouse::getPosition(window));
        }

        if (event.is<sf::Event::MouseButtonReleased>())
        {
            m_dragging = false;
        }

        if (auto key = event.getIf<sf::Event::KeyPressed>())
        {
            if (key->code == sf::Keyboard::Key::Equal) {
                ZoomInView();
            }

            if (key->code == sf::Keyboard::Key::Hyphen) {
                //ToggleView();
                ZoomOutView();
            }
            if (key->code == sf::Keyboard::Key::Left) {
                m_zoomCenter.x -= 5;
                m_zoomView.setCenter(m_zoomCenter);
            }
            if (key->code == sf::Keyboard::Key::Right) {
                m_zoomCenter.x += 5;
                m_zoomView.setCenter(m_zoomCenter);
            }
            if (key->code == sf::Keyboard::Key::Up) {
                m_zoomCenter.y -= 5;
                m_zoomView.setCenter(m_zoomCenter);
            }
            if (key->code == sf::Keyboard::Key::Down) {
                m_zoomCenter.y += 5;
                m_zoomView.setCenter(m_zoomCenter);
            }
        }
    }

    void Update(sf::RenderWindow& window)
    {
        window.setView(m_zoomView);
        if (!m_dragging)
            return;

        auto mousePos = sf::Mouse::getPosition(window);
        auto newWorldPos = window.mapPixelToCoords(mousePos);

        auto diff = m_dragStart - newWorldPos;

        m_zoomCenter += diff;
        m_zoomView.setCenter(m_zoomCenter);

    }

    sf::Vector2f GetZoomCenter() const
    {
        return m_zoomCenter;
    }

private:

    void ZoomInView() {
        m_zoomView.setSize(sf::Vector2f(m_zoomView.getSize().x / 2.f, m_zoomView.getSize().y / 2.f));
    }

    void ZoomOutView() {
        m_zoomView.setSize(sf::Vector2f(m_zoomView.getSize().x * 2.f, m_zoomView.getSize().y * 2.f));
    }

    sf::Vector2u m_windowSize;

    sf::View m_zoomView;
    sf::Vector2f m_zoomCenter;

    bool m_dragging = false;
    sf::Vector2f m_dragStart;
};


#endif //ASTGEN_CAMERA_H