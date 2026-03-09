//
// Created by Daniel White on 3/5/26.
//

#ifndef ASTGEN_CAMERA_H
#define ASTGEN_CAMERA_H

#include <SFML/Graphics.hpp>

class Camera {
public:
    Camera(sf::Vector2u worldSize,
           sf::Vector2u windowSize,
           sf::Vector2f startPos)
        : m_windowSize(windowSize),
          m_zoomCenter(startPos)
    {
        m_mainView = sf::View(
            {worldSize.x/2.f, worldSize.y/2.f},
            static_cast<sf::Vector2f>(worldSize));

        m_zoomView = sf::View(
            startPos,
            {windowSize.x / 8.f, windowSize.y / 8.f}
            );

        m_currentView = &m_mainView;
    }

    void HandleEvent(sf::RenderWindow& window, const sf::Event& event)
    {
        if (auto mouse = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (m_currentView == &m_zoomView)
            {
                m_dragging = true;
                m_dragStart =
                    window.mapPixelToCoords(
                        sf::Mouse::getPosition(window));
            }
        }

        if (event.is<sf::Event::MouseButtonReleased>())
        {
            m_dragging = false;
        }

        if (auto key = event.getIf<sf::Event::KeyPressed>())
        {
            if (key->code == sf::Keyboard::Key::Equal)
                ToggleView();
            if (key->code == sf::Keyboard::Key::Left) {
                m_zoomCenter.x -= 50;
                m_zoomView.setCenter(m_zoomCenter);
            }
            if (key->code == sf::Keyboard::Key::Right) {
                m_zoomCenter.x += 50;
                m_zoomView.setCenter(m_zoomCenter);
            }
            if (key->code == sf::Keyboard::Key::Up) {
                m_zoomCenter.y -= 50;
                m_zoomView.setCenter(m_zoomCenter);
            }
            if (key->code == sf::Keyboard::Key::Down) {
                m_zoomCenter.y += 50;
                m_zoomView.setCenter(m_zoomCenter);
            }
        }
    }

    void Update(sf::RenderWindow& window)
    {
        if (!m_dragging || m_currentView != &m_zoomView)
            return;

        auto mousePos = sf::Mouse::getPosition(window);
        auto newWorldPos = window.mapPixelToCoords(mousePos);

        auto diff = m_dragStart - newWorldPos;

        m_zoomCenter += diff;

        m_zoomView.setCenter(m_zoomCenter);
    }

    void Apply(sf::RenderWindow& window)
    {
        window.setView(*m_currentView);
    }

    sf::Vector2f GetZoomCenter() const
    {
        return m_zoomCenter;
    }

private:

    void ToggleView()
    {
        if (m_currentView == &m_zoomView)
            m_currentView = &m_mainView;
        else
            m_currentView = &m_zoomView;
    }

    sf::Vector2u m_windowSize;

    sf::View m_mainView;
    sf::View m_zoomView;
    sf::View* m_currentView;

    sf::Vector2f m_zoomCenter;

    bool m_dragging = false;
    sf::Vector2f m_dragStart;
};


#endif //ASTGEN_CAMERA_H