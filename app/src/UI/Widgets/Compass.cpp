/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <QResizeEvent>
#include <QwtCompassScaleDraw>
#include <QwtCompassMagnetNeedle>

#include "UI/Dashboard.h"
#include "Misc/ThemeManager.h"
#include "UI/Widgets/Compass.h"

/**
 * Constructor function, configures widget style & signal/slot connections.
 */
Widgets::Compass::Compass(const int index)
  : m_index(index)
{
  // Get pointers to serial studio modules
  auto dash = &UI::Dashboard::instance();

  // Invalid index, abort initialization
  if (m_index < 0 || m_index >= dash->compassCount())
    return;

  // Set compass style
  QwtCompassScaleDraw *scaleDraw = new QwtCompassScaleDraw();
  scaleDraw->enableComponent(QwtAbstractScaleDraw::Ticks, true);
  scaleDraw->enableComponent(QwtAbstractScaleDraw::Labels, true);
  scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
  scaleDraw->setTickLength(QwtScaleDiv::MinorTick, 1);
  scaleDraw->setTickLength(QwtScaleDiv::MediumTick, 1);
  scaleDraw->setTickLength(QwtScaleDiv::MajorTick, 3);

  // Configure compass scale & needle
  m_compass.setScaleDraw(scaleDraw);
  m_compass.setScaleMaxMajor(36);
  m_compass.setScaleMaxMinor(5);
  m_compass.setNeedle(
      new QwtCompassMagnetNeedle(QwtCompassMagnetNeedle::ThinStyle));

  // Set widget pointer
  setWidget(&m_compass);

  // Set visual style
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &Widgets::Compass::onThemeChanged);

  // Connect update signal
  connect(dash, &UI::Dashboard::updated, this, &Compass::updateData,
          Qt::DirectConnection);
}

/**
 * Checks if the widget is enabled, if so, the widget shall be updated
 * to display the latest data frame.
 *
 * If the widget is disabled (e.g. the user hides it, or the external
 * window is hidden), then the widget shall ignore the updateData request.
 */
void Widgets::Compass::updateData()
{
  // Widget disabled
  if (!isEnabled())
    return;

  // Invalid index, abort update
  auto dash = &UI::Dashboard::instance();
  if (m_index < 0 || m_index >= dash->compassCount())
    return;

  // Get dataset value & set text format
  auto dataset = dash->getCompass(m_index);
  auto value = dataset.value().toDouble();
  auto text = QStringLiteral("%1°").arg(
      QString::number(value, 'f', UI::Dashboard::instance().precision()));

  // Ensure that angle always has 3 characters
  if (text.length() == 2)
    text.prepend(QStringLiteral("00"));
  else if (text.length() == 3)
    text.prepend(QStringLiteral("0"));

  // Update gauge
  setValue(text);
  m_compass.setValue(value);
}

/**
 * Updates the widget's visual style and color palette to match the colors
 * defined by the application theme file.
 */
void Widgets::Compass::onThemeChanged()
{
  auto theme = &Misc::ThemeManager::instance();
  QPalette palette;
  palette.setColor(QPalette::WindowText,
                   theme->getColor(QStringLiteral("groupbox_background")));
  palette.setColor(QPalette::Text,
                   theme->getColor(QStringLiteral("widget_text")));
  m_compass.setPalette(palette);
}
