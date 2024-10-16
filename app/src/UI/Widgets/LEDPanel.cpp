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

#include "UI/Dashboard.h"
#include "Misc/ThemeManager.h"
#include "UI/Widgets/LEDPanel.h"

/**
 * Generates the user interface elements & layout
 */
Widgets::LEDPanel::LEDPanel(const int index)
  : m_index(index)
{
  // Get pointers to serial studio modules
  auto dash = &UI::Dashboard::instance();

  // Invalid index, abort initialization
  if (m_index < 0 || m_index >= dash->ledCount())
    return;

  // Get group reference
  auto group = dash->getLED(m_index);

  // Configure scroll area container
  m_dataContainer = new QWidget(this);

  // Make the value label larger
  auto valueFont = dash->monoFont();
  valueFont.setPixelSize(dash->monoFont().pixelSize() * 1.3);

  // Configure grid layout
  m_leds.reserve(group.datasetCount());
  m_titles.reserve(group.datasetCount());
  m_gridLayout = new QGridLayout(m_dataContainer);
  m_gridLayout->setSpacing(16);
  for (int dataset = 0; dataset < group.datasetCount(); ++dataset)
  {
    // Create labels
    m_leds.append(new KLed(m_dataContainer));
    m_titles.append(new QLabel(m_dataContainer));

    // Get pointers to labels
    auto led = m_leds.last();
    auto title = m_titles.last();

    // Set label styles & fonts
    title->setFont(dash->monoFont());
    title->setText(group.getDataset(dataset).title());

    // Set LED color & style
    led->setLook(KLed::Sunken);
    led->setShape(KLed::Circular);

    // Calculate column and row
    int column = 0;
    int row = dataset;
    int count = dataset + 1;
    while (count > 3)
    {
      count -= 3;
      row -= 3;
      column += 2;
    }

    // Add label and LED to grid layout
    m_gridLayout->addWidget(led, row, column);
    m_gridLayout->addWidget(title, row, column + 1);
    m_gridLayout->setAlignment(led, Qt::AlignRight | Qt::AlignVCenter);
    m_gridLayout->setAlignment(title, Qt::AlignLeft | Qt::AlignVCenter);
  }

  // Load layout into container widget
  m_dataContainer->setLayout(m_gridLayout);

  // Configure scroll area
  m_scrollArea = new QScrollArea(this);
  m_scrollArea->setWidgetResizable(true);
  m_scrollArea->setWidget(m_dataContainer);
  m_scrollArea->setFrameShape(QFrame::NoFrame);
  m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // Configure main layout
  m_mainLayout = new QVBoxLayout(this);
  m_mainLayout->addWidget(m_scrollArea);
  m_mainLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_mainLayout);

  // Configure blinker timer
  m_blinkerTimer.setTimerType(Qt::PreciseTimer);
  m_blinkerTimer.setInterval(250);
  m_blinkerTimer.start();

  // Configure visual style
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &Widgets::LEDPanel::onThemeChanged);

  // React to dashboard events
  connect(dash, SIGNAL(updated()), this, SLOT(updateData()),
          Qt::DirectConnection);
}

/**
 * Frees the memory allocated for each label and LED that represents a dataset
 */
Widgets::LEDPanel::~LEDPanel()
{
  Q_FOREACH (auto led, m_leds)
    delete led;

  Q_FOREACH (auto title, m_titles)
    delete title;

  delete m_gridLayout;
  delete m_scrollArea;
  delete m_mainLayout;
}

/**
 * Checks if the widget is enabled, if so, the widget shall be updated
 * to display the latest data frame.
 *
 * If the widget is disabled (e.g. the user hides it, or the external
 * window is hidden), then the widget shall ignore the update request.
 */
void Widgets::LEDPanel::updateData()
{
  // Widget not enabled, do nothing
  if (!isEnabled())
    return;

  // Invalid index, abort update
  auto dash = &UI::Dashboard::instance();
  if (m_index < 0 || m_index >= dash->ledCount())
    return;

  // Get group pointer
  auto group = dash->getLED(m_index);

  // Update labels
  for (int i = 0; i < group.datasetCount(); ++i)
  {
    // Check vector size
    if (m_leds.count() < i)
      break;

    // Get dataset value
    const auto dataset = group.getDataset(i);
    const auto value = dataset.value().toDouble();
    if (value >= dataset.ledHigh())
      m_leds.at(i)->on();
    else
      m_leds.at(i)->off();

    // Blink the LED if alarm value is exceeded
    if (value >= dataset.alarm())
      connect(&m_blinkerTimer, &QTimer::timeout, m_leds.at(i), &KLed::toggle);
    else
      disconnect(&m_blinkerTimer, &QTimer::timeout, m_leds.at(i),
                 &KLed::toggle);
  }
}

/**
 * Changes the size of the labels when the widget is resized
 */
void Widgets::LEDPanel::resizeEvent(QResizeEvent *event)
{
  auto width = event->size().width();
  QFont font = UI::Dashboard::instance().monoFont();
  font.setPixelSize(qMax(8, width / 24));
  auto fHeight = QFontMetrics(font).height() * 1.5;

  for (int i = 0; i < m_titles.count(); ++i)
  {
    m_titles.at(i)->setFont(font);
    m_leds.at(i)->setMinimumSize(fHeight, fHeight);
  }

  event->accept();
}

/**
 * Updates the widget's visual style and color palette to match the colors
 * defined by the application theme file.
 */
void Widgets::LEDPanel::onThemeChanged()
{
  // Generate widget stylesheets
  auto theme = &Misc::ThemeManager::instance();

  // Set window palette
  QPalette palette;
  palette.setColor(QPalette::Base,
                   theme->getColor(QStringLiteral("widget_base")));
  palette.setColor(QPalette::Window,
                   theme->getColor(QStringLiteral("widget_window")));
  palette.setColor(QPalette::Text,
                   theme->getColor(QStringLiteral("widget_text")));
  setPalette(palette);

  // Configure each LED
  for (int i = 0; i < m_leds.count(); ++i)
  {
    auto *led = m_leds.at(i);
    auto *title = m_titles.at(i);
    title->setPalette(palette);
    led->setColor(theme->getColor("led_color"));
  }
}
